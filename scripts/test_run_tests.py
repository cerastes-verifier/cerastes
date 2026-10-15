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
import hashlib
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


def scrub_inherited_environment(environ=None, env_sh=None):
    """Remove from the process environment every variable that build/env.sh
    defines, and CERASTES_EDG_SRC, so that the self-test does not depend on the
    shell that runs it (`. build/env.sh` is what CLAUDE.md tells developers to
    do).  Tests that need such a variable set it explicitly (mock.patch.dict,
    set_env); the tests of the real tools read the values from build/env.sh with
    rt.load_env().  CERASTES_REQUIRE_E2E is the caller's switch and stays.
    Returns the names removed."""
    environ = os.environ if environ is None else environ
    env_sh = Path(env_sh) if env_sh is not None else REAL_ROOT / "build" / "env.sh"
    names = {"CERASTES_EDG_SRC"}
    if env_sh.exists():
        names |= set(re.findall(r"^export (\w+)=", env_sh.read_text(), re.MULTILINE))
    names.discard("CERASTES_REQUIRE_E2E")
    removed = sorted(n for n in names if n in environ)
    for name in removed:
        del environ[name]
    return removed


scrub_inherited_environment()

_REAL_EDG_STAMP_PROBLEM = rt.edg_stamp_problem  # before any test patches it
_real_stamp_memo: list = []


def memoised_real_stamp_problem(*args, **kwargs):
    """rt.edg_stamp_problem for the real tree, computed once per process.  It
    takes ~2.5 s (git status of EDG's 110,000 files), and every real end-to-end
    harness run calls it; the tree does not change during the self-test.  Calls
    with arguments (the fixture trees) are never memoised."""
    if args or kwargs:
        return _REAL_EDG_STAMP_PROBLEM(*args, **kwargs)
    if not _real_stamp_memo:
        _real_stamp_memo.append(_REAL_EDG_STAMP_PROBLEM())
    return _real_stamp_memo[0]


class MemoisedStamp:
    """Mixin for the end-to-end classes that run rt.main on the real tree: the
    stamp check is memoised for the class (see memoised_real_stamp_problem).
    TreeScriptTwinTests and TreeGateTests keep exercising the real function."""

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        patcher = mock.patch.object(rt, "edg_stamp_problem", memoised_real_stamp_problem)
        patcher.start()
        cls.addClassCleanup(patcher.stop)


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
# prints <text> after the end line; `// crash` kills the process with SIGKILL;
# `// sleep` sleeps for 30 s (for timeout tests).
FAKE_FE = '''\
import os, sys
ROOT = {root!r}
args = sys.argv[1:]
with open(args[-1]) as f:  # relative paths are relative to the root
    lines = f.read().splitlines(keepends=True)
code = 0
if "// crash\\n" in lines:
    os.kill(os.getpid(), 9)
if "// sleep\\n" in lines:
    import time
    time.sleep(30)
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
        # The tree-consistency check reads the real build/env.sh and the shell's
        # CERASTES_EDG_SRC; here it is out of the picture (TreeConsistencyTests
        # cover it), but a test can make it fail through `self.tree_mock`.
        tree_patch = mock.patch.object(rt, "edg_tree_problem", return_value=None)
        stamp_patch = mock.patch.object(rt, "edg_stamp_problem", return_value=None)
        # build/env.sh is authoritative for the variables it defines, so the
        # fake front end and the other fixtures are named in a fixture env file
        # (`set_env`), not in the environment of the process.
        self.env_file = base / "env.sh"
        self.env_values = {"CERASTES_FE": str(fake), "EDG_CLANG_VER_SCRAPE": "0",
                           "CPFE": "/nonexistent/cpfe"}
        self.set_env()
        for patcher in (mock.patch.object(rt, "FE_COMMAND", [sys.executable, str(fake)]),
                        mock.patch.object(rt, "ENV_SH", self.env_file),
                        tree_patch, stamp_patch):
            patcher.start()
            self.addCleanup(patcher.stop)
        self.tree_mock = rt.edg_tree_problem
        self.stamp_mock = rt.edg_stamp_problem

    def set_env(self, **values):
        """Define (or change) variables of the fixture build/env.sh."""
        self.env_values.update(values)
        self.env_file.write_text("".join(f'export {k}="{v}"\n'
                                         for k, v in self.env_values.items()))

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
    # decision 0005: the EDG tree is checked before anything is built, and the
    # binary under test is verified against the tree after the tests ran
    edg = index(r"^\s*python3 scripts/edg_tree\.py check\s*$", "the EDG tree guard")
    boot = index(r"^\s*scripts/bootstrap\.sh --build-only\s*$", "the build step")
    tests = index(r"^\s*python3 scripts/run_tests\.py\s*$", "the test run")
    verify = index(r"^\s*python3 scripts/edg_tree\.py describe --verify-build\s*$",
                   "the build verification")
    green = index(r"all green", "the final green line")
    if None not in (edg, boot) and not edg < boot:
        problems.append("the EDG tree guard must run before the build")
    if None not in (tests, verify, green) and not tests < verify < green:
        problems.append("order must be: tests, build verification, final green line")
    elif None not in (verify, green) and not verify < green:
        problems.append("the build verification must run before the final green line")
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
step "edg tree"
python3 scripts/edg_tree.py check
step "build"
scripts/bootstrap.sh --build-only
python3 scripts/check_doc_examples.py
step "tests"
python3 scripts/run_tests.py
step "docs"
scripts/build_docs.sh
python3 scripts/check_doc_examples.py --html build/doc
python3 scripts/edg_tree.py describe --verify-build
printf '\\ncheck.sh: all green\\n'
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

    def test_edg_tree_wiring_mutations_are_found(self):
        guard = "python3 scripts/edg_tree.py check\n"
        verify = "python3 scripts/edg_tree.py describe --verify-build\n"
        green = "printf '\\ncheck.sh: all green\\n'\n"
        boot = "scripts/bootstrap.sh --build-only\n"
        tests = "python3 scripts/run_tests.py\n"
        mutations = {
            "guard commented out": self.SH.replace(guard, "# " + guard),
            "guard removed": self.SH.replace(guard, ""),
            "guard || true": self.SH.replace(guard, guard[:-1] + " || true\n"),
            "guard ; true": self.SH.replace(guard, guard[:-1] + "; true\n"),
            "guard in the background": self.SH.replace(guard, guard[:-1] + " &\n"),
            "guard without the subcommand": self.SH.replace(guard, "python3 scripts/edg_tree.py\n"),
            "guard describes instead": self.SH.replace(guard, "python3 scripts/edg_tree.py describe\n"),
            "guard after the build": self.SH.replace(guard, "").replace(boot, boot + guard),
            "build commented out": self.SH.replace(boot, "# " + boot),
            "build without --build-only": self.SH.replace(boot, "scripts/bootstrap.sh\n"),
            "verify commented out": self.SH.replace(verify, "# " + verify),
            "verify removed": self.SH.replace(verify, ""),
            "verify || true": self.SH.replace(verify, verify[:-1] + " || true\n"),
            "verify ; true": self.SH.replace(verify, verify[:-1] + "; true\n"),
            "verify in the background": self.SH.replace(verify, verify[:-1] + " &\n"),
            "verify without the flag": self.SH.replace(verify, "python3 scripts/edg_tree.py describe\n"),
            "verify before the tests": self.SH.replace(verify, "").replace(tests, verify + tests),
            "verify after the green line": self.SH.replace(verify, "").replace(green, green + verify),
            "no final green line": self.SH.replace(green, "printf 'done\\n'\n"),
            "tests removed": self.SH.replace(tests, ""),
            "tests commented out": self.SH.replace(tests, "# " + tests),
            "tests with a path": self.SH.replace(tests, "python3 scripts/run_tests.py tests/il\n"),
        }
        for what, text in mutations.items():
            with self.subTest(what):
                self.assertNotEqual(text, self.SH, "mutation did not apply")
                self.assertTrue(check_sh_problems(text), what)

    def test_the_edg_tree_lines_sit_between_the_right_neighbours(self):
        lines = [l for l in active_lines(self.SH) if l.strip()]
        guard = lines.index("python3 scripts/edg_tree.py check")
        self.assertLess(guard, lines.index("scripts/bootstrap.sh --build-only"))
        self.assertLess(lines.index("python3 scripts/run_tests.py"),
                        lines.index("python3 scripts/edg_tree.py describe --verify-build"))
        self.assertLess(lines.index("python3 scripts/edg_tree.py describe --verify-build"),
                        len(lines) - 1)

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


GOLDEN_EXAMPLE = ("// expect: golden\n// tag::body[]\nint f(int x) { return x + 1; }\n"
                  "// end::body[]\n")
GOLDEN_TEXT = ("routine f plain body @ L3:5\n  param 1 x: int\n  returns int\n"
               "end walk: 1 routines reported, 0 routines with bodies in non-system files "
               "outside the primary file\n")
LISTING_PAGE = page_with("[source,cpp]", "----", "include::example$a.cpp[tag=body]", "----",
                         "", "[source,text]", "----", "include::example$a.cpp.golden[]", "----")


class GoldenListingLintTests(DocLintCase):
    """Rule g: a walker listing on a page is the golden file of a golden test
    that the same page shows, and nothing else looks like one."""

    def setUp(self):
        super().setUp()
        self.example("a.cpp", GOLDEN_EXAMPLE)
        (self.component / "examples" / "a.cpp.golden").write_text(GOLDEN_TEXT)
        self.page("p.adoc", LISTING_PAGE)

    def golden(self, text, name="a.cpp.golden"):
        (self.component / "examples" / name).write_text(text)

    def test_the_listing_next_to_its_code_is_clean(self):
        report = self.check()
        self.assertEqual(report.problems, [])
        self.assertEqual((report.golden_blocks, report.blocks, report.includes), (1, 1, 2))

    def test_modes_have_their_own_golden_files(self):
        self.example("a.cpp", "// mode: c++20, c23\n" + GOLDEN_EXAMPLE)
        self.golden(GOLDEN_TEXT, "a.cpp.c23.golden")
        self.golden(GOLDEN_TEXT, "a.cpp.c++20.golden")
        (self.component / "examples" / "a.cpp.golden").unlink()
        self.page("p.adoc", LISTING_PAGE.replace("a.cpp.golden", "a.cpp.c23.golden"))
        self.assertEqual(self.messages(), [])
        # a single-mode test has `a.cpp.golden`, not a golden per mode
        self.example("a.cpp", GOLDEN_EXAMPLE)
        self.assertTrue(any("no `golden` test of this directory has it" in p
                            for p in self.messages()), self.messages())

    def test_only_a_text_block_may_hold_it(self):
        for language in ("console", "sh", "cpp", "c", "vpr", "python", "bash"):
            with self.subTest(language):
                self.page("p.adoc", LISTING_PAGE.replace("[source,text]", f"[source,{language}]"))
                self.lint("a golden file may be included only as the whole body")

    def test_it_must_be_the_whole_body(self):
        for body in (("Output:", "include::example$a.cpp.golden[]"),
                     ("include::example$a.cpp.golden[]", "more"),
                     ("include::example$a.cpp.golden[]", "include::example$a.cpp.golden[]")):
            with self.subTest(body):
                self.page("p.adoc", page_with(
                    "[source,cpp]", "----", "include::example$a.cpp[tag=body]", "----", "",
                    "[source,text]", "----", *body, "----"))
                self.lint("a golden file may be included only as the whole body")

    def test_not_in_prose_tables_or_open_blocks(self):
        base = ("[source,cpp]", "----", "include::example$a.cpp[tag=body]", "----", "")
        for what, lines in (
                ("prose", ("include::example$a.cpp.golden[]",)),
                ("table", ("|===", "a|", "include::example$a.cpp.golden[]", "|===")),
                ("listing without a language", ("----", "include::example$a.cpp.golden[]",
                                                "----")),
                ("literal block", ("....", "include::example$a.cpp.golden[]", "....")),
                ("list item", ("* item", "+", "include::example$a.cpp.golden[]"))):
            with self.subTest(what):
                self.page("p.adoc", page_with(*base, *lines))
                self.lint("a golden file may be included only as the whole body")

    def test_no_attributes(self):
        for attrs in ("tag=x", "lines=1..2", "indent=0", "opts=optional", "leveloffset=+1",
                      "tags=a;b"):
            with self.subTest(attrs):
                self.page("p.adoc", LISTING_PAGE.replace("golden[]", f"golden[{attrs}]"))
                self.lint("a golden include takes no attributes")
        self.page("p.adoc", LISTING_PAGE.replace("golden[]", "golden[ ]"))
        self.assertEqual(self.messages(), [])

    def test_it_must_be_the_golden_of_a_golden_test(self):
        self.example("b.cpp", "// expect: frontend-ok\n// tag::t[]\nint g();\n// end::t[]\n")
        self.golden(GOLDEN_TEXT, "b.cpp.golden")  # the test is not a golden test
        self.page("p.adoc", LISTING_PAGE + "\n" + "\n".join(
            ["[source,cpp]", "----", "include::example$b.cpp[tag=t]", "----", "",
             "[source,text]", "----", "include::example$b.cpp.golden[]", "----"]))
        self.lint("is not a `golden` test")
        self.example("c.cpp", "// no header\nint g();\n")
        self.golden(GOLDEN_TEXT, "c.cpp.golden")
        self.page("p.adoc", LISTING_PAGE.replace("a.cpp.golden", "c.cpp.golden"))
        self.lint("has a bad header")
        self.golden(GOLDEN_TEXT, "stray.golden")  # no test at all
        self.page("p.adoc", LISTING_PAGE.replace("a.cpp.golden", "stray.golden"))
        self.lint("no `golden` test of this directory has it")
        self.page("p.adoc", LISTING_PAGE.replace("a.cpp.golden", "missing.cpp.golden"))
        self.lint("does not exist")

    def test_the_test_must_be_on_the_same_page(self):
        self.page("p.adoc", page_with("[source,text]", "----", "include::example$a.cpp.golden[]",
                                      "----"))
        self.lint("is not included on this page", "orphan example")
        # included by another page does not do
        self.page("q.adoc", block_of("include::example$a.cpp[tag=body]"))
        problems = self.lint("is not included on this page")
        self.assertFalse(any("orphan" in p for p in problems), problems)
        # an include inside a comment block does not count
        self.page("q.adoc", "= Q\n")
        self.page("p.adoc", page_with(
            "////", "include::example$a.cpp[tag=body]", "////", "",
            "[source,text]", "----", "include::example$a.cpp.golden[]", "----"))
        self.lint("is not included on this page")
        # in either order on the page it is fine
        self.page("p.adoc", page_with(
            "[source,text]", "----", "include::example$a.cpp.golden[]", "----", "",
            "[source,cpp]", "----", "include::example$a.cpp[tag=body]", "----"))
        self.assertEqual(self.messages(), [])

    def test_a_golden_that_is_unsafe_to_embed(self):
        for what, text in (("callout", "routine f @ L1:1 <1>\n" + GOLDEN_TEXT),
                           ("callout with spaces", "a <12>  \n" + GOLDEN_TEXT),
                           ("empty", "  \n")):
            with self.subTest(what):
                self.golden(text)
                self.lint(count=1)
        # what is safe: angle brackets inside a line, ampersands, a stderr section,
        # directives (Antora shows them verbatim: see the end-to-end twin)
        self.golden("include::x.adoc[]\nifdef::a[]\n// tag::x[]\n" + GOLDEN_TEXT)
        self.assertEqual(self.messages(), [])
        self.golden("routine f<T=int> & g @ L1:1 <tptk_unknown> x\n--- stderr ---\n" + GOLDEN_TEXT)
        self.assertEqual(self.messages(), [])

    def test_hand_written_walker_output_is_refused(self):
        lines = {
            "end walk": "end walk: 1 routines reported, 0 routines with bodies in non-system "
                        "files outside the primary file",
            "routine": "routine f plain body @ L3:5",
            "indented routine": "  routine g instance body @ L3:5",
            "attribute": "    attr [[cerastes::requires]] on routine @ L5:16",
            "statement": "stmt stmk_empty @ L3:3 attr [[cerastes::check]] @ L3:5",
        }
        for language in ("text", "console", "sh"):
            for what, line in lines.items():
                with self.subTest(language=language, what=what):
                    self.page("p.adoc", page_with("[source,cpp]", "----",
                                                  "include::example$a.cpp[tag=body]", "----", "",
                                                  f"[source,{language}]", "----", "plain", line,
                                                  "----"))
                    self.lint("walker output must be included from the golden file", count=1)

    def test_walker_output_next_to_a_real_listing_is_refused_too(self):
        self.page("p.adoc", LISTING_PAGE + "\n[source,text]\n----\nroutine f plain @ L3:5\n----\n")
        self.lint("walker output must be included", count=1)

    def test_ordinary_output_is_fine(self):
        for body in ("$ scripts/cerastes-fe.sh --c++20 --cerastes_walk x.cpp", "routine",
                     "the routine f is reported", "a.cpp:3:5: error: no", "walk: not at the start",
                     "attr [x] is not walker output", "  end walk of the dog"):
            with self.subTest(body):
                self.page("p.adoc", LISTING_PAGE + f"\n[source,text]\n----\n{body}\n----\n")
                self.assertEqual(self.messages(), [])

    def test_walker_output_in_a_comment_is_not_a_block(self):
        self.page("p.adoc", LISTING_PAGE + "\n////\nroutine f @ L1:1\n////\n")
        self.assertEqual(self.messages(), [])

    def test_a_golden_include_in_a_comment_is_not_checked(self):
        self.page("p.adoc", page_with("////", "[source,console]", "----",
                                      "include::example$a.cpp.golden[tag=x]", "----", "////",
                                      "", "[source,cpp]", "----",
                                      "include::example$a.cpp[tag=body]", "----"))
        self.assertEqual(self.messages(), [])

    def test_the_command_line(self):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            self.assertEqual(cde.main(["--root", str(self.root)]), 0, out.getvalue())
        self.page("p.adoc", LISTING_PAGE.replace("[source,text]", "[source,console]"))
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            self.assertEqual(cde.main(["--root", str(self.root)]), 1)
        self.assertIn("a golden file may be included only as the whole body", out.getvalue())


TEXT_PAGE = """<!DOCTYPE html><html><body>
<div class="listingblock"><div class="content">
<pre class="highlightjs highlight"><code class="language-text hljs" data-lang="text">{code}</code></pre>
</div></div>
<div class="listingblock"><div class="content">
<pre class="highlightjs highlight"><code class="language-cpp hljs" data-lang="cpp">int f(int x) {{ return x + 1; }}</code></pre>
</div></div>
</body></html>
"""


class GoldenListingHtmlTests(DocLintCase):
    """--html: a block with walker output equals a golden file, whatever the
    sources look like, and the pages' golden includes are all found."""

    ESCAPED = (GOLDEN_TEXT.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;"))

    def setUp(self):
        super().setUp()
        self.example("a.cpp", GOLDEN_EXAMPLE)
        (self.component / "examples" / "a.cpp.golden").write_text(GOLDEN_TEXT)
        self.page("p.adoc", LISTING_PAGE)
        self.out = self.root / "site"
        self.out.mkdir()

    def site(self, code, template=TEXT_PAGE, name="p.html"):
        (self.out / name).write_text(template.format(code=code))

    def run_html(self):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            status = cde.main(["--root", str(self.root), "--html", str(self.out)])
        return status, out.getvalue(), err.getvalue()

    def test_the_golden_text_passes(self):
        self.site(self.ESCAPED)
        status, out, err = self.run_html()
        self.assertEqual(status, 0, (out, err))

    def test_whitespace_differences_pass(self):
        self.site("\n" + self.ESCAPED.replace("\n", "  \n") + "\n")
        self.assertEqual(self.run_html()[0], 0)

    def test_a_listing_that_is_not_a_golden_file_fails(self):
        for what, code in (
                ("one changed character", self.ESCAPED.replace("param 1", "param 2")),
                ("a line dropped", self.ESCAPED.replace("  returns int\n", "")),
                ("a line added", self.ESCAPED + "routine g plain body @ L9:1\n"),
                ("an invented listing", "routine h plain body @ L1:1\nend walk: 1 routines "
                                        "reported, 0 routines with bodies in non-system files "
                                        "outside the primary file\n"),
                ("only part of it", "routine f plain body @ L3:5\n  param 1 x: int")):
            with self.subTest(what):
                self.site(code)
                status, out, _ = self.run_html()
                self.assertEqual(status, 1, out)
                self.assertIn("with walker output", out)
                self.assertIn("does not equal any golden file", out)

    def test_a_golden_file_of_a_test_that_is_gone_does_not_vouch(self):
        (self.component / "examples" / "b.cpp.golden").write_text("routine z @ L1:1\n")
        self.site("routine z @ L1:1")  # equals a golden file with no test
        status, out, _ = self.run_html()
        self.assertEqual(status, 1, out)

    def test_other_text_blocks_are_untouched(self):
        self.site("$ scripts/cerastes-fe.sh --c++20 x.cpp\nplain output")
        status, out, err = self.run_html()
        # the page sources declare a golden listing that the site does not show
        self.assertEqual(status, 1, out)
        self.assertIn("found 0 walker listings equal to a golden file but the pages' sources "
                      "include 1", out)
        self.assertNotIn("with walker output", out)

    def test_the_count_cannot_be_dodged(self):
        # two includes in the sources, one listing on the site
        self.page("q.adoc", LISTING_PAGE)
        self.site(self.ESCAPED)
        status, out, _ = self.run_html()
        self.assertEqual(status, 1, out)
        self.assertIn("found 1 walker listings equal to a golden file but the pages' sources "
                      "include 2", out)

    def test_a_site_without_golden_includes_needs_none(self):
        self.page("p.adoc", block_of("include::example$a.cpp[tag=body]"))
        self.site("plain text output", template=TEXT_PAGE)
        self.assertEqual(self.run_html()[0], 0)


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
                     "DocBuildWarningsTests", "TreeScriptTwinTests", "EdgTreeToolTests",
                     "RealTagTableTests", "RealDiagnosticTests", "RealNativeTests",
                     "RealRefusedTests", "RealTagCoverageTests", "ChecksSelftestTests",
                     "PrinterPathTests"):
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
class RealFrontEndGoldenTests(MemoisedStamp, unittest.TestCase):
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
        # ... and has blocks to look at: as many as the page sources declare (the
        # copy holds the real documentation, so the number is not a constant)
        declared = cde.check_tree(self.repo).blocks
        self.assertGreaterEqual(declared, 2)  # the e2e page and a real page at least
        found = int(re.search(r"(\d+) code blocks in a cerastes language", out).group(1))
        self.assertGreaterEqual(found, declared, out)
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

    LISTING_EXAMPLE = "doc/modules/ROOT/examples/e2e/walk.cpp"
    LISTING_GOLDEN = ("routine f<T=int> & g @ L3:5 <tptk_unknown> \\x\n"
                      "  attr [[cerastes::requires]] on routine @ L3:9\n"
                      "end walk: 1 routines reported, 0 routines with bodies in non-system "
                      "files outside the primary file\n--- stderr ---\nw: <b> & </pre>\n")

    def listing_page(self, golden=LISTING_GOLDEN):
        self.write(self.LISTING_EXAMPLE, GOLDEN_EXAMPLE)
        self.write(self.LISTING_EXAMPLE + ".golden", golden)
        self.write(self.PAGE, self.GOOD + "\n[source,cpp]\n----\n"
                   "include::example$e2e/walk.cpp[tag=body]\n----\n\n[source,text]\n----\n"
                   "include::example$e2e/walk.cpp.golden[]\n----\n")
        self.commit("walker listing")

    def test_walker_listings_render_from_golden_files_and_are_checked(self):
        """Rule g against a real build: a golden file with angle brackets, an
        ampersand, a closing pre and a backslash renders verbatim and passes
        --html; changing the golden file, or a listing typed into the page,
        fails it; a golden with a line the include would change (a callout) is
        refused by the lint, for good reason."""
        self.listing_page()
        status, out = self.build()
        self.assertEqual(status, 0, out)
        site = self.tmp / "out"
        status, out = self.run_check("--html", site)
        self.assertEqual(status, 0, out)
        self.assertEqual(self.run_check()[0], 0)
        (page,) = site.rglob("e2e-twin.html")
        self.assertIn("w: &lt;b&gt; &amp; &lt;/pre&gt;", page.read_text())
        # the site is checked against the golden files as they are now
        golden = self.repo / (self.LISTING_EXAMPLE + ".golden")
        golden.write_text(self.LISTING_GOLDEN.replace("L3:9", "L3:8"))
        status, out = self.run_check("--html", site)
        self.assertEqual(status, 1, out)
        self.assertIn("with walker output", out)
        golden.write_text(self.LISTING_GOLDEN)
        self.assertEqual(self.run_check("--html", site)[0], 0)
        # a listing typed into the page: the lint refuses it, and so does --html
        typed = self.LISTING_GOLDEN.replace("L3:9", "L3:8")
        self.write(self.PAGE, self.GOOD + "\n[source,cpp]\n----\n"
                   "include::example$e2e/walk.cpp[tag=body]\n----\n\n[source,text]\n----\n"
                   + typed.split("--- stderr ---")[0] + "----\n")
        self.commit("typed listing")
        status, out = self.build()
        self.assertEqual(status, 0, out)  # Antora accepts it
        status, out = self.run_check("--html", site)
        self.assertEqual(status, 1, out)
        self.assertIn("with walker output", out)
        status, out = self.run_check()
        self.assertEqual(status, 1, out)
        self.assertIn("walker output must be included from the golden file", out)

    def test_golden_lines_that_an_include_would_change(self):
        """Why the lint refuses a callout marker in a golden: in a real build the
        page does not show the golden file's text.  (Preprocessor directives and
        tag directives, probed the same way, are shown verbatim by Antora, so
        the lint leaves them alone.)"""
        for what, line, needle in (("callout", "x <1>", "callout marker"),):
            with self.subTest(what):
                self.listing_page(self.LISTING_GOLDEN + line + "\n")
                status, out = self.run_check()
                self.assertEqual(status, 1, out)
                self.assertIn(needle, out)
                status, build_out = self.build()
                if status == 0:  # Antora accepted it: but the page must then differ
                    (page,) = (self.tmp / "out").rglob("e2e-twin.html")
                    shown = line.replace("<", "&lt;").replace(">", "&gt;")
                    self.assertNotIn(shown, page.read_text(),
                                     (what, "the include changed nothing: the rule is not needed"))
                self.git("reset", "-q", "--hard", "HEAD~1")

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
# ---------------------------------------------------------------------------
# Front-end diagnostics: `frontend-error` / `frontend-warning`
# ---------------------------------------------------------------------------

# Real output of scripts/cerastes-fe.sh --c++20 --brief_diagnostics
# --display_error_number --no_colors (run in a directory holding the files, so
# the paths are relative), recorded on the pinned front end.  The tags are the
# ones in the generated err_data.h: 20 undefined_identifier, 2803
# attribute_namespace_unrecognized, 29 exp_primary_expr, 1105 warning_directive,
# 135 not_a_member, 161 unrecognized_pragma, 304 no_matching_function, 3326
# nonviable_because_arg_mismatch, 1574 static_assert, 3453 comparison_details,
# 65 exp_semicolon, 177 declared_but_not_referenced, 68 integer_sign_change,
# 1696 cannot_open_file, 571 cl_invalid_option.
REAL_THREE_ERRORS = '''\
"x.cpp", line 2 (col. 10): error #20: identifier "zzz" is undefined
"x.cpp", line 4 (col. 3): warning #2803-D: attribute namespace "unreg_ns" is unrecognized
"x.cpp", line 5 (col. 19): error #29: expected an expression
"x.cpp", line 6 (col. 2): warning #1105-D: #warning directive: "hello"
"x.cpp", line 8 (col. 19): error #135: class "S" has no member "y"
3 errors detected in the compilation of "x.cpp".
'''
# Notes, continuation lines, a diagnostic in an included header, a context line.
REAL_NOTES = '''\
"inc/h.h", line 1 (col. 19): error #20: identifier "qqq" is undefined
"inc/h.h", line 2 (col. 9): warning #161-D: unrecognized #pragma
"n.cpp", line 5 (col. 12): error #304: no instance of overloaded function "f" matches the argument list
            argument types are: (A)
"n.cpp", line 3 (col. 6): note #3326-D: function "f(double)" does not match because argument #1 does not match parameter
"n.cpp", line 2 (col. 6): note #3326-D: function "f(int)" does not match because argument #1 does not match parameter
"n.cpp", line 6 (col. 30): error #1574: static assertion failed with "big"
"n.cpp", line 6 (col. 54): note #3453-D: the final comparison was 4 > 100
          detected during instantiation of class "B<T> [with T=int]" at line 7
"n.cpp", line 8 (col. 30): error #65: expected a ";"
"n.cpp", line 8 (col. 18): warning #177-D: variable "x" was declared but never referenced
4 errors detected in the compilation of "n.cpp".
'''
REAL_CATASTROPHIC = '''\
"c.cpp", line 1 (col. 25): catastrophic error #1696: cannot open source file "nonexistent.h"
1 catastrophic error detected in the compilation of "c.cpp".
Compilation terminated.
'''
REAL_WARNINGS_ONLY = '''\
"r.cpp", line 1 (col. 33): warning #68-D: integer conversion resulted in a change of sign
"r.cpp", line 1 (col. 18): warning #177-D: variable "unused" was declared but never referenced
"r.cpp", line 2 (col. 9): warning #161-D: unrecognized #pragma
'''
REAL_COMMAND_LINE = '''\
Command-line error #571: invalid option: --bogus_opt
1 catastrophic error detected in this compilation.
Compilation terminated.
'''
REAL_ERROR_LIMIT = '''\
"x.cpp", line 2 (col. 10): error #20: identifier "zzz" is undefined
Error limit reached.
1 error detected in the compilation of "x.cpp".
Compilation terminated.
'''
# tag -> number, for a fixture of the generated headers (all real).
FIXTURE_TAGS = {
    "undefined_identifier": 20, "attribute_namespace_unrecognized": 2803,
    "exp_primary_expr": 29, "warning_directive": 1105, "not_a_member": 135,
    "unrecognized_pragma": 161, "no_matching_function": 304,
    "nonviable_because_arg_mismatch": 3326, "static_assert": 1574,
    "comparison_details": 3453, "exp_semicolon": 65,
    "declared_but_not_referenced": 177, "integer_sign_change": 68,
    "cannot_open_file": 1696, "cl_invalid_option": 571,
}


def write_gen(directory, tags=None, drop_entries=0, count=None):
    """Write err_codes.h and err_data.h in the format of EDG's generated headers
    (mk_errinfo) for `tags` (tag -> number); the tag table is sorted by tag,
    like the real one.  `drop_entries` cuts entries from the table without
    changing the declared count; `count` overrides the declared count."""
    tags = FIXTURE_TAGS if tags is None else tags
    directory = Path(directory)
    directory.mkdir(parents=True, exist_ok=True)
    top = max(tags.values()) + 1
    codes = {0: "ec_no_error"}
    codes.update({n: f"ec_{t}" for t, n in tags.items()})
    lines = ["/*\n\nDO NOT UPDATE THIS FILE!\n\n*/\n", "enum an_error_code {"]
    lines += [f"  {codes[n]} /* = {n} */," for n in sorted(codes)]
    lines += [f"  ec_last /* = {top} */", "};", ""]
    (directory / "err_codes.h").write_text("\n".join(lines))
    entries = sorted(tags)
    table = [f'  {{ "{t}", ec_{t} }}' for t in entries]
    table = table[:len(table) - drop_entries] if drop_entries else table
    data = ["static a_const_char *message_text[(int)ec_last + 1] = {",
            "  /* ec_no_error */", '  "unknown error",', "  /* ec_last */",
            "  (a_const_char *)NULL", "};",
            f"#define NUMBER_OF_ERROR_TAGS {len(entries) if count is None else count}",
            "static an_error_tag_entry error_tags[NUMBER_OF_ERROR_TAGS]", "#ifndef _lint",
            "= {", ",\n".join(table), "}", "#else /* ifdef _lint */", "#endif", ";", ""]
    (directory / "err_data.h").write_text("\n".join(data))
    return directory


class ExpectDiagHeaderTests(unittest.TestCase):
    def parse(self, text, suffix=".cpp"):
        return rt.parse_header(text, suffix)

    def test_grammar(self):
        exp = self.parse("// expect: frontend-error undefined_identifier @ L5\n"
                         "// expect: frontend-error undefined_identifier @ L5\n"
                         "// expect: frontend-warning attribute_namespace_unrecognized @ L7\n"
                         "// expect: frontend-error cerastes-some_tag @ L9\n"
                         "// expect: frontend-error 3rd_arg_of_assume_aligned_must_be_integral "
                         "@ L10\n"
                         "// expect: frontend-error UCN_names_basic_char @ L11\n")
        self.assertEqual(exp.kind, "frontend-diag")
        self.assertEqual(exp.diags, [
            rt.ExpectedDiag("error", "undefined_identifier", 5),
            rt.ExpectedDiag("error", "undefined_identifier", 5),
            rt.ExpectedDiag("warning", "attribute_namespace_unrecognized", 7),
            rt.ExpectedDiag("error", "cerastes-some_tag", 9),
            rt.ExpectedDiag("error", "3rd_arg_of_assume_aligned_must_be_integral", 10),
            rt.ExpectedDiag("error", "UCN_names_basic_char", 11)])
        self.assertEqual(exp.modes, ["c++20"])
        self.assertEqual(str(exp.diags[0]), "undefined_identifier @ L5")

    def test_only_warnings_is_a_test(self):
        exp = self.parse("// expect: frontend-warning unrecognized_pragma @ L3\n")
        self.assertEqual(exp.kind, "frontend-diag")

    def test_modes_apply(self):
        exp = self.parse("// expect: frontend-error undefined_identifier @ L3\n"
                         "// mode: c++20, c23\n", ".c")
        self.assertEqual(exp.modes, ["c++20", "c23"])
        self.assertEqual(self.parse("// expect: frontend-warning a @ L3\n", ".c").modes, ["c23"])

    def test_bad_headers(self):
        err = "// expect: frontend-error undefined_identifier @ L5\n"
        bad = {
            "with golden": err + "// expect: golden\n",
            "with verified": err + "// expect: verified\n",
            "with a viper error": err + "// expect: error assert @ L3\n",
            "with rejected": err + "// expect: rejected unsupported-union\n",
            "with frontend-ok": err + "// expect: frontend-ok\n",
            "no line": "// expect: frontend-error undefined_identifier\n",
            "line zero": "// expect: frontend-error undefined_identifier @ L0\n",
            "no tag": "// expect: frontend-error @ L3\n",
            "tag with a dash": "// expect: frontend-error undefined-identifier @ L3\n",
            "tag with a space": "// expect: frontend-error undefined identifier @ L3\n",
            "bare cerastes-": "// expect: frontend-error cerastes- @ L3\n",
            "upper case cerastes tag": "// expect: frontend-error cerastes-Tag @ L3\n",
            "severity typo": "// expect: frontend-warn undefined_identifier @ L3\n",
            "remark": "// expect: frontend-remark undefined_identifier @ L3\n",
            "after code": "int x;\n" + err,
            "reason syntax": "// expect: frontend-error undefined_identifier:foo @ L3\n",
        }
        for what, text in bad.items():
            with self.subTest(what), self.assertRaises(rt.HeaderError):
                self.parse(text)

    def test_not_for_vpr(self):
        with self.assertRaises(rt.HeaderError):
            self.parse("// expect: frontend-error undefined_identifier @ L3\n", ".vpr")


class NativeOkHeaderTests(unittest.TestCase):
    def parse(self, text, suffix=".cpp"):
        return rt.parse_header(text, suffix)

    def test_accompanies_an_outcome(self):
        for outcome, kind in (("golden", "golden"), ("verified", "verified"),
                              ("frontend-ok", "frontend-ok"),
                              ("frontend-error undefined_identifier @ L3", "frontend-diag"),
                              ("frontend-warning unrecognized_pragma @ L3", "frontend-diag"),
                              ("error assert @ L3", "error"),
                              ("rejected unsupported-union", "rejected")):
            for order in (("{o}", "native-ok"), ("native-ok", "{o}")):
                with self.subTest(outcome=outcome, order=order):
                    text = "".join(f"// expect: {l.format(o=outcome)}\n" for l in order)
                    exp = self.parse(text)
                    self.assertEqual((exp.kind, exp.native_ok), (kind, True))

    def test_default_is_not_native(self):
        self.assertFalse(self.parse("// expect: golden\n").native_ok)

    def test_alone_is_not_a_test(self):
        with self.assertRaises(rt.HeaderError) as ctx:
            self.parse("// expect: native-ok\n")
        self.assertIn("accompanies an outcome", str(ctx.exception))

    def test_twice(self):
        with self.assertRaises(rt.HeaderError) as ctx:
            self.parse("// expect: golden\n// expect: native-ok\n// expect: native-ok\n")
        self.assertIn("more than one", str(ctx.exception))

    def test_not_in_viper(self):
        with self.assertRaises(rt.HeaderError):
            self.parse("// expect: verified\n// expect: native-ok\n", ".vpr")

    def test_sole_kinds_still_stand_alone(self):
        for text in ("// expect: golden\n// expect: golden\n// expect: native-ok\n",
                     "// expect: verified\n// expect: verified\n// expect: native-ok\n",
                     "// expect: golden\n// expect: verified\n// expect: native-ok\n",
                     "// expect: frontend-ok\n// expect: golden\n// expect: native-ok\n"):
            with self.subTest(text), self.assertRaises(rt.HeaderError):
                self.parse(text)

    def test_native_ok_must_be_in_the_header(self):
        with self.assertRaises(rt.HeaderError):
            self.parse("// expect: golden\nint x;\n// expect: native-ok\n")

    def test_misspellings(self):
        for body in ("native_ok", "native-ok x", "native", "ordinary-ok", "Native-ok"):
            with self.subTest(body), self.assertRaises(rt.HeaderError):
                self.parse(f"// expect: golden\n// expect: {body}\n")


class TwinHeaderTests(unittest.TestCase):
    def test_syntax(self):
        exp = rt.parse_header("// expect: frontend-error a @ L3\n"
                              "// twin: tests/annotations/ok.cpp\n", ".cpp")
        self.assertEqual(exp.twin, "tests/annotations/ok.cpp")
        self.assertIsNone(rt.parse_header("// expect: golden\n", ".cpp").twin)
        bad = {
            "absolute": "// twin: /tests/ok.cpp\n",
            "dot dot": "// twin: tests/../ok.cpp\n",
            "not a test suffix": "// twin: tests/ok.txt\n",
            "header suffix": "// twin: tests/ok.h\n",
            "empty": "// twin:\n",
            "spaces": "// twin: tests/a b.cpp\n",
            "two": "// twin: a.cpp\n// twin: b.cpp\n",
            "after code": "int x;\n// twin: a.cpp\n",
        }
        for what, text in bad.items():
            with self.subTest(what), self.assertRaises(rt.HeaderError):
                rt.parse_header("// expect: frontend-error a @ L3\n" + text, ".cpp")

    def test_not_for_viper(self):
        with self.assertRaises(rt.HeaderError):
            rt.parse_header("// expect: verified\n// twin: a.cpp\n", ".vpr")


class RootedCase(unittest.TestCase):
    """A temporary repository root that rt.ROOT points at."""

    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.root = Path(tmp.name).resolve()
        patcher = mock.patch.object(rt, "ROOT", self.root)
        patcher.start()
        self.addCleanup(patcher.stop)

    def put(self, rel, text):
        path = self.root / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)
        return path

    def header(self, rel, text):
        return rt.read_header(self.put(rel, text))


class NativePolicyTests(RootedCase):
    POSITIVE = "// expect: golden\nint f();\n"
    NATIVE = "// expect: golden\n// expect: native-ok\nint f();\n"

    def test_the_policy_directories_are_pinned(self):
        # Dropping a directory would let its tests skip the ordinary compilers.
        self.assertEqual(
            [str(d) for d in rt.NATIVE_POLICY_DIRS],
            ["tests/annotations", "doc/modules/ROOT/examples/annotations",
             "doc/modules/ROOT/examples/ordinary-compilers"])
        self.assertEqual(rt.POSITIVE_KINDS, ("golden", "verified"))

    def test_a_positive_test_in_a_policy_directory_needs_native_ok(self):
        for directory in ("tests/annotations", "tests/annotations/sub/deeper",
                          "doc/modules/ROOT/examples/annotations",
                          "doc/modules/ROOT/examples/ordinary-compilers/x"):
            with self.subTest(directory):
                missing = self.header(f"{directory}/a.cpp", self.POSITIVE)
                self.assertIsInstance(missing, rt.HeaderError)
                self.assertIn("native-ok", str(missing))
                self.assertIsInstance(self.header(f"{directory}/b.cpp", self.NATIVE),
                                      rt.Expectation)

    def test_verified_counts_as_positive(self):
        err = self.header("tests/annotations/v.cpp", "// expect: verified\n")
        self.assertIsInstance(err, rt.HeaderError)
        ok = self.header("tests/annotations/w.cpp", "// expect: verified\n// expect: native-ok\n")
        self.assertIsInstance(ok, rt.Expectation)

    def test_other_places_are_free(self):
        for rel in ("tests/il/a.cpp", "tests/annotationsX/a.cpp", "tests/a.cpp",
                    "doc/modules/ROOT/examples/other/a.cpp",
                    "doc/modules/ROOT/examples/annotations_old/a.cpp"):
            with self.subTest(rel):
                self.assertIsInstance(self.header(rel, self.POSITIVE), rt.Expectation)

    def test_negative_tests_are_not_positive(self):
        exp = self.header("tests/annotations/a.cpp",
                          "// expect: frontend-error undefined_identifier @ L3\n")
        self.assertIsInstance(exp, rt.Expectation)
        exp = self.header("tests/annotations/b.cpp", "// expect: error assert @ L3\n")
        self.assertIsInstance(exp, rt.Expectation)

    def test_including_cerastes_h_needs_native_ok_anywhere(self):
        for include in ('#include <cerastes.h>', '#include "cerastes.h"',
                        '  #  include   <cerastes.h>', '#include<cerastes.h>',
                        '\t#include "cerastes.h"  // spec functions'):
            with self.subTest(include):
                text = f"// expect: golden\n{include}\nint f();\n"
                err = self.header("tests/il/h.cpp", text)
                self.assertIsInstance(err, rt.HeaderError)
                self.assertIn("cerastes.h", str(err))
                ok = self.header("tests/il/h2.cpp", text.replace(
                    "golden\n", "golden\n// expect: native-ok\n"))
                self.assertIsInstance(ok, rt.Expectation)

    def test_other_includes_and_comments_are_free(self):
        for line in ('// #include <cerastes.h>', '#include <cerastes_extra.h>',
                     '#include <my/cerastes.h>', '#include "xcerastes.h"',
                     '/* #include <cerastes.h> */'):
            with self.subTest(line):
                text = f"// expect: golden\n{line}\nint f();\n"
                self.assertIsInstance(self.header("tests/il/h.cpp", text), rt.Expectation)

    def test_cerastes_h_in_a_negative_test_is_free(self):
        text = ("// expect: frontend-error undefined_identifier @ L4\n"
                '#include <cerastes.h>\nint f() { return zzz; }\n')
        self.assertIsInstance(self.header("tests/il/h.cpp", text), rt.Expectation)

    def test_check_headers_reports_the_policy(self):
        self.put("tests/annotations/a.cpp", self.POSITIVE)
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            status = rt.main(["--check-headers", str(self.root / "tests")])
        self.assertEqual(status, 1, out.getvalue())
        self.assertIn("a positive test under tests/annotations/", out.getvalue())
        self.assertIn("0 of 1 headers ok", out.getvalue())


class TwinRuleTests(RootedCase):
    NEG = "tests/annotations/negative"
    POSITIVE = "// expect: golden\n// expect: native-ok\nint f();\n"

    def setUp(self):
        super().setUp()
        self.put("tests/annotations/ok.cpp", self.POSITIVE)

    def neg(self, name, twin, outcome="frontend-error undefined_identifier @ L4"):
        lines = [f"// expect: {outcome}"]
        if twin is not None:
            lines.append(f"// twin: {twin}")
        return self.header(f"{self.NEG}/{name}", "\n".join(lines) + "\nint g();\n")

    def test_the_negative_directory_is_pinned(self):
        self.assertEqual(str(rt.NEGATIVE_DIR), "tests/annotations/negative")

    def test_a_valid_twin_passes(self):
        exp = self.neg("a.cpp", "tests/annotations/ok.cpp")
        self.assertIsInstance(exp, rt.Expectation)
        self.assertEqual(exp.twin, "tests/annotations/ok.cpp")

    def test_a_verified_twin_passes(self):
        self.put("tests/scalar/v.cpp", "// expect: verified\n")
        self.assertIsInstance(self.neg("b.cpp", "tests/scalar/v.cpp"), rt.Expectation)

    def test_every_negative_test_needs_a_twin(self):
        err = self.neg("a.cpp", None)
        self.assertIsInstance(err, rt.HeaderError)
        self.assertIn("needs `// twin:", str(err))
        # in a subdirectory of negative/ too
        err = self.header(f"{self.NEG}/deep/er/x.cpp",
                          "// expect: frontend-error undefined_identifier @ L3\n")
        self.assertIsInstance(err, rt.HeaderError)

    def test_every_must_fail_kind_needs_a_twin_in_negative(self):
        for outcome in ("error assert @ L3", "rejected unsupported-union",
                        "frontend-error undefined_identifier @ L3"):
            with self.subTest(outcome):
                err = self.neg("x.cpp", None, outcome)
                self.assertIsInstance(err, rt.HeaderError)
                self.assertIn("needs `// twin:", str(err))
                self.assertIsInstance(self.neg("y.cpp", "tests/annotations/ok.cpp", outcome),
                                      rt.Expectation)

    def test_negative_tests_must_fail(self):
        # A test under negative/ that expects no error is not a must-fail test,
        # whatever its twin is (the rule of invariant 2 would be vacuous).
        for what, text in (
                ("golden", "// expect: golden\n// expect: native-ok\n"),
                ("verified", "// expect: verified\n"),
                ("frontend-ok", "// expect: frontend-ok\n"),
                ("warnings only", "// expect: frontend-warning unrecognized_pragma @ L3\n"),
                ("two warnings", "// expect: frontend-warning unrecognized_pragma @ L3\n"
                                 "// expect: frontend-warning unrecognized_pragma @ L4\n")):
            with self.subTest(what):
                err = self.header(f"{self.NEG}/{what.replace(' ', '_')}.cpp",
                                  text + "// twin: tests/annotations/ok.cpp\n")
                self.assertIsInstance(err, rt.HeaderError)
                self.assertIn("must expect at least one error", str(err))
        # the same tests are fine outside negative/
        for name, text in (("g.cpp", "// expect: golden\n// expect: native-ok\n"),
                           ("w.cpp", "// expect: frontend-warning unrecognized_pragma @ L3\n")):
            self.assertIsInstance(self.header(f"tests/annotations/warnings/{name}", text),
                                  rt.Expectation)
        # one error among warnings is enough
        mixed = ("// expect: frontend-warning unrecognized_pragma @ L3\n"
                 "// expect: frontend-error undefined_identifier @ L4\n"
                 "// twin: tests/annotations/ok.cpp\n")
        self.assertIsInstance(self.header(f"{self.NEG}/mixed.cpp", mixed), rt.Expectation)

    def test_negative_tests_cannot_be_twin_exempt(self):
        err = self.header(f"{self.NEG}/e.cpp", "// expect: frontend-error undefined_identifier "
                          "@ L3\n// twin-exempt: because\n")
        self.assertIsInstance(err, rt.HeaderError)

    def test_a_dangling_twin(self):
        err = self.neg("a.cpp", "tests/annotations/nope.cpp")
        self.assertIsInstance(err, rt.HeaderError)
        self.assertIn("does not exist", str(err))

    def test_a_twin_in_negative_is_refused(self):
        self.put(f"{self.NEG}/other.cpp",
                 "// expect: frontend-error undefined_identifier @ L3\n"
                 "// twin: tests/annotations/ok.cpp\n")
        err = self.neg("a.cpp", f"{self.NEG}/other.cpp")
        self.assertIsInstance(err, rt.HeaderError)
        self.assertIn("is itself under", str(err))

    def test_a_non_positive_twin_is_refused(self):
        for name, text in (("fe.cpp", "// expect: frontend-error undefined_identifier @ L3\n"),
                           ("w.cpp", "// expect: frontend-warning unrecognized_pragma @ L3\n"),
                           ("e.vpr", "// expect: error assert @ L3\n"),
                           ("r.cpp", "// expect: rejected unsupported-union\n"),
                           ("x.cpp", "// expect: error assert @ L3\n")):
            with self.subTest(name):
                self.put(f"tests/annotations/{name}", text)
                if name.endswith(".vpr"):
                    continue  # a .vpr twin is a different case, below
                err = self.neg("a.cpp", f"tests/annotations/{name}")
                self.assertIsInstance(err, rt.HeaderError)
                self.assertIn("not a positive test", str(err))

    def test_a_frontend_ok_twin_is_not_positive(self):
        self.put("tests/il/o.cpp", "// expect: frontend-ok\n")
        err = self.neg("a.cpp", "tests/il/o.cpp")
        self.assertIsInstance(err, rt.HeaderError)
        self.assertIn("not a positive test", str(err))

    def test_a_twin_with_a_bad_header(self):
        self.put("tests/annotations/bad.cpp", "// no expectation here\n")
        err = self.neg("a.cpp", "tests/annotations/bad.cpp")
        self.assertIsInstance(err, rt.HeaderError)
        self.assertIn("bad header", str(err))

    def test_a_twin_that_breaks_the_native_policy_is_a_bad_twin(self):
        self.put("tests/annotations/nonative.cpp", "// expect: golden\nint f();\n")
        err = self.neg("a.cpp", "tests/annotations/nonative.cpp")
        self.assertIsInstance(err, rt.HeaderError)
        self.assertIn("bad header", str(err))

    def test_a_test_cannot_be_its_own_twin(self):
        path = self.put("tests/annotations/self.cpp",
                        "// expect: golden\n// expect: native-ok\n"
                        "// twin: tests/annotations/self.cpp\n")
        err = rt.read_header(path)
        self.assertIsInstance(err, rt.HeaderError)
        self.assertIn("own twin", str(err))

    def test_twins_are_checked_outside_negative_too(self):
        # A `twin:` that nothing would check is an ignored expectation.
        err = self.header("tests/scalar/t.cpp",
                          "// expect: verified\n// twin: tests/annotations/missing.cpp\n")
        self.assertIsInstance(err, rt.HeaderError)
        ok = self.header("tests/scalar/t2.cpp",
                         "// expect: verified\n// twin: tests/annotations/ok.cpp\n")
        self.assertIsInstance(ok, rt.Expectation)

    def test_check_headers_reports_a_missing_twin(self):
        self.put(f"{self.NEG}/a.cpp", "// expect: frontend-error undefined_identifier @ L3\n")
        out = io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(io.StringIO()), \
                mock.patch.object(rt, "check_diag_headers"):
            status = rt.main(["--check-headers", str(self.root / "tests")])
        self.assertEqual(status, 1, out.getvalue())
        self.assertIn("needs `// twin:", out.getvalue())


class TwinExemptHeaderTests(unittest.TestCase):
    def test_syntax(self):
        exp = rt.parse_header("// expect: golden\n// twin-exempt: a smoke test of the header\n",
                              ".cpp")
        self.assertEqual(exp.twin_exempt, "a smoke test of the header")
        self.assertIsNone(rt.parse_header("// expect: golden\n", ".cpp").twin_exempt)
        bad = {
            "empty reason": "// expect: golden\n// twin-exempt:\n",
            "blank reason": "// expect: golden\n// twin-exempt:    \n",
            "twice": "// expect: golden\n// twin-exempt: a\n// twin-exempt: b\n",
            "with a twin": "// expect: golden\n// twin: tests/a.cpp\n// twin-exempt: why\n",
            "after code": "// expect: golden\nint x;\n// twin-exempt: why\n",
            "not positive: frontend-error":
                "// expect: frontend-error undefined_identifier @ L3\n// twin-exempt: why\n",
            "not positive: frontend-ok": "// expect: frontend-ok\n// twin-exempt: why\n",
        }
        for what, text in bad.items():
            with self.subTest(what), self.assertRaises(rt.HeaderError):
                rt.parse_header(text, ".cpp")
        with self.assertRaises(rt.HeaderError):
            rt.parse_header("// expect: verified\n// twin-exempt: why\n", ".vpr")
        # a twin-exempt line is not a twin line
        self.assertIsNone(rt.parse_header("// expect: golden\n// twin-exempt: why\n",
                                          ".cpp").twin)


class PositiveNeedsATwinTests(RootedCase):
    """The twin rule in the other direction (invariant 2)."""

    POSITIVE = "// expect: golden\n// expect: native-ok\nint f();\n"
    NEG = ("// expect: frontend-error undefined_identifier @ L4\n"
           "// twin: tests/annotations/{twin}\nint g();\n")

    def problems(self):
        return {str(f.relative_to(self.root)): m for f, m in rt.cross_file_problems()}

    def test_no_annotation_tests_no_problems(self):
        self.assertEqual(rt.cross_file_problems(), [])
        self.put("tests/il/a.cpp", self.POSITIVE)
        self.assertEqual(rt.cross_file_problems(), [])

    def test_a_positive_test_without_a_must_fail_twin_is_a_problem(self):
        self.put("tests/annotations/contracts/a.cpp", self.POSITIVE)
        self.put("tests/annotations/contracts/b.cpp", "// expect: verified\n")
        found = self.problems()
        self.assertEqual(sorted(found), ["tests/annotations/contracts/a.cpp",
                                         "tests/annotations/contracts/b.cpp"])
        self.assertIn("must be the `twin:` of a test under tests/annotations/negative/",
                      found["tests/annotations/contracts/a.cpp"])
        self.assertIn("twin-exempt", found["tests/annotations/contracts/a.cpp"])

    def test_a_negative_test_naming_it_clears_it(self):
        self.put("tests/annotations/contracts/a.cpp", self.POSITIVE)
        self.put("tests/annotations/negative/n1.cpp",
                 self.NEG.format(twin="contracts/a.cpp"))
        self.assertEqual(self.problems(), {})
        # one negative per positive is enough; several are fine
        self.put("tests/annotations/negative/n2.cpp", self.NEG.format(twin="contracts/a.cpp"))
        self.assertEqual(self.problems(), {})

    def test_a_negative_test_naming_another_positive_does_not(self):
        self.put("tests/annotations/contracts/a.cpp", self.POSITIVE)
        self.put("tests/annotations/contracts/b.cpp", self.POSITIVE)
        self.put("tests/annotations/negative/n.cpp", self.NEG.format(twin="contracts/a.cpp"))
        self.assertEqual(list(self.problems()), ["tests/annotations/contracts/b.cpp"])

    def test_only_tests_under_negative_count(self):
        self.put("tests/annotations/contracts/a.cpp", self.POSITIVE)
        self.put("tests/annotations/warnings/w.cpp",
                 "// expect: frontend-warning unrecognized_pragma @ L4\n"
                 "// twin: tests/annotations/contracts/a.cpp\nint g();\n")
        self.put("tests/annotations/other/o.cpp",
                 "// expect: frontend-error undefined_identifier @ L4\n"
                 "// twin: tests/annotations/contracts/a.cpp\nint g();\n")
        self.assertEqual(list(self.problems()), ["tests/annotations/contracts/a.cpp"])

    def test_twin_exempt_clears_it(self):
        self.put("tests/annotations/header/h.cpp",
                 "// expect: golden\n// expect: native-ok\n"
                 "// twin-exempt: the header test has no meaningful misuse\nint f();\n")
        self.assertEqual(self.problems(), {})
        # an empty reason is a header error, and the test is still a problem
        self.put("tests/annotations/header/h2.cpp",
                 "// expect: golden\n// expect: native-ok\n// twin-exempt:\nint f();\n")
        self.assertIsInstance(rt.read_header(self.root / "tests/annotations/header/h2.cpp"),
                              rt.HeaderError)
        self.assertEqual(list(self.problems()), [])  # reported through its own header error

    def test_negative_and_warning_tests_are_not_positive(self):
        self.put("tests/annotations/negative/n.cpp", self.NEG.format(twin="contracts/a.cpp"))
        self.put("tests/annotations/warnings/w.cpp",
                 "// expect: frontend-warning unrecognized_pragma @ L4\n")
        self.put("tests/annotations/contracts/a.cpp", self.POSITIVE)
        self.assertEqual(self.problems(), {})

    def test_check_headers_and_runs_report_it(self):
        self.put("tests/annotations/contracts/a.cpp", self.POSITIVE)
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            status = rt.main(["--check-headers", str(self.root / "tests")])
        self.assertEqual(status, 1, out.getvalue())
        self.assertIn("tests/annotations/contracts/a.cpp: bad header: a positive test under "
                      "tests/annotations/ must be the `twin:` of a test under", out.getvalue())
        self.assertIn("1 of 1 headers ok", out.getvalue())
        self.assertIn("1 positive tests without a must-fail twin", out.getvalue())

    def test_a_partial_selection_still_checks_the_whole_tree(self):
        self.put("tests/annotations/contracts/a.cpp", self.POSITIVE)
        self.put("tests/annotations/contracts/b.cpp", self.POSITIVE)
        self.put("tests/annotations/negative/n.cpp", self.NEG.format(twin="contracts/a.cpp"))
        out = io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(io.StringIO()):
            status = rt.main(["--check-headers", str(self.root / "tests/annotations/contracts/a.cpp")])
        self.assertEqual(status, 1, out.getvalue())
        self.assertIn("contracts/b.cpp: bad header", out.getvalue())

    def test_selections_elsewhere_are_not_blamed(self):
        self.put("tests/annotations/contracts/a.cpp", self.POSITIVE)
        self.put("tests/il/i.cpp", "// expect: golden\nint f();\n")
        out = io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(io.StringIO()):
            status = rt.main(["--check-headers", str(self.root / "tests/il")])
        self.assertEqual(status, 0, out.getvalue())


# ---------------------------------------------------------------------------
# The tag tables
# ---------------------------------------------------------------------------

class TagRegistryTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.gen = Path(tmp.name)

    def test_parse_the_fixture(self):
        write_gen(self.gen)
        reg = rt.TagRegistry.load(self.gen)
        self.assertEqual(reg.numbers["undefined_identifier"], 20)
        self.assertEqual(reg.tags[2803], "attribute_namespace_unrecognized")
        self.assertEqual(len(reg.numbers), len(FIXTURE_TAGS))
        self.assertEqual(reg.tags[1105], "warning_directive")

    def test_a_truncated_table_is_an_error(self):
        write_gen(self.gen, drop_entries=1)
        with self.assertRaises(rt.UsageError) as ctx:
            rt.TagRegistry.load(self.gen)
        self.assertIn("were parsed", str(ctx.exception))

    def test_a_wrong_declared_count_is_an_error(self):
        write_gen(self.gen, count=len(FIXTURE_TAGS) + 1)
        with self.assertRaises(rt.UsageError):
            rt.TagRegistry.load(self.gen)

    def test_a_tag_without_a_number_is_an_error(self):
        write_gen(self.gen)
        text = (self.gen / "err_codes.h").read_text()
        (self.gen / "err_codes.h").write_text(
            text.replace("  ec_undefined_identifier /* = 20 */,\n", ""))
        with self.assertRaises(rt.UsageError) as ctx:
            rt.TagRegistry.load(self.gen)
        self.assertIn("ec_undefined_identifier", str(ctx.exception))

    def test_a_duplicate_tag_is_an_error(self):
        write_gen(self.gen)
        text = (self.gen / "err_data.h").read_text()
        (self.gen / "err_data.h").write_text(text.replace(
            '  { "exp_semicolon", ec_exp_semicolon }',
            '  { "exp_semicolon", ec_exp_semicolon },\n  { "exp_semicolon", ec_exp_semicolon }')
            .replace(f"TAGS {len(FIXTURE_TAGS)}", f"TAGS {len(FIXTURE_TAGS) + 1}"))
        with self.assertRaises(rt.UsageError) as ctx:
            rt.TagRegistry.load(self.gen)
        self.assertIn("twice", str(ctx.exception))

    def test_unrecognized_headers_are_an_error(self):
        write_gen(self.gen)
        (self.gen / "err_data.h").write_text("// an unknown format\n")
        with self.assertRaises(rt.UsageError):
            rt.TagRegistry.load(self.gen)
        write_gen(self.gen)
        (self.gen / "err_codes.h").write_text("enum { };\n")
        with self.assertRaises(rt.UsageError):
            rt.TagRegistry.load(self.gen)

    def test_missing_files_and_variable(self):
        with self.assertRaises(rt.UsageError) as ctx:
            rt.TagRegistry.load(self.gen / "nowhere")
        self.assertIn("scripts/bootstrap.sh", str(ctx.exception))
        with self.assertRaises(rt.UsageError) as ctx:
            rt.load_registry({})
        self.assertIn("CERASTES_EDG_GEN not set", str(ctx.exception))
        with self.assertRaises(rt.UsageError):
            rt.load_registry({"CERASTES_EDG_GEN": str(self.gen / "nowhere")})

    def test_the_cache_follows_the_files(self):
        write_gen(self.gen, {"undefined_identifier": 20})
        env = {"CERASTES_EDG_GEN": str(self.gen)}
        self.assertEqual(set(rt.load_registry(env).numbers), {"undefined_identifier"})
        write_gen(self.gen, {"exp_semicolon": 65})
        self.assertEqual(set(rt.load_registry(env).numbers), {"exp_semicolon"})

    def test_cerastes_tags_file(self):
        missing = self.gen / "diagnostics.def"
        self.assertIsNone(rt.load_cerastes_tags(missing))
        missing.write_text("// diagnostics\nCERASTES_DIAG(contract_not_boolean, error, \"x\")\n"
                           "  CERASTES_DIAG( \"binder_shadows_parameter\" , error, \"y\")\n"
                           "CERASTES_DIAG_LAST(end)\n#define X 1\n")
        self.assertEqual(rt.load_cerastes_tags(missing),
                         {"contract_not_boolean", "binder_shadows_parameter", "end"})
        missing.write_text("")
        self.assertEqual(rt.load_cerastes_tags(missing), set())


class ValidateTagsTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        write_gen(tmp.name)
        self.reg = rt.TagRegistry.load(tmp.name)

    def exp(self, *tags):
        return rt.Expectation("frontend-diag", [], None, ["c++20"],
                              [rt.ExpectedDiag("error", t, 3) for t in tags])

    def test_known_tags_pass(self):
        self.assertIsNone(rt.validate_diag_tags(
            self.exp("undefined_identifier", "exp_semicolon"), self.reg, None))

    def test_an_unknown_tag_is_a_header_error_with_a_hint(self):
        err = rt.validate_diag_tags(self.exp("undefined_identifer"), self.reg, None)
        self.assertIsInstance(err, rt.HeaderError)
        self.assertIn("unknown EDG diagnostic tag `undefined_identifer`", str(err))
        self.assertIn("did you mean undefined_identifier", str(err))

    def test_the_second_unknown_tag_is_found_too(self):
        err = rt.validate_diag_tags(self.exp("undefined_identifier", "nope"), self.reg, None)
        self.assertIn("`nope`", str(err))

    def test_a_number_is_not_a_tag(self):
        self.assertIsInstance(rt.validate_diag_tags(self.exp("20"), self.reg, None),
                              rt.HeaderError)

    def test_cerastes_tags_before_any_are_defined(self):
        err = rt.validate_diag_tags(self.exp("cerastes-contract_not_boolean"), self.reg, None)
        self.assertIsInstance(err, rt.HeaderError)
        self.assertIn("no cerastes diagnostics are defined yet", str(err))

    def test_cerastes_tags_once_defined(self):
        known = {"contract_not_boolean"}
        self.assertIsNone(rt.validate_diag_tags(
            self.exp("cerastes-contract_not_boolean"), self.reg, known))
        err = rt.validate_diag_tags(self.exp("cerastes-other"), self.reg, known)
        self.assertIn("unknown cerastes diagnostic `cerastes-other`", str(err))
        # an EDG tag is not a cerastes tag, nor the other way round
        self.assertIsInstance(rt.validate_diag_tags(
            self.exp("cerastes-undefined_identifier"), self.reg, known), rt.HeaderError)
        self.assertIsInstance(rt.validate_diag_tags(
            self.exp("contract_not_boolean"), self.reg, known), rt.HeaderError)

    def test_check_diag_headers_replaces_bad_expectations(self):
        good = rt.Expectation("golden", [], None, ["c++20"])
        headers = {Path("a.cpp"): good, Path("b.cpp"): self.exp("nope"),
                   Path("c.cpp"): self.exp("exp_semicolon"),
                   Path("d.cpp"): rt.HeaderError("already bad")}
        with tempfile.TemporaryDirectory() as tmp:
            write_gen(tmp)
            rt.check_diag_headers(headers, {"CERASTES_EDG_GEN": tmp})
        self.assertIs(headers[Path("a.cpp")], good)
        self.assertIsInstance(headers[Path("b.cpp")], rt.HeaderError)
        self.assertIsInstance(headers[Path("c.cpp")], rt.Expectation)

    def test_no_diagnostic_tests_need_no_tables(self):
        headers = {Path("a.cpp"): rt.Expectation("golden", [], None, ["c++20"])}
        rt.check_diag_headers(headers, {})  # no CERASTES_EDG_GEN, and no error


# ---------------------------------------------------------------------------
# Parsing EDG's diagnostics
# ---------------------------------------------------------------------------

class FeDiagnosticParseTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        write_gen(tmp.name)
        self.reg = rt.TagRegistry.load(tmp.name)

    def summary(self, text):
        parsed = rt.parse_fe_diagnostics(text)
        return [(d.severity, d.tag(self.reg), d.file, d.line, d.col) for d in parsed.diags]

    def test_three_errors_and_two_warnings(self):
        parsed = rt.parse_fe_diagnostics(REAL_THREE_ERRORS)
        self.assertEqual(parsed.problems, [])
        self.assertEqual(parsed.summary_errors, 3)
        self.assertEqual(self.summary(REAL_THREE_ERRORS), [
            ("error", "undefined_identifier", "x.cpp", 2, 10),
            ("warning", "attribute_namespace_unrecognized", "x.cpp", 4, 3),
            ("error", "exp_primary_expr", "x.cpp", 5, 19),
            ("warning", "warning_directive", "x.cpp", 6, 2),
            ("error", "not_a_member", "x.cpp", 8, 19)])
        self.assertTrue(all(d.discretionary for d in rt.parse_fe_diagnostics(
            REAL_THREE_ERRORS).diags if d.severity == "warning"))

    def test_notes_and_continuation_lines_attach_and_do_not_count(self):
        parsed = rt.parse_fe_diagnostics(REAL_NOTES)
        self.assertEqual(parsed.problems, [])
        self.assertEqual(parsed.summary_errors, 4)
        kinds = [(d.severity, d.tag(self.reg), d.file, d.line) for d in parsed.diags]
        self.assertEqual(kinds, [
            ("error", "undefined_identifier", "inc/h.h", 1),
            ("warning", "unrecognized_pragma", "inc/h.h", 2),
            ("error", "no_matching_function", "n.cpp", 5),
            ("note", "nonviable_because_arg_mismatch", "n.cpp", 3),
            ("note", "nonviable_because_arg_mismatch", "n.cpp", 2),
            ("error", "static_assert", "n.cpp", 6),
            ("note", "comparison_details", "n.cpp", 6),
            ("error", "exp_semicolon", "n.cpp", 8),
            ("warning", "declared_but_not_referenced", "n.cpp", 8)])

    def test_only_warnings_have_no_summary(self):
        parsed = rt.parse_fe_diagnostics(REAL_WARNINGS_ONLY)
        self.assertEqual((parsed.problems, parsed.summary_errors), ([], None))
        self.assertEqual(len(parsed.diags), 3)

    def test_empty_output(self):
        parsed = rt.parse_fe_diagnostics("")
        self.assertEqual((parsed.diags, parsed.problems, parsed.summary_errors), ([], [], None))

    def test_catastrophic_and_termination(self):
        parsed = rt.parse_fe_diagnostics(REAL_CATASTROPHIC)
        self.assertEqual([d.severity for d in parsed.diags], ["catastrophic error"])
        self.assertEqual(parsed.summary_errors, 0)
        self.assertEqual(len(parsed.problems), 1)
        self.assertIn("terminated", parsed.problems[0])

    def test_command_line_error_has_no_position(self):
        parsed = rt.parse_fe_diagnostics(REAL_COMMAND_LINE)
        (d,) = parsed.diags
        self.assertEqual((d.severity, d.file, d.line, d.tag(self.reg)),
                         ("command-line error", None, None, "cl_invalid_option"))
        self.assertEqual(d.where(), "(no position)")

    def test_error_limit_and_aborted(self):
        parsed = rt.parse_fe_diagnostics(REAL_ERROR_LIMIT)
        self.assertTrue(any("error limit" in p for p in parsed.problems), parsed.problems)
        parsed = rt.parse_fe_diagnostics("Compilation aborted.\n")
        self.assertTrue(any("aborted" in p for p in parsed.problems))

    def test_an_unnumbered_cerastes_diagnostic(self):
        text = '"x.cpp", line 7 (col. 3): error #cerastes-contract_not_boolean: bad\n' \
               '"x.cpp", line 8: warning #cerastes-unused_binder-D: x\n' \
               '1 error detected in the compilation of "x.cpp".\n'
        parsed = rt.parse_fe_diagnostics(text)
        self.assertEqual(parsed.problems, [])
        self.assertEqual([(d.severity, d.tag(self.reg), d.line, d.col, d.discretionary)
                          for d in parsed.diags],
                         [("error", "cerastes-contract_not_boolean", 7, 3, False),
                          ("warning", "cerastes-unused_binder", 8, None, True)])

    def test_unknown_numbers_have_no_tag(self):
        text = '"x.cpp", line 7 (col. 3): error #9999: new\n'
        (d,) = rt.parse_fe_diagnostics(text).diags
        self.assertEqual(d.tag(self.reg), "#9999")

    def test_unrecognized_column_zero_lines_are_problems(self):
        for line in ("garbage", 'x.cpp:3:4: error: gcc style', '"x.cpp", line 2: boom',
                     '"x.cpp", line 2 (col. 10): fatal error #20: x',
                     '"x.cpp", line 2 (col. 10): error 20: no hash sign',
                     'In file included from "a.h", line 1:',
                     "3 errors detected", "Segmentation fault"):
            with self.subTest(line):
                parsed = rt.parse_fe_diagnostics(REAL_THREE_ERRORS + line + "\n")
                self.assertTrue(any(line in p for p in parsed.problems), (line, parsed.problems))

    def test_a_second_summary_is_a_problem(self):
        parsed = rt.parse_fe_diagnostics(
            REAL_THREE_ERRORS + '1 error detected in the compilation of "x.cpp".\n')
        self.assertTrue(parsed.problems)

    def test_summary_variants(self):
        for text, errors in (('1 error detected in the compilation of "a b.cpp".\n', 1),
                             ('12 errors detected in the compilation of "x".\n', 12),
                             ('2 errors and 1 catastrophic error detected in the compilation '
                              'of "x".\n', 2),
                             ('1 catastrophic error detected in this compilation.\n', 0)):
            with self.subTest(text):
                parsed = rt.parse_fe_diagnostics(text)
                self.assertEqual((parsed.problems, parsed.summary_errors), ([], errors))

    def test_blank_and_indented_lines_never_make_a_diagnostic(self):
        text = ('\n   "x.cpp", line 2 (col. 10): error #20: indented, so a continuation\n'
                '\t"x.cpp", line 3 (col. 1): error #20: tab\n')
        self.assertEqual(rt.parse_fe_diagnostics(text).diags, [])

    def test_crlf_is_not_hidden(self):
        # A carriage return at the end must not let a line match silently.
        parsed = rt.parse_fe_diagnostics('"x.cpp", line 2 (col. 10): error #20: m\r\n')
        self.assertEqual(len(parsed.diags), 1)  # the message keeps the \r


class CompareDiagsTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        write_gen(tmp.name)
        self.reg = rt.TagRegistry.load(tmp.name)
        self.diags = rt.parse_fe_diagnostics(REAL_THREE_ERRORS).diags

    def compare(self, *expected, file="x.cpp"):
        return rt.compare_diags(list(expected), self.diags, self.reg, file)

    def exp(self, sev, tag, line):
        return rt.ExpectedDiag(sev, tag, line)

    def good(self):
        return [self.exp("error", "undefined_identifier", 2),
                self.exp("warning", "attribute_namespace_unrecognized", 4),
                self.exp("error", "exp_primary_expr", 5),
                self.exp("warning", "warning_directive", 6),
                self.exp("error", "not_a_member", 8)]

    def test_exact(self):
        self.assertEqual(self.compare(*self.good()), [])

    def test_order_does_not_matter(self):
        self.assertEqual(self.compare(*reversed(self.good())), [])

    def test_wrong_line(self):
        g = self.good()
        g[0] = self.exp("error", "undefined_identifier", 3)
        (problem,) = self.compare(*g)
        self.assertIn("wrong line: expected error undefined_identifier @ L3, got "
                      "undefined_identifier @ L2:10", problem)
        self.assertIn('identifier "zzz" is undefined', problem)

    def test_wrong_diagnostic_at_the_line(self):
        g = self.good()
        g[2] = self.exp("error", "exp_semicolon", 5)
        (problem,) = self.compare(*g)
        self.assertIn("wrong diagnostic at this line: expected error exp_semicolon @ L5, "
                      "got exp_primary_expr @ L5:19", problem)

    def test_missing(self):
        g = self.good() + [self.exp("error", "exp_semicolon", 20)]
        (problem,) = self.compare(*g)
        self.assertEqual(problem, "missing error: expected exp_semicolon @ L20")
        g = self.good() + [self.exp("warning", "unrecognized_pragma", 20)]
        self.assertEqual(self.compare(*g), ["missing warning: expected unrecognized_pragma @ L20"])

    def test_unexpected(self):
        (problem,) = self.compare(*self.good()[1:])
        self.assertIn("unexpected error: undefined_identifier @ L2:10", problem)
        (problem,) = self.compare(*(self.good()[:1] + self.good()[2:]))
        self.assertIn("unexpected warning: attribute_namespace_unrecognized @ L4:3", problem)

    def test_a_warning_is_not_an_error(self):
        g = self.good()
        g[1] = self.exp("error", "attribute_namespace_unrecognized", 4)
        problems = self.compare(*g)
        self.assertEqual(len(problems), 2, problems)
        self.assertTrue(any("missing error" in p for p in problems))
        self.assertTrue(any("unexpected warning" in p for p in problems))

    def test_multiplicity(self):
        g = self.good() + [self.exp("error", "undefined_identifier", 2)]
        self.assertEqual(len(self.compare(*g)), 1)  # expected twice, reported once
        dup = rt.parse_fe_diagnostics(REAL_THREE_ERRORS).diags
        dup = dup + [dup[0]]
        problems = rt.compare_diags(self.good(), dup, self.reg, "x.cpp")
        self.assertEqual(len(problems), 1)
        self.assertIn("unexpected error", problems[0])
        self.assertEqual(rt.compare_diags(self.good() + [self.good()[0]], dup, self.reg,
                                          "x.cpp"), [])

    def test_other_files_are_unexpected_whatever_they_are(self):
        notes = rt.parse_fe_diagnostics(REAL_NOTES).diags
        problems = rt.compare_diags([
            self.exp("error", "no_matching_function", 5), self.exp("error", "static_assert", 6),
            self.exp("error", "exp_semicolon", 8),
            self.exp("warning", "declared_but_not_referenced", 8)], notes, self.reg, "n.cpp")
        self.assertEqual(len(problems), 2, problems)
        self.assertTrue(all("in inc/h.h" in p for p in problems), problems)
        self.assertTrue(any("undefined_identifier" in p for p in problems))
        self.assertTrue(any("unrecognized_pragma" in p for p in problems))

    def test_the_file_must_be_the_test_file(self):
        problems = self.compare(*self.good(), file="y.cpp")
        self.assertEqual(len(problems), 5 + 5)  # unexpected in x.cpp, and missing

    def test_notes_and_remarks_never_count(self):
        notes = rt.parse_fe_diagnostics(REAL_NOTES).diags
        problems = rt.compare_diags(
            [self.exp("error", "no_matching_function", 5), self.exp("error", "static_assert", 6),
             self.exp("error", "exp_semicolon", 8),
             self.exp("warning", "declared_but_not_referenced", 8)],
            [d for d in notes if d.file == "n.cpp"], self.reg, "n.cpp")
        self.assertEqual(problems, [])
        remark = rt.FeDiag("remark", "20", "x.cpp", 9, 1, "r")
        self.assertEqual(rt.compare_diags(self.good(), self.diags + [remark], self.reg, "x.cpp"),
                         [])


class EvaluateDiagsTests(unittest.TestCase):
    """The pass/fail verdict of one run, with a fake `ToolRun`."""

    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        write_gen(tmp.name)
        self.reg = rt.TagRegistry.load(tmp.name)

    def run_of(self, stderr, code, stdout=""):
        return rt.ToolRun(code, stdout, stderr)

    def exp(self, *diags):
        return rt.Expectation("frontend-diag", [], None, ["c++20"], list(diags))

    GOOD = [rt.ExpectedDiag("error", "undefined_identifier", 2),
            rt.ExpectedDiag("warning", "attribute_namespace_unrecognized", 4),
            rt.ExpectedDiag("error", "exp_primary_expr", 5),
            rt.ExpectedDiag("warning", "warning_directive", 6),
            rt.ExpectedDiag("error", "not_a_member", 8)]

    def evaluate(self, stderr, code, *diags, stdout="", file="x.cpp"):
        return rt.evaluate_diags(self.run_of(stderr, code, stdout), self.exp(*diags),
                                 self.reg, file)

    def test_a_real_run_passes(self):
        self.assertEqual(self.evaluate(REAL_THREE_ERRORS, 2, *self.GOOD), [])

    def test_warnings_only(self):
        warn = [rt.ExpectedDiag("warning", "integer_sign_change", 1),
                rt.ExpectedDiag("warning", "declared_but_not_referenced", 1),
                rt.ExpectedDiag("warning", "unrecognized_pragma", 2)]
        self.assertEqual(self.evaluate(REAL_WARNINGS_ONLY, 0, *warn, file="r.cpp"), [])

    def test_exit_status_must_be_two_when_errors_are_expected(self):
        for code in (0, 1, 4):
            with self.subTest(code):
                problems = self.evaluate(REAL_THREE_ERRORS, code, *self.GOOD)
                self.assertTrue(any("expected 2 (errors expected)" in p for p in problems),
                                problems)

    def test_exit_status_must_be_zero_for_warnings_only(self):
        warn = [rt.ExpectedDiag("warning", "integer_sign_change", 1),
                rt.ExpectedDiag("warning", "declared_but_not_referenced", 1),
                rt.ExpectedDiag("warning", "unrecognized_pragma", 2)]
        problems = self.evaluate(REAL_WARNINGS_ONLY, 2, *warn, file="r.cpp")
        self.assertTrue(any("expected 0 (no errors expected)" in p for p in problems),
                        problems)

    def test_stdout_must_be_empty(self):
        problems = self.evaluate(REAL_THREE_ERRORS, 2, *self.GOOD, stdout="routine f\n")
        self.assertTrue(any("printed on stdout" in p for p in problems), problems)

    def test_the_summary_must_agree(self):
        problems = self.evaluate(REAL_THREE_ERRORS.replace("3 errors", "4 errors"), 2,
                                 *self.GOOD)
        self.assertTrue(any("the summary says 4 errors but 3 were parsed" in p
                            for p in problems), problems)
        problems = self.evaluate(REAL_THREE_ERRORS.replace("3 errors", "2 errors"), 2,
                                 *self.GOOD)
        self.assertTrue(any("the summary says 2" in p for p in problems), problems)

    def test_a_missing_summary_with_errors_is_a_problem(self):
        text = REAL_THREE_ERRORS.rsplit("3 errors", 1)[0]
        problems = self.evaluate(text, 2, *self.GOOD)
        self.assertTrue(any("no `N errors detected` summary" in p for p in problems), problems)

    def test_a_summary_without_errors_is_a_problem(self):
        problems = self.evaluate(REAL_WARNINGS_ONLY + '2 errors detected in the compilation '
                                 'of "r.cpp".\n', 0, rt.ExpectedDiag(
                                     "warning", "integer_sign_change", 1), file="r.cpp")
        self.assertTrue(any("the summary says 2 errors but 0 were parsed" in p
                            for p in problems), problems)

    def test_fatal_diagnostics_always_fail(self):
        problems = self.evaluate(REAL_CATASTROPHIC, 4)
        self.assertTrue(any("catastrophic error always fails" in p for p in problems), problems)
        self.assertTrue(any("terminated" in p for p in problems), problems)
        problems = self.evaluate(REAL_COMMAND_LINE, 4)
        self.assertTrue(any("command-line error always fails" in p for p in problems), problems)
        problems = self.evaluate('"x.cpp", line 3 (col. 1): internal error #1: oops\n'
                                 "Compilation aborted.\n", 4)
        self.assertTrue(any("internal error always fails" in p for p in problems), problems)

    def test_the_error_limit_fails(self):
        problems = self.evaluate(REAL_ERROR_LIMIT, 4, rt.ExpectedDiag(
            "error", "undefined_identifier", 2))
        self.assertTrue(any("error limit" in p for p in problems), problems)

    def test_unrecognized_output_fails_even_when_everything_else_matches(self):
        problems = self.evaluate(REAL_THREE_ERRORS + "internal compiler log: stuff\n", 2,
                                 *self.GOOD)
        self.assertEqual(len(problems), 1)
        self.assertIn("unrecognized front-end output", problems[0])

    def test_crash_and_timeout(self):
        problems = rt.evaluate_diags(rt.ToolRun(-9, "", REAL_THREE_ERRORS),
                                     self.exp(*self.GOOD), self.reg, "x.cpp")
        self.assertTrue(any("crashed (signal 9)" in p for p in problems), problems)
        problems = rt.evaluate_diags(rt.ToolRun(None, error="timed out after 1s"),
                                     self.exp(*self.GOOD), self.reg, "x.cpp")
        self.assertEqual(problems, ["cerastes-fe timed out after 1s"])

    def test_silence_does_not_satisfy_an_expected_error(self):
        problems = self.evaluate("", 0, rt.ExpectedDiag("error", "undefined_identifier", 2))
        self.assertTrue(any("missing error" in p for p in problems), problems)
        self.assertTrue(any("expected 2" in p for p in problems), problems)

    def test_an_unexpected_warning_fails(self):
        problems = self.evaluate(REAL_THREE_ERRORS, 2, *[g for g in self.GOOD
                                                         if g.tag != "warning_directive"])
        self.assertEqual(len(problems), 1)
        self.assertIn("unexpected warning: warning_directive @ L6:2", problems[0])


class DiagCommandTests(unittest.TestCase):
    def test_the_options(self):
        self.assertEqual(rt.DIAG_OPTIONS,
                         ["--brief_diagnostics", "--display_error_number", "--no_colors"])
        for forbidden in ("--no_code_gen", "--cerastes_walk"):
            self.assertNotIn(forbidden, rt.DIAG_OPTIONS)
            self.assertNotIn(forbidden, rt.diag_command(Path("x.cpp"), "c++20"))
        cmd = rt.diag_command(Path("tests/x.cpp"), "c23")
        self.assertEqual(cmd[-5:], ["--c23", "--brief_diagnostics", "--display_error_number",
                                    "--no_colors", str(Path("tests/x.cpp"))])
        self.assertEqual(rt.EDG_ERRORS_STATUS, 2)


class DiagRunCase(FakeFrontEndCase):
    """rt.main() on frontend-error tests with a fake front end that prints the
    test file's `// err:` lines, against a fixture of the tag tables."""

    def setUp(self):
        super().setUp()
        self.gen = write_gen(self.dir.parent / "gen")
        self.set_env(CERASTES_EDG_GEN=str(self.gen))

    def diag_test(self, name, expects, err_text, *directives, suffix_lines=()):
        header = "\n".join(f"// expect: {e}" for e in expects)
        err = [f"// err: {l}" for l in err_text.splitlines(keepends=False)]
        return self.write(name, *err, "// silent", *directives, *suffix_lines, header=header)

    def run_diag(self, name, expects, err_text, *directives, args=()):
        t = self.diag_test(name, expects, err_text, *directives)
        return t, *self.run_harness(*args, t)


def err_lines(text, file):
    """`text` (a recorded run of x.cpp or n.cpp) with the file name replaced."""
    return text.replace('"x.cpp"', f'"{file}"').replace('"n.cpp"', f'"{file}"') \
        .replace('"r.cpp"', f'"{file}"')


class DiagRunTests(DiagRunCase):
    GOOD = ["frontend-error undefined_identifier @ L2",
            "frontend-warning attribute_namespace_unrecognized @ L4",
            "frontend-error exp_primary_expr @ L5",
            "frontend-warning warning_directive @ L6",
            "frontend-error not_a_member @ L8"]

    def three_errors(self, name="x.cpp"):
        # the diagnostics name the test file the way the harness passes it
        return err_lines(REAL_THREE_ERRORS, rt.rel(self.dir / name))

    def run_good(self, expects=None, text=None, *directives, name="x.cpp"):
        return self.run_diag(name, self.GOOD if expects is None else expects,
                             self.three_errors(name) if text is None else text,
                             "// exit: 2", *directives)

    def test_a_real_run_passes(self):
        t, status, out, _ = self.run_good()
        self.assertEqual(status, 0, out)
        self.assertIn(f"PASS  {rt.rel(t)} [c++20]", out)
        self.assertEqual(self.summary(out), "1 passed, 0 failed, 0 skipped")

    def test_wrong_line_fails_and_names_both(self):
        expects = list(self.GOOD)
        expects[0] = "frontend-error undefined_identifier @ L3"
        t, status, out, _ = self.run_good(expects)
        self.assertEqual(status, 1, out)
        self.assertIn("wrong line: expected error undefined_identifier @ L3, got "
                      "undefined_identifier @ L2:10", out)
        self.assertIn('identifier "zzz" is undefined', out)

    def test_wrong_tag_fails(self):
        expects = list(self.GOOD)
        expects[2] = "frontend-error exp_semicolon @ L5"
        _, status, out, _ = self.run_good(expects)
        self.assertEqual(status, 1, out)
        self.assertIn("wrong diagnostic at this line", out)

    def test_missing_unexpected_and_severity_swaps_fail(self):
        for what, expects, needle in (
                ("missing", self.GOOD + ["frontend-error exp_semicolon @ L9"], "missing error"),
                ("unexpected error", self.GOOD[1:], "unexpected error: undefined_identifier"),
                ("unexpected warning", self.GOOD[:1] + self.GOOD[2:],
                 "unexpected warning: attribute_namespace_unrecognized"),
                ("warning declared as error",
                 ["frontend-error attribute_namespace_unrecognized @ L4"] + self.GOOD[:1]
                 + self.GOOD[2:], "missing error")):
            with self.subTest(what):
                _, status, out, _ = self.run_good(expects, name=f"{what.replace(' ', '_')}.cpp",
                                                  text=None)
                self.assertEqual(status, 1, out)
                self.assertIn(needle, out)

    def test_exit_status_is_checked(self):
        t = self.diag_test("a.cpp", self.GOOD, self.three_errors("a.cpp"), "// exit: 0")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("exited with status 0, expected 2", out)
        self.assertNotIn("missing", out)  # the diagnostics themselves matched
        t = self.diag_test("b.cpp", ["frontend-warning unrecognized_pragma @ L2"],
                           err_lines(REAL_WARNINGS_ONLY, rt.rel(self.dir / "b.cpp")).splitlines()[2],
                           "// exit: 2")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("expected 0", out)

    def test_warnings_only_passes_with_status_zero(self):
        name = "w.cpp"
        text = err_lines(REAL_WARNINGS_ONLY, rt.rel(self.dir / name))
        t, status, out, _ = self.run_diag(
            name, ["frontend-warning integer_sign_change @ L1",
                   "frontend-warning declared_but_not_referenced @ L1",
                   "frontend-warning unrecognized_pragma @ L2"], text)
        self.assertEqual(status, 0, out)

    def test_notes_do_not_count_and_other_files_do(self):
        n = self.dir / "n.cpp"
        header = rt.rel(n)
        text = err_lines(REAL_NOTES, header)
        # `inc/h.h` diagnostics are in another file: unexpected
        expects = ["frontend-error no_matching_function @ L5", "frontend-error static_assert @ L6",
                   "frontend-error exp_semicolon @ L8",
                   "frontend-warning declared_but_not_referenced @ L8"]
        _, status, out, _ = self.run_diag("n.cpp", expects, text, "// exit: 2")
        self.assertEqual(status, 1, out)
        self.assertIn("unexpected error (in inc/h.h)", out)
        self.assertIn("unexpected warning (in inc/h.h)", out)
        only_ours = "\n".join(l for l in text.splitlines() if not l.startswith('"inc/'))
        only_ours = only_ours.replace("4 errors", "3 errors")
        _, status, out, _ = self.run_diag("m.cpp", expects, only_ours.replace(
            rt.rel(n), rt.rel(self.dir / "m.cpp")), "// exit: 2")
        self.assertEqual(status, 0, out)

    def test_catastrophic_and_unrecognized_output_fail(self):
        for what, text, code, needle in (
                ("catastrophic", REAL_CATASTROPHIC, 4, "catastrophic error always fails"),
                ("command_line", REAL_COMMAND_LINE, 4, "command-line error always fails"),
                ("garbage", self.three_errors("garbage.cpp") + "something odd\n", 2,
                 "unrecognized front-end output: something odd"),
                ("limit", REAL_ERROR_LIMIT, 4, "error limit")):
            with self.subTest(what):
                expects = self.GOOD if what == "garbage" else ["frontend-error "
                                                               "undefined_identifier @ L2"]
                t = self.diag_test(f"{what}.cpp", expects,
                                   err_lines(text, rt.rel(self.dir / f"{what}.cpp")),
                                   f"// exit: {code}")
                status, out, _ = self.run_harness(t)
                self.assertEqual(status, 1, out)
                self.assertIn(needle, out)

    def test_stdout_must_be_empty(self):
        t = self.diag_test("o.cpp", self.GOOD, self.three_errors("o.cpp"), "// exit: 2",
                           "// out: routine f")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("printed on stdout", out)

    def test_crash_and_timeout_fail(self):
        t = self.diag_test("c.cpp", self.GOOD, self.three_errors("c.cpp"), "// exit: 2",
                           "// crash")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("crashed (signal 9)", out)
        t = self.diag_test("s.cpp", self.GOOD, self.three_errors("s.cpp"), "// exit: 2",
                           "// sleep")
        with mock.patch.object(rt, "FRONTEND_TIMEOUT_S", 1):
            status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("timed out after 1s", out)

    def test_the_command_line_and_the_directory(self):
        # Not silenced: the fake prints its arguments, which fails the test
        # (stdout must be empty) and shows them with -v.
        t = self.write("c.cpp", header="// expect: frontend-warning unrecognized_pragma @ L1")
        status, out, _ = self.run_harness("-v", t)
        self.assertEqual(status, 1, out)
        self.assertIn("printed on stdout", out)
        self.assertIn("options --c++20 --brief_diagnostics --display_error_number "
                      "--no_colors\n", out)
        self.assertIn(f"path {rt.rel(t)}\n", out)
        self.assertIn("cwd root\n", out)
        self.assertNotIn("--cerastes_walk", out)
        self.assertNotIn("--no_code_gen", out)

    def test_modes_are_separate_runs(self):
        t = self.diag_test("m.cpp", self.GOOD, self.three_errors("m.cpp"), "// exit: 2",
                           "// mode: c++20, c23")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 0, out)
        self.assertEqual(self.summary(out), "2 passed, 0 failed, 0 skipped")
        self.assertIn(f"PASS  {rt.rel(t)} [c23]", out)

    def test_an_unknown_tag_is_a_bad_header_not_a_failed_run(self):
        t = self.diag_test("u.cpp", ["frontend-error undefined_identifer @ L2"],
                           self.three_errors("u.cpp"), "// exit: 2")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("bad header: unknown EDG diagnostic tag `undefined_identifer`", out)
        status, out, _ = self.run_harness("--check-headers", t)
        self.assertEqual(status, 1, out)
        self.assertIn("0 of 1 headers ok", out)

    def test_cerastes_tags_before_any_are_defined(self):
        t = self.diag_test("k.cpp", ["frontend-error cerastes-contract_not_boolean @ L2"],
                           self.three_errors("k.cpp"), "// exit: 2")
        with mock.patch.object(rt, "CERASTES_DIAGNOSTICS_DEF", Path("src/nowhere.def")):
            status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("no cerastes diagnostics are defined yet", out)

    def test_a_cerastes_diagnostic_from_the_back_end_matches(self):
        name = "cb.cpp"
        file = rt.rel(self.dir / name)
        text = f'"{file}", line 3 (col. 5): error #cerastes-contract_not_boolean: bad\n' \
               f'1 error detected in the compilation of "{file}".\n'
        t = self.diag_test(name, ["frontend-error cerastes-contract_not_boolean @ L3"], text,
                           "// exit: 2")
        with mock.patch.object(rt, "load_cerastes_tags",
                               return_value={"contract_not_boolean"}):
            status, out, _ = self.run_harness(t)
        self.assertEqual(status, 0, out)
        wrong = self.diag_test("cb2.cpp", ["frontend-error cerastes-contract_not_boolean @ L4"],
                               text.replace(name, "cb2.cpp"), "// exit: 2")
        with mock.patch.object(rt, "load_cerastes_tags",
                               return_value={"contract_not_boolean"}):
            status, out, _ = self.run_harness(wrong)
        self.assertEqual(status, 1, out)
        self.assertIn("wrong line", out)

    def test_missing_tables_are_an_environment_error(self):
        t = self.diag_test("e.cpp", self.GOOD, self.three_errors("e.cpp"), "// exit: 2")
        self.set_env(CERASTES_EDG_GEN=str(self.dir / "nowhere"))
        status, out, err = self.run_harness(t)
        self.assertEqual(status, 2, out)
        self.assertIn("scripts/bootstrap.sh", err)
        with mock.patch.object(rt, "load_env", return_value={
                "CERASTES_FE": "x", "EDG_CLANG_VER_SCRAPE": "0"}):
            status, out, err = self.run_harness(t)
        self.assertEqual(status, 2, out)
        self.assertIn("CERASTES_EDG_GEN", err)

    def test_update_golden_leaves_these_tests_alone(self):
        t = self.diag_test("g.cpp", self.GOOD, self.three_errors("g.cpp"), "// exit: 2")
        status, out, _ = self.run_harness("--update-golden", t)
        self.assertEqual(status, 0, out)
        self.assertFalse((self.dir / "g.cpp.golden").exists())

    def test_the_files_next_to_the_test_are_not_touched(self):
        t = self.diag_test("f.cpp", self.GOOD, self.three_errors("f.cpp"), "// exit: 2")
        before = sorted(p.name for p in self.dir.iterdir())
        self.run_harness(t)
        self.assertEqual(sorted(p.name for p in self.dir.iterdir()), before)


# ---------------------------------------------------------------------------
# Ordinary compilers: `expect: native-ok`
# ---------------------------------------------------------------------------

# A stand-in for a C or C++ compiler.  It logs its arguments (and SDKROOT) to
# $FAKE_LOG; for `--version` it prints a banner ($FAKE_BANNER_<family> or the
# default); otherwise it fails when the source holds `unused` or a
# `// native-fail` directive (`// native-fail: gcc` for one family), prints
# noise on stderr for `// native-noise`, on stdout for `// native-stdout`.
# $FAKE_REJECT_ALL_<family> makes it fail every compile, $FAKE_ACCEPT_ALL_<family>
# makes it accept every compile (a vacuous compiler).
FAKE_COMPILER = '''#!{python}
import os, sys
FAMILY = {family!r}
BANNER = {banner!r}
args = sys.argv[1:]
log = os.environ.get("FAKE_LOG")
if log:
    with open(log, "a") as f:
        f.write(" ".join([os.path.basename(sys.argv[0]),
                          "SDKROOT=" + os.environ.get("SDKROOT", "-")] + args) + "\\n")
if args == ["--version"]:
    print(os.environ.get("FAKE_BANNER_" + FAMILY, BANNER))
    sys.exit(0)
src = open(args[-1]).read()
def has(directive):
    return ("// " + directive + "\\n") in src or ("// " + directive + ": " + FAMILY + "\\n") in src
if os.environ.get("FAKE_REJECT_ALL_" + FAMILY):
    print("fake: error: rejects everything", file=sys.stderr)
    sys.exit(1)
if os.environ.get("FAKE_ACCEPT_ALL_" + FAMILY):
    sys.exit(0)
if "unreg_ns" in src and not os.environ.get("FAKE_ACCEPT_ATTR_" + FAMILY):
    print("fake: error: unknown attribute", file=sys.stderr)
    sys.exit(1)
if "unused" in src or has("native-fail"):
    print("fake: error: unused variable", file=sys.stderr)
    sys.exit(1)
if has("native-noise"):
    print("fake: warning: noise", file=sys.stderr)
if has("native-stdout"):
    print("fake: stdout noise")
sys.exit(0)
'''

REAL_BANNERS = {
    "Apple clang": "Apple clang version 21.0.0 (clang-2100.3.34.2)\nTarget: arm64-apple-darwin27.0.0\n",
    "GCC": "g++-16 (Homebrew GCC 16.2.0) 16.2.0\nCopyright (C) 2026 Free Software Foundation, Inc.\n",
}


class BannerTests(unittest.TestCase):
    def test_real_banners(self):
        self.assertIsNone(rt.banner_problem("Apple clang", REAL_BANNERS["Apple clang"]))
        self.assertIsNone(rt.banner_problem("GCC", REAL_BANNERS["GCC"]))
        self.assertIsNone(rt.banner_problem("GCC", "gcc (GCC) 14.1.0\n"))
        self.assertIsNone(rt.banner_problem("GCC", "gcc (Ubuntu 13.2.0-23ubuntu4) 13.2.0 GCC\n"))

    def test_the_wrong_family(self):
        self.assertIn("does not say Apple clang", rt.banner_problem(
            "Apple clang", "clang version 18.1.8 (Homebrew)\n"))
        self.assertIn("does not say Apple clang", rt.banner_problem(
            "Apple clang", REAL_BANNERS["GCC"]))
        # /usr/bin/gcc on macOS is Apple clang
        self.assertIn("does not say GCC", rt.banner_problem("GCC", REAL_BANNERS["Apple clang"]))
        self.assertIn("does not say GCC", rt.banner_problem(
            "GCC", "Apple clang version 21 (GCC compatible)\n"))
        self.assertIn("does not say", rt.banner_problem("GCC", ""))
        self.assertIn("does not say", rt.banner_problem("Apple clang", "\n"))

    def test_only_the_first_line_counts(self):
        self.assertIsNotNone(rt.banner_problem("GCC", "tool 1.0\nbuilt with GCC\n"))


class NativeCommandTests(unittest.TestCase):
    def test_the_flags(self):
        cmd = rt.native_command("/x/clang++", "c++", "c++20", "/inc", "t/a.cpp")
        self.assertEqual(cmd, ["/x/clang++", "-x", "c++", "-std=c++20", "-Wall", "-Wextra",
                               "-Werror", "-U__CERASTES__", "-I", "/inc", "-c", "-o",
                               "/dev/null", "t/a.cpp"])
        cmd = rt.native_command("/x/clang", "c", "c23", "/inc", "t/a.c")
        self.assertEqual(cmd[1:4], ["-x", "c", "-std=c23"])

    def test_the_canary_snippets(self):
        # valid C and C++; one of them is rejected only by the warning flags
        self.assertIn("unused", rt.CANARY_FAIL)
        self.assertNotIn("unused", rt.CANARY_PASS)


class NativeCase(FakeFrontEndCase):
    """rt.main() on native-ok tests with fake compilers and a fake front end."""

    FAMILIES = (("clang", "Apple clang version 99 (fake)"), ("gcc", "fake-g++ (GCC) 99.0"))

    def setUp(self):
        super().setUp()
        bin_dir = self.dir.parent / "bin"
        bin_dir.mkdir()
        self.bin = {}
        for family, banner in self.FAMILIES:
            for kind in ("cc", "cxx"):
                exe = bin_dir / f"fake_{family}_{kind}"
                exe.write_text(FAKE_COMPILER.format(python=sys.executable, family=family,
                                                    banner=banner))
                exe.chmod(0o755)
                self.bin[family, kind] = str(exe)
        self.log = self.dir.parent / "compiler.log"
        self.set_env(CERASTES_CLANG_CC=self.bin["clang", "cc"],
                     CERASTES_CLANG_CXX=self.bin["clang", "cxx"],
                     CERASTES_GCC_CC=self.bin["gcc", "cc"],
                     CERASTES_GCC_CXX=self.bin["gcc", "cxx"],
                     CERASTES_INCLUDE_DIR="/fake/include")
        # (the FAKE_* switches of the fake compilers are not env.sh variables)
        patcher = mock.patch.dict(os.environ, {"FAKE_LOG": str(self.log)})
        patcher.start()
        self.addCleanup(patcher.stop)
        os.environ.pop("SDKROOT", None)
        for family in ("clang", "gcc"):
            for name in ("BANNER", "ACCEPT_ALL", "REJECT_ALL", "ACCEPT_ATTR"):
                os.environ.pop(f"FAKE_{name}_{family}", None)
        sdk = mock.patch.object(rt, "sdk_root", return_value="/fake/sdk")
        sdk.start()
        self.addCleanup(sdk.stop)

    def native_test(self, name, *lines, extra="// expect: native-ok",
                    outcome="// expect: golden"):
        t = self.write(name, *lines, header=f"{outcome}\n{extra}")
        before = self.calls()
        status, out, _ = self.run_harness("--update-golden", t)
        self.assertEqual(status, 0, out)
        self.assertEqual(self.calls(), before, "--update-golden must not run the compilers")
        return t

    def calls(self):
        if not self.log.exists():
            return []
        return self.log.read_text().splitlines()

    def compiles_of(self, t):
        return [c for c in self.calls() if c.endswith(" " + rt.rel(t))]


class NativeRunTests(NativeCase):
    def test_flags_files_and_both_compilers(self):
        t = self.native_test("a.cpp", "int f();")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 0, out)
        self.assertIn(f"PASS  {rt.rel(t)} [c++20] native-ok: Apple clang", out)
        self.assertIn(f"PASS  {rt.rel(t)} [c++20] native-ok: GCC", out)
        self.assertTrue(self.summary(out).endswith("native-ok runs: Apple clang 1, GCC 1"),
                        self.summary(out))
        flags = ("-x c++ -std=c++20 -Wall -Wextra -Werror -U__CERASTES__ -I /fake/include "
                 f"-c -o /dev/null {rt.rel(t)}")
        self.assertEqual(self.compiles_of(t), [
            f"fake_clang_cxx SDKROOT=/fake/sdk {flags}", f"fake_gcc_cxx SDKROOT=- {flags}"])

    def test_c_files_use_the_c_compilers_and_c23(self):
        t = self.native_test("a.c", "int f(void);")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 0, out)
        flags = ("-x c -std=c23 -Wall -Wextra -Werror -U__CERASTES__ -I /fake/include "
                 f"-c -o /dev/null {rt.rel(t)}")
        self.assertEqual(self.compiles_of(t), [
            f"fake_clang_cc SDKROOT=/fake/sdk {flags}", f"fake_gcc_cc SDKROOT=- {flags}"])
        self.assertIn("native-ok: Apple clang", out)

    def test_each_mode_is_compiled_in_its_language(self):
        t = self.native_test("m.cpp", "int f();", extra="// expect: native-ok\n// mode: c++20, c23")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 0, out)
        calls = self.compiles_of(t)
        self.assertEqual(len(calls), 4, calls)
        self.assertEqual(sum("-x c++ -std=c++20" in c for c in calls), 2)
        self.assertEqual(sum("-x c -std=c23" in c for c in calls), 2)
        self.assertTrue(self.summary(out).endswith("native-ok runs: Apple clang 2, GCC 2"))

    def test_a_rejected_file_fails_only_that_compiler(self):
        t = self.native_test("g.cpp", "int f();", "// native-fail: gcc")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn(f"FAIL  {rt.rel(t)} [c++20] native-ok: GCC", out)
        self.assertIn(f"PASS  {rt.rel(t)} [c++20] native-ok: Apple clang", out)
        self.assertIn("GCC rejected the file (exit status 1)", out)
        self.assertIn("fake: error: unused variable", out)
        self.assertIn(f"PASS  {rt.rel(t)} [c++20]\n", out)  # the golden itself still passes

    def test_both_reject(self):
        t = self.native_test("b.cpp", "int f();", "// native-fail")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertEqual(out.count("FAIL  "), 2)

    def test_noise_on_a_zero_exit_fails(self):
        t = self.native_test("n.cpp", "int f();", "// native-noise: clang")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("Apple clang printed diagnostics", out)
        self.assertIn("fake: warning: noise", out)
        self.assertIn(f"PASS  {rt.rel(t)} [c++20] native-ok: GCC", out)

    def test_stdout_noise_fails_too(self):
        t = self.native_test("s.cpp", "int f();", "// native-stdout: gcc")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("GCC printed diagnostics", out)
        self.assertIn("fake: stdout noise", out)

    def test_it_accompanies_a_front_end_diagnostic_test(self):
        gen = write_gen(self.dir.parent / "gen")
        self.set_env(CERASTES_EDG_GEN=str(gen))
        if True:
            t = self.write("d.cpp", "// err: x", "// silent",
                           header="// expect: frontend-warning unrecognized_pragma @ L1\n"
                                  "// expect: native-ok")
            # the fake prints a diagnostic naming the file the way the harness passes it
            t.write_text(t.read_text().replace(
                "// err: x", f'// err: "{rt.rel(t)}", line 1 (col. 9): warning #161-D: '
                            'unrecognized #pragma'))
            status, out, _ = self.run_harness(t)
        self.assertEqual(status, 0, out)
        self.assertIn("native-ok: GCC", out)

    def test_a_failing_outcome_still_runs_the_native_check(self):
        t = self.native_test("o.cpp", "int f();", "// native-fail: gcc")
        (self.dir / "o.cpp.golden").write_text("different\n")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn(f"FAIL  {rt.rel(t)} [c++20]\n", out)
        self.assertIn("native-ok: GCC", out)

    def test_no_native_tests_no_compilers(self):
        t = self.write("p.cpp", "int f();")
        self.run_harness("--update-golden", t)
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 0, out)
        self.assertEqual(self.calls(), [])
        self.assertNotIn("native-ok", out)
        self.assertNotIn("canary", out)

    def test_check_headers_runs_no_compiler(self):
        t = self.native_test("h.cpp", "int f();")
        status, out, _ = self.run_harness("--check-headers", t)
        self.assertEqual(status, 0, out)
        self.assertEqual(self.calls(), [])

    def test_a_bad_header_is_not_compiled(self):
        t = self.write("bad.cpp", "int f();", header="// expect: native-ok")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("bad header", out)
        self.assertEqual(self.calls(), [])

    def test_update_golden_runs_no_compiler(self):
        t = self.write("u.cpp", "int f();", header="// expect: golden\n// expect: native-ok")
        status, out, _ = self.run_harness("--update-golden", t)
        self.assertEqual(status, 0, out)
        self.assertNotIn("canary", out)
        self.assertEqual(self.calls(), [])

    def test_sdkroot(self):
        t = self.native_test("k.cpp", "int f();")
        self.run_harness(t)
        calls = self.compiles_of(t)
        self.assertTrue(calls[0].startswith("fake_clang_cxx SDKROOT=/fake/sdk "), calls)
        self.assertTrue(calls[1].startswith("fake_gcc_cxx SDKROOT=- "), calls)
        self.log.unlink()
        with mock.patch.dict(os.environ, {"SDKROOT": "/user/sdk"}):
            self.run_harness(t)
        calls = self.compiles_of(t)
        self.assertTrue(calls[0].startswith("fake_clang_cxx SDKROOT=/user/sdk "), calls)
        self.assertTrue(calls[1].startswith("fake_gcc_cxx SDKROOT=/user/sdk "), calls)


class NativeCanaryTests(NativeCase):
    def test_canaries_run_once_per_compiler_and_language(self):
        a = self.native_test("a.cpp", "int f();")
        b = self.native_test("b.cpp", "int f();")
        status, out, _ = self.run_harness(a, b)
        self.assertEqual(status, 0, out)
        self.assertIn("PASS  canary native-ok [Apple clang, c++]", out)
        self.assertIn("PASS  canary native-ok [GCC, c++]", out)
        self.assertNotIn("[Apple clang, c]", out)  # no C test, no C canary
        calls = self.calls()
        self.assertEqual(sum(c.endswith(" --version") for c in calls), 2)
        # per compiler: a clean snippet and one with an unused variable
        self.assertEqual(sum("canary_pass.cpp" in c for c in calls), 2)
        self.assertEqual(sum("canary_fail.cpp" in c for c in calls), 2)
        self.assertEqual(sum("canary_attribute.cpp" in c for c in calls), 2)
        self.assertEqual(sum("--version" in c for c in calls), 2)
        self.assertEqual(len(self.compiles_of(a)) + len(self.compiles_of(b)), 4)
        self.assertTrue(out.index("canary native-ok [Apple clang") < out.index(rt.rel(a)))

    def test_a_c_test_adds_the_c_canary(self):
        t = self.native_test("c.c", "int f(void);")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 0, out)
        self.assertIn("PASS  canary native-ok [Apple clang, c]", out)
        self.assertIn("canary_fail.c ", " ".join(self.calls()) + " ")

    def canary_failure(self, env, family_label, needle):
        t = self.native_test("v.cpp", "int f();")
        self.set_env(**{k: v for k, v in env.items() if k.startswith("CERASTES_")})
        with mock.patch.dict(os.environ, {k: v for k, v in env.items()
                                          if not k.startswith("CERASTES_")}):
            status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn(f"FAIL  canary native-ok [{family_label}, c++]", out)
        self.assertIn(needle, out)
        # the file's own native result is not trusted
        self.assertIn(f"FAIL  {rt.rel(t)} [c++20] native-ok: {family_label}", out)
        self.assertIn(f"the {family_label} canary failed", out)
        return out

    def test_a_banner_of_the_wrong_family(self):
        self.canary_failure({"FAKE_BANNER_clang": "clang version 18 (Homebrew)"},
                            "Apple clang", "does not say Apple clang")
        self.canary_failure({"FAKE_BANNER_gcc": "Apple clang version 21 (clang-2100)"},
                            "GCC", "does not say GCC")

    def test_a_compiler_that_accepts_everything(self):
        out = self.canary_failure({"FAKE_ACCEPT_ALL_gcc": "1"}, "GCC",
                                  "an unused variable was not rejected")
        self.assertIn("PASS  canary native-ok [Apple clang, c++]", out)
        self.canary_failure({"FAKE_ACCEPT_ALL_clang": "1"}, "Apple clang",
                            "an unused variable was not rejected")

    def test_a_compiler_that_accepts_an_unknown_attribute(self):
        # -Werror must reject [[unreg_ns::x]]; one that does not would let a
        # `[[cerastes::...]]` left in a file pass as ordinary code
        out = self.canary_failure({"FAKE_ACCEPT_ATTR_gcc": "1"}, "GCC",
                                  "an unknown scoped attribute ([[unreg_ns::x]]) was not rejected")
        self.assertIn("PASS  canary native-ok [Apple clang, c++]", out)
        self.canary_failure({"FAKE_ACCEPT_ATTR_clang": "1"}, "Apple clang",
                            "an unknown scoped attribute")

    def test_the_c_canary_has_the_attribute_snippet_too(self):
        t = self.native_test("c.c", "int f(void);")
        self.run_harness(t)
        self.assertTrue(any("canary_attribute.c " in c + " " for c in self.calls()))
        os.environ["FAKE_ACCEPT_ATTR_clang"] = "1"
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("FAIL  canary native-ok [Apple clang, c]", out)

    def test_a_compiler_that_rejects_everything(self):
        self.canary_failure({"FAKE_REJECT_ALL_clang": "1"}, "Apple clang",
                            "a clean snippet did not compile")

    def test_a_compiler_that_cannot_be_run(self):
        gone = str(self.dir.parent / "bin" / "nope")
        self.canary_failure({"CERASTES_CLANG_CXX": gone}, "Apple clang", "could not be run")

    def test_a_broken_compiler_never_makes_a_failing_file_pass(self):
        # An accept-everything GCC would let this file through; the canary
        # catches the compiler, so the run still fails.
        t = self.native_test("w.cpp", "int f();", "// native-fail: gcc")
        with mock.patch.dict(os.environ, {"FAKE_ACCEPT_ALL_gcc": "1"}):
            status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("FAIL  canary native-ok [GCC, c++]", out)

    def test_a_good_canary_hides_nothing(self):
        t = self.native_test("x.cpp", "int f();", "// native-fail: gcc")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("PASS  canary native-ok [GCC, c++]", out)
        self.assertIn(f"FAIL  {rt.rel(t)} [c++20] native-ok: GCC", out)


class NativeEnvironmentTests(NativeCase):
    def test_gcc_not_installed_is_one_note(self):
        a = self.native_test("a.cpp", "int f();")
        b = self.native_test("b.cpp", "int f();")
        self.set_env(CERASTES_GCC_CC="", CERASTES_GCC_CXX="")
        status, out, _ = self.run_harness(a, b)
        self.assertEqual(status, 0, out)
        self.assertEqual(out.count("native-ok: GCC not installed; Apple clang only"), 1)
        self.assertTrue(self.summary(out).endswith("native-ok runs: Apple clang 2"),
                        self.summary(out))
        self.assertNotIn("GCC", out.replace("native-ok: GCC not installed", ""))
        self.assertFalse(any(c.startswith("fake_gcc") for c in self.calls()))
        self.assertEqual(rt.GCC_MISSING_NOTE, "native-ok: GCC not installed; Apple clang only")

    def test_a_named_gcc_that_is_missing_fails(self):
        t = self.native_test("a.cpp", "int f();")
        gone = str(self.dir.parent / "bin" / "nope")
        self.set_env(CERASTES_GCC_CC=gone, CERASTES_GCC_CXX=gone)
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertNotIn("GCC not installed", out)
        self.assertIn("FAIL  canary native-ok [GCC, c++]", out)
        self.assertIn(f"FAIL  {rt.rel(t)} [c++20] native-ok: GCC", out)
        self.assertIn(f"PASS  {rt.rel(t)} [c++20] native-ok: Apple clang", out)

    def run_with_env(self, env):
        t = self.native_test("e.cpp", "int f();")
        with mock.patch.object(rt, "load_env", return_value=env):
            return self.run_harness(t)

    def base_env(self):
        return {"CERASTES_FE": self.env_values["CERASTES_FE"], "EDG_CLANG_VER_SCRAPE": "0",
                "CERASTES_CLANG_CC": self.bin["clang", "cc"],
                "CERASTES_CLANG_CXX": self.bin["clang", "cxx"],
                "CERASTES_GCC_CC": "", "CERASTES_GCC_CXX": "",
                "CERASTES_INCLUDE_DIR": "/fake/include"}

    def test_the_complete_environment_works(self):
        status, out, _ = self.run_with_env(self.base_env())
        self.assertEqual(status, 0, out)

    def test_missing_variables_are_environment_errors(self):
        for name in ("CERASTES_CLANG_CC", "CERASTES_CLANG_CXX", "CERASTES_INCLUDE_DIR",
                     "CERASTES_GCC_CC", "CERASTES_GCC_CXX"):
            with self.subTest(name):
                env = self.base_env()
                del env[name]
                status, out, err = self.run_with_env(env)
                self.assertEqual(status, 2, out)
                self.assertIn(name, err)
                self.assertEqual(self.calls(), [])

    def test_empty_clang_is_an_environment_error(self):
        env = self.base_env()
        env["CERASTES_CLANG_CXX"] = ""
        status, out, err = self.run_with_env(env)
        self.assertEqual(status, 2, out)
        self.assertIn("CERASTES_CLANG_CXX not set", err)

    def test_half_a_gcc_is_an_environment_error(self):
        for half in ("CERASTES_GCC_CC", "CERASTES_GCC_CXX"):
            with self.subTest(half):
                env = self.base_env()
                env[half] = self.bin["gcc", "cxx"]
                status, out, err = self.run_with_env(env)
                self.assertEqual(status, 2, out)
                self.assertIn("both be set or both empty", err)


# ---------------------------------------------------------------------------
# The EDG tree (decision 0005)
# ---------------------------------------------------------------------------

class MemoisedStampTests(unittest.TestCase):
    """The memo that the real end-to-end classes use never hides a call that has
    arguments (the fixture trees), and computes the real answer only once."""

    def test_the_real_answer_is_computed_once_and_calls_with_arguments_never_are_memoised(self):
        calls = []

        def original(*args, **kwargs):
            calls.append((args, kwargs))
            return f"answer {len(calls)}"

        with mock.patch.object(sys.modules[__name__], "_REAL_EDG_STAMP_PROBLEM", original), \
                mock.patch.object(sys.modules[__name__], "_real_stamp_memo", []):
            self.assertEqual(memoised_real_stamp_problem(), "answer 1")
            self.assertEqual(memoised_real_stamp_problem(), "answer 1")
            self.assertEqual(len(calls), 1)
            self.assertEqual(memoised_real_stamp_problem("root", {}), "answer 2")
            self.assertEqual(memoised_real_stamp_problem("root", {}), "answer 3")
            self.assertEqual(memoised_real_stamp_problem(root="r"), "answer 4")
            self.assertEqual(memoised_real_stamp_problem(), "answer 1")

    def test_only_the_real_end_to_end_classes_use_it(self):
        source = Path(__file__).read_text()
        users = re.findall(r"^class (\w+)\(MemoisedStamp, ", source, re.M)
        self.assertEqual(sorted(users), ["RealDiagnosticTests", "RealFrontEndGoldenTests",
                                         "RealNativeTests", "RealRefusedTests"])
        # the classes that check the stamp itself run the real function
        for name in ("TreeScriptTwinTests", "TreeGateTests", "EdgTreeToolTests"):
            self.assertNotRegex(source, rf"class {name}\(MemoisedStamp")


class TreeConsistencyTests(RootedCase):
    def setUp(self):
        super().setUp()
        self.pin = self.root / "third_party" / "edg"
        self.pin.mkdir(parents=True)
        self.other = self.root / "other-edg"
        self.other.mkdir()

    def env_sh(self, active):
        lines = ["# Generated by scripts/bootstrap.sh; do not edit.",
                 'export CERASTES_JAVA="/x/java"']
        if active is not None:
            lines.append(f'export CERASTES_EDG_SRC_ACTIVE="{active}"')
        self.put("build/env.sh", "\n".join(lines) + "\n")

    def problem(self, caller):
        return rt.edg_tree_problem(self.root / "build" / "env.sh", caller)

    def test_the_pin(self):
        self.env_sh(self.pin)
        self.assertIsNone(self.problem({}))
        self.assertIsNone(self.problem({"CERASTES_EDG_SRC": ""}))
        self.assertIsNone(self.problem({"CERASTES_EDG_SRC": str(self.pin)}))

    def test_a_relative_selection_is_refused(self):
        for value in ("rel/edg", "./edg", "../edg", "~/edg", "edg"):
            with self.subTest(value):
                self.env_sh(self.pin)
                msg = self.problem({"CERASTES_EDG_SRC": value})
                self.assertEqual(msg, f"CERASTES_EDG_SRC={value} is a relative path; use an "
                                      "absolute path (decision 0005)")
        # refused even where there is no env.sh to compare with
        (self.root / "build" / "env.sh").unlink()
        self.assertIn("is a relative path", self.problem({"CERASTES_EDG_SRC": "rel"}))
        self.assertIsNone(self.problem({"CERASTES_EDG_SRC": "/abs"}))

    def test_an_override(self):
        self.env_sh(self.other)
        self.assertIsNone(self.problem({"CERASTES_EDG_SRC": str(self.other)}))

    def test_a_disagreement_is_reported_in_the_contract_words(self):
        self.env_sh(self.pin)
        msg = self.problem({"CERASTES_EDG_SRC": str(self.other)})
        self.assertEqual(msg, f"build/env.sh is for {self.pin} but CERASTES_EDG_SRC selects "
                              f"{self.other}; run scripts/bootstrap.sh")
        self.env_sh(self.other)
        msg = self.problem({})
        self.assertEqual(msg, f"build/env.sh is for {self.other} but CERASTES_EDG_SRC selects "
                              f"{self.pin}; run scripts/bootstrap.sh")
        self.assertEqual(self.problem({"CERASTES_EDG_SRC": ""}), msg)
        self.assertIsNotNone(self.problem({"CERASTES_EDG_SRC": str(self.pin)}))

    def test_paths_are_compared_by_realpath(self):
        self.env_sh(self.other)
        link = self.root / "link"
        link.symlink_to(self.other)
        self.assertIsNone(self.problem({"CERASTES_EDG_SRC": str(link)}))
        self.assertIsNone(self.problem({"CERASTES_EDG_SRC": f"{self.other}/"}))
        self.assertIsNone(self.problem({"CERASTES_EDG_SRC": f"{self.other}/../{self.other.name}"}))
        # and an env.sh written with a link in it
        self.env_sh(link)
        self.assertIsNone(self.problem({"CERASTES_EDG_SRC": str(self.other)}))
        self.assertIsNotNone(self.problem({}))

    def test_a_prefix_is_not_the_same_tree(self):
        self.env_sh(self.other)
        self.assertIsNotNone(self.problem({"CERASTES_EDG_SRC": f"{self.other}-2"}))
        self.assertIsNotNone(self.problem({"CERASTES_EDG_SRC": str(self.other.parent)}))

    def test_an_env_sh_that_does_not_name_its_tree(self):
        self.env_sh(None)
        msg = self.problem({})
        self.assertIn("does not name its EDG tree", msg)
        self.assertIn("scripts/bootstrap.sh", msg)
        self.env_sh("")
        self.assertIn("does not name its EDG tree", self.problem({}))

    def test_no_env_sh_is_left_to_the_other_checks(self):
        self.assertIsNone(self.problem({"CERASTES_EDG_SRC": str(self.other)}))

    def test_the_default_arguments_read_the_shell_and_the_root(self):
        self.env_sh(self.pin)
        with mock.patch.dict(os.environ, {"CERASTES_EDG_SRC": str(self.other)}):
            self.assertIn("CERASTES_EDG_SRC selects", rt.edg_tree_problem())
        env = {k: v for k, v in os.environ.items() if k != "CERASTES_EDG_SRC"}
        with mock.patch.dict(os.environ, env, clear=True):
            self.assertIsNone(rt.edg_tree_problem())

    def test_the_caller_selects_not_env_sh(self):
        # `CERASTES_EDG_SRC` exported by env.sh itself must not hide a mismatch.
        self.put("build/env.sh", f'export CERASTES_EDG_SRC_ACTIVE="{self.pin}"\n'
                                 f'export CERASTES_EDG_SRC="{self.pin}"\n')
        self.assertIsNotNone(self.problem({"CERASTES_EDG_SRC": str(self.other)}))


class TreeGateTests(FakeFrontEndCase):
    """rt.main() stops with status 2 before running anything when the tree
    does not match, but only for tests that run tools built from the tree."""

    def test_a_mismatch_stops_a_golden_run(self):
        t = self.write("x.cpp", "// out: a")
        self.tree_mock.return_value = ("build/env.sh is for /a but CERASTES_EDG_SRC selects "
                                       "/b; run scripts/bootstrap.sh")
        status, out, err = self.run_harness(t)
        self.assertEqual(status, 2, out)
        self.assertIn("run_tests.py: build/env.sh is for /a but CERASTES_EDG_SRC selects /b", err)
        self.assertEqual(out, "")
        self.assertFalse((self.dir / "x.cpp.golden").exists())
        status, out, err = self.run_harness("--update-golden", t)
        self.assertEqual(status, 2, out)

    def test_it_stops_other_front_end_tests_too(self):
        self.tree_mock.return_value = "mismatch"
        t = self.write("f.cpp", header="// expect: frontend-ok")
        self.assertEqual(self.run_harness(t)[0], 2)
        gen = write_gen(self.dir.parent / "gen")
        self.set_env(CERASTES_EDG_GEN=str(gen))
        d = self.write("d.cpp", header="// expect: frontend-warning unrecognized_pragma @ L1")
        self.assertEqual(self.run_harness(d)[0], 2)

    def test_a_matching_tree_runs(self):
        t = self.write("x.cpp", "// out: a")
        self.run_harness("--update-golden", t)
        self.tree_mock.return_value = None
        self.assertEqual(self.run_harness(t)[0], 0)
        self.assertTrue(self.tree_mock.called)

    def test_header_checks_never_need_the_tree(self):
        t = self.write("x.cpp", "// out: a")
        self.run_harness("--update-golden", t)
        self.tree_mock.reset_mock()
        self.tree_mock.return_value = "mismatch"
        status, out, _ = self.run_harness("--check-headers", t)
        self.assertEqual(status, 0, out)
        self.assertFalse(self.tree_mock.called)

    def test_a_stale_build_stops_the_run_too(self):
        t = self.write("x.cpp", "// out: a")
        self.run_harness("--update-golden", t)
        self.stamp_mock.reset_mock()
        self.stamp_mock.return_value = ("the binary under test was built from a different EDG "
                                        "tree state than the active one; run "
                                        "scripts/bootstrap.sh")
        for args in ((t,), ("--update-golden", t)):
            status, out, err = self.run_harness(*args)
            self.assertEqual(status, 2, out)
            self.assertIn("run_tests.py: the binary under test was built from a different EDG "
                          "tree state", err)
            self.assertIn("run scripts/bootstrap.sh", err)
        self.assertTrue(self.stamp_mock.called)
        # headers need no binary
        self.stamp_mock.reset_mock()
        self.assertEqual(self.run_harness("--check-headers", t)[0], 0)
        self.assertFalse(self.stamp_mock.called)
        # a run that needs no EDG tool does not look at the stamp (a .vpr-only
        # selection has none here: the gate is needs_edg_tools)
        self.stamp_mock.return_value = None
        self.assertEqual(self.run_harness(t)[0], 0)

    def test_the_tree_problem_comes_before_the_stamp_problem(self):
        t = self.write("x.cpp", "// out: a")
        self.tree_mock.return_value = "tree mismatch"
        self.stamp_mock.return_value = "stale stamp"
        status, out, err = self.run_harness(t)
        self.assertEqual(status, 2, out)
        self.assertIn("tree mismatch", err)
        self.assertNotIn("stale stamp", err)

    def test_needs_edg_tools(self):
        def exp(kind):
            return rt.Expectation(kind, [], None, ["c++20"])
        for kind in ("golden", "frontend-ok", "frontend-diag"):
            self.assertTrue(rt.needs_edg_tools({Path("a.cpp"): exp(kind)}), kind)
        for kind in ("verified", "error", "rejected"):
            self.assertFalse(rt.needs_edg_tools({Path("a.cpp"): exp(kind)}), kind)
        self.assertFalse(rt.needs_edg_tools({Path("a.cpp"): rt.HeaderError("x")}))
        self.assertFalse(rt.needs_edg_tools({}))


def _env_sh_names_its_tree():
    env_sh = REAL_ROOT / "build" / "env.sh"
    return (env_sh.is_file() and "CERASTES_EDG_SRC_ACTIVE=" in env_sh.read_text()
            and (REAL_ROOT / "scripts" / "cerastes-fe.sh").is_file())


@e2e_gate(_env_sh_names_its_tree(),
          "build/env.sh missing or too old (no CERASTES_EDG_SRC_ACTIVE); run scripts/bootstrap.sh")
class TreeScriptTwinTests(unittest.TestCase):
    """The real scripts refuse a tree other than the one build/env.sh is for:
    cerastes-fe.sh, cpfe.sh and run_tests.py exit 2 with the message, and the
    same command with the matching selection is not refused."""

    def setUp(self):
        text = (REAL_ROOT / "build" / "env.sh").read_text()
        self.active = re.search(r'^export CERASTES_EDG_SRC_ACTIVE="(.*)"$', text, re.M).group(1)
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.elsewhere = str(Path(tmp.name).resolve())

    def run_script(self, args, selection):
        env = {k: v for k, v in os.environ.items() if k != "CERASTES_EDG_SRC"}
        if selection is not None:
            env["CERASTES_EDG_SRC"] = selection
        return subprocess.run(args, cwd=REAL_ROOT, env=env, capture_output=True, text=True,
                              timeout=300)

    def test_scripts_refuse_a_different_tree(self):
        hello = "tests/edg-smoke/hello.cpp"
        for args in ([str(REAL_ROOT / "scripts" / "cerastes-fe.sh"), "--c++20", hello],
                     [str(REAL_ROOT / "scripts" / "cpfe.sh"), "--c++20", "--no_code_gen",
                      hello]):
            with self.subTest(args[0]):
                run = self.run_script(args, self.elsewhere)
                self.assertEqual(run.returncode, 2, run.stderr)
                self.assertIn("CERASTES_EDG_SRC selects", run.stderr)
                self.assertIn(self.elsewhere, run.stderr)
                self.assertIn("scripts/bootstrap.sh", run.stderr)
                # the matching selection is accepted (twin: the check is not vacuous)
                run = self.run_script(args, self.active)
                self.assertEqual((run.returncode, run.stderr), (0, ""), run)

    def test_scripts_refuse_a_relative_selection(self):
        hello = "tests/edg-smoke/hello.cpp"
        for args in ([str(REAL_ROOT / "scripts" / "cerastes-fe.sh"), "--c++20", hello],
                     [str(REAL_ROOT / "scripts" / "cpfe.sh"), "--c++20", "--no_code_gen", hello],
                     [str(REAL_ROOT / "scripts" / "bootstrap.sh"), "--build-only"],
                     [sys.executable, str(REAL_ROOT / "scripts" / "run_tests.py"), hello],
                     [sys.executable, str(REAL_ROOT / "scripts" / "edg_tree.py"), "check"],
                     [sys.executable, str(REAL_ROOT / "scripts" / "edg_tree.py"), "resolve"]):
            with self.subTest(Path(args[0]).name + " " + Path(args[-1]).name):
                run = self.run_script(args, "third_party/edg")
                self.assertEqual(run.returncode, 2, run.stdout + run.stderr)
                self.assertIn("relative path", run.stderr)
                self.assertIn("absolute", run.stderr)

    def test_the_real_build_verifies(self):
        self.assertIsNone(rt.edg_stamp_problem())

    def test_run_tests_refuses_a_different_tree(self):
        cmd = [sys.executable, str(REAL_ROOT / "scripts" / "run_tests.py"),
               "tests/edg-smoke/hello.cpp"]
        run = self.run_script(cmd, self.elsewhere)
        self.assertEqual(run.returncode, 2, run.stdout + run.stderr)
        self.assertIn("run_tests.py: build/env.sh is for", run.stderr)
        self.assertIn("CERASTES_EDG_SRC selects", run.stderr)
        # the matching selection: the pin is selected by leaving the variable unset
        # (the stamp check goes through edg_tree.py, which refuses an explicit
        # CERASTES_EDG_SRC that names third_party/edg itself)
        pin = os.path.realpath(REAL_ROOT / "third_party" / "edg")
        run = self.run_script(cmd, None if self.active == pin else self.active)
        self.assertEqual(run.returncode, 0, run.stdout + run.stderr)
        # header checks do not need the tree
        run = self.run_script([sys.executable, str(REAL_ROOT / "scripts" / "run_tests.py"),
                               "--check-headers", "tests/edg-smoke"], self.elsewhere)
        self.assertEqual(run.returncode, 0, run.stdout + run.stderr)


# ---------------------------------------------------------------------------
# scripts/edg_tree.py against its command-line contract (decision 0005)
# ---------------------------------------------------------------------------

EDG_TREE = REAL_ROOT / "scripts" / "edg_tree.py"
EDG_URL = "https://github.com/mborland/compiler.git"  # the owner's fork (decision 0009)
EDGCPP_URL = "https://github.com/edgcpp/compiler.git"  # upstream: no longer the pin
PUSHED = "refs/remotes/origin/cerastes"
LOCAL = "refs/remotes/fork-local/cerastes"


@e2e_gate(shutil.which("git") is not None and EDG_TREE.is_file(),
          "git or scripts/edg_tree.py missing")
class EdgTreeToolTests(unittest.TestCase):
    """Runs `python3 scripts/edg_tree.py --root DIR <command>` on temporary
    repositories: a root with a nested repository at third_party/edg and a fake
    gitlink, and override trees.  Each case is a twin of a guard."""

    @staticmethod
    def make_env():
        env = {k: v for k, v in os.environ.items()
               if k not in ("CERASTES_EDG_SRC", "GIT_DIR", "GIT_WORK_TREE")}
        env.update({"GIT_CONFIG_GLOBAL": os.devnull, "GIT_CONFIG_SYSTEM": os.devnull,
                    "GIT_AUTHOR_NAME": "t", "GIT_AUTHOR_EMAIL": "t@example.invalid",
                    "GIT_COMMITTER_NAME": "t", "GIT_COMMITTER_EMAIL": "t@example.invalid"})
        return env

    @classmethod
    def build_fixture(cls, base):
        """The pristine fixture: a root with a nested repository at
        third_party/edg and a fake gitlink, and an override tree.  Built once per
        class and copied for each test (a copy is a few ms; the 17 git calls that
        build it are most of what a test cost)."""
        def git(cwd, *args):
            cwd.mkdir(parents=True, exist_ok=True)
            run = subprocess.run(["git", "-c", "commit.gpgsign=false", "-c",
                                  "core.hooksPath=/dev/null", *args], cwd=cwd, env=cls.env,
                                 capture_output=True, text=True)
            assert run.returncode == 0, (args, run.stderr)
            return run.stdout.strip()

        def write(path, text):
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(text)

        root, sub, override = base / "repo", base / "repo" / "third_party" / "edg", base / "override"
        git(root, "init", "-q", "-b", "main")
        write(root / ".gitmodules",
              f'[submodule "third_party/edg"]\n\tpath = third_party/edg\n\turl = {EDG_URL}\n')
        write(root / "README", "root\n")
        git(root, "add", ".gitmodules", "README")
        git(root, "commit", "-q", "-m", "root")
        git(sub, "init", "-q", "-b", "main")
        git(sub, "remote", "add", "origin", EDG_URL)
        write(sub / "src" / "il_def.h", "// il\n")
        git(sub, "add", "-A")
        git(sub, "commit", "-q", "-m", "pin")
        pin = git(sub, "rev-parse", "HEAD")
        # what the submodule last fetched: the pin is a commit of the fork's branch
        git(sub, "update-ref", PUSHED, pin)
        write(root / "cmake" / "EdgFrontEnd.cmake", "# cerastes configuration\n")
        write(root / "src" / "backend" / "edg_il.h", "// il layout guards\n")
        git(root, "update-index", "--add", "--cacheinfo", f"160000,{pin},third_party/edg")
        # an override tree: the pin plus two commits, on a topic branch
        git(base, "clone", "-q", str(sub), str(override))
        git(override, "switch", "-q", "-c", "cerastes")
        for n in (1, 2):
            write(override / "src" / f"x{n}.c", f"{n}\n")
            git(override, "add", "-A")
            git(override, "commit", "-q", "-m", f"c{n}")
        return pin

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls._template_tmp = tempfile.TemporaryDirectory()
        cls.template = Path(cls._template_tmp.name).resolve()
        cls.env = cls.make_env()
        cls.pin = cls.build_fixture(cls.template)

    @classmethod
    def tearDownClass(cls):
        cls._template_tmp.cleanup()
        super().tearDownClass()

    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.tmp = Path(tmp.name).resolve()
        shutil.copytree(self.template, self.tmp, symlinks=True, dirs_exist_ok=True)
        self.root = self.tmp / "repo"
        self.sub = self.root / "third_party" / "edg"
        self.override = self.tmp / "override"
        self.env = dict(self.__class__.env)

    def write(self, path, text):
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)

    def git(self, cwd, *args):
        cwd.mkdir(parents=True, exist_ok=True)
        run = subprocess.run(["git", "-c", "commit.gpgsign=false", "-c", "core.hooksPath=/dev/null",
                              *args], cwd=cwd, env=self.env, capture_output=True, text=True)
        self.assertEqual(run.returncode, 0, (args, run.stderr))
        return run.stdout.strip()

    def tool(self, *args, override=None, extra_env=None):
        env = dict(self.env)
        env.update(extra_env or {})
        if override is not None:
            env["CERASTES_EDG_SRC"] = str(override)
        run = subprocess.run([sys.executable, str(EDG_TREE), "--root", str(self.root), *args],
                             env=env, capture_output=True, text=True, timeout=120)
        return run.returncode, run.stdout, run.stderr

    def topic(self):
        self.git(self.root, "switch", "-q", "-c", "topic")

    def check_fails(self, needle, override=None):
        status, out, err = self.tool("check", override=override)
        self.assertEqual(status, 1, (out, err))
        self.assertIn("error: ", err)
        self.assertIn(needle, err)
        self.assertEqual(out, "")

    # --- check: the pin ---

    def test_a_clean_pin_passes_and_reports(self):
        status, out, err = self.tool("check")
        self.assertEqual((status, err), (0, ""), out)
        self.assertEqual(out, f"EDG tree: fork third_party/edg @ {self.pin[:8]} "
                              "(gitlink, clean)\n")

    def test_a_modified_tracked_file_fails(self):
        self.write(self.sub / "src" / "il_def.h", "// changed\n")
        self.check_fails("third_party/edg is not clean")

    def test_an_untracked_file_fails(self):
        self.write(self.sub / "src" / "new.c", "x\n")
        self.check_fails("third_party/edg is not clean")

    def test_an_ignored_file_is_not_dirt_that_git_reports(self):
        # `--untracked-files=all` reports untracked files, not ignored ones
        self.write(self.sub / ".gitignore", "*.o\n")
        self.git(self.sub, "add", ".gitignore")
        self.git(self.sub, "commit", "-q", "-m", "ignore")
        head = self.git(self.sub, "rev-parse", "HEAD")
        self.git(self.sub, "update-ref", PUSHED, head)
        self.git(self.root, "update-index", "--cacheinfo", f"160000,{head},third_party/edg")
        self.write(self.sub / "a.o", "x\n")
        status, out, err = self.tool("check")
        self.assertEqual(status, 0, (out, err))

    def test_another_commit_fails(self):
        self.write(self.sub / "src" / "il_def.h", "// next\n")
        self.git(self.sub, "commit", "-q", "-am", "next")
        self.check_fails("third_party/edg is at")
        # a staged pin move to a commit that is on neither branch fails and says
        # how to get it ...
        head = self.git(self.sub, "rev-parse", "HEAD")
        self.git(self.root, "update-index", "--cacheinfo", f"160000,{head},third_party/edg")
        self.check_fails("not contained in origin/cerastes or in fork-local/cerastes")
        self.check_fails("git -C third_party/edg fetch origin")
        self.check_fails("git -C third_party/edg fetch fork-local")
        # ... and passes, cleanly, once it is on origin/cerastes
        self.git(self.sub, "update-ref", PUSHED, head)
        status, out, err = self.tool("check")
        self.assertEqual((status, err), (0, ""), out)
        self.assertIn(head[:8], out)
        self.assertNotIn("NOT YET PUSHED", out)

    def new_commit(self):
        self.write(self.sub / "src" / "il_def.h", "// fork-only\n")
        self.git(self.sub, "commit", "-q", "-am", "fork-only")
        commit = self.git(self.sub, "rev-parse", "HEAD")
        self.git(self.root, "update-index", "--cacheinfo", f"160000,{commit},third_party/edg")
        return commit

    def test_a_commit_only_in_the_local_clone_passes_with_a_warning(self):
        commit = self.new_commit()
        self.check_fails("not contained in origin/cerastes or in fork-local/cerastes")
        self.git(self.sub, "update-ref", LOCAL, commit)
        status, out, err = self.tool("check")
        self.assertEqual(status, 0, (out, err))
        self.assertEqual(out, f"EDG tree: fork third_party/edg @ {commit[:8]} (gitlink, clean, "
                              "NOT YET PUSHED to origin/cerastes)\n")
        self.assertIn(f"warning: the pin {commit[:8]} is contained only in fork-local/cerastes: "
                      "the owner must push the fork's cerastes branch before cerastes main", err)
        # describe reports it too
        status, out, err = self.tool("describe")
        self.assertEqual(status, 0, (out, err))
        self.assertIn("NOT YET PUSHED to origin/cerastes", out)
        self.assertIn("must push", err)

    def test_pushed_wins_over_local(self):
        commit = self.new_commit()
        self.git(self.sub, "update-ref", LOCAL, commit)
        self.git(self.sub, "update-ref", PUSHED, commit)
        status, out, err = self.tool("check")
        self.assertEqual((status, err), (0, ""), out)
        self.assertNotIn("NOT YET PUSHED", out)

    def test_an_older_pin_is_fine_on_either_branch(self):
        commit = self.new_commit()
        self.git(self.sub, "update-ref", PUSHED, commit)
        self.git(self.root, "update-index", "--cacheinfo", f"160000,{self.pin},third_party/edg")
        self.git(self.sub, "checkout", "-q", self.pin)
        status, out, err = self.tool("check")
        self.assertEqual((status, err), (0, ""), out)
        self.git(self.sub, "update-ref", "-d", PUSHED)
        self.git(self.sub, "update-ref", LOCAL, commit)
        status, out, err = self.tool("check")
        self.assertEqual(status, 0, (out, err))
        self.assertIn("NOT YET PUSHED", out)

    def test_origin_main_and_other_branches_do_not_count(self):
        # the pin has to be a commit of the cerastes branch: upstream main is
        # not good enough any more, nor is any other branch of the fork
        self.git(self.sub, "update-ref", "-d", PUSHED)
        self.git(self.sub, "update-ref", "refs/remotes/origin/main", self.pin)
        self.git(self.sub, "update-ref", "refs/remotes/fork-local/other", self.pin)
        self.git(self.sub, "update-ref", "refs/remotes/origin/other", self.pin)
        self.check_fails("not contained in origin/cerastes or in fork-local/cerastes")

    def test_missing_refs_fail(self):
        self.git(self.sub, "update-ref", "-d", PUSHED)
        self.check_fails("fetch origin")
        self.check_fails("fetch fork-local")

    def test_a_wrong_url_in_gitmodules_fails(self):
        # upstream's URL is not the pin any more
        self.write(self.root / ".gitmodules", (self.root / ".gitmodules").read_text().replace(
            "mborland/compiler", "edgcpp/compiler"))
        self.check_fails(".gitmodules URL of third_party/edg is")
        self.check_fails(EDG_URL)

    def test_a_missing_gitmodules_entry_fails(self):
        (self.root / ".gitmodules").write_text("")
        self.check_fails(".gitmodules URL of third_party/edg is <missing>")

    def test_a_wrong_origin_fails(self):
        self.git(self.sub, "remote", "set-url", "origin", EDGCPP_URL)
        self.check_fails("remote.origin.url of third_party/edg is")
        self.check_fails(EDG_URL)

    def test_no_origin_fails(self):
        self.git(self.sub, "remote", "remove", "origin")
        self.check_fails("remote.origin.url of third_party/edg is <missing>")

    def test_all_violations_are_listed(self):
        self.write(self.sub / "stray", "x\n")
        self.git(self.sub, "remote", "set-url", "origin", "https://example.invalid/x.git")
        status, out, err = self.tool("check")
        self.assertEqual(status, 1)
        self.assertEqual(sum(l.startswith("error: ") for l in err.splitlines()), 2, err)

    def test_the_pin_is_checked_with_an_override_too(self):
        self.topic()
        self.write(self.sub / "stray", "x\n")
        self.check_fails("third_party/edg is not clean", override=self.override)

    # --- check: the override ---

    def test_an_override_on_main_fails(self):
        self.check_fails("CERASTES_EDG_SRC is set on branch main; check.sh on main runs "
                         "against the pin", override=self.override)

    def test_an_override_on_a_topic_branch_is_reported(self):
        self.topic()
        status, out, err = self.tool("check", override=self.override)
        self.assertEqual((status, err), (0, ""), out)
        head = self.git(self.override, "rev-parse", "HEAD")[:8]
        self.assertEqual(out, f"EDG tree: OVERRIDE {self.override} @ {head} (branch cerastes, "
                              f"2 commits on top of pin {self.pin[:8]}, 0 modified, "
                              "0 untracked)\n")

    def test_the_report_counts_modified_and_untracked(self):
        self.topic()
        self.write(self.override / "src" / "x1.c", "edited\n")
        self.write(self.override / "src" / "n1.c", "n\n")
        self.write(self.override / "src" / "n2.c", "n\n")
        status, out, err = self.tool("check", override=self.override)
        self.assertEqual(status, 0, (out, err))
        self.assertIn("1 modified, 2 untracked)", out)

    def test_an_unrelated_override_warns(self):
        self.topic()
        other = self.tmp / "unrelated"
        self.git(other, "init", "-q", "-b", "cerastes")
        self.write(other / "src" / "il_def.h", "x\n")
        self.git(other, "add", "-A")
        self.git(other, "commit", "-q", "-m", "u")
        status, out, err = self.tool("check", override=other)
        self.assertEqual(status, 0, (out, err))
        self.assertIn("warning: the pin", err)
        self.assertIn("not an ancestor", err)

    def test_an_override_that_is_the_submodule_fails(self):
        self.topic()
        self.check_fails("is, or is inside", override=self.sub)

    def test_an_override_inside_the_submodule_fails(self):
        self.topic()
        inner = self.sub / "vendor" / "edg"
        self.git(inner, "init", "-q", "-b", "cerastes")
        self.write(inner / "src" / "il_def.h", "x\n")
        self.check_fails("is, or is inside", override=inner)

    def test_a_missing_override_fails(self):
        self.topic()
        self.check_fails("is not a directory", override=self.tmp / "nowhere")

    def test_an_override_without_il_def_fails(self):
        self.topic()
        bare = self.tmp / "bare"
        self.git(bare, "init", "-q")
        self.check_fails("has no src/il_def.h", override=bare)

    def test_an_override_that_is_not_a_repository_fails(self):
        self.topic()
        plain = self.tmp / "plain"
        self.write(plain / "src" / "il_def.h", "x\n")
        self.check_fails("is not the root of a git work tree", override=plain)

    def test_a_subdirectory_of_a_repository_is_not_a_tree_root(self):
        self.topic()
        sub = self.override / "nested"
        self.write(sub / "src" / "il_def.h", "x\n")
        self.check_fails("is not the root of a git work tree", override=sub)

    def test_an_override_with_a_detached_head_fails(self):
        self.git(self.root, "switch", "-q", "--detach")
        self.check_fails("CERASTES_EDG_SRC is set with a detached HEAD", override=self.override)
        # the pin has no such requirement
        self.assertEqual(self.tool("check")[0], 0)

    def test_a_relative_override_is_refused_everywhere(self):
        self.topic()
        relative = os.path.relpath(self.override, os.getcwd())
        for command in (("resolve",), ("check",), ("describe",), ("describe", "--verify-build")):
            with self.subTest(command):
                status, out, err = self.tool(*command, override=relative)
                self.assertEqual(status, 2, (out, err))
                self.assertIn("is a relative path", err)
                self.assertEqual(out, "")
        # the same directory by absolute path is fine
        self.assertEqual(self.tool("check", override=self.override)[0], 0)
        status, out, err = self.tool("resolve", override="~/x")
        self.assertEqual(status, 2, (out, err))

    def test_git_redirecting_variables_are_ignored(self):
        self.topic()
        decoy = self.tmp / "decoy"
        self.git(decoy, "init", "-q", "-b", "other")
        self.write(decoy / "src" / "il_def.h", "decoy\n")
        self.git(decoy, "add", "-A")
        self.git(decoy, "commit", "-q", "-m", "decoy")
        poison = {"GIT_DIR": str(decoy / ".git"), "GIT_WORK_TREE": str(decoy),
                  "GIT_INDEX_FILE": str(decoy / ".git" / "index"),
                  "GIT_OBJECT_DIRECTORY": str(decoy / ".git" / "objects"),
                  "GIT_COMMON_DIR": str(decoy / ".git")}
        stamp_file = self.root / "build" / "override" / "edg" / "edg-tree.stamp"
        self.write(stamp_file, self.stamp())
        for command, override in ((("check",), None), (("check",), self.override),
                                  (("resolve",), self.override), (("stamp", str(self.override)),
                                                                  None),
                                  (("describe", "--verify-build"), self.override)):
            with self.subTest(command=command, override=override):
                clean = self.tool(*command, override=override)
                poisoned = self.tool(*command, override=override, extra_env=poison)
                self.assertEqual(poisoned, clean)
                self.assertEqual(clean[0], 0, clean)

    def test_an_empty_override_means_the_pin(self):
        status, out, err = self.tool("check", override="")
        self.assertEqual(status, 0, (out, err))
        self.assertIn("EDG tree: fork", out)

    # --- resolve ---

    def test_resolve(self):
        status, out, err = self.tool("resolve")
        self.assertEqual((status, out, err), (0, f"upstream {self.sub}\n", ""))
        status, out, err = self.tool("resolve", override=self.override)
        self.assertEqual((status, out, err), (0, f"override {self.override}\n", ""))
        link = self.tmp / "link"
        link.symlink_to(self.override)
        status, out, _ = self.tool("resolve", override=link)
        self.assertEqual((status, out), (0, f"override {self.override}\n"))

    def test_resolve_refuses_a_bad_override(self):
        bare = self.tmp / "bare"
        self.git(bare, "init", "-q")
        plain = self.tmp / "plain"
        self.write(plain / "src" / "il_def.h", "x\n")
        for what, path, needle in (("missing", self.tmp / "nowhere", "is not a directory"),
                                   ("no il_def.h", bare, "has no src/il_def.h"),
                                   ("not a repository", plain, "not the root of a git work tree"),
                                   ("the submodule", self.sub, "is, or is inside")):
            with self.subTest(what):
                status, out, err = self.tool("resolve", override=path)
                self.assertEqual(status, 2, (out, err))
                self.assertEqual(out, "")
                self.assertIn(needle, err)

    # --- stamp ---

    STAMP_RE = re.compile(r"^tree (?P<tree>/.+)\nhead (?P<head>[0-9a-f]{40})\n"
                          r"branch (?P<branch>.+)\ndirty (?P<count>\d+) (?P<hash>[0-9a-f]{64})\n"
                          r"config (?P<config>[0-9a-f]{64})\n$")

    def stamp(self, tree=None):
        status, out, err = self.tool("stamp", str(tree or self.override))
        self.assertEqual((status, err), (0, ""), out)
        return out

    def test_the_stamp_format(self):
        out = self.stamp()
        m = self.STAMP_RE.match(out)
        self.assertIsNotNone(m, out)
        self.assertEqual(m.group("tree"), str(self.override))
        self.assertEqual(m.group("head"), self.git(self.override, "rev-parse", "HEAD"))
        self.assertEqual((m.group("branch"), m.group("count")), ("cerastes", "0"))
        self.assertEqual(len(out.splitlines()), 5)
        self.assertEqual(m.group("config"), self.config_digest())

    def config_digest(self):
        h = hashlib.sha256()
        for name in ("cmake/EdgFrontEnd.cmake", "src/backend/edg_il.h"):
            h.update((self.root / name).read_bytes())
        return h.hexdigest()

    def test_the_stamp_covers_the_cerastes_configuration(self):
        before = self.stamp()
        for name in ("cmake/EdgFrontEnd.cmake", "src/backend/edg_il.h"):
            with self.subTest(name):
                path = self.root / name
                original = path.read_text()
                path.write_text(original + "// changed\n")
                changed = self.stamp()
                self.assertNotEqual(changed, before)
                self.assertEqual(changed.splitlines()[:4], before.splitlines()[:4])
                self.assertEqual(changed.splitlines()[4], "config " + self.config_digest())
                path.write_text(original)
                self.assertEqual(self.stamp(), before)
        # the same for the pin's stamp, and a file that disappears counts
        path = self.root / "src" / "backend" / "edg_il.h"
        path.unlink()
        self.assertNotEqual(self.stamp(self.sub).splitlines()[4], before.splitlines()[4])

    def test_a_binary_built_under_an_older_configuration_does_not_verify(self):
        stamp_file = self.root / "build" / "edg" / "edg-tree.stamp"
        self.write(stamp_file, self.stamp(self.sub))
        self.assertEqual(self.tool("describe", "--verify-build")[0], 0)
        config = self.root / "cmake" / "EdgFrontEnd.cmake"
        config.write_text(config.read_text() + "# a macro changed\n")
        status, out, err = self.tool("describe", "--verify-build")
        self.assertEqual(status, 1, (out, err))
        self.assertIn("built from a different EDG tree state", err)
        # a stamp in the old four-line format never verifies either
        stamp_file.write_text("".join(self.stamp(self.sub).splitlines(True)[:4]))
        config.write_text("# cerastes configuration\n")
        self.assertEqual(self.tool("describe", "--verify-build")[0], 1)

    def test_equal_state_equal_stamp(self):
        self.assertEqual(self.stamp(), self.stamp())
        self.write(self.override / "src" / "n.c", "n\n")
        self.assertEqual(self.stamp(), self.stamp())

    def test_the_stamp_records_the_realpath(self):
        link = self.tmp / "link"
        link.symlink_to(self.override)
        self.assertEqual(self.stamp(link), self.stamp())
        self.assertTrue(self.stamp(link).startswith(f"tree {self.override}\n"))

    def test_a_new_commit_changes_the_stamp(self):
        before = self.stamp()
        self.write(self.override / "src" / "x3.c", "3\n")
        self.git(self.override, "add", "-A")
        self.git(self.override, "commit", "-q", "-m", "c3")
        after = self.stamp()
        self.assertNotEqual(before, after)
        self.assertNotEqual(before.splitlines()[1], after.splitlines()[1])
        # same tree contents, other commit: the head line alone tells them apart
        self.git(self.override, "commit", "-q", "--allow-empty", "-m", "empty")
        self.assertNotEqual(after, self.stamp())

    def test_a_tracked_edit_changes_the_stamp_and_the_count(self):
        before = self.stamp()
        self.write(self.override / "src" / "x1.c", "edited\n")
        edited = self.stamp()
        self.assertNotEqual(before, edited)
        self.assertEqual(before.splitlines()[:3], edited.splitlines()[:3])
        self.assertTrue(edited.splitlines()[3].startswith("dirty 1 "))
        self.write(self.override / "src" / "x1.c", "edit_!\n")  # same length, other bytes
        self.assertNotEqual(edited, self.stamp())

    def test_an_untracked_file_changes_the_stamp(self):
        before = self.stamp()
        self.write(self.override / "src" / "new.c", "a\n")
        added = self.stamp()
        self.assertNotEqual(before, added)
        self.assertTrue(added.splitlines()[3].startswith("dirty 1 "))
        self.write(self.override / "src" / "new.c", "b\n")
        self.assertNotEqual(added, self.stamp())
        (self.override / "src" / "new.c").unlink()
        self.assertEqual(before, self.stamp())

    def test_an_ignored_file_does_not(self):
        self.write(self.override / ".gitignore", "*.o\n")
        self.git(self.override, "add", ".gitignore")
        self.git(self.override, "commit", "-q", "-m", "ignore")
        before = self.stamp()
        self.write(self.override / "build.o", "x\n")
        self.assertEqual(before, self.stamp())

    def test_a_detached_head(self):
        self.git(self.override, "checkout", "-q", "--detach")
        self.assertIn("\nbranch (detached)\n", self.stamp())

    # --- describe ---

    def test_describe_reports_without_verifying(self):
        status, out, err = self.tool("describe")
        self.assertEqual((status, out, err),
                         (0, f"EDG tree: fork third_party/edg @ {self.pin[:8]} "
                             "(gitlink, clean)\n", ""))
        status, out, _ = self.tool("describe", override=self.override)
        self.assertEqual(status, 0)
        self.assertIn("EDG tree: OVERRIDE", out)

    def test_describe_labels_a_dirty_pin(self):
        status, out, _ = self.tool("describe")
        self.assertIn("(gitlink, clean)", out)
        self.write(self.sub / "src" / "il_def.h", "// edited\n")
        status, out, _ = self.tool("describe")
        self.assertEqual(status, 0)
        self.assertIn("(gitlink, DIRTY)", out)
        self.write(self.sub / "src" / "il_def.h", "// il\n")
        self.write(self.sub / "src" / "untracked.c", "x\n")  # untracked alone is dirt too
        status, out, _ = self.tool("describe")
        self.assertIn("(gitlink, DIRTY)", out)
        status, out, _ = self.tool("describe", "--verify-build")
        self.assertIn("(gitlink, DIRTY)", out)

    def test_describe_counts_an_override_s_changes(self):
        self.topic()
        self.write(self.override / "src" / "x1.c", "edited\n")
        self.write(self.override / "src" / "n1.c", "n\n")
        self.write(self.override / "src" / "n2.c", "n\n")
        for command in (("describe",), ("describe", "--verify-build")):
            status, out, _ = self.tool(*command, override=self.override)
            self.assertIn("1 modified, 2 untracked)", out, command)

    def test_describe_with_a_bad_override_fails(self):
        status, out, err = self.tool("describe", override=self.tmp / "nowhere")
        self.assertEqual(status, 2, (out, err))
        self.assertIn("is not a directory", err)

    def test_verify_build_needs_the_stamp(self):
        status, out, err = self.tool("describe", "--verify-build")
        self.assertEqual(status, 1, (out, err))
        self.assertIn("edg-tree.stamp is missing", err)
        self.assertIn("EDG tree: fork", out)  # the report line is still printed

    def test_verify_build_compares_the_stamp_with_the_tree_now(self):
        stamp_file = self.root / "build" / "edg" / "edg-tree.stamp"
        self.write(stamp_file, self.stamp(self.sub))
        status, out, err = self.tool("describe", "--verify-build")
        self.assertEqual((status, err), (0, ""), out)
        self.write(self.sub / "src" / "il_def.h", "// changed\n")
        status, out, err = self.tool("describe", "--verify-build")
        self.assertEqual(status, 1, (out, err))
        self.assertIn("built from a different EDG tree state", err)
        self.write(self.sub / "src" / "il_def.h", "// il\n")
        self.assertEqual(self.tool("describe", "--verify-build")[0], 0)
        self.write(self.sub / "src" / "extra.c", "x\n")  # untracked
        self.assertEqual(self.tool("describe", "--verify-build")[0], 1)

    def test_the_override_has_its_own_stamp(self):
        upstream = self.root / "build" / "edg" / "edg-tree.stamp"
        self.write(upstream, self.stamp(self.sub))
        status, out, err = self.tool("describe", "--verify-build", override=self.override)
        self.assertEqual(status, 1, (out, err))
        self.assertIn("build/override/edg/edg-tree.stamp is missing", err)
        self.write(self.root / "build" / "override" / "edg" / "edg-tree.stamp", self.stamp())
        status, out, err = self.tool("describe", "--verify-build", override=self.override)
        self.assertEqual((status, err), (0, ""), out)
        self.write(self.override / "src" / "x1.c", "edited\n")
        self.assertEqual(self.tool("describe", "--verify-build", override=self.override)[0], 1)
        # and the upstream stamp does not vouch for the override
        self.write(self.override / "src" / "x1.c", "1\n")
        upstream.write_text(self.stamp())
        (self.root / "build" / "override" / "edg" / "edg-tree.stamp").unlink()
        self.assertEqual(self.tool("describe", "--verify-build", override=self.override)[0], 1)

    def stamp_problem(self, override=None):
        caller = {"CERASTES_EDG_SRC": str(override)} if override else {}
        return rt.edg_stamp_problem(self.root, caller)

    def test_the_harness_checks_the_stamp_like_describe(self):
        # no stamp: the build never finished (or never ran)
        problem = self.stamp_problem()
        self.assertIn("edg-tree.stamp is missing", problem)
        self.assertIn("scripts/bootstrap.sh", problem)
        stamp_file = self.root / "build" / "edg" / "edg-tree.stamp"
        self.write(stamp_file, self.stamp(self.sub))
        self.assertIsNone(self.stamp_problem())
        # the tree changed since the build
        self.write(self.sub / "src" / "il_def.h", "// changed\n")
        self.assertIn("built from a different EDG tree state", self.stamp_problem())
        self.write(self.sub / "src" / "il_def.h", "// il\n")
        self.assertIsNone(self.stamp_problem())
        self.write(self.sub / "src" / "extra.c", "x\n")
        self.assertIn("different EDG tree state", self.stamp_problem())
        (self.sub / "src" / "extra.c").unlink()
        # the cerastes configuration changed since the build
        config = self.root / "cmake" / "EdgFrontEnd.cmake"
        config.write_text(config.read_text() + "# a macro\n")
        self.assertIn("different EDG tree state", self.stamp_problem())

    def test_the_harness_checks_the_stamp_of_the_active_kind(self):
        self.topic()
        self.write(self.root / "build" / "edg" / "edg-tree.stamp", self.stamp(self.sub))
        self.assertIn("build/override/edg/edg-tree.stamp is missing",
                      self.stamp_problem(self.override))
        self.write(self.root / "build" / "override" / "edg" / "edg-tree.stamp", self.stamp())
        self.assertIsNone(self.stamp_problem(self.override))
        self.write(self.override / "src" / "x1.c", "edited\n")
        self.assertIn("different EDG tree state", self.stamp_problem(self.override))
        # a bad override is a problem too, never a pass
        self.assertIn("is not a directory", self.stamp_problem(self.tmp / "nowhere"))
        self.assertIn("relative path", self.stamp_problem("rel/edg"))

    def test_a_stamp_of_another_tree_does_not_verify(self):
        stamp_file = self.root / "build" / "edg" / "edg-tree.stamp"
        self.write(stamp_file, self.stamp(self.override))  # a different tree
        self.assertEqual(self.tool("describe", "--verify-build")[0], 1)


# ---------------------------------------------------------------------------
# The real front end and the real compilers
# ---------------------------------------------------------------------------

def _real_tables_available():
    gen = rt.load_env().get("CERASTES_EDG_GEN")
    return bool(gen) and (Path(gen) / "err_data.h").is_file() and (
        Path(gen) / "err_codes.h").is_file()


@e2e_gate(_real_tables_available(),
          "CERASTES_EDG_GEN (EDG's generated err_data.h/err_codes.h) missing; "
          "run scripts/bootstrap.sh")
class RealTagTableTests(unittest.TestCase):
    """The parser against the generated headers of the active build.  The
    numbers are the ones EDG prints with --display_error_number."""

    def test_the_real_tables(self):
        reg = rt.load_registry(rt.load_env())
        self.assertGreater(len(reg.numbers), 3000)
        for tag, number in (("undefined_identifier", 20), ("exp_primary_expr", 29),
                            ("not_a_member", 135), ("unrecognized_pragma", 161),
                            ("declared_but_not_referenced", 177),
                            ("no_matching_function", 304), ("cannot_open_file", 1696),
                            ("attribute_namespace_unrecognized", 2803),
                            ("unrecognized_attribute", 1097), ("warning_directive", 1105)):
            with self.subTest(tag):
                self.assertEqual(reg.numbers[tag], number)
                self.assertEqual(reg.tags[number], tag)
        # the encoding that the parser relies on: a tag is its code name without `ec_`
        data = (Path(rt.load_env()["CERASTES_EDG_GEN"]) / "err_data.h").read_text()
        self.assertIn('{ "undefined_identifier", ec_undefined_identifier }', data)
        self.assertIn("#define NUMBER_OF_ERROR_TAGS " + str(len(reg.numbers)), data)

    def test_every_number_a_real_run_prints_has_a_tag(self):
        reg = rt.load_registry(rt.load_env())
        for text in (REAL_THREE_ERRORS, REAL_NOTES, REAL_WARNINGS_ONLY, REAL_CATASTROPHIC,
                     REAL_COMMAND_LINE):
            for d in rt.parse_fe_diagnostics(text).diags:
                self.assertFalse(d.tag(reg).startswith("#"), d)


@e2e_gate(_real_front_end_available() and _real_tables_available(),
          "CERASTES_FE, scripts/cerastes-fe.sh or the generated headers missing; "
          "run scripts/bootstrap.sh")
class RealDiagnosticTests(MemoisedStamp, unittest.TestCase):
    """frontend-error/-warning tests through the real front end: an undeclared
    identifier passes with its real tag at the right line and fails at the
    wrong one; an unregistered attribute namespace is a warning with a tag."""

    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.dir = Path(tmp.name).resolve()

    def run_test(self, name, text, *args):
        path = self.dir / name
        path.write_text(text)
        out = io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(io.StringIO()):
            status = rt.main([*args, str(path)])
        return status, out.getvalue()

    UNDECLARED = ("// expect: frontend-error undefined_identifier @ L{line}\n"
                  "int f() {{\n  return zzz + 1;\n}}\n")

    def test_an_undeclared_identifier_at_the_right_line(self):
        status, out = self.run_test("a.cpp", self.UNDECLARED.format(line=3))
        self.assertEqual(status, 0, out)
        self.assertIn("PASS  ", out)

    def test_and_at_the_wrong_line(self):
        for line in (2, 4):
            with self.subTest(line):
                status, out = self.run_test(f"w{line}.cpp", self.UNDECLARED.format(line=line))
                self.assertEqual(status, 1, out)
                self.assertIn(f"wrong line: expected error undefined_identifier @ L{line}, "
                              "got undefined_identifier @ L3:10", out)

    def test_the_wrong_tag(self):
        status, out = self.run_test("t.cpp", self.UNDECLARED.format(line=3).replace(
            "undefined_identifier", "not_a_member"))
        self.assertEqual(status, 1, out)
        self.assertIn("wrong diagnostic at this line", out)

    def test_no_error_where_one_is_expected(self):
        status, out = self.run_test("n.cpp", "// expect: frontend-error undefined_identifier "
                                             "@ L2\nint f() { return 1; }\n")
        self.assertEqual(status, 1, out)
        self.assertIn("missing error", out)
        self.assertIn("expected 2", out)

    def test_an_unregistered_attribute_namespace_is_a_warning(self):
        status, out = self.run_test(
            "u.cpp", "// expect: frontend-warning attribute_namespace_unrecognized @ L2\n"
                     "[[unreg_ns::x]] int g();\n")
        self.assertEqual(status, 0, out)
        # warnings are not errors: declaring it an error fails
        status, out = self.run_test(
            "u2.cpp", "// expect: frontend-error attribute_namespace_unrecognized @ L2\n"
                      "[[unreg_ns::x]] int g();\n")
        self.assertEqual(status, 1, out)
        # and a missing expectation for the warning fails
        status, out = self.run_test("u3.cpp", "// expect: frontend-warning unrecognized_pragma "
                                              "@ L2\n[[unreg_ns::x]] int g();\n")
        self.assertEqual(status, 1, out)

    def test_error_and_warning_together_and_two_modes(self):
        text = ("// expect: frontend-error undefined_identifier @ L5\n"
                "// expect: frontend-warning attribute_namespace_unrecognized @ L7\n"
                "// mode: c++20, c23\n"
                "int f(void) {\n  return zzz + 1;\n}\n"
                "[[unreg_ns::x]] int g(void);\n")
        status, out = self.run_test("m.c", text)
        self.assertEqual(status, 0, out)
        self.assertIn("PASS  ", out)
        self.assertIn("[c23]", out)
        self.assertIn("[c++20]", out)
        self.assertIn("2 passed, 0 failed", out)

    def test_a_native_error_is_not_a_pass(self):
        # an error in a header and a missing include are never accepted
        status, out = self.run_test("c.cpp", '// expect: frontend-error undefined_identifier '
                                             '@ L2\n#include "no_such_header.h"\n')
        self.assertEqual(status, 1, out)
        self.assertIn("catastrophic error always fails", out)

    def test_a_cerastes_tag_is_refused_until_diagnostics_exist(self):
        with mock.patch.object(rt, "CERASTES_DIAGNOSTICS_DEF", Path("src/nowhere.def")):
            status, out = self.run_test("k.cpp", "// expect: frontend-error "
                                                 "cerastes-contract_not_boolean @ L2\nint x;\n")
        self.assertEqual(status, 1, out)
        self.assertIn("no cerastes diagnostics are defined yet", out)

    def test_an_unknown_edg_tag_is_refused(self):
        status, out = self.run_test("z.cpp", "// expect: frontend-error undefined_identifer "
                                             "@ L2\nint x;\n")
        self.assertEqual(status, 1, out)
        self.assertIn("unknown EDG diagnostic tag", out)
        status, out = self.run_test("z2.cpp", "// expect: frontend-error undefined_identifer "
                                              "@ L2\nint x;\n", "--check-headers")
        self.assertEqual(status, 1, out)


def _native_compilers_available():
    env = rt.load_env()
    return all(env.get(n) and Path(env[n]).is_file()
               for n in ("CERASTES_CLANG_CC", "CERASTES_CLANG_CXX")) \
        and bool(env.get("CERASTES_INCLUDE_DIR")) and "CERASTES_GCC_CC" in env


@e2e_gate(_native_compilers_available() and _real_front_end_available(),
          "CERASTES_CLANG_*, CERASTES_GCC_* or CERASTES_FE missing; run scripts/bootstrap.sh")
class RealNativeTests(MemoisedStamp, unittest.TestCase):
    """The ordinary-compiler checks with the compilers of build/env.sh."""

    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.dir = Path(tmp.name).resolve()
        self.env = rt.load_env()

    def families(self):
        out = ["Apple clang"]
        if self.env.get("CERASTES_GCC_CC"):
            out.append("GCC")
        return out

    def run_test(self, name, body, header="// expect: golden\n// expect: native-ok\n"):
        path = self.dir / name
        path.write_text(header + body)
        out = io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(io.StringIO()):
            status = rt.main(["--update-golden", str(path)])
            self.assertEqual(status, 0, out.getvalue())
            out.truncate(0)
            out.seek(0)
            status = rt.main([str(path)])
        return status, out.getvalue()

    def test_the_real_canaries_pass(self):
        setup = rt.NativeSetup(self.env["CERASTES_INCLUDE_DIR"], [])
        for comp, lang, mode in (
                (rt.NativeCompiler("Apple clang", self.env["CERASTES_CLANG_CC"],
                                   self.env["CERASTES_CLANG_CXX"]), "c++", "c++20"),
                (rt.NativeCompiler("Apple clang", self.env["CERASTES_CLANG_CC"],
                                   self.env["CERASTES_CLANG_CXX"]), "c", "c23")):
            self.assertEqual(rt.native_canary(comp, lang, mode, self.env, setup.include_dir), [])
        if self.env.get("CERASTES_GCC_CC"):
            gcc = rt.NativeCompiler("GCC", self.env["CERASTES_GCC_CC"], self.env["CERASTES_GCC_CXX"])
            for lang, mode in (("c++", "c++20"), ("c", "c23")):
                self.assertEqual(rt.native_canary(gcc, lang, mode, self.env,
                                                  setup.include_dir), [])

    def test_an_ordinary_file_passes_with_every_installed_compiler(self):
        status, out = self.run_test("ok.cpp", "#include <cstdio>\n"
                                              "int add1(int x) { return x + 1; }\n")
        self.assertEqual(status, 0, out)
        for family in self.families():
            self.assertIn(f"native-ok: {family}", out)
            self.assertIn(f"canary native-ok [{family}, c++]", out)
        self.assertTrue(out.rstrip().endswith(", ".join(f"{f} 1" for f in self.families())),
                        out)

    def test_an_inherited_empty_gcc_does_not_drop_gcc(self):
        # build/env.sh is authoritative: the blind spot the CP1 audit reproduced
        if not self.env.get("CERASTES_GCC_CC"):
            self.skipTest("this build/env.sh names no GCC")
        with mock.patch.dict(os.environ, {"CERASTES_GCC_CC": "", "CERASTES_GCC_CXX": ""}):
            status, out = self.run_test("g.cpp", "int add1(int x) { return x + 1; }\n")
        self.assertEqual(status, 0, out)
        self.assertIn("native-ok: GCC", out)
        self.assertNotIn("GCC not installed", out)

    def test_a_c_file_in_c23(self):
        status, out = self.run_test("ok.c", "#include <stdio.h>\nint add1(int x) { return x + 1; }\n")
        self.assertEqual(status, 0, out)
        self.assertIn("native-ok: Apple clang", out)

    def test_a_warning_fails_every_compiler(self):
        status, out = self.run_test("w.cpp", "int f() { int unused = 0; return 1; }\n")
        self.assertEqual(status, 1, out)
        for family in self.families():
            self.assertIn(f"FAIL  {self.dir / 'w.cpp'} [c++20] native-ok: {family}", out)
        self.assertIn("-Wunused", out)

    def test_an_unregistered_attribute_namespace_is_rejected_by_both(self):
        # why the il probes use unreg_ns and are not native-ok: ordinary
        # compilers warn about unknown namespaced attributes, and -Werror
        status, out = self.run_test("a.cpp", "[[unreg_ns::x]] int g();\n")
        self.assertEqual(status, 1, out)
        for family in self.families():
            self.assertIn(f"FAIL  {self.dir / 'a.cpp'} [c++20] native-ok: {family}", out)

    def run_fe(self, name, body):
        path = self.dir / name
        path.write_text(body)
        return rt.run_tool([*rt.FE_COMMAND, "--c++20", str(path)], self.env,
                           rt.FRONTEND_TIMEOUT_S)

    def native(self, name, body, family):
        path = self.dir / name
        path.write_text(body)
        lang, mode = "c++", "c++20"
        comp = (rt.NativeCompiler("Apple clang", self.env["CERASTES_CLANG_CC"],
                                  self.env["CERASTES_CLANG_CXX"]) if family == "Apple clang"
                else rt.NativeCompiler("GCC", self.env["CERASTES_GCC_CC"],
                                       self.env["CERASTES_GCC_CXX"]))
        return rt.run_tool(rt.native_command(comp.exe(lang), lang, mode,
                                             self.env["CERASTES_INCLUDE_DIR"], str(path)),
                           rt.native_env(self.env, family), rt.NATIVE_TIMEOUT_S)

    UNDEFINED_ERROR = "#ifndef __CERASTES__\n#error __CERASTES__ is not defined\n#endif\n"
    DEFINED_ERROR = "#ifdef __CERASTES__\n#error __CERASTES__ is defined\n#endif\n"

    def test_the_front_end_defines_the_macro_and_the_ordinary_compilers_do_not(self):
        # decision 0008: the driver defines __CERASTES__ (the annotation macros of
        # <cerastes.h> expand to attributes); ordinary compilers never do (they
        # expand to nothing), which is what native-ok relies on
        run = self.run_fe("d1.cpp", self.UNDEFINED_ERROR + "int f() { return 0; }\n")
        self.assertEqual((run.returncode, run.stdout, run.stderr), (0, "", ""), run)
        run = self.run_fe("d2.cpp", self.DEFINED_ERROR + "int f() { return 0; }\n")
        self.assertEqual(run.returncode, 4, run)  # `#error` is a catastrophic error
        self.assertIn("__CERASTES__ is defined", run.stderr)
        for family in self.families():
            with self.subTest(family):
                run = self.native("d3.cpp", self.DEFINED_ERROR + "int f();\n", family)
                self.assertEqual((run.returncode, run.stdout, run.stderr), (0, "", ""), run)
                run = self.native("d4.cpp", self.UNDEFINED_ERROR + "int f();\n", family)
                self.assertNotEqual(run.returncode, 0, run)
                self.assertIn("__CERASTES__ is not defined", run.stdout + run.stderr)

    def test_a_native_ok_test_that_expects_the_macro_defined_fails_natively(self):
        # the end-to-end twin: through the harness, the front end accepts the file
        # that needs the macro, and the ordinary compilers reject it
        status, out = self.run_test("d5.cpp", self.UNDEFINED_ERROR + "int f() { return 0; }\n")
        self.assertEqual(status, 1, out)
        self.assertNotIn("FAIL  " + str(self.dir / "d5.cpp") + " [c++20]\n", out)  # the golden passes
        for family in self.families():
            self.assertIn(f"FAIL  {self.dir / 'd5.cpp'} [c++20] native-ok: {family}", out)


class EnvironmentAuthorityTests(DiagRunCase):
    """build/env.sh is authoritative for the variables it defines: an inherited
    value that differs is ignored (with one note), so the shell cannot silently
    drop GCC, swap the tag tables or the include directory."""

    def setUp(self):
        super().setUp()
        patcher = mock.patch.object(rt, "_ignored_inherited", set())
        patcher.start()
        self.addCleanup(patcher.stop)

    def load(self, inherited):
        err = io.StringIO()
        with mock.patch.dict(os.environ, inherited), contextlib.redirect_stderr(err):
            env = rt.load_env()
        return env, err.getvalue()

    def test_the_file_wins_and_says_so_once(self):
        self.set_env(CERASTES_GCC_CC="/real/gcc", CERASTES_INCLUDE_DIR="/real/include")
        env, err = self.load({"CERASTES_GCC_CC": "", "CERASTES_INCLUDE_DIR": "/foreign"})
        self.assertEqual((env["CERASTES_GCC_CC"], env["CERASTES_INCLUDE_DIR"]),
                         ("/real/gcc", "/real/include"))
        self.assertIn("ignoring the inherited CERASTES_GCC_CC=''", err)
        self.assertIn("ignoring the inherited CERASTES_INCLUDE_DIR='/foreign'", err)
        self.assertIn("authoritative", err)
        _, again = self.load({"CERASTES_GCC_CC": ""})
        self.assertEqual(again, "")  # one note per variable and process

    def test_an_equal_inherited_value_is_silent(self):
        self.set_env(CERASTES_GCC_CC="/real/gcc")
        env, err = self.load({"CERASTES_GCC_CC": "/real/gcc"})
        self.assertEqual((env["CERASTES_GCC_CC"], err), ("/real/gcc", ""))

    def test_variables_the_file_does_not_define_are_inherited(self):
        env, err = self.load({"SDKROOT": "/my/sdk", "FAKE_LOG": "/log"})
        self.assertEqual((env["SDKROOT"], env["FAKE_LOG"], err), ("/my/sdk", "/log", ""))

    def test_without_a_file_the_environment_is_all_there_is(self):
        with mock.patch.object(rt, "ENV_SH", self.dir.parent / "no-such-env.sh"):
            env, err = self.load({"CERASTES_GCC_CC": "/inherited"})
        self.assertEqual((env["CERASTES_GCC_CC"], err), ("/inherited", ""))

    def test_an_inherited_tag_table_does_not_replace_the_files(self):
        other = write_gen(self.dir.parent / "other-gen", {"exp_semicolon": 65})
        name = rt.rel(self.dir / "a.cpp")
        t = self.diag_test("a.cpp", ["frontend-error undefined_identifier @ L2"],
                           err_lines(REAL_THREE_ERRORS, name).splitlines()[0] + "\n"
                           f'1 error detected in the compilation of "{name}".', "// exit: 2")
        with mock.patch.dict(os.environ, {"CERASTES_EDG_GEN": str(other)}):
            for first, args in ((True, ("--check-headers",)), (False, ())):
                status, out, err = self.run_harness(*args, t)
                self.assertEqual(status, 0, out)  # validated against env.sh's table
                if first:
                    self.assertIn("ignoring the inherited CERASTES_EDG_GEN", err)
            rt._ignored_inherited.clear()
            self.set_env(CERASTES_EDG_GEN=str(other))  # the file now names the other table
            status, out, _ = self.run_harness("--check-headers", t)
            self.assertEqual(status, 1, out)
            self.assertIn("unknown EDG diagnostic tag `undefined_identifier`", out)


class HermeticEnvironmentTests(unittest.TestCase):
    """The self-test does not depend on the shell that runs it: the variables
    build/env.sh defines are scrubbed from the process at import."""

    def test_the_scrub_removes_what_env_sh_defines_and_the_override(self):
        with tempfile.TemporaryDirectory() as tmp:
            env_sh = Path(tmp) / "env.sh"
            env_sh.write_text('# comment\nexport CERASTES_GCC_CC="/x"\nexport Z3_EXE="/z"\n'
                              'export EDG_BASE="/b"\n')
            environ = {"CERASTES_GCC_CC": "", "Z3_EXE": "/other", "EDG_BASE": "/b",
                       "CERASTES_EDG_SRC": "/fork", "CERASTES_REQUIRE_E2E": "1",
                       "PATH": "/bin", "SDKROOT": "/sdk", "CERASTES_UNLISTED": "keep"}
            removed = scrub_inherited_environment(environ, env_sh)
        self.assertEqual(removed, ["CERASTES_EDG_SRC", "CERASTES_GCC_CC", "EDG_BASE", "Z3_EXE"])
        self.assertEqual(environ, {"CERASTES_REQUIRE_E2E": "1", "PATH": "/bin",
                                   "SDKROOT": "/sdk", "CERASTES_UNLISTED": "keep"})

    def test_the_scrub_covers_every_variable_of_the_real_env_sh(self):
        real = REAL_ROOT / "build" / "env.sh"
        if not real.exists():
            self.skipTest("build/env.sh missing; run scripts/bootstrap.sh")
        names = set(re.findall(r"^export (\w+)=", real.read_text(), re.MULTILINE))
        environ = {n: "poison" for n in names}
        scrub_inherited_environment(environ)
        self.assertEqual(environ, {})

    def test_the_authority_tests_pass_in_a_poisoned_shell(self):
        # the twin of the scrub call at the top of this file: with it removed,
        # the inherited values reach the fixtures and these tests fail
        real = REAL_ROOT / "build" / "env.sh"
        if not real.exists():
            self.skipTest("build/env.sh missing; run scripts/bootstrap.sh")
        env = dict(os.environ)
        for name in re.findall(r"^export (\w+)=", real.read_text(), re.MULTILINE):
            env[name] = "/poison/" + name
        env["CERASTES_GCC_CC"] = ""
        env["CERASTES_EDG_SRC"] = "/poison/fork"
        proc = subprocess.run([sys.executable, str(Path(__file__).resolve()),
                               "-k", "EnvironmentAuthorityTests", "-k", "BootstrapGitEnvironmentTests"],
                              capture_output=True, text=True, env=env, cwd=REAL_ROOT)
        self.assertEqual(proc.returncode, 0, proc.stdout + proc.stderr)
        self.assertRegex(proc.stderr, r"Ran [1-9]\d* tests?")


class NativeEnvironmentAuthorityTests(NativeCase):
    def test_an_empty_inherited_gcc_does_not_drop_gcc(self):
        t = self.native_test("a.cpp", "int f();")
        with mock.patch.dict(os.environ, {"CERASTES_GCC_CC": "", "CERASTES_GCC_CXX": ""}):
            status, out, err = self.run_harness(t)
        self.assertEqual(status, 0, out)
        self.assertNotIn("GCC not installed", out)
        self.assertIn(f"PASS  {rt.rel(t)} [c++20] native-ok: GCC", out)
        self.assertTrue(self.summary(out).endswith("Apple clang 1, GCC 1"), self.summary(out))
        self.assertIn("ignoring the inherited CERASTES_GCC_CC", err)

    def test_a_foreign_include_directory_is_ignored(self):
        t = self.native_test("a.cpp", "int f();")
        with mock.patch.dict(os.environ, {"CERASTES_INCLUDE_DIR": "/foreign/include"}):
            self.run_harness(t)
        calls = self.compiles_of(t)
        self.assertTrue(calls and all("-I /fake/include " in c for c in calls), calls)
        self.assertFalse(any("/foreign" in c for c in self.calls()))

    def test_a_foreign_compiler_is_ignored(self):
        t = self.native_test("a.cpp", "int f();")
        with mock.patch.dict(os.environ, {"CERASTES_CLANG_CXX": "/bin/false"}):
            status, out, _ = self.run_harness(t)
        self.assertEqual(status, 0, out)
        self.assertTrue(self.compiles_of(t)[0].startswith("fake_clang_cxx "))


class NativePolicyHardeningTests(RootedCase):
    """frontend-ok in the annotation directories, and a native-ok test that
    switches its own check off, are header errors (CP1b audit N5, N6)."""

    def test_frontend_ok_is_refused_in_the_annotation_directories(self):
        for directory in ("tests/annotations", "tests/annotations/contracts",
                          "tests/annotations/warnings",
                          "doc/modules/ROOT/examples/annotations",
                          "doc/modules/ROOT/examples/ordinary-compilers/x"):
            for extra in ("", "// expect: native-ok\n"):
                with self.subTest(directory=directory, native=bool(extra)):
                    err = self.header(f"{directory}/f.cpp", "// expect: frontend-ok\n" + extra)
                    self.assertIsInstance(err, rt.HeaderError)
                    self.assertIn("`expect: frontend-ok` is not allowed under", str(err))
                    self.assertIn("does not define __CERASTES__", str(err))

    def test_frontend_ok_is_refused_in_negative_too(self):
        # (there the must-fail rule says so first)
        err = self.header("tests/annotations/negative/f.cpp",
                          "// expect: frontend-ok\n// twin: tests/annotations/ok.cpp\n")
        self.assertIsInstance(err, rt.HeaderError)

    def test_frontend_ok_stays_fine_elsewhere(self):
        for rel in ("tests/edg-smoke/hello.cpp", "tests/il/f.cpp", "tests/annotationsX/f.cpp",
                    "doc/modules/ROOT/examples/installation/hello.cpp",
                    "doc/modules/ROOT/examples/annotations_old/f.cpp"):
            with self.subTest(rel):
                self.assertIsInstance(self.header(rel, "// expect: frontend-ok\n"),
                                      rt.Expectation)

    def native(self, body, native_ok=True, rel="tests/il/n.cpp"):
        head = "// expect: golden\n" + ("// expect: native-ok\n" if native_ok else "")
        return self.header(rel, head + body)

    def test_a_native_ok_test_cannot_switch_the_check_off(self):
        for what, line in (
                ("GCC ignored", '#pragma GCC diagnostic ignored "-Wunused-variable"'),
                ("clang ignored", '#pragma clang diagnostic ignored "-Wunused"'),
                ("GCC warning", '#pragma GCC diagnostic warning "-Wunused-variable"'),
                ("clang warning", '#pragma clang diagnostic warning "-Wunused"'),
                ("spaced", '  #  pragma   GCC   diagnostic   ignored "-Wall"'),
                ("GCC system header", "#pragma GCC system_header"),
                ("clang system header", "#pragma clang system_header"),
                ("define", "#define __CERASTES__"),
                ("define 1", "#define __CERASTES__ 1"),
                ("spaced define", "# define   __CERASTES__ 1")):
            with self.subTest(what):
                err = self.native(line + "\nint f();\n")
                self.assertIsInstance(err, rt.HeaderError)
                self.assertIn("a native-ok test cannot switch its own check off", str(err))
                self.assertIn(line.strip(), str(err))
                # the same file is fine when it is not native-ok
                self.assertIsInstance(self.native(line + "\nint f();\n", native_ok=False),
                                      rt.Expectation)

    def test_what_stays_allowed(self):
        for line in ("#undef __CERASTES__", "#ifdef __CERASTES__", "#if defined(__CERASTES__)",
                     "#pragma clang diagnostic push", "#pragma GCC diagnostic pop",
                     '#pragma GCC diagnostic error "-Wunused"',
                     '#pragma clang diagnostic fatal "-Wunused"',
                     "#define __CERASTES_OTHER__ 1", "#define CERASTES_REQUIRES(x)",
                     '// #pragma GCC diagnostic ignored "-Wall"', "// #define __CERASTES__",
                     "#pragma once", "#pragma pack(push, 1)"):
            with self.subTest(line):
                self.assertIsInstance(self.native(line + "\nint f();\n"), rt.Expectation)

    def test_the_check_runs_in_every_policy_directory_too(self):
        err = self.native('#pragma GCC diagnostic ignored "-Wall"\nint f();\n',
                          rel="tests/annotations/contracts/p.cpp")
        self.assertIn("cannot switch its own check off", str(err))


class WalkerFingerprintTests(DocLintCase):
    """Rule g covers every kind of walker line, anywhere in a line, in blocks
    and in inline code (CP1b audit N1)."""

    def setUp(self):
        super().setUp()
        self.example("a.cpp", GOLDEN_EXAMPLE)
        (self.component / "examples" / "a.cpp.golden").write_text(GOLDEN_TEXT)

    def page_with_block(self, language, *body):
        self.page("p.adoc", page_with("[source,cpp]", "----", "include::example$a.cpp[tag=body]",
                                      "----", "", f"[source,{language}]", "----", *body, "----"))

    EXCERPTS = (
        "op eok_add type_kind=tk_integer type=int",
        "op eok_cast type_kind=tk_template_param type=T @ L9:27",
        "param 1 x: int",
        "param 2 y: T @ L5:3",
        "returns int", "returns T", "returns long long", "returns struct <unnamed> &",
        "returns std::size_t", "returns const int*", "returns vector<int>&",
        "template inc args <T=int>",
        "constraint type-constraint T std::integral",
        "constraint requires std::integral<T>", "constraint trailing std::integral<T>",
        "tconst tpck_expression type=T @ -", "tconst tpck_unknown_function type=<tptk_unknown>",
        "stmt stmk_while", "stmt stmk_empty @ L3:3 attr [[cerastes::check]] @ L3:5",
        "attr [[cerastes::requires]] on routine @ L5:16",
        "arg 1 @ L5:35", "arg 1 binder r: int @ L5:64", "arg 2 const 0 type=int",
        "binder r: int @ L5:67", "this: counter * @ L7:3",
        "const 0 type=int @ L5:39", "const 0 type=int", "const 3 type=unsigned long",
        "field val: int", "node enk_x type=int",
        "body return", "body 3 statements",
        "end walk: 1 routines reported, 0 routines with bodies in non-system files "
        "outside the primary file",
    )

    def test_an_excerpt_of_any_line_kind_is_refused(self):
        for line in self.EXCERPTS:
            with self.subTest(line):
                self.page_with_block("text", line)
                self.lint("walker output must be included from the golden file", count=1)

    def test_prefixed_and_embedded_lines_are_refused(self):
        for line in ("out> routine f plain body @ L3:5", "> param 1 x: int", "$ returns int",
                     "| op eok_add type_kind=tk_integer type=int", "// returns int",
                     "# template inc args <T=int>", "    ->  routine g prototype body @ L1:1",
                     "stdout: routine f plain body @ L3:5", "walk> constraint requires C",
                     "the walker printed op eok_add and more", "x @ L3:5 trailing",
                     "see end walk: 1 routines reported"):
            for language in ("text", "console", "sh"):
                with self.subTest(line=line, language=language):
                    self.page_with_block(language, line)
                    self.lint("walker output must be included", count=1)

    def test_inline_code_in_prose_is_checked_too(self):
        for span in ("`routine f plain body @ L3:5`", "`type_kind=tk_integer`",
                     "`op eok_add`", "+op eok_add+", "`end walk: 1 routines reported`",
                     "`param 1 x: int`", "`returns int`", "`stmt stmk_empty`",
                     "`template inc args <T=int>`", "`x @ L3:5`", "``a b @ L1:1``"):
            for where in ("{}", "* item {}", "|===\n|{}\n|===", "== Heading {}",
                          ".Title {}", "text before {} and after"):
                with self.subTest(span=span, where=where):
                    self.page("p.adoc", LISTING_PAGE + "\n" + where.format(span) + "\n")
                    self.lint("walker output in inline code")

    def test_ordinary_inline_code_and_output_are_fine(self):
        for span in ("`scripts/cerastes-fe.sh --cerastes_walk x.cpp`", "`returns`",
                     "`param`", "`routine`", "`@ L3`", "`L3:5`", "`op`", "`template <class T>`",
                     "`constraint`", "`const int x`", "`type=int`", "`attr`", "`this`",
                     "`int f(int x) { return x; }`", "`returns the value`"):
            with self.subTest(span):
                self.page("p.adoc", LISTING_PAGE + "\n" + f"Use {span} here.\n")
                self.assertEqual(self.messages(), [])
        for body in ("returns the value", "param: foo", "template <class T> struct S;",
                     "the function returns 3", "const x = 1;", "x.cpp:3:5: error: nope",
                     "arg parsing failed", "this is fine", "binder"):
            with self.subTest(body):
                self.page("p.adoc", LISTING_PAGE + f"\n[source,text]\n----\n{body}\n----\n")
                self.assertEqual(self.messages(), [])

    def test_the_real_documentation_has_no_false_positives(self):
        report = cde.check_tree(REAL_ROOT)
        self.assertEqual([str(p) for p in report.problems], [])

    def test_every_line_of_every_real_golden_is_recognised(self):
        # the fingerprint must know the walker: every line of the real goldens
        # that is part of the walk (not the stderr section, source echoes or
        # carets) is recognised
        seen = 0
        for golden in sorted((REAL_ROOT / "tests" / "il").rglob("*.golden")):
            text = rt.read_exact(golden)
            walk = text.split("\n" + rt.STDERR_MARKER + "\n")[0]
            for line in walk.split("\n"):
                if not line.strip():
                    continue
                seen += 1
                self.assertTrue(cde.walker_line(line), (golden.name, line))
        self.assertGreater(seen, 500)


class WalkerFingerprintHtmlTests(GoldenListingHtmlTests):
    """--html: excerpts and inline code with walker output fail, whatever the
    sources say."""

    INLINE_PAGE = """<!DOCTYPE html><html><body>
<p>See <code>{code}</code> for more.</p>
</body></html>
"""

    def test_an_excerpt_block_is_refused(self):
        for line in ("op eok_add type_kind=tk_integer type=int", "param 1 x: int",
                     "returns int", "template inc args &lt;T=int&gt;"):
            with self.subTest(line):
                self.site(line)
                status, out, _ = self.run_html()
                self.assertEqual(status, 1, out)
                self.assertIn("with walker output", out)

    def test_inline_code_with_walker_output_is_refused(self):
        self.site(self.ESCAPED)
        for code in ("routine f plain body @ L3:5", "op eok_add", "type_kind=tk_integer",
                     "returns int", "end walk: 1 routines reported",
                     "<em>param 1 x: int</em>"):
            with self.subTest(code):
                self.site(code, template=self.INLINE_PAGE, name="q.html")
                status, out, _ = self.run_html()
                self.assertEqual(status, 1, out)
                self.assertIn("q.html:2: inline code with walker output", out)
        (self.out / "q.html").unlink()

    def test_ordinary_inline_code_is_fine(self):
        self.site(self.ESCAPED)
        for code in ("scripts/cerastes-fe.sh", "returns", "int f()", "param"):
            with self.subTest(code):
                self.site(code, template=self.INLINE_PAGE, name="q.html")
                self.assertEqual(self.run_html()[0], 0)

    def test_nested_inline_code_is_collected_once(self):
        self.site(self.ESCAPED)
        self.site("a <code>b</code> op eok_add", template=self.INLINE_PAGE, name="q.html")
        status, out, _ = self.run_html()
        self.assertEqual(status, 1, out)
        self.assertEqual(out.count("inline code with walker output"), 1, out)


def _bootstrap_available():
    return (REAL_ROOT / "build" / "env.sh").is_file() and shutil.which("git") is not None


@e2e_gate(_bootstrap_available(), "build/env.sh or git missing; run scripts/bootstrap.sh")
class BootstrapGitEnvironmentTests(unittest.TestCase):
    """bootstrap.sh runs every git command, its own and edg_tree.py's, without
    the variables that redirect git (CP1b audit N7).  A stub `git` in front of
    the real one logs the GIT_* variables of every call."""

    @staticmethod
    def edg_tree():
        sys.path.insert(0, str(REAL_ROOT / "scripts"))
        try:
            import edg_tree
        finally:
            sys.path.pop(0)
        return edg_tree

    def test_the_list_is_the_one_of_edg_tree(self):
        edg_tree = self.edg_tree()
        lines = [l for l in active_lines((REAL_ROOT / "scripts" / "bootstrap.sh").read_text())
                 if l.startswith("unset ")]
        self.assertEqual(len(lines), 1, lines)
        self.assertEqual(set(lines[0].split()[1:]), set(edg_tree.GIT_REDIRECTS))

    def test_no_git_call_sees_a_redirecting_variable(self):
        # bootstrap.sh writes build/ of the tree it lives in: never run it from a
        # copy of scripts/ whose build/ belongs to another tree (it would
        # reconfigure that build for the copy's path)
        if (REAL_ROOT / "build").resolve().parent != REAL_ROOT.resolve():
            self.skipTest("build/ is not this tree's own (a scratch copy of scripts/)")
        edg_tree = self.edg_tree()
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp).resolve()
            log = tmp / "git.log"
            real_git = shutil.which("git")
            stub = tmp / "bin" / "git"
            stub.parent.mkdir()
            stub.write_text(f'#!/bin/sh\nenv | grep "^GIT_" >> "{log}"\n'
                            f'echo "-- $*" >> "{log}"\nexec "{real_git}" "$@"\n')
            stub.chmod(0o755)
            decoy = tmp / "decoy"
            subprocess.run([real_git, "init", "-q", str(decoy)], check=True,
                           capture_output=True)
            env = dict(os.environ)
            env.pop("CERASTES_EDG_SRC", None)
            env["PATH"] = f"{stub.parent}:{env['PATH']}"
            env.update({"GIT_DIR": str(decoy / ".git"), "GIT_WORK_TREE": str(decoy),
                        "GIT_INDEX_FILE": str(decoy / ".git" / "index"),
                        "GIT_OBJECT_DIRECTORY": str(decoy / ".git" / "objects"),
                        "GIT_COMMON_DIR": str(decoy / ".git"), "GIT_NAMESPACE": "x"})
            # This runs the real build step of the shared tree, so it can race
            # another process that is building or editing it (the one flake seen
            # was during a pin move).  The claim under test is about git's
            # environment, so it is checked first and on every attempt, and a
            # failed build is retried once before it fails the test.
            attempts = []
            for _ in range(2):
                run = subprocess.run([str(REAL_ROOT / "scripts" / "bootstrap.sh"), "--build-only"],
                                     cwd=REAL_ROOT, env=env, capture_output=True, text=True,
                                     timeout=900)
                attempts.append(run)
                calls = log.read_text()
                self.assertNotRegex(calls, r"(?m)^(?:" + "|".join(edg_tree.GIT_REDIRECTS) + ")=",
                                    calls)  # (GIT_EDITOR and the like are not redirects)
                if run.returncode == 0:
                    break
            self.assertGreater(calls.count("-- "), 3, calls)  # the stub was used
            self.assertEqual(attempts[-1].returncode, 0,
                             f"after {len(attempts)} attempt(s):\n"
                             + "\n".join(r.stdout[-2000:] + r.stderr[-2000:] for r in attempts))


# ---------------------------------------------------------------------------
# frontend-refused: the driver refuses the options
# ---------------------------------------------------------------------------

DRIVER_REFUSAL = ("cerastes-fe: error: --remove_unneeded_entities is not supported: cerastes "
                  "needs the unpruned IL (unused templates and entities)\n")


class RefusedHeaderTests(unittest.TestCase):
    def parse(self, text, suffix=".cpp"):
        return rt.parse_header(text, suffix)

    def test_grammar(self):
        exp = self.parse("// expect: frontend-refused is not supported\n"
                         "// cerastes-fe-options: --c11 --gnu_version=130000\n")
        self.assertEqual((exp.kind, exp.refused, exp.fe_options, exp.modes),
                         ("frontend-refused", "is not supported",
                          ["--c11", "--gnu_version=130000"], []))
        # the options are split like a shell would, and may be quoted
        exp = self.parse("// expect: frontend-refused x\n// cerastes-fe-options: --a=\"b c\" -d\n")
        self.assertEqual(exp.fe_options, ["--a=b c", "-d"])
        # in a .c file too, and the mode is not defaulted
        self.assertEqual(self.parse("// cerastes-fe-options: --c11\n"
                                    "// expect: frontend-refused c11\n", ".c").modes, [])

    def test_bad_headers(self):
        refused = "// expect: frontend-refused refused\n"
        options = "// cerastes-fe-options: --c11\n"
        bad = {
            "no options": refused,
            "options alone": options + "// expect: golden\n",
            "options with another kind": options + "// expect: frontend-ok\n",
            "options with an error kind": options + "// expect: frontend-error a @ L3\n",
            "empty options": refused + "// cerastes-fe-options:\n",
            "an argument is not an option": refused + "// cerastes-fe-options: --c11 x.cpp\n",
            "a file name only": refused + "// cerastes-fe-options: x.cpp\n",
            "two option lines": refused + options + options,
            "two refused lines": refused + refused + options,
            "no substring": "// expect: frontend-refused\n" + options,
            "with golden": refused + options + "// expect: golden\n",
            "with frontend-error": refused + options + "// expect: frontend-error a @ L3\n",
            "with rejected": refused + options + "// expect: rejected x\n",
            "with native-ok": refused + options + "// expect: native-ok\n",
            "with a mode": refused + options + "// mode: c23\n",
            "options after code": refused + "int x;\n" + options,
            "unbalanced quote": refused + '// cerastes-fe-options: --a="b\n',
        }
        for what, text in bad.items():
            with self.subTest(what), self.assertRaises(rt.HeaderError):
                self.parse(text)
        with self.assertRaises(rt.HeaderError):
            self.parse(refused + options, ".vpr")

    def test_a_refusal_is_a_must_fail_test(self):
        exp = self.parse("// expect: frontend-refused x\n// cerastes-fe-options: --c11\n")
        self.assertTrue(rt.expects_failure(exp))
        self.assertTrue(rt.needs_edg_tools({Path("a.cpp"): exp}))
        self.assertIn("CERASTES_FE", rt.required_env({Path("a.cpp"): exp}))


class EvaluateRefusedTests(unittest.TestCase):
    EXP = rt.Expectation("frontend-refused", [], None, [], refused="is not supported",
                         fe_options=["--remove_unneeded_entities"])

    def evaluate(self, stderr, code, stdout="", exp=None):
        return rt.evaluate_refused(rt.ToolRun(code, stdout, stderr), exp or self.EXP)

    def test_the_drivers_refusal(self):
        self.assertEqual(self.evaluate(DRIVER_REFUSAL, 2), [])
        self.assertEqual(rt.REFUSAL_PREFIX, "cerastes-fe: error: ")

    def test_edgs_command_line_error(self):
        exp = rt.Expectation("frontend-refused", [], None, [], refused="invalid option",
                             fe_options=["--bogus_opt"])
        self.assertEqual(self.evaluate(REAL_COMMAND_LINE, 4, exp=exp), [])
        # without --display_error_number EDG prints no number (what the real run shows)
        self.assertEqual(self.evaluate(REAL_COMMAND_LINE.replace(" #571", ""), 4, exp=exp), [])

    def test_the_substring_must_be_in_the_message(self):
        problems = self.evaluate(DRIVER_REFUSAL, 2, exp=rt.Expectation(
            "frontend-refused", [], None, [], refused="something else", fe_options=["--x"]))
        self.assertEqual(len(problems), 1)
        self.assertIn("does not contain `something else`", problems[0])
        # the prefix is not the message
        self.assertTrue(self.evaluate(DRIVER_REFUSAL, 2, exp=rt.Expectation(
            "frontend-refused", [], None, [], refused="error: --remove", fe_options=["--x"]))
            == [])

    def test_the_exit_status_must_fit_the_shape(self):
        for code in (0, 1, 3, 5, 139):
            with self.subTest(code):
                self.assertTrue(self.evaluate(DRIVER_REFUSAL, code), code)
        # EDG's shape with the driver's status, and the reverse
        self.assertTrue(self.evaluate(REAL_COMMAND_LINE, 2))
        self.assertTrue(self.evaluate(DRIVER_REFUSAL, 4))

    def test_nothing_on_stdout(self):
        problems = self.evaluate(DRIVER_REFUSAL, 2, stdout="routine f\n")
        self.assertTrue(any("printed on stdout" in p for p in problems), problems)

    def test_exactly_one_line(self):
        for stderr in (DRIVER_REFUSAL + DRIVER_REFUSAL, DRIVER_REFUSAL + "more\n",
                       "warning: x\n" + DRIVER_REFUSAL, "", "\n\n"):
            with self.subTest(stderr):
                self.assertTrue(self.evaluate(stderr, 2), stderr)

    def test_an_ordinary_compilation_with_errors_is_not_a_refusal(self):
        # exit status 2 is EDG's status for errors too
        problems = self.evaluate(REAL_THREE_ERRORS, 2)
        self.assertEqual(len(problems), 1)
        self.assertIn("may be an ordinary compilation with errors", problems[0])
        self.assertTrue(self.evaluate("scripts/x.sh: some other message\n", 2))
        self.assertTrue(self.evaluate("cerastes-fe.sh: build/env.sh is for /a but ...\n", 2))
        # a one-line message with the right words but not the driver's prefix
        for line in ("some tool: --remove_unneeded_entities is not supported\n",
                     "cerastes-fe: --remove_unneeded_entities is not supported\n",
                     "cerastes-fe: warning: --remove_unneeded_entities is not supported\n",
                     " cerastes-fe: error: --remove_unneeded_entities is not supported\n"):
            with self.subTest(line):
                self.assertTrue(self.evaluate(line, 2), line)

    def test_edgs_shape_is_strict(self):
        exp = rt.Expectation("frontend-refused", [], None, [], refused="invalid", fe_options=["--x"])
        for stderr in (REAL_COMMAND_LINE.replace("Command-line error #571", "error #571"),
                       REAL_COMMAND_LINE.split("\n", 1)[0] + "\n",
                       REAL_COMMAND_LINE + "extra\n",
                       REAL_COMMAND_LINE.replace("1 catastrophic error detected in this "
                                                 "compilation.", "1 error detected in this "
                                                 "compilation."),
                       REAL_COMMAND_LINE.replace("Compilation terminated.", "Done."),
                       REAL_CATASTROPHIC, ""):
            with self.subTest(stderr):
                self.assertTrue(self.evaluate(stderr, 4, exp=exp), stderr)

    def test_crash_timeout_and_never_run(self):
        self.assertIn("crashed", self.evaluate(DRIVER_REFUSAL, -9)[0])
        self.assertEqual(rt.evaluate_refused(rt.ToolRun(None, error="timed out after 1s"),
                                             self.EXP), ["cerastes-fe timed out after 1s"])


class RefusedRunTests(FakeFrontEndCase):
    """rt.main() on frontend-refused tests with the fake front end, which prints
    the test file's `// err:` lines and exits with its `// exit:` line."""

    OPTIONS = "--remove_unneeded_entities"

    def refused_test(self, name, *lines, expect="frontend-refused is not supported",
                     options=OPTIONS):
        header = f"// expect: {expect}\n// cerastes-fe-options: {options}"
        return self.write(name, *lines, header=header)

    def test_a_refusal_passes(self):
        t = self.refused_test("a.cpp", "// silent", "// err: " + DRIVER_REFUSAL.rstrip("\n"),
                              "// exit: 2")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 0, out)
        self.assertIn(f"PASS  {rt.rel(t)}\n", out)
        self.assertEqual(self.summary(out), "1 passed, 0 failed, 0 skipped")

    def test_edgs_command_line_error_passes(self):
        text = "\n".join("// err: " + l for l in REAL_COMMAND_LINE.splitlines())
        t = self.refused_test("b.cpp", "// silent", text, "// exit: 4",
                              expect="frontend-refused invalid option")
        self.assertEqual(self.run_harness(t)[0], 0)

    def test_a_run_that_is_accepted_fails(self):
        for what, lines in (("exit 0, nothing printed", ["// silent"]),
                            ("exit 0, refusal text", ["// silent", "// err: " + DRIVER_REFUSAL.rstrip()]),
                            ("diagnostics, exit 2", ["// silent", "// err: " + REAL_THREE_ERRORS.splitlines()[0], "// exit: 2"])):
            with self.subTest(what):
                t = self.refused_test(f"c{abs(hash(what))}.cpp", *lines)
                status, out, _ = self.run_harness(t)
                self.assertEqual(status, 1, out)

    def test_the_message_must_contain_the_substring(self):
        t = self.refused_test("d.cpp", "// silent", "// err: " + DRIVER_REFUSAL.rstrip("\n"),
                              "// exit: 2", expect="frontend-refused not in the message")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("the refusal does not contain `not in the message`", out)

    def test_stdout_must_be_empty(self):
        t = self.refused_test("e.cpp", "// err: " + DRIVER_REFUSAL.rstrip("\n"), "// exit: 2",
                              "// out: routine f", "// silent")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("printed on stdout", out)

    def test_crash_and_timeout_fail(self):
        t = self.refused_test("f.cpp", "// silent", "// crash")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("crashed (signal 9)", out)
        t = self.refused_test("g.cpp", "// silent", "// sleep")
        with mock.patch.object(rt, "FRONTEND_TIMEOUT_S", 1):
            status, out, _ = self.run_harness(t)
        self.assertEqual(status, 1, out)
        self.assertIn("timed out after 1s", out)

    def test_the_options_replace_the_mode_option(self):
        # the fake prints its arguments on stdout, which fails a refusal and
        # shows them with -v
        t = self.refused_test("h.cpp", options="--c11 --foo=bar")
        status, out, _ = self.run_harness("-v", t)
        self.assertEqual(status, 1, out)
        self.assertIn("options --c11 --foo=bar\n", out)
        self.assertIn(f"path {rt.rel(t)}\n", out)
        self.assertIn("cwd root\n", out)
        for forbidden in ("--c++20", "--c23", "--brief_diagnostics", "--cerastes_walk"):
            self.assertNotIn(forbidden, out)

    def test_one_run_whatever_the_suffix(self):
        t = self.refused_test("i.c", "// silent", "// err: " + DRIVER_REFUSAL.rstrip("\n"),
                              "// exit: 2", options="--c11")
        status, out, _ = self.run_harness(t)
        self.assertEqual(status, 0, out)
        self.assertEqual(self.summary(out), "1 passed, 0 failed, 0 skipped")

    def test_update_golden_leaves_it_alone_and_the_tree_is_checked(self):
        t = self.refused_test("j.cpp", "// silent", "// err: " + DRIVER_REFUSAL.rstrip("\n"),
                              "// exit: 2")
        status, out, _ = self.run_harness("--update-golden", t)
        self.assertEqual(status, 0, out)
        self.assertFalse((self.dir / "j.cpp.golden").exists())
        # a test that would fail is not run at all by --update-golden
        bad = self.refused_test("j2.cpp", "// silent")
        status, out, _ = self.run_harness("--update-golden", bad)
        self.assertEqual((status, self.summary(out)),
                         (0, "0 passed, 0 failed, 0 skipped, 0 goldens updated, 0 unchanged"))
        self.assertEqual(self.run_harness(bad)[0], 1)
        self.tree_mock.return_value = "mismatch"
        self.assertEqual(self.run_harness(t)[0], 2)
        self.tree_mock.return_value = None
        self.stamp_mock.return_value = "stale"
        self.assertEqual(self.run_harness(t)[0], 2)

    def test_check_headers_validates_it(self):
        t = self.write("k.cpp", header="// expect: frontend-refused x")  # no options
        status, out, _ = self.run_harness("--check-headers", t)
        self.assertEqual(status, 1, out)
        self.assertIn("needs a `// cerastes-fe-options:` line", out)


@e2e_gate(_real_front_end_available(),
          "CERASTES_FE or scripts/cerastes-fe.sh missing; run scripts/bootstrap.sh")
class RealRefusedTests(MemoisedStamp, unittest.TestCase):
    """frontend-refused through the real driver: it refuses
    --remove_unneeded_entities itself (exit 2, one line, the prefix) and EDG
    refuses an unknown option (exit 4, three lines)."""

    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.dir = Path(tmp.name).resolve()

    def run_test(self, name, expect, options, body="int f() { return 0; }\n"):
        path = self.dir / name
        path.write_text(f"// expect: {expect}\n// cerastes-fe-options: {options}\n{body}")
        out = io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(io.StringIO()):
            status = rt.main(["-v", str(path)])
        return status, out.getvalue()

    def test_the_drivers_own_refusal(self):
        status, out = self.run_test("a.cpp", "frontend-refused --remove_unneeded_entities is not "
                                    "supported", "--remove_unneeded_entities")
        self.assertEqual(status, 0, out)
        self.assertEqual(rt.REFUSAL_PREFIX, "cerastes-fe: error: ")

    def test_a_wrong_substring_fails(self):
        status, out = self.run_test("b.cpp", "frontend-refused will never appear",
                                    "--remove_unneeded_entities")
        self.assertEqual(status, 1, out)
        self.assertIn("the refusal does not contain", out)

    def test_edgs_command_line_error(self):
        status, out = self.run_test("c.cpp", "frontend-refused invalid option", "--bogus_option")
        self.assertEqual(status, 0, out)

    def test_an_option_that_is_accepted_is_not_a_refusal(self):
        # the driver does not refuse these: the run compiles, so the test must
        # fail (twin: the check is not vacuous)
        for options in ("--c++20", "--c++20 --brief_diagnostics", "--c23 --display_error_number",
                        "--c++17"):
            with self.subTest(options):
                status, out = self.run_test("d.cpp", "frontend-refused anything", options)
                self.assertEqual(status, 1, out)
                self.assertIn("a refusal is status 2 (the driver) or 4", out)

    def test_the_drivers_language_and_macro_refusals(self):
        # decisions 0007 and 0008: C before C23, GNU emulation and undefining
        # __CERASTES__ are refused by the driver itself (exit 2, one line)
        for expect, options in (
                ("C before C23 is not supported", "--c11"),
                ("C before C23 is not supported", "--c99"),
                ("C before C23 is not supported", "--c17"),
                ("GNU emulation", "--gnu_version=150000"),
                ("GNU emulation", "--gcc"),
                ("GNU emulation", "--g++"),
                ("-U of __CERASTES__ is not supported", "-U__CERASTES__")):
            with self.subTest(options):
                status, out = self.run_test("l.c" if "c1" in options or "c99" in options
                                            else "l.cpp", f"frontend-refused {expect}", options)
                self.assertEqual(status, 0, out)

    def test_the_refusals_have_the_drivers_shape(self):
        for options in ("--c11", "--gnu_version=150000", "-U__CERASTES__"):
            with self.subTest(options):
                path = self.dir / "s.cpp"
                path.write_text("int f() { return 0; }\n")
                run = rt.run_tool([*rt.FE_COMMAND, *options.split(), str(path)], rt.load_env(),
                                  rt.FRONTEND_TIMEOUT_S)
                self.assertEqual(run.returncode, rt.REFUSAL_STATUS, run)
                self.assertEqual(run.stdout, "")
                lines = [l for l in run.stderr.split("\n") if l.strip()]
                self.assertEqual(len(lines), 1, run.stderr)
                self.assertTrue(lines[0].startswith(rt.REFUSAL_PREFIX), lines[0])

    def test_an_ordinary_compilation_with_errors_is_not_a_refusal(self):
        status, out = self.run_test("e.cpp", "frontend-refused undefined", "--c++20",
                                    "int f() { return zzz; }\n")
        self.assertEqual(status, 1, out)
        self.assertIn("may be an ordinary compilation with errors", out)


# ---------------------------------------------------------------------------
# The back end's diagnostics: every tag has a test
# ---------------------------------------------------------------------------

# Tags of src/backend/diagnostics.def that no frontend-error test can name, each
# with the reason.  This list is pinned: an entry for a tag that a test now uses,
# or that the definitions no longer have, fails (tag_coverage_problems).
SELFTEST_ONLY_TAGS = {
    "contract_not_propagated": "a defense-in-depth check that EDG's own substitution "
                               "makes unreachable; `--cerastes_checks_selftest` simulates a "
                               "lost contract (ChecksSelftestTests)",
    "parameter_after_pack": "EDG rejects a contract that names a parameter after a pack "
                            "first (attr_parameter_after_pack); the back end's copy is "
                            "reached only by `--cerastes_checks_selftest`",
    "result_name_on_void": "EDG rejects a result name on a void function first "
                           "(attr_result_name_void_function); the back end's copy is "
                           "reached only by `--cerastes_checks_selftest`",
    "annotation_substitution_error": "EDG reports an erroneous annotation argument itself "
                                     "before the back end sees it; `--cerastes_checks_selftest` "
                                     "injects an error node into a substituted tree",
    "analysis_limit_reached": "the budget is a million shared expression nodes and no source "
                              "reaches it with a reasonable file (EDG's own parser slows down "
                              "first); `--cerastes_checks_selftest` lowers the budget",
}
# Tags with no reproducer of any kind (neither a source test nor the self-test hook).
# Empty today; an entry needs its reason, and goes when a test names the tag.
NO_REPRODUCER_TAGS: dict[str, str] = {}
# Tags reached by an input that a test file cannot express (a path on disk).
STDIN_ONLY_TAGS = {
    "source_scan_incomplete": "reached when the primary source is read from standard input "
                              "(`cerastes-fe.sh -`), which a test file cannot be; "
                              "SourceFromStdinTests runs it and its twin",
}
UNTESTED_BY_FRONTEND_ERROR = {**SELFTEST_ONLY_TAGS, **NO_REPRODUCER_TAGS, **STDIN_ONLY_TAGS}


def tag_coverage_problems(defined, used, allowed):
    """What is wrong with how `used` (the cerastes tags that frontend-error tests
    name) covers `defined` (src/backend/diagnostics.def), given `allowed`
    (tag -> reason for tags no such test can name)."""
    problems = []
    for tag in sorted(defined - used - set(allowed)):
        problems.append(f"the diagnostic `{tag}` has no frontend-error test and no entry in "
                        "UNTESTED_BY_FRONTEND_ERROR")
    for tag, reason in sorted(allowed.items()):
        if not reason.strip():
            problems.append(f"the allowlist entry `{tag}` has no reason")
        if tag not in defined:
            problems.append(f"the allowlist entry `{tag}` is not in diagnostics.def")
        elif tag in used:
            problems.append(f"the allowlist entry `{tag}` is stale: a frontend-error test "
                            "names it now (remove the entry)")
    for tag in sorted(used - defined):
        problems.append(f"a test names `cerastes-{tag}`, which diagnostics.def does not define")
    return problems


class TagCoverageTwinTests(unittest.TestCase):
    """The coverage check fails when it is weakened (fixture tag sets)."""

    DEFINED = {"a", "b", "c"}

    def test_a_covered_set_passes(self):
        self.assertEqual(tag_coverage_problems(self.DEFINED, {"a", "b"}, {"c": "why"}), [])
        self.assertEqual(tag_coverage_problems(self.DEFINED, {"a", "b", "c"}, {}), [])

    def test_a_tag_without_a_test_or_a_reason_fails(self):
        problems = tag_coverage_problems(self.DEFINED, {"a"}, {})
        self.assertEqual(len(problems), 2)
        self.assertTrue(all("has no frontend-error test" in p for p in problems))

    def test_an_allowlist_entry_needs_a_reason(self):
        self.assertTrue(tag_coverage_problems(self.DEFINED, {"a", "b"}, {"c": " "}))

    def test_a_stale_allowlist_entry_fails(self):
        problems = tag_coverage_problems(self.DEFINED, {"a", "b", "c"}, {"c": "why"})
        self.assertEqual(len(problems), 1)
        self.assertIn("stale", problems[0])

    def test_an_allowlist_entry_for_an_unknown_tag_fails(self):
        problems = tag_coverage_problems(self.DEFINED, {"a", "b", "c"}, {"zzz": "why"})
        self.assertEqual(len(problems), 1)
        self.assertIn("not in diagnostics.def", problems[0])

    def test_a_test_naming_an_undefined_tag_fails(self):
        problems = tag_coverage_problems(self.DEFINED, {"a", "b", "c", "q"}, {})
        self.assertEqual(problems, ["a test names `cerastes-q`, which diagnostics.def does "
                                    "not define"])

    def test_the_real_allowlist_has_reasons(self):
        for tag, reason in UNTESTED_BY_FRONTEND_ERROR.items():
            self.assertGreater(len(reason.split()), 5, tag)
        self.assertEqual(set(SELFTEST_ONLY_TAGS) & set(NO_REPRODUCER_TAGS), set())
        self.assertEqual(set(STDIN_ONLY_TAGS) & (set(NO_REPRODUCER_TAGS) | set(SELFTEST_ONLY_TAGS)),
                         set())


@e2e_gate((REAL_ROOT / "src" / "backend" / "diagnostics.def").is_file(),
          "src/backend/diagnostics.def missing")
class RealTagCoverageTests(unittest.TestCase):
    def used_tags(self):
        used = set()
        for base in (REAL_ROOT / "tests", REAL_ROOT / "doc" / "modules" / "ROOT" / "examples"):
            for f in base.rglob("*"):
                if f.suffix in (".c", ".cpp") and f.is_file():
                    exp = rt.read_header(f)
                    if isinstance(exp, rt.Expectation):
                        used |= {d.tag[len(rt.CERASTES_TAG_PREFIX):] for d in exp.diags
                                 if d.tag.startswith(rt.CERASTES_TAG_PREFIX)}
        return used

    def test_every_back_end_diagnostic_has_a_test_or_a_reason(self):
        defined = rt.load_cerastes_tags()
        self.assertGreater(len(defined), 10)
        self.assertEqual(tag_coverage_problems(defined, self.used_tags(),
                                               UNTESTED_BY_FRONTEND_ERROR), [])


@e2e_gate(_real_front_end_available() and _real_tables_available(),
          "CERASTES_FE, scripts/cerastes-fe.sh or the generated headers missing; "
          "run scripts/bootstrap.sh")
class ChecksSelftestTests(unittest.TestCase):
    """The defense-in-depth checks of the back end, which EDG's own checks make
    unreachable from source, through `--cerastes_checks_selftest` (it simulates a
    lost contract, a pack that is not last, a void return) on a contracted
    function template and a contracted variadic template."""

    SOURCE = ("#include <cerastes.h>\n\n"
              "template <class T>\n"
              "T twice(T v) CERASTES_REQUIRES(v >= 0) CERASTES_ENSURES(r, r >= v)\n"
              "{\n  return v + v;\n}\n\n"
              "template <class... Ts>\n"
              "int count(int n, Ts... xs) CERASTES_REQUIRES(n >= 0)\n"
              "{\n  return n + sizeof...(xs);\n}\n\n"
              "int use()\n{\n  return twice(1) + count(1, 2, 3);\n}\n")

    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.path = Path(tmp.name) / "s.cpp"
        self.path.write_text(self.SOURCE)
        self.env = rt.load_env()
        self.registry = rt.load_registry(self.env)

    def run_fe(self, *extra):
        run = rt.run_tool([*rt.FE_COMMAND, "--c++20", *extra, *rt.DIAG_OPTIONS, str(self.path)],
                          self.env, rt.FRONTEND_TIMEOUT_S)
        parsed = rt.parse_fe_diagnostics(run.stderr)
        tags = [d.tag(self.registry) for d in parsed.diags if d.severity == "error"]
        return run, parsed, tags

    def test_without_the_option_the_fixture_is_clean(self):
        run, parsed, tags = self.run_fe()
        self.assertEqual((run.returncode, run.stdout, run.stderr), (0, "", ""), run)
        self.assertEqual(tags, [])

    def test_with_the_option_every_selftest_check_fires(self):
        run, parsed, tags = self.run_fe("--cerastes_checks_selftest")
        self.assertEqual(run.returncode, rt.EDG_ERRORS_STATUS, run)
        self.assertEqual(parsed.problems, [])
        self.assertEqual(run.stdout, "")
        cerastes = {t[len(rt.CERASTES_TAG_PREFIX):] for t in tags
                    if t.startswith(rt.CERASTES_TAG_PREFIX)}
        # the hook also injects an error expression and an error type
        # (annotation_substitution_error), and makes the first
        # tree read one parameter reference as unmapped
        # and one node as visited twice: two unsupported_annotation_expression errors
        self.assertEqual(cerastes, set(SELFTEST_ONLY_TAGS) | {"unsupported_annotation_expression"},
                         tags)
        self.assertEqual(tags.count("cerastes-unsupported_annotation_expression"), 2, tags)
        # both branches of the check: an error expression in the first node, an error
        # type in the second
        self.assertEqual(tags.count("cerastes-annotation_substitution_error"), 2, tags)
        self.assertIn("(error expression)", run.stderr)
        self.assertIn("(error type)", run.stderr)
        # and nothing else: no EDG error rides along
        self.assertTrue(all(t.startswith(rt.CERASTES_TAG_PREFIX) for t in tags), tags)

    def test_the_option_changes_nothing_but_these_errors(self):
        # the hook is a test hook: a file with no contract is unaffected
        self.path.write_text("int f(int x) { return x; }\ntemplate <class T> T g(T v) { return v; }"
                             "\nint h() { return g(1); }\n")
        run, _parsed, tags = self.run_fe("--cerastes_checks_selftest")
        self.assertEqual((run.returncode, tags), (0, []), run)


@e2e_gate(_real_front_end_available() and _real_tables_available(),
          "CERASTES_FE, scripts/cerastes-fe.sh or the generated headers missing; "
          "run scripts/bootstrap.sh")
class SourceFromStdinTests(unittest.TestCase):
    """`source_scan_incomplete`: the back end reads the text of every file to find
    a change of __CERASTES__, so a primary source it cannot reopen (standard input,
    `cerastes-fe.sh -`) is an error, not a silent pass.  The same text from a file
    is clean."""

    SOURCE = "#include <cerastes.h>\nint half(int x) CERASTES_REQUIRES(x >= 0);\n"

    def run_fe(self, *args, stdin=None):
        env = rt.load_env()
        return subprocess.run([*rt.FE_COMMAND, "--c++20", *rt.DIAG_OPTIONS, *args],
                              input=stdin, capture_output=True, text=True, env=env,
                              cwd=REAL_ROOT, timeout=rt.FRONTEND_TIMEOUT_S)

    def test_standard_input_is_an_error_naming_the_input(self):
        run = self.run_fe("-", stdin=self.SOURCE)
        self.assertEqual(run.returncode, rt.EDG_ERRORS_STATUS, run)
        registry = rt.load_registry(rt.load_env())
        parsed = rt.parse_fe_diagnostics(run.stderr)
        errors = [(d.tag(registry), d.line) for d in parsed.diags if d.severity == "error"]
        self.assertEqual(errors, [("cerastes-source_scan_incomplete", 1)], run.stderr)
        self.assertEqual(parsed.problems, [])
        self.assertIn('cannot read "-"', run.stderr)

    def test_the_same_text_from_a_file_is_clean(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "half.cpp"
            path.write_text(self.SOURCE)
            run = self.run_fe(str(path))
        self.assertEqual((run.returncode, run.stdout, run.stderr), (0, "", ""), run)


@e2e_gate(_real_front_end_available() and _real_tables_available(),
          "CERASTES_FE, scripts/cerastes-fe.sh or the generated headers missing; "
          "run scripts/bootstrap.sh")
class PrinterPathTests(unittest.TestCase):
    """Each fixture is a real source that reaches one path of the walker's
    printer (and of the back end's check of the same tree); the twin next to it,
    which does not, must not print it.  A fallback node, a depth over the cap, a
    lambda body that is not one `return <expr>;`, an unmapped parameter reference
    and a node visited twice are errors (unsupported_annotation_expression,
    invariant 3), not silent output."""

    UAE = "cerastes-unsupported_annotation_expression"

    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.dir = Path(tmp.name)
        self.env = rt.load_env()
        self.registry = rt.load_registry(self.env)

    def walk(self, source, *extra, name="p.cpp"):
        path = self.dir / name
        path.write_text("#include <cerastes.h>\n" + source)
        run = rt.run_tool([*rt.FE_COMMAND, "--c++20", rt.WALK_OPTION, *extra, *rt.DIAG_OPTIONS,
                           str(path)], self.env, rt.FRONTEND_TIMEOUT_S)
        parsed = rt.parse_fe_diagnostics(run.stderr)
        errors = [(d.tag(self.registry), d.line) for d in parsed.diags if d.severity == "error"]
        return run, errors

    def assertClean(self, run, errors):
        self.assertEqual((run.returncode, run.stderr, errors), (0, "", []), run.stderr)

    # -- children not printed: a requires-expression in a dependent contract
    def test_a_requires_expression_in_a_dependent_contract(self):
        run, errors = self.walk("template <class T> int h(T y) CERASTES_REQUIRES(requires (T q) "
                                "{ q + y; });\nint use() { return h(1); }\n")
        self.assertEqual(errors, [(self.UAE, 2)], run.stderr)
        self.assertEqual(run.returncode, rt.EDG_ERRORS_STATUS)
        self.assertRegex(run.stdout, r"node enk_requires type=bool @ L2:\d+ \(children not printed\)")
        self.assertRegex(run.stdout, r"const true type=bool @ -")  # its instance is a constant
        self.assertIn("an expression of kind enk_requires", run.stderr)
        self.assertClean(*self.walk("template <class T> int h(T y) CERASTES_REQUIRES(y > 0);\n"
                                    "int use() { return h(1); }\n"))
        run, _ = self.walk("template <class T> int h(T y) CERASTES_REQUIRES(y > 0);\n"
                           "int use() { return h(1); }\n")
        self.assertNotIn("children not printed", run.stdout)

    # -- depth limit
    def chain(self, n):
        return "int d(int y) CERASTES_REQUIRES(%s > 0);\n" % " + ".join(["y"] + ["1"] * n)

    def test_a_deep_expression_hits_the_depth_cap(self):
        run, errors = self.walk(self.chain(300))
        self.assertEqual(errors, [(self.UAE, 2)], run.stderr)
        self.assertIn("(depth limit)", run.stdout)
        self.assertIn("an expression nested deeper than 256 levels", run.stderr)
        ops = len(re.findall(r"^\s+op eok_add ", run.stdout, re.M))
        self.assertTrue(200 < ops < 300, ops)
        run, errors = self.walk(self.chain(100))
        self.assertClean(run, errors)
        self.assertNotIn("(depth limit)", run.stdout)
        self.assertEqual(len(re.findall(r"^\s+op eok_add ", run.stdout, re.M)), 100)

    # -- a lambda body that is not a single return
    def test_a_lambda_body_with_several_statements(self):
        run, errors = self.walk("int lam(int n) CERASTES_REQUIRES(cerastes::forall("
                                "[](int i, int n) { int t = i; return t < n; }, n));\n")
        self.assertEqual(errors, [(self.UAE, 2)], run.stderr)
        self.assertIn("a lambda whose body is not a single return statement", run.stderr)
        self.assertRegex(run.stdout, r"body <\d+ statements>")
        run, errors = self.walk("int lam(int n) CERASTES_REQUIRES(cerastes::forall("
                                "[](int i, int n) { return i < n; }, n));\n")
        self.assertClean(run, errors)
        self.assertIn("body return", run.stdout)
        self.assertNotIn("statements>", run.stdout)

    # -- tconst kinds other than tpck_expression
    def test_tconst_kinds(self):
        src = ("template <class T> T g(T v) CERASTES_REQUIRES(v >= sizeof(T) && alignof(T) > 0 "
               "&& noexcept(v + 1));\n"
               "int use() { return g(1); }\n")
        run, errors = self.walk(src)
        self.assertClean(run, errors)
        for kind in ("tpck_sizeof", "tpck_alignof"):
            self.assertRegex(run.stdout, rf"tconst {kind} operand=T type=unsigned long @ L2:\d+")
        self.assertRegex(run.stdout, r"tconst tpck_noexcept type=bool @ L2:\d+")
        run, errors = self.walk("template <class T> T g(T v) CERASTES_REQUIRES(v >= 0);\n"
                                "int use() { return g(1); }\n")
        self.assertClean(run, errors)
        for kind in ("tpck_sizeof", "tpck_alignof", "tpck_noexcept"):
            self.assertNotIn(kind, run.stdout)

    # -- contract from root template
    def test_a_member_template_prototype_shows_the_root_contract(self):
        src = ("template <class U> struct S { template <class T> T g(T v); };\n"
               "template <class U> template <class T> T S<U>::g(T v) CERASTES_REQUIRES(v >= 0) "
               "{ return v; }\nint use() { S<long> s; return s.g(1); }\n")
        run, errors = self.walk(src)
        self.assertClean(run, errors)
        self.assertIn("contract from root template S<U>::g", run.stdout)
        run, errors = self.walk("template <class T> T g(T v) CERASTES_REQUIRES(v >= 0) "
                                "{ return v; }\nint use() { return g(1); }\n")
        self.assertNotIn("from root template", run.stdout)

    # -- the walker's contract-missing warning
    CONTRACTED = ("template <typename T>\nT inc(T v) CERASTES_REQUIRES(v >= 0) { return v + 1; }\n\n"
                  "template <typename T>\nT never(T v) CERASTES_REQUIRES(v >= 0) { return v; }\n\n"
                  "int use() { return inc(3); }\n")

    def test_the_contract_missing_warning_fires_with_the_hook_only(self):
        run, errors = self.walk(self.CONTRACTED)
        self.assertClean(run, errors)
        self.assertNotIn("warning:", run.stdout)
        run, errors = self.walk(self.CONTRACTED, "--cerastes_walk_selftest")
        self.assertClean(run, errors)
        lines = run.stdout.splitlines()
        self.assertIn("warning: contract of inc missing on inc", lines)
        # the template with no instance has no instance routine to complain about
        self.assertNotIn("warning: contract of never missing on never", lines)

    def test_the_warning_needs_a_contract_on_the_root(self):
        run, errors = self.walk("template <typename T>\nT inc(T v) { return v + 1; }\n"
                                "int use() { return inc(3); }\n", "--cerastes_walk_selftest")
        self.assertClean(run, errors)
        self.assertFalse([l for l in run.stdout.splitlines()
                          if l.startswith("warning: contract of")])

    # -- (unmapped) and (repeated node), through --cerastes_checks_selftest
    TWO_PARAMS = ("int f(int x, int y) CERASTES_REQUIRES(x > 0 && y > x) "
                  "CERASTES_ENSURES(r, r > x);\n")

    def test_unmapped_and_repeated_nodes_with_the_hook(self):
        run, errors = self.walk(self.TWO_PARAMS, "--cerastes_checks_selftest")
        self.assertEqual(run.returncode, rt.EDG_ERRORS_STATUS)
        self.assertIn("(unmapped)", run.stdout)
        self.assertIn("(repeated node)", run.stdout)
        self.assertEqual(errors.count((self.UAE, 2)), 2, run.stderr)
        self.assertIn("a parameter reference that is not understood", run.stderr)
        self.assertIn("a node that is part of the tree twice", run.stderr)

    def test_without_the_hook_neither_appears(self):
        run, errors = self.walk(self.TWO_PARAMS)
        self.assertClean(run, errors)
        for marker in ("(unmapped)", "(repeated node)", "children not printed", "depth limit"):
            self.assertNotIn(marker, run.stdout)

    def test_the_hook_changes_only_the_first_tree(self):
        run, errors = self.walk("int g(int x) CERASTES_REQUIRES(x > 0);\n" + self.TWO_PARAMS,
                                "--cerastes_checks_selftest")
        self.assertEqual(run.stdout.count("(unmapped)"), 1, run.stdout)
        self.assertEqual(run.stdout.count("(repeated node)"), 1, run.stdout)


class DocumentationTests(unittest.TestCase):
    """tests/README.md describes every kind and rule of this harness."""

    def test_the_readme_documents_the_extensions(self):
        text = (REAL_ROOT / "tests" / "README.md").read_text()
        for needle in ("--cerastes_checks_selftest", "UNTESTED_BY_FRONTEND_ERROR",
                       "diagnostics.def", "frontend-refused", "cerastes-fe-options", "cerastes-fe: error: ",
                       "frontend-ok` is a header error", "switch its own check off",
                       "inline code", "verifies the active kind's build stamp",
                       "GIT_REDIRECTS", "twin-exempt", "tests/annotations/warnings", "authoritative",
                       "relative path", "origin/cerastes", "fork-local", "NOT YET PUSHED", "unreg_ns::x", "config <sha256",
                       "frontend-error", "frontend-warning", "native-ok", "// twin:",
                       "tests/annotations/negative", "CERASTES_EDG_SRC", "CERASTES_EDG_GEN",
                       "diagnostics.def", "--brief_diagnostics", "--display_error_number",
                       "-U__CERASTES__", "-Werror", "canary", "NATIVE_POLICY_DIRS",
                       "err_data.h", "cerastes-<tag>", "scripts/edg_tree.py",
                       "GCC not installed; Apple clang only"):
            with self.subTest(needle):
                self.assertIn(needle, text)
        for directory in rt.NATIVE_POLICY_DIRS:
            self.assertIn(str(directory), text)


if __name__ == "__main__":
    unittest.main()
