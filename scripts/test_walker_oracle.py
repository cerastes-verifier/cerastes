#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Self-test for scripts/walker_oracle.py: the oracle must fail a golden that
disagrees with `--il_display`, whatever the disagreement.

The oracle is what makes the walker goldens trustworthy (they are
expectations, invariant 1), so each check of it needs a twin: this file injects
discrepancies into a real golden (a line deleted or duplicated, a column or a
line moved, a type, an operation kind, a signature, a constant, a name) and into
the text of `--il_display` (a kind, a name, a type, positions), and requires that
every one of them is reported.  It also covers the report's verdict (a golden
with no checkable fact fails; so does a missing `end walk:` line), the golden
reader, test discovery and the command line's exit statuses (0 clean, 1 a
failure, 2 a usage error).

The pure parts run always.  The rest needs the built front end
(scripts/cerastes-fe.sh): small sources are compiled at test time and their
walker listing and il_display are the fixtures.  Those classes are skipped when
the front end is missing, unless CERASTES_REQUIRE_E2E=1 (scripts/check.sh sets
it), in which case they fail: a missing build must not switch them off.

Known gap of the oracle (reported to its owner, pinned here as an expected
failure that turns into "unexpected success" when it is fixed): a duplicated `arg`
line of a statement attribute is not noticed.  The second count of the `end walk:`
line (bodies in user headers) is compared with il_display's own source-file entries.
"""

import atexit
import contextlib
import io
import os
import re
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_tests as rt  # noqa: E402
import walker_oracle as w  # noqa: E402

REAL_ROOT = Path(__file__).resolve().parent.parent
ORACLE = REAL_ROOT / "scripts" / "walker_oracle.py"
FE_SCRIPT = REAL_ROOT / "scripts" / "cerastes-fe.sh"

END = ("end walk: {n} routines reported, {u} routines with bodies in non-system files "
       "outside the primary file")


def e2e_gate(available, reason):
    """Skip the class without the toolchain, or fail it under CERASTES_REQUIRE_E2E=1."""
    if available:
        return lambda cls: cls
    if os.environ.get("CERASTES_REQUIRE_E2E") == "1":
        def decorate(cls):
            def fail(klass):
                raise AssertionError("CERASTES_REQUIRE_E2E=1 but the end-to-end "
                                     f"prerequisites are missing: {reason}")
            cls.setUpClass = classmethod(fail)
            return cls
        return decorate
    return unittest.skip(reason)


def _front_end_available():
    try:
        fe = rt.load_env().get("CERASTES_FE")
    except Exception:  # noqa: BLE001 - any trouble reading the environment: not available
        return False
    return bool(fe) and Path(fe).is_file() and FE_SCRIPT.is_file()


# ---------------------------------------------------------------------------
# The fixtures: small real sources, their walker listing and their il_display
# ---------------------------------------------------------------------------

CPP_SOURCE = """\
#include <cerastes.h>
template <typename T>
T inc(T v) { return (T)(v + 1); }
int plain(int a, int b) CERASTES_REQUIRES(a > 0) CERASTES_ENSURES(r, r > a) { return a * b + 1; }
int decl(int n, char c);
int loop(int x)
{
  int i = 0;
  CERASTES_INVARIANT(i <= x)
  while (i < x) { i++; }
  return i;
}
int use() { return inc(1) + plain(2, 3) + loop(2); }
template <typename T>
T c100() { return (T)100; }
"""

C_SOURCE = """\
struct point { int x; int y; };
int sq(int v) { return v * v; }
int dot(const struct point* a, const struct point* b) { return a->x * b->x + a->y * b->y; }
int decl(int n, char c);
"""

_tmp = tempfile.mkdtemp(prefix="walker-oracle-selftest-")
atexit.register(shutil.rmtree, _tmp, True)
_fixtures = {}


class Fixture:
    """A source file, its walker listing (the golden) and its il_display."""

    def __init__(self, name, source, mode, header="", files=None):
        self.mode = mode
        self.path = Path(_tmp) / name
        self.path.write_text(header + source)
        for extra, text in (files or {}).items():  # headers next to the source
            (Path(_tmp) / extra).write_text(text)
        self.golden = self.walk()
        self.il = w.run_il_display(self.path, mode)
        self.lines = self.golden.split("\n")

    def walk(self):
        run = subprocess.run([str(FE_SCRIPT), w.MODE_OPTIONS[self.mode], "--cerastes_walk",
                              str(self.path)], capture_output=True, text=True, cwd=REAL_ROOT)
        if run.returncode != 0 or not run.stdout:
            raise RuntimeError(f"walker failed on {self.path}: {run.stderr[-300:]}")
        return run.stdout


def fixture(name, source, mode, header="", files=None):
    key = (name, header)
    if key not in _fixtures:
        _fixtures[key] = Fixture(name, source, mode, header, files)
    return _fixtures[key]


def cpp():
    return fixture("a.cpp", CPP_SOURCE, "c++20")


def c():
    return fixture("b.c", C_SOURCE, "c23")


# ---------------------------------------------------------------------------
# The golden reader, the report, discovery: no front end needed
# ---------------------------------------------------------------------------

GOLDEN = f"""\
routine f plain body @ L3:5
  returns int
  param 1 x: int
  attr [[cerastes::requires]] on routine @ L3:17
    arg 1 @ L3:30
      op eok_gt type_kind=tk_integer type=bool @ L3:32
        param 1 x: int @ L3:30
routine g prototype body @ L9:3
  returns T
  op eok_add type_kind=tk_template_param type=<tptk_unknown> @ L9:20
warning: something the walker says
attr [[unreg_ns::z]] on variable v @ L12:3
{END.format(n=2, u=0)}
--- stderr ---
routine ignored plain body @ L1:1
"""


class ParseGoldenTests(unittest.TestCase):
    def test_a_listing(self):
        g = w.parse_golden(GOLDEN)
        self.assertEqual([v.header for v in g.views], ["routine f plain body @ L3:5",
                                                       "routine g prototype body @ L9:3"])
        self.assertEqual(g.end, (2, 0))
        self.assertEqual(g.warnings, ["warning: something the walker says"])
        self.assertEqual([b.head for b in g.top], ["attr [[unreg_ns::z]] on variable v @ L12:3"])
        f = g.views[0]
        self.assertEqual(f.sig, ["returns int", "param 1 x: int"])
        self.assertEqual(len(f.attrs), 1)
        self.assertEqual(g.views[1].ops, ["op eok_add type_kind=tk_template_param "
                                          "type=<tptk_unknown> @ L9:20"])

    def test_the_stderr_section_is_not_the_listing(self):
        g = w.parse_golden(GOLDEN)
        self.assertNotIn("ignored", " ".join(v.header for v in g.views))

    def test_a_missing_end_line_is_none(self):
        text = "\n".join(l for l in GOLDEN.split("\n") if not l.startswith("end walk"))
        self.assertIsNone(w.parse_golden(text).end)

    def test_the_end_line_must_have_the_exact_form(self):
        for bad in ("end walk: 2 routines reported",
                    "end walk: 2 routines reported, x routines with bodies in non-system files "
                    "outside the primary file",
                    "end walk: 2 routines reported, 0 routines with bodies in non-system files "
                    "outside the primary file extra"):
            with self.subTest(bad):
                self.assertIsNone(w.parse_golden(bad + "\n").end)

    def test_blank_lines_and_the_no_newline_marker_are_ignored(self):
        g = w.parse_golden("routine f plain body @ L1:1\n\n  returns int\n"
                           "\\ No newline at end of stdout\n" + END.format(n=1, u=0) + "\n")
        self.assertEqual((len(g.views), g.end), (1, (1, 0)))


class ReportTests(unittest.TestCase):
    def test_nothing_checked_is_not_ok(self):
        self.assertFalse(w.Report().ok)

    def test_a_checked_fact_is_ok(self):
        rep = w.Report()
        rep.fact("routine")
        self.assertTrue(rep.ok)
        self.assertEqual(rep.checked, 1)

    def test_a_mismatch_is_not_ok_however_much_was_checked(self):
        rep = w.Report()
        rep.fact("routine", 100)
        rep.mismatches.append("x")
        self.assertFalse(rep.ok)

    def test_unverifiable_facts_are_not_checked_facts(self):
        rep = w.Report()
        rep.unverifiable["x"] += 5
        rep.fact("routine", 0)
        self.assertEqual(rep.checked, 0)
        self.assertFalse(rep.ok)


class DiscoverTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.dir = Path(tmp.name).resolve()

    def put(self, name, text=""):
        path = self.dir / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)
        return path

    def test_golden_tests_in_their_modes(self):
        self.put("a.cpp", "// expect: golden\nint f();\n")
        self.put("a.cpp.golden", "x")
        self.put("b.c", "// expect: golden\nint f(void);\n")
        self.put("b.c.golden", "x")
        self.put("m.c", "// expect: golden\n// mode: c++20, c23\nint f(void);\n")
        self.put("m.c.c++20.golden", "x")
        self.put("m.c.c23.golden", "x")
        jobs = w.discover([str(self.dir)])
        self.assertEqual([(j.source.name, j.mode, j.golden.name) for j in jobs], [
            ("a.cpp", "c++20", "a.cpp.golden"), ("b.c", "c23", "b.c.golden"),
            ("m.c", "c++20", "m.c.c++20.golden"), ("m.c", "c23", "m.c.c23.golden")])

    def test_tests_that_are_not_golden_tests_are_skipped(self):
        self.put("v.cpp", "// expect: verified\nint f();\n")
        self.put("e.cpp", "// expect: frontend-error undefined_identifier @ L2\n")
        self.put("n.cpp", "int f();\n")
        self.put("h.hpp", "// expect: golden\n")
        self.assertEqual(w.discover([str(self.dir)]), [])

    def test_the_expect_line_must_be_in_the_header(self):
        self.put("late.cpp", "int x;\n// expect: golden\n")
        self.assertEqual(w.discover([str(self.dir)]), [])

    def test_a_missing_golden_is_an_error(self):
        self.put("a.cpp", "// expect: golden\nint f();\n")
        with self.assertRaises(w.OracleError) as ctx:
            w.discover([str(self.dir)])
        self.assertIn("is missing", str(ctx.exception))

    def test_a_missing_golden_of_one_mode_is_an_error(self):
        self.put("m.c", "// expect: golden\n// mode: c++20, c23\n")
        self.put("m.c.c23.golden", "x")
        with self.assertRaises(w.OracleError):
            w.discover([str(self.dir)])

    def test_a_golden_file_names_its_test(self):
        self.put("a.cpp", "// expect: golden\n")
        self.put("a.cpp.golden", "x")
        self.put("m.c", "// expect: golden\n// mode: c++20, c23\n")
        self.put("m.c.c23.golden", "x")
        self.put("m.c.c++20.golden", "x")
        (job,) = w.discover([str(self.dir / "a.cpp.golden")])
        self.assertEqual((job.source.name, job.mode), ("a.cpp", "c++20"))
        # a golden names its test: every mode of the test is checked
        jobs = w.discover([str(self.dir / "m.c.c23.golden")])
        self.assertEqual([(j.source.name, j.mode) for j in jobs],
                         [("m.c", "c++20"), ("m.c", "c23")])

    def test_bad_paths_are_errors(self):
        self.put("x.golden", "x")
        for path in (self.dir / "nowhere", self.dir / "x.golden"):
            with self.subTest(path.name), self.assertRaises(w.OracleError):
                w.discover([str(path)])

    def test_a_file_named_twice_is_one_job(self):
        self.put("a.cpp", "// expect: golden\n")
        self.put("a.cpp.golden", "x")
        jobs = w.discover([str(self.dir / "a.cpp"), str(self.dir), str(self.dir / "a.cpp.golden")])
        self.assertEqual(len(jobs), 1)

    def test_the_default_paths_are_pinned(self):
        # Dropping one would silently stop checking its goldens.
        self.assertEqual(w.DEFAULT_PATHS, ("tests", "doc/modules/ROOT/examples"))


def job(name="a.cpp", mode="c++20"):
    return w.Job(Path(f"/x/{name}"), mode, Path(f"/x/{name}.golden"))


def good_report():
    rep = w.Report()
    rep.fact("routine", 3)
    return rep


class MainTests(unittest.TestCase):
    """The exit status and the output of the command line, with check_job and
    discover replaced."""

    def run_main(self, results, jobs=None, argv=()):
        jobs = [r[0] for r in results] if jobs is None else jobs
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err), \
                mock.patch.object(w, "discover", return_value=jobs), \
                mock.patch.object(w, "check_job", side_effect=lambda j, cache=None: next(
                    r for r in results if r[0] is j)):
            status = w.main(["-j", "1", *argv, "x"])
        return status, out.getvalue(), err.getvalue()

    def test_all_clean_is_zero(self):
        status, out, _ = self.run_main([(job(), good_report(), None)])
        self.assertEqual(status, 0, out)
        self.assertIn("1 goldens, 3 facts checked, 0 failed", out)
        self.assertNotIn("FAIL", out)

    def test_a_mismatch_is_one(self):
        rep = good_report()
        rep.mismatches.append("op: not in il_display")
        status, out, _ = self.run_main([(job(), rep, None)])
        self.assertEqual(status, 1, out)
        self.assertIn("FAIL", out)
        self.assertIn("op: not in il_display", out)
        self.assertIn("1 failed", out)

    def test_a_golden_that_checks_nothing_is_one(self):
        status, out, _ = self.run_main([(job(), w.Report(), None)])
        self.assertEqual(status, 1, out)
        self.assertIn("no checkable fact", out)

    def test_a_front_end_failure_is_one(self):
        status, out, _ = self.run_main([(job(), None, "x.cpp [c++20]: front end failed (status 2)")])
        self.assertEqual(status, 1, out)
        self.assertIn("x.cpp [c++20]: front end failed (status 2)", out)
        self.assertTrue(out.startswith("FAIL "), out)

    def test_one_failure_among_good_ones_is_one(self):
        bad = good_report()
        bad.mismatches.append("m")
        status, out, _ = self.run_main([(job("a.cpp"), good_report(), None),
                                        (job("b.cpp"), bad, None),
                                        (job("c.cpp"), good_report(), None)])
        self.assertEqual(status, 1, out)
        self.assertIn("3 goldens, 9 facts checked, 1 failed", out)

    def test_every_failure_is_listed(self):
        bad = good_report()
        bad.mismatches.append("m")
        status, out, _ = self.run_main([(job("a.cpp"), bad, None), (job("b.cpp"), None, "boom")])
        self.assertEqual(out.count("FAIL"), 2)
        self.assertIn("2 failed", out)

    def test_mismatches_are_cut_but_counted(self):
        bad = good_report()
        bad.mismatches += [f"mismatch {i}" for i in range(45)]
        _, out, _ = self.run_main([(job(), bad, None)])
        self.assertIn("mismatch 39", out)
        self.assertNotIn("mismatch 40", out)
        self.assertIn("5 more", out)

    def test_a_usage_error_is_two(self):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err), \
                mock.patch.object(w, "discover", side_effect=w.OracleError("x: no such file")):
            status = w.main(["x"])
        self.assertEqual(status, 2)
        self.assertIn("walker_oracle: x: no such file", err.getvalue())

    def test_no_goldens_is_two(self):
        status, out, err = self.run_main([], jobs=[])
        self.assertEqual(status, 2, out)
        self.assertIn("no golden tests found", err)

    def test_verbose_lists_the_clean_ones(self):
        _, quiet, _ = self.run_main([(job(), good_report(), None)])
        _, loud, _ = self.run_main([(job(), good_report(), None)], argv=["-v"])
        self.assertNotIn("ok   ", quiet)
        self.assertRegex(loud, r"(?m)^ok   \S*a\.cpp \[c\+\+20\]: 3 checked")

    def test_the_default_paths_are_used_without_arguments(self):
        seen = []
        out = io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(io.StringIO()), \
                mock.patch.object(w, "discover", side_effect=lambda p: seen.append(p) or []):
            w.main([])
        self.assertEqual(seen, [list(w.DEFAULT_PATHS)])


# ---------------------------------------------------------------------------
# Real front end: baselines, injected discrepancies, the command line
# ---------------------------------------------------------------------------

def golden_mutants(lines):
    """(kind, line number, text) of every single mutation of the listing that
    the oracle must notice: a line deleted or duplicated, a column or a line
    moved, a type, an operation kind, a signature, a constant or a name changed.
    The `end walk:` line and what follows the stderr marker are left alone."""
    cut = lines.index("--- stderr ---") if "--- stderr ---" in lines else len(lines)
    for i, ln in enumerate(lines[:cut]):
        if not ln.strip() or ln.startswith("end walk"):
            continue

        def with_line(new):
            return "\n".join(lines[:i] + [new] + lines[i + 1:])
        yield "delete", i + 1, "\n".join(lines[:i] + lines[i + 1:])
        yield "duplicate", i + 1, "\n".join(lines[:i + 1] + [ln] + lines[i + 1:])
        if m := re.search(r"@ L(\d+):(\d+)", ln):
            yield "column", i + 1, with_line(
                ln[:m.start()] + f"@ L{m.group(1)}:{int(m.group(2)) + 1}" + ln[m.end():])
            yield "line", i + 1, with_line(
                ln[:m.start()] + f"@ L{int(m.group(1)) + 1}:{m.group(2)}" + ln[m.end():])
        if m := re.search(r"type=([^ ]+)", ln):
            yield "type", i + 1, with_line(ln[:m.end(1)] + "X" + ln[m.end(1):])
        if m := re.search(r"\beok_\w+", ln):
            yield "operation kind", i + 1, with_line(ln[:m.start()] + "eok_xxx" + ln[m.end():])
        if ln.lstrip().startswith(("param", "returns")):
            yield "signature", i + 1, with_line(ln + "Z")
        if m := re.search(r"const (-?\d+)", ln):
            yield "constant", i + 1, with_line(
                ln[:m.start(1)] + str(int(m.group(1)) + 1) + ln[m.end(1):])
        if ln.lstrip().startswith(("var", "binder", "arg", "param")) and (
                m := re.search(r"(?:var|binder|param \d+) (\w+)", ln)):
            yield "name", i + 1, with_line(ln[:m.end(1)] + "Q" + ln[m.end(1):])


# Mutations of the text of il_display, each applied to every occurrence, so that
# the record the listing is about is hit wherever it is.  (name, function)
def _shift(pattern):
    return lambda il: re.sub(pattern, lambda m: m.group(1) + str(int(m.group(2)) + 1), il)


IL_MUTATIONS_CPP = {
    "an operation kind (add -> sub)": lambda il: il.replace("eok_add", "eok_sub"),
    "an operation kind in a contract (gt -> ge)": lambda il: il.replace("eok_gt", "eok_ge"),
    "an operation kind in a statement attribute (le -> lt)":
        lambda il: il.replace("eok_le", "eok_lt"),
    "a cast kind": lambda il: il.replace("eok_cast", "eok_bool_cast"),
    "a type_kind": lambda il: il.replace("tk_template_param", "tk_integer"),
    "a routine name": lambda il: re.sub(r"\bdecl\b", "decl_x", il),
    "another routine name": lambda il: re.sub(r"\bplain\b", "plan", il),
    "a parameter type": lambda il: re.sub(r"(type@[0-9a-f]+: )char\b", r"\1short", il),
    "an attribute name": lambda il: il.replace("ensures", "requires"),
    "every position column": _shift(r"(position\.column:\s+)(\d+)"),
    "every declaration column": _shift(r"(decl_position\.column:\s+)(\d+)"),
    "every position sequence": _shift(r"(position\.seq:\s+)(\d+)"),
    "the second half is missing": lambda il: il[:len(il) // 2],
    "il_display is empty": lambda il: "",
}
IL_MUTATIONS_C = {
    "a parameter type": lambda il: re.sub(r"(type@[0-9a-f]+: )char\b", r"\1short", il),
    "a routine name": lambda il: re.sub(r"\bdot\b", "dott", il),
    "every declaration column": _shift(r"(decl_position\.column:\s+)(\d+)"),
}


@e2e_gate(_front_end_available(), "build/cerastes-fe missing; run scripts/bootstrap.sh")
class OracleBaselineTests(unittest.TestCase):
    def test_the_fixtures_agree_with_their_il_display(self):
        for name, fx in (("a.cpp", cpp()), ("b.c", c())):
            with self.subTest(name):
                rep = w.analyse(fx.golden, fx.il)
                self.assertEqual(rep.mismatches, [])
                self.assertTrue(rep.ok)
                self.assertGreaterEqual(rep.checked, 3)
        self.assertGreater(w.analyse(cpp().golden, cpp().il).checked, 10)

    def test_the_fixtures_exercise_every_kind_of_fact_we_mutate(self):
        rep = w.analyse(cpp().golden, cpp().il)
        for category in ("routine", "op", "tmpl", "tconst", "attribute", "statement-attribute",
                         "statement-annotation-tree"):
            self.assertGreater(rep.checked_by[category], 0, category)
        for text in ("prototype", "instance", "[[cerastes::requires]]", "[[cerastes::ensures]]",
                     "binder r", "stmt stmk_while", "const 0", "var i", "tconst tpck_expression"):
            self.assertIn(text, cpp().golden)

    def test_the_real_goldens_that_the_fixtures_stand_for_pass(self):
        # one real golden of each language, through the same function the CLI uses
        for source, mode in (("tests/il/walker/basic.c", "c23"),
                             ("tests/annotations/contracts/ensures_plain.cpp", "c++20")):
            with self.subTest(source):
                src = REAL_ROOT / source
                _job, rep, err = w.check_job(w.Job(src, mode, src.with_name(src.name + ".golden")))
                self.assertIsNone(err)
                self.assertTrue(rep.ok, rep.mismatches)


@e2e_gate(_front_end_available(), "build/cerastes-fe missing; run scripts/bootstrap.sh")
class GoldenMutationTests(unittest.TestCase):
    """Every discrepancy injected into the golden is reported."""

    def check_all(self, fx, allowed_survivors=()):
        survivors = []
        kinds = set()
        for kind, lineno, text in golden_mutants(fx.lines):
            kinds.add(kind)
            if (kind, fx.lines[lineno - 1].strip()) in allowed_survivors:
                continue
            if w.analyse(text, fx.il).ok:
                survivors.append((kind, lineno, fx.lines[lineno - 1].strip()[:80]))
        self.assertEqual(survivors, [], "the oracle accepted a golden with an injected "
                                        "discrepancy")
        return kinds

    def test_cpp_listing(self):
        kinds = self.check_all(cpp(), {("duplicate", "arg 1 @ L9:22")})  # a known gap, below
        self.assertEqual(kinds, {"delete", "duplicate", "column", "line", "type",
                                 "operation kind", "signature", "constant", "name"})

    def test_c_listing(self):
        kinds = self.check_all(c())
        self.assertTrue({"delete", "duplicate", "column", "line", "signature", "name"} <= kinds)

    def test_every_mutation_changes_the_golden(self):
        for fx in (cpp(), c()):
            for kind, lineno, text in golden_mutants(fx.lines):
                self.assertNotEqual(text, fx.golden, (kind, lineno))

    def test_a_whole_routine_added_or_removed(self):
        lines = cpp().lines
        start = next(i for i, l in enumerate(lines) if l.startswith("routine decl"))
        stop = next(i for i in range(start + 1, len(lines)) if not lines[i].startswith(" "))
        block = lines[start:stop]
        self.assertFalse(w.analyse("\n".join(lines[:start] + lines[stop:]), cpp().il).ok)
        self.assertFalse(w.analyse("\n".join(lines[:stop] + block + lines[stop:]), cpp().il).ok)
        # with the `end walk:` count corrected as well, only the routine itself is wrong
        e = next(i for i, l in enumerate(lines) if l.startswith("end walk"))
        n = int(re.search(r"\d+", lines[e]).group())
        for delta, text in ((-1, lines[:start] + lines[stop:e]), (1, lines[:stop] + block + lines[stop:e])):
            end = lines[e].replace(str(n), str(n + delta), 1)
            rep = w.analyse("\n".join(text + [end] + lines[e + 1:]), cpp().il)
            self.assertFalse(rep.ok)
            self.assertFalse(any("`end walk:` says" in m for m in rep.mismatches), rep.mismatches)

    def test_a_routine_in_the_wrong_place_is_not_a_mismatch_but_the_wrong_position_is(self):
        lines = list(cpp().lines)
        start = next(i for i, l in enumerate(lines) if l.startswith("routine decl"))
        lines[start] = lines[start].replace("L5:5", "L5:6")
        self.assertNotEqual(lines, cpp().lines)
        self.assertFalse(w.analyse("\n".join(lines), cpp().il).ok)

    def test_the_end_walk_line(self):
        fx = cpp()
        lines = fx.lines
        e = next(i for i, l in enumerate(lines) if l.startswith("end walk"))
        without = w.analyse("\n".join(lines[:e] + lines[e + 1:]), fx.il)
        self.assertFalse(without.ok)
        self.assertTrue(any("no `end walk:` line" in m for m in without.mismatches),
                        without.mismatches)
        n = int(re.search(r"\d+", lines[e]).group())
        wrong = w.analyse("\n".join(lines[:e] + [lines[e].replace(str(n), str(n + 1), 1)]
                                    + lines[e + 1:]), fx.il)
        self.assertFalse(wrong.ok)
        self.assertTrue(any("`end walk:` says" in m for m in wrong.mismatches), wrong.mismatches)
        garbled = w.analyse("\n".join(lines[:e] + [lines[e] + " extra"] + lines[e + 1:]), fx.il)
        self.assertFalse(garbled.ok)

    def test_an_empty_golden_fails(self):
        for text in ("", "\n", END.format(n=0, u=0) + "\n"):
            with self.subTest(text):
                self.assertFalse(w.analyse(text, cpp().il).ok)

    @unittest.expectedFailure
    def test_known_gap_a_duplicated_arg_line_of_a_statement_attribute(self):
        fx = cpp()
        i = next(i for i, l in enumerate(fx.lines) if l.strip() == "arg 1 @ L9:22")
        text = "\n".join(fx.lines[:i + 1] + [fx.lines[i]] + fx.lines[i + 1:])
        self.assertFalse(w.analyse(text, fx.il).ok)


USER_HEADER = ("inline int from_header(int a) { return a + 1; }\n"
               "inline int other_header(int a) { return a + 2; }\n"
               "int declared_only(int a);\n")
USER_HEADER_SOURCE = '#include "oracle_uh.h"\nint use(int a) { return from_header(a); }\n'


@e2e_gate(_front_end_available(), "build/cerastes-fe missing; run scripts/bootstrap.sh")
class EndWalkSecondCountTests(unittest.TestCase):
    """The second count of the `end walk:` line, the routines with bodies in user
    headers, is checked against il_display's source-file entries."""

    def fx(self):
        return fixture("with_header.cpp", USER_HEADER_SOURCE, "c++20",
                       files={"oracle_uh.h": USER_HEADER})

    def test_the_fixture_has_two_header_bodies(self):
        fx = self.fx()
        self.assertIn(" 2 routines with bodies in non-system files outside the primary file",
                      fx.golden)
        rep = w.analyse(fx.golden, fx.il)
        self.assertEqual(rep.mismatches, [])
        self.assertEqual(rep.checked_by["end-walk-user-header-bodies"], 1)

    def test_a_wrong_count_is_a_mismatch(self):
        fx = self.fx()
        for wrong in (0, 1, 3):
            with self.subTest(wrong):
                text = fx.golden.replace(" 2 routines with", f" {wrong} routines with")
                rep = w.analyse(text, fx.il)
                self.assertFalse(rep.ok)
                self.assertTrue(any("il_display shows 2" in m for m in rep.mismatches),
                                rep.mismatches)

    def test_the_count_is_zero_without_header_bodies(self):
        fx = cpp()
        self.assertIn(" 0 routines with", fx.golden)
        wrong = fx.golden.replace(" 0 routines with", " 1 routines with")
        self.assertFalse(w.analyse(wrong, fx.il).ok)

    def test_the_count_comes_from_il_display_not_from_the_listing(self):
        # a header whose bodies il_display hides from the count (a different text): the
        # same golden against the il of a source without the header fails
        fx, plain = self.fx(), cpp()
        self.assertFalse(w.analyse(fx.golden, plain.il).ok)
        self.assertFalse(w.analyse(plain.golden, fx.il).ok)


@e2e_gate(_front_end_available(), "build/cerastes-fe missing; run scripts/bootstrap.sh")
class RoutineKindTests(unittest.TestCase):
    """The kind of a routine, R2-2: an explicit specialization written inside its class template is a
    prototype instantiation and specialized at once, and is listed (and expected) as a specialization."""

    IN_CLASS = """\
template <class T> struct Outer {
  template <class U> static int f(U y) { return 0; }
  template <> int f<int>(int y) { return 1; }
};
"""

    def fx(self):
        return fixture("in_class.cpp", self.IN_CLASS, "c++20")

    def test_the_walker_lists_the_in_class_specialization_as_one(self):
        fx = self.fx()
        self.assertIn("routine Outer<T>::f prototype body @ L2:33", fx.golden)
        self.assertIn("routine Outer<T>::f specialization body @ L3:19", fx.golden)
        rep = w.analyse(fx.golden, fx.il)
        self.assertEqual(rep.mismatches, [])

    def test_the_oracle_expects_it_from_the_flags(self):
        # twin: the listing that still calls it a prototype (the walker's old rule) is a mismatch
        fx = self.fx()
        wrong = fx.golden.replace("routine Outer<T>::f specialization body @ L3:19",
                                  "routine Outer<T>::f prototype body @ L3:19")
        self.assertNotEqual(wrong, fx.golden)
        rep = w.analyse(wrong, fx.il)
        self.assertFalse(rep.ok)
        self.assertTrue(any("L3:19" in m for m in rep.mismatches), rep.mismatches[:3])

    def test_the_in_class_flag_decides_not_the_order_of_tests(self):
        # the real in-class specialization has is_in_class_specialization: the oracle says specialization from
        # that flag.  Without the flag (doctored) the same routine is an unexplained prototype that is also
        # specialized, which is reported, not silently classified.
        fx = self.fx()
        real_flag = w.Rec.flag

        def without_in_class(self, name):
            if self.kind == "routine" and name == "is_in_class_specialization":
                return False
            return real_flag(self, name)
        with mock.patch.object(w.Rec, "flag", without_in_class):
            rep = w.analyse(fx.golden, fx.il)
        self.assertFalse(rep.ok)
        self.assertTrue(any("without is_in_class_specialization" in m for m in rep.mismatches), rep.mismatches[:3])
        self.assertTrue(w.analyse(fx.golden, fx.il).ok)

    def test_both_flags_without_the_in_class_flag_are_reported(self):
        fx = cpp()
        real_flag = w.Rec.flag

        def both(self, name):
            if self.kind == "routine" and name in ("is_prototype_instantiation", "is_specialized"):
                return True
            return real_flag(self, name)
        with mock.patch.object(w.Rec, "flag", both):
            rep = w.analyse(fx.golden, fx.il)
        self.assertFalse(rep.ok)
        self.assertTrue(any("without is_in_class_specialization" in m for m in rep.mismatches), rep.mismatches[:3])
        # without the doctored flags the same input is clean
        self.assertTrue(w.analyse(fx.golden, fx.il).ok)


@e2e_gate(_front_end_available(), "build/cerastes-fe missing; run scripts/bootstrap.sh")
class IlDisplayMutationTests(unittest.TestCase):
    """Every discrepancy injected into the text of il_display is reported."""

    def test_cpp(self):
        for what, mutate in IL_MUTATIONS_CPP.items():
            with self.subTest(what):
                mutated = mutate(cpp().il)
                self.assertNotEqual(mutated, cpp().il, "the mutation did not apply")
                rep = w.analyse(cpp().golden, mutated)
                self.assertFalse(rep.ok, what)
                self.assertTrue(rep.mismatches, what)

    def test_c(self):
        for what, mutate in IL_MUTATIONS_C.items():
            with self.subTest(what):
                mutated = mutate(c().il)
                self.assertNotEqual(mutated, c().il, "the mutation did not apply")
                self.assertFalse(w.analyse(c().golden, mutated).ok, what)

    def test_a_golden_with_nothing_to_check_fails(self):
        fx = fixture("empty.cpp", "int g;\n", "c++20")
        self.assertEqual(fx.golden.strip(), END.format(n=0, u=0))
        rep = w.analyse(fx.golden, fx.il)
        self.assertEqual(rep.mismatches, [])
        self.assertEqual(rep.checked, 0)
        self.assertFalse(rep.ok)

    def test_il_display_of_another_program_fails(self):
        other = fixture("c.cpp", "int other(int q) { return q; }\n", "c++20")
        self.assertTrue(w.analyse(other.golden, other.il).ok)
        self.assertFalse(w.analyse(cpp().golden, other.il).ok)
        self.assertFalse(w.analyse(other.golden, cpp().il).ok)


EMPTY_ATTRIBUTE_SOURCE = """\
int e(int x)
{
  int i = 0;
  [[]] while (i < x) { i++; }
  return i;
}
"""


@e2e_gate(_front_end_available(), "build/cerastes-fe missing; run scripts/bootstrap.sh")
class EmptyAttributeTests(unittest.TestCase):
    """An attributed statement that the listing omits is accepted only for an empty
    `[[]]`, and only when the source is at hand (analyse(.., source_text))."""

    def fx(self):
        return fixture("empty_attr.cpp", EMPTY_ATTRIBUTE_SOURCE, "c++20")

    def test_without_the_source_it_is_a_mismatch(self):
        fx = self.fx()
        self.assertNotIn("stmt", fx.golden)  # the listing says nothing of it
        rep = w.analyse(fx.golden, fx.il)
        self.assertFalse(rep.ok)
        self.assertTrue(any("pass the source text to analyse()" in m for m in rep.mismatches),
                        rep.mismatches)

    def test_with_the_source_it_is_a_fact(self):
        fx = self.fx()
        rep = w.analyse(fx.golden, fx.il, EMPTY_ATTRIBUTE_SOURCE)
        self.assertEqual(rep.mismatches, [])
        self.assertTrue(rep.ok)
        self.assertEqual(rep.checked_by["statement-empty-attribute"], 1)

    def test_the_source_must_show_the_empty_attribute_on_that_line(self):
        fx = self.fx()
        lines = EMPTY_ATTRIBUTE_SOURCE.split("\n")
        for what, text in (
                ("no brackets", EMPTY_ATTRIBUTE_SOURCE.replace("[[]] ", "")),
                ("a real attribute", EMPTY_ATTRIBUTE_SOURCE.replace("[[]]", "[[unreg_ns::a]]")),
                ("on another line", "\n".join(lines[:3] + [lines[3].replace("[[]] ", "")]
                                              + ["  [[]] return i;"] + lines[5:])),
                ("a shorter source", "int e(int x)\n{\n}\n"),
                ("an empty source", ""),
                ("brackets in a comment elsewhere", EMPTY_ATTRIBUTE_SOURCE.replace(
                    "[[]] while", "while").replace("int i = 0;", "int i = 0; // [[]]"))):
            with self.subTest(what):
                rep = w.analyse(fx.golden, fx.il, text)
                self.assertFalse(rep.ok, what)

    def test_spaces_inside_the_brackets_are_still_empty(self):
        src = EMPTY_ATTRIBUTE_SOURCE.replace("[[]]", "[[ ]]")
        fx = fixture("empty_attr2.cpp", src, "c++20")
        self.assertTrue(w.analyse(fx.golden, fx.il, src).ok)

    def test_the_command_line_passes_the_source(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "t.cpp"
            path.write_text("// expect: golden\n" + EMPTY_ATTRIBUTE_SOURCE)
            run = subprocess.run([str(FE_SCRIPT), "--c++20", "--cerastes_walk", str(path)],
                                 capture_output=True, text=True, cwd=REAL_ROOT)
            (Path(tmp) / "t.cpp.golden").write_text(run.stdout)
            out = subprocess.run([sys.executable, str(ORACLE), tmp], capture_output=True,
                                 text=True, cwd=REAL_ROOT)
            self.assertEqual(out.returncode, 0, out.stdout + out.stderr)
            # the same golden through analyse() without the source fails: the CLI does the difference
            il = w.run_il_display(path, "c++20")
            self.assertFalse(w.analyse(run.stdout, il).ok)


@e2e_gate(_front_end_available(), "build/cerastes-fe missing; run scripts/bootstrap.sh")
class CommandLineTests(unittest.TestCase):
    """walker_oracle.py as a program: exit 0 clean, 1 on a failure, 2 on a usage error."""

    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.dir = Path(tmp.name).resolve()

    def make(self, name="t.cpp", source="int f(int a) { return a + 1; }\n", mode="c++20",
                  mutate=None, golden=True, expect="golden"):
        """Write a golden test (its golden made by the walker, then `mutate`d)."""
        path = self.dir / name
        path.write_text(f"// expect: {expect}\n" + source)
        if golden:
            run = subprocess.run([str(FE_SCRIPT), w.MODE_OPTIONS[mode], "--cerastes_walk",
                                  str(path)], capture_output=True, text=True, cwd=REAL_ROOT)
            text = run.stdout if mutate is None else mutate(run.stdout)
            (self.dir / (name + ".golden")).write_text(text)
        return path

    def run_oracle(self, *args):
        run = subprocess.run([sys.executable, str(ORACLE), *map(str, args)], capture_output=True,
                             text=True, cwd=REAL_ROOT)
        return run.returncode, run.stdout, run.stderr

    def test_a_clean_golden_is_zero(self):
        self.make()
        status, out, err = self.run_oracle(self.dir)
        self.assertEqual((status, err), (0, ""), out)
        self.assertIn("1 goldens", out)
        self.assertIn("0 failed", out)

    def test_a_changed_golden_is_one(self):
        self.make(mutate=lambda t: t.replace("param 1 a: int", "param 1 a: long"))
        status, out, _ = self.run_oracle(self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn("FAIL", out)
        self.assertIn("1 failed", out)

    def test_a_golden_with_nothing_to_check_is_one(self):
        self.make(source="int g;\n")
        status, out, _ = self.run_oracle(self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn("no checkable fact", out)

    def test_a_missing_end_line_is_one(self):
        self.make(mutate=lambda t: "\n".join(
            l for l in t.split("\n") if not l.startswith("end walk")))
        status, out, _ = self.run_oracle(self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn("no `end walk:` line", out)

    def test_a_front_end_failure_is_one(self):
        self.make(source="int f( {\n", golden=False)
        (self.dir / "t.cpp.golden").write_text("x\n")
        status, out, _ = self.run_oracle(self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn("front end failed", out)

    def test_one_bad_golden_among_good_ones_is_one(self):
        self.make("a.cpp")
        self.make("b.cpp", source="int g(int b) { return b * 2; }\n",
                       mutate=lambda t: t.replace("returns int", "returns long"))
        status, out, _ = self.run_oracle(self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn("2 goldens", out)
        self.assertIn("1 failed", out)

    def test_a_missing_golden_file_is_two(self):
        self.make(golden=False)
        status, _, err = self.run_oracle(self.dir)
        self.assertEqual(status, 2)
        self.assertIn("is missing", err)

    def test_a_path_that_does_not_exist_is_two(self):
        status, _, err = self.run_oracle(self.dir / "nowhere")
        self.assertEqual(status, 2)
        self.assertIn("no such file or directory", err)

    def test_a_directory_without_golden_tests_is_two(self):
        self.make(expect="frontend-ok", golden=False)
        status, _, err = self.run_oracle(self.dir)
        self.assertEqual(status, 2)
        self.assertIn("no golden tests found", err)

    def test_a_c_golden_runs_in_c23(self):
        self.make("t.c", source="int f(int a) { return a + 1; }\n", mode="c23")
        status, out, err = self.run_oracle(self.dir / "t.c.golden")
        self.assertEqual((status, err), (0, ""), out)

    def test_the_real_tree_passes(self):
        # the acceptance command: every golden of tests/ and the doc examples
        status, out, err = self.run_oracle()
        self.assertEqual(status, 0, out + err)
        self.assertRegex(out, r"walker_oracle: \d+ goldens, \d+ facts checked, 0 failed")


@e2e_gate(_front_end_available(), "build/cerastes-fe missing; run scripts/bootstrap.sh")
class InstanceClosureTests(unittest.TestCase):
    """The closure class of a forall lambda in an INSTANCE of a function template (the copy made when the
    front end rescans the lambda; EDG bug 0020, fork series L) has no `lambda` record in il_display.  The
    oracle knows it is a closure from EDG's flag on its class supplement, corroborated by an `operator()`
    with is_lambda_body in the class's scope.  The rule is narrow: each half alone is not enough."""

    SOURCE = """\
#include <cerastes.h>
template <class T>
bool all_pos([[maybe_unused]] const T* d, [[maybe_unused]] int n)
  CERASTES_REQUIRES(cerastes::forall([](int i, const T* a, int m) { return !(0 <= i && i < m) || a[i] > 0; }, d, n))
{
  return true;
}
class { public: int v; } g1, g2;
void copy() { g1 = g2; }
bool use(const int* p) { return all_pos(p, 1); }
"""

    def fx(self):
        return fixture("instance_closure.cpp", self.SOURCE, "c++20")

    def test_the_instance_closure_is_listed_and_is_checked(self):
        fx = self.fx()
        for text in ("routine <lambda@L4:", "::_FUN plain implicit nobody", "template all_pos args <T=int>",
                     "routine <unnamed>::operator= plain implicit nobody @ L8:7"):
            self.assertIn(text, fx.golden)
        rep = w.analyse(fx.golden, fx.il)
        self.assertEqual(rep.mismatches, [])
        self.assertTrue(rep.ok)

    def test_the_fixture_has_a_closure_class_that_no_lambda_record_names(self):
        # without this the other tests would pass for the wrong reason
        ex = w.Extractor(fx_il := w.IL(self.fx().il))
        named = {c.id for c in (fx_il.ref(lam.get("closure_class")) for lam in fx_il.by_kind["lambda"]) if c}
        self.assertTrue(named)
        self.assertTrue(ex.closure_ids - named, "no unnamed closure class: the rule is not exercised")

    def test_without_the_flag_the_instance_closure_is_a_mismatch(self):
        # twin: remove the rule's first half and the listing's `<lambda@..>::...` routines are not found
        fx = self.fx()
        real_flag = w.Rec.flag

        def no_closure_flag(self, name):
            return False if name == "is_lambda_closure_class" else real_flag(self, name)
        with mock.patch.object(w.Rec, "flag", no_closure_flag):
            rep = w.analyse(fx.golden, fx.il)
        self.assertFalse(rep.ok)
        self.assertTrue(any("not found in il_display" in m and "<lambda@" in m for m in rep.mismatches),
                        rep.mismatches[:3])

    def test_without_the_lambda_body_the_flag_alone_is_not_enough(self):
        # twin: the second half of the rule
        fx = self.fx()
        real_flag = w.Rec.flag

        def no_body(self, name):
            return False if (self.kind == "routine" and name == "is_lambda_body") else real_flag(self, name)
        with mock.patch.object(w.Rec, "flag", no_body):
            rep = w.analyse(fx.golden, fx.il)
        self.assertFalse(rep.ok)

    def test_the_rule_does_not_widen_to_other_classes(self):
        # the flag claimed for every class (doctored): an unnamed class without a lambda body in its scope
        # stays `<unnamed>`, so its `operator=` still matches.  Widening the rule (dropping the scope test)
        # would name it `<lambda@..>` and fail here.  (Only unnamed classes can be misnamed: a named one is
        # printed by its name whatever the closure set says.)
        fx = self.fx()
        real_flag = w.Rec.flag

        def every_class_is_a_closure(self, name):
            if self.kind == "class-type-supplement" and name == "is_lambda_closure_class":
                return True
            return real_flag(self, name)
        with mock.patch.object(w.Rec, "flag", every_class_is_a_closure):
            rep = w.analyse(fx.golden, fx.il)
        self.assertEqual(rep.mismatches, [])
        self.assertTrue(rep.ok)

    def test_a_listing_that_names_the_wrong_closure_is_still_a_mismatch(self):
        fx = self.fx()
        wrong = fx.golden.replace("routine <lambda@L4:", "routine <lambda@L5:", 1)
        self.assertNotEqual(wrong, fx.golden)
        self.assertFalse(w.analyse(wrong, fx.il).ok)


if __name__ == "__main__":
    unittest.main()
