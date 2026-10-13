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
The golden-test cases run the real harness against a fake front end (a small
Python script standing in for scripts/cerastes-fe.sh), so they always run;
a few more run the real cerastes front end when CERASTES_FE is available.
Those end-to-end classes are skipped when their prerequisites are missing,
unless CERASTES_REQUIRE_E2E=1 (scripts/check.sh sets it), in which case they
fail: a missing front end must not silently turn them off.
"""

import contextlib
import io
import os
import re
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
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
                     "RealFrontEndGoldenTests", "WalkerSelfTestHookTests"):
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


if __name__ == "__main__":
    unittest.main()
