#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Self-test for scripts/run_tests.py: the harness must fail tests that fail
for the wrong reason or at the wrong line, not only tests with the wrong
verdict.

The end-to-end cases run Silicon on mutated temporary copies of
tests/viper-smoke/fail.vpr; they are skipped when build/env.sh is missing.
"""

import contextlib
import io
import re
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_tests as rt  # noqa: E402

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


@unittest.skipUnless((rt.ROOT / "build" / "env.sh").exists(),
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


if __name__ == "__main__":
    unittest.main()
