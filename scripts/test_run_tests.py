#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Self-test for scripts/run_tests.py: the harness must fail tests that fail
for the wrong reason or at the wrong line, not only tests with the wrong
verdict; must fail golden tests on any difference in the IL walker's output;
and must never count a skipped directory as passed.

Each test here is a twin for the harness itself: it fails if the check it
covers is weakened.

The Silicon end-to-end cases run Silicon on mutated temporary copies of
tests/viper-smoke/fail.vpr; they are skipped when build/env.sh is missing.
The documentation-example cases cover default discovery (tests/ and
doc/modules/ROOT/examples/) and scripts/check_doc_examples.py on fixture trees;
one end-to-end class builds a temporary copy of doc/ with Antora to prove that
a warning fails scripts/build_docs.sh (skipped without doc/node_modules).
The golden-test cases run the real harness against a fake front end (a small
Python script standing in for scripts/cerastes-fe.sh), so they always run;
a few more run the real cerastes front end when CERASTES_FE is available.
Those end-to-end classes are skipped when their prerequisites are missing,
unless CERASTES_REQUIRE_E2E=1 (scripts/check.sh sets it), in which case they
fail: a missing front end must not silently turn them off.
"""

import contextlib
import io
import json
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
import check_doc_examples as cde  # noqa: E402
import run_tests as rt  # noqa: E402

REAL_ROOT = rt.ROOT  # before any test patches it


def e2e_required(environ=None):
    """Whether CERASTES_REQUIRE_E2E=1: end-to-end tests must run, not skip."""
    return (os.environ if environ is None else environ).get("CERASTES_REQUIRE_E2E") == "1"


def e2e_gate(available, reason, require=None):
    """Class decorator for end-to-end tests that need the built toolchain.

    Without the toolchain the class is skipped, unless CERASTES_REQUIRE_E2E=1
    (set by scripts/check.sh), in which case it fails: a missing front end must
    not silently switch these tests off.  `require` overrides the variable.
    """
    if available:
        return lambda cls: cls
    if e2e_required() if require is None else require:
        def fail(cls):
            raise AssertionError(
                f"CERASTES_REQUIRE_E2E=1 but the end-to-end prerequisites are missing: "
                f"{reason}")

        def decorate(cls):
            cls.setUpClass = classmethod(fail)
            return cls
        return decorate
    return unittest.skip(reason)

SUCCESS = """\
Silicon 1.1-SNAPSHOT (6b540aa4@(detached))
Silicon finished verification successfully in 1.17s.
"""

TWO_ERRORS = """\
Silicon 1.1-SNAPSHOT (6b540aa4@(detached))
Silicon found 2 errors in 1.24s:
  [0] Postcondition of inc might not hold. Assertion r > x might not hold. (t.vpr@5.11--5.16)

  [1] Assignment might fail. There might be insufficient permission to access r.f (t.vpr@9.3--9.11)

"""


def expected(category, line, reason=None):
    return rt.ExpectedError(category, reason, line)


class HeaderTests(unittest.TestCase):
    def test_verified(self):
        exp = rt.parse_header("// expect: verified\nmethod m() {}\n", ".vpr")
        self.assertEqual(exp.kind, "verified")

    def test_errors_and_reason(self):
        exp = rt.parse_header(
            "// expect: error postcondition @ L5\n"
            "// expect: error assignment:insufficient.permission @ L9\n"
            "// a description\n\nmethod m() {}\n", ".vpr")
        self.assertEqual(exp.errors, [expected("postcondition", 5),
                                      expected("assignment", 9, "insufficient.permission")])

    def test_modes(self):
        exp = rt.parse_header("// expect: frontend-ok\n// mode: c++20, c23\nint x;\n", ".cpp")
        self.assertEqual(exp.modes, ["c++20", "c23"])
        self.assertEqual(rt.parse_header("// expect: frontend-ok\n", ".c").modes, ["c23"])

    def test_rejects_bad_headers(self):
        bad = {
            "no expect line": ("// just a comment\nmethod m() {}\n", ".vpr"),
            "expect after code": ("// expect: verified\nmethod m() {}\n// expect: verified\n", ".vpr"),
            "mixed kinds": ("// expect: verified\n// expect: error assert @ L3\n", ".vpr"),
            "unknown category": ("// expect: error nonsense @ L3\n", ".vpr"),
            "unknown reason": ("// expect: error assert:nonsense @ L3\n", ".vpr"),
            "no line": ("// expect: error assert\n", ".vpr"),
            "line zero": ("// expect: error assert @ L0\n", ".vpr"),
            "mode on vpr": ("// expect: verified\n// mode: c23\n", ".vpr"),
            "rejected on vpr": ("// expect: rejected unsupported-union\n", ".vpr"),
            "unknown mode": ("// expect: frontend-ok\n// mode: c++98\n", ".cpp"),
            "duplicate verified": ("// expect: verified\n// expect: verified\n", ".vpr"),
            "golden on vpr": ("// expect: golden\n", ".vpr"),
            "golden twice": ("// expect: golden\n// expect: golden\n", ".cpp"),
            "golden and verified": ("// expect: golden\n// expect: verified\n", ".cpp"),
            "golden and error": ("// expect: golden\n// expect: error assert @ L3\n", ".cpp"),
            "golden and frontend-ok": ("// expect: frontend-ok\n// expect: golden\n", ".c"),
            "golden and rejected": ("// expect: golden\n// expect: rejected unsupported-union\n",
                                    ".cpp"),
            "golden after code": ("// expect: verified\nint x;\n// expect: golden\n", ".cpp"),
            "golden with unknown mode": ("// expect: golden\n// mode: c++17\n", ".cpp"),
        }
        for what, (text, suffix) in bad.items():
            with self.subTest(what), self.assertRaises(rt.HeaderError):
                rt.parse_header(text, suffix)


class SiliconOutputTests(unittest.TestCase):
    def test_success(self):
        result = rt.parse_silicon_output(SUCCESS, 0)
        self.assertTrue(result.verified)
        self.assertIsNone(result.problem)

    def test_errors(self):
        result = rt.parse_silicon_output(TWO_ERRORS, 1)
        self.assertFalse(result.verified)
        self.assertIsNone(result.problem)
        self.assertEqual([(e.category, e.reason, e.line, e.file) for e in result.errors],
                         [("postcondition", "assertion.false", 5, "t.vpr"),
                          ("assignment", "insufficient.permission", 9, "t.vpr")])

    def test_padded_indices(self):
        out = "Silicon found 1 error in 1s:\n  [ 0] Assert might fail. Assertion x > 0 might not hold. (t.vpr@3.10--3.15)\n"
        self.assertEqual(rt.parse_silicon_output(out, 1).errors[0].category, "assert")

    def test_parse_error_is_unknown(self):
        out = ("Silicon found 1 error in 0.55s:\n"
               "  [0] Parse error: Expected (parens | atom):4:1, found \"}\\n\". (t.vpr@4.1--4.1)\n")
        (err,) = rt.parse_silicon_output(out, 1).errors
        self.assertEqual(err.category, rt.UNKNOWN)
        self.assertTrue(rt.compare_errors([], [err], "t.vpr"))

    def test_timeout_is_never_accepted(self):
        out = ("Silicon found 1 error in 9s:\n  [0] Assert might fail. Assertion x > 0 might "
               "not hold. An SMT query timed out after 100ms. (t.vpr@3.10--3.15)\n")
        (err,) = rt.parse_silicon_output(out, 1).errors
        self.assertEqual(err.reason, rt.TIMEOUT)
        problems = rt.compare_errors([expected("assert", 3)], [err], "t.vpr")
        self.assertTrue(any("timeout" in p for p in problems), problems)

    def test_crash_is_a_problem(self):
        result = rt.parse_silicon_output("Exception in thread \"main\" java.lang.Error\n", 1)
        self.assertFalse(result.verified)
        self.assertIsNotNone(result.problem)

    def test_count_mismatch_is_a_problem(self):
        out = TWO_ERRORS.replace("found 2 errors", "found 3 errors")
        self.assertIsNotNone(rt.parse_silicon_output(out, 1).problem)


class CompareTests(unittest.TestCase):
    def setUp(self):
        self.reported = rt.parse_silicon_output(TWO_ERRORS, 1).errors

    def check(self, exp, needle):
        problems = rt.compare_errors(exp, self.reported, "t.vpr")
        self.assertTrue(any(needle in p for p in problems), problems)

    def test_exact_match(self):
        self.assertEqual(rt.compare_errors(
            [expected("postcondition", 5), expected("assignment", 9, "insufficient.permission")],
            self.reported, "t.vpr"), [])

    def test_wrong_line(self):
        self.check([expected("postcondition", 6), expected("assignment", 9)], "wrong line")

    def test_wrong_category(self):
        self.check([expected("assert", 5), expected("assignment", 9)], "wrong reason")

    def test_wrong_viper_reason(self):
        self.check([expected("postcondition", 5),
                    expected("assignment", 9, "division.by.zero")], "wrong reason")

    def test_missing(self):
        self.check([expected("postcondition", 5), expected("assignment", 9),
                    expected("assert", 20)], "missing error")

    def test_unexpected(self):
        self.check([expected("postcondition", 5)], "unexpected error")

    def test_other_file(self):
        problems = rt.compare_errors([], self.reported, "other.vpr")
        self.assertTrue(all("another file" in p for p in problems), problems)


# ---------------------------------------------------------------------------
# Golden tests
# ---------------------------------------------------------------------------

# Walker output in the shape of the M1 output contract (plan, Phase 2).
WALK = """\
routine inc<T> prototype @ L5:3
  param v: T
  op L6: eok_add type_kind=tk_template_param
routine inc<int> instance @ L5:3
  template inc args T=int
end walk: 2 routines reported, 0 routines with bodies in non-system files outside the primary file
"""

FAKE_END = ("end walk: 1 routines reported, 0 routines with bodies in non-system files "
            "outside the primary file\n")
END_WALK = ("end walk: 2 routines reported, 0 routines with bodies in non-system files "
            "outside the primary file\n")


class GoldenHeaderTests(unittest.TestCase):
    def test_golden(self):
        exp = rt.parse_header("// expect: golden\n// G1 probe\nint x;\n", ".cpp")
        self.assertEqual((exp.kind, exp.modes), ("golden", ["c++20"]))
        self.assertEqual(rt.parse_header("// expect: golden\n", ".c").modes, ["c23"])

    def test_golden_modes(self):
        exp = rt.parse_header("// expect: golden\n// mode: c++20, c23\n", ".c")
        self.assertEqual((exp.kind, exp.modes), ("golden", ["c++20", "c23"]))


class GoldenNamingTests(unittest.TestCase):
    def test_one_mode(self):
        t = Path("tests/il/g1/check1_inc.cpp")
        self.assertEqual(rt.golden_path(t, "c++20", ["c++20"]),
                         Path("tests/il/g1/check1_inc.cpp.golden"))

    def test_one_explicit_mode_uses_the_plain_name(self):
        t = Path("x.cpp")
        self.assertEqual(rt.golden_path(t, "c23", ["c23"]), Path("x.cpp.golden"))

    def test_several_modes(self):
        t = Path("d/x.c")
        modes = ["c++20", "c23"]
        self.assertEqual([rt.golden_path(t, m, modes) for m in modes],
                         [Path("d/x.c.c++20.golden"), Path("d/x.c.c23.golden")])

    def test_goldens_are_not_tests(self):
        self.assertNotIn(rt.GOLDEN_SUFFIX, rt.TEST_SUFFIXES)
        for name in ("x.cpp.golden", "x.c.c23.golden", "x.cpp.c++20.golden"):
            self.assertNotIn(Path(name).suffix, rt.TEST_SUFFIXES)


class CompareGoldenTests(unittest.TestCase):
    def differs(self, actual, *needles):
        problems = rt.compare_golden(WALK, actual, "x.cpp.golden")
        self.assertTrue(problems, "a changed output was accepted")
        self.assertTrue(problems[0].startswith("output differs from x.cpp.golden"), problems)
        for needle in needles:
            self.assertIn(needle, problems)
        return problems

    def test_identical(self):
        self.assertEqual(rt.compare_golden(WALK, WALK), [])

    def test_changed_type(self):
        problems = self.differs(WALK.replace("param v: T", "param v: int"),
                                "-  param v: T", "+  param v: int")
        self.assertNotIn("whitespace", problems[0])

    def test_changed_type_kind(self):
        self.differs(WALK.replace("tk_template_param", "tk_unknown"),
                     "-  op L6: eok_add type_kind=tk_template_param",
                     "+  op L6: eok_add type_kind=tk_unknown")

    def test_missing_line(self):
        self.differs(WALK.replace("  template inc args T=int\n", ""),
                     "-  template inc args T=int")

    def test_missing_routine(self):
        self.differs(WALK.split("routine inc<int>")[0], "-routine inc<int> instance @ L5:3")

    def test_extra_line(self):
        self.differs(WALK + "routine dec<T> prototype @ L9:3\n",
                     "+routine dec<T> prototype @ L9:3")

    def test_changed_line_number(self):
        self.differs(WALK.replace("@ L5:3", "@ L6:3", 1),
                     "-routine inc<T> prototype @ L5:3", "+routine inc<T> prototype @ L6:3")

    def test_changed_column(self):
        self.differs(WALK.replace("@ L5:3", "@ L5:4", 1), "+routine inc<T> prototype @ L5:4")

    def test_reordered_lines(self):
        first, second = WALK.split("routine inc<int>")
        self.differs("routine inc<int>" + second + first)

    def test_trailing_space(self):
        problems = self.differs(WALK.replace("param v: T\n", "param v: T \n"),
                                "+  param v: T<SP>")
        self.assertIn("whitespace", problems[0])

    def test_trailing_tab(self):
        self.differs(WALK.replace("param v: T\n", "param v: T\t\n"), "+  param v: T<TAB>")

    def test_crlf_line_ends(self):
        self.differs(WALK.replace("\n", "\r\n"), "+  param v: T<CR>")

    def test_missing_final_newline(self):
        self.differs(WALK[:-1], "\\ No newline at end of file")

    def test_extra_final_newline(self):
        self.differs(WALK + "\n", "+")

    def test_extra_blank_line_inside(self):
        self.differs(WALK.replace("  param v: T\n", "  param v: T\n\n"))

    def test_empty_output(self):
        self.differs("", "-routine inc<T> prototype @ L5:3")

    def test_stderr_noise(self):
        self.differs(rt.golden_text(WALK, "warning: noise\n"),
                     "+" + rt.STDERR_MARKER, "+warning: noise")


class GoldenTextTests(unittest.TestCase):
    def test_stdout_only(self):
        self.assertEqual(rt.golden_text(WALK, ""), WALK)

    def test_stderr_follows_a_marker_line(self):
        self.assertEqual(rt.golden_text(WALK, "w\n"), WALK + "--- stderr ---\nw\n")
        self.assertEqual(rt.golden_text("", "w\n"), "--- stderr ---\nw\n")

    def test_missing_newline_before_stderr_stays_visible(self):
        self.assertNotEqual(rt.golden_text(WALK[:-1], "w\n"), rt.golden_text(WALK, "w\n"))


class EvaluateGoldenTests(unittest.TestCase):
    def test_pass(self):
        self.assertEqual(rt.evaluate_golden(rt.ToolRun(0, WALK, ""), WALK, "g"), [])

    def test_missing_golden(self):
        problems = rt.evaluate_golden(rt.ToolRun(0, WALK, ""), None, "x.cpp.golden")
        self.assertTrue(any("missing golden file x.cpp.golden" in p for p in problems),
                        problems)

    def test_nonzero_exit_fails_even_if_output_matches(self):
        problems = rt.evaluate_golden(rt.ToolRun(2, WALK, ""), WALK, "g")
        self.assertEqual(problems, ["cerastes-fe exited with status 2"])

    def test_crash_fails_even_if_output_matches(self):
        problems = rt.evaluate_golden(rt.ToolRun(-11, WALK, ""), WALK, "g")
        self.assertEqual(problems, ["cerastes-fe crashed (signal 11)"])

    def test_timeout(self):
        problems = rt.evaluate_golden(rt.ToolRun(None, error="timed out after 120s"),
                                      WALK, "g")
        self.assertEqual(problems, ["cerastes-fe timed out after 120s"])

    def test_empty_output_matching_empty_golden_fails(self):
        problems = rt.evaluate_golden(rt.ToolRun(0, "", ""), "", "g")
        self.assertTrue(any("printed nothing" in p for p in problems), problems)

    def test_stderr_noise(self):
        problems = rt.evaluate_golden(rt.ToolRun(0, WALK, "warning: noise\n"), WALK, "g")
        self.assertTrue(problems and problems[0].startswith("output differs"), problems)


class EndWalkLineTests(unittest.TestCase):
    """The `end walk:` line proves the walk ran; without it a run fails even
    when its output equals the golden file."""

    def fails(self, run, expected=None):
        """The problems of `run`, checked against a golden file that matches it
        exactly (so that only the end-line check can fail)."""
        if expected is None:
            expected = rt.golden_text(run.stdout, run.stderr)
        problems = rt.evaluate_golden(run, expected, "g")
        self.assertTrue(any(p.startswith("walker did not finish") for p in problems),
                        problems)
        return problems

    def test_complete_run_passes(self):
        run = rt.ToolRun(0, WALK, "")
        self.assertEqual(rt.evaluate_golden(run, WALK, "g"), [])
        self.assertTrue(rt.walk_finished(WALK))

    def test_no_end_line(self):
        self.fails(rt.ToolRun(0, WALK.replace(END_WALK, ""), ""))

    def test_end_line_not_last(self):
        for after in ("routine late plain @ L9:1\n", "\n", "  \n", "late",
                      "end walk: unfinished"):
            with self.subTest(after=after):
                self.fails(rt.ToolRun(0, WALK + after, ""))

    def test_end_line_only_in_stderr(self):
        self.fails(rt.ToolRun(0, WALK.replace(END_WALK, ""), END_WALK))

    def test_malformed_end_line(self):
        good = END_WALK.rstrip("\n")
        bad = {
            "missing count": good.replace("2 routines reported", "routines reported"),
            "non-numeric": good.replace("2 routines", "x routines"),
            "negative": good.replace("2 routines", "-2 routines"),
            "truncated": good[:-len(" the primary file")],
            "trailing space": good + " ",
            "leading space": " " + good,
            "other prefix": "End walk" + good[len("end walk"):],
            "extra text": good + " ok",
            "second number missing": good.replace("0 routines with", "routines with"),
        }
        for what, line in bad.items():
            with self.subTest(what):
                self.fails(rt.ToolRun(0, WALK.replace(END_WALK, line + "\n"), ""))

    def test_no_final_newline_after_the_end_line(self):
        self.fails(rt.ToolRun(0, WALK[:-1], ""))

    def test_empty_stdout_with_stderr(self):
        problems = self.fails(rt.ToolRun(0, "", "error: something\n"))
        self.assertFalse(any("output differs" in p for p in problems), problems)

    def test_empty_stdout_with_the_end_line_missing_in_a_failed_run(self):
        problems = self.fails(rt.ToolRun(2, "", "error: expected a \"}\"\n"))
        self.assertTrue(any("exited with status 2" in p for p in problems), problems)

    def test_walk_finished_directly(self):
        self.assertTrue(rt.walk_finished(END_WALK))
        self.assertTrue(rt.walk_finished("x\n" + END_WALK))
        self.assertFalse(rt.walk_finished(""))
        self.assertFalse(rt.walk_finished("\n"))
        self.assertFalse(rt.walk_finished(END_WALK.rstrip("\n")))
        self.assertFalse(rt.walk_finished(END_WALK + "\n"))


# A stand-in for scripts/cerastes-fe.sh: prints its options, whether it runs
# in the repository root and the path it was given, then the test file's
# `// out:` lines on stdout and `// err:` lines on stderr, and exits with the
# status of a `// exit:` line.  It ends stdout with the walker's `end walk:`
# line.  Directives: `// silent` suppresses the first three lines and the end
# line (stdout empty unless `// out:` lines exist); `// noend` omits the end
# line; `// end: <text>` prints <text> as the end line; `// after: <text>`
# prints <text> after the end line.
FAKE_FE = '''\
import os, sys
ROOT = {root!r}
args = sys.argv[1:]
with open(args[-1]) as f:  # relative paths are relative to the root
    lines = f.read().splitlines(keepends=True)
code = 0
END = ("end walk: 1 routines reported, 0 routines with bodies in non-system files "
       "outside the primary file\\n")
if "// silent\\n" not in lines:
    print("options", " ".join(args[:-1]))
    print("cwd", "root" if os.path.realpath(os.getcwd()) == os.path.realpath(ROOT)
          else "elsewhere")
    print("path", args[-1])
for line in lines:
    if line.startswith("// out: "):
        sys.stdout.write(line[len("// out: "):])
    elif line.startswith("// err: "):
        sys.stderr.write(line[len("// err: "):])
    elif line.startswith("// exit: "):
        code = int(line[len("// exit: "):])
if "// silent\\n" not in lines and "// noend\\n" not in lines:
    end = [l[len("// end: "):] for l in lines if l.startswith("// end: ")]
    sys.stdout.write(end[0] if end else END)
for line in lines:
    if line.startswith("// after: "):
        sys.stdout.write(line[len("// after: "):])
sys.exit(code)
'''


class FakeFrontEndCase(unittest.TestCase):
    """Runs rt.main() on temporary golden tests with a fake front end."""

    TMP_PARENT = None  # where the temporary directory goes (None: system temp)

    def setUp(self):
        tmp = tempfile.TemporaryDirectory(dir=self.TMP_PARENT)
        self.addCleanup(tmp.cleanup)
        base = Path(tmp.name).resolve()
        fake = base / "fake_fe.py"
        fake.write_text(FAKE_FE.format(root=str(rt.ROOT)))
        self.dir = base / "t"
        self.dir.mkdir()
        # Run from a directory that is not the repository root, so that a
        # harness that forgets cwd=ROOT cannot find the relative test path.
        old_cwd = os.getcwd()
        os.chdir(base)
        self.addCleanup(os.chdir, old_cwd)
        for patcher in (mock.patch.object(rt, "FE_COMMAND", [sys.executable, str(fake)]),
                        mock.patch.dict(os.environ, {"CERASTES_FE": str(fake),
                                                     "EDG_CLANG_VER_SCRAPE": "0"})):
            patcher.start()
            self.addCleanup(patcher.stop)

    def allow(self, *dirs):
        """Permit skip markers in `dirs` (the real allowlist has one entry)."""
        patcher = mock.patch.object(rt, "SKIP_ALLOWED", list(dirs))
        patcher.start()
        self.addCleanup(patcher.stop)

    def write(self, name, *lines, header="// expect: golden"):
        path = self.dir / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("".join(line + "\n" for line in (header, *lines)))
        return path

    def run_harness(self, *args):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            status = rt.main([str(a) for a in args])
        return status, out.getvalue(), err.getvalue()

    def summary(self, out):
        return out.rstrip().splitlines()[-1]


class FakeGoldenRunTests(FakeFrontEndCase):
    def test_missing_golden_fails_and_is_not_created(self):
        t = self.write("x.cpp", "// out: routine f plain @ L5:3")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("missing golden file", out)
        self.assertFalse((self.dir / "x.cpp.golden").exists())

    def test_update_writes_then_a_normal_run_passes(self):
        t = self.write("x.cpp", "// out: routine f plain @ L5:3")
        golden = self.dir / "x.cpp.golden"
        status, out, _ = self.run_harness("--update-golden", t)
        self.assertEqual(status, 0, out)
        self.assertIn(f"UPDATED {rt.rel(golden)}", out)
        self.assertEqual(golden.read_text(),
                         f"options --c++20 --cerastes_walk\ncwd root\npath {rt.rel(t)}\n"
                         "routine f plain @ L5:3\n" + FAKE_END)
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 0, out)
        self.assertIn(f"PASS  {rt.rel(t)} [c++20]", out)
        self.assertEqual(self.summary(out), "1 passed, 0 failed, 0 skipped")

    def test_update_of_an_unchanged_golden(self):
        t = self.write("x.cpp", "// out: a")
        self.run_harness("--update-golden", t)
        status, out, _ = self.run_harness("--update-golden", t)
        self.assertEqual(status, 0, out)
        self.assertIn("UNCHANGED", out)
        self.assertNotIn("UPDATED", out)

    def test_update_shows_the_change(self):
        t = self.write("x.cpp", "// out: routine f plain @ L5:3")
        self.run_harness("--update-golden", t)
        self.write("x.cpp", "// out: routine f plain @ L6:3")
        status, out, _ = self.run_harness("--update-golden", t)
        self.assertEqual(status, 0, out)
        self.assertIn("UPDATED", out)
        self.assertIn("+routine f plain @ L6:3", out)

    def test_changed_output_fails_with_a_diff(self):
        t = self.write("x.cpp", "// out: routine f plain @ L5:3")
        self.run_harness("--update-golden", t)
        self.write("x.cpp", "// out: routine f plain @ L6:3")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn(f"FAIL  {rt.rel(t)} [c++20]", out)
        self.assertIn("-routine f plain @ L5:3", out)
        self.assertIn("+routine f plain @ L6:3", out)

    def test_long_diffs_are_truncated_unless_verbose(self):
        t = self.write("x.cpp", *[f"// out: line {i}" for i in range(200)])
        self.run_harness("--update-golden", t)
        self.write("x.cpp", *[f"// out: other {i}" for i in range(200)])
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("more lines (-v shows them all)", out)
        self.assertNotIn("+other 199", out)
        status, out, _ = self.run_harness("-v", t)
        self.assertIn("+other 199", out)
        self.assertIn("--- output ---", out)

    def test_golden_file_is_read_byte_for_byte(self):
        t = self.write("x.cpp", "// out: a")
        self.run_harness("--update-golden", t)
        golden = self.dir / "x.cpp.golden"
        golden.write_bytes(golden.read_bytes().replace(b"\n", b"\r\n"))
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("<CR>", out)

    def test_nonzero_exit_fails_even_if_output_matches(self):
        t = self.write("x.cpp", "// out: a")
        self.run_harness("--update-golden", t)
        self.write("x.cpp", "// out: a", "// exit: 2")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("exited with status 2", out)
        self.assertNotIn("output differs", out)

    def test_update_refuses_a_failed_run(self):
        t = self.write("x.cpp", "// out: a", "// exit: 1")
        golden = self.dir / "x.cpp.golden"
        status, out, _ = self.run_harness("--update-golden", t)
        self.assertEqual(status, 1, out)
        self.assertIn("not updated", out)
        self.assertFalse(golden.exists())
        golden.write_text("old\n")
        self.run_harness("--update-golden", t)
        self.assertEqual(golden.read_text(), "old\n")

    def test_empty_output_fails_and_is_not_written(self):
        t = self.write("x.cpp", "// silent")
        golden = self.dir / "x.cpp.golden"
        status, out, _ = self.run_harness("--update-golden", t)
        self.assertEqual(status, 1, out)
        self.assertFalse(golden.exists())
        golden.write_text("")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("printed nothing", out)

    def test_stderr_noise_fails(self):
        t = self.write("x.cpp", "// out: a")
        self.run_harness("--update-golden", t)
        self.write("x.cpp", "// out: a", "// err: warning: noise")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("+--- stderr ---", out)
        self.assertIn("+warning: noise", out)

    def test_stderr_is_part_of_the_golden(self):
        t = self.write("x.cpp", "// out: a", "// err: warning #2803-D: x")
        golden = self.dir / "x.cpp.golden"
        self.run_harness("--update-golden", t)
        self.assertTrue(golden.read_text().endswith("a\n" + FAKE_END
                                              + "--- stderr ---\nwarning #2803-D: x\n"))
        self.assertEqual(self.run_harness(t)[0], 0)
        self.write("x.cpp", "// out: a")  # the warning disappears
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("--- stderr ---", out)

    def test_missing_end_line_fails_even_if_the_golden_matches(self):
        t = self.write("x.cpp", "// out: a")
        self.assertEqual(self.run_harness("--update-golden", t)[0], 0)
        golden = self.dir / "x.cpp.golden"
        self.assertTrue(golden.read_text().endswith(FAKE_END))
        golden.write_text(golden.read_text()[:-len(FAKE_END)])  # the golden has no end line
        t = self.write("x.cpp", "// out: a", "// noend")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("walker did not finish", out)
        self.assertNotIn("output differs", out)  # the golden matches; only the end line fails

    def test_end_line_variants_fail_and_are_never_written(self):
        cases = {
            "not last": ["// out: a", "// after: routine late\\n"],
            "malformed": ["// out: a", "// end: end walk: many routines reported\\n"],
            "absent": ["// out: a", "// noend"],
            "stderr only": ["// silent", "// err: error: boom"],
        }
        for what, lines in cases.items():
            with self.subTest(what):
                t = self.write(f"{what.replace(' ', '_')}.cpp", *lines)
                golden = self.dir / (t.name + ".golden")
                status, out, _ = self.run_harness("--update-golden", t)
                self.assertEqual(status, 1, out)
                self.assertIn("not updated: walker did not finish", out)
                self.assertFalse(golden.exists())
                # A golden written by hand does not make the test pass.
                golden.write_text("anything\n")
                status, out, _ = self.run_harness(t)
                self.assertEqual(status, 1, out)
                self.assertIn("walker did not finish", out)
                golden.write_text(golden.read_text() + "x\n")
                self.run_harness("--update-golden", t)
                self.assertEqual(golden.read_text(), "anything\nx\n")

    def test_stdout_empty_and_stderr_non_empty_does_not_pass(self):
        t = self.write("x.cpp", "// silent", "// err: boom")
        golden = self.dir / "x.cpp.golden"
        golden.write_text("--- stderr ---\nboom\n")  # equal to the output
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("walker did not finish", out)

    def test_several_modes_have_one_golden_each(self):
        t = self.write("x.cpp", "// mode: c++20, c23", "// out: a")
        status, out, _ = self.run_harness("--update-golden", t)
        self.assertEqual(status, 0, out)
        cxx = self.dir / "x.cpp.c++20.golden"
        c = self.dir / "x.cpp.c23.golden"
        self.assertIn("options --c++20 --cerastes_walk\n", cxx.read_text())
        self.assertIn("options --c23 --cerastes_walk\n", c.read_text())
        self.assertFalse((self.dir / "x.cpp.golden").exists())
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 0, out)
        self.assertEqual(self.summary(out), "2 passed, 0 failed, 0 skipped")

    def test_each_mode_is_compared_with_its_own_golden(self):
        t = self.write("x.cpp", "// mode: c++20, c23", "// out: a")
        self.run_harness("--update-golden", t)
        cxx = self.dir / "x.cpp.c++20.golden"
        c = self.dir / "x.cpp.c23.golden"
        cxx_text, c_text = cxx.read_text(), c.read_text()
        cxx.write_text(c_text)
        c.write_text(cxx_text)
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertEqual(self.summary(out), "0 passed, 2 failed, 0 skipped")

    def test_one_explicit_mode(self):
        t = self.write("x.cpp", "// mode: c23", "// out: a")
        self.run_harness("--update-golden", t)
        self.assertIn("options --c23 --cerastes_walk\n",
                      (self.dir / "x.cpp.golden").read_text())

    def test_missing_front_end_is_an_environment_error(self):
        t = self.write("x.cpp", "// out: a")
        with mock.patch.object(rt, "load_env", return_value={"EDG_CLANG_VER_SCRAPE": "0"}):
            status, out, err = self.run_harness(t)
        self.assertEqual(status, 2, out)
        self.assertIn("CERASTES_FE not set", err)

    def test_golden_file_named_explicitly_is_not_a_test(self):
        t = self.write("x.cpp", "// out: a")
        self.run_harness("--update-golden", t)
        status, _, err = self.run_harness(self.dir / "x.cpp.golden")
        self.assertEqual(status, 2)
        self.assertIn("not a test file", err)

    def test_goldens_are_not_discovered_as_tests(self):
        t = self.write("x.cpp", "// out: a")
        self.run_harness("--update-golden", t)
        sel = rt.discover([str(self.dir)])
        self.assertEqual(sel.tests, [t])
        self.assertEqual(sel.goldens, [self.dir / "x.cpp.golden"])


@e2e_gate((rt.ROOT / "build").is_dir(), "build/ missing; run scripts/bootstrap.sh")
class FakeGoldenRelativePathTests(FakeFrontEndCase):
    """A test inside the repository is passed by its path relative to the root."""

    TMP_PARENT = rt.ROOT / "build"

    def test_relative_path_from_the_root(self):
        t = self.write("x.cpp", "// out: a")
        self.run_harness("--update-golden", t)
        text = (self.dir / "x.cpp.golden").read_text()
        relative = t.relative_to(rt.ROOT)
        self.assertIn(f"path {relative}\n", text)
        self.assertIn("cwd root\n", text)
        self.assertFalse(Path(str(relative)).is_absolute())


class OrphanGoldenTests(FakeFrontEndCase):
    def setUp(self):
        super().setUp()
        self.test = self.write("x.cpp", "// out: a")
        self.assertEqual(self.run_harness("--update-golden", self.test)[0], 0)

    def test_no_orphans(self):
        status, out, _ = self.run_harness("--check-headers", self.dir)
        self.assertEqual(status, 0, out)
        self.assertIn("1 of 1 headers ok, 0 orphan golden files, 0 skipped", out)

    def test_renamed_test_leaves_an_orphan(self):
        self.test.rename(self.dir / "y.cpp")
        status, out, _ = self.run_harness("--check-headers", self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn(f"{rt.rel(self.dir / 'x.cpp.golden')}: orphan golden file", out)
        self.assertIn("1 of 1 headers ok, 1 orphan golden files", out)

    def test_orphan_fails_a_normal_run(self):
        (self.dir / "gone.cpp.golden").write_text("a\n")
        status, out, _ = self.run_harness(self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn(f"FAIL  {rt.rel(self.dir / 'gone.cpp.golden')}", out)
        self.assertIn("orphan golden file", out)

    def test_single_mode_golden_after_adding_a_mode(self):
        self.write("x.cpp", "// mode: c++20, c23", "// out: a")
        status, out, _ = self.run_harness("--check-headers", self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn(f"{rt.rel(self.dir / 'x.cpp.golden')}: orphan golden file", out)

    def test_mode_golden_for_an_unlisted_mode(self):
        (self.dir / "x.cpp.c23.golden").write_text("a\n")
        status, out, _ = self.run_harness("--check-headers", self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn("x.cpp.c23.golden: orphan golden file", out)

    def test_golden_of_a_test_with_another_expectation(self):
        self.write("x.cpp", header="// expect: frontend-ok")
        status, out, _ = self.run_harness("--check-headers", self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn("x.cpp.golden: orphan golden file", out)

    def test_golden_of_a_vpr_test(self):
        self.write("m.vpr", "method m() {}", header="// expect: verified")
        (self.dir / "m.vpr.golden").write_text("a\n")
        status, out, _ = self.run_harness("--check-headers", self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn("m.vpr.golden: orphan golden file", out)

    def test_golden_of_a_bad_header_is_reported_as_the_bad_header(self):
        self.write("x.cpp", "// out: a", header="// expect: goldne")
        status, out, _ = self.run_harness("--check-headers", self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn("bad header", out)
        self.assertNotIn("orphan golden file:", out)


class SkipMarkerTests(FakeFrontEndCase):
    REASON = "reference copies, no expect headers"

    def setUp(self):
        super().setUp()
        self.run_test = self.write("run/a.cpp", "// out: a")
        self.assertEqual(self.run_harness("--update-golden", self.run_test)[0], 0)
        self.skipped = self.dir / "skipme"
        self.skipped.mkdir()
        self.allow(self.skipped)
        (self.skipped / rt.SKIP_MARKER).write_text(self.REASON + "\n")
        # Files that would fail if they were not skipped.
        (self.skipped / "b.cpp").write_text("int f(int x) { return x; } //@ no header\n")
        (self.skipped / "b.cpp.golden").write_text("an orphan if it were seen\n")
        (self.skipped / "sub").mkdir()
        (self.skipped / "sub" / "c.cpp").write_text("int g();\n")
        self.skip_line = f"SKIP  {rt.rel(self.skipped)}: {self.REASON}"

    def test_skipped_directory_is_reported_and_not_passed(self):
        status, out, _ = self.run_harness(self.dir)
        self.assertEqual(status, 0, out)
        self.assertIn(self.skip_line, out.splitlines())
        self.assertNotIn("b.cpp", out)
        self.assertNotIn("c.cpp", out)
        self.assertEqual(self.summary(out), "1 passed, 0 failed, 1 skipped")

    def test_check_headers_skips_it(self):
        status, out, _ = self.run_harness("--check-headers", self.dir)
        self.assertEqual(status, 0, out)
        self.assertIn(self.skip_line, out.splitlines())
        self.assertNotIn("b.cpp", out)
        self.assertEqual(self.summary(out), "1 of 1 headers ok, 0 orphan golden files, 1 skipped")

    def test_explicit_file_inside_is_skipped(self):
        for args in ([self.skipped / "b.cpp"], ["--check-headers", self.skipped / "b.cpp"],
                     [self.skipped / "sub" / "c.cpp"], [self.skipped / "sub"],
                     [self.skipped]):
            with self.subTest(args=args):
                status, out, err = self.run_harness(*args)
                self.assertEqual(status, 2, out)  # nothing left to run
                self.assertEqual(out.splitlines(), [self.skip_line])
                self.assertIn("no tests found (1 directories skipped)", err)

    def test_skip_is_reported_once(self):
        status, out, _ = self.run_harness(self.dir, self.skipped / "b.cpp", self.skipped)
        self.assertEqual(status, 0, out)
        self.assertEqual(out.count("SKIP  "), 1)
        self.assertEqual(self.summary(out), "1 passed, 0 failed, 1 skipped")

    def test_each_skipped_directory_counts(self):
        other = self.dir / "run" / "also"
        other.mkdir()
        self.allow(self.skipped, other)
        (other / rt.SKIP_MARKER).write_text("second reason\n")
        (other / "d.cpp").write_text("no header\n")
        status, out, _ = self.run_harness(self.dir)
        self.assertEqual(status, 0, out)
        self.assertIn(f"SKIP  {rt.rel(other)}: second reason", out)
        self.assertEqual(self.summary(out), "1 passed, 0 failed, 2 skipped")

    def test_empty_marker_is_an_error(self):
        for content in ("", "  \n\n"):
            for args in ([self.dir], ["--check-headers", self.dir], [self.skipped / "b.cpp"]):
                with self.subTest(content=content, args=args):
                    (self.skipped / rt.SKIP_MARKER).write_text(content)
                    status, out, err = self.run_harness(*args)
                    self.assertEqual(status, 2, out)
                    self.assertIn(".harness-skip is empty", err)
                    self.assertNotIn("PASS", out)

    def test_multiline_marker_is_an_error(self):
        (self.skipped / rt.SKIP_MARKER).write_text("one\ntwo\n")
        status, _, err = self.run_harness(self.dir)
        self.assertEqual(status, 2)
        self.assertIn("single line", err)

    def test_without_the_marker_the_files_run(self):
        (self.skipped / rt.SKIP_MARKER).unlink()
        status, out, _ = self.run_harness("--check-headers", self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn("b.cpp: bad header", out)


class UnrecognizedSuffixTests(FakeFrontEndCase):
    """A source-like file that is not a test suffix would silently never run."""

    BAD = [".cc", ".cxx", ".c++", ".CPP", ".C", ".hpp", ".h", ".hh", ".hxx", ".cp", ".CXX",
           ".Cpp", ".H", ".ipp", ".sil", ".VPR"]

    def setUp(self):
        super().setUp()
        self.good = self.write("good.cpp", "// out: a")
        self.assertEqual(self.run_harness("--update-golden", self.good)[0], 0)

    def test_each_suffix_is_an_error_with_check_headers(self):
        for suffix in self.BAD:
            with self.subTest(suffix=suffix):
                f = self.dir / f"stray{suffix}"
                f.write_text("// expect: golden\nint x;\n")
                status, out, _ = self.run_harness("--check-headers", self.dir)
                self.assertEqual(status, 1, out)
                self.assertIn(f"{rt.rel(f)}: looks like a C/C++/Viper source file", out)
                self.assertIn("1 files with an unrecognized suffix", out)
                self.assertNotIn("good.cpp: ", out)
                f.unlink()

    def test_each_suffix_is_an_error_in_a_normal_run(self):
        for suffix in self.BAD:
            with self.subTest(suffix=suffix):
                f = self.dir / f"stray{suffix}"
                f.write_text("int x;\n")  # not even a header: it is still reported
                status, out, _ = self.run_harness(self.dir)
                self.assertEqual(status, 1, out)
                self.assertIn(f"FAIL  {rt.rel(f)}", out)
                self.assertIn("would never run", out)
                self.assertIn(f"PASS  {rt.rel(self.good)} [c++20]", out)  # the rest still runs
                self.assertEqual(self.summary(out), "1 passed, 1 failed, 0 skipped")
                f.unlink()

    def test_a_directory_with_only_such_files_is_an_error(self):
        (self.dir / "good.cpp").unlink()
        (self.dir / "good.cpp.golden").unlink()
        f = self.dir / "only.hpp"
        f.write_text("// expect: golden\n")
        for args in ([self.dir], ["--check-headers", self.dir]):
            status, out, err = self.run_harness(*args)
            self.assertEqual(status, 1, (out, err))
            self.assertIn("only.hpp", out)

    def test_nested_directories_are_scanned(self):
        f = self.dir / "a" / "b" / "deep.cc"
        f.parent.mkdir(parents=True)
        f.write_text("int x;\n")
        status, out, _ = self.run_harness("--check-headers", self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn(rt.rel(f), out)

    def test_test_suffixes_and_other_files_are_fine(self):
        for name in ("notes.md", "data.txt", "x.py", "run.sh", "Makefile", "x.cpp.golden",
                     "x.cpp.orig", "cpp", ".cpp", "x.cpps", "x.json", "x.hs", "x.o"):
            (self.dir / name).write_text("text\n")
        # A hidden file named like a suffix has no suffix, and `.cpp.golden`
        # is a golden file (here an orphan: the only complaint allowed).
        status, out, _ = self.run_harness("--check-headers", self.dir)
        self.assertEqual(status, 1, out)  # x.cpp.golden is an orphan
        self.assertEqual([l for l in out.splitlines() if "looks like" in l], [])
        self.assertIn("orphan golden file", out)
        (self.dir / "x.cpp.golden").unlink()
        status, out, _ = self.run_harness("--check-headers", self.dir)
        self.assertEqual(status, 0, out)

    def test_the_exact_test_suffixes_are_not_flagged(self):
        for name in ("a.cpp", "b.c", "c.vpr"):
            (self.dir / name).write_text("// expect: " + ("verified" if name.endswith("vpr")
                                                          else "golden") + "\n")
        status, out, _ = self.run_harness("--check-headers", self.dir)
        self.assertNotIn("looks like", out)

    def test_an_allowed_skipped_directory_is_exempt(self):
        skipped = self.dir / "refs"
        skipped.mkdir()
        self.allow(skipped)
        (skipped / rt.SKIP_MARKER).write_text("reference copies\n")
        for name in ("a.hpp", "b.cc", "c.CPP", "d.h"):
            (skipped / name).write_text("int x;\n")  # no expect header
        (skipped / "sub").mkdir()
        (skipped / "sub" / "e.cxx").write_text("int y;\n")
        for args in ([self.dir], ["--check-headers", self.dir]):
            status, out, err = self.run_harness(*args)
            self.assertEqual(status, 0, (out, err))
            self.assertNotIn("looks like", out)
            self.assertIn(f"SKIP  {rt.rel(skipped)}", out)

    def test_an_explicit_file_with_such_a_suffix_is_a_usage_error(self):
        f = self.dir / "stray.cc"
        f.write_text("int x;\n")
        status, out, err = self.run_harness(f)
        self.assertEqual(status, 2, out)
        self.assertIn("not a test file", err)


class HiddenTestTests(FakeFrontEndCase):
    """A test hiding in a skipped directory (a file with an expect header)."""

    def setUp(self):
        super().setUp()
        self.good = self.write("run/good.cpp", "// out: a")
        self.assertEqual(self.run_harness("--update-golden", self.good)[0], 0)
        self.skipped = self.dir / "refs"
        self.skipped.mkdir()
        self.allow(self.skipped)
        (self.skipped / rt.SKIP_MARKER).write_text("reference copies\n")
        (self.skipped / "plain.cpp").write_text("// a comment\nint f();\n//@ spec\n")

    def test_clean_skipped_directory_is_fine(self):
        for args in ([self.dir], ["--check-headers", self.dir]):
            status, out, err = self.run_harness(*args)
            self.assertEqual(status, 0, (out, err))
            self.assertNotIn("hidden", out)

    def test_header_in_the_skipped_directory_is_an_error(self):
        for name, text in (("t.cpp", "// expect: golden\nint x;\n"),
                           ("t.c", "\n// note\n// expect: frontend-ok\n"),
                           ("t.vpr", "// expect: verified\nmethod m() {}\n"),
                           ("t.hpp", "// expect: golden\n"),
                           ("t.cc", "//  expect:   golden\n"),
                           ("sub/deep.cpp", "// expect: golden\n")):
            with self.subTest(name=name):
                f = self.skipped / name
                f.parent.mkdir(exist_ok=True)
                f.write_text(text)
                status, out, _ = self.run_harness("--check-headers", self.dir)
                self.assertEqual(status, 1, out)
                self.assertIn(f"{rt.rel(f)}: has an `// expect:` header but is in a skipped "
                              "directory", out)
                self.assertIn("1 tests hidden in skipped directories", out)
                status, out, _ = self.run_harness(self.dir)
                self.assertEqual(status, 1, out)
                self.assertIn(f"FAIL  {rt.rel(f)}", out)
                self.assertEqual(self.summary(out), "1 passed, 1 failed, 1 skipped")
                self.assertIn("SKIP  ", out)  # still reported as skipped
                f.unlink()

    def test_explicit_paths_inside_the_skipped_directory_report_it_too(self):
        f = self.skipped / "t.cpp"
        f.write_text("// expect: golden\n")
        for args in ([f], ["--check-headers", f], [self.skipped], [self.skipped / "plain.cpp"]):
            with self.subTest(args=args):
                status, out, _ = self.run_harness(*args)
                self.assertEqual(status, 1, out)
                self.assertIn(rt.rel(f), out)

    def test_expect_after_code_or_in_other_files_is_not_a_header(self):
        (self.skipped / "late.cpp").write_text("int x;\n// expect: golden\n")
        (self.skipped / "README.md").write_text("// expect: golden\n")
        (self.skipped / "data.txt").write_text("// expect: golden\n")
        status, out, err = self.run_harness("--check-headers", self.dir)
        self.assertEqual(status, 0, (out, err))

class RealTreeTests(unittest.TestCase):
    def test_no_hidden_tests_and_no_unrecognized_files_in_tests(self):
        sel = rt.discover([str(REAL_ROOT / "tests")])
        self.assertEqual(sel.hidden, [])
        self.assertEqual(sel.unrecognized, [])
        self.assertEqual([rt.rel(d) for d in sel.skipped], ["tests/differential/verifast"])


class SkipAllowlistTests(unittest.TestCase):
    """A skip marker is permitted only in the directories of rt.SKIP_ALLOWED.
    These tests use a temporary repository root with the same layout."""

    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.root = Path(tmp.name).resolve()
        patcher = mock.patch.object(rt, "ROOT", self.root)
        patcher.start()
        self.addCleanup(patcher.stop)
        for d in ("tests/il", "tests/il/g1", "tests/differential/verifast", "tests/other"):
            (self.root / d).mkdir(parents=True)
        (self.root / "tests/il/g1/a.cpp").write_text("// expect: frontend-ok\n")
        (self.root / "tests/other/b.cpp").write_text("// expect: frontend-ok\n")
        (self.root / "tests/differential/verifast/c.cpp").write_text("int f();\n")

    def run_harness(self, *args):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            status = rt.main([str(a) for a in args])
        return status, out.getvalue(), err.getvalue()

    def test_the_real_allowlist_is_one_directory(self):
        self.assertEqual(rt.SKIP_ALLOWED, [Path("tests/differential/verifast")])

    def test_the_real_tree_has_no_other_marker(self):
        real = REAL_ROOT
        allowed = {(real / a).resolve() for a in rt.SKIP_ALLOWED}
        markers = [m.parent.resolve() for m in (real / "tests").rglob(rt.SKIP_MARKER)]
        self.assertTrue(markers, "the allowed marker is missing")
        self.assertEqual(set(markers) - allowed, set())

    def test_allowed_marker_skips(self):
        (self.root / "tests/differential/verifast" / rt.SKIP_MARKER).write_text("reference copies\n")
        skip_line = "SKIP  tests/differential/verifast: reference copies"
        status, out, err = self.run_harness("--check-headers", self.root / "tests")
        self.assertEqual(status, 0, (out, err))
        self.assertIn(skip_line, out.splitlines())
        self.assertIn("2 of 2 headers ok, 0 orphan golden files, 1 skipped", out)
        self.assertNotIn("c.cpp", out)
        # An explicit file inside is skipped too; nothing is left to run.
        status, out, err = self.run_harness(self.root / "tests/differential/verifast/c.cpp")
        self.assertEqual((status, out.splitlines()), (2, [skip_line]), err)

    def assert_rejected(self, marker, *args):
        marker.write_text("looks harmless\n")
        status, out, err = self.run_harness(*args)
        self.assertEqual(status, 2, (out, err))
        self.assertIn(f"{rt.rel(marker)}: a skip marker is not permitted here", err)
        self.assertNotIn("SKIP", out)
        self.assertNotIn("PASS", out)

    def test_marker_in_tests_il_is_rejected(self):
        marker = self.root / "tests/il" / rt.SKIP_MARKER
        for args in (["--check-headers", self.root / "tests"],
                     ["--check-headers", self.root / "tests/il"],
                     [self.root / "tests"],
                     [self.root / "tests/il/g1/a.cpp"],  # an explicit file below the marker
                     ["--check-headers", self.root / "tests/il/g1"]):
            with self.subTest(args=args):
                self.assert_rejected(marker, *args)
        self.assertIn("tests/il/.harness-skip", rt.rel(marker))

    def test_marker_in_tests_other_is_rejected(self):
        self.assert_rejected(self.root / "tests/other" / rt.SKIP_MARKER,
                             "--check-headers", self.root / "tests")

    def test_marker_in_the_root_or_in_tests_is_rejected(self):
        self.assert_rejected(self.root / rt.SKIP_MARKER, "--check-headers", self.root / "tests")
        self.assert_rejected(self.root / "tests" / rt.SKIP_MARKER,
                             "--check-headers", self.root / "tests/other/b.cpp")

    def test_a_rejected_marker_is_rejected_even_if_it_is_empty(self):
        marker = self.root / "tests/il" / rt.SKIP_MARKER
        marker.write_text("")
        status, _, err = self.run_harness("--check-headers", self.root / "tests")
        self.assertEqual(status, 2)
        self.assertIn("not permitted", err)

    def test_a_directory_with_the_allowed_name_elsewhere_is_rejected(self):
        for d in ("tests/other/verifast", "tests/verifast", "verifast",
                  "tests/differential/x/verifast"):
            with self.subTest(d=d):
                (self.root / d).mkdir(parents=True, exist_ok=True)
                marker = self.root / d / rt.SKIP_MARKER
                self.assert_rejected(marker, "--check-headers", self.root)
                marker.unlink()  # so that the next case is the only marker

    def test_the_allowed_marker_does_not_cover_its_siblings(self):
        (self.root / "tests/differential/verifast" / rt.SKIP_MARKER).write_text("reference copies\n")
        (self.root / "tests/differential/other").mkdir()
        self.assert_rejected(self.root / "tests/differential/other" / rt.SKIP_MARKER,
                             "--check-headers", self.root / "tests")

    def test_marker_in_an_arbitrary_directory(self):
        # No patched root: a plain temporary directory outside the repository.
        with tempfile.TemporaryDirectory() as tmp, mock.patch.object(
                rt, "ROOT", REAL_ROOT):
            d = Path(tmp)
            (d / "t.cpp").write_text("// expect: frontend-ok\n")
            (d / rt.SKIP_MARKER).write_text("nothing to see\n")
            status, out, err = self.run_harness(d)
            self.assertEqual(status, 2, (out, err))
            self.assertIn(rt.SKIP_MARKER, err)
            self.assertIn("not permitted", err)
            self.assertNotIn("SKIP", out)


# ---------------------------------------------------------------------------
# Documentation examples: default discovery
# ---------------------------------------------------------------------------

DOC_EXAMPLES = "doc/modules/ROOT/examples"


class DefaultPathsTests(unittest.TestCase):
    def test_the_constant_names_tests_and_the_doc_examples(self):
        self.assertEqual(rt.DEFAULT_PATHS, (Path("tests"), Path(DOC_EXAMPLES)))

    def test_the_default_directories_exist_in_the_real_tree(self):
        for p in rt.DEFAULT_PATHS:
            self.assertTrue((REAL_ROOT / p).is_dir(), p)

    def test_the_default_paths_are_the_ones_used(self):
        # A harness that ignores DEFAULT_PATHS (say, a hard-coded tests/)
        # would keep the constant right and still drop the doc examples.
        with mock.patch.object(rt, "DEFAULT_PATHS", (Path("a"), Path("b/c"))), \
                mock.patch.object(rt, "ROOT", Path("/nonexistent-root")):
            with self.assertRaises(rt.UsageError) as ctx:
                rt.default_paths()
        self.assertIn("default test directory a is missing", str(ctx.exception))

    def test_the_help_text_names_the_doc_examples(self):
        out = io.StringIO()
        with contextlib.redirect_stdout(out), self.assertRaises(SystemExit):
            rt.main(["--help"])
        self.assertIn("doc/modules/ROOT/examples/", " ".join(out.getvalue().split()))

    def test_no_doc_path_may_hold_a_skip_marker(self):
        for p in rt.SKIP_ALLOWED:
            self.assertFalse(str(p).startswith("doc"), p)

    def test_the_real_doc_examples_are_clean(self):
        sel = rt.discover([str(REAL_ROOT / DOC_EXAMPLES)])
        self.assertEqual((sel.hidden, sel.unrecognized, sel.skipped), ([], [], {}))
        self.assertTrue(sel.tests, "the doc examples directory has no tests")


class DefaultDiscoveryTests(FakeFrontEndCase):
    """A run with no paths covers tests/ and the doc examples, in a temporary
    repository root with the same layout."""

    EXAMPLE = f"{DOC_EXAMPLES}/installation/hello.cpp"

    def setUp(self):
        super().setUp()
        self.repo = self.dir.parent / "repo"
        self.repo.mkdir()
        self.dir = self.repo  # where self.write puts files
        patcher = mock.patch.object(rt, "ROOT", self.repo)
        patcher.start()
        self.addCleanup(patcher.stop)
        self.test = self.write("tests/a/t.cpp", "// out: t")
        self.example = self.write(self.EXAMPLE, "// out: d")
        status, out, _ = self.run_harness("--update-golden", self.test, self.example)
        self.assertEqual(status, 0, out)

    def test_no_arguments_run_tests_and_doc_examples(self):
        status, out, err = self.run_harness()
        self.assertEqual(status, 0, (out, err))
        self.assertIn("PASS  tests/a/t.cpp [c++20]", out)
        self.assertIn(f"PASS  {self.EXAMPLE} [c++20]", out)
        self.assertEqual(self.summary(out), "2 passed, 0 failed, 0 skipped")

    def test_a_failing_doc_example_fails_the_default_run(self):
        self.write(self.EXAMPLE, "// out: changed")
        status, out, _ = self.run_harness()
        self.assertEqual(status, 1, out)
        self.assertIn(f"FAIL  {self.EXAMPLE} [c++20]", out)
        self.assertEqual(self.summary(out), "1 passed, 1 failed, 0 skipped")

    def test_check_headers_covers_the_doc_examples(self):
        status, out, _ = self.run_harness("--check-headers")
        self.assertEqual(status, 0, out)
        self.assertIn("2 of 2 headers ok", out)
        self.write(self.EXAMPLE, header="// expect: goldne")
        status, out, _ = self.run_harness("--check-headers")
        self.assertEqual(status, 1, out)
        self.assertIn(f"{self.EXAMPLE}: bad header", out)
        self.assertIn("1 of 2 headers ok", out)

    def test_a_missing_default_directory_is_a_usage_error(self):
        for missing in (DOC_EXAMPLES, "tests"):
            with self.subTest(missing=missing):
                backup = self.repo.parent / "backup"
                shutil.move(str(self.repo / missing), backup)
                try:
                    for args in ([], ["--check-headers"]):
                        status, out, err = self.run_harness(*args)
                        self.assertEqual(status, 2, (out, err))
                        self.assertIn(f"default test directory {missing} is missing", err)
                        self.assertNotIn("PASS", out)
                        self.assertNotIn("headers ok", out)
                finally:
                    shutil.move(str(backup), self.repo / missing)

    def test_explicit_paths_replace_the_defaults(self):
        status, out, _ = self.run_harness(self.test)
        self.assertEqual(status, 0, out)
        self.assertNotIn("hello.cpp", out)
        self.assertEqual(self.summary(out), "1 passed, 0 failed, 0 skipped")
        # ...and a missing default directory does not matter then.
        shutil.rmtree(self.repo / DOC_EXAMPLES)
        self.assertEqual(self.run_harness(self.test)[0], 0)

    def test_a_skip_marker_in_the_doc_examples_is_rejected(self):
        marker = self.repo / DOC_EXAMPLES / rt.SKIP_MARKER
        marker.write_text("hide the examples\n")
        for args in ([], ["--check-headers"], [self.repo / DOC_EXAMPLES]):
            with self.subTest(args=args):
                status, out, err = self.run_harness(*args)
                self.assertEqual(status, 2, (out, err))
                self.assertIn(f"{rt.rel(marker)}: a skip marker is not permitted here", err)
                self.assertNotIn("SKIP", out)
                self.assertNotIn("PASS", out)

    def test_an_unrecognized_suffix_in_the_doc_examples_is_an_error(self):
        stray = self.repo / DOC_EXAMPLES / "installation" / "helper.hpp"
        stray.write_text("int x;\n")
        status, out, _ = self.run_harness()
        self.assertEqual(status, 1, out)
        self.assertIn(f"FAIL  {rt.rel(stray)}", out)
        self.assertIn("would never run", out)
        status, out, _ = self.run_harness("--check-headers")
        self.assertEqual(status, 1, out)
        self.assertIn("1 files with an unrecognized suffix", out)

    def test_an_orphan_golden_in_the_doc_examples_is_an_error(self):
        self.example.rename(self.example.with_name("renamed.cpp"))
        status, out, _ = self.run_harness("--check-headers")
        self.assertEqual(status, 1, out)
        self.assertIn(f"{self.EXAMPLE}.golden: orphan golden file", out)

    def test_a_bad_header_in_a_doc_example_fails_a_normal_run(self):
        self.write(self.EXAMPLE, header="// no expectation here")
        status, out, _ = self.run_harness()
        self.assertEqual(status, 1, out)
        self.assertIn(f"FAIL  {self.EXAMPLE}", out)
        self.assertIn("bad header", out)


# ---------------------------------------------------------------------------
# Documentation examples: scripts/check_doc_examples.py
# ---------------------------------------------------------------------------

def scan(*lines):
    """The rule-a problems of a page given as lines: [(line, message)]."""
    return cde.scan_page("\n".join(lines) + "\n").problems


class BlockLintTests(unittest.TestCase):
    """Rule a: a source block in a language cerastes reads holds only includes."""

    INCLUDE = "include::example$a.cpp[tag=body]"

    def flagged(self, *lines, at="int x;", ignore=()):
        """The page has exactly one problem, on the line holding `at` (problems
        whose message contains one of `ignore` are left out)."""
        problems = [p for p in scan(*lines) if not any(i in p[1] for i in ignore)]
        want = [i for i, l in enumerate(lines, start=1) if l == at]
        self.assertEqual(len(want), 1, "fixture error")
        self.assertEqual([n for n, _ in problems], want, problems)
        self.assertIn("inline", problems[0][1])
        return problems

    def clean(self, *lines):
        self.assertEqual(scan(*lines), [])

    def test_each_attribute_line_form_is_detected(self):
        forms = ["[source,cpp]", "[source, cpp]", "[source,cpp,linenums]", "[,cpp]",
                 "[source,c++]", "[source,CPP]", "[source,c]", "[source,C]", "[source,h]",
                 "[source,hpp]", "[source,cxx]", "[source,vpr]", "[source,viper]",
                 "[source,Viper]", "[source#id.role,cpp]", "[source%linenums,cpp]",
                 "[source,language=cpp]", "[source,lang=cpp]", '[source,"cpp"]',
                 '[source,cpp,subs="+quotes"]', "[.role,cpp]", "[#id,cpp]",
                 "[listing,cpp]", "[source,subs=none,cpp]", "[source,{lang}]",
                 "[source,cpp,indent=0]", "[ source , cpp ]"]
        for form in forms:
            with self.subTest(form=form):
                self.flagged(form, "----", "int x;", "----", ignore=("`subs=`",))

    def test_each_block_delimiter_form_is_detected(self):
        for open_, close in (("----", "----"), ("-----", "-----"), ("........", "........"),
                             ("....", "...."), ("--", "--")):
            with self.subTest(delimiter=open_):
                self.flagged("[source,cpp]", open_, "int x;", close,
                             ignore=("literal blocks",))

    def test_the_delimiter_less_paragraph_form_is_detected(self):
        self.flagged("[source,cpp]", "int x;")
        self.flagged("[source,cpp]", "int x;", "", "Next paragraph.")
        # The paragraph ends at the blank line: the text after it is not code.
        self.clean("[source,cpp]", self.INCLUDE, "", "int x; is prose here.")

    def test_a_fenced_block_is_detected(self):
        self.flagged("```cpp", "int x;", "```")
        self.flagged("```c++", "int x;", "```")
        self.flagged("``` cpp", "int x;", "```")
        self.clean("```sh", "make", "```")
        # A fence must name a language.
        (problem,) = scan("```", "text", "```")
        self.assertEqual(problem[0], 1)
        self.assertIn("fenced block without a declared language", problem[1])

    def test_a_title_anchor_blank_line_or_attribute_between_is_allowed(self):
        self.flagged("[source,cpp]", ".A title", "----", "int x;", "----")
        self.flagged("[source,cpp]", "[[anchor]]", "----", "int x;", "----")
        self.flagged("[source,cpp]", "", "----", "int x;", "----")
        self.flagged("[source,cpp]", "[#id]", "----", "int x;", "----")
        self.flagged(".A title", "[source,cpp]", "----", "int x;", "----")
        self.flagged("[source,cpp]", "// a comment", ".Title", "----", "int x;", "----")
        self.flagged("[source,sh]", "[source,cpp]", "----", "int x;", "----")

    def test_a_default_source_language_is_rejected_and_still_applied(self):
        # The attribute is an error of its own, a block that relies on it has
        # no declared language, and its inline code is still found.
        for lines in ([":source-language: cpp", "", "[source]", "----", "int x;", "----"],
                      [":source-language: cpp", "", "----", "int x;", "----"],
                      [":source-language: viper", "[source]", "----", "int x;", "----"]):
            with self.subTest(lines=lines):
                messages = [m for _n, m in scan(*lines)]
                self.assertTrue(any("`:source-language:` is not allowed" in m for m in messages),
                                messages)
                self.assertTrue(any("without a declared language" in m for m in messages),
                                messages)
                self.assertTrue(any("inline" in m for m in messages), messages)
        # Unset again, or never used: only the attribute entry is reported.
        for lines in ([":source-language: cpp", ":!source-language:"],
                      [":source-language: cpp", ":source-language!:"],
                      [":source-language: sh"]):
            with self.subTest(lines=lines):
                self.assertEqual(len(scan(*lines)), 1)
        self.clean(":!source-language:", "[source,sh]", "----", "make", "----")

    def test_a_mixed_body_is_detected(self):
        self.flagged("[source,cpp]", "----", self.INCLUDE, "int x;", "----")
        self.flagged("[source,cpp]", "----", "int x;", self.INCLUDE, "----")
        # Comments inside a delimited block are code, not AsciiDoc comments.
        problems = scan("[source,cpp]", "----", "// just a comment", "----")
        self.assertEqual([n for n, _ in problems], [3])

    def test_one_problem_per_block_at_its_first_bad_line(self):
        problems = scan("[source,cpp]", "----", "int x;", "int y;", "----",
                        "", "[source,c]", "----", "int z;", "----")
        self.assertEqual([n for n, _ in problems], [3, 9])

    def test_an_empty_block_is_detected(self):
        problems = scan("[source,cpp]", "----", "", "----")
        self.assertEqual(len(problems), 1)
        self.assertIn("empty cpp source block", problems[0][1])

    def test_include_only_blocks_are_fine(self):
        self.clean("[source,cpp]", "----", self.INCLUDE, "----")
        self.clean("[source,cpp]", "----", "", self.INCLUDE, "", "----")
        self.clean("[source,cpp,linenums]", "----", "include::example$sub/b.vpr[tags=a;b]", "----")
        self.clean("[source,cpp]", self.INCLUDE)  # the paragraph form
        self.clean("[source,cpp]", "// note", self.INCLUDE)  # a comment in a paragraph
        self.clean("```cpp", self.INCLUDE, "```")

    def test_other_languages_and_plain_listings_are_fine(self):
        for lang in sorted(cde.OUTPUT_LANGUAGES):
            with self.subTest(lang=lang):
                self.clean(f"[source,{lang}]", "----", "int x;", "----")
        self.assertEqual(cde.OUTPUT_LANGUAGES, {"sh", "console", "text"})
        for lang in ("bash", "python", "yaml", "cmake", "json", "adoc", "asciidoc", "cs",
                     "cpps", "objc", "shell", "xml"):
            with self.subTest(lang=lang):
                (problem,) = scan(f"[source,{lang}]", "----", "int x;", "----")
                self.assertEqual(problem[0], 1)
                self.assertIn(f"language `{lang}` is not allowed in a documentation block",
                              problem[1])
        self.clean("[source,text]", "----", "int x;", "----")

    def test_an_undeclared_listing_or_a_literal_block_is_an_error(self):
        undeclared = "listing block without a declared language"
        for lines in (["----", "x", "----"], ["[source]", "----", "x", "----"],
                      ["[listing]", "----", "x", "----"], ["[quote,cpp]", "----", "x", "----"],
                      ["[#id.role]", "----", "x", "----"], [".Title", "----", "x", "----"],
                      ["[source,sh]", "", "----", "x", "----", "", "----", "y", "----"]):
            with self.subTest(lines=lines):
                problems = scan(*lines)
                self.assertEqual([n for n, m in problems if undeclared in m],
                                 [lines.index("----") + 1 if lines[0] != "[source,sh]" else 7],
                                 problems)
        for lines in (["....", "x", "...."], ["[source,sh]", "....", "x", "...."],
                      ["[source,cpp]", "......", "x", "......"]):
            with self.subTest(lines=lines):
                self.assertTrue(any("literal blocks (....) are not allowed" in m
                                    for _n, m in scan(*lines)), lines)

    def test_an_undeclared_source_paragraph_is_an_error(self):
        for style in ("source", "listing", "literal"):
            with self.subTest(style=style):
                (problem,) = scan(f"[{style}]", "x")
                self.assertEqual(problem[0], 2)
                self.assertIn("source paragraph without a declared language", problem[1])
        self.clean("[source,sh]", "x")

    def test_an_indented_paragraph_is_a_literal_block(self):
        (problem,) = scan("Text.", "", "  int main() {}")
        self.assertEqual(problem[0], 3)
        self.assertIn("indented paragraph is a literal block", problem[1])
        # Continuation lines of one paragraph may be indented; a paragraph that
        # *starts* indented is a literal block wherever it appears.
        self.clean("* A list item", "  continued text")
        self.clean("  * an indented list item", "  continued text")
        self.clean("term::", "  its definition on the next line")
        for lines, at in ((["* item", "", "  an indented paragraph"], 3),
                          (["* item", "+", "  literal after a list continuation"], 3),
                          (["term::", "+", "  literal after a dlist continuation"], 3),
                          (["term::", "", "  literal after a blank line"], 3),
                          (["|===", "a|", "  literal in an a cell", "|==="], 3),
                          (["|===", "a|text", "", "  literal in an a cell", "|==="], 4),
                          (["|===", "a|", "", "  literal in an a cell", "|==="], 4),
                          (["* item", "", "== Heading", "", "  code"], 5),
                          (["\tcode with a tab"], 1)):
            with self.subTest(lines=lines):
                problems = [p for p in scan(*lines) if "literal block" in p[1]]
                self.assertEqual([n for n, _ in problems], [at], scan(*lines))
        self.clean("|===", "  |b |c", "|===")  # an indented row of cells

    def test_subs_is_not_allowed(self):
        for line in ("[subs=none]", "[source,text,subs=none]", "[subs=-specialchars]",
                     '[source,cpp,subs="+quotes"]', "[ subs = none ]", "[source,text,SUBS=none]",
                     "[source,subs=none,text]", '[subs="attributes+"]', "[%hardbreaks,subs=none]"):
            with self.subTest(line=line):
                problems = [p for p in scan(line, "text") if "`subs=` is not allowed" in p[1]]
                self.assertEqual([n for n, _ in problems], [1], scan(line, "text"))
                self.assertIn("raw HTML", problems[0][1])
        for line in ("[source,text]", "[quote,\"a note on subs\"]", "[%hardbreaks]", "[NOTE]"):
            with self.subTest(line=line):
                self.assertEqual([p for p in scan(line, "text") if "`subs=`" in p[1]], [])

    def test_list_items_are_separate_scopes_for_formatting_marks(self):
        message = cde.FORMAT_MESSAGE
        # One mark per item (or entry) is safe; two in one item are not.
        self.assertEqual(scan("* `__a` here", "* `__b` there", "* `__c` too"), [])
        self.assertEqual(scan("* `__a`", "** `__b`", ". `__c`"), [])
        self.assertEqual(scan("term one:: the `__a` header", "term two:: the `__b` header"), [])
        self.assertEqual(scan("* item `__a`", "", "* item `__b`"), [])
        self.assertEqual(scan("* item `__a`", "  and `__b` on a continuation line"),
                         [(2, message)])
        self.assertEqual(scan("* first `__a` and `__b`", "* second `__c`"), [(1, message)])
        self.assertEqual(scan("* x", "* `__a` and", "  `__b` continued", "* `__c`"),
                         [(3, message)])
        self.assertEqual(scan("term:: a `__x` and", "  `__y`"), [(2, message)])
        # Plain paragraphs keep one scope.
        self.assertEqual(scan("Text `__a`", "more `__b`"), [(2, message)])

    def test_two_includes_in_a_block_are_an_error(self):
        (problem,) = scan("[source,cpp]", "----", self.INCLUDE, "", self.INCLUDE, "----")
        self.assertEqual(problem[0], 5)
        self.assertIn("holds one include", problem[1])

    def test_a_block_may_include_only_a_test_file_of_the_examples(self):
        for target in ("partial$snippet.cpp", "partial$frag.adoc", "example$bad.txt",
                       "example$frag.adoc", "example$a.hpp", "example$a.h", "example$a",
                       "attachment$a.cpp", "a.cpp", "../x/a.cpp", "other:example$a.cpp",
                       "https://example.com/a.cpp", "example$a.CPP"):
            with self.subTest(target=target):
                (problem,) = scan("[source,cpp]", "----", f"include::{target}[tag=x]", "----")
                self.assertEqual(problem[0], 3)
                self.assertIn("may include only example$<file ending in", problem[1])
        for target in ("example$a.cpp", "ROOT:example$a.cpp", "example$d/e/a.c",
                       "example$m.vpr"):
            with self.subTest(target=target):
                self.clean("[source,cpp]", "----", f"include::{target}[tag=x]", "----")

    def test_comments_are_skipped(self):
        self.clean("////", "[source,cpp]", "----", "int x;", "----", "////")
        self.clean("//////", "[source,cpp]", "----", "int x;", "----", "//////")
        self.clean("// [source,cpp]", "[source,text]", "----", "int x;", "----")
        self.clean("[source,sh]", "////", "x", "////", "----", "make", "----")
        # A comment block closes only with its own delimiter, and then the
        # next block is checked again.
        self.flagged("////", "[source,cpp]", "////", "[source,cpp]", "----", "int x;", "----")

    def test_attributes_do_not_leak_into_later_blocks(self):
        problems = scan("[source,cpp]", self.INCLUDE, "", "----", "int x;", "----")
        self.assertEqual([m for _n, m in problems if "inline" in m], [])
        # The heading takes the attributes: the next listing is not a cpp block.
        problems = scan("[source,cpp]", "== Heading", "----", "int x;", "----")
        self.assertEqual([m for _n, m in problems if "inline" in m], [])
        self.flagged("[source,cpp]", "----", "int x;", "----")  # control
        problems = scan("[source,cpp]", "----", "int x;", "----", "----", "int y;", "----")
        self.assertEqual([n for n, _ in problems], [3, 5])
        self.assertIn("inline", problems[0][1])
        self.assertIn("without a declared language", problems[1][1])

    def test_a_nested_block_is_still_seen(self):
        self.flagged("====", "[source,cpp]", "----", "int x;", "----", "====")
        self.flagged("|===", "a|", "[source,cpp]", "----", "int x;", "----", "|===")

    def test_an_escaped_include_is_not_an_include(self):
        problems = scan("[source,cpp]", "----", "\\include::example$a.cpp[tag=body]", "----")
        self.assertEqual([n for n, _ in problems], [3])

    def test_crlf_pages_are_read_like_lf_pages(self):
        text = "[source,cpp]\r\n----\r\nint x;\r\n----\r\n"
        self.assertEqual([n for n, _ in cde.scan_page(text).problems], [3])


GOOD_EXAMPLE = ("// expect: frontend-ok\n"
                "// tag::body[]\n"
                "int f() { return 1; }\n"
                "// end::body[]\n"
                "// tag::other[]\n"
                "int g() { return 2; }\n"
                "// end::other[]\n")


def page_with(*lines):
    return "= Page\n\n" + "\n".join(lines) + "\n"


def block_of(include):
    return page_with("[source,cpp]", "----", include, "----")


class DocLintCase(unittest.TestCase):
    """Runs cde.check_tree on a fixture documentation tree."""

    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.root = Path(tmp.name).resolve()
        self.component = self.root / "doc" / "modules" / "ROOT"
        (self.component / "pages").mkdir(parents=True)
        (self.component / "examples").mkdir()

    def page(self, name, text):
        path = self.component / "pages" / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)
        return path

    def example(self, name, text=GOOD_EXAMPLE):
        path = self.component / "examples" / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)
        return path

    def check(self):
        return cde.check_tree(self.root)

    def messages(self):
        return [str(p) for p in self.check().problems]

    def lint(self, *needles, count=None):
        """The problems; each needle must appear in some problem."""
        problems = self.messages()
        for needle in needles:
            self.assertTrue(any(needle in p for p in problems), (needle, problems))
        if count is not None:
            self.assertEqual(len(problems), count, problems)
        return problems


class IncludeLintTests(DocLintCase):
    def test_a_tagged_include_in_a_block_is_clean(self):
        self.example("a.cpp")
        self.page("p.adoc", block_of("include::example$a.cpp[tag=body]"))
        report = self.check()
        self.assertEqual(report.problems, [])
        self.assertEqual((report.pages, report.blocks, report.includes, report.examples),
                         (1, 1, 1, 1))

    def test_good_tag_forms(self):
        self.example("a.cpp")
        for attrs in ("tag=body", "tags=body", 'tag="body"', "tag=body,indent=0",
                      "indent=0,tag=body", "tags=body;other", 'tags="body;other"',
                      "tags=body,other"):
            with self.subTest(attrs=attrs):
                self.page("p.adoc", block_of(f"include::example$a.cpp[{attrs}]"))
                self.assertEqual(self.messages(), [])

    def test_untagged_and_non_name_tag_forms_are_errors(self):
        self.example("a.cpp")
        cases = {
            "": "untagged include",
            "indent=0": "untagged include",
            "opts=optional": "untagged include",
            "lines=1..2": "`lines=`",
            "lines=1..2,tag=body": "`lines=`",
            "tag=body,lines=1..2": "`lines=`",
            "tag=": "empty tag name",
            "tags=": "empty tag name",
            "tags=body;": "empty tag name",
            "tags=**": "not a plain tag name",
            "tags=*": "not a plain tag name",
            "tag=*": "not a plain tag name",
            "tag=**": "not a plain tag name",
            "tags=!body": "not a plain tag name",
            "tag=!body": "not a plain tag name",
            "tags=**;!body": "not a plain tag name",
            "tags=body;!other": "not a plain tag name",
            "tags=*;body": "not a plain tag name",
            "tag={name}": "attribute reference",
        }
        for attrs, needle in cases.items():
            for where in ("in a block", "in text"):
                with self.subTest(attrs=attrs, where=where):
                    line = f"include::example$a.cpp[{attrs}]"
                    self.page("p.adoc", block_of(line) if where == "in a block"
                              else page_with("Some text.", "", line))
                    problems = self.messages()
                    self.assertTrue(problems and all(needle in p for p in problems),
                                    (needle, problems))
                    self.assertTrue(all("p.adoc:" in p for p in problems), problems)

    def test_the_problem_names_the_include_line(self):
        self.example("a.cpp")
        self.page("p.adoc", block_of("include::example$a.cpp[]"))
        (problem,) = self.check().problems
        self.assertEqual((problem.file, problem.line),
                         ("doc/modules/ROOT/pages/p.adoc", 5))

    def test_a_missing_target_is_an_error(self):
        self.page("p.adoc", block_of("include::example$nothing.cpp[tag=body]"))
        self.lint("does not exist", "nothing.cpp", count=1)

    def test_a_missing_target_in_a_subdirectory_or_with_a_module_prefix(self):
        self.page("p.adoc", page_with("include::example$d/nothing.cpp[tag=body]",
                                      "", "include::ROOT:example$nothing.cpp[tag=body]"))
        self.lint("does not exist", count=2)

    def test_prefixed_and_nested_targets_resolve(self):
        self.example("d/e/a.cpp")
        self.page("p.adoc", page_with("include::ROOT:example$d/e/a.cpp[tag=body]", "",
                                      "include::example$d/e/a.cpp[tag=other]"))
        self.assertEqual(self.messages(), [])

    def test_other_targets_are_errors(self):
        self.example("a.cpp")
        for target in ("../examples/a.cpp", "a.cpp", "attachment$a.cpp",
                       "other:example$a.cpp", "page$a.cpp", "https://x.test/a.cpp",
                       "example$../../pages/p.adoc", "{examplesdir}/a.cpp"):
            with self.subTest(target=target):
                self.page("p.adoc", page_with(f"include::{target}[tag=body]"))
                problems = self.messages()
                self.assertTrue(any("p.adoc:3:" in p for p in problems), problems)

    def test_a_target_without_an_expect_header_is_an_error(self):
        self.example("a.cpp", "// tag::body[]\nint f();\n// end::body[]\n")
        self.page("p.adoc", block_of("include::example$a.cpp[tag=body]"))
        self.lint("not a valid test", "no `// expect:` line", count=1)

    def test_a_target_with_a_bad_header_is_an_error(self):
        self.example("a.cpp", GOOD_EXAMPLE.replace("frontend-ok", "frontend-nope"))
        self.page("p.adoc", block_of("include::example$a.cpp[tag=body]"))
        self.lint("not a valid test", "cannot parse", count=1)

    def test_an_expect_line_after_code_is_a_bad_header(self):
        self.example("a.cpp", "// expect: frontend-ok\nint f();\n// expect: frontend-ok\n"
                              "// tag::body[]\n// end::body[]\n")
        self.page("p.adoc", block_of("include::example$a.cpp[tag=body]"))
        self.lint("not a valid test", count=1)

    def test_a_viper_example_header_is_checked_too(self):
        self.example("m.vpr", "// expect: verified\n// tag::body[]\nmethod m() {}\n"
                              "// end::body[]\n")
        self.page("p.adoc", block_of("include::example$m.vpr[tag=body]"))
        self.assertEqual(self.messages(), [])
        self.example("m.vpr", "// expect: frontend-ok\n// tag::body[]\n// end::body[]\n")
        self.lint("not a valid test", count=1)

    def test_a_source_like_example_without_a_test_suffix_is_an_error(self):
        self.example("a.hpp", "// tag::body[]\nint f();\n// end::body[]\n")
        self.page("p.adoc", block_of("include::example$a.hpp[tag=body]"))
        self.lint("never run", "may include only", "not a test or a golden file", count=3)

    def test_a_tag_that_is_not_defined_is_an_error(self):
        self.example("a.cpp")
        self.page("p.adoc", block_of("include::example$a.cpp[tag=nothing]"))
        self.lint("tag `nothing` is not defined", count=1)
        self.page("p.adoc", block_of("include::example$a.cpp[tags=body;nothing]"))
        self.lint("tag `nothing` is not defined", count=1)

    def test_a_tag_that_is_never_closed_is_an_error(self):
        self.example("a.cpp", "// expect: frontend-ok\n// tag::body[]\nint f();\n")
        self.page("p.adoc", block_of("include::example$a.cpp[tag=body]"))
        self.lint("tag `body` is not defined", count=1)

    def test_a_tag_covering_the_header_is_an_error(self):
        self.example("a.cpp", "// tag::body[]\n// expect: frontend-ok\nint f();\n"
                              "// end::body[]\n")
        self.page("p.adoc", block_of("include::example$a.cpp[tag=body]"))
        self.lint("covers an `// expect:` or `// mode:` line", count=1)
        self.example("a.cpp", "// expect: frontend-ok\n// tag::body[]\n// mode: c23\n"
                              "int f();\n// end::body[]\n")
        self.lint("covers an `// expect:` or `// mode:` line", count=1)

    def test_a_partial_include_is_allowed_and_checked_for_existence(self):
        part = self.component / "partials"
        part.mkdir()
        (part / "note.adoc").write_text("A note.\n")
        self.page("p.adoc", page_with("include::partial$note.adoc[]"))
        self.assertEqual(self.messages(), [])
        self.page("p.adoc", page_with("include::partial$missing.adoc[]"))
        self.lint("does not exist", count=1)

    def test_a_cpp_block_in_a_partial_is_linted(self):
        part = self.component / "partials"
        part.mkdir()
        (part / "code.adoc").write_text("[source,cpp]\n----\nint x;\n----\n")
        self.page("p.adoc", "= P\n")
        problems = self.lint("inline cpp source", count=1)
        self.assertIn("partials/code.adoc:3:", problems[0])

    def test_the_nav_file_is_linted(self):
        self.example("a.cpp")
        (self.component / "nav.adoc").write_text("* xref:p.adoc[P]\n[source,cpp]\n----\nint x;\n----\n")
        self.page("p.adoc", block_of("include::example$a.cpp[tag=body]"))
        problems = self.lint("inline cpp source", count=1)
        self.assertIn("nav.adoc:4:", problems[0])

    def test_pages_in_subdirectories_are_linted(self):
        self.page("deep/er/p.adoc", "[source,cpp]\n----\nint x;\n----\n")
        problems = self.lint("inline cpp source", count=1)
        self.assertIn("pages/deep/er/p.adoc:3:", problems[0])

    def test_an_include_in_a_comment_block_is_still_checked(self):
        self.page("p.adoc", page_with("////", "include::example$nothing.cpp[tag=body]",
                                      "////"))
        self.lint("does not exist", count=1)


class OrphanLintTests(DocLintCase):
    def test_an_example_no_page_includes_is_an_orphan(self):
        self.example("a.cpp")
        self.example("used.cpp")
        self.page("p.adoc", block_of("include::example$used.cpp[tag=body]"))
        (problem,) = self.check().problems
        self.assertEqual((problem.file, problem.line),
                         ("doc/modules/ROOT/examples/a.cpp", 1))
        self.assertIn("orphan example", problem.message)

    def test_every_test_suffix_counts_in_subdirectories(self):
        self.example("x/y/a.cpp")
        self.example("x/b.c", GOOD_EXAMPLE)
        self.example("x/m.vpr", "// expect: verified\n")
        self.lint("orphan example", count=3)

    def test_an_include_in_a_comment_block_does_not_use_an_example(self):
        self.example("a.cpp")
        self.page("p.adoc", page_with("////", block_of("include::example$a.cpp[tag=body]"),
                                      "////"))
        self.lint("orphan example", count=1)

    def test_any_page_may_use_an_example(self):
        self.example("a.cpp")
        self.example("b.cpp")
        self.page("one/p.adoc", block_of("include::example$a.cpp[tag=body]"))
        self.page("q.adoc", page_with("Text.", "", "include::example$b.cpp[tag=body]"))
        self.assertEqual(self.messages(), [])

    def test_a_golden_file_is_not_an_orphan(self):
        self.example("a.cpp.golden", "x\n")
        self.page("p.adoc", "= P\n")
        self.assertEqual(self.messages(), [])

    def test_a_wrongly_used_example_still_counts_as_used(self):
        # The include is an error (untagged), not also an orphan.
        self.example("a.cpp")
        self.page("p.adoc", block_of("include::example$a.cpp[]"))
        self.lint("untagged include", count=1)


class BareCppTests(DocLintCase):
    """Rule e: C++ is written pass:[C++] in text (a bare C++ opens an AsciiDoc
    passthrough that silently swallows text; Antora does not warn)."""

    MESSAGE = "write C++ as pass:[C++] (a bare C++ opens an AsciiDoc passthrough)"

    def bare(self, *lines, at=None):
        """The page has exactly one rule-e problem, on line `at`."""
        problems = scan(*lines)
        self.assertEqual(problems, [(at, self.MESSAGE)], problems)

    def ok(self, *lines):
        self.assertEqual(scan(*lines), [])

    def test_bare_cpp_in_each_kind_of_text_is_an_error(self):
        cases = {
            "prose": ["A C++ verifier."],
            "second line of a paragraph": ["First line.", "A C++ verifier."],
            "title": [".The C++ front end", "text"],
            "heading": ["== C++ support"],
            "document title": ["= C++ verifier"],
            "unordered list item": ["* C++ is supported"],
            "ordered list item": [". C++ is supported"],
            "nested list item": ["** a C++ item"],
            "backticks": ["Use `C++` here."],
            "backticks with text": ["The `C++20` mode."],
            "xref text": ["See xref:overview.adoc[C++ overview]."],
            "link text": ["See https://example.com[C++ docs]."],
            "table cell": ["|===", "|C++ |yes", "|==="],
            "attribute entry": [":description: a C++ verifier"],
            "block attribute line": ['[caption="C++ example: "]'],
            "admonition": ["NOTE: C++ only."],
            "inside an example block": ["====", "C++ text", "===="],
            "inside an open block": ["--", "C++ text", "--"],
            "start of line": ["C++"],
            "end of line": ["written in C++"],
            "two on one line": ["C++ and C++"],
            "pass macro not exact": ["Use pass:[C++20] here."],
            "pass macro with a quoting substitution": ["Use pass:q[C++] here."],
            "bare next to a good one": ["pass:[C++] and C++"],
            "before a version": ["C++20"],
            "monospace passthrough": ["`+C++ code+`"],
        }
        for what, lines in cases.items():
            with self.subTest(what):
                at = next(i for i, l in enumerate(lines, start=1) if "C++" in l)
                self.bare(*lines, at=at)

    def test_pass_macro_is_fine_everywhere(self):
        for line in ("A pass:[C++] verifier.", "pass:[C++]", "* pass:[C++] is supported",
                     ".The pass:[C++] front end", "== pass:[C++] support",
                     "Use `pass:[C++]` here.", "See xref:overview.adoc[pass:[C++] overview].",
                     "Modes pass:[C++]20 and pass:[C++]23.", ":description: a pass:[C++] verifier",
                     "|pass:[C++] |yes", "An `int` and a pass:[C++] `int`.",
                     "pass:[C++] and pass:[C++]"):
            with self.subTest(line=line):
                self.ok(line)

    def test_text_without_cpp_is_fine(self):
        self.ok("C and c and C+ and C+ + and + + plus", "Use C or `cpp`.")
        self.ok("A line with C and with +increment+ and 2+2.")

    def test_verbatim_contexts_are_exempt(self):
        contexts = {
            "listing": ["[source,text]", "----", "g++ -std=c++20 -o C++ x.cpp", "----"],
            "listing with a shell language": ["[source,sh]", "----", "C++", "----"],
            "literal block": ["....", "C++", "...."],
            "longer delimiters": ["[source,text]", "------", "C++", "------"],
            "passthrough block": ["++++", "<b>C++</b>", "++++"],
            "comment block": ["////", "C++", "////"],
            "comment block with a block inside": ["////", "== C++", "* C++", "////"],
            "line comment": ["// the C++ one"],
            "fenced block": ["```sh", "C++", "```"],
            "fenced block without a language": ["```", "C++", "```"],
            "source paragraph": ["[source,sh]", "C++ output"],
            "listing paragraph": ["[listing]", "C++ output"],
            "literal paragraph": ["[literal]", "C++ output"],
            "open block with a source style": ["[source,sh]", "--", "C++", "--"],
            "source block language": ["[source,c++]", "----", "include::example$a.cpp[tag=body]",
                                      "----"],
            "source block language, upper case": ["[source,C++]", "----",
                                                  "include::example$a.cpp[tag=body]", "----"],
            "source block language attribute": ["[source,language=C++]", "----",
                                                "include::example$a.cpp[tag=body]", "----"],
            "shorthand source block": ["[,C++]", "----", "include::example$a.cpp[tag=body]",
                                       "----"],
            "include directive": ["include::example$C++/a.cpp[tag=body]"],
            "anchor": ["[[C++]]"],
        }
        for what, lines in contexts.items():
            with self.subTest(what):
                problems = [p for p in scan(*lines) if p[1] == self.MESSAGE]
                self.assertEqual(problems, [])

    def test_a_listing_does_not_hide_the_text_after_it(self):
        self.bare("[source,text]", "----", "C++", "----", "", "After the C++ listing.", at=6)
        self.bare("////", "C++", "////", "After the C++ comment.", at=4)
        problems = [p for p in scan("++++", "C++", "++++", "After the C++ block.")
                    if p[1] == self.MESSAGE]
        self.assertEqual(problems, [(4, self.MESSAGE)])
        self.bare("```sh", "C++", "```", "After the C++ fence.", at=4)
        self.bare("[source,sh]", "C++ output", "", "After the C++ paragraph.", at=4)

    def test_a_verbatim_style_applies_to_one_block_only(self):
        self.bare("[source,sh]", "----", "x", "----", "", "A C++ paragraph.", at=6)
        self.bare("[source,sh]", "C++ output", "", "A C++ paragraph.", at=4)

    def test_a_cpp_source_block_still_gets_rule_a_only(self):
        problems = scan("[source,cpp]", "----", "int C++ = 1;", "----")
        self.assertEqual([n for n, _ in problems], [3])
        self.assertIn("inline cpp source", problems[0][1])

    def test_one_problem_per_line(self):
        self.assertEqual(len(scan("C++ and C++ and C++")), 1)

    def test_the_problem_is_reported_as_file_line_message(self):
        self.page("p.adoc", "= Page\n\nA C++ verifier.\n")
        (problem,) = self.check().problems
        self.assertEqual(str(problem),
                         f"doc/modules/ROOT/pages/p.adoc:3: {self.MESSAGE}")

    def test_the_nav_file_and_partials_are_checked(self):
        self.page("p.adoc", "= Page\n")
        (self.component / "nav.adoc").write_text("* xref:p.adoc[C++]\n")
        (self.component / "partials").mkdir()
        (self.component / "partials" / "x.adoc").write_text("\nC++\n")
        problems = self.messages()
        self.assertEqual(len(problems), 2, problems)
        self.assertTrue(any("nav.adoc:1:" in p for p in problems), problems)
        self.assertTrue(any("partials/x.adoc:2:" in p for p in problems), problems)

    def test_a_clean_page_has_no_problem(self):
        self.page("p.adoc", "= Page\n\nA pass:[C++] verifier.\n\n[source,text]\n----\nC++\n----\n")
        self.assertEqual(self.messages(), [])


class PlusPlusTests(unittest.TestCase):
    """Rule e, generalised: any `++` in text outside pass:[...] opens an
    AsciiDoc passthrough (a real build garbles a paragraph with two of them,
    in backticks too, and `+...+` does not protect a `++`)."""

    GENERIC = ("write it inside pass:[...] (a ++ opens an AsciiDoc passthrough; write C++ "
               "as pass:[C++], libc++ as pass:[libc++])")

    def test_each_form_is_an_error(self):
        for text in ("The libc++ library.", "Use g++ or clang++.", "Pass -std=c++20 here.",
                     "Use `--c++20` here.", "Use `+--c++20+` here.", "A +++raw+++ text.",
                     "++x and x++", "* item with c++", ".A title with c++",
                     "== Heading with c++", ":attr: a c++ value", "[quote,\"g++\"]"):
            with self.subTest(text=text):
                problems = scan(text)
                self.assertEqual(problems, [(1, self.GENERIC)], problems)

    def test_pass_macros_protect_it(self):
        for text in ("The pass:[libc++] library.", "Use `pass:[--c++20]` here.",
                     "pass:[g++] and pass:[clang++]", "Use pass:q[c++] or pass:[c++,c++].",
                     "The pass:[libc++] and `pass:[--c++20]` and pass:[libc++]."):
            with self.subTest(text=text):
                self.assertEqual(scan(text), [])

    def test_text_that_is_not_a_plusplus_is_fine(self):
        self.assertEqual(scan("A +literal+ and 2 + 2 and a+b and +x+ and + +."), [])

    def test_code_blocks_and_comments_are_exempt(self):
        self.assertEqual(scan("[source,sh]", "----", "g++ -std=c++20 x.cpp", "----"), [])
        self.assertEqual(scan("////", "g++", "////"), [])
        self.assertEqual(scan("// g++"), [])
        self.assertEqual(scan("[source,sh]", "g++ x.cpp"), [])

    def test_the_c_plus_plus_message_stays_specific(self):
        self.assertEqual(scan("A C++ and libc++ text."),
                         [(1, "write C++ as pass:[C++] (a bare C++ opens an AsciiDoc "
                              "passthrough)")])


class FormattingMarkTests(unittest.TestCase):
    """Rule f: a doubled formatting mark (`__`, `**`, `##`) twice in a
    paragraph outside `+...+` and pass:[...] formats across code spans; a
    single one is safe (checked against a real build in the e2e twin)."""

    def test_a_second_mark_in_a_paragraph_is_an_error(self):
        for mark in ("__", "**", "##"):
            with self.subTest(mark=mark):
                (problem,) = scan(f"Use `{mark}algorithm` and `{mark}new/size.h` here.")
                self.assertEqual(problem, (1, cde.FORMAT_MESSAGE))
                # across lines of one paragraph: reported on the line of the second one
                (problem,) = scan(f"Use `{mark}algorithm` and", f"the `{mark}new/size.h` here.")
                self.assertEqual(problem, (2, cde.FORMAT_MESSAGE))

    def test_the_message(self):
        self.assertEqual(cde.FORMAT_MESSAGE, "wrap it in a literal span (`+...+`): a doubled "
                         "formatting mark opens unconstrained formatting")

    def test_a_pair_in_one_span_or_one_title_is_an_error(self):
        for lines in (["The macro `__GNUC__` is set."], [".The __a and __b title"],
                      ["== About **this** page"], [":attr: a ##b## c"],
                      ["* __a and __b"], ["Names __a and __b."]):
            with self.subTest(lines=lines):
                self.assertEqual([m for _n, m in scan(*lines)], [cde.FORMAT_MESSAGE])

    def test_a_single_mark_is_safe(self):
        for text in ("Use the `__algorithm` header.", "A 2**3 and 4 here.", "Item ##1 here."):
            with self.subTest(text=text):
                self.assertEqual(scan(text), [])
        self.assertEqual(scan("Use `__algorithm` here.", "", "Use `__new` there."), [])
        self.assertEqual(scan("A `__a`.", "", "== A `__b`", "", ".A `__c` title"), [])

    def test_literal_spans_and_pass_macros_protect_the_marks(self):
        for text in ("Use `+__algorithm+` and `+__new/interference_size.h+` here.",
                     "Use `pass:[__algorithm]` and `pass:[__new/size.h]` here.",
                     "A +**bold**+ and +##x##+ text.", "A `+__a+` and `__b` here."):
            with self.subTest(text=text):
                self.assertEqual(scan(text), [])

    def test_list_markers_are_not_marks(self):
        self.assertEqual(scan("** nested item", "** another item"), [])
        self.assertEqual(scan("* item", "** nested", "*** deeper"), [])

    def test_code_blocks_comments_and_cells_are_handled(self):
        self.assertEqual(scan("[source,sh]", "----", "__a __b", "----"), [])
        self.assertEqual(scan("// __a __b"), [])
        # Cells are separate: one mark per cell is safe, two in a cell are not.
        self.assertEqual(scan("|===", "|`__a` |`__b`", "|==="), [])
        self.assertEqual([m for _n, m in scan("|===", "|`__a` and `__b` |x", "|===")],
                         [cde.FORMAT_MESSAGE])

    def test_only_the_lines_with_the_marks_are_reported(self):
        problems = scan("A __a", "plain line", "another __b", "plain again")
        self.assertEqual([n for n, _ in problems], [3])


class RenderedTextRuleTests(unittest.TestCase):
    """N1(b): ways to show code or raw HTML without a declared language."""

    def messages(self, *lines):
        return [m for _n, m in scan(*lines)]

    def test_a_passthrough_block_is_an_error(self):
        for lines in (["++++", "<pre>x</pre>", "++++"], ["++++", "text", "++++"],
                      ["[source,text]", "++++", "x", "++++"], ["+++++", "x", "+++++"]):
            with self.subTest(lines=lines):
                problems = [p for p in scan(*lines) if "passthrough blocks" in p[1]]
                self.assertEqual(len(problems), 1, scan(*lines))
                self.assertIn("raw HTML", problems[0][1])

    def test_a_pass_macro_with_markup_is_an_error(self):
        for text in ("A pass:[<pre>int x;</pre>] b.", "pass:[<b>x</b>]", "pass:q[<i>x</i>]",
                     "pass:a,q[<code class=\"lang-cpp\">x</code>]", "* pass:[<br>]",
                     ".A pass:[<b>] title"):
            with self.subTest(text=text):
                self.assertTrue(any("pass:[...] with markup" in m for m in self.messages(text)),
                                text)
        self.assertEqual(self.messages("pass:[C++] and pass:[libc++] and pass:[x &amp; y]"), [])

    def test_the_verse_and_pass_styles_are_errors(self):
        for lines, at in ((["[verse]", "line one"], 1), (["[verse,Author]", "x"], 1),
                          (["[pass]", "<pre>x</pre>"], 1), (["[verse]", "----", "x", "----"], 1)):
            with self.subTest(lines=lines):
                problems = [p for p in scan(*lines) if "style is not allowed" in p[1]]
                self.assertEqual([n for n, _ in problems], [at], scan(*lines))
        self.assertEqual(self.messages("[quote,Author]", "A quote."), [])
        self.assertEqual(self.messages("[NOTE]", "A note."), [])

    def test_literal_and_monospace_cell_styles_are_errors(self):
        for cell in ("l|int x;", "m|int x;", "2+l|x", ".2+^.^l|x", "|a l|b", "3*m|x"):
            with self.subTest(cell=cell):
                problems = [p for p in scan("|===", cell, "|===") if "cell styles l and m" in p[1]]
                self.assertEqual([n for n, _ in problems], [2], scan("|===", cell, "|==="))

    def test_other_cell_styles_are_fine(self):
        self.assertEqual(self.messages("|===", "a|text", "h|head", "e|x", "s|y", "d|z",
                                       "|plain |cells", "|==="), [])

    def test_literal_and_monospace_column_specs_are_errors(self):
        for cols in ("1l", "l", "1,2m", "<.^3l,1", "2*l", "50%m", "1a,2l"):
            with self.subTest(cols=cols):
                problems = [p for p in scan(f'[cols="{cols}"]', "|===", "|x", "|===")
                            if "column styles l and m" in p[1]]
                self.assertEqual([n for n, _ in problems], [1])
        for cols in ("1,2", "1a,2", "3*", "1h,2e,3s,4d", "2,3"):
            with self.subTest(cols=cols):
                self.assertEqual(self.messages(f'[cols="{cols}"]', "|===", "|x", "|==="), [])

    def test_a_list_continuation_ends_the_paragraph(self):
        self.assertEqual(self.messages("* item", "+", "A continued paragraph."), [])
        self.assertEqual(self.messages("* item", "+", "[source,sh]", "----", "x", "----"), [])



class DocLintCliTests(DocLintCase):
    def run_cli(self, *args):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            status = cde.main([str(a) for a in args])
        return status, out.getvalue(), err.getvalue()

    def test_clean_tree_exits_0_with_a_summary(self):
        self.example("a.cpp")
        self.page("p.adoc", block_of("include::example$a.cpp[tag=body]"))
        status, out, _ = self.run_cli("--root", self.root)
        self.assertEqual(status, 0, out)
        self.assertEqual(out.strip(), "check_doc_examples: OK: 1 pages, 1 source blocks, "
                                      "1 example includes, 1 examples, 0 problems")

    def test_problems_exit_1_and_print_file_line_message(self):
        self.page("p.adoc", "[source,cpp]\n----\nint x;\n----\n")
        status, out, _ = self.run_cli("--root", self.root)
        self.assertEqual(status, 1, out)
        first = out.splitlines()[0]
        self.assertTrue(first.startswith("doc/modules/ROOT/pages/p.adoc:3: inline cpp source"),
                        first)
        self.assertIn("FAILED", out.splitlines()[-1])
        self.assertIn("1 problems", out.splitlines()[-1])

    def test_a_missing_component_is_a_usage_error(self):
        shutil.rmtree(self.component / "pages")
        status, out, err = self.run_cli("--root", self.root)
        self.assertEqual(status, 2, out)
        self.assertIn("no documentation component to check", err)
        status, _, err = self.run_cli("--root", self.root / "nowhere")
        self.assertEqual(status, 2)

    def test_the_default_root_is_the_repository(self):
        self.assertEqual(cde.ROOT, REAL_ROOT)


class ExamplesDirectoryTests(DocLintCase):
    """Every file under examples/ is a test or its golden file; no other
    C/C++/Viper source is published (so nothing escapes the harness)."""

    def test_only_tests_and_goldens_are_allowed_in_examples(self):
        # Reviewed change: these used to be ignored (notes.txt, README.adoc).
        self.page("p.adoc", "= P\n")
        for name in ("notes.txt", "README.adoc", "frag.adoc", "bad.txt", "x.h", "x.hpp",
                     ".gitkeep", ".harness-skip", "sub/data.json", "a.cc", "page.html"):
            with self.subTest(name=name):
                self.example(name, "text\n")
                problems = self.messages()
                self.assertEqual(len(problems), 1, problems)
                self.assertIn(f"examples/{name}:1: not a test or a golden file", problems[0])
                (self.component / "examples" / name).unlink()

    def test_tests_goldens_and_ds_store_are_allowed(self):
        self.example("a.cpp")
        self.example("a.cpp.golden", "x\n")
        self.example("b.c", "// expect: frontend-ok\n")
        self.example("b.c.c23.golden", "x\n")
        self.example(".DS_Store", "\0")
        self.page("p.adoc", block_of("include::example$a.cpp[tag=body]")
                  + "\n" + page_with("include::example$b.c[tag=nothing]"))
        problems = self.messages()
        self.assertFalse(any("not a test or a golden file" in p for p in problems), problems)

    def test_source_outside_the_examples_is_an_error(self):
        self.page("p.adoc", "= P\n")
        for path in ("partials/snippet.cpp", "attachments/x.cpp", "images/x.h",
                     "pages/x.vpr", "partials/y.CPP", "attachments/deep/er/z.c",
                     "partials/w.hpp", "partials/v.sil", "x.cc"):
            with self.subTest(path=path):
                f = self.component / path
                f.parent.mkdir(parents=True, exist_ok=True)
                f.write_text("int x;\n")
                problems = self.messages()
                self.assertEqual(len(problems), 1, problems)
                self.assertIn(f"{path}:1: C/C++/Viper source outside "
                              "doc/modules/ROOT/examples/", problems[0])
                f.unlink()

    def test_other_files_outside_the_examples_are_fine(self):
        self.page("p.adoc", "= P\n")
        (self.component / "images").mkdir()
        (self.component / "images" / "x.svg").write_text("<svg/>\n")
        (self.component / "partials").mkdir()
        (self.component / "partials" / "x.adoc").write_text("text\n")
        (self.component / "attachments").mkdir()
        (self.component / "attachments" / "x.pdf").write_text("pdf\n")
        self.assertEqual(self.messages(), [])


class ModuleAndComponentTests(DocLintCase):
    def test_a_module_other_than_root_is_a_usage_error(self):
        self.page("p.adoc", "= P\n")
        extra = self.root / "doc" / "modules" / "extra" / "pages"
        extra.mkdir(parents=True)
        (extra / "x.adoc").write_text("[source,cpp]\n----\nint x;\n----\n")
        with self.assertRaises(cde.UsageError) as ctx:
            cde.check_tree(self.root)
        self.assertIn("doc/modules/extra", str(ctx.exception))
        self.assertIn("module other than ROOT", str(ctx.exception))
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            status = cde.main(["--root", str(self.root)])
        self.assertEqual(status, 2, out.getvalue())
        self.assertIn("doc/modules/extra", err.getvalue())

    def test_a_module_without_pages_is_still_refused(self):
        self.page("p.adoc", "= P\n")
        (self.root / "doc" / "modules" / "empty").mkdir()
        with self.assertRaises(cde.UsageError):
            cde.check_tree(self.root)

    def test_a_source_language_in_a_component_file_is_an_error(self):
        self.page("p.adoc", "= P\n")
        doc = self.root / "doc"
        cases = {
            "antora.yml": "name: ROOT\nasciidoc:\n  attributes:\n    source-language: cpp\n",
            "local-playbook.yml": "site:\n  title: x\nasciidoc:\n  attributes:\n"
                                  "    source-language: cpp@\n",
            "other-playbook.yml": "asciidoc:\n  attributes:\n    source-language: c\n",
        }
        for name, text in cases.items():
            with self.subTest(name=name):
                (doc / name).write_text(text)
                (problem,) = self.check().problems
                self.assertEqual(problem.file, f"doc/{name}")
                self.assertEqual(problem.line, text.split("\n").index(
                    next(l for l in text.split("\n") if "source-language" in l)) + 1)
                self.assertIn("`source-language` must not be set", problem.message)
                (doc / name).unlink()

    def test_clean_component_files_and_comments_are_fine(self):
        self.page("p.adoc", "= P\n")
        (self.root / "doc" / "antora.yml").write_text(
            "name: ROOT\n# source-language is not set here\nasciidoc:\n  attributes:\n"
            "    experimental: ''\n")
        (self.root / "doc" / "local-playbook.yml").write_text("site:\n  title: x\n")
        self.assertEqual(self.messages(), [])


class TableCellTests(unittest.TestCase):
    def test_a_source_block_that_starts_on_a_cell_line_is_detected(self):
        cells = ["a|[source,cpp]", "2+a|[source,cpp]", ".2+^.^a|[source,cpp]",
                 "|[source,cpp]", "|x a|[source,cpp]", "a|[source,language=cpp]"]
        for cell in cells:
            with self.subTest(cell=cell):
                problems = scan("|===", cell, "----", "int x;", "----", "|===")
                self.assertEqual([n for n, _ in problems], [4], problems)
                self.assertIn("inline", problems[0][1])

    def test_a_cell_after_another_cell_line_is_detected(self):
        problems = scan("|===", "|first cell", "a|[source,cpp]", "----", "int x;", "----", "|===")
        self.assertEqual([n for n, _ in problems], [5], problems)

    def test_an_include_only_cell_block_is_fine(self):
        self.assertEqual(scan("|===", "a|[source,cpp]", "----",
                              "include::example$a.cpp[tag=body]", "----", "|==="), [])

    def test_text_cells_and_other_languages_are_fine(self):
        self.assertEqual(scan("|===", "|a |b", "a|[source,sh]", "----", "x", "----", "|==="), [])

    def test_the_table_ends_at_its_closing_delimiter(self):
        # After the table, a line starting with `a|` is plain text again.
        problems = scan("|===", "|x", "|===", "", "a|[source,cpp]", "----", "int x;", "----")
        self.assertEqual([m for _n, m in problems if "inline" in m], [])


class DeadCodeTagTests(DocLintCase):
    """A tag directive inside #if/#ifdef/#ifndef or /* */ can show code that
    the test never compiles."""

    def example_page(self, text, name="d.cpp"):
        self.example(name, text)
        self.page("p.adoc", block_of(f"include::example$%s[tag=x]" % name))

    def risky(self, text, name="d.cpp"):
        self.example_page(text, name)
        return [m for m in self.messages() if "`#if`" in m]

    def test_a_tag_inside_a_conditional_is_an_error(self):
        for directive in ("#if 0", "#ifdef NEVER", "#ifndef ALWAYS_DEFINED", "# if 0",
                          "  #ifdef X"):
            with self.subTest(directive=directive):
                problems = self.risky("// expect: frontend-ok\n" + directive + "\n"
                                      "// tag::x[]\nint shown() { return 1/0 +; }\n"
                                      "// end::x[]\n#endif\nint real() { return 0; }\n")
                self.assertEqual(len(problems), 1, problems)
                self.assertIn("tag `x` in `example$d.cpp` has a tag directive inside", problems[0])

    def test_an_include_guard_around_the_tags_is_an_error(self):
        self.assertEqual(len(self.risky("// expect: frontend-ok\n#ifndef G_H\n#define G_H\n"
                                        "// tag::x[]\nint f();\n// end::x[]\n#endif\n")), 1)

    def test_a_directive_closing_inside_a_conditional_is_an_error(self):
        self.assertEqual(len(self.risky("// expect: frontend-ok\n// tag::x[]\n#if 0\nint f();\n"
                                        "// end::x[]\n#endif\n")), 1)

    def test_a_tag_inside_a_block_comment_is_an_error(self):
        for text in ("// expect: frontend-ok\n/*\n// tag::x[]\nint f();\n// end::x[]\n*/\n",
                     "// expect: frontend-ok\n/* // tag::x[] */\nint f();\n// end::x[]\n",
                     "// expect: frontend-ok\n// tag::x[]\nint f();\n/* start\n// end::x[]\n*/\n"):
            with self.subTest(text=text):
                self.assertEqual(len(self.risky(text)), 1)

    def test_a_viper_block_comment_is_checked_too(self):
        problems = self.risky("// expect: verified\n/*\n// tag::x[]\nmethod m() {}\n"
                              "// end::x[]\n*/\n", name="d.vpr")
        self.assertEqual(len(problems), 1)
        # `#if` is C preprocessor syntax: it means nothing in Viper.
        self.assertEqual(self.risky("// expect: verified\n#if 0\n// tag::x[]\nmethod m() {}\n"
                                    "// end::x[]\n#endif\n", name="d.vpr"), [])

    def test_balanced_conditionals_and_comments_around_the_tags_are_fine(self):
        for text in ("// expect: frontend-ok\n#if 0\nint dead();\n#endif\n"
                     "// tag::x[]\nint f();\n// end::x[]\n",
                     "// expect: frontend-ok\n/* a\ncomment */\n// tag::x[]\nint f();\n"
                     "// end::x[]\n/* tail */\n",
                     "// expect: frontend-ok\n// tag::x[]\n#if 1\nint f();\n#endif\n"
                     "// end::x[]\n",
                     "// expect: frontend-ok\n// tag::x[] /* not a block comment here\n"
                     "int f();\n// end::x[]\n"):
            with self.subTest(text=text):
                self.assertEqual(self.risky(text), [])

    def test_only_the_included_tags_are_checked(self):
        self.example("d.cpp", "// expect: frontend-ok\n// tag::x[]\nint f();\n// end::x[]\n"
                              "#if 0\n// tag::y[]\nint g();\n// end::y[]\n#endif\n")
        self.page("p.adoc", block_of("include::example$d.cpp[tag=x]"))
        self.assertEqual(self.messages(), [])
        self.page("p.adoc", block_of("include::example$d.cpp[tags=x;y]"))
        self.assertEqual(len(self.messages()), 1)

    def test_a_tag_inside_unclosed_parentheses_is_an_error(self):
        # The arguments of a macro that discards them: shown, never compiled.
        for text in ("// expect: frontend-ok\n#define DROP(...)\nDROP(\n// tag::x[]\n"
                     "int shown() { return 1/0 +; }\n// end::x[]\n)\nint real() { return 0; }\n",
                     "// expect: frontend-ok\n#define DROP(...)\nDROP(a,\n// tag::x[]\nint f();\n"
                     "// end::x[]\n)\n",
                     "// expect: frontend-ok\nint f(int a,\n// tag::x[]\nint b);\n// end::x[]\n",
                     "// expect: frontend-ok\nint g = h(1,\n  (2 +\n// tag::x[]\n3));\n"
                     "// end::x[]\n"):
            with self.subTest(text=text):
                problems = self.risky(text)
                self.assertEqual(len(problems), 1, problems)
                self.assertIn("unclosed parentheses", problems[0])

    def test_parentheses_that_close_or_hide_in_strings_and_comments_are_fine(self):
        for text in ("// expect: frontend-ok\nint f(int a,\n int b);\n// tag::x[]\nint g();\n"
                     "// end::x[]\n",
                     "// expect: frontend-ok\nconst char *s = \"(\";\nchar c = '(';\n// (\n"
                     "/* ( */\n// tag::x[]\nint g();\n// end::x[]\n",
                     "// expect: frontend-ok\nconst char *s = \"a \\\" (\";\n// tag::x[]\nint g();\n"
                     "// end::x[]\n",
                     "// expect: frontend-ok\n// tag::x[]\nint f(int a,\n int b);\n// end::x[]\n"):
            with self.subTest(text=text):
                self.assertEqual(self.risky(text), [])

    def test_a_raw_string_in_an_example_is_an_error(self):
        for literal in ('const char *s = R"(x)";', 'auto s = u8R"(x)";', 'auto s = LR"x(y)x";',
                        'auto s = uR"(x)"; ', 'auto s = UR"(x)";', 'f(R"(x)");', '  R"(x)"'):
            with self.subTest(literal=literal):
                self.example("d.cpp", "// expect: frontend-ok\n// tag::x[]\nint f();\n"
                                      "// end::x[]\n" + literal + "\n")
                self.page("p.adoc", block_of("include::example$d.cpp[tag=x]"))
                (problem,) = self.check().problems
                self.assertEqual((problem.file, problem.line),
                                 ("doc/modules/ROOT/examples/d.cpp", 5))
                self.assertIn("raw string literal", problem.message)

    def test_raw_string_lookalikes_and_viper_are_fine(self):
        self.example("d.cpp", "// expect: frontend-ok\n// tag::x[]\nint FOOR;\n// end::x[]\n"
                              'const char *s = "R";\nint f(int bR) { return bR; }\n'
                              'const char *t = FOOR"x";\n')
        self.page("p.adoc", block_of("include::example$d.cpp[tag=x]"))
        self.assertEqual(self.messages(), [])
        (self.component / "examples" / "d.cpp").unlink()
        self.example("m.vpr", '// expect: verified\n// tag::x[]\nmethod m() {}\n// end::x[]\n'
                              '// R"(x)"\n')
        self.page("p.adoc", block_of("include::example$m.vpr[tag=x]"))
        self.assertEqual(self.messages(), [])

    def test_a_line_comment_ending_in_a_backslash_is_an_error(self):
        for line in ("// a note \\", "//\\", "int x; // note \\", "// a note \\  ",
                     "  // indented \\"):
            with self.subTest(line=line):
                self.example("d.cpp", "// expect: frontend-ok\n// tag::y[]\nint x;\n// end::y[]\n"
                                      + line + "\nint shown_not_compiled();\n")
                self.page("p.adoc", block_of("include::example$d.cpp[tag=y]"))
                (problem,) = self.check().problems
                self.assertEqual((problem.file, problem.line),
                                 ("doc/modules/ROOT/examples/d.cpp", 5))
                self.assertIn("a `//` comment ends in a backslash", problem.message)

    def test_a_backslash_elsewhere_is_fine(self):
        self.example("d.cpp", "// expect: frontend-ok\n// tag::y[]\n#define M(a) \\\n  (a)\n"
                              "// end::y[]\n// a path C:\\dir\\x\nconst char *p = \"a\\\\\";\n")
        self.page("p.adoc", block_of("include::example$d.cpp[tag=y]"))
        self.assertEqual(self.messages(), [])

    def test_strip_code(self):
        self.assertEqual(cde.strip_code('f("(", \')\') // (', False), ("f(, ) ", False))
        self.assertEqual(cde.strip_code("a /* ( */ b (", False), ("a  b (", False))
        self.assertEqual(cde.strip_code("a /* ( ", False), ("a ", True))
        self.assertEqual(cde.strip_code(" ) */ b", True), (" b", False))

    def test_a_tag_directive_must_be_a_line_comment_alone_on_its_line(self):
        shown = "int shown() { return 1/0 +; }"
        cases = {
            "continued define": f"#define UNUSED \\\n tag::x[] \\\n {shown} \\\n end::x[]\n",
            "continued define, comment markers": (f"#define UNUSED \\\n// tag::x[] \\\n{shown} \\\n"
                                                  "// end::x[]\n"),
            "code before": f"int a; // tag::x[]\n{shown}\nint b; // end::x[]\n",
            "text after": f"// tag::x[] trailing\n{shown}\n// end::x[]\n",
            "in a block comment": f"/* tag::x[] */\n{shown}\n/* end::x[] */\n",
            "no comment": f"tag::x[]\n{shown}\nend::x[]\n",
            "hash comment": f"# tag::x[]\n{shown}\n# end::x[]\n",
        }
        for what, body in cases.items():
            with self.subTest(what):
                self.example("d.cpp", "// expect: frontend-ok\n" + body)
                self.page("p.adoc", block_of("include::example$d.cpp[tag=x]"))
                problems = [m for m in self.messages() if "a tag directive" in m]
                self.assertTrue(problems, (what, self.messages()))
                self.assertIn("d.cpp:", problems[0])

    def test_a_tag_directive_after_a_backslash_line_is_an_error(self):
        self.example("d.cpp", "// expect: frontend-ok\n#define M \\\n// tag::x[]\nint f();\n"
                              "// end::x[]\n")
        self.page("p.adoc", block_of("include::example$d.cpp[tag=x]"))
        problems = [m for m in self.messages() if "follows a line ending in a backslash" in m]
        self.assertEqual(len(problems), 1, self.messages())
        self.assertIn("d.cpp:3:", problems[0])

    def test_well_formed_directive_lines_are_fine(self):
        for tag, end in (("// tag::x[]", "// end::x[]"), ("  // tag::x[]", "  // end::x[]"),
                         ("//tag::x[]", "//end::x[]"), ("// tag::x[]  ", "//  end::x[]\t")):
            with self.subTest(tag=tag):
                self.example("d.cpp", f"// expect: frontend-ok\n{tag}\nint f();\n{end}\n")
                self.page("p.adoc", block_of("include::example$d.cpp[tag=x]"))
                self.assertEqual(self.messages(), [])

    def test_comment_state_helpers(self):
        self.assertTrue(cde.in_block_comment("/* open", False))
        self.assertFalse(cde.in_block_comment("/* a */ b", False))
        self.assertTrue(cde.in_block_comment("/* a */ /* b", False))
        self.assertFalse(cde.in_block_comment("// /* not a block", False))
        self.assertFalse(cde.in_block_comment("end */", True))
        self.assertTrue(cde.in_block_comment("still open", True))


HTML_PAGE = """<!DOCTYPE html><html><body>
<div class="listingblock"><div class="content">
<pre class="highlightjs highlight"><code class="language-cpp hljs" data-lang="cpp">{code}</code></pre>
</div></div>
</body></html>
"""
HTML_EXAMPLE = ("// expect: frontend-ok\n// tag::one[]\n#include <cstdio>\n"
                "int f() { return 1 < 2 && 3 > 2; }\n// end::one[]\n"
                "// tag::two[]\nint g() { return 2; }\n// end::two[]\n"
                "int unshown() { return 3; }\n")


class HtmlCheckTests(DocLintCase):
    """--html: every cerastes-language code block of the built site equals a
    tagged region of a test, whatever the page sources look like."""

    def setUp(self):
        super().setUp()
        self.example("a.cpp", HTML_EXAMPLE)
        self.page("p.adoc", block_of("include::example$a.cpp[tag=one]"))
        self.out = self.root / "site"
        self.out.mkdir()

    def site(self, code, name="p.html", template=HTML_PAGE):
        (self.out / name).parent.mkdir(parents=True, exist_ok=True)
        (self.out / name).write_text(template.format(code=code))

    def run_html(self):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            status = cde.main(["--root", str(self.root), "--html", str(self.out)])
        return status, out.getvalue(), err.getvalue()

    GOOD = "#include &lt;cstdio&gt;\nint f() { return 1 &lt; 2 &amp;&amp; 3 &gt; 2; }"

    def test_a_block_equal_to_its_region_passes(self):
        self.site(self.GOOD)
        status, out, err = self.run_html()
        self.assertEqual(status, 0, (out, err))
        self.assertIn("check_doc_examples --html: OK: 1 html files, 1 code blocks", out)

    def test_whitespace_differences_are_ignored(self):
        self.site("\n\n  #include &lt;cstdio&gt;   \n  int f() { return 1 &lt; 2 &amp;&amp; 3 "
                  "&gt; 2; }\n\n")
        self.assertEqual(self.run_html()[0], 0)

    def test_code_that_is_not_a_region_fails(self):
        for code in ("int f() { return 1/0; }", "int f() { return 1 &lt; 2 &amp;&amp; 3 &gt; 2; }",
                     self.GOOD + "\nint extra();", "#include &lt;cstdio&gt;",
                     "int unshown() { return 3; }",  # in the file, outside every tag
                     "// expect: frontend-ok"):
            with self.subTest(code=code):
                self.site(code)
                status, out, _ = self.run_html()
                self.assertEqual(status, 1, out)
                self.assertIn("p.html:3: cpp code block", out)
                self.assertIn("does not equal any tagged region", out)

    def test_every_block_of_every_page_is_checked(self):
        self.site(self.GOOD)
        self.site("int bad;", name="deep/er/q.html")
        status, out, _ = self.run_html()
        self.assertEqual(status, 1, out)
        self.assertIn("deep/er/q.html:3:", out)
        self.assertNotIn("p.html:", out)

    def test_two_tags_join_in_file_order_and_any_tag_alone_matches(self):
        self.site("int g() { return 2; }")
        self.assertEqual(self.run_html()[0], 0)
        self.site(self.GOOD + "\nint g() { return 2; }")
        self.assertEqual(self.run_html()[0], 0)
        self.site("int g() { return 2; }\n" + self.GOOD)  # not file order
        self.assertEqual(self.run_html()[0], 1)

    def test_language_names_and_data_lang(self):
        for attrs in ('class="language-cpp"', 'class="language-C++ hljs"',
                      'data-lang="cpp"', 'class="hljs language-hpp"', 'data-lang="viper"',
                      'class="language-VPR"', 'class="language-c"'):
            with self.subTest(attrs=attrs):
                template = HTML_PAGE.replace('class="language-cpp hljs" data-lang="cpp"', attrs)
                self.site("int bad;", template=template)
                self.assertEqual(self.run_html()[0], 1)

    def test_other_languages_and_plain_code_are_ignored(self):
        self.site(self.GOOD)
        (self.out / "o.html").write_text(
            '<pre><code class="language-sh" data-lang="sh">make</code></pre>'
            '<code>int bad;</code><code class="language-text">int bad;</code>'
            '<pre><code class="language-console">$ make</code></pre>')
        self.assertEqual(self.run_html()[0], 0)

    def test_nested_markup_in_a_block_is_part_of_its_text(self):
        self.site('<span class="hljs-meta">#include</span> &lt;cstdio&gt;\n'
                  "int f() { return 1 &lt; 2 &amp;&amp; 3 &gt; 2; }")
        self.assertEqual(self.run_html()[0], 0)

    def test_every_pre_is_classified(self):
        self.site(self.GOOD)
        bad = {
            "pre without code": '<pre>int bad;</pre>',
            "verse": '<pre class="content">int bad;</pre>',
            "literal": '<div class="literalblock"><div class="content"><pre>x</pre></div></div>',
            "code without a language": '<pre class="highlight"><code>int bad;</code></pre>',
            "code with an unrelated class": '<pre><code class="hljs">int bad;</code></pre>',
            "other language": '<pre><code class="language-python" data-lang="python">x</code></pre>',
            "lang- class not allowed": '<pre><code class="lang-python">x</code></pre>',
            "uppercase output language": '<pre><code class="language-SH2">x</code></pre>',
            "raw html code with lang-cpp": '<pre><code class="lang-cpp">int bad;</code></pre>',
            "standalone code with lang-cpp": '<code class="lang-cpp">int bad;</code>',
            "standalone code with language-python": '<code class="language-python">x</code>',
            "data-lang only": '<pre><code data-lang="cpp">int bad;</code></pre>',
        }
        for what, markup in bad.items():
            with self.subTest(what):
                (self.out / "o.html").write_text(f"<html><body>{markup}</body></html>")
                status, out, _ = self.run_html()
                self.assertEqual(status, 1, out)
                self.assertIn("o.html:1:", out)
                (self.out / "o.html").unlink()

    def test_the_messages_name_the_problem(self):
        for markup, needle in (('<pre>x</pre>', "a <pre> without a <code>"),
                               ('<pre><code>x</code></pre>', "without a language class"),
                               ('<pre><code class="language-yaml">x</code></pre>',
                                "code block in language `yaml`"),
                               ('<pre><code class="lang-cpp">int bad;</code></pre>',
                                "cpp code block `int bad;` does not equal")):
            with self.subTest(markup=markup):
                self.site(self.GOOD)
                (self.out / "o.html").write_text(markup)
                self.assertIn(needle, self.run_html()[1])

    def test_the_allowed_output_languages_are_fine(self):
        self.site(self.GOOD)
        for language in sorted(cde.OUTPUT_LANGUAGES):
            with self.subTest(language=language):
                (self.out / "o.html").write_text(
                    f'<pre class="highlightjs highlight"><code class="language-{language} '
                    f'hljs" data-lang="{language}">anything</code></pre>')
                self.assertEqual(self.run_html()[0], 0)

    def test_a_backtick_in_the_text_is_broken_markup(self):
        self.site(self.GOOD)
        for markup in ("<p>Use `--c20` and `--c23` here.</p>",
                       "<p>Use <code>--c20+` and `+--c23</code> here.</p>",
                       "<p>A <code>+x+`</code></p>", "<h1>A ` title</h1>"):
            with self.subTest(markup=markup):
                (self.out / "o.html").write_text(markup)
                status, out, _ = self.run_html()
                self.assertEqual(status, 1, out)
                self.assertIn("a literal backtick in the text", out)
        # In a <pre> or a <script>/<style> it is just text.
        (self.out / "o.html").write_text(
            '<pre><code class="language-sh">echo `date`</code></pre>'
            "<script>var s = `template ${x}`;</script><style>a::after{content:'`'}</style>")
        self.assertEqual(self.run_html()[0], 0)

    def test_misnested_inline_elements_are_broken_markup(self):
        # What a doubled formatting mark spanning two code spans renders to.
        self.site(self.GOOD)
        for markup, needle in (
                ("<p>Use <code><em>algorithm</code> and <code></em>new/size.h</code> here.</p>",
                 "</code> closes while <em>"),
                ("<p>A <strong>bold and <code>x</strong>y</code></p>", "</strong> closes while <code>"),
                ("<p>An <em>unclosed emphasis</p>", "<em> (line 1) is still open at </p>"),
                ("<li>a <code>b</li>", "<code> (line 1) is still open at </li>")):
            with self.subTest(markup=markup):
                (self.out / "o.html").write_text(markup)
                status, out, _ = self.run_html()
                self.assertEqual(status, 1, out)
                self.assertIn(needle, out)
        (self.out / "o.html").write_text(
            "<p>Fine <em>a <code>b</code> c</em> and <strong><em>x</em></strong> "
            "<a href='x'><code>y</code></a><br>z<img src='i'></p>")
        self.assertEqual(self.run_html()[0], 0)

    def test_the_real_site_has_no_chrome_pre_or_backticks(self):
        # Nothing is exempted silently: the built pages (404 and nav included)
        # have no <pre> outside page content, and the check passes on them.
        build = REAL_ROOT / "build" / "doc"
        if not build.is_dir():
            self.skipTest("build/doc missing; run scripts/build_docs.sh")
        for page in ("404.html", "index.html"):
            if (build / page).is_file():
                scanner = cde.SiteScanner()
                scanner.feed((build / page).read_text())
                self.assertEqual(scanner.blocks, [], page)
                self.assertEqual(scanner.problems, [], page)


    def test_no_blocks_at_all_fails_when_the_sources_have_blocks(self):
        # E.g. a UI change renames the class: the check must not pass vacuously.
        self.site(self.GOOD, template=HTML_PAGE.replace("language-cpp", "lang-x").replace(
            'data-lang="cpp"', 'data-x="cpp"'))
        status, out, _ = self.run_html()
        self.assertEqual(status, 1, out)
        self.assertIn("found 0 code blocks in a cerastes language but the pages' sources have 1",
                      out)

    def test_fewer_blocks_than_the_sources_declare_fails(self):
        self.page("q.adoc", block_of("include::example$a.cpp[tag=two]"))
        self.site(self.GOOD)
        status, out, _ = self.run_html()
        self.assertEqual(status, 1, out)
        self.assertIn("found 1 code blocks", out)
        self.assertIn("sources have 2", out)

    def test_an_include_without_a_block_still_needs_one_block(self):
        self.page("p.adoc", page_with("include::example$a.cpp[tag=one]"))
        self.site("x", template="<html><body><pre><code>x</code></pre></body></html>")
        status, out, _ = self.run_html()
        self.assertEqual(status, 1, out)
        self.assertIn("found 0 code blocks", out)

    def test_no_html_is_a_usage_error(self):
        (self.out / "notes.txt").write_text("x")
        status, _, err = self.run_html()
        self.assertEqual(status, 2)
        self.assertIn("has no HTML files", err)
        shutil.rmtree(self.out)
        self.assertEqual(self.run_html()[0], 2)

    def test_region_texts(self):
        texts = cde.region_texts(HTML_EXAMPLE)
        self.assertEqual(texts, {
            "#include <cstdio>\nint f() { return 1 < 2 && 3 > 2; }",
            "int g() { return 2; }",
            "#include <cstdio>\nint f() { return 1 < 2 && 3 > 2; }\nint g() { return 2; }"})
        nested = ("// tag::a[]\nx\n// tag::b[]\ny\n// end::b[]\nz\n// end::a[]\n")
        self.assertEqual(cde.region_texts(nested), {"x\ny\nz", "y"})
        self.assertEqual(cde.region_texts("no tags\n"), set())

    def test_normalize_code(self):
        self.assertEqual(cde.normalize_code("\n\n    a  \n      b\n\n"), "a\n  b")
        self.assertNotEqual(cde.normalize_code("a b"), cde.normalize_code("a  b"))


def active_lines(text, comment="#"):
    """The lines of a script or workflow without comments (whole-line and
    trailing ` #` ones)."""
    out = []
    for line in text.split("\n"):
        if line.lstrip().startswith(comment):
            continue
        out.append(re.split(r"\s#\s", line)[0].rstrip())
    return out


EVASIONS = ("||", "; true", ";true", "set +e", "continue-on-error", "if:")


def check_sh_problems(text):
    """What is wrong with how scripts/check.sh runs the doc lint and the docs
    build: both lint calls and the build must be active (not commented out),
    in order, with no way to ignore a failure."""
    lines = active_lines(text)
    problems = []
    if not any(re.match(r"^set -[A-Za-z]*e", l) for l in lines):
        problems.append("check.sh does not `set -e`")
    for l in lines:
        if "set +e" in l:
            problems.append(f"`set +e`: {l.strip()}")
    def index(pattern, what):
        for i, l in enumerate(lines):
            if re.search(pattern, l):
                for bad in ("||", "; true", ";true", "&"):
                    if bad in l.replace("&&", ""):
                        problems.append(f"{what} can fail silently (`{bad}`): {l.strip()}")
                return i
        problems.append(f"no active line runs {what}")
        return None
    lint = index(r"check_doc_examples\.py\s*$", "the doc lint")
    build = index(r"scripts/build_docs\.sh\s*$", "the docs build")
    html = index(r"check_doc_examples\.py\s+--html\s+build/doc\s*$", "the --html check")
    if None not in (lint, build, html) and not lint < build < html:
        problems.append("order must be: doc lint, docs build, --html check")
    return problems


def workflow_problems(text):
    """The same for .github/workflows/docs.yml: its build job runs the plain
    lint, then the build, then `--html build/doc`, as plain steps."""
    job = re.search(r"^  build:\n(.*?)(?=^  \w[\w-]*:\n|\Z)", text, re.S | re.M)
    if not job:
        return ["no `build` job"]
    lines = active_lines(job.group(1))
    steps, cur = [], None
    for line in lines:
        if re.match(r"^\s+- (?:name|uses|run):", line):
            cur = []
            steps.append(cur)
        if cur is not None:
            cur.append(line)
    found = {}
    problems = []
    for i, step in enumerate(steps):
        body = "\n".join(step)
        runs = re.search(r"^\s+(?:- )?run:\s*(.*)$", body, re.M)
        run = runs.group(1) if runs else ""
        if runs and run in ("|", ">", "|-", ">-"):
            run = "\n".join(l.strip() for l in step[step.index(runs.group(0)) + 1:])
        for key, pattern in (("lint", r"check_doc_examples\.py\s*$"),
                             ("build", r"scripts/build_docs\.sh\s*$"),
                             ("html", r"check_doc_examples\.py\s+--html\s+build/doc\s*$")):
            if re.search(pattern, run, re.M):
                found[key] = i
                for bad in EVASIONS:
                    if bad in body.replace("&&", ""):
                        problems.append(f"the {key} step can be ignored (`{bad}`)")
    for key in ("lint", "build", "html"):
        if key not in found:
            problems.append(f"the build job has no active step that runs the {key}")
    if len(found) == 3 and not found["lint"] < found["build"] < found["html"]:
        problems.append("order must be: doc lint, docs build, --html check")
    return problems


class WiringCheckTests(unittest.TestCase):
    """The checks of check.sh and docs.yml fail on every way to switch the
    doc checks off (each case is a mutation of a correct file)."""

    SH = """\
#!/usr/bin/env bash
set -euo pipefail
python3 scripts/check_doc_examples.py
step "docs"
scripts/build_docs.sh
python3 scripts/check_doc_examples.py --html build/doc
printf 'done\\n'
"""
    YML = """\
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - name: Lint the examples
        run: python3 scripts/check_doc_examples.py
      - name: Build
        run: scripts/build_docs.sh
      - name: Check the built examples
        run: |
          python3 scripts/check_doc_examples.py --html build/doc
      - name: Upload
        uses: actions/upload-pages-artifact@v5
  deploy:
    needs: build
    runs-on: ubuntu-latest
    steps:
      - uses: actions/deploy-pages@v5
"""

    def test_the_fixtures_are_fine(self):
        self.assertEqual(check_sh_problems(self.SH), [])
        self.assertEqual(workflow_problems(self.YML), [])

    def test_check_sh_mutations_are_found(self):
        lint = "python3 scripts/check_doc_examples.py\n"
        html = "python3 scripts/check_doc_examples.py --html build/doc\n"
        mutations = {
            "lint commented out": self.SH.replace(lint, "# " + lint, 1),
            "lint commented with indentation": self.SH.replace(lint, "  # " + lint, 1),
            "lint removed": self.SH.replace(lint, "", 1),
            "html commented out": self.SH.replace(html, "# " + html),
            "html removed": self.SH.replace(html, ""),
            "build commented out": self.SH.replace("scripts/build_docs.sh\n",
                                                   "# scripts/build_docs.sh\n"),
            "lint || true": self.SH.replace(lint, lint[:-1] + " || true\n", 1),
            "html || true": self.SH.replace(html, html[:-1] + " || true\n"),
            "build || true": self.SH.replace("scripts/build_docs.sh", "scripts/build_docs.sh || true"),
            "html ; true": self.SH.replace(html, html[:-1] + "; true\n"),
            "html in the background": self.SH.replace(html, html[:-1] + " &\n"),
            "set +e": self.SH.replace("step", "set +e\nstep", 1),
            "no set -e": self.SH.replace("set -euo pipefail", "set -uo pipefail"),
            "set -e only in a comment": self.SH.replace("set -euo pipefail", "# set -e"),
            "html before build": self.SH.replace(html, "").replace(
                "scripts/build_docs.sh\n", html + "scripts/build_docs.sh\n"),
            "html wrong directory": self.SH.replace("--html build/doc", "--html other"),
            "html without the flag": self.SH.replace("--html build/doc", ""),
        }
        for what, text in mutations.items():
            with self.subTest(what):
                self.assertNotEqual(text, self.SH, "mutation did not apply")
                self.assertTrue(check_sh_problems(text), what)

    def test_workflow_mutations_are_found(self):
        mutations = {
            "lint commented out": self.YML.replace(
                "        run: python3 scripts/check_doc_examples.py\n",
                "        # run: python3 scripts/check_doc_examples.py\n"),
            "html commented out": self.YML.replace(
                "          python3 scripts/check_doc_examples.py --html build/doc\n",
                "          # python3 scripts/check_doc_examples.py --html build/doc\n"),
            "build commented out": self.YML.replace("        run: scripts/build_docs.sh\n",
                                                    "        # run: scripts/build_docs.sh\n"),
            "html continue-on-error": self.YML.replace(
                "      - name: Check the built examples\n",
                "      - name: Check the built examples\n        continue-on-error: true\n"),
            "lint continue-on-error": self.YML.replace(
                "      - name: Lint the examples\n",
                "      - name: Lint the examples\n        continue-on-error: true\n"),
            "build || true": self.YML.replace("scripts/build_docs.sh", "scripts/build_docs.sh || true"),
            "html || true": self.YML.replace("--html build/doc", "--html build/doc || true"),
            "html with an if": self.YML.replace(
                "      - name: Check the built examples\n",
                "      - name: Check the built examples\n        if: false\n"),
            "html in the deploy job only": self.YML.replace(
                "      - name: Check the built examples\n        run: |\n"
                "          python3 scripts/check_doc_examples.py --html build/doc\n", "").replace(
                "      - uses: actions/deploy-pages@v5\n",
                "      - run: python3 scripts/check_doc_examples.py --html build/doc\n"),
            "html before build": self.YML.replace(
                "      - name: Build\n        run: scripts/build_docs.sh\n", "").replace(
                "      - name: Upload\n",
                "      - name: Build\n        run: scripts/build_docs.sh\n      - name: Upload\n"),
            "html wrong directory": self.YML.replace("--html build/doc", "--html other"),
        }
        for what, text in mutations.items():
            with self.subTest(what):
                self.assertNotEqual(text, self.YML, "mutation did not apply")
                self.assertTrue(workflow_problems(text), what)

    def test_no_build_job_is_a_problem(self):
        self.assertEqual(workflow_problems("jobs:\n  other:\n    steps: []\n"),
                         ["no `build` job"])


class RealDocTreeTests(unittest.TestCase):
    def test_the_real_documentation_passes_the_lint(self):
        report = cde.check_tree(REAL_ROOT)
        self.assertEqual([str(p) for p in report.problems], [])

    def test_the_real_lint_is_not_vacuous(self):
        component = REAL_ROOT / "doc" / "modules" / "ROOT"
        self.assertTrue((component / "pages" / "installation.adoc").is_file())
        report = cde.check_tree(REAL_ROOT)
        self.assertGreaterEqual(report.pages, 2)
        self.assertGreaterEqual(report.examples, 1)
        self.assertGreaterEqual(report.includes, 1)
        self.assertGreaterEqual(report.blocks, 1)

    def test_the_command_line_passes_on_the_real_tree(self):
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            self.assertEqual(cde.main([]), 0, out.getvalue())

    def test_check_sh_runs_the_lint_the_build_and_the_html_check(self):
        text = (REAL_ROOT / "scripts" / "check.sh").read_text()
        self.assertEqual(check_sh_problems(text), [])

    def test_the_docs_workflow_runs_the_lint_the_build_and_the_html_check(self):
        text = (REAL_ROOT / ".github" / "workflows" / "docs.yml").read_text()
        self.assertEqual(workflow_problems(text), [])

    def test_the_lint_reuses_the_harness_header_rules(self):
        # A copy of the header grammar would drift from run_tests.py.
        self.assertIs(cde.rt, rt)
        source = Path(cde.__file__).read_text()
        self.assertIn("rt.read_header", source)
        self.assertNotIn("def parse_header", source)


class E2EGateTests(unittest.TestCase):
    """The end-to-end classes must fail, not skip, under CERASTES_REQUIRE_E2E=1."""

    @staticmethod
    def run_gated(available, require):
        @e2e_gate(available, "front end missing", require=require)
        class Gated(unittest.TestCase):
            def test_it(self):
                pass

        result = unittest.TestResult()
        unittest.defaultTestLoader.loadTestsFromTestCase(Gated).run(result)
        return result

    def test_missing_prerequisites_skip_by_default(self):
        result = self.run_gated(False, False)
        self.assertEqual((len(result.skipped), len(result.errors), len(result.failures)), (1, 0, 0))
        self.assertTrue(result.wasSuccessful())

    def test_missing_prerequisites_fail_when_required(self):
        result = self.run_gated(False, True)
        self.assertFalse(result.wasSuccessful())
        self.assertEqual(result.skipped, [])
        self.assertEqual(len(result.errors) + len(result.failures), 1)
        self.assertIn("CERASTES_REQUIRE_E2E=1", (result.errors + result.failures)[0][1])
        self.assertIn("front end missing", (result.errors + result.failures)[0][1])

    def test_available_prerequisites_run_the_tests(self):
        for require in (False, True):
            with self.subTest(require=require):
                result = self.run_gated(True, require)
                self.assertEqual(result.testsRun, 1)
                self.assertTrue(result.wasSuccessful())
                self.assertEqual(result.skipped, [])

    def test_the_variable(self):
        self.assertTrue(e2e_required({"CERASTES_REQUIRE_E2E": "1"}))
        for value in ("", "0", "true", "yes", " 1"):
            self.assertFalse(e2e_required({"CERASTES_REQUIRE_E2E": value}), value)
        self.assertFalse(e2e_required({}))
        with mock.patch.dict(os.environ, {"CERASTES_REQUIRE_E2E": "1"}):
            self.assertTrue(e2e_required())
        with mock.patch.dict(os.environ, {"CERASTES_REQUIRE_E2E": "0"}):
            self.assertFalse(e2e_required())

    def test_the_variable_decides_when_require_is_not_given(self):
        for value, errors in (("1", 1), ("0", 0)):
            with self.subTest(value=value), mock.patch.dict(
                    os.environ, {"CERASTES_REQUIRE_E2E": value}):
                @e2e_gate(False, "missing")
                class Gated(unittest.TestCase):
                    def test_it(self):
                        pass

                result = unittest.TestResult()
                unittest.defaultTestLoader.loadTestsFromTestCase(Gated).run(result)
                self.assertEqual(len(result.errors) + len(result.failures), errors)

    def test_no_end_to_end_class_can_still_skip_silently(self):
        source = Path(__file__).read_text()
        # A plain skip decorator would skip even under CERASTES_REQUIRE_E2E=1.
        self.assertNotIn("unittest." + "skipUnless", source)
        for name in ("FakeGoldenRelativePathTests", "EndToEndTests",
                     "RealFrontEndGoldenTests", "WalkerSelfTestHookTests",
                     "DocBuildWarningsTests"):
            self.assertRegex(source, rf"@e2e_gate\([^@]*?\)\nclass {name}\b", name)


class ScriptsNeverUpdateGoldensTests(unittest.TestCase):
    def test_no_script_or_workflow_updates_goldens(self):
        files = sorted((rt.ROOT / "scripts").glob("*.sh"))
        files += sorted((rt.ROOT / ".github").rglob("*.yml"))
        self.assertIn(rt.ROOT / "scripts" / "check.sh", files)
        for f in files:
            with self.subTest(file=str(f.relative_to(rt.ROOT))):
                self.assertNotIn("--update-golden", f.read_text())


@e2e_gate((rt.ROOT / "build" / "env.sh").exists(),
          "build/env.sh missing; run scripts/bootstrap.sh")
class EndToEndTests(unittest.TestCase):
    """Run the real harness (and Silicon) on mutated copies of fail.vpr."""

    FIXTURE = rt.ROOT / "tests" / "viper-smoke" / "fail.vpr"

    def run_harness(self, header_line):
        text = self.FIXTURE.read_text()
        self.assertTrue(text.startswith("// expect: error postcondition"))
        mutated = re.sub(r"^// expect: .*$", header_line, text, count=1, flags=re.MULTILINE)
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "fail.vpr"
            path.write_text(mutated)
            out = io.StringIO()
            with contextlib.redirect_stdout(out):
                status = rt.main([str(path)])
        return status, out.getvalue()

    def test_fixture_as_written_passes(self):
        status, out = self.run_harness(self.FIXTURE.read_text().splitlines()[0])
        self.assertEqual(status, 0, out)

    def test_wrong_line_fails(self):
        status, out = self.run_harness("// expect: error postcondition @ L11")
        self.assertEqual(status, 1, out)
        self.assertIn("wrong line", out)

    def test_wrong_category_fails(self):
        status, out = self.run_harness("// expect: error assert @ L12")
        self.assertEqual(status, 1, out)
        self.assertIn("wrong reason", out)

    def test_wrong_viper_reason_fails(self):
        status, out = self.run_harness("// expect: error postcondition:insufficient.permission @ L12")
        self.assertEqual(status, 1, out)
        self.assertIn("wrong reason", out)

    def test_expected_verified_fails(self):
        status, out = self.run_harness("// expect: verified")
        self.assertEqual(status, 1, out)


def _real_front_end_available():
    fe = rt.load_env().get("CERASTES_FE")
    return bool(fe) and Path(fe).is_file() and Path(rt.FE_COMMAND[0]).is_file()


@e2e_gate(_real_front_end_available(),
          "CERASTES_FE or scripts/cerastes-fe.sh missing; run scripts/bootstrap.sh")
class RealFrontEndGoldenTests(unittest.TestCase):
    """Golden tests through the real scripts/cerastes-fe.sh.  These depend only
    on the walker printing routine names, not on the rest of its format."""

    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.dir = Path(tmp.name)

    def write(self, name, body):
        path = self.dir / name
        path.write_text("// expect: golden\n" + body)
        return path

    def run_harness(self, *args):
        out = io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(io.StringIO()):
            status = rt.main([str(a) for a in args])
        return status, out.getvalue()

    def test_update_pass_and_a_changed_source_fails(self):
        t = self.write("x.cpp", "int add1(int x) { return x + 1; }\n")
        golden = self.dir / "x.cpp.golden"
        status, out = self.run_harness("--update-golden", t)
        self.assertEqual(status, 0, out)
        self.assertIn("add1", golden.read_text())
        status, out = self.run_harness(t)
        self.assertEqual(status, 0, out)
        t.write_text(t.read_text().replace("add1", "add2"))
        status, out = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("output differs", out)

    def test_warnings_go_into_the_golden(self):
        t = self.write("w.c", '#warning "harness self-test"\nint f(void) { return 0; }\n')
        status, out = self.run_harness("--update-golden", t)
        self.assertEqual(status, 0, out)
        text = (self.dir / "w.c.golden").read_text()
        self.assertIn("\n" + rt.STDERR_MARKER + "\n", text)
        self.assertIn("harness self-test", text.split(rt.STDERR_MARKER)[1])
        self.assertEqual(self.run_harness(t)[0], 0)

    def test_front_end_errors_fail_and_are_never_written(self):
        t = self.write("bad.cpp", "int f( {\n")
        golden = self.dir / "bad.cpp.golden"
        status, out = self.run_harness("--update-golden", t)
        self.assertEqual(status, 1, out)
        self.assertFalse(golden.exists())
        golden.write_text("anything\n")
        status, out = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("cerastes-fe exited with status", out)


@e2e_gate(_real_front_end_available(),
          "CERASTES_FE or scripts/cerastes-fe.sh missing; run scripts/bootstrap.sh")
class WalkerSelfTestHookTests(unittest.TestCase):
    """--cerastes_walk_selftest proves that the walker's two cross-check
    warnings (which the goldens rely on being absent) can fire.  There is no
    expectation kind for this; the hook is exercised here."""

    SOURCE = ("template <typename T>\n"
              "T inc(T v) { return v + 1; }\n"
              "\n"
              "template <typename T>\n"
              "T never(T v) { return v; }\n"
              "\n"
              "int use() { return inc(3); }\n")
    NO_PROTOTYPE = "warning: template {} has no prototype routine"
    NO_ASSOC = "warning: instance {} has no assoc_template"

    def walk(self, *extra):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "hook.cpp"
            path.write_text(self.SOURCE)
            run = rt.run_tool([*rt.FE_COMMAND, "--c++20", rt.WALK_OPTION, *extra, str(path)],
                              rt.load_env(), rt.FRONTEND_TIMEOUT_S)
        self.assertEqual((run.returncode, run.stderr), (0, ""), run)
        return run.stdout

    def test_without_the_hook_no_warning(self):
        out = self.walk()
        self.assertNotIn("warning:", out)
        # The walk itself is not empty: the template and its instance are there.
        self.assertIn("routine inc prototype body @ L2:3\n", out)
        self.assertIn("routine inc instance body @ L2:3\n", out)
        self.assertIn("routine never prototype body @ L5:3\n", out)

    def test_with_the_hook_both_warnings_fire(self):
        lines = self.walk("--cerastes_walk_selftest").splitlines()
        self.assertIn(self.NO_PROTOTYPE.format("inc"), lines)
        self.assertIn(self.NO_PROTOTYPE.format("never"), lines)
        self.assertIn(self.NO_ASSOC.format("inc"), lines)
        # One instance (inc<int>) and two templates, nothing more.
        self.assertEqual(sum(l.startswith("warning:") for l in lines), 3, lines)

    def test_remove_unneeded_entities_is_refused(self):
        # With --remove_unneeded_entities EDG deletes unreferenced entities
        # before back_end(), so the walk would silently omit them.
        for option in ("--remove_unneeded_entities", "--remove_unneeded_ent",
                       "--remove_unneeded_e", "--remove_unneeded"):
            with self.subTest(option=option), tempfile.TemporaryDirectory() as tmp:
                path = Path(tmp) / "t.cpp"
                path.write_text(self.SOURCE)
                run = rt.run_tool([*rt.FE_COMMAND, "--c++20", rt.WALK_OPTION, option, str(path)],
                                  rt.load_env(), rt.FRONTEND_TIMEOUT_S)
                self.assertIsNotNone(run.returncode, run)
                self.assertNotEqual(run.returncode, 0, run)
                self.assertTrue(run.stderr.strip(), run)
                self.assertFalse(rt.walk_finished(run.stdout), run)

    def test_the_hook_changes_only_the_warnings(self):
        plain = self.walk()
        hooked = "".join(l for l in self.walk("--cerastes_walk_selftest").splitlines(keepends=True)
                         if not l.startswith("warning:"))
        self.assertEqual(hooked, plain)


def _docs_build_available():
    needed = [REAL_ROOT / "doc" / "node_modules", REAL_ROOT / "doc" / "local-playbook.yml",
              REAL_ROOT / "scripts" / "build_docs.sh",
              REAL_ROOT / "doc" / "ui" / "supplemental"]
    return all(p.exists() for p in needed) and bool(shutil.which("git") and shutil.which("node"))


@e2e_gate(_docs_build_available(),
          "doc/node_modules, doc/local-playbook.yml, git or node missing; "
          "run scripts/bootstrap.sh")
class DocBuildWarningsTests(unittest.TestCase):
    """A warning must fail scripts/build_docs.sh (the --log-failure-level=warn
    flag), or a broken xref or include would pass check.sh.

    The twin builds a temporary git repository holding a copy of doc/ with a
    page of its own, through a playbook that does not set a failure level (so
    only the flag of build_docs.sh can make a warning fatal): the clean copy
    must build (so the twin is not vacuous), a broken xref must fail the build
    and say why, and so must an include of a tag that does not exist.
    """

    PAGE = "doc/modules/ROOT/pages/e2e-twin.adoc"
    EXAMPLE = "doc/modules/ROOT/examples/e2e/twin.cpp"
    GOOD = ("= E2E twin\n\nSee xref:e2e-twin.adoc[this page].\n\n[source,cpp]\n----\n"
            "include::example$e2e/twin.cpp[tag=body]\n----\n")

    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.tmp = Path(tmp.name).resolve()
        self.repo = self.tmp / "repo"
        real_doc = REAL_ROOT / "doc"

        def ignore(directory, names):
            return {"node_modules", "ui"} & set(names) if Path(directory) == real_doc else ()

        shutil.copytree(real_doc, self.repo / "doc", ignore=ignore)
        self.write(self.EXAMPLE, "// expect: frontend-ok\n// tag::body[]\n"
                                 "int e2e_twin_body() { return 0; }\n// end::body[]\n")
        self.write(self.PAGE, self.GOOD)
        self.git("init", "-q", "-b", "main")
        self.commit("copy of doc/")
        self.playbook = self.tmp / "playbook.yml"
        q = json.dumps
        # No failure level here: only build_docs.sh's flag can make warnings fatal.
        self.playbook.write_text(
            "site:\n  title: twin\n  url: https://example.invalid/twin\n"
            "  start_page: ROOT::e2e-twin.adoc\n"
            f"content:\n  sources:\n    - url: {q(str(self.repo))}\n      start_path: doc\n"
            "      branches: HEAD\n"
            f"ui:\n  bundle:\n    url: {q(str(real_doc / 'ui' / 'antora-ui-default.zip'))}\n"
            f"    snapshot: false\n  supplemental_files: {q(str(real_doc / 'ui' / 'supplemental'))}\n"
            f"output:\n  dir: {q(str(self.tmp / 'out'))}\n  clean: true\n"
            f"runtime:\n  cache_dir: {q(str(self.tmp / 'cache'))}\n  fetch: false\n"
            "antora:\n  extensions:\n    - '@antora/lunr-extension'\n")

    def write(self, rel, text):
        path = self.repo / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)

    def git(self, *args):
        subprocess.run(["git", "-c", "user.name=twin", "-c", "user.email=twin@example.invalid",
                        "-c", "commit.gpgsign=false", *args], cwd=self.repo, check=True,
                       capture_output=True)

    def commit(self, message):
        self.git("add", "-A")
        self.git("commit", "-q", "-m", message)

    def build(self):
        run = subprocess.run([str(REAL_ROOT / "scripts" / "build_docs.sh"), str(self.playbook)],
                             cwd=self.tmp, capture_output=True, text=True, timeout=600)
        return run.returncode, run.stdout + run.stderr

    def test_a_warning_fails_the_build_and_a_clean_copy_builds(self):
        status, out = self.build()
        self.assertEqual(status, 0, out)  # not vacuous: the copy builds
        built = list((self.tmp / "out").rglob("e2e-twin.html"))
        self.assertEqual(len(built), 1, out)
        self.assertIn("e2e_twin_body", built[0].read_text())  # the tagged include worked

        self.write(self.PAGE, self.GOOD + "\nSee xref:no-such-page.adoc[].\n")
        self.commit("broken xref")
        status, out = self.build()
        self.assertNotEqual(status, 0, out)
        self.assertIn("no-such-page", out)

        self.write(self.PAGE, self.GOOD.replace("tag=body", "tag=no-such-tag"))
        self.commit("missing tag")
        status, out = self.build()
        self.assertNotEqual(status, 0, out)
        self.assertIn("no-such-tag", out)

        self.write(self.PAGE, self.GOOD)
        self.commit("fixed")
        status, out = self.build()
        self.assertEqual(status, 0, out)  # the failures were caused by the edits

    def run_check(self, *args):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            status = cde.main(["--root", str(self.repo), *map(str, args)])
        return status, out.getvalue() + err.getvalue()

    def test_code_that_evades_the_lint_fails_the_html_check(self):
        """Pages that the source lint missed (the audit's probes P1-P7), built
        for real: Antora accepts every one of them, and the built site shows a
        cpp code block that no test runs.  `--html` must fail each.  The
        source lint is expected to catch them too now (second assertion), but
        the html check must not depend on it."""
        status, out = self.build()
        self.assertEqual(status, 0, out)
        site = self.tmp / "out"
        status, out = self.run_check("--html", site)
        self.assertEqual(status, 0, out)  # not vacuous: the clean copy passes
        self.assertIn("2 code blocks", out)  # ... and has blocks to look at
        self.assertEqual(self.run_check()[0], 0)
        base = subprocess.run(["git", "rev-parse", "HEAD"], cwd=self.repo, check=True,
                              capture_output=True, text=True).stdout.strip()
        bad = "int evading_code() { return 1/0; }"
        block = "\n[source,cpp]\n----\n{}\n----\n"
        probes = {
            "P1 partial in a cpp block": (
                {"doc/modules/ROOT/partials/snippet.cpp": bad + "\n",
                 self.PAGE: self.GOOD + block.format("include::partial$snippet.cpp[]")},
                "include only example$"),
            "P2 non-test file in a cpp block": (
                {"doc/modules/ROOT/examples/bad.txt": f"// tag::x[]\n{bad}\n// end::x[]\n",
                 self.PAGE: self.GOOD + block.format("include::example$bad.txt[tag=x]")},
                "may include only example$"),
            "P3 adoc fragment in examples": (
                {"doc/modules/ROOT/examples/frag.adoc":
                     f"// tag::x[]\n[source,cpp]\n----\n{bad}\n----\n// end::x[]\n",
                 self.PAGE: self.GOOD + "\ninclude::example$frag.adoc[tag=x]\n"},
                "not a test or a golden file"),
            "P4 source block in a table cell": (
                {self.PAGE: self.GOOD + f"\n|===\na|[source,cpp]\n----\n{bad}\n----\n|===\n"},
                "inline cpp source"),
            "P5 page in another module": (
                {"doc/modules/extra/pages/x.adoc": "= Extra\n" + block.format(bad)},
                "module other than ROOT"),
            "P6 component-wide source-language": (
                {"doc/antora.yml": (REAL_ROOT / "doc" / "antora.yml").read_text()
                 + "asciidoc:\n  attributes:\n    source-language: cpp\n",
                 self.PAGE: self.GOOD + f"\n----\n{bad}\n----\n"},
                "`source-language` must not be set"),
            "P7 partial holding a cpp block": (
                {"doc/modules/ROOT/partials/frag.adoc": block.format(bad).lstrip(),
                 self.PAGE: self.GOOD + "\ninclude::partial$frag.adoc[]\n"},
                "inline cpp source"),
            "P9 inline cpp block": (
                {self.PAGE: self.GOOD + block.format(bad)},
                "inline cpp source"),
        }
        for what, (files, lint_needle) in probes.items():
            self.probe(what, files, "does not equal any tagged region", lint_needle,
                       2 if lint_needle.startswith("module") else 1, site, base)
        status, out = self.build()
        self.assertEqual(status, 0, out)
        self.assertEqual(self.run_check("--html", site)[0], 0)  # back to clean

    def probe(self, what, files, html_needle, lint_needle, lint_status, site, base):
        """Build a page that the lint once missed (Antora must accept it), then
        require --html to fail (with `html_needle`, if given) and the source
        lint to fail with `lint_needle`."""
        with self.subTest(what):
            try:
                for rel, text in files.items():
                    self.write(rel, text)
                self.commit(what)
                status, out = self.build()
                self.assertEqual(status, 0, (what, out))  # Antora accepts the page
                status, out = self.run_check("--html", site)
                if html_needle is None:
                    self.assertEqual(status, 0, (what, out))  # only the source lint sees it
                else:
                    self.assertEqual(status, 1, (what, out))
                    self.assertIn(html_needle, out)
                status, out = self.run_check()
                self.assertEqual(status, lint_status, (what, out))
                self.assertIn(lint_needle, out)
            finally:
                self.git("reset", "-q", "--hard", base)
                self.git("clean", "-fdq")

    def test_code_without_a_language_class_and_broken_markup_fail(self):
        """Audit probes N1-N3: code that Asciidoctor renders as `<pre>` without a
        language class (or as raw HTML), and markup that garbles the page
        silently.  Each page is built for real."""
        status, out = self.build()
        self.assertEqual(status, 0, out)
        site = self.tmp / "out"
        base = subprocess.run(["git", "rev-parse", "HEAD"], cwd=self.repo, check=True,
                              capture_output=True, text=True).stdout.strip()
        code = "int evading() { return 1/0; }"
        no_code = "a <pre> without a <code>"
        probes = {  # name: (text appended to the page, html needle, lint needle)
            "q1 ++++ with a pre": (f"\n++++\n<pre>{code}</pre>\n++++\n", no_code,
                                   "passthrough blocks"),
            "q2 inline pass with a pre": (f"\npass:[<pre>{code}</pre>]\n", no_code,
                                          "pass:[...] with markup"),
            "q3 l| cell": (f"\n|===\nl|{code}\n|===\n", no_code, "table cell styles l and m"),
            "q4 cols with l": (f'\n[cols="1l"]\n|===\n|{code}\n|===\n', no_code,
                               "column styles l and m"),
            "q5 literal after a list continuation": (f"\n* item\n+\n  {code}\n", no_code,
                                                     "indented paragraph is a literal block"),
            "q6 literal in an a| cell": (f"\n|===\na|\n\n  {code}\n|===\n", no_code,
                                         "indented paragraph is a literal block"),
            "q7 literal after a description list continuation": (
                f"\nterm::\n+\n  {code}\n", no_code, "indented paragraph is a literal block"),
            "q8 verse": (f"\n[verse]\n{code}\n", no_code, "the [verse] style is not allowed"),
            "q9 ++++ with a lang-cpp code": (
                f'\n++++\n<pre><code class="lang-cpp">{code}</code></pre>\n++++\n',
                "cpp code block", "passthrough blocks"),
            "q10 pass style": (f"\n[pass]\n<pre>{code}</pre>\n", no_code,
                               "the [pass] style is not allowed"),
            "q11 inline +++ passthrough": (f"\nA +++<pre>{code}</pre>+++ b.\n", no_code,
                                           "write it inside pass:[...]"),
            "q12 literal block": (f"\n....\n{code}\n....\n", no_code,
                                  "literal blocks (....) are not allowed"),
            "q13 undeclared listing": (f"\n----\n{code}\n----\n", no_code,
                                       "listing block without a declared language"),
            "q14 other language": (f"\n[source,python]\n----\n{code}\n----\n",
                                   "code block in language `python`",
                                   "language `python` is not allowed"),
            "q15 two `--c++20` spans": ("\nUse `--c++20` and `--c++23` here.\n",
                                        "a literal backtick in the text", "write it inside pass:"),
            "q16 two `__` spans": ("\nUse `__algorithm` and `__new/size.h` here.\n",
                                   "</code> closes while <em>", "doubled formatting mark"),
            # well-formed HTML (`<strong>bold and </strong>more**`): only the lint sees it
            "q17 two `**` marks": ("\nA **bold and **more** here.\n", None,
                                   "doubled formatting mark"),
            "e1 subs=none paragraph with raw html": (
                f'\n[subs=none]\n<div style="white-space:pre">{code}</div>\n', None,
                "`subs=` is not allowed"),
            "e2 subs=none closes the code and reopens": (
                '\n[source,text,subs=none]\n----\n</code></pre>'
                f'<div style="white-space:pre">{code}</div><pre><code>\n----\n',
                "a code block without a language class", "`subs=` is not allowed"),
            "e5 subs=-specialchars with a pre": (
                f"\n[subs=-specialchars]\n<pre>{code}</pre>\n", no_code,
                "`subs=` is not allowed"),
            "q18 libc++ twice (only the lint can see it)": (
                "\nThe libc++ library and libc++ again.\n", None, "write it inside pass:"),
        }
        for what, (text, html_needle, lint_needle) in probes.items():
            self.probe(what, {self.PAGE: self.GOOD + text}, html_needle, lint_needle, 1, site,
                       base)

    def test_a_single_formatting_mark_or_plusplus_renders_fine(self):
        """The reality that the rules are matched to (a real build): one `__` or
        `++` in a paragraph is left alone, two garble it, `+...+` protects the
        marks `__`, `**`, `##` but not `++`, and pass:[...] protects all."""
        cases = {
            "Use the `__algorithm` header.": "<code>__algorithm</code>",
            "The libc++ library.": "The libc++ library.",
            "Use `--c++20` here.": "<code>--c++20</code>",
            "Use `__algorithm` and `__new/size.h` here.": "<code><em>algorithm</code>",
            "The libc++ library and libc++ again.": "The libc library and libc again.",
            "Use `+__algorithm+` and `+__new/size.h+` here.":
                "<code>__algorithm</code> and <code>__new/size.h</code>",
            "Use `+--c++20+` and `+--c++23+` here.": "<code>--c20+` and `+--c23</code>",
            "Use `pass:[--c++20]` and `pass:[--c++23]` here.":
                "<code>--c++20</code> and <code>--c++23</code>",
            "Use `__algorithm` here.\n\nUse `__new` there.": "<code>__new</code>",
            "* an item with `__algorithm`\n* another item with `__new/size.h`":
                "<code>__new/size.h</code>",
            "term one:: the `__algorithm` header\nterm two:: the `__new` header":
                "<code>__new</code> header",
            "* one item with `__algorithm`\nand `__new/size.h` on a continuation line":
                "<code><em>algorithm</code>",
        }
        for text, rendered in cases.items():
            with self.subTest(text=text):
                self.write(self.PAGE, "= E2E\n\n" + text + "\n")
                self.commit(text)
                status, out = self.build()
                self.assertEqual(status, 0, out)
                (page,) = (self.tmp / "out").rglob("e2e-twin.html")
                self.assertIn(rendered, page.read_text())
if __name__ == "__main__":
    unittest.main()
