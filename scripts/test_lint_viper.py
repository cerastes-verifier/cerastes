#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Self-test for scripts/lint_viper.py (invariant 4, internal/encoding.md section
6, decisions 0012 items 1, 11 and 13).

Fixture `.vpr` and `.vpr.map` pairs in the shape the emitter produces (section
4 of encoding.md) are built from a table of lines; each check of the lint has a
twin that mutates the good fixture so that exactly that check must fail, so
weakening the lint fails a test here.  Directories, the manifest and the
command line are covered with temporary trees.
"""

import contextlib
import re
import io
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import lint_viper as lv  # noqa: E402
import run_tests as rt  # noqa: E402

# the ids of the position map (encoding.md section 5): an assert, a method ensures, a call, an
# invariant clause, a method requires (its well-formedness: a failed precondition is reported
# at the call)
A, P, PRE, POST, INV, ASSERT = ("assert.failed", "postcondition.violated", "call.precondition",
                                "postcondition.violated", "invariant.not.preserved",
                                "assert.failed")
WF = "not.wellformed"


TERMINATION_RULES = ("`decreases` clause", "`decreases _`", "without a `decreases`")
# decision 0059 item 7's rules about where a `forall` may stand: the older tests that use a quantifier only as an example of a trigger
# set or of an axiom (a loop-header parse, an axiom outside its domain) are about other rules and filter these out; the array rules have
# their own tests (ArrayQuantifierTests), which call lv.lint_vpr itself.
QUANTIFIER_RULES = ("`forall` outside the shapes of decision 0059", "not one of decision 0059 item 8's four conjuncts")


def without_quantifier_rules(problems):
    return [p for p in problems if not any(r in p for r in QUANTIFIER_RULES)]


def lint_vpr_other(*args, **kwargs):
    """lv.lint_vpr without the termination rules (decision 0013 item 5): the fixtures of the older tests are
    about other rules, and the termination rules have their own tests (TerminationClauseTests), which call
    lv.lint_vpr itself."""
    return [p for p in lv.lint_vpr(*args, **kwargs) if not any(t in p for t in TERMINATION_RULES)]


def drop_offset_conjuncts(text):
    """`text` without decision 0059 item 10's `offset_of(P) == 0` conjuncts: the wrapped exemption comparison
    `((offset_of(P) == 0) && (t == loc(block_of(P), (-1))))` becomes `(t == loc(block_of(P), (-1)))`, and the token-reading update
    `((P != null) && (offset_of(P) == 0)) && (perm(...) > none)` becomes `(P != null) && (perm(...) > none)`: the shapes before Part D."""
    text = re.sub(r"\(\(offset_of\((.+?)\) == 0\) && \((t == loc\(block_of\(\1\), \(-1\)\))\)\)", r"(\2)", text)
    return re.sub(r"\(\((.+?) != null\) && \(offset_of\(\1\) == 0\)\)", r"(\1 != null)", text)


class Check:
    """A line of a fixture that needs a position-map entry."""

    def __init__(self, text, error_id="assert.failed", category="overflow", message=None):
        self.text, self.error_id, self.category, self.message = text, error_id, category, message


def good_lines():
    """The Viper of a small program in the emitter's shape: a prelude function with
    a body, a verified method, a call, a loop and a trusted bodyless method."""
    return [
        "// prelude",
        "function c_abs(x: Int): Int",
        "{",
        "  x < 0 ? -x : x",
        "}",
        "// cerastes:trusted-contract defined in another unit",
        "method ext__1(n: Int) returns (res: Int)",
        Check("  requires -2147483648 <= n && n <= 2147483647", WF, "internal"),
        Check("  ensures -2147483648 <= res && res <= 2147483647", P, "postcondition"),
        "  decreases _",
        "method f__2(x: Int) returns (res: Int)",
        Check("  requires -2147483648 <= x && x <= 2147483647", WF, "internal"),
        Check("  requires x > 0", WF, "internal"),
        Check("  ensures res > x", P, "postcondition"),
        "  decreases",
        "{",
        "  var x__1: Int := x",
        "  var t__1: Int",
        Check("  assert -2147483648 <= x__1 + 1 && x__1 + 1 <= 2147483647", A, "overflow"),
        "  res := c_abs(x__1 + 1)",
        Check("  assert res > x", A, "postcondition"),
        "  goto end",
        Check("  assert false", A, "missing-return"),
        "  label end",
        "}",
        "method g__3(y: Int) returns (res: Int)",
        Check("  requires y > 0", WF, "internal"),
        "  decreases",
        "{",
        "  var i__1: Int := 0",
        "  while (i__1 < y)",
        Check("    invariant 0 <= i__1", INV, "invariant-preserved"),
        Check("    decreases y - i__1", "termination.failed", "termination"),
        "  {",
        "    i__1 := i__1 + 1",
        "  }",
        "  var t__2: Int",
        Check("  assert y > 0", A, "precondition"),
        Check("  t__2 := f__2(y)", PRE, "precondition"),
        "  res := t__2",
        "  label end",
        "}",
    ]


def build(lines):
    """(vpr text, map text) of a fixture: every Check gets a map entry."""
    text, entries = [], []
    for i, item in enumerate(lines, start=1):
        if isinstance(item, Check):
            text.append(item.text)
            entries.append("\t".join([str(i), item.error_id, item.category, "t.c", str(i + 100),
                                      "5", item.message or f"message of line {i}"]))
        else:
            text.append(item)
    return "\n".join(text) + "\n", "\n".join(entries) + "\n"


def lint(lines=None, text=None, map_text="auto"):
    """Problems of the fixture (or of `text`), with its generated map."""
    if text is None:
        text, generated = build(good_lines() if lines is None else lines)
    else:
        generated = build(good_lines())[1]
    return lint_vpr_other(text, generated if map_text == "auto" else map_text, "t.vpr", "t.vpr.map")


def find(lines, fragment):
    """The index of the one line of `lines` that contains `fragment`."""
    hits = [i for i, l in enumerate(lines)
            if fragment in (l.text if isinstance(l, Check) else l)]
    assert len(hits) == 1, (fragment, hits)
    return hits[0]


def edited(*edits):
    """(text, map) of the good fixture after `edits`, each a function of the list
    of lines; the map is generated from the edited lines, so a mutation changes
    one thing only."""
    lines = good_lines()
    for edit in edits:
        edit(lines)
    return build(lines)


def insert_before(fragment, *new):
    def edit(lines):
        i = find(lines, fragment)
        lines[i:i] = list(new)
    return edit


def remove(fragment):
    return lambda lines: lines.pop(find(lines, fragment))


def drop_after(fragment, n):
    def edit(lines):
        i = find(lines, fragment)
        del lines[i + 1:i + 1 + n]
    return edit


def replace_line(fragment, *new):
    def edit(lines):
        i = find(lines, fragment)
        lines[i:i + 1] = list(new)
    return edit


class GoodFixtureTests(unittest.TestCase):
    def test_the_good_fixture_is_clean(self):
        self.assertEqual(lint(), [])

    def test_the_fixture_has_every_kind_of_check_line(self):
        text, map_text = build(good_lines())
        for kind in ("  assert ", "  requires ", "  ensures ", "    invariant ", ":= f__2("):
            self.assertIn(kind, text)
        self.assertEqual(len(map_text.strip().split("\n")),
                         sum(isinstance(l, Check) for l in good_lines()))

    def test_a_program_with_no_checks_needs_no_map_entries(self):
        self.assertEqual(lint_vpr_other("method m() {\n}\n", "", "t.vpr", "t.vpr.map"), [])


class InhaleTests(unittest.TestCase):
    def problems(self, statement):
        text, map_text = edited(insert_before("  goto end", *statement.split("\n")))
        return lint_vpr_other(text, map_text, "t.vpr", "t.vpr.map")

    def test_untagged_inhale_and_assume_fail(self):
        for word in ("inhale", "assume"):
            with self.subTest(word):
                probs = self.problems(f"  {word} x > 0")
                self.assertEqual(len(probs), 1, probs)
                self.assertIn(f"`{word}` without `// cerastes:inhale-ok <reason>`", probs[0])
                self.assertIn("t.vpr:", probs[0])

    def test_a_tag_with_a_reason_passes(self):
        for word in ("inhale", "assume"):
            self.assertEqual(self.problems(f"  {word} x > 0  // cerastes:inhale-ok the callee's "
                                           "postcondition"), [])

    def test_a_tag_needs_a_reason(self):
        probs = self.problems("  inhale x > 0  // cerastes:inhale-ok")
        self.assertEqual(len(probs), 1)
        self.assertIn("needs a reason", probs[0])
        self.assertTrue(self.problems("  inhale x > 0  // cerastes:inhale-ok   "))

    def test_other_comments_and_other_tags_do_not_count(self):
        for comment in ("// fine", "// cerastes:trusted-contract why", "// cerastes:inhale-okay x",
                        "// inhale-ok reason", "// cerastes: inhale-ok reason"):
            with self.subTest(comment):
                self.assertTrue(self.problems(f"  inhale x > 0  {comment}"), comment)

    def test_the_tag_must_be_on_the_same_line(self):
        self.assertTrue(self.problems("  // cerastes:inhale-ok a reason\n  inhale x > 0"))
        self.assertTrue(self.problems("  inhale x > 0\n  // cerastes:inhale-ok a reason"))

    def test_a_tag_in_a_block_comment_does_not_count(self):
        self.assertTrue(self.problems("  inhale x > 0  /* cerastes:inhale-ok a reason */"))

    def test_the_words_inside_comments_and_names_are_fine(self):
        self.assertEqual(self.problems("  // we never inhale here, or assume"), [])
        self.assertEqual(self.problems("  /* assume */ var inhaled: Int := 0"), [])
        self.assertEqual(self.problems("  var assumed: Int := 0"), [])

    def test_inhale_inside_an_expression_line_is_found_too(self):
        self.assertTrue(self.problems("  var z: Int := 0; inhale z == 0"))


class BodylessTests(unittest.TestCase):
    TAG = "// cerastes:trusted-contract defined in another unit"
    EXT = "method ext__1(n: Int) returns (res: Int)"

    def problems(self, *edits):
        text, map_text = edited(*edits)
        return lint_vpr_other(text, map_text, "t.vpr", "t.vpr.map")

    def test_a_bodyless_method_needs_the_trusted_tag(self):
        probs = self.problems(remove(self.TAG))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("bodyless `method ext__1` without `// cerastes:trusted-contract <reason>`",
                      probs[0])
        self.assertTrue(probs[0].startswith("t.vpr:"))

    def test_the_tag_may_be_on_the_method_line_or_above(self):
        self.assertEqual(self.problems(remove(self.TAG), replace_line(
            self.EXT, self.EXT + "  // cerastes:trusted-contract why")), [])
        self.assertEqual(self.problems(replace_line(
            self.TAG, "// cerastes:trusted-contract defined elsewhere",
            "// and said so twice")), [])
        self.assertEqual(self.problems(replace_line(
            self.TAG, "// first a note", "// cerastes:trusted-contract in another unit")), [])

    def test_the_tag_must_be_next_to_the_method(self):
        self.assertTrue(self.problems(insert_before(self.EXT, "")))  # a blank line between
        self.assertTrue(self.problems(insert_before(self.EXT, "field f: Int")))  # code between
        probs = self.problems(remove(self.TAG), insert_before(
            "  var x__1", "  // cerastes:trusted-contract in the body is no excuse"))
        self.assertEqual(len(probs), 1, probs)

    def test_the_trusted_tag_needs_a_reason(self):
        probs = self.problems(replace_line(self.TAG, "// cerastes:trusted-contract"))
        self.assertEqual(len(probs), 1)
        self.assertIn("needs a reason", probs[0])

    def test_a_tag_in_a_block_comment_does_not_count(self):
        self.assertTrue(self.problems(replace_line(
            self.TAG, "/* cerastes:trusted-contract defined elsewhere */")))

    def test_the_tag_does_not_excuse_other_methods(self):
        probs = self.problems(insert_before("method g__3", "method h__9(y: Int) returns (res: Int)"))
        self.assertTrue(any("bodyless `method h__9`" in p for p in probs), probs)

    def test_a_method_with_a_body_needs_no_tag(self):
        self.assertEqual(lint(), [])  # f__2 and g__3 have bodies and no tag

    def test_a_bodyless_function_fails(self):
        probs = self.problems(drop_after("function c_abs", 3))
        self.assertTrue(any("bodyless `function c_abs`" in p for p in probs), probs)

    def test_a_bodyless_function_at_the_end_of_the_file_fails(self):
        text = "method m() {\n}\nfunction f(x: Int): Int\n"
        probs = lint_vpr_other(text, "", "t.vpr", "t.vpr.map")
        self.assertEqual(len(probs), 1)
        self.assertIn("bodyless `function f`", probs[0])

    def test_a_function_with_a_body_and_a_contract_passes(self):
        text = ("function f(x: Int): Int\n  requires x > 0\n  ensures result > 0\n{\n  x\n}\n")
        probs = lint_vpr_other(text, "", "t.vpr", "t.vpr.map")
        self.assertTrue(all("bodyless" not in p for p in probs), probs)

    def test_a_quantifier_trigger_is_not_a_body(self):
        # `{ f(i) }` after `::` must not make a bodyless function look like it has a body,
        # and a body that contains a trigger is still a body
        bodyless = "function f(x: Int): Int\n  requires forall i: Int :: { f(i) } f(i) > 0\n"
        probs = lint_vpr_other(bodyless, "", "t.vpr", "t.vpr.map")
        self.assertTrue(any("bodyless `function f`" in p for p in probs), probs)
        body = ("function f(x: Int): Int\n{\n  (forall i: Int :: { f(i) } f(i) > 0) ? x : 0\n}\n")
        self.assertTrue(all("bodyless" not in p for p in lint_vpr_other(body, "", "t.vpr", "t.vpr.map")))

    def test_domains_and_axioms_fail(self):
        domain = "domain D {\n  function g(): D\n}\n"
        probs = lint_vpr_other(domain, "", "t.vpr", "t.vpr.map")
        self.assertTrue(any("`domain D`" in p for p in probs), probs)  # and what is inside it
        axiom = "domain D {\n  axiom ax { true }\n}\n"
        probs = lint_vpr_other(axiom, "", "t.vpr", "t.vpr.map")
        self.assertTrue(any("`axiom`" in p for p in probs), probs)
        self.assertTrue(any("`domain D`" in p for p in probs), probs)

    def test_the_words_in_comments_are_fine(self):
        text = "// no domain, no axiom here\nmethod m() {\n  /* domain axiom */\n}\n"
        self.assertEqual(lint_vpr_other(text, "", "t.vpr", "t.vpr.map"), [])


class MapCoverageTests(unittest.TestCase):
    """Every kind of check line needs its entry."""

    def drop_entry_of(self, fragment):
        text, map_text = build(good_lines())
        lines = text.split("\n")
        (lineno,) = [i for i, l in enumerate(lines, start=1) if fragment in l]
        kept = [e for e in map_text.split("\n") if e and not e.startswith(f"{lineno}\t")]
        self.assertEqual(len(kept), len([e for e in map_text.split("\n") if e]) - 1)
        return lint_vpr_other(text, "\n".join(kept) + "\n", "t.vpr", "t.vpr.map"), lineno

    def test_each_kind_of_line(self):
        for fragment, kind in (("assert false", "assert"), ("  requires x > 0", "requires"),
                               ("  ensures res > x", "ensures"),
                               ("    invariant 0 <= i__1", "invariant"),
                               ("t__2 := f__2(y)", "call of f__2")):
            with self.subTest(kind):
                probs, lineno = self.drop_entry_of(fragment)
                self.assertEqual(len(probs), 1, probs)
                self.assertIn(f"t.vpr:{lineno}: `{kind}` line has no entry in the position map",
                              probs[0])

    def test_a_call_without_a_result_is_a_call(self):
        text = "method h(a: Int)\n{\n}\nmethod m() {\n  h(1)\n}\n"
        probs = lint_vpr_other(text, "", "t.vpr", "t.vpr.map")
        self.assertTrue(any(":5: `call of h` line" in p for p in probs), probs)

    def test_a_function_call_is_not_a_method_call(self):
        text = "function c_abs(x: Int): Int {\n  x\n}\nmethod m() returns (r: Int) {\n" \
               "  r := c_abs(1)\n}\n"
        self.assertEqual(lint_vpr_other(text, "", "t.vpr", "t.vpr.map"), [])

    def test_a_method_call_with_several_results(self):
        text = "method h() returns (a: Int, b: Int)\n{\n}\nmethod m() {\n  var p: Int\n" \
               "  var q: Int\n  p, q := h()\n}\n"
        probs = lint_vpr_other(text, "", "t.vpr", "t.vpr.map")
        self.assertTrue(any("`call of h` line" in p for p in probs), probs)

    def test_commented_out_checks_need_no_entry(self):
        text = "method m() {\n  // assert false\n  /* assert true */\n}\n"
        self.assertEqual(lint_vpr_other(text, "", "t.vpr", "t.vpr.map"), [])

    def test_a_check_after_a_block_comment_is_still_a_check(self):
        text = "method m() {\n  /* x */ assert false\n}\n"
        self.assertTrue(lint_vpr_other(text, "", "t.vpr", "t.vpr.map"))

    def test_the_whole_map_missing_is_a_problem(self):
        text, _ = build(good_lines())
        probs = lint_vpr_other(text, None, "t.vpr", "t.vpr.map")
        self.assertEqual(len(probs), 1)
        self.assertIn("no position map (t.vpr.map)", probs[0])

    def test_an_empty_map_leaves_every_check_unmapped(self):
        text, _ = build(good_lines())
        probs = lint_vpr_other(text, "", "t.vpr", "t.vpr.map")
        self.assertEqual(len(probs), sum(isinstance(l, Check) for l in good_lines()))

    def test_an_entry_for_another_line_does_not_count(self):
        text, map_text = build(good_lines())
        shifted = "\n".join("\t".join([str(int(f.split("\t")[0]) + 1)] + f.split("\t")[1:])
                            for f in map_text.split("\n") if f) + "\n"
        self.assertTrue(lint_vpr_other(text, shifted, "t.vpr", "t.vpr.map"))


class MapFormatTests(unittest.TestCase):
    ENTRY = ["3", "assert.failed", "overflow", "t.c", "7", "5", "signed addition may overflow"]

    def parse(self, *fields_list):
        text = "\n".join("\t".join(f) for f in fields_list) + "\n"
        return lv.parse_map(text, "t.vpr.map")

    def test_a_good_entry(self):
        entries, problems = self.parse(self.ENTRY, self.ENTRY[:2] + ["check"] + self.ENTRY[3:]
                                       + ["t.c:2:the clause"])
        self.assertEqual(problems, [])
        self.assertEqual((entries[0].vpr_line, entries[0].error_id, entries[0].category,
                          entries[0].file, entries[0].line, entries[0].column), (
            3, "assert.failed", "overflow", "t.c", 7, 5))
        self.assertIsNone(entries[0].note)
        self.assertEqual(entries[1].note, "t.c:2:the clause")

    def test_every_driver_category_and_internal_are_known(self):
        for category in sorted(rt.DRIVER_CATEGORIES | {"internal"}):
            _, problems = self.parse(self.ENTRY[:2] + [category] + self.ENTRY[3:])
            self.assertEqual(problems, [], category)
        self.assertEqual(lv.MAP_CATEGORIES, rt.DRIVER_CATEGORIES | {"internal"})

    def test_the_viper_error_ids_are_the_harnesses(self):
        for error_id in ("assert.failed", "postcondition.violated", "call.precondition",
                         "invariant.not.established", "invariant.not.preserved",
                         "not.wellformed"):
            self.assertIn(error_id, lv.MAP_ERROR_IDS)
            _, problems = self.parse([self.ENTRY[0], error_id] + self.ENTRY[2:])
            self.assertEqual(problems, [], error_id)

    def test_application_precondition_is_not_an_id_of_the_map(self):
        # the prelude's functions have no preconditions: no emitted check can produce it
        self.assertNotIn("application.precondition", lv.MAP_ERROR_IDS)
        entries, problems = self.parse([self.ENTRY[0], "application.precondition"]
                                       + self.ENTRY[2:])
        self.assertEqual(entries, [])
        self.assertEqual(len(problems), 1)
        self.assertIn("unknown Viper error id `application.precondition`", problems[0])

    def test_malformed_entries(self):
        bad = {
            "too few fields": self.ENTRY[:6],
            "too many fields": self.ENTRY + ["note", "extra"],
            "empty note": self.ENTRY + [" "],
            "vpr line not a number": ["x"] + self.ENTRY[1:],
            "line not a number": self.ENTRY[:4] + ["x"] + self.ENTRY[5:],
            "column not a number": self.ENTRY[:5] + ["x"] + self.ENTRY[6:],
            "vpr line zero": ["0"] + self.ENTRY[1:],
            "line zero": self.ENTRY[:4] + ["0"] + self.ENTRY[5:],
            "column zero": self.ENTRY[:5] + ["0"] + self.ENTRY[6:],
            "negative line": self.ENTRY[:4] + ["-3"] + self.ENTRY[5:],
            "unknown id": [self.ENTRY[0], "nonsense"] + self.ENTRY[2:],
            "a message instead of an id": [self.ENTRY[0], "Assert might fail."] + self.ENTRY[2:],
            "unknown category": self.ENTRY[:2] + ["assert"] + self.ENTRY[3:],
            "category with capitals": self.ENTRY[:2] + ["Overflow"] + self.ENTRY[3:],
            "empty file": self.ENTRY[:3] + [" "] + self.ENTRY[4:],
            "empty message": self.ENTRY[:6] + [""],
        }
        for what, fields in bad.items():
            with self.subTest(what):
                entries, problems = self.parse(fields)
                self.assertEqual(entries, [], what)
                self.assertEqual(len(problems), 1, (what, problems))
                self.assertTrue(problems[0].startswith("t.vpr.map:1: "), problems)

    def test_spaces_instead_of_tabs_are_a_format_error(self):
        _, problems = lv.parse_map("3 assert.failed overflow t.c 7 5 msg\n", "m")
        self.assertEqual(len(problems), 1)

    def test_blank_lines_are_ignored_and_line_numbers_count_them(self):
        entries, problems = lv.parse_map("\n" + "\t".join(self.ENTRY) + "\n\nbad\n", "m")
        self.assertEqual(len(entries), 1)
        self.assertEqual(entries[0].map_line, 2)
        self.assertTrue(problems[0].startswith("m:4: "))

    def test_a_bad_map_fails_the_lint(self):
        text, _ = build(good_lines())
        probs = lint_vpr_other(text, "garbage\n", "t.vpr", "t.vpr.map")
        self.assertTrue(any(p.startswith("t.vpr.map:1:") for p in probs), probs)

    def test_entries_for_lines_that_are_not_there(self):
        text, map_text = build(good_lines())
        nlines = len(text.rstrip("\n").split("\n"))
        past = map_text + "\t".join([str(nlines + 1)] + self.ENTRY[1:]) + "\n"
        probs = lint_vpr_other(text, past, "t.vpr", "t.vpr.map")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("past the end of the file", probs[0])
        comment_line = 1  # `// prelude`
        blank = map_text + "\t".join([str(comment_line)] + self.ENTRY[1:]) + "\n"
        probs = lint_vpr_other(text, blank, "t.vpr", "t.vpr.map")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("blank or a comment", probs[0])

    def test_the_last_line_is_there(self):
        text, map_text = build(good_lines())
        nlines = len(text.rstrip("\n").split("\n"))
        last = map_text + "\t".join([str(nlines)] + self.ENTRY[1:]) + "\n"
        self.assertEqual(lint_vpr_other(text, last, "t.vpr", "t.vpr.map"), [])

    def test_two_entries_with_one_id_for_a_line_fail_two_ids_do_not(self):
        text, map_text = build(good_lines())
        # an `ensures` line (it may carry two ids)
        first = next(e.split("\t") for e in map_text.split("\n")
                     if e.split("\t")[1:2] == ["postcondition.violated"])
        twice = map_text + "\t".join(first) + "\n"
        probs = lint_vpr_other(text, twice, "t.vpr", "t.vpr.map")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("two entries with the Viper error id `postcondition.violated`", probs[0])
        other = map_text + "\t".join(first[:1] + ["not.wellformed"] + first[2:]) + "\n"
        self.assertEqual(lint_vpr_other(text, other, "t.vpr", "t.vpr.map"), [])


class TriggerAndImportTests(unittest.TestCase):
    """Auditor round 2, items 7 and 8."""

    def problems(self, text):
        return lint_vpr_other(text, "", "t.vpr", "t.vpr.map")

    BODYLESS = ("function f(x: Int): Int { x }\nfunction g(x: Int): Int { x }\n"
                "method m(n: Int) returns (r: Int)\n  ensures forall x: Int :: {SETS} f(x) == g(x)\n"
                "  ensures r > n\n")

    def test_a_bodyless_method_with_several_trigger_sets_is_still_bodyless(self):
        for sets in ("{ f(x) }", "{ f(x) } { g(x) }", "{ f(x) } { g(x) } { f(x), g(x) }",
                     "{f(x)}{g(x)}"):
            with self.subTest(sets):
                probs = self.problems(self.BODYLESS.replace("{SETS}", sets))
                self.assertTrue(any("bodyless `method m`" in p for p in probs), probs)

    def test_a_method_with_a_body_and_several_trigger_sets_has_one(self):
        text = self.BODYLESS.replace("{SETS}", "{ f(x) } { g(x) }") + "{\n  r := n + 1\n}\n"
        self.assertEqual([p for p in self.problems(text) if "bodyless" in p], [])

    def test_a_trigger_set_in_a_body_does_not_end_it(self):
        text = ("function f(x: Int): Int { x }\nmethod m(n: Int)\n{\n"
                "  var b: Bool := forall x: Int :: { f(x) } { f(x + 1) } f(x) == x\n}\n"
                "method z(n: Int)\n")
        probs = self.problems(text)
        self.assertEqual(len([p for p in probs if "bodyless" in p]), 1, probs)
        self.assertIn("`method z`", [p for p in probs if "bodyless" in p][0])

    def test_import_is_refused(self):
        for text in ('import "evil.vpr"\nmethod z() {\n}\n', "import <evil.vpr>\n",
                     'method z() {\n}\nimport "late.vpr"\n'):
            with self.subTest(text):
                probs = self.problems(text)
                self.assertTrue(any("`import`: an emitted file is self-contained" in p
                                    for p in probs), probs)

    def test_import_in_a_comment_or_as_a_name_is_fine(self):
        text = ("// import \"evil.vpr\"\n/* import \"x\" */\nmethod z() {\n"
                "  var imported: Int := 0\n}\n")
        self.assertEqual(self.problems(text), [])


class OwnLineTests(unittest.TestCase):
    """Auditor round 2, items 9 and 10: every check is on its own line, with the
    map entry its kind can have."""

    def problems(self, text, map_lines=()):
        return lint_vpr_other(text, "".join(l + "\n" for l in map_lines), "t.vpr", "t.vpr.map")

    @staticmethod
    def entry(line, error_id="assert.failed"):
        return "\t".join([str(line), error_id, "check", "t.c", "1", "1", "m"])

    def test_the_probe_of_the_auditor(self):
        text = ("method k(x: Int) returns (r: Int) requires x > 0\n{\n"
                "  r := x; assert r > 100\n  if (x > 1) { assert x > 1000 }\n}\n")
        probs = self.problems(text)
        for line, word in ((1, "requires"), (3, "assert"), (4, "assert")):
            self.assertTrue(any(f":{line}: `{word}` does not start its line" in p
                                for p in probs), probs)

    def test_each_keyword_not_at_the_start_of_its_line(self):
        for word in ("assert", "requires", "ensures", "invariant"):
            for line in (f"method m() {word} true", f"  x := 1; {word} true",
                         f"  {{ {word} true }}", f"  {word} true {word} false",
                         f"  while (b) {word} true"):
                with self.subTest(line):
                    probs = self.problems(f"method m() {{\n{line}\n}}\n" if "method m()" not in line
                                          else line + "\n{\n}\n")
                    self.assertTrue(any("does not start its line" in p for p in probs), (line, probs))

    def test_a_keyword_that_starts_its_line_is_fine(self):
        text = "method m(x: Int)\n  requires x > 0\n  ensures true\n{\n  assert x > 0\n}\n"
        self.assertEqual([p for p in self.problems(text) if "does not start" in p], [])

    def test_calls_must_be_alone_on_their_line(self):
        head = "method h() returns (a: Int)\n{\n  a := 1\n}\n"
        for body in ("  var p: Int; p := h()", "  var p: Int\n  p := h(); p := h()",
                     "  if (true) { var p: Int; p := h() }", "  var p: Int\n  p := h() ; assert p > 0",
                     "  var p: Int\n  { p := h() }"):
            with self.subTest(body):
                probs = self.problems(head + "method u()\n{\n" + body + "\n}\n")
                self.assertTrue(any("the call of method `h` is not alone on its line" in p
                                    for p in probs), (body, probs))

    def test_a_call_alone_on_its_line_needs_its_entry_only(self):
        head = "method h() returns (a: Int)\n{\n  a := 1\n}\n"
        text = head + "method u()\n{\n  var p: Int\n  p := h()\n}\n"
        probs = self.problems(text)
        self.assertEqual(probs, ["t.vpr:8: `call of h` line has no entry in the position map"])
        self.assertEqual(self.problems(text, [self.entry(8, "call.precondition")]), [])

    def test_the_declaration_line_of_a_method_is_not_a_call(self):
        text = "method h() returns (a: Int)\n{\n  a := 1\n}\n"
        self.assertEqual(self.problems(text), [])

    def test_the_id_must_fit_the_kind_of_the_line(self):
        text = ("method m(x: Int) returns (r: Int)\n  requires x > 0\n  ensures r > x\n{\n"
                "  assert x > 0\n  r := x + 1\n}\n")
        ok = [self.entry(2, "not.wellformed"), self.entry(3, "postcondition.violated"),
              self.entry(3, "not.wellformed"), self.entry(5, "assert.failed")]
        self.assertEqual(self.problems(text, ok), [])
        ids = {"assert.failed", "postcondition.violated", "call.precondition",
               "invariant.not.established", "invariant.not.preserved", "call.failed",
               "not.wellformed"}
        for lineno, kind in ((2, "requires"), (3, "ensures"), (5, "assert")):
            right = {2: {"not.wellformed"}, 3: {"not.wellformed", "postcondition.violated"},
                     5: {"assert.failed"}}[lineno]
            for wrong in sorted(ids - right):
                with self.subTest(kind, wrong=wrong):
                    entries = [e for e in ok if not e.startswith(f"{lineno}\t")] + [
                        self.entry(lineno, wrong)]
                    if kind == "ensures":  # keep one right entry so only the wrong one is judged
                        entries.append(self.entry(lineno, "postcondition.violated"))
                    probs = self.problems(text, entries)
                    self.assertTrue(any(f"which a `{kind}` line cannot produce" in p
                                        for p in probs), (wrong, probs))

    def test_a_requires_line_never_carries_a_call_precondition(self):
        # Silicon reports a failed call precondition at the call, not at the clause
        text = "method m(x: Int)\n  requires x > 0\n{\n}\n"
        self.assertEqual(self.problems(text, [self.entry(2, "not.wellformed")]), [])
        for wrong in ("call.precondition", "application.precondition", "assert.failed"):
            probs = self.problems(text, [self.entry(2, wrong)])
            self.assertTrue(probs, wrong)

    def test_an_invariant_line_may_have_both_ids(self):
        text = "method m(n: Int)\n{\n  while (n > 0)\n    invariant n >= 0\n  {\n    assert true\n  }\n}\n"
        both = [self.entry(4, "invariant.not.established"), self.entry(4, "invariant.not.preserved"),
                self.entry(6)]
        self.assertEqual(self.problems(text, both), [])

    def test_an_invariant_line_has_either_invariant_id(self):
        text = "method m(n: Int)\n{\n  while (n > 0)\n    invariant n >= 0\n  {\n    assert true\n  }\n}\n"
        for error_id in ("invariant.not.established", "invariant.not.preserved"):
            probs = self.problems(text, [self.entry(4, error_id), self.entry(6)])
            self.assertEqual(probs, [], error_id)
        probs = self.problems(text, [self.entry(4, "assert.failed"), self.entry(6)])
        self.assertTrue(any("which a `invariant` line cannot produce" in p for p in probs), probs)

    def test_a_call_line_has_a_call_id(self):
        head = "method h() returns (a: Int)\n{\n  a := 1\n}\n"
        text = head + "method u()\n{\n  var p: Int\n  p := h()\n}\n"
        probs = self.problems(text, [self.entry(8, "assert.failed")])
        self.assertTrue(any("which a `call of h` line cannot produce" in p for p in probs), probs)


class SplitSourceTests(unittest.TestCase):
    def test_comments(self):
        s = lv.split_source("a // c1\n/* b\nc */ d // c2\ne /* x */ f\n")
        self.assertEqual([c.strip() for c in s.code], ["a", "", "d", "e   f", ""])
        self.assertEqual(s.comments, ["// c1", "", "// c2", "", ""])

    def test_a_block_comment_start_inside_a_line_comment_is_a_line_comment(self):
        s = lv.split_source("a // /* not a block\nb\n")
        self.assertEqual([c.strip() for c in s.code], ["a", "b", ""])

    def test_declarations(self):
        s = lv.split_source("function f(x: Int): Int\nmethod m() {\n}\nmethod n()\n  requires true\n")
        self.assertEqual([(d.kind, d.name, d.has_body) for d in lv.declarations(s)],
                         [("function", "f", False), ("method", "m", True), ("method", "n", False)])


class DirectoryTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.dir = Path(tmp.name).resolve()

    def put_pair(self, name="a.c.c23.vpr", lines=None, manifest=True):
        text, map_text = build(good_lines() if lines is None else lines)
        path = self.dir / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)
        path.with_name(name.split("/")[-1] + ".map").write_text(map_text)
        return path

    RUN = "20261016T120000Z-0a1b2c3d"

    def manifest(self, *names, run=True):
        head = f"# run-id: {self.RUN}\n" if run else ""
        (self.dir / lv.MANIFEST).write_text(head + "".join(f"{n}\n" for n in names))

    def lint_dir(self, expect=False):
        return lv.lint_directory(self.dir, lambda: expect)

    def test_the_files_of_the_manifest_pass(self):
        self.put_pair("a.c.c23.vpr")
        self.put_pair("sub/b.c.c++20.vpr")
        self.manifest("a.c.c23.vpr", "sub/b.c.c++20.vpr")
        self.assertEqual(self.lint_dir(), ([], 2, None, self.RUN))

    def test_a_file_the_manifest_lists_but_is_not_there_fails(self):
        self.put_pair("a.c.c23.vpr")
        self.manifest("a.c.c23.vpr", "b.c.c23.vpr")
        problems, n, _, _ = self.lint_dir()
        self.assertEqual(n, 1)
        self.assertEqual(len(problems), 1, problems)
        self.assertIn("`b.c.c23.vpr` is listed but not there", problems[0])

    def test_fewer_files_than_listed_fails_even_if_all_are_clean(self):
        self.put_pair("a.c.c23.vpr")
        self.manifest("a.c.c23.vpr", "b.c.c23.vpr", "c.c.c23.vpr")
        problems, _, _, _ = self.lint_dir()
        self.assertEqual(len(problems), 2)

    def test_an_empty_manifest_fails_whatever_else_is_there(self):
        self.manifest()
        problems, _, _, _ = self.lint_dir()
        self.assertTrue(any("manifest is empty" in p for p in problems), problems)
        self.put_pair()
        problems, _, _, _ = self.lint_dir()
        self.assertTrue(any("manifest is empty" in p for p in problems), problems)

    def test_an_empty_manifest_fails_with_no_verify_tests_too(self):
        self.manifest()
        self.assertTrue(self.lint_dir(expect=False)[0])

    def test_files_that_are_not_in_the_manifest_are_linted_too(self):
        self.put_pair("a.c.c23.vpr")
        bad = good_lines()
        bad.insert(5, "  inhale true")
        self.put_pair("extra.c.c23.vpr", lines=bad)
        self.manifest("a.c.c23.vpr", "extra.c.c23.vpr")
        problems, n, _, _ = self.lint_dir()
        self.assertEqual(n, 2)
        self.assertTrue(any("extra.c.c23.vpr" in p and "inhale" in p for p in problems), problems)

    def test_a_vpr_file_that_the_manifest_does_not_list_is_stale(self):
        self.put_pair("a.c.c23.vpr")
        self.put_pair("old.c.c23.vpr")  # left by an earlier run, or by a partial one
        self.manifest("a.c.c23.vpr")
        problems, n, _, run = self.lint_dir()
        self.assertEqual(n, 2)
        self.assertEqual(len(problems), 1, problems)
        self.assertIn("old.c.c23.vpr: not listed in the current MANIFEST (run "
                      f"{self.RUN})", problems[0])

    def test_the_manifest_needs_a_run_id_as_its_first_line(self):
        self.put_pair()
        self.manifest("a.c.c23.vpr", run=False)
        problems, _, _, run = self.lint_dir()
        self.assertEqual(len(problems), 1, problems)
        self.assertIn("the first line is not `# run-id: <id>`", problems[0])
        self.assertIsNone(run)
        # not the first line, or malformed
        for first in ("# run-id:", "run-id: x", "# runid: x", "# run-id: a b"):
            with self.subTest(first):
                (self.dir / lv.MANIFEST).write_text(first + "\na.c.c23.vpr\n")
                self.assertTrue(any("run-id" in p for p in self.lint_dir()[0]))
        (self.dir / lv.MANIFEST).write_text("a.c.c23.vpr\n# run-id: x\n")
        self.assertTrue(self.lint_dir()[0])

    def test_the_run_id_is_reported(self):
        self.put_pair()
        self.manifest("a.c.c23.vpr")
        self.assertEqual(self.lint_dir()[3], self.RUN)

    def test_a_manifest_that_is_not_below_the_directory_fails(self):
        self.put_pair()
        for entry in ("/etc/x.vpr", "../x.vpr", "a.txt"):
            with self.subTest(entry):
                self.manifest("a.c.c23.vpr", entry)
                problems, _, _, _ = self.lint_dir()
                self.assertTrue(any(f"`{entry}` is not a path" in p for p in problems), problems)

    def test_a_manifest_listing_a_file_twice_fails(self):
        self.put_pair()
        self.manifest("a.c.c23.vpr", "a.c.c23.vpr")
        self.assertTrue(any("listed twice" in p for p in self.lint_dir()[0]))

    def test_a_missing_map_fails(self):
        path = self.put_pair()
        path.with_name(path.name + ".map").unlink()
        self.manifest("a.c.c23.vpr")
        problems, _, _, _ = self.lint_dir()
        self.assertTrue(any("no position map" in p for p in problems), problems)

    def test_a_manifest_with_no_files_fails(self):
        self.manifest("a.c.c23.vpr")
        problems, n, _, _ = self.lint_dir()
        self.assertEqual(n, 0)
        self.assertTrue(any("no .vpr file to lint" in p for p in problems), problems)

    def test_no_manifest_and_no_files_is_clean_only_without_verify_tests(self):
        problems, n, note, run = self.lint_dir(expect=False)
        self.assertEqual((problems, n, note, run), ([], 0, "no emitted Viper to lint", None))
        problems, n, note, run = self.lint_dir(expect=True)
        self.assertEqual(n, 0)
        self.assertIsNone(note)
        self.assertEqual(len(problems), 1)
        self.assertIn("a verify test of the tree expects Viper", problems[0])

    def test_a_missing_directory_is_treated_like_an_empty_one(self):
        gone = self.dir / "nowhere"
        self.assertEqual(lv.lint_directory(gone, lambda: False)[0], [])
        self.assertTrue(lv.lint_directory(gone, lambda: True)[0])

    def test_files_without_a_manifest_cannot_be_tied_to_a_run(self):
        self.put_pair()
        problems, n, _, run = self.lint_dir()
        self.assertEqual(n, 1)
        self.assertIsNone(run)
        self.assertEqual(len(problems), 1, problems)
        self.assertIn("but no MANIFEST", problems[0])
        bad = good_lines()
        bad[1] = "function c_abs(x: Int): Int"
        bad.pop(2), bad.pop(2), bad.pop(2)
        self.put_pair("b.c.c23.vpr", lines=bad)
        problems, n, _, _ = self.lint_dir()
        self.assertEqual(n, 2)
        self.assertTrue(any("bodyless `function c_abs`" in p for p in problems), problems)  # still linted


class VerifyTestsExpectViperTests(unittest.TestCase):
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

    def test_no_tests_no_viper(self):
        self.assertFalse(lv.verify_tests_expect_viper(self.root))
        self.put("tests/il/a.cpp", "// expect: golden\nint f();\n")
        self.assertFalse(lv.verify_tests_expect_viper(self.root))

    def test_a_verified_or_error_test_expects_viper(self):
        for header in ("// expect: verified\n// expect: native-ok\n",
                       "// expect: error check @ L3\n"):
            for suffix in (".c", ".cpp"):
                with self.subTest(header, suffix=suffix):
                    # (a .c test of tests/scalar runs in both modes)
                    self.put(f"tests/scalar/x{suffix}", header + (
                        "// mode: c23, c++20\n" if suffix == ".c" else ""))
                    self.assertTrue(lv.verify_tests_expect_viper(self.root))
                    (self.root / f"tests/scalar/x{suffix}").unlink()

    def test_the_documentation_examples_count(self):
        self.put("doc/modules/ROOT/examples/verifying/first.c",
                 "// expect: verified\n// expect: native-ok\n// mode: c23, c++20\n")
        self.assertTrue(lv.verify_tests_expect_viper(self.root))

    def test_a_rejection_writes_no_viper(self):
        self.put("tests/scalar/negative/r.c", "// expect: rejected undefined_identifier @ L3\n"
                                              "// twin: tests/scalar/ok.c\n")
        self.assertFalse(lv.verify_tests_expect_viper(self.root))

    def test_viper_tests_run_silicon_not_the_driver(self):
        self.put("tests/viper-smoke/ok.vpr", "// expect: verified\n")
        self.put("tests/viper-smoke/fail.vpr", "// expect: error assert @ L3\n")
        self.assertFalse(lv.verify_tests_expect_viper(self.root))

    def test_a_bad_header_is_not_a_verify_test(self):
        self.put("tests/scalar/x.c", "// expect: error nonsense @ L3\n")
        self.assertFalse(lv.verify_tests_expect_viper(self.root))


class CommandLineTests(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.dir = Path(tmp.name).resolve()

    def run_lint(self, *args, expect=False):
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err), \
                mock.patch.object(lv, "verify_tests_expect_viper", return_value=expect):
            status = lv.main([str(a) for a in args])
        return status, out.getvalue(), err.getvalue()

    def put(self, name, lines=None):
        text, map_text = build(good_lines() if lines is None else lines)
        (self.dir / name).write_text(text)
        (self.dir / (name + ".map")).write_text(map_text)
        return self.dir / name

    def test_clean_is_zero(self):
        self.put("a.vpr")
        (self.dir / lv.MANIFEST).write_text("# run-id: 20261016T120000Z-0a1b2c3d\na.vpr\n")
        status, out, _ = self.run_lint(self.dir)
        self.assertEqual(status, 0, out)
        self.assertIn("1 files clean (run 20261016T120000Z-0a1b2c3d)", out)

    def test_a_problem_is_one(self):
        bad = good_lines()
        bad.insert(5, "  assume true")
        self.put("a.vpr", bad)
        (self.dir / lv.MANIFEST).write_text("# run-id: r\na.vpr\n")
        status, out, _ = self.run_lint(self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn("`assume` without", out)
        self.assertIn("1 problems in 1 files", out)

    def test_a_single_file(self):
        path = self.put("a.vpr")
        status, out, _ = self.run_lint(path)
        self.assertEqual(status, 0, out)
        bad = good_lines()
        bad.insert(5, "  inhale true")
        path = self.put("b.vpr", bad)
        self.assertEqual(self.run_lint(path)[0], 1)

    def test_usage_errors_are_two(self):
        status, _, err = self.run_lint(self.dir / "nowhere")
        self.assertEqual(status, 2)
        self.assertIn("not a directory or a .vpr file", err)
        (self.dir / "x.txt").write_text("x")
        self.assertEqual(self.run_lint(self.dir / "x.txt")[0], 2)

    def test_nothing_to_lint_is_zero_only_without_verify_tests(self):
        status, out, _ = self.run_lint(self.dir, expect=False)
        self.assertEqual(status, 0, out)
        self.assertIn("no emitted Viper to lint", out)
        status, out, _ = self.run_lint(self.dir, expect=True)
        self.assertEqual(status, 1, out)
        self.assertIn("a verify test of the tree expects Viper", out)

    def test_the_default_is_build_vpr(self):
        self.assertEqual(lv.DEFAULT_DIR, Path("build") / "vpr")
        seen = []
        with contextlib.redirect_stdout(io.StringIO()), \
                mock.patch.object(lv, "lint_directory",
                                  side_effect=lambda p, *a: seen.append(p) or ([], 0, None, None)):
            lv.main([])
        self.assertEqual(seen, [lv.ROOT / "build" / "vpr"])

    def test_the_manifest_name_is_the_harnesss(self):
        self.assertEqual(lv.MANIFEST, rt.VPR_MANIFEST)



class TerminationClauseTests(unittest.TestCase):
    """Decision 0013 item 5: every method and every `while` has a `decreases` clause (Silicon checks termination
    only where one is present), `decreases _` is an assumption allowed only on a bodyless method with the
    trusted-contract tag, and a loop's measure is a check (`termination.failed`) with its map entry; each rule
    has a twin that breaks exactly it."""

    def strict(self, *edits):
        text, map_text = edited(*edits)
        return lv.lint_vpr(text, map_text, "t.vpr", "t.vpr.map")

    def test_the_good_fixture_is_clean_under_every_rule(self):
        self.assertEqual(self.strict(), [])

    def test_a_method_without_a_clause_is_a_problem(self):
        probs = lv.lint_vpr(*build(self._lines_without("  ensures res > x", 1)), "t.vpr", "t.vpr.map")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`method f__2` has no `decreases` clause", probs[0])

    @staticmethod
    def _lines_without(after_fragment, n):
        lines = good_lines()
        i = find(lines, after_fragment)
        del lines[i + 1:i + 1 + n]
        return lines

    def test_a_trusted_method_without_a_clause_is_a_problem(self):
        probs = lv.lint_vpr(*build(self._lines_without("  ensures -2147483648 <= res && res <= 2147483647", 1)),
                            "t.vpr", "t.vpr.map")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`method ext__1` has no `decreases` clause", probs[0])

    def test_the_clause_of_another_method_does_not_count(self):
        # g__3's clause must not serve f__2: remove f__2's only
        probs = lv.lint_vpr(*build(self._lines_without("  ensures res > x", 1)), "t.vpr", "t.vpr.map")
        self.assertTrue(any("`method f__2`" in p for p in probs), probs)
        self.assertFalse(any("`method g__3`" in p for p in probs), probs)

    def test_a_wf_method_needs_its_empty_measure_too(self):
        lines = good_lines()
        lines[find(lines, "method g__3"):find(lines, "method g__3")] = [
            "method g__3__wf(y: Int)", Check("  requires y > 0", WF, "internal"), "{", "}"]
        probs = lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`method g__3__wf` has no `decreases` clause", probs[0])
        lines[find(lines, "method g__3__wf") + 2:find(lines, "method g__3__wf") + 2] = ["  decreases"]
        self.assertEqual(lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map"), [])

    def test_a_while_without_a_clause_is_a_problem(self):
        lines = good_lines()
        del lines[find(lines, "    decreases y - i__1")]
        probs = lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`while` loop without a `decreases` clause", probs[0])

    def test_a_while_with_its_brace_on_its_line_is_a_problem(self):
        lines = good_lines()
        lines[find(lines, "  while (i__1 < y)")] = "  while (i__1 < y) {"
        i = find(lines, "    decreases y - i__1")
        del lines[i]
        del lines[find(lines, "    invariant 0 <= i__1")]
        del lines[find(lines, "  {")]
        probs = lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")
        self.assertTrue(any("`while` loop without a `decreases` clause" in p for p in probs), probs)

    def test_a_clause_after_the_loop_body_does_not_count(self):
        lines = good_lines()
        i = find(lines, "    decreases y - i__1")
        moved = lines.pop(i)
        lines.insert(find(lines, "  var t__2: Int"), moved)
        probs = lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")
        self.assertTrue(any("`while` loop without a `decreases` clause" in p for p in probs), probs)

    def test_decreases_underscore_is_only_for_a_trusted_bodyless_method(self):
        # on a method with a body
        probs = lv.lint_vpr(*build(self._replaced("  ensures res > x", "  decreases", "  decreases _")),
                            "t.vpr", "t.vpr.map")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`decreases _` assumes termination", probs[0])
        # on a loop
        lines = good_lines()
        lines[find(lines, "    decreases y - i__1")] = "    decreases _"
        probs = lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")
        self.assertTrue(any("`decreases _` assumes termination" in p for p in probs), probs)
        # on a bodyless method without the tag (and the bodyless rule says so too)
        text, map_text = edited(remove(BodylessTests.TAG))
        probs = lv.lint_vpr(text, map_text, "t.vpr", "t.vpr.map")
        self.assertTrue(any("`decreases _` assumes termination" in p for p in probs), probs)
        self.assertTrue(any("bodyless `method ext__1`" in p for p in probs), probs)

    @staticmethod
    def _replaced(after_fragment, old, new):
        lines = good_lines()
        i = find(lines, after_fragment) + 1
        assert lines[i] == old, lines[i]
        lines[i] = new
        return lines

    def test_a_decreases_underscore_on_a_trusted_method_is_fine(self):
        # the good fixture has one
        text, _ = build(good_lines())
        self.assertIn("  decreases _\n", text)
        self.assertEqual(self.strict(), [])

    def test_a_loop_measure_is_a_check_with_its_entry(self):
        lines = good_lines()
        i = find(lines, "    decreases y - i__1")
        lines[i] = "    decreases y - i__1"  # no longer a Check: no map entry is generated
        probs = lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`decreases` line has no entry in the position map", probs[0])

    def test_the_loop_measure_entry_must_be_termination_failed(self):
        lines = good_lines()
        i = find(lines, "    decreases y - i__1")
        lines[i] = Check("    decreases y - i__1", "assert.failed", "termination")
        probs = lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("which a `decreases` line cannot produce", probs[0])

    def test_a_call_line_may_carry_termination_failed(self):
        text, map_text = edited()
        call_line = text.split("\n").index("  t__2 := f__2(y)") + 1
        extra = "\t".join([str(call_line), "termination.failed", "termination", "t.c", "7", "5", "recursive call"])
        self.assertEqual(lv.lint_vpr(text, map_text + extra + "\n", "t.vpr", "t.vpr.map"), [])
        wrong = "\t".join([str(call_line), "invariant.not.preserved", "invariant-preserved", "t.c", "7", "5", "x"])
        probs = lv.lint_vpr(text, map_text + wrong + "\n", "t.vpr", "t.vpr.map")
        self.assertTrue(any("which a `call of f__2` line cannot produce" in p for p in probs), probs)

    def test_decreases_must_start_its_line(self):
        lines = good_lines()
        i = find(lines, "    decreases y - i__1")
        lines[i] = Check("    invariant true decreases y - i__1", "termination.failed", "termination")
        probs = lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")
        self.assertTrue(any("`decreases` does not start its line" in p for p in probs), probs)

    def _loop_measure(self, text, error_id="termination.failed"):
        lines = good_lines()
        lines[find(lines, "    decreases y - i__1")] = Check(text, error_id, "termination")
        return lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")

    def test_decreases_star_is_a_problem_on_a_loop(self):
        # `decreases *` switches Silicon's termination check off (invariant 4, decision 0013)
        probs = self._loop_measure("    decreases *")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`decreases *` assumes termination", probs[0])
        self.assertIn("decision 0013", probs[0])

    def test_decreases_star_is_a_problem_on_a_method_even_a_trusted_one(self):
        # `decreases _` is allowed on the trusted bodyless method; `*` never is
        probs = lv.lint_vpr(*build(self._replaced("  ensures res > x", "  decreases", "  decreases *")),
                            "t.vpr", "t.vpr.map")
        self.assertTrue(any("`decreases *` assumes termination" in p for p in probs), probs)
        probs = lv.lint_vpr(*build(self._replaced("  ensures -2147483648 <= res && res <= 2147483647",
                                                  "  decreases _", "  decreases *")), "t.vpr", "t.vpr.map")
        self.assertTrue(any("`decreases *` assumes termination" in p for p in probs), probs)

    def test_decreases_star_among_a_list_of_measures_is_a_problem(self):
        probs = self._loop_measure("    decreases y - i__1, *")
        self.assertTrue(any("`decreases *` assumes termination" in p for p in probs), probs)

    def test_a_product_in_a_measure_is_not_decreases_star(self):
        # the twin of the star rule: a `*` inside a measure is arithmetic
        self.assertEqual(self._loop_measure("    decreases (y - i__1) * 2"), [])

    def test_a_conditional_decreases_is_a_problem(self):
        probs = self._loop_measure("    decreases y - i__1 if y > 0")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("conditional `decreases <measure> if <condition>`", probs[0])
        probs = lv.lint_vpr(*build(self._replaced("  ensures res > x", "  decreases", "  decreases if true")),
                            "t.vpr", "t.vpr.map")
        self.assertTrue(any("conditional `decreases" in p for p in probs), probs)

    def test_an_identifier_containing_if_is_not_a_conditional_decreases(self):
        self.assertEqual(self._loop_measure("    decreases y - i__1 + gif - elif_"), [])

    # A clause may run over several lines: the termination rules read it whole (defence in depth: the
    # emitter writes one line per clause today).
    def _loop_clause_lines(self, first, *more):
        """The loop's measure line replaced by `first` (a check with its map entry) and the lines `more`."""
        lines = good_lines()
        i = find(lines, "    decreases y - i__1")
        lines[i:i + 1] = [Check(first, "termination.failed", "termination"), *more]
        return lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")

    def test_a_decreases_star_on_the_next_line_is_a_problem(self):
        probs = self._loop_clause_lines("    decreases", "      *")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`decreases *` assumes termination", probs[0])
        # the problem is reported at the line of the keyword
        self.assertTrue(probs[0].startswith("t.vpr:%d:" % (find(self._loop_lines_text("    decreases", "      *"),
                                                                "    decreases") + 1)), probs)

    @staticmethod
    def _loop_lines_text(first, *more):
        lines = good_lines()
        i = find(lines, "    decreases y - i__1")
        lines[i:i + 1] = [first, *more]
        return lines

    def test_a_decreases_star_after_a_comma_on_the_next_line_is_a_problem(self):
        probs = self._loop_clause_lines("    decreases y - i__1,", "      *")
        self.assertTrue(any("`decreases *` assumes termination" in p for p in probs), probs)

    def test_a_split_measure_that_is_a_product_is_not_decreases_star(self):
        # the twin of the split-star rule: a `*` inside a measure that continues on the next line is arithmetic
        self.assertEqual(self._loop_clause_lines("    decreases (y - i__1)", "      * 2"), [])

    def test_a_conditional_decreases_on_the_next_line_is_a_problem(self):
        probs = self._loop_clause_lines("    decreases y - i__1", "      if y > 0")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("conditional `decreases <measure> if <condition>`", probs[0])
        probs = self._loop_clause_lines("    decreases", "      if y > 0")
        self.assertTrue(any("conditional `decreases" in p for p in probs), probs)

    def test_a_decreases_underscore_on_the_next_line_is_a_problem_except_where_it_is_allowed(self):
        probs = self._loop_clause_lines("    decreases", "      _")
        self.assertTrue(any("`decreases _` assumes termination" in p for p in probs), probs)
        lines = good_lines()
        i = find(lines, "  ensures res > x") + 1
        lines[i:i + 1] = ["  decreases", "    _"]  # on a method with a body
        probs = lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")
        self.assertTrue(any("`decreases _` assumes termination" in p for p in probs), probs)
        # the twin: the trusted bodyless method may have it, written over two lines too
        lines = good_lines()
        i = find(lines, "  ensures -2147483648 <= res && res <= 2147483647")
        lines[i + 1:i + 2] = ["  decreases", "    _"]
        self.assertEqual(lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map"), [])

    def test_a_split_clause_is_a_check_with_its_entry(self):
        # `decreases` alone on a line is an empty measure (no entry needed); with a measure on the next line it is
        # a measure, so its line needs the entry
        lines = good_lines()
        i = find(lines, "    decreases y - i__1")
        lines[i:i + 1] = ["    decreases", "      y - i__1"]
        probs = lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`decreases` line has no entry in the position map", probs[0])
        lines[i] = Check("    decreases", "termination.failed", "termination")
        self.assertEqual(lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map"), [])

    def test_a_clause_ends_at_a_brace_a_clause_or_a_declaration(self):
        for following in ("{", "}", "  requires true", "  ensures true", "  invariant true", "  decreases n", "while (x)",
                          "  label end", "  goto end", "  var v: Int", "  assert true", "  inhale true", "method m()",
                          "function f(): Int", "predicate p()", "field f: Int", "domain D {"):
            with self.subTest(following):
                src = lv.split_source("method m()\n  decreases n\n" + following + "\n")
                self.assertEqual(lv.decreases_measures(src)[2], "n")
        # blank lines and comments inside a clause do not end it
        src = lv.split_source("method m()\n  decreases n,\n\n  // comment\n  m\n{\n")
        self.assertEqual(lv.decreases_measures(src)[2], "n, m")

    def test_a_clause_that_continues_to_the_end_of_the_file_is_read_whole(self):
        src = lv.split_source("method m()\n  decreases n,\n   *\n")
        self.assertIn("*", lv._top_level_items(lv.decreases_measures(src)[2]))

    def test_a_termination_error_id_still_needs_a_known_category(self):
        text, map_text = edited()
        bad = map_text.replace("termination.failed\ttermination", "termination.failed\tnonsense", 1)
        self.assertNotEqual(bad, map_text)
        probs = lv.lint_vpr(text, bad, "t.vpr", "t.vpr.map")
        self.assertTrue(any("unknown category `nonsense`" in p for p in probs), probs)


class JumpTests(unittest.TestCase):
    """Decision 0014 item 9: a backward `goto`, a `label` that carries `invariant` clauses and a
    label defined twice in one method are problems (cycles Silicon does not check for termination;
    ambiguous targets); forward gotos and labels without invariants, as the emitter produces them
    (`goto l_brk`, `label l_brk`), stay accepted.  Each rule has a twin that breaks exactly it."""

    @staticmethod
    def strict(lines):
        return without_quantifier_rules(lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map"))

    @staticmethod
    def index_of_last(lines, fragment):
        return max(i for i, l in enumerate(lines) if fragment in (l.text if isinstance(l, Check) else l))

    def with_loop_labels(self, top_line, goto_line, label_first):
        """The good fixture with a `label` and a `goto` in g__3: `top_line` is the label's
        line(s), `goto_line` the jump; `label_first` puts the label before the goto."""
        lines = good_lines()
        start = find(lines, "  var i__1: Int := 0") + 1
        end = self.index_of_last(lines, "  label end")
        if label_first:
            lines[end:end] = [goto_line]
            lines[start:start] = list(top_line)
        else:
            lines[end:end] = list(top_line)
            lines[start:start] = [goto_line]
        return lines

    def test_a_forward_goto_to_a_label_without_invariants_is_clean(self):
        # the shape of a break: `goto l_brk__1` ... `label l_brk__1`
        lines = self.with_loop_labels(["  label l_brk__1"], "  goto l_brk__1", label_first=False)
        self.assertEqual(self.strict(lines), [])

    def test_a_backward_goto_is_a_problem(self):
        lines = self.with_loop_labels(["  label l_top__1"], "  goto l_top__1", label_first=True)
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("backward `goto l_top__1`", probs[0])
        self.assertIn("decision 0014", probs[0])
        text, _ = build(lines)
        goto_line = len(text.split("\n")[:text.split("\n").index("  goto l_top__1")]) + 1
        self.assertTrue(probs[0].startswith(f"t.vpr:{goto_line}:"), probs)

    def test_a_goto_right_after_its_own_label_is_backward(self):
        lines = self.with_loop_labels(["  label l_x__1", "  goto l_x__1"], "  // no jump", label_first=True)
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("backward `goto l_x__1`", probs[0])

    def test_the_same_label_name_in_another_method_is_not_a_backward_target(self):
        # `end` is defined in f__2 (earlier in the file) and g__3: a goto end in g__3 before its
        # own label is forward
        lines = good_lines()
        lines.insert(self.index_of_last(lines, "  label end"), "  goto end")
        self.assertEqual(self.strict(lines), [])

    def test_a_goto_to_a_label_of_an_earlier_method_is_not_flagged_as_backward(self):
        # an undefined label is Silicon's business, not the lint's: no cycle is possible
        lines = good_lines()
        lines.insert(self.index_of_last(lines, "  label end"), "  goto l_only_in_f__1")
        i = find(lines, "  goto end")
        lines.insert(i, "  label l_only_in_f__1")
        self.assertEqual(self.strict(lines), [])

    def test_a_label_with_an_invariant_on_the_next_line_is_a_problem(self):
        lines = good_lines()
        i = find(lines, "  while (i__1 < y)")
        lines[i:i] = ["  label l_loop__1", Check("    invariant 0 <= i__1", INV, "invariant-preserved")]
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`label l_loop__1` carries an `invariant`", probs[0])

    def test_a_label_with_an_invariant_on_its_own_line_is_a_problem(self):
        lines = good_lines()
        i = find(lines, "  while (i__1 < y)")
        lines[i:i] = ["  label l_loop__1 invariant true"]
        probs = self.strict(lines)
        self.assertTrue(any("`label l_loop__1` carries an `invariant`" in p for p in probs), probs)

    def test_a_label_before_a_while_is_not_a_label_with_an_invariant(self):
        # the invariant belongs to the `while`, which follows the label: the twin of the rule above
        lines = good_lines()
        i = find(lines, "  while (i__1 < y)")
        lines[i:i] = ["  label l_before_loop__1"]
        self.assertEqual(self.strict(lines), [])

    def test_a_label_defined_twice_in_one_method_is_a_problem(self):
        lines = good_lines()
        i = find(lines, "  var t__1: Int")
        lines[i:i] = ["  label l_twice__1", "  label l_twice__1"]
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("duplicate `label l_twice__1` in `method f__2`", probs[0])

    def test_one_label_name_in_two_methods_is_fine(self):
        text, _ = build(good_lines())
        self.assertEqual(text.count("label end"), 2)
        self.assertEqual(self.strict(good_lines()), [])

    def test_a_label_named_like_a_keyword_fragment_is_not_confused(self):
        lines = good_lines()
        lines.insert(find(lines, "  goto end"), "  var label_x: Int := 0")
        self.assertEqual(self.strict(lines), [])

    def test_the_rules_ignore_labels_and_gotos_in_comments(self):
        lines = good_lines()
        lines.insert(self.index_of_last(lines, "  label end"), "  // goto l_a")
        lines.insert(find(lines, "  var i__1: Int := 0") + 1, "  // label l_a")
        self.assertEqual(self.strict(lines), [])

    # -- a goto into a block (decision 0015) --

    def nested_loops(self, goto_at, label_at):
        """The good fixture with a second `while` inside the one of g__3, a `goto l_x` at the
        place `goto_at` and a `label l_x` at `label_at`, each one of 'before' (before the outer
        loop), 'outer' (in the outer body, before the inner loop), 'inner' (in the inner body),
        'after_inner' (the outer body, after the inner loop), 'after' (after the outer loop).
        At one place the goto comes first."""
        lines = good_lines()
        lines.insert(find(lines, "  while (i__1 < y)"), "@before@")
        lines.insert(find(lines, "  var t__2: Int"), "@after@")
        i = find(lines, "    i__1 := i__1 + 1")
        lines[i:i] = [
            "@outer@",
            "    while (i__1 < y)",
            Check("      invariant 0 <= i__1", INV, "invariant-preserved"),
            Check("      decreases y - i__1", "termination.failed", "termination"),
            "    {",
            "@inner@",
            "    }",
            "@after_inner@",
        ]
        out = []
        for line in lines:
            if isinstance(line, str) and line.startswith("@"):
                name = line.strip("@")
                out += (["  goto l_x"] if name == goto_at else []) \
                    + (["  label l_x"] if name == label_at else [])
            else:
                out.append(line)
        return out

    def test_a_goto_into_a_loop_body_is_a_problem(self):
        probs = self.strict(self.nested_loops("before", "outer"))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`goto l_x` into a block", probs[0])
        self.assertIn("decision 0015", probs[0])

    def test_the_problem_is_reported_at_the_goto(self):
        lines = self.nested_loops("before", "outer")
        text, _ = build(lines)
        goto_line = text.split("\n").index("  goto l_x") + 1
        (prob,) = self.strict(lines)
        self.assertTrue(prob.startswith(f"t.vpr:{goto_line}:"), prob)

    def test_a_goto_into_an_inner_loop_from_the_outer_one_is_a_problem(self):
        probs = self.strict(self.nested_loops("outer", "inner"))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("into a block", probs[0])

    def test_a_goto_from_one_loop_body_into_another_is_a_problem(self):
        # a jump from the inner body to a label in the body of a later loop: not enclosed
        lines = good_lines()
        i = find(lines, "  var t__2: Int")
        lines[i:i] = [
            "  while (i__1 < y)",
            Check("    invariant 0 <= i__1", INV, "invariant-preserved"),
            Check("    decreases y - i__1", "termination.failed", "termination"),
            "  {",
            "    label l_second",
            "  }",
        ]
        j = find(lines, "    i__1 := i__1 + 1")
        lines.insert(j, "    goto l_second")
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`goto l_second` into a block", probs[0])

    def test_a_goto_out_of_a_loop_is_clean(self):
        self.assertEqual(self.strict(self.nested_loops("outer", "after")), [])

    def test_a_goto_out_of_nested_loops_is_clean(self):
        self.assertEqual(self.strict(self.nested_loops("inner", "after")), [])
        self.assertEqual(self.strict(self.nested_loops("inner", "after_inner")), [])

    def test_a_goto_within_one_loop_body_is_clean(self):
        self.assertEqual(self.strict(self.nested_loops("outer", "after_inner")), [])
        self.assertEqual(self.strict(self.nested_loops("inner", "inner")), [])

    def test_a_goto_and_its_label_outside_every_loop_are_clean(self):
        self.assertEqual(self.strict(self.nested_loops("before", "after")), [])

    def test_a_quantifier_trigger_in_the_loop_header_does_not_open_the_body(self):
        # the trigger set `{ f(i) }` of an invariant is not the loop's `{`: the label after the
        # loop is outside the loop, and the label inside the body is inside it
        lines = good_lines()
        i = find(lines, "    invariant 0 <= i__1")
        lines[i] = Check("    invariant forall k: Int :: { c_abs(k) } c_abs(k) >= 0", INV,
                         "invariant-preserved")
        lines[i], lines[i + 1] = lines[i + 1], lines[i]  # the older `decreases` lint reads up to the first `{`
        j = find(lines, "  var t__2: Int")
        lines.insert(j, "  label l_out")
        lines.insert(find(lines, "  var i__1: Int := 0") + 1, "  goto l_out")
        self.assertEqual(self.strict(lines), [])
        k = find(lines, "    i__1 := i__1 + 1")
        lines[k + 1:k + 1] = ["    label l_in"]
        lines.insert(find(lines, "  var i__1: Int := 0") + 1, "  goto l_in")
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`goto l_in` into a block", probs[0])

    def test_two_trigger_sets_in_the_header_do_not_open_the_body(self):
        lines = good_lines()
        i = find(lines, "    invariant 0 <= i__1")
        lines[i] = Check("    invariant forall k: Int :: { c_abs(k) } { c_abs(k + 1) } c_abs(k) >= 0",
                         INV, "invariant-preserved")
        lines[i], lines[i + 1] = lines[i + 1], lines[i]
        lines.insert(find(lines, "  var t__2: Int"), "  label l_out")
        lines.insert(find(lines, "  var i__1: Int := 0") + 1, "  goto l_out")
        self.assertEqual(self.strict(lines), [])

    def test_a_goto_into_an_if_branch_is_a_problem(self):
        lines = good_lines()
        i = find(lines, "  var t__2: Int")
        lines[i:i] = ["  if (y > 0) {", "    label l_if", "  }"]
        lines.insert(find(lines, "  var i__1: Int := 0") + 1, "  goto l_if")
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`goto l_if` into a block", probs[0])

    def test_a_goto_from_the_else_branch_into_the_then_branch_is_a_problem(self):
        # the label is earlier than the goto only if the then-branch is written first: here the goto
        # is in the then-branch and the label in the else-branch, and the reverse needs a label that
        # comes after the goto: two ifs
        lines = good_lines()
        i = find(lines, "  var t__2: Int")
        lines[i:i] = ["  if (y > 0) {", "    goto l_else", "  } else {", "    label l_else", "  }"]
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`goto l_else` into a block", probs[0])

    def test_a_goto_out_of_nested_ifs_and_loops_is_clean(self):
        lines = good_lines()
        i = find(lines, "  var t__2: Int")
        lines[i:i] = ["  if (y > 0) {", "    if (y > 1) {", "      goto l_far", "    } else {",
                      "      goto l_far", "    }", "  }", "  label l_far"]
        self.assertEqual(self.strict(lines), [])
        lines = self.nested_loops("inner", "after")
        j = lines.index("  goto l_x")
        lines[j:j] = ["  if (y > 0) {", "  if (y > 1) {"]
        lines[j + 3:j + 3] = ["  }", "  }"]
        self.assertEqual(self.strict(lines), [])

    def test_a_goto_to_a_label_later_in_the_same_if_branch_is_clean(self):
        lines = good_lines()
        i = find(lines, "  var t__2: Int")
        lines[i:i] = ["  if (y > 0) {", "    goto l_in", "    label l_in", "  }"]
        self.assertEqual(self.strict(lines), [])

    def test_a_goto_from_a_loop_body_to_a_label_in_the_enclosing_branch_is_clean(self):
        lines = good_lines()
        i = find(lines, "  var i__1: Int := 0")
        lines.insert(i, "  if (y > 0) {")
        lines.insert(find(lines, "  var t__2: Int"), "  label l_end_of_branch")
        lines.insert(find(lines, "  var t__2: Int"), "  }")
        lines.insert(find(lines, "    i__1 := i__1 + 1"), "    goto l_end_of_branch")
        self.assertEqual(self.strict(lines), [])

    def test_a_goto_to_a_label_never_defined_is_not_this_rules_business(self):
        lines = good_lines()
        lines.insert(find(lines, "  var i__1: Int := 0") + 1, "  goto l_nowhere")
        self.assertEqual(self.strict(lines), [])

    def test_a_goto_in_a_comment_into_a_loop_is_ignored(self):
        lines = self.nested_loops("before", "outer")
        lines[lines.index("  goto l_x")] = "  // goto l_x"
        self.assertEqual(self.strict(lines), [])


class FlagTests(unittest.TestCase):
    """Decision 0019 item 5: the ghost flag `d_<name>` of a local without an initializer is set only by a write
    of that local (`<name> := ...` directly before `d_<name> := true`), and a reset (`d_<name> := false`)
    is directly followed by the dummy value (`<name> := <literal>`).  Each rule has a twin that breaks
    exactly it; the fixtures use the emitter's shapes (`var d_v_x__2: Bool`, `v_x__2 := 0`)."""

    X = "v_x__2"

    @staticmethod
    def strict(lines):
        return lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")

    def with_body(self, *body):
        """The good fixture with `body` (lines) inserted in g__3 before `var t__2: Int`, after the
        declarations of a variable and its flag."""
        lines = good_lines()
        i = find(lines, "  var t__2: Int")
        lines[i:i] = [f"  var {self.X}: Int", f"  var d_{self.X}: Bool", *body]
        return lines

    def probs(self, *body):
        return self.strict(self.with_body(*body))

    def good_body(self):
        x = self.X
        return [f"  d_{x} := false", f"  {x} := 0", f"  {x} := y", f"  d_{x} := true"]

    def test_the_emitters_shape_is_clean(self):
        self.assertEqual(self.probs(*self.good_body()), [])

    def test_a_dummy_may_be_any_literal_the_emitter_writes(self):
        x = self.X
        for dummy in ("0", "-3", "(-3)", "42", "false", "true"):
            with self.subTest(dummy):
                self.assertEqual(self.probs(f"  d_{x} := false", f"  {x} := {dummy}"), [])

    def test_blank_lines_and_comments_between_the_pair_are_skipped(self):
        x = self.X
        self.assertEqual(self.probs(f"  d_{x} := false", "", "  // the dummy", f"  {x} := 0", f"  {x} := y",
                                    "  /* set */", f"  d_{x} := true"), [])

    def test_a_write_in_a_branch_with_its_flag_is_clean(self):
        x = self.X
        self.assertEqual(self.probs(f"  d_{x} := false", f"  {x} := 0", "  if (y > 0) {", f"    {x} := y",
                                    f"    d_{x} := true", "  }"), [])

    def test_a_flag_set_without_the_write_is_a_problem(self):
        x = self.X
        probs = self.probs(f"  d_{x} := false", f"  {x} := 0", "  t__2 := y", f"  d_{x} := true")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn(f"`d_{x} := true` does not directly follow a write `{x} := ...`", probs[0])
        self.assertIn("decision 0019 item 5", probs[0])

    def test_the_problem_is_at_the_flag_line(self):
        x = self.X
        lines = self.with_body(f"  d_{x} := false", f"  {x} := 0", "  t__2 := y", f"  d_{x} := true")
        text, _ = build(lines)
        flag_line = text.split("\n").index(f"  d_{x} := true") + 1
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertTrue(probs[0].startswith(f"t.vpr:{flag_line}:"), probs)

    def test_an_unrelated_statement_between_the_write_and_the_flag_is_a_problem(self):
        x = self.X
        probs = self.probs(f"  d_{x} := false", f"  {x} := 0", f"  {x} := y", "  t__2 := y", f"  d_{x} := true")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("does not directly follow a write", probs[0])

    def test_the_write_of_another_variable_before_the_flag_is_a_problem(self):
        x = self.X
        probs = self.probs(f"  d_{x} := false", f"  {x} := 0", f"  t__2 := y", f"  d_{x} := true")
        self.assertEqual(len(probs), 1, probs)

    def test_a_write_of_a_longer_name_does_not_stand_for_the_variable(self):
        # `v_x__20 := ...` is not a write of `v_x__2`
        x = self.X
        probs = self.probs(f"  d_{x} := false", f"  {x} := 0", f"  {x}0 := y", f"  d_{x} := true")
        self.assertEqual(len(probs), 1, probs)

    def test_a_write_of_a_shorter_name_does_not_stand_for_the_variable(self):
        probs = self.probs("  d_v_x__20 := false", "  v_x__20 := 0", "  v_x__2 := y", "  d_v_x__20 := true")
        self.assertEqual(len(probs), 1, probs)

    def test_a_flag_set_as_the_very_first_line_is_a_problem(self):
        probs = lv.lint_vpr("d_v := true\n", "", "t.vpr", "t.vpr.map")
        self.assertTrue(any("does not directly follow a write" in p for p in probs), probs)

    def test_a_comment_that_looks_like_a_write_does_not_count(self):
        x = self.X
        probs = self.probs(f"  d_{x} := false", f"  {x} := 0", "  t__2 := y", f"  // {x} := y", f"  d_{x} := true")
        self.assertEqual(len(probs), 1, probs)

    def test_a_reset_that_is_not_followed_by_the_dummy_is_a_problem(self):
        x = self.X
        probs = self.probs(f"  d_{x} := false", "  t__2 := 0")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn(f"`d_{x} := false` is not directly followed by `{x} := <literal>`", probs[0])
        self.assertIn("decision 0019 item 5", probs[0])

    def test_a_reset_followed_by_a_value_that_is_not_a_literal_is_a_problem(self):
        x = self.X
        for value in ("y", "y + 1", "0 + y", "d_" + x, "(y)"):
            with self.subTest(value):
                probs = self.probs(f"  d_{x} := false", f"  {x} := {value}")
                self.assertEqual(len(probs), 1, probs)

    def test_a_reset_followed_by_the_dummy_of_another_variable_is_a_problem(self):
        x = self.X
        probs = self.probs(f"  d_{x} := false", "  t__2 := 0")
        self.assertEqual(len(probs), 1, probs)

    def test_a_reset_in_the_last_line_of_a_file_is_a_problem(self):
        probs = lv.lint_vpr("method m() {\n  d_v := false\n", "", "t.vpr", "t.vpr.map")
        self.assertTrue(any("is not directly followed by `v := <literal>`" in p for p in probs), probs)

    def test_a_flag_is_assigned_nothing_but_true_and_false(self):
        x = self.X
        for value in ("y > 0", f"d_{x}", "(true)", "!false", "true && true"):
            with self.subTest(value):
                probs = self.probs(f"  d_{x} := false", f"  {x} := 0", f"  {x} := y", f"  d_{x} := {value}")
                self.assertEqual(len(probs), 1, probs)
                self.assertIn(f"`d_{x}` is assigned", probs[0])

    def test_a_flag_assignment_must_stand_alone_on_its_line(self):
        x = self.X
        for line in (f"  {x} := y; d_{x} := true", f"  d_{x} := true; {x} := 0", f"  if (y > 0) {{ d_{x} := true }}"):
            with self.subTest(line):
                probs = self.probs(f"  d_{x} := false", f"  {x} := 0", f"  {x} := y", line)
                self.assertTrue(any("does not stand alone" in p or "is assigned" in p for p in probs), probs)

    def test_a_flag_declared_with_an_initial_value_is_a_problem(self):
        x = self.X
        lines = self.with_body(f"  d_{x} := false", f"  {x} := 0")
        lines[lines.index(f"  var d_{x}: Bool")] = f"  var d_{x}: Bool := true"
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("declared without an initializer", probs[0])

    def test_the_snapshot_and_the_reads_of_a_flag_are_not_assignments_to_it(self):
        x = self.X
        body = self.good_body() + [f"  var ds_{x}__1: Bool", f"  ds_{x}__1 := d_{x}",
                                   Check(f"  assert ds_{x}__1 ==> d_{x}", A, "uninitialized")]
        self.assertEqual(self.probs(*body), [])

    def test_two_flags_each_follow_their_own_write(self):
        reset = ["  d_a := false", "  a := 0", "  d_b := false", "  b := 0"]
        self.assertEqual(self.probs(*reset, "  a := y", "  d_a := true", "  b := y", "  d_b := true"), [])
        # the writes first and the flags after: each flag follows the wrong statement
        probs = self.probs(*reset, "  a := y", "  b := y", "  d_a := true", "  d_b := true")
        self.assertEqual(len(probs), 2, probs)
        self.assertIn("d_a := true", probs[0])
        self.assertIn("d_b := true", probs[1])

    def test_a_flag_that_is_not_ours_is_checked_all_the_same(self):
        # any `d_<name>` is a flag: the C variables are `v_...` in the emitted text
        probs = self.probs("  d_other := false", "  other := 0", "  t__2 := y", "  d_other := true")
        self.assertEqual(len(probs), 1, probs)

    def test_the_rule_runs_in_lint_vpr(self):
        import inspect
        self.assertIn("lint_flags(src, add)", inspect.getsource(lv.lint_vpr))
        self.assertTrue(callable(lv.lint_flags))

    def test_a_file_without_flags_is_unaffected(self):
        self.assertEqual(self.strict(good_lines()), [])


    def test_owner_ghosts_may_stand_between_a_write_and_its_flag(self):
        """Decision 0052 item 3: an owning local's `held_<v> := ...` and `tv_<v> := ...` follow its write and precede its flag."""
        x = self.X
        body = [f"  d_{x} := false", f"  {x} := 0", f"  {x} := y", "  held_v := y != null",
                "  held_v := false", f"  d_{x} := true"]
        self.assertEqual(self.probs(*body), [])

    def test_another_assignment_between_a_write_and_its_flag_is_still_a_problem(self):
        x = self.X
        for other in ("  t__2 := 1", "  held := y", "  xheld_v := y", "  tvv := y", "  tv_v.f := y"):
            with self.subTest(other=other):
                probs = self.probs(f"  d_{x} := false", f"  {x} := 0", f"  {x} := y", other, f"  d_{x} := true")
                self.assertTrue(any("does not directly follow a write" in p for p in probs), probs)

    def test_owner_ghosts_without_the_write_do_not_make_a_flag_legal(self):
        x = self.X
        probs = self.probs("  held_v := y != null", "  held_v := false", f"  d_{x} := true")
        self.assertTrue(any("does not directly follow a write" in p for p in probs), probs)


class BitwisePreludeTests(unittest.TestCase):
    """The range postcondition of a bitwise prelude function (decision 0012 item 11 as amended for 0023, owner
    2026-10-24) is the one `ensures` line that needs no position map entry; everything else about it stays checked."""

    @staticmethod
    def bit_function(name="c_and_32", ensures="auto", signature="(a: Int, b: Int): Int", extra=(), body="a"):
        width = int(name.rsplit("_", 1)[1]) if name.rsplit("_", 1)[1].isdigit() else 32
        if ensures == "auto":
            lo, hi = (0, 2 ** width - 1) if name.startswith("c_u") else (-2 ** (width - 1), 2 ** (width - 1) - 1)
            ensures = f"ensures {'(' + str(lo) + ')' if lo < 0 else lo} <= result && result <= {hi}"
        lines = [f"function {name}{signature}"] + [f"  {l}" for l in ([ensures] if ensures else []) + list(extra)]
        return "\n".join(lines + ["{", f"  {body}", "}"]) + "\n"

    def problems(self, text):
        return lint_vpr_other(text, "", "t.vpr", "t.vpr.map")

    def test_the_range_form_of_every_bitwise_function_is_accepted(self):
        for op in ("and", "or", "xor"):
            for u in ("", "u"):
                for width in (32, 64, 128):
                    name = f"c_{u}{op}_{width}"
                    with self.subTest(name):
                        self.assertEqual(self.problems(self.bit_function(name)), [])

    def test_the_signed_form_reads_exactly_as_the_emitter_writes_it(self):
        text = self.bit_function("c_xor_64")
        self.assertIn("ensures (-9223372036854775808) <= result && result <= 9223372036854775807", text)
        self.assertEqual(self.problems(text), [])
        unsigned = self.bit_function("c_uor_32")
        self.assertIn("ensures 0 <= result && result <= 4294967295", unsigned)
        self.assertEqual(self.problems(unsigned), [])

    def test_a_range_of_another_shape_is_reported(self):
        for ensures in ("ensures (-2147483648) <= result && result <= 2147483646",      # one short
                        "ensures (-2147483647) <= result && result <= 2147483647",      # the other end
                        "ensures result <= 2147483647",                                  # half a range
                        "ensures (-2147483648) <= result && result <= 2147483647 && result != 0",
                        "ensures true"):
            with self.subTest(ensures):
                probs = self.problems(self.bit_function("c_and_32", ensures))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("`ensures` line has no entry in the position map", probs[0])

    def test_the_unsigned_function_needs_the_unsigned_range(self):
        signed_range = "ensures (-2147483648) <= result && result <= 2147483647"
        self.assertEqual(len(self.problems(self.bit_function("c_uand_32", signed_range))), 1)
        unsigned_range = "ensures 0 <= result && result <= 4294967295"
        self.assertEqual(len(self.problems(self.bit_function("c_and_32", unsigned_range))), 1)

    def test_the_range_of_the_wrong_width_is_reported(self):
        wrong = "ensures (-2147483648) <= result && result <= 2147483647"
        self.assertEqual(len(self.problems(self.bit_function("c_and_64", wrong))), 1)

    def test_a_second_ensures_in_the_function_is_reported(self):
        probs = self.problems(self.bit_function("c_and_32", extra=("ensures result == 0",)))
        self.assertEqual(len(probs), 2, probs)  # neither is exempt once there are two
        both = self.bit_function("c_and_32", extra=(self.bit_function("c_and_32").split("\n")[1].strip(),))
        self.assertEqual(len(self.problems(both)), 2)

    def test_an_ensures_in_another_prelude_function_is_reported(self):
        for name in ("c_div", "c_rem", "c_abs", "c_and_16", "c_band_32", "c_and_32x", "c_shl_32"):
            with self.subTest(name):
                text = (f"function {name}(a: Int, b: Int): Int\n  ensures (-2147483648) <= result && "
                        "result <= 2147483647\n{\n  a\n}\n")
                probs = self.problems(text)
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("`ensures` line has no entry in the position map", probs[0])

    def test_a_requires_or_decreases_in_a_bitwise_function_is_reported(self):
        for extra in ("requires a >= 0", "requires true", "decreases a"):
            with self.subTest(extra):
                probs = self.problems(self.bit_function("c_and_32", extra=(extra,)))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn(extra.split()[0], probs[0])

    def test_a_bitwise_function_without_a_body_is_reported(self):
        probs = self.problems("function c_and_32(a: Int, b: Int): Int\n  ensures (-2147483648) <= result && "
                              "result <= 2147483647\n")
        self.assertTrue(any("bodyless `function c_and_32`" in p for p in probs), probs)
        probs = self.problems("function c_uor_64(a: Int, b: Int): Int\n")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("bodyless", probs[0])

    def test_a_different_signature_is_not_exempt(self):
        for signature in ("(x: Int, y: Int): Int", "(a: Int, b: Int, c: Int): Int", "(a: Int, b: Int): Bool",
                          "(a: Int): Int"):
            with self.subTest(signature):
                probs = self.problems(self.bit_function("c_and_32", signature=signature))
                self.assertEqual(len(probs), 1, probs)

    def test_a_method_of_that_name_is_not_exempt(self):
        text = ("method c_and_32(a: Int, b: Int) returns (r: Int)\n  ensures (-2147483648) <= r && r <= 2147483647\n"
                "  decreases\n{\n}\n")
        self.assertTrue(any("`ensures` line has no entry in the position map" in p for p in self.problems(text)))

    def test_only_the_range_line_is_exempt(self):
        method = ("method m__1(x: Int) returns (res: Int)\n  ensures res > x\n  decreases\n{\n  res := x + 1\n}\n")
        probs = self.problems(self.bit_function("c_and_32") + method)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`ensures` line has no entry in the position map", probs[0])
        self.assertIn("t.vpr:7:", probs[0])  # the method's ensures, not the function's (line 2)

    def test_an_ensures_in_the_body_of_a_bitwise_function_is_not_exempt(self):
        text = "function c_and_32(a: Int, b: Int): Int\n{\n  ensures a\n}\n"
        self.assertTrue(any("ensures" in p for p in self.problems(text)))

    def test_the_function_still_has_no_axiom_or_domain(self):
        probs = self.problems(self.bit_function("c_and_32") + "axiom a1 { true }\n")
        self.assertTrue(any("`axiom`" in p for p in probs), probs)
        probs = self.problems(self.bit_function("c_and_32") + "domain D {}\n")
        self.assertTrue(any("`domain D`" in p for p in probs), probs)


class XorMaskPreludeTests(unittest.TestCase):
    """The mask functions of `^` (decision 0027 item 3 as revised) carry one range implication as `ensures`, which needs
    no position map entry; the exemption is exact, so every other shape is still reported (each twin of an accepted form)."""

    @staticmethod
    def mask_function(name="c_xor_mask_32_8", ensures="auto", signature="(a: Int): Int", extra=(), body=r"(256 * (a \ 256)) + (255 - (a % 256))"):
        width = int(name.split("_")[-2]) if name.split("_")[-2].isdigit() else 32
        if ensures == "auto":
            lo, hi = (0, 2 ** width - 1) if name.startswith("c_uxor") else (-2 ** (width - 1), 2 ** (width - 1) - 1)
            t = f"({lo})" if lo < 0 else str(lo)
            ensures = f"ensures ({t} <= a && a <= {hi}) ==> ({t} <= result && result <= {hi})"
        lines = [f"function {name}{signature}"] + [f"  {l}" for l in ([ensures] if ensures else []) + list(extra)]
        return "\n".join(lines + ["{", f"  {body}", "}"]) + "\n"

    def problems(self, text):
        return lint_vpr_other(text, "", "t.vpr", "t.vpr.map")

    def test_the_emitted_form_of_every_mask_function_is_accepted(self):
        for u in ("", "u"):
            for c in ("", "c"):
                for width in (32, 64, 128):
                    for k in (0, 1, 8, width - 1) + ((width,) if u else ()):
                        name = f"c_{u}xor_{c}mask_{width}_{k}"
                        with self.subTest(name):
                            self.assertEqual(self.problems(self.mask_function(name)), [])

    def test_the_text_is_what_the_emitter_writes(self):
        text = self.mask_function("c_xor_cmask_32_4")
        self.assertIn("ensures ((-2147483648) <= a && a <= 2147483647) ==> ((-2147483648) <= result && result <= 2147483647)", text)
        self.assertEqual(self.problems(text), [])
        unsigned = self.mask_function("c_uxor_mask_32_32")
        self.assertIn("ensures (0 <= a && a <= 4294967295) ==> (0 <= result && result <= 4294967295)", unsigned)
        self.assertEqual(self.problems(unsigned), [])

    def test_a_range_of_another_shape_is_reported(self):
        for ensures in ("ensures ((-2147483648) <= a && a <= 2147483647) ==> ((-2147483648) <= result && result <= 2147483646)",
                        "ensures ((-2147483648) <= a && a <= 2147483646) ==> ((-2147483648) <= result && result <= 2147483647)",
                        "ensures ((-2147483647) <= a && a <= 2147483647) ==> ((-2147483648) <= result && result <= 2147483647)",
                        "ensures ((-2147483648) <= a && a <= 2147483647) ==> ((-2147483647) <= result && result <= 2147483647)",
                        "ensures (-2147483648) <= result && result <= 2147483647",          # the bit function's form
                        "ensures ((-2147483648) <= a && a <= 2147483647) ==> (result <= 2147483647)",
                        "ensures ((-2147483648) <= a && a <= 2147483647) && ((-2147483648) <= result && result <= 2147483647)",
                        "ensures true"):
            with self.subTest(ensures):
                probs = self.problems(self.mask_function("c_xor_mask_32_8", ensures))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("`ensures` line has no entry in the position map", probs[0])

    def test_the_unsigned_function_needs_the_unsigned_range_and_the_width_its_own(self):
        signed = "ensures ((-2147483648) <= a && a <= 2147483647) ==> ((-2147483648) <= result && result <= 2147483647)"
        self.assertEqual(len(self.problems(self.mask_function("c_uxor_mask_32_8", signed))), 1)
        unsigned = "ensures (0 <= a && a <= 4294967295) ==> (0 <= result && result <= 4294967295)"
        self.assertEqual(len(self.problems(self.mask_function("c_xor_mask_32_8", unsigned))), 1)
        self.assertEqual(len(self.problems(self.mask_function("c_xor_mask_64_8", signed))), 1)

    def test_a_different_name_shape_is_not_exempt(self):
        for name in ("c_xor_mask_32", "c_xor_mask_32_", "c_xor_mask_32_x", "c_xor_mask_16_8", "c_xor_masks_32_8",
                     "c_xor_mask_32_8x", "c_or_mask_32_8", "c_and_mask_32_8", "c_uxor_mmask_32_8", "my_c_xor_mask_32_8",
                     "c_xor_cmask_32_-1"):
            with self.subTest(name):
                text = (f"function {name}(a: Int): Int\n  ensures ((-2147483648) <= a && a <= 2147483647) ==> "
                        "((-2147483648) <= result && result <= 2147483647)\n{\n  a\n}\n")
                probs = self.problems(text)
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("`ensures` line has no entry in the position map", probs[0])

    def test_k_out_of_range_or_written_oddly_is_not_exempt(self):
        # k > N, a leading zero, a signed k = N (not a value of the type), a sign, a space and an empty k
        for name in ("c_uxor_mask_32_33", "c_xor_mask_32_32", "c_xor_cmask_32_32", "c_uxor_cmask_64_65", "c_xor_mask_128_128",
                     "c_xor_mask_32_08", "c_xor_mask_32_00", "c_uxor_cmask_32_0008", "c_xor_mask_32_+8", "c_xor_mask_32_8 ",
                     "c_xor_mask_32_", "c_xor_mask_32_1000", "c_xor_mask_32_999"):
            with self.subTest(name):
                text = (f"function {name}(a: Int): Int\n  ensures ((-2147483648) <= a && a <= 2147483647) ==> "
                        "((-2147483648) <= result && result <= 2147483647)\n{\n  a\n}\n")
                probs = self.problems(text)
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("`ensures` line has no entry in the position map", probs[0])
        # and the unsigned k = N the prelude allows, with its own range, is accepted (the twin of the refusals above)
        self.assertEqual(self.problems(self.mask_function("c_uxor_cmask_32_32")), [])
        self.assertEqual(self.problems(self.mask_function("c_xor_mask_32_31")), [])

    def test_a_second_ensures_is_reported(self):
        probs = self.problems(self.mask_function("c_xor_mask_32_8", extra=("ensures result == 0",)))
        self.assertEqual(len(probs), 2, probs)  # neither is exempt once there are two
        first = self.mask_function("c_xor_mask_32_8").split("\n")[1].strip()
        self.assertEqual(len(self.problems(self.mask_function("c_xor_mask_32_8", extra=(first,)))), 2)

    def test_a_requires_or_decreases_is_reported(self):
        for extra in ("requires a >= 0", "requires true", "decreases a"):
            with self.subTest(extra):
                probs = self.problems(self.mask_function("c_xor_mask_32_8", extra=(extra,)))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn(extra.split()[0], probs[0])

    def test_a_different_signature_is_not_exempt(self):
        for signature in ("(x: Int): Int", "(a: Int, b: Int): Int", "(a: Int): Bool", "(a: Int)"):
            with self.subTest(signature):
                probs = self.problems(self.mask_function("c_xor_mask_32_8", signature=signature))
                self.assertGreaterEqual(len(probs), 1, probs)

    def test_a_body_that_divides_by_a_variable_is_still_reported(self):
        # the exemption is only for the ensures line: the body is held to the remainder rule (power-of-two literals)
        probs = self.problems(self.mask_function("c_xor_mask_32_8", body=r"(a \ b) + (255 - (a % 256))"))
        self.assertTrue(any("divisor" in p for p in probs), probs)

    def test_a_bodyless_mask_function_is_reported(self):
        probs = self.problems("function c_xor_mask_32_8(a: Int): Int\n  ensures ((-2147483648) <= a && a <= 2147483647) ==> "
                              "((-2147483648) <= result && result <= 2147483647)\n")
        self.assertTrue(any("bodyless" in p for p in probs), probs)

    def test_a_method_of_that_name_is_not_exempt(self):
        text = ("method c_xor_mask_32_8(a: Int) returns (r: Int)\n  ensures ((-2147483648) <= a && a <= 2147483647) ==> "
                "((-2147483648) <= r && r <= 2147483647)\n  decreases\n{\n}\n")
        self.assertTrue(any("`ensures` line has no entry in the position map" in p for p in self.problems(text)))

    def test_only_the_range_line_is_exempt(self):
        method = "method m__1(x: Int) returns (res: Int)\n  ensures res > x\n  decreases\n{\n  res := x + 1\n}\n"
        probs = self.problems(self.mask_function("c_xor_mask_32_8") + method)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("t.vpr:7:", probs[0])  # the method's ensures, not the function's (line 2)


class RemainderTests(unittest.TestCase):
    """Decision 0026 item 7: Viper's `%` is the Euclidean remainder and C's is not, so outside the prelude's
    `c_rem` and `c_urem` the divisor of every emitted `%` must be a decimal power of two (the constant-mask
    rewrite `a & (2^k-1)` = `a % 2^k`; the complement `a & ~(2^k-1)` is `2^k * (a \\ 2^k)`, FloorDivisionTests).
    Every rejected shape has a twin in the accepted ones, so weakening the check fails a test here."""

    PRELUDE = [
        "function c_rem(a: Int, b: Int): Int",
        "{",
        "  (b == 0 ? 0 : (a >= 0 ? (b > 0 ? (a % b) : (a % (-b))) : (b > 0 ? (-((-a) % b)) : (-((-a) % (-b))))))",
        "}",
        "function c_urem(a: Int, b: Int): Int",
        "{",
        "  ((a >= 0 && b > 0) ? (a % b) : 0)",
        "}",
    ]

    @staticmethod
    def probs(*lines):
        text = "\n".join(lines) + "\n"
        return [p for p in lv.lint_vpr(text, "", "t.vpr", "t.vpr.map") if "`%`" in p]

    def in_method(self, *body):
        return self.probs(*self.PRELUDE, "method m__1(x: Int) returns (res: Int)", "{", *body, "}")

    def test_the_prelude_bodies_are_clean(self):
        self.assertEqual(self.probs(*self.PRELUDE), [])

    def test_the_mask_forms_are_clean(self):
        for body in ("  res := (x % 256)", "  res := x - x % 16", "  res := (x % 1)", "  res := x % 2",
                     "  res := (x % 4294967296)", "  res := (x % 18446744073709551616)",
                     "  res := (x % 340282366920938463463374607431768211456)",
                     "  res := (x - (x % 8))"):
            with self.subTest(body):
                self.assertEqual(self.in_method(body), [])

    def test_a_divisor_that_is_not_a_power_of_two_is_a_problem(self):
        for lit in ("0", "3", "255", "257", "1000", "4294967295", "18446744073709551615"):
            with self.subTest(lit):
                probs = self.in_method(f"  res := (x % {lit})")
                self.assertEqual(len(probs), 1, probs)
                self.assertIn(f"the divisor `{lit}`", probs[0])
                self.assertIn("decision 0026 item 7", probs[0])

    def test_a_divisor_that_is_not_a_plain_literal_is_a_problem(self):
        for divisor in ("y", "(16)", "-16", "(-16)", "x__1", "(1 + 1)", "16.0", "0x10", "256x", "16'", "+16"):
            with self.subTest(divisor):
                probs = self.in_method(f"  res := (x % {divisor})")
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("not a decimal literal", probs[0])

    def test_a_leading_zero_is_not_a_plain_literal(self):
        probs = self.in_method("  res := (x % 016)")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("the divisor `016`", probs[0])

    def test_the_problem_is_at_the_line_of_the_percent(self):
        probs = lv.lint_vpr("\n".join([*self.PRELUDE, "method m__1(x: Int) returns (res: Int)", "{",
                                       "  res := (x % 255)", "}"]) + "\n", "", "t.vpr", "t.vpr.map")
        probs = [p for p in probs if "`%`" in p]
        self.assertEqual(len(probs), 1, probs)
        self.assertTrue(probs[0].startswith(f"t.vpr:{len(self.PRELUDE) + 3}:"), probs)

    def test_each_percent_of_a_line_is_read(self):
        self.assertEqual(len(self.in_method("  res := (x % 8) + (x % 12) + (x % 16) + (x % 7)")), 2)

    def test_the_prelude_exemption_is_only_the_two_bodies(self):
        # a percent with a variable divisor after c_urem's body, in another function, in a method
        # and in a function that only has the prelude's name as a prefix
        for lines in (["function c_rem2(a: Int, b: Int): Int", "{", "  (a % b)", "}"],
                      ["function c_div(a: Int, b: Int): Int", "{", "  (a % b)", "}"],
                      ["method c_rem(a: Int, b: Int) returns (res: Int)", "{", "  res := (a % b)", "}"]):
            with self.subTest(lines[0]):
                self.assertEqual(len(self.probs(*self.PRELUDE, *lines)), 1)
        # and the exemption does not reach the next declaration
        probs = self.probs(*self.PRELUDE, "function g(a: Int): Int", "{", "  (a % 3)", "}")
        self.assertEqual(len(probs), 1, probs)

    def test_c_rem_and_c_urem_themselves_are_exempt_wherever_they_stand(self):
        self.assertEqual(self.probs(*self.PRELUDE[4:], *self.PRELUDE[:4]), [])
        self.assertEqual(self.probs(*self.PRELUDE[:4]), [])

    def test_comments_do_not_count(self):
        self.assertEqual(self.in_method("  res := x // % 3", "  /* x % y */ res := x"), [])

    def test_the_rule_applies_in_contracts_and_conditions_too(self):
        for line in ("  ensures res == x % 3", "  requires x % y == 0"):
            with self.subTest(line):
                text = "\n".join(["method m__1(x: Int, y: Int) returns (res: Int)", line, "{", "}"]) + "\n"
                probs = [p for p in lv.lint_vpr(text, "", "t.vpr", "t.vpr.map") if "`%`" in p]
                self.assertEqual(len(probs), 1, probs)

    def test_a_good_program_with_a_mask_remainder_is_clean_with_the_other_rules_on(self):
        lines = good_lines()
        lines.insert(find(lines, "  res := t__2"), "  t__2 := (t__2 % 256)")
        self.assertEqual(lint(lines), [])
        lines = good_lines()
        lines.insert(find(lines, "  res := t__2"), "  t__2 := (t__2 % 255)")
        probs = lint(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("the divisor `255`", probs[0])


class FloorDivisionTests(unittest.TestCase):
    r"""Decision 0026 item 7, the other half: Viper's `\` is the floor division, never C's `/`, so outside the
    prelude bodies that use it (`c_div`, `c_udiv`, `c_shr`, by name) the divisor of every `\` must be a decimal
    power of two (the complement mask `2^k * (a \ 2^k)`, and the bit functions' `a \ 2`).  The prelude texts are the
    emitter's (src/viper/prelude.cpp).  Every rejected shape has an accepted twin."""

    DIV = [
        "function c_div(a: Int, b: Int): Int",
        "{",
        r"  (b == 0 ? 0 : (a >= 0 ? (b > 0 ? (a \ b) : (-(a \ (-b)))) : (b > 0 ? (-((-a) \ b)) : ((-a) \ (-b)))))",
        "}",
    ]
    UDIV = ["function c_udiv(a: Int, b: Int): Int", "{", r"  ((a >= 0 && b > 0) ? (a \ b) : 0)", "}"]
    SHR = ["function c_shr(a: Int, n: Int): Int", "{", r"  (a \ c_pow2(n))", "}"]
    LEAF = ["function c_and_32(a: Int, b: Int): Int", "{",
            r"  let qa1 == (a \ 2) in let qb1 == (b \ 2) in (((a - 2 * qa1) + (b - 2 * qb1) == 2) ? 1 : 0)", "}"]
    COMPOSITE = ["function c_and_128(a: Int, b: Int): Int", "{",
                 r"  c_uand_64(a, b) + 18446744073709551616 * c_and_64(a \ 18446744073709551616, "
                 r"b \ 18446744073709551616)", "}"]

    @staticmethod
    def probs(*lines):
        text = "\n".join(lines) + "\n"
        return [p for p in lv.lint_vpr(text, "", "t.vpr", "t.vpr.map") if "`\\`" in p]

    def in_method(self, *body):
        return self.probs(*self.DIV, *self.UDIV, *self.SHR, *self.LEAF, *self.COMPOSITE,
                          "method m__1(x: Int) returns (res: Int)", "{", *body, "}")

    def test_the_prelude_bodies_are_clean(self):
        for lines in (self.DIV, self.UDIV, self.SHR, self.LEAF, self.COMPOSITE):
            with self.subTest(lines[0]):
                self.assertEqual(self.probs(*lines), [])

    def test_the_complement_mask_forms_are_clean(self):
        for body in (r"  res := (16 * (x \ 16))", r"  res := 8 * (x \ 8)", r"  res := (1 * (x \ 1))",
                     r"  res := (4294967296 * (x \ 4294967296))", r"  res := (x \ 18446744073709551616)",
                     r"  res := (340282366920938463463374607431768211456 * (x \ 340282366920938463463374607431768211456))"):
            with self.subTest(body):
                self.assertEqual(self.in_method(body), [])

    def test_a_divisor_that_is_not_a_power_of_two_is_a_problem(self):
        for lit in ("0", "3", "15", "255", "1000", "4294967295"):
            with self.subTest(lit):
                probs = self.in_method(rf"  res := ({lit} * (x \ {lit}))")
                self.assertEqual(len(probs), 1, probs)
                self.assertIn(f"the divisor `{lit}`", probs[0])
                self.assertIn("decision 0026 item 7", probs[0])

    def test_a_divisor_that_is_not_a_plain_literal_is_a_problem(self):
        for divisor in ("y", "(16)", "-16", "(-16)", "c_pow2(4)", "(1 + 1)", "16.0", "0x10", "256x", "16'", "+16"):
            with self.subTest(divisor):
                probs = self.in_method(rf"  res := (x \ {divisor})")
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("not a decimal literal", probs[0])

    def test_a_leading_zero_is_not_a_plain_literal(self):
        probs = self.in_method(r"  res := (x \ 016)")
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("the divisor `016`", probs[0])

    def test_the_problem_is_at_the_line_of_the_backslash(self):
        prelude = [*self.DIV, *self.UDIV, *self.SHR]
        text = "\n".join([*prelude, "method m__1(x: Int) returns (res: Int)", "{", r"  res := (x \ 15)", "}"]) + "\n"
        probs = [p for p in lv.lint_vpr(text, "", "t.vpr", "t.vpr.map") if "`\\`" in p]
        self.assertEqual(len(probs), 1, probs)
        self.assertTrue(probs[0].startswith(f"t.vpr:{len(prelude) + 3}:"), probs)

    def test_each_backslash_of_a_line_is_read(self):
        self.assertEqual(len(self.in_method(r"  res := (x \ 8) + (x \ 12) + (x \ 16) + (x \ y)")), 2)

    def test_the_prelude_exemption_is_by_name_and_only_the_bodies(self):
        # c_div, c_udiv and c_shr are exempt; a bit function, a function with another name, a method named
        # like a prelude function and the next declaration are not
        for lines in (["function c_div2(a: Int, b: Int): Int", "{", r"  (a \ b)", "}"],
                      ["function c_and_32(a: Int, b: Int): Int", "{", r"  (a \ b)", "}"],
                      ["function c_rem(a: Int, b: Int): Int", "{", r"  (a \ b)", "}"],
                      ["function c_urem(a: Int, b: Int): Int", "{", r"  (a \ b)", "}"],
                      ["method c_div(a: Int, b: Int) returns (res: Int)", "{", r"  res := (a \ b)", "}"]):
            with self.subTest(lines[0]):
                self.assertEqual(len(self.probs(*self.DIV, *self.UDIV, *self.SHR, *lines)), 1)
        probs = self.probs(*self.DIV, "function g(a: Int): Int", "{", r"  (a \ 3)", "}")
        self.assertEqual(len(probs), 1, probs)

    def test_the_two_operators_have_their_own_exemptions(self):
        # c_div may use `\` but not `%` with a variable divisor, and c_rem the other way round
        div_with_mod = ["function c_div(a: Int, b: Int): Int", "{", "  (a % b)", "}"]
        rem_with_div = ["function c_rem(a: Int, b: Int): Int", "{", r"  (a \ b)", "}"]
        probs = lv.lint_vpr("\n".join(div_with_mod) + "\n", "", "t.vpr", "t.vpr.map")
        self.assertEqual(len([p for p in probs if "`%`" in p]), 1, probs)
        probs = lv.lint_vpr("\n".join(rem_with_div) + "\n", "", "t.vpr", "t.vpr.map")
        self.assertEqual(len([p for p in probs if "`\\`" in p]), 1, probs)

    def test_a_percent_does_not_excuse_a_backslash_on_its_line(self):
        probs = self.in_method(r"  res := (x % 16) + (x \ 15)")
        self.assertEqual(len(probs), 1, probs)

    def test_comments_do_not_count(self):
        self.assertEqual(self.in_method(r"  res := x // \ 3", r"  /* x \ y */ res := x"), [])

    def test_the_rule_applies_in_contracts_and_conditions_too(self):
        for line in (r"  ensures res == x \ 3", r"  requires x \ y == 0"):
            with self.subTest(line):
                text = "\n".join(["method m__1(x: Int, y: Int) returns (res: Int)", line, "{", "}"]) + "\n"
                probs = [p for p in lv.lint_vpr(text, "", "t.vpr", "t.vpr.map") if "`\\`" in p]
                self.assertEqual(len(probs), 1, probs)

    def test_a_good_program_with_a_complement_mask_is_clean_with_the_other_rules_on(self):
        lines = good_lines()
        lines.insert(find(lines, "  res := t__2"), r"  t__2 := (16 * (t__2 \ 16))")
        self.assertEqual(lint(lines), [])
        lines = good_lines()
        lines.insert(find(lines, "  res := t__2"), r"  t__2 := (15 * (t__2 \ 15))")
        probs = lint(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("the divisor `15`", probs[0])


class MemoryModelTests(unittest.TestCase):
    """Decision 0028 item 2 and decision 0032 item 11: the `Block` domain (`// cerastes:domain-ok 0028`) and exactly
    its four axioms (`loc_inv` 0033, `loc_nonnull` 0034, `member_inv` 0035, `member_lv` 0036, each with its exact
    text and `// cerastes:axiom-ok <its record>`, comment lines directly above, the record's file there) are
    accepted, Viper's `new` is refused, the flag of a cell is a field written `<recv>.d_<t> := true` directly after
    `<recv>.v_<t> := ...`, and a `Ref` local's dummy is `null`.  Each rule has a twin that breaks exactly it, the
    fixture is the emitter's shape (tests/viper-smoke/memory/e2_swap.vpr)."""

    DOMAIN_TAG = "// cerastes:domain-ok 0028"
    AXIOMS = [("loc_inv", "0033"), ("loc_nonnull", "0034"), ("member_inv", "0035"), ("member_lv", "0036")]

    @staticmethod
    def strict(lines):
        return lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")

    @staticmethod
    def axiom_line(name):
        return "  " + lv.BLOCK_AXIOMS[name][1]

    def fixture(self, domain_tag=None, tags=None, texts=None, domain="domain Block {", drop=(), extra=()):
        """`tags` and `texts` override, by axiom name, the record of the tag and the text of the axiom; `drop`
        names axioms to leave out, `extra` lines are added to the domain."""
        d = self.DOMAIN_TAG if domain_tag is None else domain_tag
        axiom_lines = []
        for name, record in self.AXIOMS:
            if name in drop:
                continue
            tag = (tags or {}).get(name, "  // cerastes:axiom-ok " + record)
            axiom_lines += [tag, *(texts or {}).get(name, self.axiom_line(name)).split("\n")]
        lines = [
            "field v_int: Int",
            "field d_int: Bool",
            "field lv: Bool",
            "",
            *([d] if isinstance(d, str) else d),
            domain,
            "  function loc(b: Block, i: Int): Ref",
            "  function block_of(r: Ref): Block",
            *axiom_lines,
            *extra,
            "}",
            "",
            "// cerastes:trusted-contract allocation of an address-taken local of type int",
            *self.alloc_lines(),
            "method f__1()",
            "  decreases",
            "{",
            "  var x: Ref",
            "  var p: Ref",
            Check("  x := alloc_int()", PRE, "internal"),
            "  p := null",
            Check("  assert x != null", A, "null-dereference"),
            "  x.v_int := 1",
            "  x.d_int := true",
            "  exhale acc(x.v_int) && acc(x.d_int)",
            "}",
        ]
        return lines

    @staticmethod
    def alloc_lines(suffix="int", header=None, ensures=None, extra=()):
        """The lines of `alloc_<suffix>` as the emitter prints them (Check lines for the `ensures`)."""
        want = lv.alloc_contract(suffix)
        texts = ensures if ensures is not None else want[1:3]
        return [header or "method " + want[0].split(" ", 1)[1].replace("{", ""), *[Check("  " + t, P, "internal") for t in texts],
                *extra, "  decreases _"]

    def test_the_tagged_model_is_clean(self):
        self.assertEqual(self.strict(self.fixture()), [])

    def test_the_text_of_the_axioms_is_the_records(self):
        # the lint's table is the text of the records (0033-0036) and of 0028 item 2
        for name, record in self.AXIOMS:
            text = next(Path(__file__).resolve().parent.parent.joinpath("internal", "decisions").glob(f"{record}-*.md")).read_text()
            normalized = " ".join(text.split())
            self.assertIn(lv.BLOCK_AXIOMS[name][1], normalized, name)
            self.assertEqual(lv.BLOCK_AXIOMS[name][0], record)
        self.assertEqual(sorted(lv.BLOCK_AXIOMS), sorted(n for n, _ in self.AXIOMS))

    def test_whitespace_in_an_axiom_does_not_matter(self):
        text = "  axiom   loc_nonnull   {   forall b: Block, i: Int ::   { loc(b, i) }\n      loc(b, i)  !=  null   }"
        self.assertEqual(self.strict(self.fixture(texts={"loc_nonnull": text})), [])

    def test_functions_inside_the_domain_are_not_bodyless_top_level_functions(self):
        lines = self.fixture()
        self.assertTrue(any(isinstance(l, str) and l.startswith("  function loc") for l in lines))
        self.assertEqual(self.strict(lines), [])
        # the same function at top level is refused
        lines.append("function loc2(b: Int): Int")
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("bodyless `function loc2`", probs[0])

    def test_exhale_is_not_restricted(self):
        lines = self.fixture()
        self.assertTrue(any(isinstance(l, str) and "exhale" in l for l in lines))
        self.assertEqual(self.strict(lines), [])

    def test_inhale_and_assume_are_still_refused_without_their_tag(self):
        lines = self.fixture()
        lines.insert(lines.index("  p := null"), "  inhale acc(x.v_int)")
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`inhale` without `// cerastes:inhale-ok", probs[0])

    # --- the domain's tag ---

    def test_a_domain_without_the_tag_is_a_problem(self):
        lines = self.fixture()
        i = lines.index(self.DOMAIN_TAG)
        del lines[i]
        probs = self.strict(lines)
        # the domain, and its four axioms with it
        self.assertEqual(len(probs), 5, probs)
        self.assertIn("`domain Block`", probs[0])
        self.assertTrue(probs[0].startswith(f"t.vpr:{i + 1}:"), probs)
        self.assertTrue(all("`axiom`" in p for p in probs[1:]), probs)

    def test_a_domain_tag_with_another_record_is_a_problem(self):
        # a record that does not exist, one that exists but is not 0028, a malformed one, none
        for tag in ("9999", "0012", "0033", "28", "abcd", "00280", ""):
            with self.subTest(tag):
                probs = self.strict(self.fixture(domain_tag=("// cerastes:domain-ok " + tag).rstrip()))
                self.assertTrue(any("must name decision 0028" in p for p in probs), probs)
                self.assertTrue(any("`domain Block`" in p for p in probs), probs)

    def test_a_domain_tag_must_be_directly_above(self):
        tag = self.DOMAIN_TAG
        # a blank line between, the tag below, the tag on the domain's own line, the tag in a block comment
        for variant in ("blank", "below", "same line", "block comment", "code between"):
            with self.subTest(variant):
                lines = self.fixture(domain_tag="")
                i = lines.index("domain Block {")
                del lines[i - 1]
                if variant == "blank":
                    lines[i - 1:i - 1] = [tag, ""]
                elif variant == "below":
                    lines.insert(i, tag)
                elif variant == "same line":
                    lines[i - 1] = "domain Block { " + tag
                elif variant == "block comment":
                    lines.insert(i - 1, "/* " + tag + " */")
                else:
                    lines[i - 1:i - 1] = [tag, "field w: Int"]
                probs = self.strict(lines)
                self.assertTrue(any("`domain Block`" in p for p in probs), probs)

    def test_the_tag_may_sit_in_a_run_of_comment_lines(self):
        lines = self.fixture(domain_tag=["// the memory model", self.DOMAIN_TAG, "// and why"])
        self.assertEqual(self.strict(lines), [])

    def test_only_a_domain_named_block_is_accepted(self):
        probs = self.strict(self.fixture(domain="domain Other {"))
        self.assertTrue(any("`domain Other`" in p for p in probs), probs)

    def test_the_axiom_tag_is_not_a_domain_tag_and_the_reverse(self):
        probs = self.strict(self.fixture(domain_tag="// cerastes:axiom-ok 0028"))
        self.assertTrue(any("`domain Block`" in p for p in probs), probs)
        probs = self.strict(self.fixture(tags={"loc_inv": "  // cerastes:domain-ok 0033"}))
        self.assertTrue(any("`axiom`:" in p for p in probs), probs)

    # --- the axioms' tags and texts ---

    def test_an_axiom_without_the_tag_is_a_problem_at_its_line(self):
        lines = self.fixture()
        i = lines.index(self.axiom_line("loc_nonnull"))
        del lines[i - 1]
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`axiom`", probs[0])
        self.assertTrue(probs[0].startswith(f"t.vpr:{i}:"), probs)

    def test_an_axiom_tag_with_another_record_is_a_problem(self):
        # a record that does not exist, another axiom's record, the domain's, a malformed one, none
        for name, record in self.AXIOMS:
            other = next(r for n, r in self.AXIOMS if n != name)
            for tag in ("9999", other, "0028", "33", ""):
                with self.subTest(axiom=name, tag=tag):
                    probs = self.strict(self.fixture(tags={name: ("  // cerastes:axiom-ok " + tag).rstrip()}))
                    self.assertEqual(len(probs), 2, probs)
                    self.assertIn(f"must name decision {record} for `axiom {name}`", probs[0])
                    self.assertIn("`axiom`:", probs[1])

    def test_an_axiom_tag_must_be_directly_above(self):
        lines = self.fixture()
        i = lines.index(self.axiom_line("loc_inv"))
        lines.insert(i, "")  # the tag, a blank line, the axiom
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`axiom`", probs[0])

    def test_a_fifth_axiom_is_a_problem_even_with_a_tag(self):
        for tag in ("0033", "0028", "0012"):
            with self.subTest(tag):
                extra = ["  // cerastes:axiom-ok " + tag, "  axiom bad { false }"]
                probs = self.strict(self.fixture(extra=extra))
                self.assertTrue(any("`axiom bad` is not one of the memory model's four axioms" in p for p in probs), probs)
                self.assertTrue(any("`axiom`:" in p for p in probs), probs)

    def test_the_same_axiom_twice_is_a_problem(self):
        extra = ["  // cerastes:axiom-ok 0034", self.axiom_line("loc_nonnull")]
        probs = self.strict(self.fixture(extra=extra))
        self.assertTrue(any("`axiom loc_nonnull` twice" in p for p in probs), probs)

    def test_an_altered_axiom_body_is_a_problem(self):
        alterations = {
            "loc_inv": [("block_of(loc(b, i)) == b &&", "block_of(loc(b, i)) == b ||"),
                        ("{ loc(b, i) }", "{ block_of(loc(b, i)) }"), ("&& offset_of(loc(b, i)) == i", "")],
            "loc_nonnull": [("!= null", "== null"), ("i: Int", "i: Bool"), ("forall", "exists")],
            "member_inv": [("parent_of(member(r, k)) == r", "true"), ("index_of(member(r, k)) == k", "k == k")],
            "member_lv": [("{ lvloc(member(r, k)) }", "{ member(r, k) }"),
                          ("lvloc(member(r, k)) == lvloc(block_of(r))", "true")],
        }
        for name, edits in alterations.items():
            for old, new in edits:
                with self.subTest(axiom=name, edit=old):
                    text = self.axiom_line(name)
                    self.assertIn(old, text)
                    probs = self.strict(self.fixture(texts={name: text.replace(old, new)}))
                    self.assertEqual(len(probs), 2, probs)
                    self.assertIn(f"`axiom {name}` is not the text of decision", probs[0])

    def test_an_axiom_that_is_false_with_a_good_tag_is_a_problem(self):
        probs = self.strict(self.fixture(texts={"loc_inv": "  axiom loc_inv { false }"}))
        self.assertEqual(len(probs), 2, probs)

    def test_a_missing_axiom_is_a_problem_at_the_domain(self):
        for name, _record in self.AXIOMS:
            with self.subTest(name):
                lines = self.fixture(drop=(name,))
                probs = self.strict(lines)
                self.assertEqual(len(probs), 1, probs)
                self.assertIn(f"lacks `axiom {name}`", probs[0])
                self.assertTrue(probs[0].startswith(f"t.vpr:{lines.index('domain Block {') + 1}:"), probs)

    def test_a_tagged_axiom_outside_a_domain_is_a_problem(self):
        lines = self.fixture()
        lines += ["// cerastes:axiom-ok 0033", self.axiom_line("loc_inv").strip()]
        probs = without_quantifier_rules(self.strict(lines))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`axiom`", probs[0])

    def test_a_tagged_axiom_in_a_domain_that_is_refused_is_refused_too(self):
        lines = self.fixture()
        del lines[lines.index(self.DOMAIN_TAG)]
        probs = self.strict(lines)
        self.assertEqual(len([p for p in probs if "`axiom`:" in p]), 4, probs)
        self.assertEqual(len([p for p in probs if p.startswith("t.vpr:5: `domain Block`")]), 1, probs)

    def test_the_record_must_have_a_file(self):
        import tempfile
        with tempfile.TemporaryDirectory() as tmp:
            for _name, record in self.AXIOMS:
                (Path(tmp) / f"{record}-a-record.md").write_text("# record\n")
            (Path(tmp) / "0028-memory-model.md").write_text("# 0028\n")
            with mock.patch.object(lv, "DECISIONS_DIR", Path(tmp)):
                self.assertTrue(lv.record_exists("0033"))
                self.assertFalse(lv.record_exists("0778"))
                self.assertFalse(lv.record_exists(None))
                self.assertEqual(self.strict(self.fixture()), [])
                (Path(tmp) / "0035-a-record.md").unlink()
                probs = self.strict(self.fixture())
                self.assertTrue(any("must name decision 0035 for `axiom member_inv`" in p for p in probs), probs)
                (Path(tmp) / "0028-memory-model.md").unlink()
                probs = self.strict(self.fixture())
                self.assertTrue(any("must name decision 0028" in p for p in probs), probs)

    def test_the_real_records_of_the_model_resolve(self):
        for record in ("0028", "0032", "0033", "0034", "0035", "0036"):
            self.assertTrue(lv.record_exists(record), record)

    # --- the trusted allocation `alloc_<t>` (decision 0032 item 4) ---

    def with_alloc(self, **kw):
        lines = self.fixture()
        i = lines.index("method alloc_int() returns (r: Ref)")
        j = lines.index("  decreases _", i)
        lines[i:j + 1] = self.alloc_lines(**kw)
        return lines

    def test_the_emitters_allocation_is_clean_for_every_suffix(self):
        for suffix in ("int", "bool", "uchar", "enum_1_color", "long"):
            with self.subTest(suffix):
                lines = self.fixture()
                i = lines.index("method alloc_int() returns (r: Ref)")
                j = lines.index("  decreases _", i)
                lines[i:j + 1] = [f"method alloc_{suffix}() returns (r: Ref)",
                                  *[Check("  " + t, P, "internal") for t in lv.alloc_contract(suffix)[1:3]], "  decreases _"]
                self.assertEqual(self.strict(lines), [])

    def test_the_real_emitters_text_is_the_pinned_one(self):
        # the shape in tests/viper-smoke and in the emitted programs: parenthesized, `(-2)`
        self.assertEqual(lv.alloc_contract("int")[1],
                         "ensures ((r != null) && (r == loc(block_of(r), 0))) && (len(block_of(r)) == 1)")
        self.assertIn("(!r.d_int)) && acc(lvloc(block_of(r)).lv)", lv.alloc_contract("int")[2])

    def test_an_altered_allocation_contract_is_a_problem(self):
        good = lv.alloc_contract("int")
        alterations = {
            "dropped !r.d": [good[1], good[2].replace(" && (!r.d_int))", ")")],
            "extra fact": [good[1], good[2] + " && r.v_int == 0"],
            "ensures false": [good[1], good[2], "ensures false"],
            "first line weakened": [good[1].replace(" && (len(block_of(r)) == 1)", ""), good[2]],
            "wrong suffix": [good[1], good[2].replace("r.d_int", "r.d_bool")],
            "the liveness permission dropped": [good[1], good[2].replace(" && acc(lvloc(block_of(r)).lv)", "")],
            "only one ensures": [good[1]],
            "swapped": [good[2], good[1]],
        }
        for what, ensures in alterations.items():
            with self.subTest(what):
                probs = self.strict(self.with_alloc(ensures=[e for e in ensures]))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("`method alloc_int` is not the trusted allocation", probs[0])

    def test_an_extra_clause_or_a_body_on_an_allocation_is_a_problem(self):
        for what, extra in (("requires", ["  requires true"]), ("ensures", ["  ensures true"])):
            with self.subTest(what):
                lines = self.with_alloc()
                i = lines.index("  decreases _", lines.index("method alloc_int() returns (r: Ref)"))
                lines[i:i] = [Check(extra[0], P if what == "ensures" else WF, "internal")]
                probs = self.strict(lines)
                self.assertTrue(any("not the trusted allocation" in p for p in probs), probs)
        lines = self.with_alloc()
        i = lines.index("  decreases _", lines.index("method alloc_int() returns (r: Ref)"))
        lines[i + 1:i + 1] = ["{", "}"]
        probs = self.strict(lines)
        self.assertTrue(any("not the trusted allocation" in p for p in probs), probs)

    def test_a_method_named_alloc_something_else_is_a_problem(self):
        lines = self.fixture()
        lines.append("// cerastes:trusted-contract an allocation that is not")
        lines += ["method alloc_other() returns (r: Ref)", Check("  ensures r != null", P, "internal"), "  decreases _"]
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`method alloc_other` is not the trusted allocation", probs[0])
        # a malformed suffix, or no suffix at all
        for name in ("alloc_", "alloc_a-b"):
            with self.subTest(name):
                probs = lv.lint_vpr(f"// cerastes:trusted-contract x\nmethod {name}() returns (r: Ref)\n  decreases _\n", "", "t.vpr", "t.vpr.map")
                self.assertTrue(any("not the trusted allocation" in p for p in probs), probs)

    # --- the trusted struct allocation `alloc_struct_<n>_<tag>` (decision 0043 items 4 and 13) ---

    STRUCT_S = [
        "ensures ((r != null) && (r == loc(block_of(r), 0))) && (len(block_of(r)) == 1)",
        "ensures (((len(member(r, 0)) == 1) && acc(loc(member(r, 0), 0).v_int)) && acc(loc(member(r, 0), 0).d_int)) && (!loc(member(r, 0), 0).d_int)",
        "ensures (((len(member(r, 1)) == 1) && acc(loc(member(r, 1), 0).v_int)) && acc(loc(member(r, 1), 0).d_int)) && (!loc(member(r, 1), 0).d_int)",
        "ensures (lvloc(block_of(r)) == loc(block_of(r), (-2))) && acc(lvloc(block_of(r)).lv)",
    ]
    # struct T { struct S s; int z; }: the nested slot is loc(member(r, 0), 0); the slot fact is its own clause (or part of the member's)
    STRUCT_T = [
        "ensures ((r != null) && (r == loc(block_of(r), 0))) && (len(block_of(r)) == 1)",
        "ensures len(member(r, 0)) == 1",
        "ensures r != loc(member(r, 0), 0)",
        "ensures (((len(member(loc(member(r, 0), 0), 0)) == 1) && acc(loc(member(loc(member(r, 0), 0), 0), 0).v_int)) && acc(loc(member(loc(member(r, 0), 0), 0), 0).d_int)) && (!loc(member(loc(member(r, 0), 0), 0), 0).d_int)",
        "ensures (((len(member(loc(member(r, 0), 0), 1)) == 1) && acc(loc(member(loc(member(r, 0), 0), 1), 0).v_int)) && acc(loc(member(loc(member(r, 0), 0), 1), 0).d_int)) && (!loc(member(loc(member(r, 0), 0), 1), 0).d_int)",
        "ensures (((len(member(r, 1)) == 1) && acc(loc(member(r, 1), 0).v_int)) && acc(loc(member(r, 1), 0).d_int)) && (!loc(member(r, 1), 0).d_int)",
        "ensures (lvloc(block_of(r)) == loc(block_of(r), (-2))) && acc(lvloc(block_of(r)).lv)",
    ]

    def with_struct_alloc(self, ensures, name="alloc_struct_0_S", extra=(), body=False):
        lines = self.fixture()
        i = lines.index("method f__1()")
        block = ["// cerastes:trusted-contract allocation of a local of a struct type", f"method {name}() returns (r: Ref)",
                 *[Check("  " + t, P, "internal") for t in ensures], *extra, "  decreases _"]
        if body:
            block += ["{", "}"]
        lines[i:i] = block
        return lines

    def test_the_emitters_struct_allocations_are_clean(self):
        self.assertEqual(self.strict(self.with_struct_alloc(self.STRUCT_S)), [])
        self.assertEqual(self.strict(self.with_struct_alloc(self.STRUCT_T, name="alloc_struct_1_T")), [])
        # the slot fact as a conjunct of the nested member's clause
        merged = list(self.STRUCT_T)
        merged[1:3] = ["ensures len(member(r, 0)) == 1 && r != loc(member(r, 0), 0)"]
        self.assertEqual(self.strict(self.with_struct_alloc(merged, name="alloc_struct_1_T")), [])

    def test_a_struct_allocation_with_a_different_shape_is_a_problem(self):
        S, T = self.STRUCT_S, self.STRUCT_T
        dup = list(S)
        dup.insert(2, S[1])
        dup_spelled = list(S)
        dup_spelled.insert(2, S[1].replace("loc(member(r, 0), 0)", "loc(member(loc(block_of(r), 0), 0), 0)").replace("member(r, 0))", "member(loc(block_of(r), 0), 0))"))
        two_lv = list(S[:3]) + [S[3] + " && acc(lvloc(block_of(r)).lv)"]
        alterations = {
            "a duplicated leaf": dup,
            "a duplicated leaf spelled through block_of": dup_spelled,
            "a second lv permission": two_lv,
            "a missing !d": [S[0], S[1].replace(" && (!loc(member(r, 0), 0).d_int)", ""), S[2], S[3]],
            "len == 2": [S[0], S[1].replace("len(member(r, 0)) == 1", "len(member(r, 0)) == 2"), S[2], S[3]],
            "an extra conjunct": [S[0], S[1] + " && loc(member(r, 0), 0).v_int == 0", S[2], S[3]],
            "an unrooted chain": [S[0], S[1].replace("member(r, 0)", "member(q, 0)"), S[2], S[3]],
            "parent_of inside a chain": [S[0], S[1].replace("member(r, 0), 0).v_int", "member(parent_of(member(r, 0)), 0), 0).v_int"), S[2], S[3]],
            "a negative index": [S[0], S[1].replace("member(r, 0)", "member(r, -1)"), S[2], S[3]],
            "an undeclared field suffix": [S[0], S[1].replace("_int", "_long"), S[2], S[3]],
            "a flag of another suffix": [S[0], S[1].replace("d_int", "d_bool"), S[2], S[3]],
            "ensures false": list(S) + ["ensures false"],
            "the liveness clause missing": list(S[:3]),
            "the root facts weakened": [S[0].replace(" && (len(block_of(r)) == 1)", ""), S[1], S[2], S[3]],
            "no member block": [S[0], S[3]],
        }
        for what, ensures in alterations.items():
            with self.subTest(what):
                probs = self.strict(self.with_struct_alloc(ensures))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("`method alloc_struct_0_S` is not the trusted struct allocation", probs[0])

    def test_a_struct_allocation_needs_exactly_the_slot_facts(self):
        T = self.STRUCT_T
        alterations = {
            "the slot fact missing": T[:2] + T[3:],
            "the slot fact twice": T[:3] + [T[2]] + T[3:],
            "a fact in the wrong direction": T[:2] + ["ensures loc(member(r, 0), 0) != r"] + T[3:],
            "a fact between a leaf-level chain": T[:2] + ["ensures r != loc(member(loc(member(r, 0), 0), 0), 0)"] + T[3:],
            "a fact with a non-chain": T[:2] + ["ensures r != null"] + T[3:],
        }
        for what, ensures in alterations.items():
            with self.subTest(what):
                probs = self.strict(self.with_struct_alloc(ensures, name="alloc_struct_1_T"))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("is not the trusted struct allocation", probs[0])

    def test_a_struct_allocation_with_a_body_or_an_extra_clause_is_a_problem(self):
        probs = self.strict(self.with_struct_alloc(self.STRUCT_S, body=True))
        self.assertTrue(any("not the trusted struct allocation" in p for p in probs), probs)
        lines = self.with_struct_alloc(self.STRUCT_S)
        i = lines.index("  decreases _", lines.index("method alloc_struct_0_S() returns (r: Ref)"))
        lines[i:i] = [Check("  requires true", WF, "internal")]
        probs = self.strict(lines)
        self.assertTrue(any("not the trusted struct allocation" in p for p in probs), probs)

    def test_a_struct_allocation_is_not_the_scalar_allocation_and_back(self):
        # alloc_struct_0_S with the scalar `alloc_<t>` text, and alloc_int with a struct's clauses, are both refused
        scalar = self.alloc_lines(suffix="struct_0_S")[1:3]
        lines = self.with_struct_alloc([t.text if hasattr(t, "text") else t for t in scalar])
        probs = self.strict(lines)
        self.assertTrue(any("not the trusted struct allocation" in p for p in probs), probs)
        lines = self.with_alloc(ensures=self.STRUCT_S)
        probs = self.strict(lines)
        self.assertTrue(any("`method alloc_int` is not the trusted allocation" in p for p in probs), probs)

    def test_other_methods_are_not_alloc(self):
        lines = self.fixture()
        self.assertEqual(self.strict(lines), [])
        lines.append("method allocate_it()")
        lines.append("  decreases")
        lines.append("{")
        lines.append("}")
        self.assertEqual(self.strict(lines), [])

    # --- Viper's `new` ---

    def test_new_is_refused(self):
        for line in ("  x := new(v_int, d_int)", "  x := new (v_int)", "  x := new(*)"):
            with self.subTest(line):
                lines = self.fixture()
                lines.insert(lines.index("  p := null"), line)
                probs = self.strict(lines)
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("`new(...)`", probs[0])
                self.assertIn("decision 0028 item 2", probs[0])

    def test_new_is_refused_without_a_domain_too(self):
        probs = lv.lint_vpr("method m() {\n  x := new(f)\n}\n", "", "t.vpr", "t.vpr.map")
        self.assertTrue(any("`new(...)`" in p for p in probs), probs)

    def test_names_that_contain_new_and_comments_are_fine(self):
        for line in ("  x := renew(1)", "  x := new_cell(1)", "  x := y.new", "  // x := new(f)", "  /* new(f) */"):
            with self.subTest(line):
                text = f"function renew(a: Int): Int {{ a }}\nfunction new_cell(a: Int): Int {{ a }}\nmethod m() {{\n{line}\n}}\n"
                self.assertTrue(all("`new(...)`" not in p for p in lv.lint_vpr(text, "", "t.vpr", "t.vpr.map")))

    # --- the flag of a cell ---

    def test_a_field_flag_written_after_the_value_is_clean(self):
        self.assertEqual(self.strict(self.fixture()), [])

    def test_a_field_flag_with_blank_lines_and_comments_between_is_clean(self):
        lines = self.fixture()
        lines.insert(lines.index("  x.d_int := true"), "")
        lines.insert(lines.index("  x.d_int := true"), "  // the flag")
        self.assertEqual(self.strict(lines), [])

    def test_a_field_flag_set_without_the_write_of_its_cell_is_a_problem(self):
        lines = self.fixture()
        lines.insert(lines.index("  x.d_int := true"), "  p := x")
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`x.d_int := true` does not directly follow a write `x.v_int := ...`", probs[0])
        self.assertIn("decision 0032 item 11", probs[0])

    def test_a_field_flag_after_the_write_of_another_cell_or_type_is_a_problem(self):
        for write in ("  p.v_int := 1", "  x.v_bool := true", "  x.v_ptr_int := p"):
            with self.subTest(write):
                lines = self.fixture()
                lines[lines.index("  x.v_int := 1")] = write
                probs = self.strict(lines)
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("does not directly follow a write", probs[0])

    def test_a_field_flag_is_assigned_only_true(self):
        for value in ("false", "x.d_int", "p != null", "(true)"):
            with self.subTest(value):
                lines = self.fixture()
                lines[lines.index("  x.d_int := true")] = f"  x.d_int := {value}"
                probs = self.strict(lines)
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("is assigned", probs[0])

    def test_a_field_flag_assignment_must_stand_alone(self):
        for line in ("  x.v_int := 1; x.d_int := true", "  x.d_int := true; x.v_int := 1",
                     "  if (p != null) { x.d_int := true }", "  x.d_int := true x.d_int := true"):
            with self.subTest(line):
                lines = self.fixture()
                lines[lines.index("  x.d_int := true")] = line
                probs = self.strict(lines)
                self.assertTrue(any("does not stand alone" in p or "does not directly follow" in p
                                    for p in probs), probs)
                self.assertTrue(all("is assigned" not in p for p in probs), probs)

    def test_the_local_flag_rule_is_unchanged_by_the_field_form(self):
        # a local flag set after a write of a field of the same name is still a problem
        lines = self.fixture()
        lines.insert(lines.index("  exhale acc(x.v_int) && acc(x.d_int)"), "  var d_x: Bool")
        lines.insert(lines.index("  exhale acc(x.v_int) && acc(x.d_int)"), "  x.v_x := 1")
        lines.insert(lines.index("  exhale acc(x.v_int) && acc(x.d_int)"), "  d_x := true")
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`d_x := true` does not directly follow a write `x := ...`", probs[0])

    def test_a_ref_local_is_reset_to_null(self):
        lines = self.fixture()
        i = lines.index("  p := null")
        lines[i:i] = ["  var d_p: Bool", "  d_p := false"]
        self.assertEqual(self.strict(lines), [])

    def test_the_dummy_of_a_reset_is_still_a_literal(self):
        for value in ("q", "null || true", "nullx", "x"):
            with self.subTest(value):
                lines = self.fixture()
                i = lines.index("  p := null")
                lines[i:i] = ["  var d_p: Bool", "  d_p := false"]
                lines[i + 2] = f"  p := {value}"
                probs = self.strict(lines)
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("is not directly followed by `p := <literal>`", probs[0])

    def test_a_reset_to_null_of_another_variable_is_a_problem(self):
        lines = self.fixture()
        i = lines.index("  p := null")
        lines[i:i] = ["  var d_p: Bool", "  d_p := false", "  x := null"]
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)


class GhostAmountTests(unittest.TestCase):
    """Decision 0037 items 6 and 14: the `Perm` formals of a method are the amounts of the atoms of its contract.  Each has
    exactly one `requires none < a && a < write`, before any clause that mentions it (Silicon's well-formedness check
    reports `negative.permission` otherwise: tests/viper-smoke targets, S27), `wildcard` is refused, the case split
    `if (...) { }` is plain Viper, and `inhale`/`assume` stay refused.  Each rule has a twin that breaks exactly it."""

    BOUNDS_0 = "  requires (none < amt_0) && (amt_0 < write)"
    BOUNDS_1 = "  requires (none < amt_1) && (amt_1 < write)"
    ATOM_0 = "  requires (p != null) && acc(lvloc(block_of(p)).lv, amt_0)"

    @staticmethod
    def strict(lines):
        return lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")

    def fixture(self, header="method f__1(p: Ref, amt_0: Perm, amt_1: Perm)", clauses=None, body=None):
        clauses = clauses if clauses is not None else [self.BOUNDS_0, self.BOUNDS_1, self.ATOM_0,
                                                       "  requires acc(lvloc(block_of(p)).lv, amt_1)"]
        return [
            "field lv: Bool",
            header,
            *[Check(c, WF, "internal") for c in clauses],
            Check("  ensures acc(lvloc(block_of(p)).lv, amt_0)", P, "permission"),
            "  decreases",
            "{",
            *(body if body is not None else ["  if (lvloc(block_of(p)) == lvloc(block_of(p))) { }"]),
            "}",
        ]

    def test_the_emitters_shape_is_clean(self):
        self.assertEqual(self.strict(self.fixture()), [])

    def test_the_spelling_of_the_bounds_does_not_matter(self):
        for b in ("  requires none < amt_0 && amt_0 < write", "  requires ((none < amt_0) && (amt_0 < write))"):
            with self.subTest(b=b):
                self.assertEqual(self.strict(self.fixture(clauses=[b, self.BOUNDS_1, self.ATOM_0])), [])

    def test_a_missing_bound_is_a_problem(self):
        probs = self.strict(self.fixture(clauses=[self.BOUNDS_0, self.ATOM_0, "  requires acc(lvloc(block_of(p)).lv, amt_1)"]))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("ghost amount `amt_1` must have exactly one", probs[0])

    def test_a_half_bound_or_another_bound_is_a_problem(self):
        for b in ("  requires none < amt_1", "  requires amt_1 < write", "  requires (none < amt_1) && (amt_1 <= write)",
                  "  requires (none <= amt_1) && (amt_1 < write)", "  requires (none < amt_0) && (amt_0 < write)",
                  "  requires (amt_1 < write) && (none < amt_1)", "  requires (none < amt_1) && (amt_1 < write) && true"):
            with self.subTest(b=b):
                probs = self.strict(self.fixture(clauses=[self.BOUNDS_0, b, self.ATOM_0,
                                                          "  requires acc(lvloc(block_of(p)).lv, amt_1)"]))
                self.assertTrue(any("ghost amount `amt_1`" in p for p in probs), probs)

    def test_a_bound_in_an_ensures_is_a_problem(self):
        lines = self.fixture(clauses=[self.BOUNDS_0, self.ATOM_0, "  requires acc(lvloc(block_of(p)).lv, amt_1)"])
        i = lines.index("  decreases")
        lines[i:i] = [Check(self.BOUNDS_1.replace("requires", "ensures"), P, "internal")]
        probs = self.strict(lines)
        self.assertTrue(any("ghost amount `amt_1` must have exactly one" in p for p in probs), probs)

    def test_two_bounds_are_a_problem(self):
        probs = self.strict(self.fixture(clauses=[self.BOUNDS_0, self.BOUNDS_1, self.BOUNDS_0, self.ATOM_0,
                                                  "  requires acc(lvloc(block_of(p)).lv, amt_1)"]))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("found 2", probs[0])

    def test_a_clause_that_reads_an_amount_before_its_bounds_is_a_problem(self):
        probs = self.strict(self.fixture(clauses=[self.BOUNDS_0, self.ATOM_0, self.BOUNDS_1,
                                                  "  requires acc(lvloc(block_of(p)).lv, amt_1)"]))
        self.assertEqual(probs, [], "an atom of amt_0 before the bounds of amt_1 is fine: only a mention of amt_1 counts")
        for first in ("  requires acc(lvloc(block_of(p)).lv, amt_0)", "  requires p != null ==> amt_0 > none"):
            with self.subTest(first=first):
                probs = self.strict(self.fixture(clauses=[first, self.BOUNDS_0, self.BOUNDS_1,
                                                          "  requires acc(lvloc(block_of(p)).lv, amt_1)"]))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("mentions the ghost amount `amt_0` before its bounds", probs[0])

    def test_a_longer_name_that_contains_an_amount_is_not_a_mention(self):
        clauses = ["  requires p != null ==> amt_00 > none", self.BOUNDS_0, self.BOUNDS_1, self.ATOM_0,
                   "  requires acc(lvloc(block_of(p)).lv, amt_1)"]
        self.assertEqual(self.strict(self.fixture(clauses=clauses)), [])

    def test_only_perm_formals_are_amounts(self):
        lines = self.fixture(header="method f__1(p: Ref, n: Int, amt_0: Perm, amt_1: Perm)")
        self.assertEqual(self.strict(lines), [])
        lines = self.fixture(header="method f__1(p: Ref, amt_0: Perm, amt_1: Perm) returns (res: Perm)")
        i = lines.index("  decreases")
        lines[i:i] = [Check("  ensures none < res && res <= write", P, "internal")]
        self.assertEqual(self.strict(lines), [], "a Perm result is no formal amount: it needs no `requires` bounds, only its ensures row")

    def test_a_header_over_several_lines_is_read(self):
        lines = self.fixture(header="method f__1(p: Ref,\n    amt_0: Perm, amt_1: Perm)",
                             clauses=[self.BOUNDS_0, self.ATOM_0])
        probs = self.strict(lines)
        self.assertTrue(any("ghost amount `amt_1`" in p for p in probs), probs)

    def test_a_method_without_amounts_has_no_rule(self):
        self.assertEqual(self.strict(self.fixture(header="method f__1(p: Ref)", clauses=["  requires p != null"])), [])

    def test_wildcard_is_refused(self):
        for text in ("  requires acc(lvloc(block_of(p)).lv, wildcard)", "  var w: Perm := wildcard"):
            with self.subTest(text=text):
                lines = self.fixture()
                i = lines.index("{")
                lines[i + 1:i + 1] = [text]
                probs = [x for x in self.strict(lines) if "wildcard" in x]
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("decision 0037 item 14", probs[0])

    def test_a_name_that_contains_wildcard_is_not_the_keyword(self):
        lines = self.fixture(body=["  var v_wildcard__0: Int", "  var wildcards: Int"])
        self.assertEqual(self.strict(lines), [])

    def test_the_case_split_line_is_accepted_and_inhale_is_still_refused(self):
        lines = self.fixture(body=["  if (lvloc(block_of(p)) == lvloc(block_of(p))) { }", "  if (p == p) { }"])
        self.assertEqual(self.strict(lines), [])
        lines = self.fixture(body=["  inhale acc(lvloc(block_of(p)).lv, amt_0)"])
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`inhale` without", probs[0])
        lines = self.fixture(body=["  assume amt_0 == amt_1"])
        self.assertTrue(any("`assume` without" in p for p in self.strict(lines)))


class GhostResultTests(unittest.TestCase):
    """Decision 0052 item 5: a method's ghost `Perm` *results* are the output amounts of its partnerless `ensures` atoms.  Each has
    exactly one `ensures none < o && o <= write` (parentheses and spaces ignored) and no clause mentions it before that line, so a
    caller that receives it in `t, o := f(...)` receives an amount in `(none, write]` whatever the body, or a trusted
    contract, says.  A call's `Perm` target is allowed where the callee is a method of the file whose result at that position
    carries the row; every other `Perm` target of a call is still the rule of decision 0038 item 9.  Each rule has a twin that
    breaks exactly it."""

    ROW = "  ensures none < o_0 && o_0 <= write"
    BOUNDS = "  requires (none < amt_0) && (amt_0 < write)"
    ATOM = "  requires (p != null) && acc(lvloc(block_of(p)).lv, amt_0)"
    POST = "  ensures acc(lvloc(block_of(p)).lv, o_0)"

    @staticmethod
    def strict(lines):
        return lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")

    def callee(self, header="method g__1(p: Ref, amt_0: Perm) returns (res: Ref, o_0: Perm)", ensures=None, body=True):
        ensures = ensures if ensures is not None else [self.ROW, self.POST]
        out = [header, Check(self.BOUNDS, WF, "internal"), Check(self.ATOM, WF, "internal")]
        out += [Check(e, P, "permission") for e in ensures]
        if body:
            out += ["  decreases", "{", "  res := p", "  o_0 := amt_0", "}"]
        else:
            out += ["  decreases _"]
        return out

    def fixture(self, callee=None, caller_body=None, caller_vars=None):
        return [
            "field lv: Bool",
            *(callee if callee is not None else self.callee()),
            "method h__2(p: Ref, amt_0: Perm)",
            Check(self.BOUNDS, WF, "internal"),
            Check(self.ATOM, WF, "internal"),
            "  decreases",
            "{",
            *(caller_vars if caller_vars is not None else ["  var t: Ref", "  var o: Perm"]),
            *(caller_body if caller_body is not None else [Check("  t, o := g__1(p, amt_0)", PRE, "precondition")]),
            "}",
        ]

    def test_the_emitters_shape_is_clean(self):
        self.assertEqual(self.strict(self.fixture()), [])

    def test_the_spelling_of_the_row_does_not_matter(self):
        for row in ("  ensures (none < o_0) && (o_0 <= write)", "  ensures ((none < o_0) && (o_0 <= write))",
                    "  ensures none<o_0&&o_0<=write"):
            with self.subTest(row=row):
                self.assertEqual(self.strict(self.fixture(callee=self.callee(ensures=[row, self.POST]))), [])

    def test_a_missing_row_is_a_problem(self):
        probs = self.strict(self.fixture(callee=self.callee(ensures=[self.POST])))
        self.assertTrue(any("ghost amount result `o_0` must have exactly one `ensures none < o_0 && o_0 <= write`" in p and "found 0" in p
                            for p in probs), probs)

    def test_a_half_or_another_row_is_a_problem(self):
        for row in ("  ensures none < o_0", "  ensures o_0 <= write", "  ensures (none < o_0) && (o_0 < write)",
                    "  ensures (none <= o_0) && (o_0 <= write)", "  ensures (o_0 <= write) && (none < o_0)",
                    "  ensures (none < o_0) && (o_0 <= write) && true", "  requires (none < o_0) && (o_0 <= write)",
                    "  ensures (none < amt_0) && (amt_0 <= write)"):
            with self.subTest(row=row):
                probs = self.strict(self.fixture(callee=self.callee(ensures=[row, self.POST])))
                self.assertTrue(any("ghost amount result `o_0` must have exactly one" in p for p in probs), probs)

    def test_two_rows_are_a_problem(self):
        probs = self.strict(self.fixture(callee=self.callee(ensures=[self.ROW, self.ROW, self.POST])))
        self.assertTrue(any("found 2" in p for p in probs), probs)

    def test_a_clause_that_reads_the_result_before_its_row_is_a_problem(self):
        probs = self.strict(self.fixture(callee=self.callee(ensures=[self.POST, self.ROW])))
        mentions = [p for p in probs if "mentions the ghost amount result `o_0` before its bounds" in p]
        self.assertEqual(len(mentions), 1, probs)
        self.assertTrue(any("`Perm` local `o` is assigned" in p for p in probs),
                        "a row after a mention is no row: the caller's target is refused as well")
        self.assertEqual(self.strict(self.fixture(callee=self.callee(ensures=["  ensures acc(lvloc(block_of(p)).lv, amt_0)", self.ROW, self.POST]))), [],
                         "a clause that does not mention o_0 may come before its row")

    def test_a_longer_name_that_contains_the_result_is_not_a_mention(self):
        self.assertEqual(self.strict(self.fixture(callee=self.callee(ensures=["  ensures o_00 > none", self.ROW, self.POST]))), [])

    def test_only_perm_results_have_the_rule(self):
        header = "method g__1(p: Ref, amt_0: Perm) returns (res: Ref, n: Int)"
        callee = self.callee(header=header, ensures=["  ensures n > 0"])
        callee[callee.index("  res := p") + 1] = "  n := 1"
        self.assertEqual(self.strict(self.fixture(callee=callee, caller_body=["  var u: Ref", "  var m: Int", Check("  u, m := g__1(p, amt_0)", PRE, "precondition")],
                                                  caller_vars=[])), [])

    def test_a_header_over_several_lines_is_read(self):
        header = "method g__1(p: Ref, amt_0: Perm)\n    returns (res: Ref,\n    o_0: Perm)"
        probs = self.strict(self.fixture(callee=self.callee(header=header, ensures=[self.POST])))
        self.assertTrue(any("ghost amount result `o_0`" in p for p in probs), probs)

    def test_a_bodyless_trusted_method_needs_the_row_too(self):
        trusted = ["// cerastes:trusted-contract a function defined elsewhere", *self.callee(body=False)]
        self.assertEqual(self.strict(self.fixture(callee=trusted)), [])
        trusted = ["// cerastes:trusted-contract a function defined elsewhere", *self.callee(ensures=[self.POST], body=False)]
        probs = self.strict(self.fixture(callee=trusted))
        self.assertTrue(any("ghost amount result `o_0`" in p for p in probs), probs)

    def test_a_call_target_is_allowed_where_the_callee_result_carries_the_row(self):
        for body in (["  t, o := g__1(p, amt_0)"], ["  t,o := g__1(p, amt_0)"]):
            with self.subTest(body=body):
                self.assertEqual(self.strict(self.fixture(caller_body=[Check(b, PRE, "precondition") for b in body])), [])

    def test_the_perm_target_may_come_first_or_last(self):
        callee = self.callee(header="method g__1(p: Ref, amt_0: Perm) returns (o_0: Perm, res: Ref)")
        callee[callee.index("  res := p") + 1] = "  o_0 := amt_0"
        self.assertEqual(self.strict(self.fixture(callee=callee, caller_body=[Check("  o, t := g__1(p, amt_0)", PRE, "precondition")])), [])
        self.assertTrue(any("`Perm` local `o` is assigned" in p for p in
                            self.strict(self.fixture(callee=callee, caller_body=[Check("  t, o := g__1(p, amt_0)", PRE, "precondition")]))),
                        "the position matters: `o` is the second target, and the callee's second result is a `Ref`")

    def test_a_call_target_is_a_problem_where_the_callee_has_no_row(self):
        probs = self.strict(self.fixture(callee=self.callee(ensures=[self.POST])))
        self.assertTrue(any("`Perm` local `o` is assigned `g__1(p, amt_0)`" in p for p in probs), probs)

    def test_a_call_target_is_a_problem_where_the_callee_result_is_not_a_perm(self):
        header = "method g__1(p: Ref, amt_0: Perm) returns (res: Ref, o_0: Int)"
        callee = self.callee(header=header, ensures=["  ensures o_0 > 0"])
        probs = self.strict(self.fixture(callee=callee))
        self.assertTrue(any("`Perm` local `o` is assigned" in p for p in probs), probs)

    def test_a_call_target_is_a_problem_where_the_arity_differs(self):
        header = "method g__1(p: Ref, amt_0: Perm) returns (res: Ref, o_0: Perm, o_1: Perm)"
        callee = self.callee(header=header, ensures=[self.ROW, "  ensures none < o_1 && o_1 <= write", self.POST])
        callee[callee.index("  o_0 := amt_0")] = "  o_0 := amt_0\n  o_1 := amt_0"
        probs = self.strict(self.fixture(callee=callee))
        self.assertTrue(any("`Perm` local `o` is assigned" in p for p in probs), probs)

    def test_a_call_of_an_undeclared_method_is_still_a_problem(self):
        probs = self.strict(self.fixture(caller_body=[Check("  t, o := other__9(p, amt_0)", PRE, "precondition")]))
        self.assertTrue(any("`Perm` local `o` is assigned `other__9(p, amt_0)`" in p for p in probs), probs)

    def test_arithmetic_on_the_received_amount_is_still_a_problem(self):
        probs = self.strict(self.fixture(caller_body=[Check("  t, o := g__1(p, amt_0)", PRE, "precondition"), "  o := o / 2"]))
        self.assertTrue(any("`Perm` local `o` is assigned `o / 2`" in p for p in probs), probs)

    def test_a_call_target_in_a_loop_whose_invariant_uses_it_is_still_a_problem(self):
        body = [
            "  var i: Int := 0",
            "  while (i < 3)",
            Check("    invariant acc(lvloc(block_of(p)).lv, o)", INV, "internal"),
            Check("    decreases 3 - i", "termination.failed", "termination"),
            "  {",
            Check("    t, o := g__1(p, amt_0)", PRE, "precondition"),
            "    i := i + 1",
            "  }",
        ]
        probs = self.strict(self.fixture(caller_body=body))
        self.assertTrue(any("is assigned inside the loop of line" in p for p in probs), probs)


class LoopAmountTests(unittest.TestCase):
    """Decision 0038 item 9 (amending 0037 item 14): the amounts of a loop's invariant are `Perm` locals that snapshot what the
    method holds.  A `Perm` local is assigned only `perm(...)`, `none`, `write` or a conditional of those; it is never assigned
    inside a loop whose invariant mentions it; `acc` with such an amount is allowed in an invariant and `perm(...)` is not.
    Each rule has a twin that breaks exactly it, so weakening a rule fails here."""

    SNAP = ["  kv := perm(x.v_int)", "  kd := perm(x.d_int)"]
    INV_ACC = "    invariant acc(x.v_int, kv) && acc(x.d_int, kd)"
    INV_VAL = "    invariant kv > none ==> x.v_int == vs"

    @staticmethod
    def strict(lines):
        return lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")

    def fixture(self, snapshot=None, invariants=None, body=None, locals_=None, between=None):
        invs = invariants if invariants is not None else [self.INV_ACC, self.INV_VAL]
        return [
            "field v_int: Int",
            "field d_int: Bool",
            "method f__1(x: Ref, n: Int) returns (res: Perm)",
            Check("  requires n >= 0", WF, "internal"),
            Check("  ensures none < res && res <= write", P, "internal"),
            "  decreases",
            "{",
            *(locals_ if locals_ is not None else ["  var kv: Perm", "  var kd: Perm", "  var vs: Int", "  var i: Int"]),
            *(snapshot if snapshot is not None else self.SNAP),
            "  if (kv > none) { vs := x.v_int }",
            "  i := 0",
            "  while (i < n)",
            *[Check(t, INV, "internal") for t in invs],
            Check("    decreases n - i", "termination.failed", "termination"),
            "  {",
            *(body if body is not None else ["    i := i + 1"]),
            "  }",
            *(between if between is not None else []),
            "}",
        ]

    def test_the_emitters_shape_is_clean(self):
        self.assertEqual(self.strict(self.fixture()), [])

    def test_acc_with_a_snapshot_amount_is_allowed_in_an_invariant(self):
        probs = self.strict(self.fixture(invariants=[self.INV_ACC, "    invariant acc(lvloc(block_of(x)).lv, kd)"]))
        self.assertEqual(probs, [])

    def test_the_allowed_values(self):
        for value in ("perm(x.v_int)", "none", "write", "(perm(x.v_int))", "kd > none ? perm(x.v_int) : none",
                      "(kd > none) ? write : ((vs > 0) ? none : perm(x.d_int))", "perm(lvloc(block_of(x)).lv)"):
            with self.subTest(value=value):
                self.assertEqual(self.strict(self.fixture(snapshot=[f"  kv := {value}", "  kd := none"])), [])

    def test_a_declaration_with_an_allowed_initializer_is_clean(self):
        locals_ = ["  var kv: Perm := perm(x.v_int)", "  var kd: Perm := none", "  var vs: Int", "  var i: Int"]
        self.assertEqual(self.strict(self.fixture(locals_=locals_, snapshot=[])), [])

    def test_any_other_value_is_a_problem(self):
        for value in ("perm(x.v_int) / 2", "kd", "res", "write - kv", "perm(x.v_int) + perm(x.d_int)", "1 / 2",
                      "perm(x.v_int) < perm(x.d_int) ? perm(x.v_int) : res", "kd > none ? 1 / 2 : none",
                      "perm(x.v_int) perm(x.d_int)", "none none", "(none) / 2"):
            with self.subTest(value=value):
                probs = self.strict(self.fixture(snapshot=[f"  kv := {value}", "  kd := none"]))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("`Perm` local `kv` is assigned", probs[0])
                self.assertIn("decision 0038 item 9", probs[0])

    def test_a_bad_initializer_in_a_declaration_is_a_problem(self):
        locals_ = ["  var kv: Perm := perm(x.v_int) / 2", "  var kd: Perm := none", "  var vs: Int", "  var i: Int"]
        probs = self.strict(self.fixture(locals_=locals_, snapshot=[]))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("is initialized with", probs[0])

    def test_wildcard_is_one_problem_not_two(self):
        probs = self.strict(self.fixture(snapshot=["  kv := wildcard", "  kd := none"]))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`wildcard`", probs[0])

    def test_a_one_line_conditional_assignment_is_read(self):
        ok = self.fixture(snapshot=["  kv := none", "  if (n > 0) { kd := perm(x.d_int) }"])
        self.assertEqual(self.strict(ok), [])
        bad = self.fixture(snapshot=["  kv := none", "  if (n > 0) { kd := kv / 2 }"])
        probs = self.strict(bad)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`Perm` local `kd` is assigned `kv / 2`", probs[0])

    def test_a_result_of_type_perm_is_not_a_local(self):
        self.assertEqual(self.strict(self.fixture(between=["  res := kv / 2"])), [])

    def test_a_field_that_is_named_like_a_local_is_not_an_assignment(self):
        self.assertEqual(self.strict(self.fixture(body=["    i := i + 1", "    x.kv := 1"])), [])

    def test_a_perm_local_assigned_in_a_loop_whose_invariant_uses_it_is_a_problem(self):
        for body in (["    kv := perm(x.v_int)", "    i := i + 1"], ["    i := i + 1", "    kv := none"],
                     ["    if (i > 0) {", "      kd := write", "    }", "    i := i + 1"]):
            with self.subTest(body=body):
                probs = self.strict(self.fixture(body=body))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("is assigned inside the loop of line", probs[0])

    def test_a_loop_may_assign_a_perm_local_that_its_invariant_does_not_use(self):
        locals_ = ["  var kv: Perm", "  var kd: Perm", "  var vs: Int", "  var i: Int", "  var other: Perm"]
        self.assertEqual(self.strict(self.fixture(locals_=locals_, body=["    other := perm(x.v_int)", "    i := i + 1"])), [])

    def test_every_target_of_a_multi_target_call_is_read(self):
        """Decision 0044 item 13: `a, b := m(args)` assigns every target, so a `Perm` local is a problem as the first, a
        middle or the last target: a lint that reads only the target next to `:=` (the old rule) misses the first two."""
        for targets in ("kv, vs", "vs, kv", "vs, kv, i", "kv, vs, i", "kv,vs"):
            with self.subTest(targets=targets):
                probs = self.strict(self.fixture(snapshot=[f"  {targets} := other__9(n)", "  kd := none"]))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("`Perm` local `kv` is assigned `other__9(n)`", probs[0])

    def test_a_multi_target_call_of_non_perm_locals_is_clean(self):
        self.assertEqual(self.strict(self.fixture(snapshot=["  vs, i := other__9(n)", "  kv := none", "  kd := none"])), [])

    def test_two_perm_targets_are_two_problems(self):
        probs = self.strict(self.fixture(snapshot=["  kv, kd := other__9(n)"]))
        self.assertEqual(len(probs), 2, probs)
        self.assertIn("`Perm` local `kv`", probs[0] + probs[1])
        self.assertIn("`Perm` local `kd`", probs[0] + probs[1])

    def test_a_multi_target_call_in_a_loop_assigns_each_target_there(self):
        for targets in ("kv, vs", "vs, kv", "vs, kd, i"):
            with self.subTest(targets=targets):
                probs = self.strict(self.fixture(body=[f"    {targets} := other__9(n)"]))
                self.assertTrue(any("is assigned inside the loop of line" in p for p in probs), probs)

    def test_a_multi_target_call_in_a_loop_of_other_locals_is_clean(self):
        self.assertEqual(self.strict(self.fixture(body=["    vs, i := other__9(n)"])), [])

    def test_an_assignment_before_or_after_the_loop_is_fine(self):
        self.assertEqual(self.strict(self.fixture(between=["  kv := none", "  kd := perm(x.d_int)"])), [])

    def test_the_invariant_use_may_be_a_guard_not_an_amount(self):
        probs = self.strict(self.fixture(invariants=["    invariant 0 <= i", self.INV_VAL], body=["    kv := none", "    i := i + 1"]))
        self.assertEqual(len(probs), 1, probs)

    def test_a_name_that_contains_a_local_is_not_a_use(self):
        locals_ = ["  var kv: Perm", "  var kd: Perm", "  var vs: Int", "  var i: Int", "  var kv2: Perm"]
        probs = self.strict(self.fixture(locals_=locals_, invariants=[self.INV_ACC], body=["    kv2 := none", "    i := i + 1"]))
        self.assertEqual(probs, [])

    def test_the_inner_loop_snapshot_in_an_outer_body_is_fine_and_the_outer_one_is_not(self):
        inner = ["    kv2 := perm(x.v_int)", "    j := 0",
                 "    while (j < n)",
                 Check("      invariant acc(x.v_int, kv2)", INV, "internal"),
                 Check("      decreases n - j", "termination.failed", "termination"),
                 "    {", "      j := j + 1", "    }", "    i := i + 1"]
        locals_ = ["  var kv: Perm", "  var kd: Perm", "  var vs: Int", "  var i: Int", "  var j: Int", "  var kv2: Perm"]
        self.assertEqual(self.strict(self.fixture(locals_=locals_, body=inner)), [])
        inner_bad = inner[:6] + ["      kv := none"] + inner[6:]
        probs = self.strict(self.fixture(locals_=locals_, body=inner_bad))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`Perm` local `kv` is assigned inside the loop", probs[0])
        inner_bad2 = inner[:6] + ["      kv2 := none"] + inner[6:]
        probs = self.strict(self.fixture(locals_=locals_, body=inner_bad2))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`Perm` local `kv2` is assigned inside the loop", probs[0])

    def test_a_trigger_set_in_the_invariant_does_not_hide_the_body(self):
        inv = "    invariant (forall k: Int :: { loc(b, k) } 0 <= k ==> acc(x.v_int, kv))"
        # (the older `while`-without-`decreases` rule reads the body's `{` as the first one on a line: a trigger set
        # in an invariant is a false positive there, so it is filtered out here)
        lines = self.fixture(invariants=[inv], body=["    kv := none", "    i := i + 1"])
        probs = [p for p in without_quantifier_rules(self.strict(lines)) if "without a `decreases`" not in p]
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("is assigned inside the loop", probs[0])

    def test_perm_in_an_invariant_is_a_problem(self):
        for inv in ("    invariant perm(x.v_int) == write", "    invariant kv > none ==> perm(x.d_int) >= kd",
                    "    invariant (perm (x.v_int) > none)"):
            with self.subTest(inv=inv):
                probs = self.strict(self.fixture(invariants=[self.INV_ACC, inv]))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("`perm(...)` in a loop invariant", probs[0])

    def test_perm_elsewhere_is_fine(self):
        body = ["    assert perm(x.v_int) >= kv", "    i := i + 1"]
        self.assertEqual(self.strict(self.fixture(body=[Check(body[0], A, "permission"), body[1]])), [])
        self.assertEqual(self.strict(self.fixture(between=[Check("  assert perm(x.v_int) == write", A, "permission")])), [])

    def test_a_name_that_contains_perm_is_not_the_function(self):
        self.assertEqual(self.strict(self.fixture(invariants=[self.INV_ACC, "    invariant x.perm(1) == 2 || kperm(x) > 0"])), [])

    def test_a_perm_in_a_method_contract_is_not_a_loop_invariant(self):
        lines = self.fixture()
        lines.insert(lines.index("  decreases"), Check("  requires perm(x.v_int) == write", WF, "internal"))
        self.assertEqual(self.strict(lines), [])

    def test_inhale_and_assume_stay_refused_next_to_a_snapshot(self):
        probs = self.strict(self.fixture(snapshot=["  inhale acc(x.v_int, kv)", "  kv := none", "  kd := none"]))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`inhale` without", probs[0])


class GlobalsTests(unittest.TestCase):
    """Decision 0041 item 4: `unique function GB_<ident>(): Block` only inside `domain Block`; the trusted `program_start()` pinned row by
    row (the facts row first, then exactly one row per declared `GB_` function, each with a value of its type); the facts as the first
    `requires` of every method but `main`'s (the one whose first statement is `program_start()`); `program_start()` called once, first.
    The fixture is the emitter's shape (tests/viper-smoke/memory/s31_*.vpr, fully parenthesized as the emitter prints); each twin breaks one
    thing."""

    MM = MemoryModelTests
    NAMES = ["GB_g__1", "GB_h__2", "GB_p__3"]

    @staticmethod
    def facts(names=None):
        names = GlobalsTests.NAMES if names is None else names
        return " && ".join(f"((len({n}()) == 1) && (lvloc({n}()) == loc({n}(), (-2))))" for n in names)

    @staticmethod
    def row(g, suffix, value):
        c = f"loc({g}(), 0)"
        return f"(((acc({c}.v_{suffix}) && acc({c}.d_{suffix})) && {c}.d_{suffix}) && {value}) && acc(lvloc({g}()).lv)"

    def rows(self):
        return [
            self.row("GB_g__1", "int", "(loc(GB_g__1(), 0).v_int == 3)"),
            self.row("GB_h__2", "int", "(((-2147483648) <= loc(GB_h__2(), 0).v_int) && (loc(GB_h__2(), 0).v_int <= 2147483647))"),
            self.row("GB_p__3", "ptr_int", "(loc(GB_p__3(), 0).v_ptr_int == loc(GB_h__2(), 0))"),
        ]

    def fixture(self, names=None, facts=None, rows=None, ps_header="method program_start()", ps_tag=True, ps_extra=(), ps_decreases="  decreases _",
                domain_extra=(), f_requires=None, main_lines=None):
        names = self.NAMES if names is None else names
        lines = [
            "field v_int: Int",
            "field d_int: Bool",
            "field v_ptr_int: Ref",
            "field d_ptr_int: Bool",
            "field lv: Bool",
            "",
            "// cerastes:domain-ok 0028",
            "domain Block {",
            "  function loc(b: Block, i: Int): Ref",
            "  function block_of(r: Ref): Block",
            *[f"  unique function {n}(): Block" for n in names],
            *domain_extra,
            *[x for name, record in MemoryModelTests.AXIOMS for x in ("  // cerastes:axiom-ok " + record, MemoryModelTests.axiom_line(name))],
            "}",
            "",
            "// cerastes:trusted-contract allocation of an address-taken local of type int",
            *MemoryModelTests.alloc_lines(),
        ]
        if ps_tag:
            lines.append("// cerastes:trusted-contract program start: the initial state of every global (decision 0041 item 4)")
        lines.append(ps_header)
        lines.append(Check("  ensures " + (self.facts(names) if facts is None else facts), P, "internal"))
        lines += [Check("  ensures " + r, P, "internal") for r in (self.rows() if rows is None else rows)]
        lines += [*ps_extra, ps_decreases]
        lines += [
            "method f__1(amt_0: Perm)",
            Check("  requires " + (self.facts(names) if f_requires is None else f_requires), WF, "internal"),
            Check("  requires (none < amt_0) && (amt_0 < write)", WF, "internal"),
            "  decreases",
            "{",
            "  var x: Ref",
            Check("  x := alloc_int()", PRE, "internal"),
            "  exhale acc(x.v_int) && acc(x.d_int) && acc(lvloc(block_of(x)).lv)",
            "}",
            "method main__2()",
            "  decreases",
            "{",
            "  var y: Int",
            *(main_lines if main_lines is not None else [Check("  program_start()", PRE, "internal"), "  y := 1"]),
            "}",
            "method main__2__wf()",
            "  decreases",
            "{",
            "}",
        ]
        return lines

    @staticmethod
    def strict(lines):
        return lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")

    def one(self, lines, fragment):
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn(fragment, probs[0])

    def some(self, lines, fragment):
        probs = self.strict(lines)
        self.assertTrue(any(fragment in p for p in probs), probs)

    def test_the_emitters_shape_is_clean(self):
        self.assertEqual(self.strict(self.fixture()), [])

    def test_a_plain_spelling_of_the_same_rows_is_clean(self):
        rows = ["acc(loc(GB_g__1(), 0).v_int) && acc(loc(GB_g__1(), 0).d_int) && loc(GB_g__1(), 0).d_int && loc(GB_g__1(), 0).v_int == 3 && acc(lvloc(GB_g__1()).lv)",
                self.rows()[1], self.rows()[2]]
        self.assertEqual(self.strict(self.fixture(rows=rows)), [])

    def test_known_and_unknown_values_of_every_field_type_are_clean(self):
        names = ["GB_a__1", "GB_b__2", "GB_c__3", "GB_d__4", "GB_e__5"]
        lines = self.fixture(names=names, rows=[
            self.row("GB_a__1", "int", "(loc(GB_a__1(), 0).v_int == (-5))"),
            self.row("GB_b__2", "ptr_int", "(loc(GB_b__2(), 0).v_ptr_int == null)"),
            self.row("GB_c__3", "ptr_int", "((loc(GB_c__3(), 0).v_ptr_int == null) || (((loc(GB_c__3(), 0).v_ptr_int == loc(block_of(loc(GB_c__3(), 0).v_ptr_int), offset_of(loc(GB_c__3(), 0).v_ptr_int))) && (0 <= offset_of(loc(GB_c__3(), 0).v_ptr_int))) && (offset_of(loc(GB_c__3(), 0).v_ptr_int) <= len(block_of(loc(GB_c__3(), 0).v_ptr_int)))))"),
            self.row("GB_d__4", "int", "(loc(GB_d__4(), 0).v_int == 0)"),
            self.row("GB_e__5", "int", "(((-128) <= loc(GB_e__5(), 0).v_int) && (loc(GB_e__5(), 0).v_int <= 127))"),
        ])
        self.assertEqual(self.strict(lines), [])
        # an unknown bool: no value row at all; a known one is `== true`
        bl = self.fixture(names=["GB_a__1", "GB_b__2"], rows=[
            "((acc(loc(GB_a__1(), 0).v_bool) && acc(loc(GB_a__1(), 0).d_bool)) && loc(GB_a__1(), 0).d_bool) && acc(lvloc(GB_a__1()).lv)",
            self.row("GB_b__2", "bool", "(loc(GB_b__2(), 0).v_bool == true)")])
        bl[0:0] = ["field v_bool: Bool", "field d_bool: Bool"]
        self.assertEqual(self.strict(bl), [])

    # --- `unique` ---

    def test_unique_outside_the_domain_is_refused(self):
        lines = self.fixture()
        lines.append("unique function GB_z__9(): Block")
        self.some(lines, "`unique`: admitted only as `unique function GB_<name>(): Block` inside `domain Block`")

    def test_unique_of_another_function_in_the_domain_is_refused(self):
        for text in ("  unique function other(): Block", "  unique function GB_z__9(): Int", "  unique function GB_z__9(x: Int): Block"):
            with self.subTest(text):
                self.one(self.fixture(domain_extra=[text]), "`unique`: admitted only as")

    def test_a_gb_function_that_is_not_unique_is_refused(self):
        self.some(self.fixture(domain_extra=["  function GB_z__9(): Block"]), "must be `unique`")

    def test_a_gb_function_twice_is_refused(self):
        self.some(self.fixture(domain_extra=["  unique function GB_g__1(): Block"]), "declared twice")

    # --- program_start's contract ---

    def test_a_duplicate_row_is_refused(self):
        rows = self.rows()
        self.one(self.fixture(rows=rows + [rows[0]]), "a second `program_start` row for `GB_g__1`")

    def test_a_missing_row_is_refused(self):
        self.one(self.fixture(rows=self.rows()[:2]), "has no row for `GB_p__3`")

    def test_a_row_for_an_undeclared_global_is_refused(self):
        rows = self.rows() + [self.row("GB_x__9", "int", "(loc(GB_x__9(), 0).v_int == 0)")]
        self.one(self.fixture(rows=rows), "a `program_start` row is")

    def test_another_value_is_refused(self):
        c = "loc(GB_g__1(), 0)"
        bad = {
            "not a literal": f"({c}.v_int == loc(GB_h__2(), 0).v_int)",
            "a disjunction": f"(({c}.v_int == 3) || true)",
            "a bound only": f"({c}.v_int <= 3)",
            "an inverted range": f"((7 <= {c}.v_int) && ({c}.v_int <= 3))",
            "true as an Int": f"({c}.v_int == true)",
            "the wrong cell": "(loc(GB_h__2(), 0).v_int == 3)",
        }
        for what, value in bad.items():
            with self.subTest(what):
                rows = self.rows()
                rows[0] = self.row("GB_g__1", "int", value)
                self.some(self.fixture(rows=rows), "the value of the `program_start` row of `GB_g__1`")

    def test_extra_conjuncts_or_a_dropped_part_of_a_row_are_refused(self):
        rows = self.rows()
        for what, row in {"extra fact": rows[0] + " && false", "no flag": rows[0].replace(" && loc(GB_g__1(), 0).d_int)", ")"),
                          "no liveness": rows[0].replace(" && acc(lvloc(GB_g__1()).lv)", ""),
                          "half the permission": rows[0].replace("acc(loc(GB_g__1(), 0).v_int)", "acc(loc(GB_g__1(), 0).v_int, 1/2)")}.items():
            with self.subTest(what):
                self.some(self.fixture(rows=[row, *rows[1:]]), "a `program_start` row is")

    def test_a_pointer_row_targets_a_declared_global_of_the_pointee_type(self):
        rows = self.rows()
        cases = {"undeclared": ("(loc(GB_p__3(), 0).v_ptr_int == loc(GB_x__9(), 0))", "points at `GB_x__9`"),
                 "itself, whose field is a pointer's": ("(loc(GB_p__3(), 0).v_ptr_int == loc(GB_p__3(), 0))", "points at `GB_p__3`")}
        for what, (value, frag) in cases.items():
            with self.subTest(what):
                self.some(self.fixture(rows=[rows[0], rows[1], self.row("GB_p__3", "ptr_int", value)]), frag)

    # --- a struct-pointer global of unknown value: WF, then the guarded WF_S conjunct (decision 0043 item 8, audit round 2 S2) ---

    @staticmethod
    def struct_pointer_row(lens, g="GB_gp__1", suffix="ptr_struct_0_S", guard=None):
        v = f"loc({g}(), 0).v_{suffix}"
        wf = (f"(({v} == null) || ((({v} == loc(block_of({v}), offset_of({v}))) && (0 <= offset_of({v}))) && (offset_of({v}) <= len(block_of({v})))))")
        guard = guard if guard is not None else f"(({v} != null) && (offset_of({v}) < len(block_of({v}))))"
        return GlobalsTests.row(g, suffix, f"({wf} && ({guard} ==> ({lens})))")

    @staticmethod
    def chain(v, *ks):
        x = v
        for k in ks[:-1]:
            x = f"loc(member({x}, {k}), 0)"
        return x, ks[-1]

    def struct_fixture(self, lens, **kw):
        lines = self.fixture(names=["GB_gp__1"], rows=[self.struct_pointer_row(lens, **kw)])
        lines[0:0] = ["field v_ptr_struct_0_S: Ref", "field d_ptr_struct_0_S: Bool"]
        return lines

    V = "loc(GB_gp__1(), 0).v_ptr_struct_0_S"
    LENS_S = f"(len(member({V}, 0)) == 1) && (len(member({V}, 1)) == 1)"
    # struct T { struct S s; int z; }: the member blocks depth first, then the slot fact root != nested slot
    LENS_T = (f"(((len(member({V}, 0)) == 1) && (len(member(loc(member({V}, 0), 0), 0)) == 1)) && (len(member(loc(member({V}, 0), 0), 1)) == 1)) && "
              f"(len(member({V}, 1)) == 1) && ({V} != loc(member({V}, 0), 0))")

    def test_a_struct_pointer_row_with_wf_s_is_clean(self):
        self.assertEqual(self.strict(self.struct_fixture(self.LENS_S)), [])
        self.assertEqual(self.strict(self.struct_fixture(self.LENS_T)), [])

    def test_a_struct_pointer_row_with_another_shape_is_refused(self):
        V = self.V
        bad = {
            "a wrong length": self.LENS_S.replace("(len(member(%s, 1)) == 1)" % V, "(len(member(%s, 1)) == 2)" % V),
            "an extra conjunct": self.LENS_S + " && false",
            "a member twice": self.LENS_S + f" && (len(member({V}, 0)) == 1)",
            "a nested member without its struct": f"(len(member({V}, 0)) == 1) && (len(member(loc(member({V}, 1), 0), 0)) == 1)",
            "a block_of inside the chain": self.LENS_S.replace("member(%s, 1)" % V, "member(loc(block_of(%s), 0), 1)" % V),
        }
        for what, lens in bad.items():
            with self.subTest(what):
                self.some(self.struct_fixture(lens), "the value of the `program_start` row of `GB_gp__1`")
        T = self.LENS_T
        bad_t = {
            "the slot fact missing": T.replace(f" && ({V} != loc(member({V}, 0), 0))", ""),
            "a reversed slot fact": T.replace(f"({V} != loc(member({V}, 0), 0))", f"(loc(member({V}, 0), 0) != {V})"),
            "a slot fact twice": T + f" && ({V} != loc(member({V}, 0), 0))",
            "a slot fact between leaves": T.replace(f"({V} != loc(member({V}, 0), 0))", f"({V} != loc(member(loc(member({V}, 0), 0), 0), 0))"),
        }
        for what, lens in bad_t.items():
            with self.subTest(what):
                self.some(self.struct_fixture(lens), "the value of the `program_start` row of `GB_gp__1`")

    def test_a_struct_pointer_row_needs_the_guard(self):
        V = self.V
        for what, guard in {"no non-null test": f"(offset_of({V}) < len(block_of({V})))",
                            "a weaker bound": f"(({V} != null) && (offset_of({V}) <= len(block_of({V}))))",
                            "another pointer": f"(({V} != null) && (offset_of(null) < len(block_of(null))))"}.items():
            with self.subTest(what):
                self.some(self.struct_fixture(self.LENS_S, guard=guard), "the value of the `program_start` row of `GB_gp__1`")

    def test_the_facts_row_is_pinned(self):
        bad = {
            "len 2": self.facts().replace("(len(GB_g__1()) == 1)", "(len(GB_g__1()) == 2)"),
            "the liveness cell elsewhere": self.facts().replace("loc(GB_h__2(), (-2))", "loc(GB_h__2(), (-1))"),
            "a global missing": self.facts(self.NAMES[:2]),
            "a global twice": self.facts(self.NAMES + self.NAMES[:1]),
            "a disjunct": self.facts() + " || true",
        }
        for what, facts in bad.items():
            with self.subTest(what):
                self.one(self.fixture(facts=facts), "its first `ensures` row must be the global facts")

    def test_the_facts_may_be_in_another_order(self):
        self.assertEqual(self.strict(self.fixture(facts=self.facts(list(reversed(self.NAMES))))), [])

    def test_the_header_the_tag_and_the_clauses_of_program_start_are_pinned(self):
        self.some(self.fixture(ps_tag=False), "bodyless `method program_start` without `// cerastes:trusted-contract")
        self.some(self.fixture(ps_header="method program_start() returns (r: Int)"), "it must be a bodyless `method program_start()`")
        self.some(self.fixture(ps_extra=[Check("  requires true", WF, "internal")]), "`ensures` rows and a final `decreases _`")
        self.some(self.fixture(ps_decreases="  decreases"), "`ensures` rows and a final `decreases _`")
        lines = self.fixture()
        i = lines.index("  decreases _", lines.index("method program_start()"))
        lines[i + 1:i + 1] = ["{", "}"]
        self.some(lines, "it must be a bodyless `method program_start()`")

    def test_program_start_without_globals_is_refused(self):
        lines = self.fixture(names=[], facts="true", rows=[])
        self.some(lines, "`method program_start` in a program with no `unique function GB_...`")

    def test_two_program_starts_are_refused(self):
        lines = self.fixture()
        lines += ["// cerastes:trusted-contract another", "method program_start()", "  decreases _"]
        self.some(lines, "exactly one `method program_start()`")

    # --- the facts of every other method; the one call of program_start ---

    def test_a_method_without_the_facts_is_refused(self):
        lines = self.fixture()
        i = find(lines, "requires " + self.facts())
        lines.pop(i)
        self.one(lines, "`method f__1` of a program with globals must take the global facts as its first `requires`")

    def test_the_trusted_zero_fill_of_an_array_needs_no_facts(self):
        # M4.5e1e1 R4: `zero_arr_<t>` is a trusted method like `alloc_*` and carries no facts line (decision 0059 item 3), so a program with a
        # global and a zero-filled local array (`int a[4] = {1};`) is clean; the exemption is the emitter's name prefix `zero_arr_` and no wider
        A = ArrayAllocationTests("test_the_two_contracts_are_clean_for_each_field_type")
        lines = self.fixture() + A.method("zero_arr_int", A.zero_clauses(), fields=False)
        self.assertEqual(self.strict(lines), [])
        # twins: the same clauses under a name that is not the prefix's (`zero_arr`, `zero_array_int`) are methods like any other
        for name in ("zero_arr", "zero_array_int", "my_zero_arr_int"):
            with self.subTest(name=name):
                lines = self.fixture() + A.method(name, A.zero_clauses(), fields=False)
                self.some(lines, f"`method {name}` of a program with globals must take the global facts as its first `requires`")

    def test_a_method_with_the_facts_second_or_changed_is_refused(self):
        lines = self.fixture()
        i = find(lines, "requires " + self.facts())
        lines[i], lines[i + 1] = lines[i + 1], lines[i]
        self.one(lines, "must take the global facts as its first `requires`")
        self.one(self.fixture(f_requires=self.facts(self.NAMES[:2])), "must take the global facts as its first `requires`")
        self.one(self.fixture(f_requires=self.facts() + " && true"), "must take the global facts as its first `requires`")

    def test_a_method_with_no_requires_at_all_is_refused(self):
        lines = self.fixture()
        i = find(lines, "requires " + self.facts())
        del lines[i:i + 2]
        self.some(lines, "`method f__1` of a program with globals must take the global facts")

    def test_the_method_that_starts_the_program_needs_no_facts_but_must_start_first(self):
        # `var` declarations may precede the call, a statement may not
        self.assertEqual(self.strict(self.fixture(main_lines=["  var z: Int", Check("  program_start()", PRE, "internal")])), [])
        self.some(self.fixture(main_lines=["  y := 1", Check("  program_start()", PRE, "internal")]),
                  "`program_start()` is called only as the first statement of `main`'s method")

    def test_program_start_called_twice_is_refused(self):
        lines = self.fixture(main_lines=[Check("  program_start()", PRE, "internal"), Check("  program_start()", PRE, "internal")])
        self.some(lines, "`program_start()` is called once")

    def test_a_call_of_the_entry_method_is_refused(self):
        # a call right after program_start (the audit's mutant: `res := f_main(...)`), from the entry itself, and from another method
        lines = self.fixture(main_lines=[Check("  program_start()", PRE, "internal"), Check("  main__2()", PRE, "internal")])
        self.some(lines, "a call of the entry method `main__2`")
        lines = self.fixture()
        i = find(lines, "x := alloc_int()")
        lines.insert(i + 1, Check("  main__2()", PRE, "internal"))
        self.some(lines, "a call of the entry method `main__2`")
        # a method whose name only starts like the entry's is not a call of it
        self.assertEqual(self.strict(self.fixture(main_lines=[Check("  program_start()", PRE, "internal"), "  var main__2__x: Int"])), [])

    def test_the_entry_method_takes_no_requires(self):
        lines = self.fixture()
        i = lines.index("method main__2()")
        lines.insert(i + 1, Check("  requires true", WF, "internal"))
        self.some(lines, "starts the program (its first statement is `program_start()`): it takes no `requires`")

    def test_program_start_called_by_a_method_that_takes_the_facts_is_refused(self):
        lines = self.fixture()
        i = find(lines, "x := alloc_int()")
        lines.insert(i + 1, Check("  program_start()", PRE, "internal"))
        self.some(lines, "`program_start()` is called once")
        self.some(lines, "`program_start()` is called only as the first statement")

    def test_a_program_without_globals_is_not_linted_for_facts(self):
        self.assertEqual(self.strict(MemoryModelTests().fixture()), [])

    # --- the flag of a global's cell ---

    def test_the_flag_of_a_globals_cell_is_set_by_its_write_only(self):
        g = "loc(GB_g__1(), 0)"
        ok = [f"  {g}.v_int := 1", f"  {g}.d_int := true"]
        lines = self.fixture()
        lines[lines.index("  y := 1"):lines.index("  y := 1")] = ok
        self.assertEqual(self.strict(lines), [])
        for what, bad, frag in (("no write before", ["  y := 2", f"  {g}.d_int := true"], "does not directly follow a write"),
                                ("false", [f"  {g}.v_int := 1", f"  {g}.d_int := false"], "is assigned `false`")):
            with self.subTest(what):
                lines = self.fixture()
                lines[lines.index("  y := 1"):lines.index("  y := 1")] = bad
                self.some(lines, frag)


class StructGlobalTests(unittest.TestCase):
    """Decision 0044 items 8 and 13: the `program_start` rows of struct globals and the facts that go with them are pinned like
    `alloc_struct_<n>_<tag>` (decision 0043 item 13): a struct global's row is exactly `acc(L.v_t) && acc(L.d_t) && L.d_t && <value>` for each
    leaf L (a canonical chain `loc(member(<chain>, k), 0)` rooted at `loc(GB_g(), 0)`, each leaf once, none a prefix of another), then exactly
    one `acc(lvloc(GB_g()).lv)`; the facts (the first `ensures` of `program_start` and the first `requires` of every other method) are each
    global's scalar facts, a struct global's followed by the member lengths of every slot of its row and its slot pairs, nothing else.  The
    fixture is the emitter's shape (build/m4.5c2a/sg/s1.vpr, s2.c23.vpr: `static int h = 7`, `struct P {int a; int *p;} g = {1, &h}` and a nested
    `struct N {int a; struct S s;} n = {1, {2, 3}}`); each twin breaks one thing."""

    G = GlobalsTests("test_the_emitters_shape_is_clean")
    NAMES = ["GB_h__1", "GB_g__2", "GB_n__3"]
    EXTRA = ["field v_ptr_struct_0_P: Ref", "field d_ptr_struct_0_P: Bool"]

    @staticmethod
    def slot(g, *chain):
        out = f"loc({g}(), 0)"
        for k in chain:
            out = f"loc(member({out}, {k}), 0)"
        return out

    @classmethod
    def cell(cls, g, *chain):
        return f"loc(member({cls.slot(g, *chain[:-1])}, {chain[-1]}), 0)"

    @classmethod
    def leaf(cls, g, chain, suffix, value, cell=None):
        c = cell if cell is not None else cls.cell(g, *chain)
        return f"(((acc({c}.v_{suffix}) && acc({c}.d_{suffix})) && {c}.d_{suffix}) && {value})"

    @classmethod
    def srow(cls, g, leaves, tail=None):
        return " && ".join(leaves) + " && " + (tail if tail is not None else f"acc(lvloc({g}()).lv)")

    @classmethod
    def int_leaf(cls, g, chain, v):
        c = cls.cell(g, *chain)
        return cls.leaf(g, chain, "int", f"({c}.v_int == {v})")

    @classmethod
    def scalar_facts(cls, names):
        return [f"((len({n}()) == 1) && (lvloc({n}()) == loc({n}(), (-2))))" for n in names]

    @classmethod
    def g_facts(cls):  # g: leaves (0,) and (1,)
        r = cls.slot("GB_g__2")
        return [f"(len(member({r}, 0)) == 1)", f"(len(member({r}, 1)) == 1)"]

    @classmethod
    def n_facts(cls):  # n: leaves (0,), (1, 0), (1, 1)
        r, s1 = cls.slot("GB_n__3"), cls.slot("GB_n__3", 1)
        return [f"(len(member({r}, 0)) == 1)", f"(len(member({r}, 1)) == 1)", f"(len(member({s1}, 0)) == 1)", f"(len(member({s1}, 1)) == 1)",
                f"({r} != {s1})"]

    @classmethod
    def facts(cls, g_extra=(), n_extra=(), drop=None):
        parts = cls.scalar_facts(["GB_h__1"]) + cls.scalar_facts(["GB_g__2"]) + cls.g_facts() + list(g_extra) \
            + cls.scalar_facts(["GB_n__3"]) + cls.n_facts() + list(n_extra)
        if drop is not None:
            parts = [x for x in parts if drop not in x]
        return " && ".join(parts)

    @classmethod
    def rows(cls):
        h = cls.slot("GB_h__1")
        g_p = cls.cell("GB_g__2", 1)
        return [
            cls.row_h(),
            cls.srow("GB_g__2", [cls.int_leaf("GB_g__2", (0,), 1), cls.leaf("GB_g__2", (1,), "ptr_int", f"({g_p}.v_ptr_int == {h})")]),
            cls.srow("GB_n__3", [cls.int_leaf("GB_n__3", (0,), 1), cls.int_leaf("GB_n__3", (1, 0), 2), cls.int_leaf("GB_n__3", (1, 1), 3)]),
        ]

    @classmethod
    def row_h(cls):
        return GlobalsTests.row("GB_h__1", "int", "(loc(GB_h__1(), 0).v_int == 7)")

    def fixture(self, rows=None, facts=None, f_requires=None, names=None):
        lines = self.G.fixture(names=self.NAMES if names is None else names, facts=self.facts() if facts is None else facts,
                               rows=self.rows() if rows is None else rows, f_requires=self.facts() if f_requires is None else f_requires)
        lines[0:0] = self.EXTRA
        return lines

    @staticmethod
    def strict(lines):
        return lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")

    def one(self, lines, fragment):
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn(fragment, probs[0])

    def some(self, lines, fragment):
        probs = self.strict(lines)
        self.assertTrue(any(fragment in p for p in probs), probs)

    def with_row(self, index, row):
        rows = self.rows()
        rows[index] = row
        return self.fixture(rows=rows)

    def test_the_emitters_shape_is_clean(self):
        self.assertEqual(self.strict(self.fixture()), [])

    def test_a_plain_spelling_is_clean(self):
        # the same rows and facts without the emitter's grouping parentheses
        g = "GB_g__2"
        c0, c1 = self.cell(g, 0), self.cell(g, 1)
        row = (f"acc({c0}.v_int) && acc({c0}.d_int) && {c0}.d_int && {c0}.v_int == 1 && acc({c1}.v_ptr_int) && acc({c1}.d_ptr_int) && "
               f"{c1}.d_ptr_int && {c1}.v_ptr_int == loc(GB_h__1(), 0) && acc(lvloc({g}()).lv)")
        self.assertEqual(self.strict(self.with_row(1, row)), [])

    def test_a_struct_global_with_an_unknown_value_row_is_clean(self):
        g = "GB_g__2"
        c0 = self.cell(g, 0)
        rng = f"(((-2147483648) <= {c0}.v_int) && ({c0}.v_int <= 2147483647))"
        self.assertEqual(self.strict(self.with_row(1, self.srow(g, [self.leaf(g, (0,), "int", rng), self.leaf(g, (1,), "ptr_int", f"({self.cell(g, 1)}.v_ptr_int == null)")]))), [])

    # --- the row of a struct global ---

    def test_a_duplicated_leaf_is_refused(self):
        g = "GB_g__2"
        leaves = [self.int_leaf(g, (0,), 1), self.int_leaf(g, (0,), 1), self.leaf(g, (1,), "ptr_int", f"({self.cell(g, 1)}.v_ptr_int == null)")]
        self.one(self.with_row(1, self.srow(g, leaves)), "a leaf twice")

    def test_a_duplicated_leaf_spelled_through_block_of_is_refused(self):
        g = "GB_g__2"
        c0 = self.cell(g, 0)
        alias = f"loc(block_of({c0}), offset_of({c0}))"  # the same cell, not in the canonical grammar
        leaves = [self.int_leaf(g, (0,), 1), self.leaf(g, (0,), "int", f"({alias}.v_int == 1)", cell=alias),
                  self.leaf(g, (1,), "ptr_int", f"({self.cell(g, 1)}.v_ptr_int == null)")]
        self.some(self.with_row(1, self.srow(g, leaves)), "struct row is wrong")

    def test_a_leaf_without_its_flag_permission_is_refused(self):
        g = "GB_g__2"
        c0 = self.cell(g, 0)
        bad = f"((acc({c0}.v_int) && {c0}.d_int) && ({c0}.v_int == 1))"
        self.some(self.with_row(1, self.srow(g, [bad, self.leaf(g, (1,), "ptr_int", f"({self.cell(g, 1)}.v_ptr_int == null)")])), "a leaf is not exactly")

    def test_a_leaf_with_the_flag_not_asserted_is_refused(self):
        g = "GB_g__2"
        c0 = self.cell(g, 0)
        bad = f"(((acc({c0}.v_int) && acc({c0}.d_int)) && !{c0}.d_int) && ({c0}.v_int == 1))"
        self.some(self.with_row(1, self.srow(g, [bad, self.leaf(g, (1,), "ptr_int", f"({self.cell(g, 1)}.v_ptr_int == null)")])), "a leaf is not exactly")

    def test_an_extra_conjunct_is_refused(self):
        g = "GB_g__2"
        row = self.rows()[1]
        for extra in ("(loc(GB_g__2(), 0) != null)", "(loc(GB_g__2(), 0) == loc(GB_g__2(), 0))", "false"):
            with self.subTest(extra):
                bad = row.replace(f"acc(lvloc({g}()).lv)", f"{extra} && acc(lvloc({g}()).lv)")
                self.some(self.with_row(1, bad), "program_start")

    def test_an_extra_conjunct_after_the_liveness_cell_is_refused(self):
        g = "GB_g__2"
        self.some(self.with_row(1, self.rows()[1] + " && true"), "does not end with exactly one")

    def test_an_unrooted_chain_is_refused(self):
        g = "GB_g__2"
        other = self.cell("GB_h__1", 0)  # a chain rooted at another global
        leaves = [self.leaf(g, (0,), "int", f"({other}.v_int == 1)", cell=other), self.leaf(g, (1,), "ptr_int", f"({self.cell(g, 1)}.v_ptr_int == null)")]
        self.some(self.with_row(1, self.srow(g, leaves)), "struct row is wrong")

    def test_a_chain_with_a_non_literal_member_index_is_refused(self):
        g = "GB_g__2"
        c = f"loc(member({self.slot(g)}, 0 + 0), 0)"
        leaves = [self.leaf(g, (0,), "int", f"({c}.v_int == 1)", cell=c), self.leaf(g, (1,), "ptr_int", f"({self.cell(g, 1)}.v_ptr_int == null)")]
        self.some(self.with_row(1, self.srow(g, leaves)), "struct row is wrong")

    def test_a_second_liveness_cell_is_refused(self):
        g = "GB_g__2"
        self.some(self.with_row(1, self.rows()[1] + f" && acc(lvloc({g}()).lv)"), "a leaf is not exactly")

    def test_a_missing_liveness_cell_is_refused(self):
        g = "GB_g__2"
        self.some(self.with_row(1, self.srow(g, self.rows()[1].split(" && acc(lvloc")[:1], tail="true")), "does not end with exactly one")

    def test_a_leaf_that_is_a_prefix_of_another_is_refused(self):
        g = "GB_n__3"
        leaves = [self.int_leaf(g, (0,), 1), self.int_leaf(g, (1,), 2), self.int_leaf(g, (1, 0), 2), self.int_leaf(g, (1, 1), 3)]
        self.some(self.with_row(2, self.srow(g, leaves)), "is a prefix of the leaf")

    def test_the_value_must_be_of_the_leafs_type(self):
        g = "GB_g__2"
        c0 = self.cell(g, 0)
        for value in (f"({c0}.v_int == null)", f"({c0}.v_int == {self.slot('GB_h__1')})", f"({c0}.v_int > 1)", "true"):
            with self.subTest(value):
                leaves = [self.leaf(g, (0,), "int", value), self.leaf(g, (1,), "ptr_int", f"({self.cell(g, 1)}.v_ptr_int == null)")]
                self.some(self.with_row(1, self.srow(g, leaves)), "is not a literal of its type")

    def test_two_rows_for_one_global_are_refused(self):
        rows = self.rows()
        self.some(self.fixture(rows=rows + [rows[1]]), "a second `program_start` row for `GB_g__2`")

    def test_a_scalar_row_and_a_struct_row_for_one_global_are_refused(self):
        rows = self.rows()
        self.some(self.fixture(rows=rows + [GlobalsTests.row("GB_g__2", "int", "(loc(GB_g__2(), 0).v_int == 1)")]), "a second `program_start` row for `GB_g__2`")

    def test_a_missing_row_is_refused(self):
        rows = self.rows()
        self.some(self.fixture(rows=rows[:2]), "has no row for `GB_n__3`")

    def test_a_row_of_an_undeclared_global_is_refused(self):
        g = "GB_z__9"
        self.some(self.fixture(rows=self.rows() + [self.srow(g, [self.int_leaf(g, (0,), 1)])]), "not a declared `GB_` global")

    # --- addresses ---

    def test_an_address_leaf_of_the_wrong_type_is_refused(self):
        g = "GB_g__2"
        n = self.slot("GB_n__3")
        cp = self.cell(g, 1)
        # an `int *` leaf pointing at a struct global
        bad = self.srow(g, [self.int_leaf(g, (0,), 1), self.leaf(g, (1,), "ptr_int", f"({cp}.v_ptr_int == {n})")])
        self.some(self.with_row(1, bad), "that is not a pointer to a struct")
        # a pointer-to-struct leaf pointing at a scalar global
        row = self.srow(g, [self.int_leaf(g, (0,), 1), self.leaf(g, (1,), "ptr_struct_0_P", f"({cp}.v_ptr_struct_0_P == {self.slot('GB_h__1')})")])
        self.some(self.with_row(1, row), "which is not a declared global of the pointee's field")

    def test_an_address_leaf_of_the_right_type_is_clean(self):
        g = "GB_g__2"
        cp = self.cell(g, 1)
        row = self.srow(g, [self.int_leaf(g, (0,), 1), self.leaf(g, (1,), "ptr_struct_0_P", f"({cp}.v_ptr_struct_0_P == {self.slot('GB_n__3')})")])
        self.assertEqual(self.strict(self.with_row(1, row)), [])

    def test_a_struct_pointer_field_may_not_point_at_two_shapes(self):
        # `ptr_struct_0_P` pointing at the struct global g (two leaves) and at n (three): not one type
        g, n = "GB_g__2", "GB_n__3"
        row_g = self.srow(g, [self.int_leaf(g, (0,), 1), self.leaf(g, (1,), "ptr_struct_0_P", f"({self.cell(g, 1)}.v_ptr_struct_0_P == {self.slot(n)})")])
        row_n = self.srow(n, [self.int_leaf(n, (0,), 1), self.leaf(n, (1, 0), "ptr_struct_0_P", f"({self.cell(n, 1, 0)}.v_ptr_struct_0_P == {self.slot(g)})"),
                              self.int_leaf(n, (1, 1), 3)])
        rows = self.rows()
        rows[1], rows[2] = row_g, row_n
        self.some(self.fixture(rows=rows), "two different shapes")

    # --- the facts ---

    def test_the_facts_row_is_pinned(self):
        for what, facts in (
                ("a missing member length", self.facts(drop=f"len(member({self.slot('GB_g__2')}, 1))")),
                ("an extra member length", self.facts(g_extra=[f"(len(member({self.slot('GB_g__2')}, 2)) == 1)"])),
                ("a length of a leaf's own block twice", self.facts(g_extra=[self.g_facts()[0]])),
                ("a missing slot fact", self.facts(drop=" != ")),
                ("a reversed slot fact", self.facts(drop=" != ").replace(" && " + "", " && ", 1) + f" && ({self.slot('GB_n__3', 1)} != {self.slot('GB_n__3')})"),
                ("a slot fact of two leaves", self.facts(n_extra=[f"({self.cell('GB_n__3', 1, 0)} != {self.cell('GB_n__3', 1, 1)})"])),
                ("a parent_of fact", self.facts(n_extra=["(parent_of(GB_h__1()) == null)"])),
                ("a missing scalar fact", self.facts(drop="lvloc(GB_h__1())")),
                ("a length of the root block", self.facts(g_extra=["(len(GB_g__2()) == 1)"]))):
            with self.subTest(what):
                lines = self.fixture(facts=facts, f_requires=facts)
                probs = self.strict(lines)
                self.assertTrue(probs and all("global facts" in p or "first `requires`" in p for p in probs), (what, probs))

    def test_a_methods_first_requires_missing_a_length_is_refused(self):
        lines = self.fixture(f_requires=self.facts(drop=f"len(member({self.slot('GB_n__3', 1)}, 1))"))
        self.one(lines, "must take the global facts as its first `requires`")

    def test_a_methods_first_requires_with_a_scalar_only_facts_row_is_refused(self):
        lines = self.fixture(f_requires=" && ".join(self.scalar_facts(self.NAMES)))
        self.one(lines, "must take the global facts as its first `requires`")

    def test_the_facts_follow_the_rows_not_the_declarations(self):
        # lengths for a struct global whose row has other leaves are the wrong lengths
        g = "GB_g__2"
        row = self.srow(g, [self.int_leaf(g, (0,), 1), self.int_leaf(g, (1,), 2), self.int_leaf(g, (2,), 3)])
        lines = self.with_row(1, row)
        probs = self.strict(lines)
        self.assertTrue(any("first `ensures` row must be the global facts" in p for p in probs), probs)
        self.assertTrue(any("must take the global facts" in p for p in probs), probs)


class GhostRefTests(unittest.TestCase):
    """Decision 0045 item 14: the ghost `Ref` locals of places reached through a heap read.  `eb_<N>` is assigned once, outside
    every loop, from `null`, a read of a pointer cell or a conditional of those; `lb_<N>__<L>` is assigned `null` and then at
    most one plain read, only before its loop; `aw_q<j>__<L>` is assigned once, from a read or a conditional of reads, in the
    leading statements of a loop body.  Each rule has a twin that breaks exactly it."""

    EB = "  eb_1 := x.v_ptr_int"
    LB = ["  lb_2__0 := null", "  if (perm(x.v_ptr_int) > none) { lb_2__0 := x.v_ptr_int }"]
    AW = "    aw_q0__0 := (perm(x.v_ptr_int) > none) ? x.v_ptr_int : null"

    @staticmethod
    def strict(lines):
        return lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")

    def fixture(self, entry=None, before=None, body=None, locals_=None):
        return [
            "field v_int: Int",
            "field v_ptr_int: Ref",
            "method f__1(x: Ref, n: Int)",
            Check("  requires n >= 0", WF, "internal"),
            "  decreases",
            "{",
            *(locals_ if locals_ is not None else ["  var eb_1: Ref", "  var lb_2__0: Ref", "  var aw_q0__0: Ref", "  var i: Int"]),
            *(entry if entry is not None else [self.EB]),
            "  i := 0",
            *(before if before is not None else self.LB),
            "  while (i < n)",
            Check("    invariant n >= 0", INV, "internal"),
            Check("    decreases n - i", "termination.failed", "termination"),
            "  {",
            *(body if body is not None else [self.AW, "    i := i + 1"]),
            "  }",
            "}",
        ]

    def test_the_emitters_shape_is_clean(self):
        self.assertEqual(self.strict(self.fixture()), [])

    def test_the_allowed_entry_values(self):
        for rhs in ("x.v_ptr_int", "null", "(perm(x.v_ptr_int) > none) ? x.v_ptr_int : null",
                    "(x != null) ? ((x.v_ptr_int != null) ? x.v_ptr_int.v_ptr_int : null) : null",
                    "((x.v_int == 0) ? x : null).v_ptr_int"):
            with self.subTest(rhs=rhs):
                self.assertEqual(self.strict(self.fixture(entry=[f"  eb_1 := {rhs}"])), [])

    def test_an_entry_snapshot_of_anything_else_is_a_problem(self):
        for rhs in ("x", "x.v_int", "(i > 0) ? x : null", "x.v_ptr_int == null ? x : null"):
            with self.subTest(rhs=rhs):
                probs = self.strict(self.fixture(entry=[f"  eb_1 := {rhs}"]))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("ghost `Ref` local `eb_1` is assigned", probs[0])
                self.assertIn("decision 0045 item 14", probs[0])

    def test_an_entry_snapshot_assigned_twice_is_a_problem(self):
        probs = self.strict(self.fixture(entry=[self.EB, "  eb_1 := null"]))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`eb_1` is assigned 2 times", probs[0])

    def test_an_entry_snapshot_never_assigned_is_a_problem(self):
        probs = self.strict(self.fixture(entry=[]))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`eb_1` is assigned 0 times", probs[0])

    def test_an_entry_snapshot_assigned_in_a_loop_is_a_problem(self):
        probs = self.strict(self.fixture(body=[self.AW, "    eb_1 := x.v_ptr_int", "    i := i + 1"]))
        self.assertTrue(any("`eb_1` is assigned inside a loop" in p for p in probs), probs)

    def test_a_loop_entry_snapshot_is_null_then_a_plain_read(self):
        self.assertEqual(self.strict(self.fixture(before=["  lb_2__0 := null"])), [])
        probs = self.strict(self.fixture(before=["  lb_2__0 := x.v_ptr_int"]))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`null` first", probs[0])

    def test_a_loop_entry_snapshot_never_takes_a_conditional(self):
        probs = self.strict(self.fixture(before=["  lb_2__0 := null",
                                                 "  lb_2__0 := (perm(x.v_ptr_int) > none) ? x.v_ptr_int : null"]))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`lb_2__0` is assigned", probs[0])

    def test_a_loop_entry_snapshot_assigned_three_times_or_in_the_loop_is_a_problem(self):
        probs = self.strict(self.fixture(before=[*self.LB, "  lb_2__0 := x.v_ptr_int"]))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`lb_2__0` is assigned 3 times", probs[0])
        probs = self.strict(self.fixture(body=[self.AW, "    lb_2__0 := null", "    i := i + 1"]))
        self.assertTrue(any("`lb_2__0` is assigned inside a loop" in p for p in probs), probs)

    def test_a_written_atoms_object_is_the_first_statement_of_a_loop_body(self):
        two = ["  var eb_1: Ref", "  var lb_2__0: Ref", "  var aw_q0__0: Ref", "  var aw_q1__0: Ref", "  var i: Int"]
        body = [self.AW, "    aw_q1__0 := x.v_ptr_int", "    i := i + 1"]
        self.assertEqual(self.strict(self.fixture(locals_=two, body=body)), [])

    def test_a_written_atoms_object_after_another_statement_is_a_problem(self):
        probs = self.strict(self.fixture(body=["    i := i + 1", self.AW]))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`aw_q0__0` is assigned after other statements of the loop body", probs[0])

    def test_a_written_atoms_object_outside_a_loop_or_twice_is_a_problem(self):
        probs = self.strict(self.fixture(before=[*self.LB, "  aw_q0__0 := x.v_ptr_int"], body=["    i := i + 1"]))
        self.assertTrue(any("`aw_q0__0` is assigned outside a loop body" in p for p in probs), probs)
        probs = self.strict(self.fixture(body=[self.AW, self.AW, "    i := i + 1"]))
        self.assertTrue(any("`aw_q0__0` is assigned 2 times" in p for p in probs), probs)

    def test_a_written_atoms_object_of_another_shape_is_a_problem(self):
        probs = self.strict(self.fixture(body=["    aw_q0__0 := x", "    i := i + 1"]))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`aw_q0__0` is assigned `x`", probs[0])

    def test_a_struct_atoms_leaf_cell_is_a_written_atoms_object(self):
        leaf = "    aw_q0__0 := loc(member((perm(x.v_ptr_int) > none) ? x.v_ptr_int : null, 1), 0)"
        self.assertEqual(self.strict(self.fixture(body=[leaf, "    i := i + 1"])), [])
        bad = "    aw_q0__0 := loc(member(x, 1), 0)"
        probs = self.strict(self.fixture(body=[bad, "    i := i + 1"]))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`aw_q0__0` is assigned", probs[0])

    def test_other_ref_locals_are_not_ghosts(self):
        locals_ = ["  var eb_1: Ref", "  var lb_2__0: Ref", "  var aw_q0__0: Ref", "  var t__0: Ref", "  var i: Int"]
        body = [self.AW, "    t__0 := x", "    i := i + 1"]
        self.assertEqual(self.strict(self.fixture(locals_=locals_, body=body)), [])


class HeapTokenTests(unittest.TestCase):
    """Decision 0047 items 6, 7 and 10 (audit S7): the free token of a heap block.  `field tok: Bool` is declared exactly in a
    program that uses it; `tok` appears only as `loc(block_of(...), -1).tok` and only in the conjunction of a view or a block
    site, an explicit row, a loop place's invariant or the `perm(...)` of its snapshot; nothing in a body exhales, assigns or
    tests it; `forperm` is only `assert forperm t: Ref [t.tok] :: <E>` on a `leak` row.  Each rule has a twin that breaks
    exactly it."""

    SLOT = "loc(block_of(p), (-1)).tok"
    BLOCK = f"p != null ==> acc({SLOT}) && {SLOT}"
    ROW = f"  assert p == null || (perm({SLOT}) == write && {SLOT})"
    LEAK = "  assert forperm t: Ref [t.tok] :: false"

    @staticmethod
    def strict(lines):
        return lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")

    def fixture(self, field="field tok: Bool", body=None, leak_category="leak", extra_top=None, requires=None):
        return [
            "field v_int: Int",
            *([field] if field else []),
            *(extra_top or []),
            "// cerastes:trusted-contract a function defined elsewhere that takes the block (a block site in its requires)",
            "method f_free__1(p: Ref)",
            Check(f"  requires {requires or self.BLOCK}", WF, "internal"),
            "  decreases _",
            "method f__2(p: Ref)",
            Check(f"  requires {requires or self.BLOCK}", WF, "internal"),
            "  decreases",
            "{",
            "  var kt: Perm",
            f"  kt := perm({self.SLOT})",
            *(body if body is not None else [
                Check(self.ROW, A, "invalid-free"),
                Check("  f_free__1(p)", PRE, "invalid-free"),
                Check(self.LEAK, A, leak_category),
            ]),
            "}",
        ]

    def test_the_emitters_shape_is_clean(self):
        self.assertEqual(self.strict(self.fixture()), [])

    def test_a_minus_one_slot_without_parentheses_is_clean_too(self):
        lines = self.fixture(requires="p != null ==> acc(loc(block_of(p), -1).tok) && loc(block_of(p), -1).tok")
        self.assertEqual(self.strict(lines), [])

    def test_a_loop_place_snapshot_invariant_and_back_edge_row_are_clean(self):
        body = [
            "  var i: Int := 0",
            "  while (i < 3)",
            Check(f"    invariant p != null ==> acc({self.SLOT}, kt)", INV, "internal"),
            Check(f"    invariant kt > none ==> {self.SLOT}", INV, "internal"),
            Check("    decreases 3 - i", "termination.failed", "termination"),
            "  {",
            "    i := i + 1",
            Check(f"    assert perm({self.SLOT}) >= kt", A, "permission"),
            "  }",
            Check(self.LEAK, A, "leak"),
        ]
        self.assertEqual(self.strict(self.fixture(body=body)), [])

    def test_an_identifier_that_only_starts_with_tok_is_not_the_token(self):
        body = ["  var token: Int := 0", "  var toks: Int := 1", "  token := toks"]
        self.assertEqual(self.strict(self.fixture(body=body)), [])

    def test_field_tok_in_a_program_that_never_uses_it_is_a_problem(self):
        lines = self.fixture(body=[], requires="p != null")
        lines[lines.index(f"  kt := perm({self.SLOT})")] = "  kt := none"
        probs = self.strict(lines)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`field tok: Bool` in a program that never uses a token", probs[0])

    def test_field_tok_of_another_type_is_a_problem(self):
        probs = self.strict(self.fixture(field="field tok: Int"))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`field tok` must be exactly `field tok: Bool`", probs[0])

    def test_tok_without_its_field_is_a_problem(self):
        probs = self.strict(self.fixture(field=None))
        self.assertTrue(any("`tok` is used but `field tok: Bool` is not declared" in p for p in probs), probs)

    def test_tok_at_another_slot_or_receiver_is_a_problem(self):
        for slot in ("loc(block_of(p), 0).tok", "p.tok", "loc(block_of(p), (-2)).tok", "lvloc(block_of(p)).tok",
                     "loc(block_of(p), 1).tok"):
            with self.subTest(slot=slot):
                probs = self.strict(self.fixture(body=[Check(f"  assert {slot}", A, "invalid-free"),
                                                       Check(self.LEAK, A, "leak")]))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("`tok` outside the token slot of a block", probs[0])

    def test_exhale_of_a_token_is_a_problem(self):
        for stmt in (f"  exhale acc({self.SLOT})", f"  exhale {self.SLOT}"):
            with self.subTest(stmt=stmt):
                probs = self.strict(self.fixture(body=[stmt, Check(self.LEAK, A, "leak")]))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("`exhale` of a `tok` place", probs[0])

    def test_an_assignment_to_a_token_is_a_problem(self):
        probs = self.strict(self.fixture(body=[f"  {self.SLOT} := false", Check(self.LEAK, A, "leak")]))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("assignment to `tok`", probs[0])

    def test_an_inhale_of_a_token_is_a_problem_even_with_its_tag(self):
        stmt = f"  inhale acc({self.SLOT}) // cerastes:inhale-ok a token made from nothing"
        probs = self.strict(self.fixture(body=[stmt, Check(self.LEAK, A, "leak")]))
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("`tok` in `inhale`", probs[0])

    def test_a_token_in_a_condition_or_a_variable_or_a_fold_is_a_problem(self):
        for stmt in (f"  if ({self.SLOT}) {{ }}", f"  while ({self.SLOT}) {{ }}", "  var tok: Bool", f"  unfold q({self.SLOT})"):
            with self.subTest(stmt=stmt):
                probs = self.strict(self.fixture(body=[stmt, Check(self.LEAK, A, "leak")]))
                self.assertTrue(any("`tok`" in x for x in probs), probs)

    def test_a_token_in_an_assignment_beyond_a_perm_snapshot_is_a_problem(self):
        for rhs in (f"{self.SLOT} ? write : none", f"perm({self.SLOT}) + (({self.SLOT}) ? write : none)"):
            with self.subTest(rhs=rhs):
                probs = self.strict(self.fixture(body=[f"  kt := {rhs}", Check(self.LEAK, A, "leak")]))
                self.assertTrue(any("right-hand side of an assignment" in x for x in probs), probs)

    def test_a_snapshot_of_perm_of_a_token_is_the_one_allowed_assignment(self):
        body = [f"  kt := perm({self.SLOT})", f"  kt := kt > none ? perm({self.SLOT}) : none",
                Check(self.LEAK, A, "leak")]
        self.assertEqual(self.strict(self.fixture(body=body)), [])

    # Decision 0047 item 10 as amended at M4.5d2v: the ending rows of a heap place's back edge, `if (!(<row>)) {`, `assert false`, `}`.
    ROWTXT = "p != null ==> !(perm(loc(block_of(p), (-1)).tok) < kt)"

    FALSE = Check("      assert false", A, "permission")   # an ending row's `assert false` carries the row's map entry

    def ending(self, row=None, mid=None, indent="    "):
        return [f"{indent}if (!({row or self.ROWTXT})) {{", *([self.FALSE] if mid is None else mid), f"{indent}}}"]

    def test_an_ending_row_over_a_token_is_clean(self):
        for row in (self.ROWTXT, f"(p != null) ==> (!(perm({self.SLOT}) < kt))",
                    f"!(perm({self.SLOT}) < kt || (kt > none && {self.SLOT} != ktv_h1))"):
            with self.subTest(row=row):
                body = [*self.ending(row), Check(self.LEAK, A, "leak")]
                self.assertEqual(self.strict(self.fixture(body=body)), [])

    def test_an_ending_row_reads_a_token_only_at_the_token_slot(self):
        for slot in ("loc(block_of(p), 0).tok", "p.tok", "lvloc(block_of(p)).tok"):
            with self.subTest(slot=slot):
                probs = self.strict(self.fixture(body=[*self.ending(f"perm({slot}) >= kt"), Check(self.LEAK, A, "leak")]))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("`tok` outside the token slot of a block", probs[0])

    def test_an_if_over_a_token_with_anything_else_in_its_body_is_a_problem(self):
        cases = {
            "an extra statement": self.ending(mid=[self.FALSE, "      var x: Int := 0"]),
            "another statement": self.ending(mid=["      assert true"]),
            "an empty body": self.ending(mid=[]),
            "an exhale": self.ending(mid=[f"      exhale acc({self.SLOT})"]),
            "an assignment": self.ending(mid=["      kt := none"]),
            "assert false twice": self.ending(mid=[self.FALSE, "      assert false"]),
        }
        for what, body in cases.items():
            with self.subTest(what=what):
                probs = self.strict(self.fixture(body=[*body, Check(self.LEAK, A, "leak")]))
                self.assertTrue(any("`tok` in `if`" in x or "`exhale` of" in x for x in probs), (what, probs))

    def test_an_if_over_a_token_that_is_not_the_negation_of_one_row_is_a_problem(self):
        cases = {
            "not negated": [f"    if ({self.ROWTXT}) {{", self.FALSE, "    }"],
            "negated and joined": [f"    if (!({self.ROWTXT}) || i > 0) {{", self.FALSE, "    }"],
            "two groups": [f"    if (!(p != null) || ({self.SLOT})) {{", self.FALSE, "    }"],
            "an else": [f"    if (!({self.ROWTXT})) {{", self.FALSE, "    } else {", "      var y: Int := 0", "    }"],
            "an ordinary condition": [f"    if ({self.SLOT}) {{", self.FALSE, "    }"],
        }
        for what, body in cases.items():
            with self.subTest(what=what):
                probs = self.strict(self.fixture(body=[*body, Check(self.LEAK, A, "leak")]))
                self.assertTrue(any("`tok` in `if`" in x for x in probs), (what, probs))

    def test_a_bare_assert_false_is_no_token_problem(self):
        # the lint has no rule on `assert false` (a failing assertion proves nothing): outside an ending `if` it is an ordinary assert
        body = [Check("  assert false", A, "permission"), *self.ending(self.ROWTXT), Check(self.LEAK, A, "leak")]
        self.assertEqual(self.strict(self.fixture(body=body)), [])

    # Decision 0050 item 5 (M4.5d2v): the loop value snapshot `ktv_<n>`, the one stored read of a token's value.
    def test_a_loop_value_snapshot_under_its_guard_is_clean(self):
        for body in ([f"  var ktv_h1: Bool := false", f"  if (kt_h1 > none) {{ ktv_h1 := {self.SLOT} }}", Check(self.LEAK, A, "leak")],
                     [f"  var ktv_h1: Bool := false", "  if (kt_h1 > none) {", f"    ktv_h1 := {self.SLOT}", "  }", Check(self.LEAK, A, "leak")],
                     [f"  var ktv_h1__0: Bool := false", "  if (kt_h1__0 > none) {", f"    ktv_h1__0 := {self.SLOT}", "  }", Check(self.LEAK, A, "leak")]):
            with self.subTest(body=body):
                self.assertEqual(self.strict(self.fixture(body=body)), [])

    def test_a_snapshot_under_another_places_guard_is_a_problem(self):
        # audit round 2 N8: `ktv_<n>` is taken under its own `kt_<n>`, not under the guard of another place
        cases = {
            "another place (one line)": [f"  if (kt_h2 > none) {{ ktv_h1 := {self.SLOT} }}"],
            "another place (two lines)": ["  if (kt_h2 > none) {", f"    ktv_h1 := {self.SLOT}", "  }"],
            "a longer suffix": ["  if (kt_h1__0 > none) {", f"    ktv_h1 := {self.SLOT}", "  }"],
            "a shorter suffix": ["  if (kt_h1 > none) {", f"    ktv_h1__0 := {self.SLOT}", "  }"],
            "the unnamed guard": [f"  if (kt > none) {{ ktv_h1 := {self.SLOT} }}"],
        }
        for what, body in cases.items():
            with self.subTest(what=what):
                probs = self.strict(self.fixture(body=[*body, Check(self.LEAK, A, "leak")]))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("is taken under the guard of", probs[0])

    def test_the_value_invariant_and_back_edge_row_over_the_snapshot_are_clean(self):
        body = [
            "  var i: Int := 0",
            "  var ktv_h1: Bool := false",
            f"  if (kt_h1 > none) {{ ktv_h1 := {self.SLOT} }}",
            "  while (i < 3)",
            Check(f"    invariant p != null ==> acc({self.SLOT}, kt)", INV, "internal"),
            Check(f"    invariant (kt > none) ==> {self.SLOT} == ktv_h1", INV, "internal"),
            Check("    decreases 3 - i", "termination.failed", "termination"),
            "  {",
            "    i := i + 1",
            Check(f"    assert !(perm({self.SLOT}) < kt || (kt > none && {self.SLOT} != ktv_h1))", A, "permission"),
            "  }",
            Check(self.LEAK, A, "leak"),
        ]
        self.assertEqual(self.strict(self.fixture(body=body)), [])

    def test_a_stray_value_snapshot_is_a_problem(self):
        cases = {
            "not under the guard": [f"  ktv_h1 := {self.SLOT}"],
            "under another condition (one line)": [f"  if (i > 0) {{ ktv_h1 := {self.SLOT} }}"],
            "under another condition (two lines)": ["  if (i > 0) {", f"    ktv_h1 := {self.SLOT}", "  }"],
            "a guard that is not a loop snapshot": ["  if (ktv_h1 > none) {", f"    ktv_h1 := {self.SLOT}", "  }"],
            "not named ktv_": [f"  if (kt > none) {{ kx := {self.SLOT} }}"],
            "a negated read": [f"  if (kt > none) {{ ktv_h1 := !{self.SLOT} }}"],
            "another slot": ["  if (kt > none) { ktv_h1 := loc(block_of(p), 0).tok }"],
            "the value of a condition": [f"  if (kt > none) {{ ktv_h1 := {self.SLOT} ? true : false }}"],
        }
        for what, body in cases.items():
            with self.subTest(what=what):
                probs = self.strict(self.fixture(body=[*body, Check(self.LEAK, A, "leak")]))
                self.assertTrue(probs, what)

    def test_a_snapshot_is_not_a_way_to_assign_a_token(self):
        body = [f"  if (kt > none) {{ {self.SLOT} := ktv_h1 }}", Check(self.LEAK, A, "leak")]
        probs = self.strict(self.fixture(body=body))
        self.assertTrue(probs, probs)

    def test_forperm_must_have_the_leak_shape(self):
        for row in ("  assert forperm t: Ref [t.lv] :: false",
                    "  assert forperm t: Ref [u.tok] :: false",
                    "  assert forperm t: Int [t.tok] :: false",
                    "  assert forperm t: Ref [t.tok] ::",
                    f"  assert forperm t: Ref [t.tok] :: ((offset_of(p) == 0) && (t == loc(block_of(p), (-1)))) || {self.SLOT}",
                    "  exhale forperm t: Ref [t.tok] :: false",
                    "  assert (forperm t: Ref [t.tok] :: false)",
                    "  var b: Bool := forperm t: Ref [t.tok] :: false"):
            with self.subTest(row=row):
                probs = self.strict(self.fixture(body=[Check(row, A, "leak")]))
                self.assertTrue(probs, row)

    def test_forperm_with_an_exemption_is_clean(self):
        row = "  assert forperm t: Ref [t.tok] :: ((offset_of(res) == 0) && (t == loc(block_of(res), (-1)))) || (p != null && ((offset_of(p) == 0) && (t == loc(block_of(p), (-1)))))"
        self.assertEqual(self.strict(self.fixture(body=[Check(row, A, "leak")])), [])

    def test_an_exemption_term_without_offset_of_zero_is_a_problem(self):
        # decision 0059 item 10: a pointer into the middle of a block carries no token, so each term holds offset_of(<its pointer>) == 0
        prefix = "  assert forperm t: Ref [t.tok] :: "
        for body in ("t == loc(block_of(res), (-1))", "(p != null) && (t == loc(block_of(q), (-1)))",
                     "(offset_of(q) == 0) && (t == loc(block_of(p), (-1)))", "(offset_of(p) == 1) && (t == loc(block_of(p), (-1)))",
                     "((offset_of(res) == 0) && (t == loc(block_of(res), (-1)))) || ((p != null) && (t == loc(block_of(q), (-1))))"):
            with self.subTest(body=body):
                probs = self.strict(self.fixture(body=[Check(prefix + body, A, "leak")]))
                self.assertTrue(any("holds `offset_of(<pointer>) == 0`" in p for p in probs), probs)
        # twins: the conjunct at any grouping is read
        for body in ("(offset_of(p) == 0) && (t == loc(block_of(p), (-1)))", "((p != null) && (offset_of(p) == 0)) && (t == loc(block_of(p), (-1)))",
                     "(p != null) && ((offset_of(p) == 0) && (t == loc(block_of(p), (-1))))"):
            with self.subTest(clean=body):
                self.assertEqual(self.strict(self.fixture(body=[Check(prefix + body, A, "leak")])), [])

    def test_forperm_must_be_a_leak_row(self):
        for category in ("overflow", "permission", "invalid-free", "internal"):
            with self.subTest(category=category):
                probs = self.strict(self.fixture(leak_category=category))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("`forperm` is only a `leak` row", probs[0])

    def test_a_forperm_without_a_map_entry_is_the_missing_entry_problem(self):
        text, map_text = build(self.fixture())
        lines = map_text.rstrip("\n").split("\n")
        n = text.split("\n").index(self.LEAK) + 1
        kept = "\n".join(l for l in lines if l.split("\t")[0] != str(n)) + "\n"
        probs = lv.lint_vpr(text, kept, "t.vpr", "t.vpr.map")
        self.assertTrue(any("`assert` line has no entry in the position map" in p for p in probs), probs)

    def test_the_heap_rules_leave_programs_without_a_token_alone(self):
        self.assertEqual(lint(), [])


class VlibcTokenTests(unittest.TestCase):
    """Decision 0052 item 7 (0050 item 3): a program that names `cerastes_owns_vlibc_block` is a vlibc heap program, whose block
    site holds the token's value `false`: `!loc(block_of(p), -1).tok` is the token slot like `loc(block_of(p), -1).tok` is, in the
    conjuncts of a `requires`, `ensures`, `invariant` or `assert`.  It is no more than that: a condition, an assignment or
    another slot is as refused as for the platform's token."""

    SLOT = HeapTokenTests.SLOT

    @staticmethod
    def strict(lines):
        return lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")

    def fixture(self, requires=None, body=None):
        block = requires or f"p != null ==> acc({self.SLOT}) && !{self.SLOT}"
        return HeapTokenTests("test_the_emitters_shape_is_clean").fixture(requires=block, body=body if body is not None else [
            Check(f"  assert p == null || (perm({self.SLOT}) == write && !{self.SLOT})", A, "invalid-free"),
            Check("  f_free__1(p)", PRE, "invalid-free"),
            Check(HeapTokenTests.LEAK, A, "leak"),
        ])

    def test_a_negated_token_is_clean_in_a_block_site_and_a_row(self):
        self.assertEqual(self.strict(self.fixture()), [])

    def test_a_negated_token_in_a_loop_place_is_clean(self):
        body = [
            "  var i: Int := 0",
            "  while (i < 3)",
            Check(f"    invariant p != null ==> acc({self.SLOT}, kt)", INV, "internal"),
            Check(f"    invariant kt > none ==> !{self.SLOT}", INV, "internal"),
            Check("    decreases 3 - i", "termination.failed", "termination"),
            "  {",
            "    i := i + 1",
            Check(f"    assert perm({self.SLOT}) >= kt", A, "permission"),
            "  }",
            Check(HeapTokenTests.LEAK, A, "leak"),
        ]
        self.assertEqual(self.strict(self.fixture(body=body)), [])

    def test_a_negated_token_at_another_slot_is_a_problem(self):
        for slot in ("p.tok", "loc(block_of(p), 0).tok", "loc(block_of(p), (-2)).tok"):
            with self.subTest(slot=slot):
                probs = self.strict(self.fixture(body=[Check(f"  assert !{slot}", A, "invalid-free"),
                                                       Check(HeapTokenTests.LEAK, A, "leak")]))
                self.assertTrue(any("`tok` outside the token slot of a block" in p for p in probs), probs)

    def test_a_negated_token_in_a_condition_is_a_problem(self):
        probs = self.strict(self.fixture(body=[f"  if (!{self.SLOT}) {{", "  }", Check(HeapTokenTests.LEAK, A, "leak")]))
        self.assertTrue(any("`tok` in `if`" in p for p in probs), probs)

    def test_a_negated_token_in_an_assignment_is_a_problem(self):
        probs = self.strict(self.fixture(body=["  var b: Bool", f"  b := !{self.SLOT}", Check(HeapTokenTests.LEAK, A, "leak")]))
        self.assertTrue(any("`tok` in the right-hand side of an assignment" in p for p in probs), probs)

    def test_a_negated_token_in_an_exhale_is_a_problem(self):
        probs = self.strict(self.fixture(body=[f"  exhale !{self.SLOT}", Check(HeapTokenTests.LEAK, A, "leak")]))
        self.assertTrue(any("`exhale` of a `tok` place" in p for p in probs), probs)


class OwnerGhostTests(unittest.TestCase):
    """Decision 0052 item 3: an owning local `v` has the method-level `Bool` ghosts `held_v` and `tv_v`.  The lint reads the shapes
    the lowering emits (the real output of tests/memory/owning_local_free.c): the owner's token value
    `tv_v := (t != null) ? loc(block_of(t), -1).tok : tv_v` is the one assignment that reads a token outside a `perm(...)`
    snapshot (the same ghost on both sides, the same pointer in the guard and the slot); `held_o := held_o && o != null &&
    perm(loc(block_of(o), -1).tok) > none` reads it inside `perm(...)`; the value row and the ending rows are `if (!(<row>)) {`,
    `assert false`, `}`.  Each rule has a twin that breaks exactly it."""

    SLOT = HeapTokenTests.SLOT
    TV = f"  tv_v := (t != null) ? loc(block_of(t), (-1)).tok : tv_v"
    HELD = f"  held_v := held_v && (((v != null) && (offset_of(v) == 0)) && (perm(loc(block_of(v), (-1)).tok) > none))"

    @staticmethod
    def strict(lines):
        return lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")

    def fixture(self, tv=None, extra=None):
        return HeapTokenTests("test_the_emitters_shape_is_clean").fixture(body=[
            "  var held_v: Bool",
            "  var tv_v: Bool",
            "  var v: Ref",
            "  var t: Ref",
            "  held_v := false",
            "  t := p",
            "  if (!((t != null) ==> (offset_of(t) == 0 && perm(loc(block_of(t), (-1)).tok) == write))) {",
            Check("    assert false", A, "permission"),
            "  }",
            "  v := t",
            "  held_v := t != null",
            tv if tv is not None else self.TV,
            self.HELD,
            "  if (!(!held_v)) {",
            Check("    assert false", A, "leak"),
            "  }",
            *(extra or []),
            Check(HeapTokenTests.LEAK, A, "leak"),
        ])

    def test_the_emitters_shape_is_clean(self):
        self.assertEqual(self.strict(self.fixture()), [])

    def test_an_owner_update_without_offset_of_zero_is_a_problem(self):
        # decision 0059 item 10: holds_token(v) is v != null && offset_of(v) == 0 && perm(tok) > none
        for rhs in (drop_offset_conjuncts(self.HELD), self.HELD.replace("offset_of(v) == 0", "offset_of(t) == 0"),
                    self.HELD.replace("offset_of(v) == 0", "offset_of(v) == 1")):
            with self.subTest(rhs=rhs[:70]):
                lines = self.fixture()
                lines[lines.index(self.HELD)] = rhs
                probs = self.strict(lines)
                self.assertTrue(any("an owner's ghost is assigned" in p for p in probs), probs)

    def test_the_token_value_of_another_pointer_is_a_problem(self):
        probs = self.strict(self.fixture(tv="  tv_v := (t != null) ? loc(block_of(v), (-1)).tok : tv_v"))
        self.assertTrue(any("`tok` in the right-hand side of an assignment" in p for p in probs), probs)

    def test_a_token_value_whose_else_branch_is_another_variable_is_a_problem(self):
        for tv in ("  tv_v := (t != null) ? loc(block_of(t), (-1)).tok : false", "  tv_v := (t != null) ? loc(block_of(t), (-1)).tok : tv_w",
                   "  tv_v := (t == null) ? tv_v : loc(block_of(t), (-1)).tok"):
            with self.subTest(tv=tv):
                probs = self.strict(self.fixture(tv=tv))
                self.assertTrue(any("`tok` in the right-hand side of an assignment" in p for p in probs), probs)

    def test_a_token_value_that_is_not_guarded_by_a_null_test_is_a_problem(self):
        for tv in ("  tv_v := loc(block_of(t), (-1)).tok", "  tv_v := (held_v) ? loc(block_of(t), (-1)).tok : tv_v",
                   "  tv_v := (t != null) ? loc(block_of(t), (-1)).tok : tv_v || true"):
            with self.subTest(tv=tv):
                probs = self.strict(self.fixture(tv=tv))
                self.assertTrue(any("`tok` in the right-hand side of an assignment" in p for p in probs), probs)

    def test_only_a_tv_ghost_may_take_a_token_value(self):
        for name in ("flag", "held_v", "ktv_1x", "tvv"):
            with self.subTest(name=name):
                probs = self.strict(self.fixture(tv=f"  {name} := (t != null) ? loc(block_of(t), (-1)).tok : {name}"))
                self.assertTrue(any("`tok` in the right-hand side of an assignment" in p for p in probs), probs)

    def test_the_token_value_at_another_slot_is_a_problem(self):
        for slot in ("loc(block_of(t), 0)", "loc(block_of(t), (-2))"):
            with self.subTest(slot=slot):
                probs = self.strict(self.fixture(tv=f"  tv_v := (t != null) ? {slot}.tok : tv_v"))
                self.assertTrue(any("`tok` outside the token slot of a block" in p for p in probs), probs)

    def test_the_held_update_reads_the_token_only_inside_perm(self):
        probs = self.strict(self.fixture(extra=["  held_v := held_v && loc(block_of(v), (-1)).tok"]))
        self.assertTrue(any("`tok` in the right-hand side of an assignment" in p for p in probs), probs)

    def test_a_token_in_an_ordinary_condition_is_still_a_problem(self):
        probs = self.strict(self.fixture(extra=["  if (loc(block_of(v), (-1)).tok) {", "  }"]))
        self.assertTrue(any("`tok` in `if`" in p for p in probs), probs)


class OwnerLoopTests(unittest.TestCase):
    """Decision 0052 item 3, loops (the real output of tests/memory/owning_loop_replace.c and tests/vlibc/vlibc_owning_loop_replace.c):
    an owning variable of a loop has entry and back-edge rows (`if (!(<row>)) {`, `assert false`, `}`, which read the token under
    `perm(...)`), the invariants `held_v == (v != null)` and `(v != null) ==> (acc(<slot>.tok) && <slot>.tok == tve_...)`, and a
    disjunct of the leak exemption.  In a vlibc heap program `tve_<v>__<loop> := (v != null) ? tv_<v> : true` is the allocator it held
    at the entry.  The lint accepts exactly these shapes: the token rules read the rows and the conjuncts, and the `tve` rule is its own."""

    SLOT = HeapTokenTests.SLOT
    TVE = "  tve_v__0 := (p != null) ? tv_v : true"

    @staticmethod
    def strict(lines):
        return lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")

    def row(self, cond):
        return [f"  if (!({cond})) {{", Check("    assert false", A, "permission"), "  }"]

    def fixture(self, tve=None, entry=None, invariants=None, body=None, exempt=None):
        slot = self.SLOT
        inv = invariants if invariants is not None else [
            "    invariant held_v == (p != null)",
            f"    invariant (p != null) ==> (acc({slot}) && ({slot} == tve_v__0))",
        ]
        return HeapTokenTests("test_the_emitters_shape_is_clean").fixture(body=[
            "  var held_v: Bool",
            "  var tv_v: Bool",
            "  var tve_v__0: Bool",
            "  var i: Int",
            "  held_v := false",
            f"  tv_v := (p != null) ? {slot} : tv_v",
            tve if tve is not None else self.TVE,
            *(entry if entry is not None else [*self.row("(p != null) ==> held_v"),
                                              *self.row(f"(p != null) ==> (!((perm({slot}) != write) || ({slot} != tve_v__0)))")]),
            "  i := 0",
            "  while (i < 3)",
            *[Check(t, INV, "internal") for t in inv],
            Check("    decreases 3 - i", "termination.failed", "termination"),
            "  {",
            *(body if body is not None else ["    i := i + 1"]),
            *self.row("(p != null) ==> held_v"),
            "  }",
            Check(f"  assert forperm t: Ref [t.tok] :: {exempt or '(p != null) && ((offset_of(p) == 0) && (t == loc(block_of(p), (-1))))'}", A, "leak"),
        ])

    def test_the_emitters_shape_is_clean(self):
        self.assertEqual(self.strict(self.fixture()), [])

    def test_a_tve_ghost_is_assigned_only_the_owners_own_token_value(self):
        for tve in ("  tve_v__0 := (p != null) ? " + self.SLOT + " : true", "  tve_v__0 := true", "  tve_v__0 := tv_v",
                    "  tve_v__0 := (p != null) ? tv_v : false", "  tve_v__0 := (p != null) ? tv_v : tve_v__0", "  tve_v__0 := !tv_v"):
            with self.subTest(tve=tve):
                probs = self.strict(self.fixture(tve=tve))
                self.assertTrue(any("`tve_v__0` is assigned something but" in p or "`tok` in the right-hand side" in p for p in probs), probs)

    def test_a_tve_ghost_assigned_in_a_loop_that_mentions_it_is_a_problem(self):
        for body in (["    tve_v__0 := (p != null) ? tv_v : true", "    i := i + 1"], ["    i := i + 1", "    tve_v__0 := (p != null) ? tv_v : true"]):
            with self.subTest(body=body):
                probs = self.strict(self.fixture(body=body))
                self.assertEqual([p for p in probs if "is assigned inside the loop" in p].__len__(), 1, probs)

    def test_a_tve_ghost_the_invariant_does_not_mention_may_be_assigned_in_a_loop(self):
        inv = ["    invariant held_v == (p != null)"]
        probs = self.strict(self.fixture(invariants=inv, body=["    tve_v__0 := (p != null) ? tv_v : true", "    i := i + 1"]))
        self.assertEqual(probs, [])

    def test_the_token_in_an_invariant_is_the_slot_of_a_block(self):
        probs = self.strict(self.fixture(invariants=["    invariant held_v == (p != null)", "    invariant (p != null) ==> (loc(block_of(p), 0).tok == tve_v__0)"]))
        self.assertTrue(any("`tok` outside the token slot of a block" in p for p in probs), probs)

    def test_perm_in_an_invariant_is_still_a_problem(self):
        probs = self.strict(self.fixture(invariants=["    invariant held_v == (perm(%s) > none)" % self.SLOT]))
        self.assertTrue(any("`perm(...)` in a loop invariant" in p for p in probs), probs)

    def test_an_entry_row_is_an_ending_row_or_it_may_not_read_a_token(self):
        probs = self.strict(self.fixture(entry=[f"  if (perm({self.SLOT}) == write) {{", "  }"]))
        self.assertTrue(any("`tok` in `if`" in p for p in probs), probs)
        probs = self.strict(self.fixture(entry=[f"  if (!(perm({self.SLOT}) == write)) {{", Check("    assert false", A, "permission"), "    held_v := true", "  }"]))
        self.assertTrue(any("`tok` in `if`" in p for p in probs), probs)

    def test_the_alias_guards_of_a_place_are_plain_conjuncts(self):
        """Shape L6: a loop place `a` of a pointer variable is guarded against each carried owner `v` (`gv`, `gl`, `gt`)."""
        inv = ["    invariant held_v == (p != null)", f"    invariant (p != null) ==> (acc({self.SLOT}) && ({self.SLOT} == tve_v__0))",
               "    invariant (p != null) ==> (lvloc(block_of(q)) != lvloc(block_of(p)))",
               "    invariant (p != null) ==> (block_of(q) != block_of(p))",
               "    invariant (p != null) ==> (q != p.v_int)"]
        self.assertEqual(self.strict(self.fixture(invariants=inv)), [])

    def test_a_vlibc_loop_class_without_tve_reads_the_token_in_its_rows_and_conjunct(self):
        """Shape L8: an owner assigned in the loop only from one allocator has no `tve`: the conjunct is `tok` (platform) or `!tok`
        (vlibc) and the class rows are `!(perm(tok) != write || !tok)` (vlibc) or `!(perm(tok) != write || tok)` (platform)."""
        slot = self.SLOT
        for cls in (f"!{slot}", slot):
            with self.subTest(cls=cls):
                inv = ["    invariant held_v == (p != null)", f"    invariant (p != null) ==> (acc({slot}) && {cls})"]
                entry = [*self.row("(p != null) ==> held_v"), *self.row(f"(p != null) ==> (!((perm({slot}) != write) || !({cls})))")]
                self.assertEqual(self.strict(self.fixture(tve="  held_v := false", invariants=inv, entry=entry)), [])

    def test_a_class_row_or_conjunct_at_another_slot_is_a_problem(self):
        inv = ["    invariant held_v == (p != null)", "    invariant (p != null) ==> (acc(loc(block_of(p), 0).tok) && !loc(block_of(p), 0).tok)"]
        probs = self.strict(self.fixture(invariants=inv))
        self.assertTrue(any("`tok` outside the token slot of a block" in x for x in probs), probs)
        entry = self.row(f"(p != null) ==> (!((perm(p.tok) != write) || !p.tok))")
        probs = self.strict(self.fixture(entry=entry))
        self.assertTrue(any("`tok` outside the token slot of a block" in x for x in probs), probs)

    def test_a_dropped_atom_is_an_unconditional_row_without_a_token(self):
        """Shape L7: a written atom on a carried owner is replaced by `if (!false) { assert false }` before the loop."""
        entry = ["  if (!false) {", Check("    assert false", A, "permission"), "  }"]
        self.assertEqual(self.strict(self.fixture(entry=entry)), [])
        probs = self.strict(self.fixture(entry=[f"  if (!false || {self.SLOT}) {{", "  }"]))
        self.assertTrue(any("`tok` in `if`" in x for x in probs), probs)

    def test_the_leak_exemption_does_not_read_a_token(self):
        probs = self.strict(self.fixture(exempt=f"(p != null) && ({self.SLOT} == tve_v__0)"))
        self.assertTrue(any("does not mention `tok`" in p for p in probs), probs)


class HeapViewTests(unittest.TestCase):
    """Decision 0047 items 2, 4 and 7 (audit S7): the typed views of `malloc`, `calloc` and `free` are bodyless trusted methods
    (an assumption: invariant 4), so their text is pinned as `alloc_<t>`'s is, rebuilt from the type's leaves.  The fixtures in
    scripts/testdata/heap_views are views that `cerastes verify` emitted for an int, a structure with nested structures, a
    structure with a pointer to itself, and structures of every leaf type; each mutation of one of them adds, drops or changes one
    clause and must be reported once."""

    DATA = Path(__file__).resolve().parent / "testdata" / "heap_views"
    NAMES = ("malloc_int", "free_int", "calloc_int", "malloc_struct_nested", "free_struct_nested", "calloc_struct_node",
             "calloc_struct_all", "calloc_struct_cfg", "free_struct_cfg", "malloc_struct_global", "free_struct_global")

    def text(self, name):
        return (self.DATA / f"{name}.vpr").read_text()

    @staticmethod
    def problems(text):
        src = lv.split_source(text)
        decls = lv.declarations(src)
        has_globals, facts = lv.lint_globals(src, decls, lambda n, why: None)
        out = []
        lv.lint_views(src, decls, lambda n, why: out.append(why), has_globals, facts)
        return out

    def one(self, text, fragment=None):
        probs = self.problems(text)
        self.assertEqual(len(probs), 1, probs)
        self.assertIn("is not the trusted typed view of decision 0047", probs[0])
        if fragment:
            self.assertIn(fragment, probs[0])

    def edit_last(self, name, old, new):
        text = self.text(name)
        head, sep, tail = text.rpartition(old)
        self.assertTrue(sep, (name, old))
        return head + new + tail

    def edit(self, name, old, new, count=1):
        text = self.text(name)
        self.assertEqual(text.count(old), count, (name, old, text.count(old)))
        return text.replace(old, new)

    def test_every_emitted_view_is_clean(self):
        for name in self.NAMES:
            with self.subTest(name=name):
                self.assertEqual(self.problems(self.text(name)), [])

    def test_the_fixtures_are_the_emitters_not_a_made_up_shape(self):
        for name in self.NAMES:
            text = self.text(name)
            self.assertIn("typed view: one ", text)
            self.assertEqual(text.count("typed view: one "), 1)

    def test_a_view_is_found_by_its_tag_and_a_method_without_it_is_not_a_view(self):
        # the same text with an extra clause but another reason is an ordinary trusted method: this rule does not apply
        text = self.edit("malloc_int", "vlibc: malloc returns NULL or the start of a new allocation of size bytes, its contents indeterminate (C23 7.24.3.6); typed view: one int (4 bytes), decision 0047",
                         "a user routine that allocates").replace("  decreases _", "  ensures false\n  decreases _")
        self.assertEqual(self.problems(text), [])
        self.one(self.edit("malloc_int", "  decreases _", "  ensures false\n  decreases _"))

    def test_a_vlibc_allocation_tag_without_typed_view_is_reported(self):
        # audit N3: a bodyless method tagged as vlibc's malloc, calloc or free must be a view
        for name in ("malloc_int", "calloc_int", "free_int"):
            with self.subTest(name=name):
                text = self.text(name)
                k = text.index("; typed view:")
                end = text.index("\n", k)
                probs = self.problems(text[:k] + text[end:])
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("tagged as vlibc's malloc, calloc or free but not as a typed view", probs[0])

    def test_the_global_facts_of_a_program_with_globals_are_the_one_leading_requires(self):
        for name in ("malloc_struct_global", "free_struct_global"):
            text = self.text(name)
            facts_line = next(l for l in text.split("\n") if l.startswith("  requires ((len(GB_x__0())"))
            with self.subTest(name=name, what="dropped"):
                self.one(text.replace(facts_line + "\n", ""), "exactly one leading `requires`")
            with self.subTest(name=name, what="twice"):
                self.one(text.replace(facts_line + "\n", facts_line + "\n" + facts_line + "\n"))
            with self.subTest(name=name, what="a fact dropped"):
                self.one(text.replace(facts_line, "  requires (len(GB_x__0()) == 1) && (lvloc(GB_x__0()) == loc(GB_x__0(), (-2)))"),
                         "global facts of the program")
            with self.subTest(name=name, what="an extra fact"):
                self.one(text.replace(facts_line, facts_line + " && false"), "global facts of the program")
            with self.subTest(name=name, what="not the first clause"):
                lines = text.split("\n")
                i = lines.index(facts_line)
                lines[i], lines[i + 1] = lines[i + 1], lines[i]
                self.assertTrue(self.problems("\n".join(lines)))

    def test_a_program_without_globals_has_no_leading_requires_on_a_view(self):
        facts = "  requires ((len(GB_x__0()) == 1) && (lvloc(GB_x__0()) == loc(GB_x__0(), (-2))))\n"
        self.one(self.edit("malloc_int", "method f_malloc__4() returns (res: Ref)\n", "method f_malloc__4() returns (res: Ref)\n" + facts))
        self.one(self.edit("free_int", "method f_free__5(p_ptr__0: Ref)\n", "method f_free__5(p_ptr__0: Ref)\n" + facts.replace("GB_x__0()", "GB_x__0()")))

    def test_an_extra_clause_or_a_missing_one_is_reported(self):
        for name in self.NAMES:
            kw = "requires" if name.startswith("free") else "ensures"
            with self.subTest(name=name, what="extra"):
                self.one(self.edit_last(name, "  decreases _", f"  {kw} false\n  decreases _"))
            with self.subTest(name=name, what="missing"):
                lines = self.text(name).split("\n")
                k = max(i for i, l in enumerate(lines) if l.startswith(f"  {kw} "))
                self.one("\n".join(lines[:k] + lines[k + 1:]))
            with self.subTest(name=name, what="no decreases _"):
                self.one(self.edit_last(name, "  decreases _", "  decreases"))

    def test_an_extra_conjunct_in_the_block_part_is_reported(self):
        for name in self.NAMES:
            lines = self.text(name).split("\n")
            kw = "requires" if name.startswith("free") else "ensures"
            k = max(i for i, l in enumerate(lines) if l.startswith(f"  {kw} "))
            with self.subTest(name=name):
                lines[k] = lines[k] + " && false"
                self.one("\n".join(lines))

    def test_the_pieces_of_the_token_and_the_liveness_cell_are_each_needed(self):
        for old in ("acc(loc(block_of(res), (-1)).tok)) && loc(block_of(res), (-1)).tok", "acc(lvloc(block_of(res)).lv)",
                    "lvloc(block_of(res)) == loc(block_of(res), (-2))", "offset_of(res) == 0"):
            with self.subTest(old=old):
                text = self.text("malloc_int")
                self.assertEqual(text.count(old), 1, old)
                self.one(text.replace(old, "true"))
        # a second token
        self.one(self.edit("malloc_int", "acc(lvloc(block_of(res)).lv)", "acc(lvloc(block_of(res)).lv) && acc(loc(block_of(res), (-1)).tok)"))

    def test_the_token_slot_is_minus_one(self):
        self.one(self.edit("malloc_int", "acc(loc(block_of(res), (-1)).tok)", "acc(loc(block_of(res), (-2)).tok)"))
        self.one(self.edit("malloc_int", "acc(loc(block_of(res), (-1)).tok)", "acc(loc(block_of(res), 0).tok)"))

    def test_a_malloc_view_states_no_flag_and_no_value(self):
        self.one(self.edit("malloc_int", "&& acc(res.d_int)", "&& acc(res.d_int) && res.d_int"))
        self.one(self.edit("malloc_int", "&& acc(res.d_int)", "&& acc(res.d_int) && res.v_int == 0"))
        self.one(self.edit("malloc_int", "&& acc(res.d_int)", "&& acc(res.d_int) && !res.d_int"))

    def test_a_calloc_view_needs_its_flags_and_zero_values_and_nothing_more(self):
        for name, flag, zero in (("calloc_int", " && res.d_int)", " && (res.v_int == 0))"),):
            with self.subTest(name=name, what="no zero"):
                self.one(self.edit(name, zero, ")"), "adds the flags and then the zero values")
            with self.subTest(name=name, what="no flag"):
                self.one(self.edit(name, flag, ")"), "adds the flags and then the zero values")
            with self.subTest(name=name, what="wrong zero"):
                self.one(self.edit(name, "(res.v_int == 0)", "(res.v_int == 1)"), "adds the flags and then the zero values")
        self.one(self.edit("calloc_struct_cfg", "(loc(member(res, 1), 0).v_bool == false)", "(loc(member(res, 1), 0).v_bool == true)"))
        text = self.text("calloc_struct_cfg")
        zero = "(loc(member(res, 2), 0).v_ptr_int == null)"
        head, _, tail = text.rpartition(zero)
        self.one(head + "true" + tail)
        self.one(self.edit("calloc_struct_cfg", "(loc(member(res, 3), 0).v_enum_1_enum_x20color == 0)", "(loc(member(res, 3), 0).v_enum_1_enum_x20color == 1)"))

    def test_a_range_is_the_range_of_its_fields_type(self):
        self.one(self.edit("malloc_int", "(res.v_int <= 2147483647)", "(res.v_int <= 2147483648)"), "not the range of its field `int`")
        self.one(self.edit("malloc_int", "((-2147483648) <= res.v_int)", "((-2147483649) <= res.v_int)"), "not the range of its field `int`")
        self.one(self.edit("calloc_struct_all", "(0 <= loc(member(res, 2), 0).v_uint)", "((-1) <= loc(member(res, 2), 0).v_uint)"),
                 "not the range of its field `uint`")
        self.one(self.edit("calloc_struct_all", "<= 255)", "<= 256)"), "not the range of its field `uchar`")

    def test_an_enumerations_range_must_hold_zero(self):
        self.one(self.edit("calloc_struct_cfg", "(0 <= loc(member(res, 3), 0).v_enum_1_enum_x20color)", "(1 <= loc(member(res, 3), 0).v_enum_1_enum_x20color)"),
                 "not the range of its field")

    def test_leaves_are_in_member_order_with_their_cells(self):
        text = self.text("calloc_struct_cfg")
        swapped = text.replace("loc(member(res, 0), 0)", "TMP").replace("loc(member(res, 1), 0)", "loc(member(res, 0), 0)").replace("TMP", "loc(member(res, 1), 0)")
        self.one(swapped)

    def test_an_undeclared_field_suffix_is_reported(self):
        self.one(self.edit("malloc_int", "field v_int: Int\n", ""), "is not declared")
        self.one(self.edit("malloc_int", "field d_int: Bool\n", ""), "is not declared")

    def test_the_header_and_the_name_are_pinned(self):
        self.one(self.edit("malloc_int", "method f_malloc__4() returns (res: Ref)", "method f_free__4() returns (res: Ref)"), "its name")
        self.one(self.edit("malloc_int", "method f_malloc__4() returns (res: Ref)", "method f_malloc__4() returns (res: Ref, x: Int)"), "header")
        self.one(self.edit("free_int", "method f_free__5(p_ptr__0: Ref)", "method f_free__5(q: Ref)"), "header")
        self.one(self.edit("malloc_int", "method f_malloc__4", "method g_malloc__4"), "its name")

    # Decision 0050 items 3 and 9 (M4.5d2v): the views of vlibc's `vlibc_malloc`, `vlibc_calloc` and `vlibc_free` are pinned like
    # `malloc`'s, with the token's value false.  The fixtures scripts/testdata/heap_views/vlibc_*.vpr are views `cerastes verify`
    # emitted (for an int, and for the structures `account` and `pair`), extracted from the programs of tests/vlibc.
    VLIBC_NAMES = ("vlibc_malloc_int", "vlibc_free_int", "vlibc_calloc_int", "vlibc_calloc_struct_account",
                   "vlibc_free_struct_account", "vlibc_calloc_struct_pair", "vlibc_free_struct_pair")

    def test_every_emitted_vlibc_view_is_clean(self):
        for name in self.VLIBC_NAMES:
            with self.subTest(name=name):
                text = self.text(name)
                self.assertNotIn("PROVISIONAL", text)
                self.assertIn("typed view: one ", text)
                self.assertEqual(text.count("typed view: one "), 1)
                self.assertEqual(self.problems(text), [])

    def test_a_vlibc_view_has_a_false_token_and_a_platform_view_a_true_one(self):
        for name in self.VLIBC_NAMES:
            self.assertEqual(self.text(name).count("!loc(block_of("), 1, name)
        for name in self.NAMES:
            self.assertNotIn("!loc(block_of(", self.text(name))

    def test_a_vlibc_view_with_a_true_token_is_reported(self):
        for name in self.VLIBC_NAMES:
            with self.subTest(name=name):
                text = self.text(name)
                self.one(text.replace("!loc(block_of(", "loc(block_of("), "`!tok`: a block of vlibc's heap")

    def test_a_platform_view_with_a_false_token_is_reported(self):
        import re as _re
        for name in self.NAMES:
            with self.subTest(name=name):
                text = self.text(name)
                changed = _re.sub(r"(acc\(loc\(block_of\((\w+)\), \(-1\)\)\.tok\)\) && )(loc\(block_of\(\2\), \(-1\)\)\.tok)\)",
                                  r"\1!\3)", text)
                self.assertNotEqual(changed, text)
                self.one(changed, "`tok`: a block of the platform's heap")

    def test_a_vlibc_view_needs_its_own_name_and_tag(self):
        # the routine in the tag, the method's name and the token's value must agree
        for name, platform in (("vlibc_malloc_int", "malloc_int"), ("vlibc_free_int", "free_int"), ("vlibc_calloc_int", "calloc_int")):
            with self.subTest(name=name, what="the name of a platform view"):
                self.one(self.text(name).replace("method f_vlibc_", "method f_", 1), "its name is not `f_vlibc_")
            with self.subTest(name=name, what="the name of a vlibc view on a platform tag"):
                self.one(self.text(platform).replace("method f_", "method f_vlibc_", 1), "its name is not `f_")
            with self.subTest(name=name, what="the tag of a platform view on a vlibc method"):
                self.one(self.text(name).replace("vlibc: vlibc_", "vlibc: ", 1), "its name is not")

    def test_a_vlibc_view_is_pinned_like_a_platform_view(self):
        self.one(self.text("vlibc_calloc_int").replace("  decreases _", "  ensures false\n  decreases _", 1))
        self.one(self.text("vlibc_malloc_int").replace("  decreases _", "  decreases _\n{\n}"), "it has a body")
        self.one(self.text("vlibc_free_int").replace("acc(p_ptr__0.d_int)", "true"))
        self.one(self.text("vlibc_free_struct_pair").replace("acc(loc(member(p_ptr__0, 0), 0).d_int)", "true"))
        self.one(self.text("vlibc_calloc_struct_pair").replace("  decreases _", "  ensures false\n  decreases _", 1))

    def test_a_vlibc_allocation_tag_without_typed_view_is_reported_too(self):
        for name in ("vlibc_malloc_int", "vlibc_calloc_int", "vlibc_free_int"):
            with self.subTest(name=name):
                text = self.text(name)
                k = text.index("; typed view:")
                end = text.index("\n", k)
                probs = self.problems(text[:k] + text[end:])
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("tagged as vlibc's malloc, calloc or free but not as a typed view", probs[0])

    def test_a_typed_view_tag_of_an_unknown_routine_is_reported_not_skipped(self):
        # decision 0050 item 9: such a method was skipped silently, and its contract is an assumption
        for name in ("malloc_int", "free_int"):
            for old, new in (("vlibc: malloc ", "vlibc: realloc "), ("vlibc: malloc ", "vlibc: vlibc_realloc "), ("vlibc: free ", "vlibc: aligned_alloc "),
                             ("vlibc: malloc ", "vlibc: vlibc_free_sized ")):
                text = self.text(name)
                if old not in text:
                    continue
                with self.subTest(name=name, new=new):
                    probs = self.problems(text.replace(old, new, 1))
                    self.assertEqual(len(probs), 1, probs)
                    self.assertIn("is tagged `; typed view:` but is not a view of", probs[0])
        # a method that is not tagged as a view at all is still no view
        text = self.text("malloc_int").replace("; typed view: one int (4 bytes), decision 0047", "")
        self.assertEqual([x for x in self.problems(text) if "typed view" in x and "not as a typed view" not in x], [])

    def test_a_view_with_a_body_is_reported(self):
        self.one(self.edit("malloc_int", "  decreases _", "  decreases _\n{\n}"), "it has a body")

    def test_the_first_clause_is_the_pointers_wellformedness(self):
        first = "(res == null) || (((res == loc(block_of(res), offset_of(res))) && (0 <= offset_of(res))) && (offset_of(res) <= len(block_of(res))))"
        self.one(self.edit("malloc_int", "  ensures " + first + "\n", "  ensures true\n"), "first clause")
        self.one(self.edit("malloc_int", "  ensures " + first + "\n", "  ensures " + first + " && res != null\n"), "first clause")

    def test_a_structures_member_blocks_and_slot_facts_are_pinned(self):
        name = "malloc_struct_nested"
        text = self.text(name)
        self.assertIn("(len(member(res, 0)) == 1)", text)
        # a member block dropped, twice, or a slot fact dropped, in the body and in the first clause alike
        self.one(text.replace("(len(member(res, 0)) == 1) && ", "", 2))
        self.one(text.replace("(len(member(res, 0)) == 1)", "(len(member(res, 0)) == 1) && (len(member(res, 0)) == 1)", 1))
        self.one(text.replace(" && (res != loc(member(res, 0), 0))", "", 2))
        self.one(text.replace("(len(member(res, 0)) == 1)", "(len(member(res, 0)) == 2)", 1))

    def test_a_pointers_pointee_member_blocks_are_pinned(self):
        name = "calloc_struct_node"
        text = self.text(name)
        self.assertIn("len(member(loc(member(res, 0), 0).v_ptr_struct_2_node, 1)) == 1", text)
        self.one(text.replace("(len(member(loc(member(res, 0), 0).v_ptr_struct_2_node, 1)) == 1)", "(len(member(loc(member(res, 0), 0).v_ptr_struct_2_node, 1)) == 0)", 1))

    def test_a_pointer_leaf_needs_its_wellformedness(self):
        wf = "(loc(member(res, 2), 0).v_ptr_int == null) || (((loc(member(res, 2), 0).v_ptr_int == loc(block_of(loc(member(res, 2), 0).v_ptr_int), offset_of(loc(member(res, 2), 0).v_ptr_int))) && (0 <= offset_of(loc(member(res, 2), 0).v_ptr_int))) && (offset_of(loc(member(res, 2), 0).v_ptr_int) <= len(block_of(loc(member(res, 2), 0).v_ptr_int))))"
        self.one(self.edit("calloc_struct_cfg", wf, "true"))

    def test_a_free_view_takes_exactly_what_owns_block_gives(self):
        self.one(self.edit("free_int", "acc(p_ptr__0.d_int)", "true"))
        self.one(self.edit("free_struct_cfg", "acc(loc(member(p_ptr__0, 1), 0).d_bool)", "true"))

    def test_lint_vpr_runs_the_view_rule(self):
        with mock.patch.object(lv, "lint_views") as m:
            lv.lint_vpr(*build(good_lines()), "t.vpr", "t.vpr.map")
        self.assertTrue(m.called)
        text, map_text = build(good_lines())
        text = self.text("malloc_int") + text.replace("field", "// field")
        text = text.replace("  decreases _", "  ensures false\n  decreases _", 1)
        probs = lv.lint_vpr(text, "", "t.vpr", "t.vpr.map")
        self.assertTrue(any("is not the trusted typed view" in x for x in probs), probs)


class SelfTestHygieneTests(unittest.TestCase):
    """A test method defined twice in one class replaces the first, which then never runs (it happened here:
    two `test_labels_followed_by_no_deletable_statement_give_no_mutant`)."""

    @staticmethod
    def defined_twice(source):
        """["Class.name", ...] for every method that a class of `source` defines more than once."""
        import ast
        import collections
        out = []
        for cls in [n for n in ast.walk(ast.parse(source)) if isinstance(n, ast.ClassDef)]:
            names = collections.Counter(f.name for f in cls.body if isinstance(f, ast.FunctionDef))
            out += [f"{cls.name}.{name}" for name, n in sorted(names.items()) if n > 1]
        return out

    def test_no_test_name_is_defined_twice_in_a_class(self):
        self.assertEqual(self.defined_twice(Path(__file__).read_text()), [])

    def test_the_check_sees_a_duplicate(self):
        src = "class C:\n    def test_a(self): pass\n    def test_a(self): pass\n    def test_b(self): pass\n"
        self.assertEqual(self.defined_twice(src), ["C.test_a"])
        self.assertEqual(self.defined_twice(src.replace("def test_a(self): pass\n    def test_b", "def test_c(self): pass\n    def test_b")), [])


class LoopCarrierGhostTests(unittest.TestCase):
    """Decision 0054 items 3 and 4 (as built): the loop-entry ghosts of the loop carriers and the clauses and rows that name them.
    `ev_<v>__<L>` (`Ref`) is `<v>` at entry and the value set `v == ev || v == null || v == x...` is the one invariant that names it;
    `hs_<v>__<L>` (`Bool`) is `held_<v>` at entry, named by `invariant held_<v> == hs` and the row `if (!(hs ==> held_<v>)) {`;
    `dup_...`/`ad_...` is whether two carriers shared a block at entry, named positively only by the exclusivity clause
    `invariant S || (!(<its pair formula>))` and its row.  Each is assigned once before the `while` of its own loop, from exactly its
    right-hand side.  A token clause of a loop carrier is an implication whose antecedent names its pointer, and the exemption of a
    `forperm` leak row only names a token by its block's location.  Each rule has a twin that breaks exactly it."""

    PAIR = "((v != null) && (w != null)) && (block_of(v) == block_of(w))"
    LOCALS = ["  var v: Ref", "  var w: Ref", "  var held_v: Bool", "  var ev_v__0: Ref", "  var hs_v__0: Bool",
              "  var dup_v_w__0: Bool", "  var i: Int"]
    ENTRY = ["  ev_v__0 := v", "  hs_v__0 := held_v",
             f"  dup_v_w__0 := {PAIR}"]
    VALUE_SET = "    invariant (v == ev_v__0) || (v == null)"
    HELD = "    invariant held_v == hs_v__0"
    EXCL = f"    invariant dup_v_w__0 || (!({PAIR}))"

    @staticmethod
    def strict(lines):
        return lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")

    def fixture(self, locals_=None, entry=None, invariants=None, body=None, loops=1):
        invs = invariants if invariants is not None else [self.VALUE_SET, self.HELD, self.EXCL]
        rows = [
            f"    if (!(hs_v__0 ==> held_v)) {{", Check("      assert false", A, "permission"), "    }",
            f"    if (!(dup_v_w__0 || (!({self.PAIR})))) {{", Check("      assert false", A, "permission"), "    }",
        ]
        return [
            "field v_int: Int",
            "method f__1(p: Ref, n: Int)",
            Check("  requires n >= 0", WF, "internal"),
            "  decreases",
            "{",
            *(locals_ if locals_ is not None else self.LOCALS),
            "  held_v := false",
            *(entry if entry is not None else self.ENTRY),
            "  i := 0",
            "  while (i < n)",
            *[Check(i, INV, "internal") for i in invs],
            Check("    decreases n - i", "termination.failed", "termination"),
            "  {",
            *(body if body is not None else ["    i := i + 1", *rows]),
            "  }",
            "}",
        ]

    def probs(self, **kw):
        return self.strict(self.fixture(**kw))

    def test_the_emitters_shape_is_clean(self):
        self.assertEqual(self.probs(), [])

    def test_ad_snapshots_and_the_other_pair_are_clean_too(self):
        pair = "((x != null) && (y != null)) && (block_of(x) == block_of(y))"
        locals_ = [*self.LOCALS[:5], "  var ad_h0_x_y__0: Bool", "  var x: Ref", "  var y: Ref", "  var i: Int"]
        entry = [*self.ENTRY[:2], f"  ad_h0_x_y__0 := {pair}"]
        invs = [self.VALUE_SET, self.HELD, f"    invariant ad_h0_x_y__0 || (!({pair}))",
                "    invariant ((x != null) && (!ad_h0_x_y__0)) ==> (x.v_int == 0)"]
        body = ["    i := i + 1", f"    if (!(hs_v__0 ==> held_v)) {{", Check("      assert false", A, "permission"), "    }",
                f"    if (!(ad_h0_x_y__0 || (!({pair})))) {{", Check("      assert false", A, "permission"), "    }"]
        self.assertEqual(self.probs(locals_=locals_, entry=entry, invariants=invs, body=body), [])

    # ev_ ---------------------------------------------------------------------------------------------------------------------------
    def test_an_entry_value_of_anything_but_its_own_owner_is_a_problem(self):
        for rhs in ("w", "null", "v.v_ptr_int", "(held_v ? v : null)"):
            with self.subTest(rhs=rhs):
                probs = self.probs(entry=[f"  ev_v__0 := {rhs}", *self.ENTRY[1:]])
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("loop-entry ghost `ev_v__0` is assigned", probs[0])
                self.assertIn("`v`, the owner's own value", probs[0])

    def test_an_entry_value_assigned_in_the_loop_twice_or_never_is_a_problem(self):
        probs = self.probs(body=["    ev_v__0 := v", "    i := i + 1"])
        self.assertTrue(any("`ev_v__0` is assigned at or after the `while` of its own loop" in p for p in probs), probs)
        probs = self.probs(entry=[*self.ENTRY, "  ev_v__0 := v"])
        self.assertTrue(any("`ev_v__0` is assigned 2 times" in p for p in probs), probs)
        probs = self.probs(entry=self.ENTRY[1:])
        self.assertTrue(any("`ev_v__0` is assigned 0 times" in p for p in probs), probs)

    def test_an_entry_value_of_another_type_or_of_a_loop_that_does_not_exist_is_a_problem(self):
        locals_ = ["  var ev_v__0: Bool", *[l for l in self.LOCALS if "ev_v__0" not in l]]
        probs = self.probs(locals_=locals_)
        self.assertTrue(any("`ev_v__0` is a `Bool` local" in p for p in probs), probs)
        locals_ = [*self.LOCALS, "  var ev_v__3: Ref"]
        probs = self.probs(locals_=locals_, entry=[*self.ENTRY, "  ev_v__3 := v"])
        self.assertTrue(any("`ev_v__3` names loop 3, which the method does not have" in p for p in probs), probs)

    def test_the_value_set_is_the_only_invariant_that_names_the_entry_value(self):
        for inv in ("    invariant (v == null) || (v == ev_v__0)", "    invariant (v == ev_v__0) || (v != null)",
                    "    invariant (v == ev_v__0) || (w == null)", "    invariant ev_v__0 != null", "    invariant true || (v == ev_v__0)",
                    "    invariant (v == ev_v__0) || (v == w.v_ptr_int)"):
            with self.subTest(inv=inv):
                probs = self.probs(invariants=[inv, self.HELD, self.EXCL])
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("`ev_v__0` appears outside the value set", probs[0])

    def test_a_value_set_with_more_locals_is_clean(self):
        inv = "    invariant (((v == ev_v__0) || (v == null)) || (v == w))"
        self.assertEqual(self.probs(invariants=[inv, self.HELD, self.EXCL]), [])

    def test_the_entry_value_in_an_assert_or_a_precondition_is_a_problem(self):
        probs = self.probs(body=["    i := i + 1", "    assert v == ev_v__0"])
        self.assertTrue(any("`ev_v__0` appears outside the value set" in p for p in probs), probs)

    # hs_ ---------------------------------------------------------------------------------------------------------------------------
    def test_an_entry_flag_of_anything_but_its_own_flag_is_a_problem(self):
        for rhs in ("true", "false", "v != null", "held_w"):
            with self.subTest(rhs=rhs):
                probs = self.probs(entry=[self.ENTRY[0], f"  hs_v__0 := {rhs}", self.ENTRY[2]])
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("loop-entry ghost `hs_v__0` is assigned", probs[0])
                self.assertIn("`held_v`", probs[0])

    def test_an_entry_flag_assigned_in_the_loop_or_of_another_type_is_a_problem(self):
        probs = self.probs(body=["    hs_v__0 := held_v", "    i := i + 1"])
        self.assertTrue(any("`hs_v__0` is assigned at or after the `while` of its own loop" in p for p in probs), probs)
        locals_ = ["  var hs_v__0: Ref", *[l for l in self.LOCALS if "hs_v__0" not in l]]
        probs = self.probs(locals_=locals_)
        self.assertTrue(any("`hs_v__0` is a `Ref` local" in p for p in probs), probs)

    def test_the_entry_flag_is_named_by_one_invariant_and_one_row_only(self):
        for inv in ("    invariant held_v ==> hs_v__0", "    invariant hs_v__0", "    invariant held_v == (hs_v__0 || true)",
                    "    invariant held_w == hs_v__0"):
            with self.subTest(inv=inv):
                probs = self.probs(invariants=[self.VALUE_SET, inv, self.EXCL])
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("`hs_v__0` appears outside `invariant held_v == hs_v__0`", probs[0])

    def test_the_entry_flag_row_is_exactly_its_implication(self):
        for row in ("    if (!(held_v ==> hs_v__0)) {", "    if (!(hs_v__0)) {", "    if (!(hs_v__0 ==> held_v) || true) {"):
            with self.subTest(row=row):
                body = ["    i := i + 1", row, Check("      assert false", A, "permission"), "    }"]
                probs = self.probs(body=body)
                self.assertTrue(any("`hs_v__0` appears outside" in p for p in probs), probs)
        body = ["    i := i + 1", "    if (hs_v__0 ==> held_v) {", "      i := i + 2", "    }"]
        self.assertTrue(any("`hs_v__0` appears outside" in p for p in self.probs(body=body)))

    # dup_/ad_ ----------------------------------------------------------------------------------------------------------------------
    def test_a_pair_snapshot_is_exactly_the_pair_formula(self):
        for rhs in ("(v != null) && (w != null)", "((v != null) && (w != null)) && (block_of(v) != block_of(w))",
                    "((v != null) && (w != null)) && (block_of(v) == block_of(v))", "false", "held_v",
                    "((v != null) || (w != null)) && (block_of(v) == block_of(w))"):
            with self.subTest(rhs=rhs):
                probs = self.probs(entry=[*self.ENTRY[:2], f"  dup_v_w__0 := {rhs}"])
                self.assertTrue(any("loop-entry ghost `dup_v_w__0` is assigned" in p for p in probs), probs)

    def test_the_exclusivity_clause_must_use_the_snapshots_own_formula(self):
        other = "((v != null) && (w != null)) && (block_of(v) == block_of(p))"
        for inv in (f"    invariant dup_v_w__0 || (!({other}))", f"    invariant dup_v_w__0 || ({self.PAIR})",
                    "    invariant dup_v_w__0 || true", "    invariant dup_v_w__0"):
            with self.subTest(inv=inv):
                probs = self.probs(invariants=[self.VALUE_SET, self.HELD, inv])
                self.assertTrue(any("`dup_v_w__0` is used positively outside the exclusivity clause" in p for p in probs) or
                                any("`dup_v_w__0`" in p for p in probs), probs)

    def test_a_snapshot_is_never_an_assumption_in_a_body_row(self):
        body = ["    i := i + 1", f"    if (!(dup_v_w__0 || ({self.PAIR}))) {{", Check("      assert false", A, "permission"), "    }"]
        self.assertTrue(any("`dup_v_w__0` is used positively" in p for p in self.probs(body=body)))
        body = ["    i := i + 1", "    assert dup_v_w__0"]
        self.assertTrue(any("`dup_v_w__0` is used positively" in p for p in self.probs(body=body)))

    def test_a_negated_snapshot_is_a_guard_and_is_clean(self):
        inv = "    invariant (held_v && (!dup_v_w__0)) ==> (v.v_int == 0)"
        self.assertEqual(self.probs(invariants=[self.VALUE_SET, self.HELD, self.EXCL, inv]), [])

    def test_a_snapshot_assigned_in_the_loop_or_twice_is_a_problem(self):
        body = ["    i := i + 1", f"    dup_v_w__0 := {self.PAIR}"]
        self.assertTrue(any("`dup_v_w__0` is assigned at or after the `while`" in p for p in self.probs(body=body)))
        probs = self.probs(entry=[*self.ENTRY, f"  dup_v_w__0 := {self.PAIR}"])
        self.assertTrue(any("`dup_v_w__0` is assigned 2 times" in p for p in probs), probs)

    def test_an_inner_loops_ghost_may_stand_in_the_outer_loops_body(self):
        lines = [
            "field v_int: Int",
            "method f__1(n: Int)",
            Check("  requires n >= 0", WF, "internal"),
            "  decreases",
            "{",
            "  var held_v: Bool",
            "  var hs_v__1: Bool",
            "  var i: Int",
            "  while (i < n)",
            Check("    invariant n >= 0", INV, "internal"),
            Check("    decreases n - i", "termination.failed", "termination"),
            "  {",
            "    hs_v__1 := held_v",
            "    while (i < n)",
            Check("      invariant held_v == hs_v__1", INV, "internal"),
            Check("      decreases n - i", "termination.failed", "termination"),
            "    {",
            "      i := i + 1",
            "    }",
            "  }",
            "}",
        ]
        self.assertEqual(self.strict(lines), [])
        lines[lines.index("    hs_v__1 := held_v")] = "    i := i + 0"
        lines[lines.index("      i := i + 1")] = "      hs_v__1 := held_v"
        probs = self.strict(lines)
        self.assertTrue(any("`hs_v__1` is assigned at or after the `while` of its own loop" in p for p in probs), probs)

    def test_other_locals_with_similar_names_are_not_ghosts(self):
        locals_ = [*self.LOCALS, "  var ev: Ref", "  var hs: Bool", "  var dup: Bool", "  var advance: Bool"]
        body = ["    ev := v", "    hs := true", "    dup := hs", "    advance := dup", "    i := i + 1"]
        self.assertEqual(self.probs(locals_=locals_, body=body), [])

    # the token clause of a carrier ----------------------------------------------------------------------------------------------------
    SLOT = "loc(block_of(v), (-1)).tok"

    def carrier(self, inv):
        lines = self.fixture(invariants=[self.VALUE_SET, self.HELD, self.EXCL, inv])
        lines.insert(0, "field tok: Bool")
        return lines

    def test_a_guarded_token_clause_is_clean(self):
        for inv in (f"    invariant (v != null) ==> (acc({self.SLOT}) && {self.SLOT})",
                    f"    invariant (held_v && (!(((v != null) && (w != null)) && (block_of(v) == block_of(w))))) ==> (acc({self.SLOT}) && {self.SLOT})",
                    f"    invariant (((v != null) && (!dup_v_w__0)) && (i >= 0)) ==> acc({self.SLOT}, none)"):
            with self.subTest(inv=inv):
                self.assertEqual(self.strict(self.carrier(inv)), [])

    def test_a_token_clause_without_a_guard_that_names_its_pointer_is_a_problem(self):
        for inv in (f"    invariant acc({self.SLOT})", f"    invariant acc({self.SLOT}) && {self.SLOT}",
                    f"    invariant (w != null) ==> acc({self.SLOT})", f"    invariant acc({self.SLOT}) ==> (v != null)",
                    f"    invariant (i >= 0) || acc({self.SLOT})"):
            with self.subTest(inv=inv):
                probs = self.strict(self.carrier(inv))
                self.assertEqual(len(probs), 1, probs)
                self.assertIn("an `invariant` holds the token of `block_of(v)` outside an implication whose antecedent names `v`", probs[0])

    # forperm exemptions --------------------------------------------------------------------------------------------------------------
    def leak(self, exemption):
        row = f"  assert forperm t: Ref [t.tok] :: {exemption}"
        lines = HeapTokenTests("test_the_emitters_shape_is_clean").fixture(body=[Check(row, A, "leak")])
        return self.strict(lines)

    def test_exemption_terms_that_name_a_carried_block_are_clean(self):
        for e in ("false", "((offset_of(res) == 0) && (t == loc(block_of(res), (-1))))", "(v != null) && ((offset_of(v) == 0) && (t == loc(block_of(v), (-1))))",
                  "(held_v && ((offset_of(v) == 0) && (t == loc(block_of(v), (-1))))) || ((w != null) && ((offset_of(w) == 0) && (t == loc(block_of(w), (-1)))))",
                  "((((loc(member(s, 0), 0) == q) && (k > none)) ? q.v_ptr_int : null) != null) && ((offset_of(q.v_ptr_int) == 0) && (t == loc(block_of(q.v_ptr_int), (-1))))",
                  "(held_v && ((offset_of(v) == 0) && (t == loc(block_of(v), -1))))"):
            with self.subTest(e=e):
                self.assertEqual(self.leak(e), [])

    def test_nested_disjunctions_of_exemption_terms_are_flattened(self):
        a, b, c = ("(held_%s && ((offset_of(%s) == 0) && (t == loc(block_of(%s), (-1)))))" % (n, n, n) for n in ("u", "v", "w"))
        d = "(w != null) && ((offset_of(w) == 0) && (t == loc(block_of(w), (-1))))"
        for e in (f"(({a} || {b}) || {c})", f"({a} || ({b} || {c}))", f"((({a} || {b}) || {c}) || ({d}))", f"{a} || {b} || {c}"):
            with self.subTest(e=e):
                self.assertEqual(self.leak(e), [])

    def test_a_nested_disjunct_without_a_token_term_is_still_a_problem(self):
        a, b = ("(held_%s && ((offset_of(%s) == 0) && (t == loc(block_of(%s), (-1)))))" % (n, n, n) for n in ("u", "v"))
        for e in (f"(({a} || {b}) || true)", f"(({a} || (v != null)) || {b})", f"(({a} || {b}) || (t == t))",
                  f"(({a} || {b}) || (t != loc(block_of(v), (-1))))", f"(({a} || {b}) || !((offset_of(v) == 0) && (t == loc(block_of(v), (-1)))))"):
            with self.subTest(e=e):
                probs = self.leak(e)
                self.assertEqual(len(probs), 1, probs)
                self.assertTrue("a `forperm` leak row is" in probs[0] or "in a `forperm` exemption" in probs[0], probs)

    def test_an_exemption_that_could_exempt_more_than_a_carried_block_is_a_problem(self):
        for e in ("true", "v != null", "((offset_of(v) == 0) && (t == loc(block_of(v), (-1)))) || true", "(v != null) || ((offset_of(v) == 0) && (t == loc(block_of(v), (-1))))",
                  "!((offset_of(v) == 0) && (t == loc(block_of(v), (-1))))", "t != loc(block_of(v), (-1))", "t == v", "t == loc(block_of(v), 0)",
                  "t == loc(block_of(v), (-2))", "((offset_of(v) == 0) && (t == loc(block_of(v), (-1)))) && ((offset_of(w) == 0) && (t == loc(block_of(w), (-1))))",
                  "(t == t) && ((offset_of(v) == 0) && (t == loc(block_of(v), (-1))))", "(t == null) && ((offset_of(v) == 0) && (t == loc(block_of(v), (-1))))"):
            with self.subTest(e=e):
                probs = self.leak(e)
                self.assertEqual(len(probs), 1, probs)
                self.assertTrue("a `forperm` leak row is" in probs[0] or "in a `forperm` exemption" in probs[0], probs)

    # ab_q ----------------------------------------------------------------------------------------------------------------------------
    def test_a_written_sites_iteration_pointer_is_a_ghost_like_aw_q(self):
        fx = GhostRefTests("test_the_emitters_shape_is_clean")
        ab = ["  var eb_1: Ref", "  var lb_2__0: Ref", "  var ab_q0__0: Ref", "  var i: Int"]
        read = "    ab_q0__0 := ((loc(member(x, 0), 0) == y) && (k > none)) ? loc(member(x, 0), 0).v_ptr_int : null"
        self.assertEqual(fx.strict(fx.fixture(locals_=ab, body=[read, "    i := i + 1"])), [])
        probs = fx.strict(fx.fixture(locals_=ab, body=["    i := i + 1", read]))
        self.assertTrue(any("`ab_q0__0` is assigned after other statements of the loop body" in p for p in probs), probs)
        probs = fx.strict(fx.fixture(locals_=ab, body=["    ab_q0__0 := x", "    i := i + 1"]))
        self.assertTrue(any("`ab_q0__0` is assigned `x`" in p for p in probs), probs)
        probs = fx.strict(fx.fixture(locals_=ab, body=[read, read, "    i := i + 1"]))
        self.assertTrue(any("`ab_q0__0` is assigned 2 times" in p for p in probs), probs)
        probs = fx.strict(fx.fixture(locals_=ab, before=[*fx.LB, read], body=["    i := i + 1"]))
        self.assertTrue(any("`ab_q0__0` is assigned outside a loop body" in p for p in probs), probs)


class OwningGlobalTagTests(unittest.TestCase):
    """Decision 0054 items 5, 6 and 8 (Part C, as built): the ownership ghosts of an owning global, `static` local or address-taken
    owner and `main`'s exemption of an owning global's block.  `program_start` tags each owning global `// cerastes:owning-global
    GB_x` (right after its head, before its first clause, once, for a declared block); `held_GB_x` exists only for a tagged
    block; an owner's `held_` is assigned only `false`, `x != null`, the two updates that read the token's permission, or the
    address-taken owner's `held || ...`; `tv_` only `true` or the conditional read of the token; an exemption term with a `held_GB_x`
    guard names exactly `((offset_of(loc(GB_x(), 0).v_ptr_T) == 0) && (t == loc(block_of(loc(GB_x(), 0).v_ptr_T), -1)))`; `held_GB_x == ...` only compares with the cell's
    nullness.  Each rule has a twin that breaks exactly it (only the messages of these rules are read)."""

    CELL = "loc(GB_g__0(), 0).v_ptr_int"
    TERM = f"held_GB_g__0 && ((offset_of({CELL}) == 0) && (t == loc(block_of({CELL}), (-1))))"
    UPD_LOCAL = "  held_v := held_v && (((v != null) && (offset_of(v) == 0)) && (perm(loc(block_of(v), (-1)).tok) > none))"
    UPD_GLOBAL = (f"  held_GB_g__0 := (held_GB_g__0 && (perm({CELL}) > none)) && ((({CELL} != null) && (offset_of({CELL}) == 0)) && "
                  f"(perm(loc(block_of({CELL}), (-1)).tok) > none))")
    UPD_CELL = ("  held_v := (held_v && (perm(v.v_ptr_int) > none)) && (((v.v_ptr_int != null) && (offset_of(v.v_ptr_int) == 0)) && "
                "(perm(loc(block_of(v.v_ptr_int), (-1)).tok) > none))")
    RECEIVE = "  held_v := held_v || (((t != null) && (offset_of(t) == 0)) && (perm(loc(block_of(t), (-1)).tok) == write))"
    TV = "  tv_v := (t != null) ? loc(block_of(t), (-1)).tok : tv_v"
    LEGAL = ["  held_GB_g__0 := false", "  held_GB_g__0 := t != null", TV, UPD_LOCAL, UPD_GLOBAL, UPD_CELL,
             RECEIVE, "  held_v := false", "  held_v := t != null"]

    @staticmethod
    def strict(lines):
        return lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")

    def fixture(self, tag="  // cerastes:owning-global GB_g__0", body=None, term=None, extra_gb=False):
        return [
            "field v_ptr_int: Ref",
            "field tok: Bool",
            "unique function GB_g__0(): Block",
            *(["unique function GB_h__0(): Block"] if extra_gb else []),
            "method program_start()",
            *([tag] if tag else []),
            "  ensures len(GB_g__0()) == 1",
            "  decreases _",
            "method f__1(p: Ref)",
            Check("  requires true", WF, "internal"),
            "  decreases",
            "{",
            "  var held_GB_g__0: Bool",
            "  var tv_v: Bool",
            "  var held_v: Bool",
            "  var t: Ref",
            "  var v: Ref",
            *(body if body is not None else self.LEGAL),
            Check(f"  assert forperm t: Ref [t.tok] :: {term or self.TERM}", A, "leak"),
            "}",
        ]

    def probs(self, **kw):
        return [p for p in self.strict(self.fixture(**kw)) if "decision 0054" in p]

    def test_the_emitters_shapes_are_clean(self):
        self.assertEqual(self.probs(), [])

    def test_a_loop_invariant_and_row_on_the_cells_nullness_are_clean(self):
        body = [*self.LEGAL, "  var i: Int := 0", "  while (i < 3)",
                Check(f"    invariant held_GB_g__0 == ({self.CELL} != null)", INV, "internal"),
                Check("    decreases 3 - i", "termination.failed", "termination"), "  {", "    i := i + 1",
                f"    if (!(held_GB_g__0 == ({self.CELL} != null))) {{", Check("      assert false", A, "permission"), "    }", "  }"]
        self.assertEqual(self.probs(body=body), [])

    # tags --------------------------------------------------------------------------------------------------------------------------
    def test_a_ghost_of_an_untagged_block_is_a_problem(self):
        probs = self.probs(tag=None)
        self.assertTrue(any("does not tag `// cerastes:owning-global GB_g__0`" in p and "ownership ghost" in p for p in probs), probs)

    def test_a_tag_outside_program_start_or_for_an_undeclared_block_is_a_problem(self):
        lines = self.fixture(tag=None)
        lines.insert(lines.index("method f__1(p: Ref)"), "// cerastes:owning-global GB_g__0")
        self.assertTrue(any("stands only inside `method program_start`" in p for p in self.strict(lines)))
        probs = self.probs(tag="  // cerastes:owning-global GB_h__0")
        self.assertTrue(any("names no single `unique function GB_...`" in p for p in probs), probs)
        probs = self.probs(tag="  // cerastes:owning-global")
        self.assertTrue(any("names no single `unique function GB_...`" in p for p in probs), probs)

    def test_a_tag_twice_or_after_the_first_clause_is_a_problem(self):
        lines = self.fixture()
        lines.insert(lines.index("  // cerastes:owning-global GB_g__0") + 1, "  // cerastes:owning-global GB_g__0")
        self.assertTrue(any("is tagged `// cerastes:owning-global` twice" in p for p in self.strict(lines)))
        lines = self.fixture()
        lines.insert(lines.index("  decreases _"), "  // cerastes:owning-global GB_h__0")
        lines.insert(lines.index("method program_start()") - 1, "unique function GB_h__0(): Block")
        probs = self.strict(lines)
        self.assertTrue(any("right after the head of `method program_start`, before its first clause" in p for p in probs), probs)

    def test_a_block_declared_twice_cannot_be_tagged(self):
        lines = self.fixture()
        lines.insert(3, "unique function GB_g__0(): Block")
        self.assertTrue(any("names no single `unique function GB_...`" in p for p in self.strict(lines)))

    # held_ / tv_ assignments ---------------------------------------------------------------------------------------------------------
    def test_an_owners_flag_never_becomes_true_without_a_read_of_the_token(self):
        for rhs in ("true", "held_v || true", "t != null || held_v", "held_v && (v != null)", "held_v || (t != null)",
                    "held_v || (((t != null) && (offset_of(t) == 1)) && (perm(loc(block_of(t), (-1)).tok) == write))",
                    "held_v || (((t != null) && (offset_of(t) == 0)) && (perm(loc(block_of(t), (-1)).tok) > none))",
                    "held_v || (((t != null) && (offset_of(v) == 0)) && (perm(loc(block_of(t), (-1)).tok) == write))",
                    "held_GB_g__0 && ((v != null) && (perm(loc(block_of(v), (-1)).tok) > none))",
                    "held_v && ((v != null) && (perm(loc(block_of(t), (-1)).tok) > none))",
                    "held_v && ((v != null) && (perm(loc(block_of(v), (-1)).tok) >= none))"):
            with self.subTest(rhs=rhs):
                probs = self.probs(body=[f"  held_v := {rhs}"])
                self.assertEqual(len([p for p in probs if "`held_v` is assigned" in p]), 1, probs)

    def test_the_update_of_a_cell_owner_names_its_own_cell_three_times(self):
        other = self.UPD_GLOBAL.replace("loc(block_of(loc(GB_g__0(), 0).v_ptr_int), (-1)).tok", "loc(block_of(loc(GB_h__0(), 0).v_ptr_int), (-1)).tok")
        self.assertTrue(any("`held_GB_g__0` is assigned" in p for p in self.probs(body=[other])))
        wrong_owner = self.UPD_CELL.replace("held_v :=", "held_w :=").replace("(held_v &&", "(held_w &&")
        self.assertTrue(any("`held_w` is assigned" in p for p in self.probs(body=[wrong_owner])))
        other_global = self.UPD_GLOBAL.replace("held_GB_g__0 :=", "held_GB_h__0 :=").replace("(held_GB_g__0 &&", "(held_GB_h__0 &&")
        self.assertTrue(any("`held_GB_h__0` is assigned" in p for p in self.probs(body=[other_global], extra_gb=True)))

    def test_an_update_or_a_term_without_offset_of_zero_is_a_problem(self):
        # decision 0059 item 10: the token-reading updates and the exemption term of a held owner name a pointer that starts its block
        for what, upd in (("local", self.UPD_LOCAL), ("global", self.UPD_GLOBAL), ("cell", self.UPD_CELL)):
            with self.subTest(update=what):
                old = drop_offset_conjuncts(upd)
                self.assertNotEqual(old, upd)
                probs = self.probs(body=[old])
                self.assertTrue(any("is assigned" in p and "an owner's ghost" in p for p in probs), probs)
        every = lambda **kw: self.strict(self.fixture(**kw))  # the new rule's message is decision 0059's, which `probs` filters out
        probs = every(term=drop_offset_conjuncts(self.TERM))
        self.assertTrue(any("holds `offset_of(" in p and "decision 0059 item 10" in p for p in probs), probs)
        # another pointer's offset does not do
        probs = every(term=f"held_GB_g__0 && ((offset_of(v) == 0) && (t == loc(block_of({self.CELL}), (-1))))")
        self.assertTrue(any("holds `offset_of(" in p for p in probs), probs)

    def test_a_flag_assigned_inside_a_one_line_statement_is_read_too(self):
        probs = self.probs(body=["  if (t != null) { held_v := true }"])
        self.assertTrue(any("`held_v` is assigned `true`" in p for p in probs), probs)

    def test_an_owning_locals_token_value_is_the_conditional_read_of_its_own_block(self):
        for rhs in ("true", "false", "t != null", "(t != null) ? loc(block_of(v), (-1)).tok : tv_v",
                    "(t != null) ? loc(block_of(t), (-1)).tok : true", "(t != null) ? loc(block_of(t), (-1)).tok : tv_w",
                    "(t != null) ? !loc(block_of(t), (-1)).tok : tv_v"):
            with self.subTest(rhs=rhs):
                probs = self.probs(body=[f"  tv_v := {rhs}"])
                self.assertTrue(any("`tv_v` is assigned" in p for p in probs), probs)

    def test_the_ghosts_of_a_global_are_the_flag_only(self):
        probs = self.probs(body=["  tv_GB_g__0 := true"])
        self.assertTrue(any("`tv_GB_g__0` is assigned" in p for p in probs), probs)

    # exemption terms ------------------------------------------------------------------------------------------------------------------
    def test_an_exemption_term_of_a_held_owning_global_names_its_own_block(self):
        other = "held_GB_g__0 && ((offset_of(loc(GB_h__0(), 0).v_ptr_int) == 0) && (t == loc(block_of(loc(GB_h__0(), 0).v_ptr_int), (-1))))"
        probs = self.probs(term=other, extra_gb=True)
        self.assertTrue(any("guarded by `held_GB_g__0`" in p for p in probs), probs)
        probs = self.probs(term="held_GB_g__0 && ((offset_of(v) == 0) && (t == loc(block_of(v), (-1))))")
        self.assertTrue(any("guarded by `held_GB_g__0`" in p for p in probs), probs)
        probs = self.probs(term="held_GB_g__0 && ((offset_of(loc(GB_g__0(), 0).v_int) == 0) && (t == loc(block_of(loc(GB_g__0(), 0).v_int), (-1))))")
        self.assertTrue(any("guarded by `held_GB_g__0`" in p for p in probs), probs)

    def test_the_terms_of_a_nested_disjunction_are_each_read(self):
        def term(n):
            return f"(held_GB_{n}__0 && ((offset_of(loc(GB_{n}__0(), 0).v_ptr_int) == 0) && (t == loc(block_of(loc(GB_{n}__0(), 0).v_ptr_int), (-1)))))"
        lines = self.fixture(extra_gb=True, term=f"(({term('g')} || {term('h')}) || {term('g')})")
        probs = [p for p in self.strict(lines) if "decision 0054" in p]
        self.assertTrue(any("`GB_h__0`, which `program_start` does not tag" in p for p in probs), probs)
        lines = self.fixture(extra_gb=True, term=f"(({term('g')} || {term('g')}) || {term('g')})")
        self.assertEqual([p for p in self.strict(lines) if "decision 0054" in p], [])

    def test_an_untagged_block_in_a_held_exemption_term_is_a_problem(self):
        probs = self.probs(tag=None)
        self.assertTrue(any("exemption term" in p and "does not tag" in p for p in probs), probs)

    def test_a_term_without_a_held_guard_is_an_ordinary_block_exemption(self):
        probs = self.probs(tag=None, term=f"((offset_of({self.CELL}) == 0) && (t == loc(block_of({self.CELL}), (-1))))", body=["  held_v := false"])
        self.assertEqual([p for p in probs if "exemption term" in p], [])

    def test_the_flag_of_a_global_is_compared_only_with_its_cells_nullness(self):
        for inv in (f"    invariant held_GB_g__0 == ({self.CELL} == null)", "    invariant held_GB_g__0 == true",
                    f"    invariant held_GB_g__0 == ({self.CELL} != null) || true",
                    "    invariant held_GB_g__0 == (loc(GB_h__0(), 0).v_ptr_int != null)"):
            with self.subTest(inv=inv):
                body = [*self.LEGAL, "  var i: Int := 0", "  while (i < 3)", Check(inv, INV, "internal"),
                        Check("    decreases 3 - i", "termination.failed", "termination"), "  {", "    i := i + 1", "  }"]
                probs = self.probs(body=body, extra_gb=True)
                self.assertTrue(any("`held_GB_g__0` is compared with something but" in p for p in probs), probs)



class OwningLeafTests(unittest.TestCase):
    """Decision 0055 item 9: the owning leaves of struct objects.  `program_start` tags an owning leaf of a struct global
    `// cerastes:owning-global GB_x/<k>[/<k>...]` (the member path pinned against the global's own row: a pointer leaf of
    that struct; a struct global only by member paths); the ghost `held_GB_x_<k>` of the leaf (a struct local's: `held_<var>_<k>`)
    is assigned only `false`, `x != null`, the leaf's own cell `!= null`, or the framed update that reads the cell's and the
    token's permission, the ghost's name being the one that leaf's cell spells; `main`'s exemption term for it is exactly the
    block of that leaf's cell; a comparison only with that cell's nullness.  Each rule has twins that break exactly it, and the
    pointer-global rules of OwningGlobalTagTests keep their meaning."""

    L0 = "loc(member(loc(GB_g__0(), 0), 0), 0)"        # the owning pointer leaf, path (0)
    L1 = "loc(member(loc(GB_g__0(), 0), 1), 0)"        # an int leaf, path (1)
    CELL = L0 + ".v_ptr_int"
    ROW = (f"  ensures acc({L0}.v_ptr_int) && acc({L0}.d_ptr_int) && {L0}.d_ptr_int && ({L0}.v_ptr_int == null) && "
           f"acc({L1}.v_int) && acc({L1}.d_int) && {L1}.d_int && ({L1}.v_int == 0) && acc(lvloc(GB_g__0()).lv)")
    TERM = f"held_GB_g__0_0 && ((offset_of({CELL}) == 0) && (t == loc(block_of({CELL}), (-1))))"
    TAG = "  // cerastes:owning-global GB_g__0/0"

    @staticmethod
    def upd(ghost, cell):
        return (f"  {ghost} := ({ghost} && (perm({cell}) > none)) && ((({cell} != null) && (offset_of({cell}) == 0)) && "
                f"(perm(loc(block_of({cell}), (-1)).tok) > none))")

    LEGAL = ["  held_GB_g__0_0 := false", "  held_GB_g__0_0 := t != null", f"  held_GB_g__0_0 := {CELL} != null",
             upd.__func__("held_GB_g__0_0", CELL),
             "  held_v_s__0_0 := false", "  held_v_s__0_0 := t != null",
             "  held_v_s__0_0 := loc(member(v_s__0, 0), 0).v_ptr_int != null",
             upd.__func__("held_v_s__0_0", "loc(member(v_s__0, 0), 0).v_ptr_int"),
             upd.__func__("held_v_s__1_0_1", "loc(member(loc(member(v_s__1, 0), 0), 1), 0).v_ptr_int")]

    @staticmethod
    def strict(lines):
        return lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")

    def fixture(self, tags=("  // cerastes:owning-global GB_g__0/0",), body=None, term=None, row=None, extra=()):
        return [
            "field v_ptr_int: Ref",
            "field d_ptr_int: Bool",
            "field v_int: Int",
            "field d_int: Bool",
            "field tok: Bool",
            "unique function GB_g__0(): Block",
            "method program_start()",
            *tags,
            "  ensures len(GB_g__0()) == 1",
            row if row is not None else self.ROW,
            "  decreases _",
            "method f__1(p: Ref)",
            Check("  requires true", WF, "internal"),
            "  decreases",
            "{",
            "  var held_GB_g__0_0: Bool",
            "  var held_v_s__0_0: Bool",
            "  var t: Ref",
            *(body if body is not None else self.LEGAL),
            *extra,
            Check(f"  assert forperm t: Ref [t.tok] :: {term or self.TERM}", A, "leak"),
            "}",
        ]

    def probs(self, **kw):
        return [p for p in self.strict(self.fixture(**kw)) if "decision 0054" in p or "decision 0055" in p]

    def test_the_emitters_shapes_are_clean(self):
        self.assertEqual(self.probs(), [])

    def test_a_deeper_member_path_pinned_against_the_row_is_clean(self):
        leaf = "loc(member(loc(member(loc(GB_g__0(), 0), 0), 0), 2), 0)"
        row = (f"  ensures acc({leaf}.v_ptr_int) && acc({leaf}.d_ptr_int) && {leaf}.d_ptr_int && ({leaf}.v_ptr_int == null) && "
               "acc(lvloc(GB_g__0()).lv)")
        ghost = "held_GB_g__0_0_2"
        body = [f"  {ghost} := false", self.upd(ghost, leaf + ".v_ptr_int")]
        term = f"{ghost} && ((offset_of({leaf}.v_ptr_int) == 0) && (t == loc(block_of({leaf}.v_ptr_int), (-1))))"
        lines = self.fixture(tags=("  // cerastes:owning-global GB_g__0/0/2",), row=row, body=body, term=term)
        lines[lines.index("  var held_GB_g__0_0: Bool")] = f"  var {ghost}: Bool"
        probs = [p for p in self.strict(lines) if "decision 0054" in p or "decision 0055" in p]
        self.assertEqual(probs, [])
        # and the same program with the tag for another path is a problem (the pin reads the tag, not the ghost)
        lines = [l.replace("GB_g__0/0/2", "GB_g__0/0/1") if isinstance(l, str) else l for l in lines]
        self.assertTrue(any("is not a pointer leaf" in p for p in self.strict(lines)))

    # tags ----------------------------------------------------------------------------------------------------------------------------
    def test_a_member_path_must_name_a_pointer_leaf_of_the_globals_row(self):
        for tag in ("GB_g__0/1", "GB_g__0/5", "GB_g__0/0/0", "GB_g__0/0/1"):
            with self.subTest(tag=tag):
                probs = self.probs(tags=(f"  // cerastes:owning-global {tag}",), body=["  held_v_s__0_0 := false"], term="t == t")
                self.assertTrue(any("is not a pointer leaf of the struct global" in p for p in probs), probs)

    def test_a_path_on_a_global_with_no_struct_row_is_a_problem(self):
        probs = self.probs(row="  ensures len(GB_g__0()) == 1", body=["  held_v_s__0_0 := false"], term="t == t")
        self.assertTrue(any("is not a pointer leaf of the struct global" in p for p in probs), probs)

    def test_a_struct_global_is_tagged_only_by_member_paths(self):
        probs = self.probs(tags=("  // cerastes:owning-global GB_g__0",), body=["  held_v_s__0_0 := false"], term="t == t")
        self.assertTrue(any("tags a struct global without a member path" in p for p in probs), probs)

    def test_a_malformed_member_path_is_a_problem(self):
        for tag in ("GB_g__0/a", "GB_g__0/00", "GB_g__0/", "GB_g__0//0", "GB_g__0/0/", "GB_g__0/-1"):
            with self.subTest(tag=tag):
                probs = self.probs(tags=(f"  // cerastes:owning-global {tag}",), body=["  held_v_s__0_0 := false"], term="t == t")
                self.assertTrue(any("the member path is" in p for p in probs), probs)

    def test_a_leaf_tagged_twice_or_a_tag_of_an_undeclared_block_is_a_problem(self):
        probs = self.probs(tags=(self.TAG, self.TAG))
        self.assertTrue(any("is tagged `// cerastes:owning-global` twice" in p for p in probs), probs)
        probs = self.probs(tags=("  // cerastes:owning-global GB_h__0/0",), body=["  held_v_s__0_0 := false"], term="t == t")
        self.assertTrue(any("names no single `unique function GB_...`" in p for p in probs), probs)

    def test_a_leaf_tag_stands_right_after_the_head_of_program_start(self):
        lines = self.fixture(tags=())
        lines.insert(lines.index("  decreases _"), self.TAG)
        probs = [p for p in self.strict(lines) if "decision 0054" in p]
        self.assertTrue(any("right after the head of `method program_start`" in p for p in probs), probs)

    # ghosts ---------------------------------------------------------------------------------------------------------------------------
    def test_the_ghost_of_an_untagged_leaf_is_a_problem(self):
        probs = self.probs(tags=())
        self.assertTrue(any("`held_GB_g__0_0` in a program whose `program_start` tags no" in p for p in probs), probs)

    def test_a_ghost_needs_the_tag_that_spells_it(self):
        probs = self.probs(body=["  held_GB_g__0_1 := false"], term="t == t")
        self.assertTrue(any("`held_GB_g__0_1` in a program whose `program_start` tags no" in p for p in probs), probs)
        probs = self.probs(body=["  held_GB_g__0 := false"], term="t == t")
        self.assertTrue(any("ownership ghost `held_GB_g__0` of `GB_g__0`" in p for p in probs), probs)

    # assignments ---------------------------------------------------------------------------------------------------------------------
    def test_a_leafs_flag_never_becomes_true_without_a_read_of_the_cells_permission(self):
        other = "loc(member(loc(GB_h__0(), 0), 0), 0).v_ptr_int"
        l1 = self.L1 + ".v_ptr_int"
        good = self.upd("held_GB_g__0_0", self.CELL)
        bad = [
            "held_GB_g__0_0 := true",
            "held_GB_g__0_0 := held_GB_g__0_0 || true",
            f"held_GB_g__0_0 := {other} != null",
            f"held_GB_g__0_0 := {l1} != null",
            f"held_GB_g__0_0 := {self.CELL} == null",
            good.replace("held_GB_g__0_0 :=", "held_GB_g__0_1 :=").lstrip().replace("(held_GB_g__0_0 &&", "(held_GB_g__0_1 &&"),
            good.lstrip().replace(f"(perm({self.CELL}) > none)", "true"),
            good.lstrip().replace("(perm(loc(block_of(" + self.CELL + "), (-1)).tok) > none)", "true"),
            good.lstrip().replace("> none", ">= none"),
            good.lstrip().replace(f"(({self.CELL} != null)", f"(({other} != null)"),
            good.lstrip().replace(f"perm({self.CELL})", f"perm({l1})", 1),
            good.lstrip().replace("(held_GB_g__0_0 &&", "(held_GB_g__0_0 ||"),
        ]
        for rhs in bad:
            with self.subTest(rhs=rhs):
                probs = self.probs(body=[f"  {rhs.lstrip()}"])
                self.assertTrue(any("is assigned" in p for p in probs), probs)

    def test_the_framed_update_of_a_struct_locals_leaf_names_its_own_variable_and_path(self):
        cell = "loc(member(v_s__0, 0), 0).v_ptr_int"
        for ghost, c in (("held_v_s__0_0", "loc(member(v_s__1, 0), 0).v_ptr_int"),    # another variable's cell
                         ("held_v_s__0_0", "loc(member(v_s__0, 1), 0).v_ptr_int"),    # another member's cell
                         ("held_v_s__0_1", cell),                                     # the ghost of another member
                         ("held_v_t__0_0", cell),                                     # the ghost of another variable
                         ("held_v_s__0", cell)):                                      # no path in the name
            with self.subTest(ghost=ghost, cell=c):
                probs = self.probs(body=[self.upd(ghost, c)])
                self.assertTrue(any(f"`{ghost}` is assigned" in p for p in probs), probs)
        self.assertEqual(self.probs(body=[self.upd("held_v_s__0_0", cell)]), [])

    # the guarded update of a call through a pointer that may be the owner's struct (decision 0056 item 4(a)) -------------------------
    LCELL = "loc(member(v_s__0, 0), 0).v_ptr_int"

    @staticmethod
    def guarded(ghost, a, r, cell, t="t", read_guard=None, other="null", else_=None, read_t=None, upd_guard=None):
        """The two lines of the guarded update: the conditional read of the leaf's cell, then the ghost's conditional update."""
        guard = read_guard if read_guard is not None else f"({a} == {r}) && (perm({cell}) > none)"
        read = f"  {read_t or t} := ({guard}) ? {cell} : {other}"
        upd = f"  {ghost} := {upd_guard or f'({a} == {r})'} ? ({t} != null) : {else_ or ghost}"
        return [read, upd]

    def guarded_probs(self, lines):
        return self.probs(body=lines)

    def test_the_guarded_update_of_a_struct_locals_and_a_globals_leaf_is_clean(self):
        g = self.guarded
        for lines in (g("held_v_s__0_0", "v_a__1", "v_s__0", self.LCELL),
                      g("held_GB_g__0_0", "v_p__0", "loc(GB_g__0(), 0)", self.CELL),
                      # other statements may stand between the read and the update
                      [g("held_v_s__0_0", "v_a__1", "v_s__0", self.LCELL)[0], "  var_unused := 0",
                       g("held_v_s__0_0", "v_a__1", "v_s__0", self.LCELL)[1]]):
            with self.subTest(lines=lines):
                self.assertEqual(self.guarded_probs(lines), [])

    def test_a_guarded_update_names_the_owners_own_root_address(self):
        g = self.guarded
        for what, lines in (
                ("another struct local", g("held_v_s__0_0", "v_a__1", "v_s__1", self.LCELL)),
                ("another struct local, the read of its own cell", g("held_v_s__0_0", "v_a__1", "v_s__1", "loc(member(v_s__1, 0), 0).v_ptr_int")),
                ("the address of a member, not of the struct", g("held_v_s__0_0", "v_a__1", "loc(member(v_s__0, 0), 0)", self.LCELL)),
                ("another global", g("held_GB_g__0_0", "v_p__0", "loc(GB_h__0(), 0)", self.CELL)),
                ("the global's block, not its slot", g("held_GB_g__0_0", "v_p__0", "GB_g__0()", self.CELL)),
                ("the global's other member", g("held_GB_g__0_0", "v_p__0", "loc(GB_g__0(), 1)", self.CELL)),
                ("a null root", g("held_v_s__0_0", "v_a__1", "null", self.LCELL))):
            with self.subTest(what):
                probs = self.guarded_probs(lines)
                self.assertTrue(probs, probs)

    def test_a_guarded_update_reads_the_cell_under_the_same_guard_and_the_permission(self):
        g = self.guarded
        a, r, c = "v_a__1", "v_s__0", self.LCELL
        for what, lines in (
                ("a read without the root guard", g("held_v_s__0_0", a, r, c, read_guard=f"perm({c}) > none")),
                ("a read without the permission", g("held_v_s__0_0", a, r, c, read_guard=f"{a} == {r}")),
                ("a read under a weaker permission test", g("held_v_s__0_0", a, r, c, read_guard=f"({a} == {r}) && (perm({c}) >= none)")),
                ("a read under a guard of another pointer", g("held_v_s__0_0", a, r, c, read_guard=f"(v_b__2 == {r}) && (perm({c}) > none)")),
                ("a read under a disjunction", g("held_v_s__0_0", a, r, c, read_guard=f"({a} == {r}) || (perm({c}) > none)")),
                ("a read that is not null in the else branch", g("held_v_s__0_0", a, r, c, other="v_x__0")),
                ("a read that is a bare cell", [f"  t := {c}", g("held_v_s__0_0", a, r, c)[1]]),
                ("a read of the value stored beside it", [f"  t := v_x__0", g("held_v_s__0_0", a, r, c)[1]]),
                ("an update guarded by an inequality", g("held_v_s__0_0", a, r, c, upd_guard=f"({a} != {r})")),
                ("an update guarded by another pointer", g("held_v_s__0_0", a, r, c, upd_guard=f"(v_b__2 == {r})")),
                ("an update that keeps something else", g("held_v_s__0_0", a, r, c, else_="false")),
                ("an update that keeps another ghost", g("held_v_s__0_0", a, r, c, else_="held_v_s__0_1")),
                ("an update that becomes true unguarded", ["  held_v_s__0_0 := true"]),
                ("an update that is true under the guard", g("held_v_s__0_0", a, r, c)[:1] + [f"  held_v_s__0_0 := ({a} == {r}) ? true : held_v_s__0_0"]),
                ("an update of the value being null", g("held_v_s__0_0", a, r, c)[:1] + [f"  held_v_s__0_0 := ({a} == {r}) ? (t == null) : held_v_s__0_0"])):
            with self.subTest(what):
                probs = self.guarded_probs(lines)
                self.assertTrue(probs, probs)

    def test_a_guarded_update_needs_the_read_just_before_it(self):
        g = self.guarded
        a, r, c = "v_a__1", "v_s__0", self.LCELL
        read, upd = g("held_v_s__0_0", a, r, c)
        for what, lines in (
                ("no read at all", [upd]),
                ("the read assigned to another temp", [read.replace("t :=", "t2 :=", 1), upd]),
                ("the temp assigned again in between", [read, "  t := null", upd]),
                ("the temp assigned a stored value in between", [read, "  t := v_x__0", upd]),
                ("the read after the update", [upd, read]),
                ("a read under another pointer than the update's", [g("held_v_s__0_0", "v_b__2", r, c)[0], upd]),
                ("a read under another root than the update's", [g("held_v_s__0_0", a, "v_s__1", c)[0], upd])):
            with self.subTest(what):
                probs = self.guarded_probs(lines)
                self.assertTrue(probs, probs)

    def test_a_guarded_update_reads_the_cell_its_ghost_spells(self):
        g = self.guarded
        a, r = "v_a__1", "v_s__0"
        for what, ghost, cell in (
                ("the ghost of another member", "held_v_s__0_1", self.LCELL),
                ("the ghost of another variable", "held_v_t__0_0", self.LCELL),
                ("a ghost with no path", "held_v_s__0", self.LCELL),
                ("another member's cell", "held_v_s__0_0", "loc(member(v_s__0, 1), 0).v_ptr_int"),
                ("a cell that is not a leaf's", "held_v_s__0_0", "loc(v_s__0, 0).v_ptr_int"),
                ("a global's ghost over a local's cell", "held_GB_g__0_0", self.LCELL)):
            with self.subTest(what):
                probs = self.guarded_probs(g(ghost, a, r, cell))
                self.assertTrue(probs, probs)

    def test_the_guarded_update_is_for_a_leaf_ghost_only(self):
        # an owning local's `held_<v>` and a pointer global's `held_GB_x` have no leaf cell to read: the old rule stands
        for ghost, r in (("held_v_p__0", "v_p__0"), ("held_GB_g__0", "loc(GB_g__0(), 0)")):
            with self.subTest(ghost):
                lines = self.guarded(ghost, "v_a__1", r, "loc(GB_g__0(), 0).v_ptr_int")
                self.assertTrue(self.guarded_probs(lines))
        # and a tv_ ghost is not an owner ghost of this kind
        probs = self.probs(body=["  tv_v__0 := (v_a__1 == v_s__0) ? (t != null) : tv_v__0"])
        self.assertTrue(any("is assigned" in p for p in probs), probs)

    def test_a_leaf_cell_is_a_canonical_chain_rooted_at_its_block(self):
        for c in ("loc(member(loc(GB_g__0(), 1), 0), 0).v_ptr_int",       # a chain whose path is not the ghost's
                  "loc(member(loc(GB_g__0(), 0), 0), 0).v_ptr_int + 0",    # arithmetic in the cell
                  "loc(member(block_of(loc(GB_g__0(), 0)), 0), 0).v_ptr_int"):
            with self.subTest(cell=c):
                probs = self.probs(body=[self.upd("held_GB_g__0_0", c)])
                self.assertTrue(any("`held_GB_g__0_0` is assigned" in p for p in probs), probs)

    # exemption terms -----------------------------------------------------------------------------------------------------------------
    def test_an_exemption_term_of_a_held_leaf_names_its_own_leafs_block(self):
        l1 = self.L1 + ".v_ptr_int"
        for cell in (l1, "loc(GB_g__0(), 0).v_ptr_int", "loc(member(loc(GB_h__0(), 0), 0), 0).v_ptr_int",
                     "loc(member(loc(member(loc(GB_g__0(), 0), 0), 0), 0), 0).v_ptr_int", "v_s__0"):
            with self.subTest(cell=cell):
                term = f"held_GB_g__0_0 && ((offset_of({cell}) == 0) && (t == loc(block_of({cell}), (-1))))"
                probs = self.probs(term=term)
                self.assertTrue(any("guarded by `held_GB_g__0_0`" in p for p in probs), probs)
        probs = self.probs(term=f"held_GB_g__0_0 && (t == loc(block_of({self.CELL}), 0))")
        self.assertTrue(any("guarded by `held_GB_g__0_0`" in p for p in probs), probs)

    def test_the_exemption_of_a_leaf_of_an_untagged_block_is_a_problem(self):
        probs = self.probs(tags=())
        self.assertTrue(any("exemption term" in p and "does not tag" in p for p in probs), probs)

    def test_a_pointer_globals_term_still_names_the_globals_own_cell(self):
        # the pointer-global rule is not widened to the leaf's cell: the ghost `held_GB_g__0` spells the whole global
        lines = self.fixture(tags=("  // cerastes:owning-global GB_h__0",), body=["  held_GB_g__0 := false"],
                             term=f"held_GB_g__0 && ((offset_of({self.CELL}) == 0) && (t == loc(block_of({self.CELL}), (-1))))")
        lines.insert(lines.index("unique function GB_g__0(): Block"), "unique function GB_h__0(): Block")
        probs = [p for p in self.strict(lines) if "decision 0054" in p or "decision 0055" in p]
        self.assertTrue(any("`held_GB_g__0`" in p for p in probs), probs)

    # comparisons ---------------------------------------------------------------------------------------------------------------------
    def test_the_flag_of_a_leaf_is_compared_only_with_its_cells_nullness(self):
        def body(inv):
            return [*self.LEGAL, "  var i: Int := 0", "  while (i < 3)", Check(inv, INV, "internal"),
                    Check("    decreases 3 - i", "termination.failed", "termination"), "  {", "    i := i + 1", "  }"]
        good = f"    invariant held_GB_g__0_0 == ({self.CELL} != null)"
        self.assertEqual(self.probs(body=body(good)), [])
        for inv in (f"    invariant held_GB_g__0_0 == ({self.CELL} == null)", "    invariant held_GB_g__0_0 == true",
                    f"    invariant held_GB_g__0_0 == ({self.CELL} != null) || true",
                    f"    invariant held_GB_g__0_0 == ({self.L1}.v_ptr_int != null)",
                    "    invariant held_GB_g__0_0 == (loc(GB_g__0(), 0).v_ptr_int != null)"):
            with self.subTest(inv=inv):
                probs = self.probs(body=body(inv))
                self.assertTrue(any("`held_GB_g__0_0` is compared with something but" in p for p in probs), probs)



class OwningLeafEntryAndExemptionTests(OwningLeafTests):
    """Decision 0055 item 9, the entry and gives-back of a leaf ghost through a temp, and `main`'s deeper exemption (S4).  A leaf
    ghost set from a temp that was just given a leaf's cell is set from the cell its name spells, under `null` as the else branch and,
    where the guard reads a permission, exactly `perm(<that cell>) > none`; an `exm_<n>` is assigned once, in the shape of a held
    ghost (tagged, its own cell) or an earlier `exm_` guarding the cell that one read, and a `forperm` disjunct `exm_<n> && t ==
    loc(block_of(<cell>), -1)` names exactly that cell.  Each rule has twins that break exactly it (the inherited tests run again
    over the same fixture, so the older rules keep their meaning)."""

    L0CELL = OwningLeafTests.CELL
    NEXT0 = f"loc(member({OwningLeafTests.CELL}, 0), 0).v_ptr_int"
    NEXT1 = f"loc(member({NEXT0}, 1), 0).v_ptr_int"
    EXM0 = f"  exm_0 := (held_GB_g__0_0 && ({OwningLeafTests.CELL} != null)) && (perm({NEXT0}) > none)"
    EXM1 = f"  exm_1 := (exm_0 && ({NEXT0} != null)) && (perm({NEXT1}) > none)"
    EXM_TERM = (f"(held_GB_g__0_0 && ((offset_of({L0CELL}) == 0) && (t == loc(block_of({L0CELL}), (-1))))) || (exm_0 && ((offset_of({NEXT0}) == 0) && (t == loc(block_of({NEXT0}), (-1))))) || "
                f"(exm_1 && ((offset_of({NEXT1}) == 0) && (t == loc(block_of({NEXT1}), (-1)))))")
    EXM_VARS = ["  var exm_0: Bool", "  var exm_1: Bool"]

    def body(self, *lines):
        return [*self.EXM_VARS, *lines]

    def exm_probs(self, lines, term=None):
        return self.probs(body=self.body(*lines), term=term or self.EXM_TERM)

    # the temp ---------------------------------------------------------------------------------------------------------------------
    def temp_probs(self, rhs, ghost="held_GB_g__0_0"):
        return [p for p in self.probs(body=[f"  t := {rhs}", f"  {ghost} := t != null"]) if "is set from" in p]

    def test_a_leaf_ghost_set_from_its_own_cell_through_a_temp_is_clean(self):
        c = self.CELL
        for rhs in (c, f"(k != 0) ? {c} : null", f"((((k != 0) ? 1 : 0) != 0) && (!(((m == null) ? 1 : 0) != 0))) ? {c} : null",
                    f"(perm({c}) > none) ? {c} : null", f"perm({c}) > none ? {c} : null", "q", "(k != 0) ? q : null"):
            with self.subTest(rhs=rhs):
                self.assertEqual(self.temp_probs(rhs), [])
        cell = "loc(member(v_s__0, 0), 0).v_ptr_int"
        self.assertEqual(self.temp_probs(f"(perm({cell}) > none) ? {cell} : null", "held_v_s__0_0"), [])

    def test_a_temp_set_from_another_leafs_cell_is_a_problem(self):
        other = self.L1 + ".v_ptr_int"
        for rhs in (other, f"(k != 0) ? {other} : null", f"(perm({other}) > none) ? {other} : null",
                    "loc(member(loc(GB_h__0(), 0), 0), 0).v_ptr_int", "(k != 0) ? loc(member(v_s__0, 0), 0).v_ptr_int : null"):
            with self.subTest(rhs=rhs):
                self.assertTrue(any("another leaf's" in p for p in self.temp_probs(rhs)), self.temp_probs(rhs))
        cell = self.CELL
        self.assertTrue(any("another leaf's" in p for p in self.temp_probs(cell, "held_v_s__0_0")))

    def test_the_else_branch_of_a_leaf_temp_is_null(self):
        c = self.CELL
        for rhs in (f"(k != 0) ? {c} : t", f"(k != 0) ? {c} : q", f"(k != 0) ? {c} : {c}", f"(perm({c}) > none) ? {c} : q"):
            with self.subTest(rhs=rhs):
                self.assertTrue(any("else branch is not `null`" in p for p in self.temp_probs(rhs)), self.temp_probs(rhs))

    def test_a_guard_that_reads_a_permission_reads_the_leafs_own(self):
        c, other = self.CELL, self.L1 + ".v_ptr_int"
        for g in (f"(perm({other}) > none)", f"(perm({c}) >= none)", f"(perm({c}) > write)", f"(perm({c}) > none) || true",
                  f"(perm(loc(GB_h__0(), 0).v_ptr_int) > none)"):
            with self.subTest(guard=g):
                self.assertTrue(any("guard reads a permission" in p for p in self.temp_probs(f"{g} ? {c} : null")), g)

    def test_the_temp_rule_reads_only_the_held_ghosts_of_leaves(self):
        # an unrelated temp, a pointer global's ghost and a local owner's ghost are not constrained by it
        probs = [p for p in self.probs(body=["  t := (k != 0) ? v : null", "  held_v_s__0_0 := t != null"]) if "is set from" in p]
        self.assertEqual(probs, [])

    # exemption ghosts -------------------------------------------------------------------------------------------------------------
    def test_the_deeper_exemption_of_the_emitters_shape_is_clean(self):
        self.assertEqual(self.exm_probs([self.EXM0, self.EXM1]), [])

    def test_an_exm_is_assigned_exactly_once(self):
        self.assertTrue(any("is assigned twice" in p for p in self.exm_probs([self.EXM0, self.EXM0, self.EXM1])))

    def test_an_exm_rooted_at_an_untagged_or_unknown_ghost_is_a_problem(self):
        bad = self.EXM0.replace("held_GB_g__0_0 &&", "held_v_s__0_0 &&")
        self.assertTrue(any("not the ghost of an owner tagged" in p for p in self.exm_probs([bad, self.EXM1])))
        bad = self.EXM0.replace("held_GB_g__0_0 &&", "held_GB_g__0_1 &&")
        self.assertTrue(any("not the ghost of an owner tagged" in p for p in self.exm_probs([bad, self.EXM1])))
        bad = self.EXM0.replace("held_GB_g__0_0 &&", "true &&")
        self.assertTrue(any("`exm_0` is assigned" in p for p in self.exm_probs([bad, self.EXM1])))

    def test_an_exm_guards_its_owners_own_cell(self):
        for cell in (self.L1 + ".v_ptr_int", "loc(GB_g__0(), 0).v_ptr_int", self.NEXT0):
            with self.subTest(cell=cell):
                bad = self.EXM0.replace(f"({self.CELL} != null)", f"({cell} != null)")
                self.assertTrue(any("guards the cell of its own owner" in p for p in self.exm_probs([bad, self.EXM1])))

    def test_the_cell_an_exm_reads_is_a_member_of_the_block_its_guard_points_to(self):
        for nxt in ("loc(GB_h__0(), 0).v_ptr_int", self.CELL, f"loc(member({self.L1}, 0), 0).v_ptr_int",
                    f"loc(member({self.CELL}, 0), 1).v_ptr_int", f"loc(block_of({self.NEXT0}), 0).v_ptr_int"):
            with self.subTest(next=nxt):
                bad = self.EXM0.replace(f"perm({self.NEXT0})", f"perm({nxt})")
                self.assertTrue(any("`exm_0` is assigned" in p and "a member of the block" in p for p in self.exm_probs([bad])), nxt)

    def test_an_exm_is_rooted_through_an_earlier_exm_and_the_cell_it_read(self):
        self.assertTrue(any("is not assigned before it" in p for p in self.exm_probs([self.EXM1, self.EXM0])))
        self.assertTrue(any("is not assigned before it" in p for p in self.exm_probs([self.EXM0, self.EXM1.replace("exm_0 &&", "exm_7 &&")])))
        bad = self.EXM1.replace(f"({self.NEXT0} != null)", f"({self.CELL} != null)")
        self.assertTrue(any("that it read, and no other" in p for p in self.exm_probs([self.EXM0, bad])))

    def test_an_exm_has_the_shape_of_a_guarded_permission_read(self):
        for rhs in (f"(held_GB_g__0_0 && ({self.CELL} != null))", f"(perm({self.NEXT0}) > none)", "true",
                    f"(held_GB_g__0_0 && ({self.CELL} != null)) || (perm({self.NEXT0}) > none)",
                    f"(held_GB_g__0_0 && ({self.CELL} != null)) && (perm({self.NEXT0}) >= none)",
                    f"(held_GB_g__0_0 && ({self.CELL} == null)) && (perm({self.NEXT0}) > none)"):
            with self.subTest(rhs=rhs):
                self.assertTrue(any("`exm_0` is assigned" in p for p in self.exm_probs([f"  exm_0 := {rhs}", self.EXM1])), rhs)

    def test_an_exm_is_read_only_in_the_row_it_frames(self):
        probs = self.exm_probs([self.EXM0, self.EXM1, "  assert exm_0"])
        self.assertTrue(any("is read only in the `forperm` row" in p for p in probs), probs)

    def test_a_disjunct_names_the_cell_its_exm_read(self):
        n0, n1 = self.NEXT0, self.NEXT1
        for term in (self.EXM_TERM.replace(f"exm_0 && ((offset_of({n0}) == 0) && (t == loc(block_of({n0})", f"exm_0 && ((offset_of({n1}) == 0) && (t == loc(block_of({n1})"),
                     self.EXM_TERM.replace(f"exm_1 && ((offset_of({n1}) == 0) && (t == loc(block_of({n1})", f"exm_1 && ((offset_of({n0}) == 0) && (t == loc(block_of({n0})"),
                     self.EXM_TERM.replace(f"exm_0 && ((offset_of({n0}) == 0) && (t == loc(block_of({n0})", f"exm_0 && ((offset_of({self.L0CELL}) == 0) && (t == loc(block_of({self.L0CELL})"),
                     self.EXM_TERM.replace(f"exm_0 && ((offset_of({n0}) == 0) && (t == loc(block_of({n0}), (-1))))", f"exm_0 && (t == loc(block_of({n0}), 0))")):
            with self.subTest(term=term):
                probs = self.exm_probs([self.EXM0, self.EXM1], term=term)
                self.assertTrue(any("the exemption term guarded by `exm_" in p for p in probs), probs)

    def test_an_exm_term_without_offset_of_zero_is_a_problem(self):
        # decision 0059 item 10: the exemption of an exm_<n> names the block of the cell it read only when that cell starts its block
        term = drop_offset_conjuncts(self.EXM_TERM)
        self.assertNotEqual(term, self.EXM_TERM)
        every = lambda t: self.strict(self.fixture(body=self.body(self.EXM0, self.EXM1), term=t))  # `probs` keeps only decisions 0054 and 0055
        probs = every(term)
        self.assertTrue(any("holds `offset_of(" in p and "decision 0059 item 10" in p for p in probs), probs)
        self.assertEqual([p for p in every(self.EXM_TERM) if "decision 0059" in p], [])

    def test_a_disjunct_of_an_exm_never_assigned_is_a_problem(self):
        probs = self.exm_probs([self.EXM0])
        self.assertTrue(any("`exm_1` is read by this exemption row but not assigned" in p for p in probs), probs)

    def test_an_exm_assigned_after_its_row_is_a_problem(self):
        lines = self.fixture(body=self.body(self.EXM0), term=self.EXM_TERM)
        lines.insert(len(lines) - 1, self.EXM1)
        probs = [p for p in self.strict(lines) if "decision 0055" in p]
        self.assertTrue(any("`exm_1` is read by this exemption row but not assigned before it" in p for p in probs), probs)
        lines = self.fixture(body=self.body(self.EXM0, self.EXM1), term=self.EXM_TERM)
        self.assertEqual([p for p in self.strict(lines) if "decision 0055" in p], [])

    def test_an_exm_outside_a_method_is_a_problem(self):
        lines = self.fixture(body=self.body(self.EXM0, self.EXM1), term=self.EXM_TERM)
        lines.insert(0, "  var exm_9: Bool")
        probs = [p for p in self.strict(lines) if "decision 0055" in p]
        self.assertTrue(any("stands only inside a method" in p for p in probs), probs)

    def test_a_pointer_global_roots_an_exemption_at_its_own_cell(self):
        base = OwningGlobalTagTests()
        cell = base.CELL
        nxt = f"loc(member({cell}, 0), 0).v_ptr_int"
        exm0 = f"  exm_0 := (held_GB_g__0 && ({cell} != null)) && (perm({nxt}) > none)"
        term = f"(held_GB_g__0 && ((offset_of({cell}) == 0) && (t == loc(block_of({cell}), (-1))))) || (exm_0 && ((offset_of({nxt}) == 0) && (t == loc(block_of({nxt}), (-1)))))"

        def probs(line, t=term):
            lines = base.fixture(body=["  var exm_0: Bool", *base.LEGAL, line], term=t)
            return [p for p in base.strict(lines) if "decision 0055" in p]
        self.assertEqual(probs(exm0), [])
        self.assertTrue(any("guards the cell of its own owner" in p for p in probs(exm0.replace(f"({cell} != null)", "(loc(GB_h__0(), 0).v_ptr_int != null)"))))
        self.assertTrue(any("not the ghost of an owner tagged" in p for p in probs(exm0.replace("held_GB_g__0 &&", "held_v &&"))))
        self.assertTrue(any("the exemption term guarded by `exm_0`" in p for p in probs(exm0, term.replace(f"exm_0 && ((offset_of({nxt}) == 0) && (t == loc(block_of({nxt})", f"exm_0 && ((offset_of({cell}) == 0) && (t == loc(block_of({cell})"))))



class OwningLeafLoopCarryTests(OwningLeafTests):
    """Decision 0055 item 7 (Part D): the loop carry of an owning struct local's leaf, `invariant held_<var>_<k> == (<leaf cell> != null)`
    and its entry and back-edge rows `if (!(held_<var>_<k> == (<cell> != null))) { assert false }`: the comparison is with the cell of that very
    leaf, the one the ghost's name spells (the global leaf's and the pointer global's comparisons are OwningLeafTests' and
    OwningGlobalTagTests').  A twin changes the cell, the variable, the path, the operator or the right-hand side."""

    SCELL = "loc(member(v_s__0, 0), 0).v_ptr_int"

    def loop(self, inv, row=None, ghost="held_v_s__0_0"):
        body = ["  var v_s__0: Ref", *self.LEGAL, "  var i: Int := 0", "  while (i < 3)", Check(inv, INV, "internal"),
                Check("    decreases 3 - i", "termination.failed", "termination"), "  {", "    i := i + 1"]
        if row is not None:
            body += [f"    if (!({row})) {{", Check("      assert false", A, "permission"), "    }"]
        body += ["  }"]
        return self.probs(body=body)

    def test_the_emitters_loop_carry_of_a_local_leaf_is_clean(self):
        g, c = "held_v_s__0_0", self.SCELL
        self.assertEqual(self.loop(f"    invariant {g} == ({c} != null)", f"{g} == ({c} != null)"), [])

    def test_a_local_leafs_flag_is_compared_only_with_its_own_cells_nullness(self):
        g, c = "held_v_s__0_0", self.SCELL
        other_var = "loc(member(v_t__0, 0), 0).v_ptr_int"
        other_member = "loc(member(v_s__0, 1), 0).v_ptr_int"
        deeper = "loc(member(loc(member(v_s__0, 0), 0), 1), 0).v_ptr_int"
        for rhs in (f"({c} == null)", "true", f"({c} != null) || true", f"({other_var} != null)", f"({other_member} != null)",
                    f"({deeper} != null)", "(v_s__0 != null)", f"(!({c} != null))"):
            for shape in ("    invariant {g} == {rhs}", None):
                with self.subTest(rhs=rhs, shape=shape):
                    if shape:
                        probs = self.loop(shape.format(g=g, rhs=rhs))
                    else:
                        probs = self.loop(f"    invariant {g} == ({c} != null)", f"{g} == {rhs}")
                    self.assertTrue(any(f"`{g}` is compared with something but" in p for p in probs), probs)

    def test_the_ghost_of_another_leaf_compares_with_that_leafs_cell(self):
        probs = self.loop(f"    invariant held_v_s__0_1 == ({self.SCELL} != null)", ghost="held_v_s__0_1")
        self.assertTrue(any("`held_v_s__0_1` is compared with something but" in p for p in probs), probs)
        probs = self.loop(f"    invariant held_v_t__0_0 == ({self.SCELL} != null)")
        self.assertTrue(any("`held_v_t__0_0` is compared with something but" in p for p in probs), probs)

    def test_an_owning_locals_own_comparison_is_not_read_by_the_leaf_rule(self):
        # the owning local of decision 0052 compares its flag with its own value; the rule reads leaf ghosts only
        body = ["  var v_p__0: Ref", "  var held_v_p__0: Bool", "  var i: Int := 0", "  while (i < 3)",
                Check("    invariant held_v_p__0 == (v_p__0 != null)", INV, "internal"),
                Check("    decreases 3 - i", "termination.failed", "termination"), "  {", "    i := i + 1", "  }"]
        self.assertEqual([p for p in self.probs(body=body) if "is compared with something" in p], [])



class OwningLeafEntrySnapshotTests(OwningLeafTests):
    """Decision 0056 item 5: the entry snapshot `hsl_<held>__<n>` of a leaf that `main` reaches only by type.  A `Bool` local assigned once
    before the `while` of its own loop as `<held> && (perm(<cell>) > none)`, `held` and the cell the leaf's own (the ghost's name spells
    it), read only in the pair `(hsl ==> (held == (<cell> != null))) && ((!hsl) ==> (!held))` (as the invariant and as its entry and
    back-edge rows) and as the first conjunct of a guard.  Each rule has twins that break exactly it; the `hp` pair's rules and the older
    comparison rules keep their meaning."""

    GCELL = OwningLeafTests.CELL
    LCELL = "loc(member(v_s__0, 0), 0).v_ptr_int"

    @staticmethod
    def pair(h, g, c):
        return f"({h} ==> ({g} == ({c} != null))) && ((!{h}) ==> (!{g}))"

    def both(self):
        return (("held_GB_g__0_0", self.GCELL), ("held_v_s__0_0", self.LCELL))

    def hsl(self, g, c, assign=None, inv=None, rows=None, name=None, before=(), guard=None, declare=None, loop_assign=False,
            after_loop=False):
        h = name or f"hsl_{g}__0"
        assign_lines = [f"  {h} := {g} && (perm({c}) > none)"] if assign is None else assign
        body = ["  var v_s__0: Ref", *([f"  var {h}: Bool"] if declare is None else declare), *self.LEGAL, *before,
                *([] if loop_assign or after_loop else assign_lines),
                *(rows[0] if rows is not None else [f"  if (!({self.pair(h, g, c)})) {{", Check("    assert false", A, "permission"), "  }"]),
                "  var i: Int := 0", "  while (i < 3)", Check(inv or f"    invariant {self.pair(h, g, c)}", INV, "internal"),
                *([Check(f"    invariant {guard}", INV, "internal")] if guard else []),
                Check("    decreases 3 - i", "termination.failed", "termination"), "  {", "    i := i + 1",
                *(assign_lines if loop_assign else [])]
        for r in (rows[1] if rows is not None else [f"    if (!({self.pair(h, g, c)})) {{"]):
            body += [r, Check("      assert false", A, "permission"), "    }"]
        body += ["  }", *(assign_lines if after_loop else [])]
        return [p for p in self.strict(self.fixture(body=body)) if "decision 0056" in p or "decision 0055" in p or "decision 0054" in p]

    def test_the_emitters_snapshot_of_a_global_and_a_local_leaf_is_clean(self):
        for g, c in self.both():
            with self.subTest(g=g):
                self.assertEqual(self.hsl(g, c), [])

    def test_the_snapshot_is_held_and_the_permission_to_its_own_leafs_cell(self):
        for g, c in self.both():
            h = f"hsl_{g}__0"
            other = (self.L1 + ".v_ptr_int", "loc(member(v_t__0, 0), 0).v_ptr_int", "loc(member(v_s__0, 1), 0).v_ptr_int")
            for rhs in (*[f"{g} && (perm({o}) > none)" for o in other], f"perm({c}) > none", f"{g}", "true",
                        f"{g} && (perm({c}) >= none)", f"{g} || (perm({c}) > none)", f"held_v_t__0_0 && (perm({c}) > none)",
                        f"{g} && (perm({c}) > none) && true", f"({c} != null)"):
                with self.subTest(g=g, rhs=rhs):
                    probs = self.hsl(g, c, assign=[f"  {h} := {rhs}"])
                    self.assertTrue(any(f"`{h}` is assigned" in p for p in probs), probs)

    def test_the_snapshot_name_spells_the_ghost_it_snapshots(self):
        g, c = self.both()[1]
        for h in ("hsl_held_v_s__0_1__0", "hsl_held_v_t__0_0__0", "hsl_x__0", "hsl_held_v_s__0_0", "hsl_held_v_s__0_0__x"):
            with self.subTest(h=h):
                probs = self.hsl(g, c, name=h)
                self.assertTrue(probs, probs)

    def test_a_snapshot_is_assigned_once_before_its_loop(self):
        for g, c in self.both():
            h = f"hsl_{g}__0"
            line = f"  {h} := {g} && (perm({c}) > none)"
            for what, kw, needle in (("twice", dict(assign=[line, line]), "is assigned twice"),
                                     ("never", dict(assign=[]), "never assigned"),
                                     ("in the loop", dict(loop_assign=True), "at or after the `while`"),
                                     ("after the loop", dict(after_loop=True), "at or after the `while`"),
                                     ("for a loop the method does not have", dict(name=f"hsl_{g}__1"), "the method does not have")):
                with self.subTest(g=g, what=what):
                    probs = self.hsl(g, c, **kw)
                    self.assertTrue(any(needle in p for p in probs), probs)

    def test_a_snapshot_is_a_bool_local_that_is_declared(self):
        g, c = self.both()[1]
        h = f"hsl_{g}__0"
        probs = self.hsl(g, c, declare=[f"  var {h}: Ref"])
        self.assertTrue(any("is a `Ref` local" in p for p in probs), probs)
        probs = self.hsl(g, c, declare=[])
        self.assertTrue(any("not declared" in p for p in probs), probs)

    def test_a_global_leafs_snapshot_needs_its_tag(self):
        g, c = "held_GB_g__1_0", "loc(member(loc(GB_g__1(), 0), 0), 0).v_ptr_int"
        probs = self.hsl(g, c)
        self.assertTrue(any(f"`hsl_{g}__0` is assigned" in p for p in probs), probs)

    def test_the_pair_names_the_snapshots_own_ghost_and_cell(self):
        for g, c in self.both():
            h = f"hsl_{g}__0"
            o = self.L1 + ".v_ptr_int"
            pr = self.pair(h, g, c)
            bad_invs = [f"    invariant {self.pair(h, g, o)}",                                   # another cell
                        f"    invariant {self.pair(h, 'held_v_t__0_0', c)}",                     # another ghost
                        f"    invariant {self.pair('hsl_held_v_t__0_0__0', g, c)}",              # another snapshot
                        f"    invariant ({h} ==> ({g} == ({c} != null)))",                       # no `!hsl ==> !held`
                        f"    invariant ({h} ==> ({g} == ({c} == null))) && ((!{h}) ==> (!{g}))",
                        f"    invariant ({h} ==> ({g} == ({c} != null))) && ((!{h}) ==> {g})",
                        f"    invariant (true ==> ({g} == ({c} != null))) && ((!true) ==> (!{g}))",
                        f"    invariant ({pr}) || true"]
            for inv in bad_invs:
                with self.subTest(g=g, inv=inv):
                    probs = self.hsl(g, c, inv=inv)
                    self.assertTrue(probs, probs)

    def test_the_rows_name_one_cell_one_ghost_and_come_after_the_assignment(self):
        for g, c in self.both():
            h = f"hsl_{g}__0"
            o = self.L1 + ".v_ptr_int"
            good = f"if (!({self.pair(h, g, c)})) {{"
            for row in (f"if (!({self.pair(h, g, o)})) {{", good.replace(f"(!{h})", h),
                        f"if (!({self.pair(h, 'held_v_t__0_0', c)})) {{", f"if (!({self.pair(h, g, c)} || true)) {{"):
                with self.subTest(g=g, row=row):
                    probs = self.hsl(g, c, rows=([f"  {row}", Check("    assert false", A, "permission"), "  }"], [f"    {row}"]))
                    self.assertTrue(probs, probs)
        # an entry row that stands before the assignment reads the snapshot too early
        g, c = self.both()[1]
        h = f"hsl_{g}__0"
        assign = [f"  {h} := {g} && (perm({c}) > none)"]
        row = [f"  if (!({self.pair(h, g, c)})) {{", Check("    assert false", A, "permission"), "  }"]
        body = ["  var v_s__0: Ref", f"  var {h}: Bool", *self.LEGAL, *row, *assign, "  var i: Int := 0", "  while (i < 3)",
                Check(f"    invariant {self.pair(h, g, c)}", INV, "internal"),
                Check("    decreases 3 - i", "termination.failed", "termination"), "  {", "    i := i + 1", "  }"]
        probs = [p for p in self.strict(self.fixture(body=body)) if "decision 0056" in p]
        self.assertTrue(any("read before its assignment" in p for p in probs), probs)

    def test_a_snapshot_elsewhere_is_only_the_first_conjunct_of_a_guard(self):
        g, c = self.both()[1]
        h = f"hsl_{g}__0"
        for use in (f"  t := {h} && (v_s__0 != null)", f"  assert ({h} && (v_s__0 != null)) ==> true"):
            with self.subTest(use=use):
                self.assertEqual(self.hsl(g, c, before=[use]), [])
        for use in (f"  assert {h}", f"  assert !{h}", f"  assert (v_s__0 != null) && {h}", f"  assert (v_s__0 != null) || {h}",
                    f"  assert {h} || true", f"  assert ({h}) ==> true", f"  t := {h}"):
            with self.subTest(use=use):
                probs = self.hsl(g, c, before=[use])
                self.assertTrue(any("appears other than as the first conjunct" in p for p in probs), probs)

    def test_a_snapshot_is_for_a_leaf_ghost_named_hsl_only(self):
        g, c = self.both()[1]
        probs = self.hsl(g, c, before=["  assert hsl_x"])
        self.assertTrue(any("is not named `hsl_<held ghost>__<loop>`" in p for p in probs), probs)


class OwningLeafAgreesAndOwnBlockTests(OwningLeafEntryAndExemptionTests):
    """Decision 0055 items 7 and 9, the shapes of the re-audit's fixes.  (1) The loop carry of a leaf ghost, `leaf_held_agrees`: the snapshot
    `hp_<held>__<n> := perm(<cell>) > none` assigned once before the loop, the invariant `(hp ==> (held == (<cell> != null))) && ((!hp) ==>
    (!held))` and the rows `if (!(((perm(<cell>) > none) ==> (held == (<cell> != null))) && ((!(perm(<cell>) > none)) ==> (!held)))) {`, all over
    the leaf's own cell, the one the ghost's name spells; the snapshot is read only there.  (2) `main`'s term for a leaf's own block,
    `exm_0 := held_<tagged leaf> && (perm(<that leaf's cell>) > none)` with the disjunct `exm_0 && ((offset_of(<that cell>) == 0) && (t == loc(block_of(<that cell>), -1)))`, deeper
    `exm_` rooted at it.  Each rule has twins that break exactly it; the older forms keep their rules (the classes this one inherits from)."""

    GCELL = OwningLeafTests.CELL
    LCELL = "loc(member(v_s__0, 0), 0).v_ptr_int"

    @staticmethod
    def inv(hp, g, c):
        return f"    invariant ({hp} ==> ({g} == ({c} != null))) && ((!{hp}) ==> (!{g}))"

    @staticmethod
    def row(g, c):
        return f"    if (!(((perm({c}) > none) ==> ({g} == ({c} != null))) && ((!(perm({c}) > none)) ==> (!{g})))) {{"

    def carry(self, g, c, assign=None, inv=None, rows=None, hp=None, before=()):
        hp = hp or f"hp_{g}__0"
        body = ["  var v_s__0: Ref", f"  var {hp}: Bool", *self.LEGAL, *before,
                *([f"  {hp} := perm({c}) > none"] if assign is None else assign),
                *([self.row(g, c)] if False else []),
                "  var i: Int := 0", "  while (i < 3)", Check(inv or self.inv(hp, g, c), INV, "internal"),
                Check("    decreases 3 - i", "termination.failed", "termination"), "  {", "    i := i + 1"]
        for r in (rows if rows is not None else [self.row(g, c)]):
            body += [r, Check("      assert false", A, "permission"), "    }"]
        body += ["  }"]
        return self.probs(body=body)

    def both(self):
        return (("held_GB_g__0_0", self.GCELL), ("held_v_s__0_0", self.LCELL))

    def test_the_emitters_carry_of_a_global_and_a_local_leaf_is_clean(self):
        for g, c in self.both():
            with self.subTest(g=g):
                self.assertEqual(self.carry(g, c), [])

    def test_the_snapshot_is_the_permission_to_its_own_leafs_cell(self):
        for g, c in self.both():
            hp = f"hp_{g}__0"
            other = (self.L1 + ".v_ptr_int", "loc(member(v_t__0, 0), 0).v_ptr_int", "loc(member(v_s__0, 1), 0).v_ptr_int")
            for rhs in (*[f"perm({o}) > none" for o in other], f"perm({c}) >= none", f"perm({c}) > write", "true", f"perm({c}) > none || true",
                        f"({c} != null)", f"perm({c})"):
                with self.subTest(g=g, rhs=rhs):
                    probs = self.carry(g, c, assign=[f"  {hp} := {rhs}"])
                    self.assertTrue(any(f"`{hp}` is assigned" in p for p in probs), probs)

    def test_the_snapshot_name_spells_the_ghost_it_snapshots(self):
        g, c = self.both()[1]
        for hp in ("hp_held_v_s__0_1__0", "hp_held_v_t__0_0__0", "hp_x__0", "hp_held_v_s__0_0"):
            with self.subTest(hp=hp):
                probs = self.carry(g, c, assign=[f"  {hp} := perm({c}) > none"], hp=hp)
                self.assertTrue(any(f"`{hp}` is assigned" in p for p in probs), probs)

    def test_a_snapshot_is_assigned_once(self):
        for g, c in self.both():
            hp = f"hp_{g}__0"
            probs = self.carry(g, c, assign=[f"  {hp} := perm({c}) > none", f"  {hp} := perm({c}) > none"])
            self.assertTrue(any("is assigned twice" in p for p in probs), probs)

    def test_a_global_leafs_snapshot_needs_its_tag(self):
        g, c = "held_GB_g__1_0", "loc(member(loc(GB_g__1(), 0), 0), 0).v_ptr_int"
        probs = self.carry(g, c)
        self.assertTrue(any(f"`hp_{g}__0` is assigned" in p for p in probs), probs)

    def test_the_invariant_and_rows_compare_the_ghost_with_its_own_cell_under_its_own_snapshot(self):
        for g, c in self.both():
            hp = f"hp_{g}__0"
            o = self.L1 + ".v_ptr_int"
            bad_invs = [self.inv(hp, g, o),                                                    # another cell
                        self.inv("hp_held_v_t__0_0__0", g, c),                                 # another ghost's snapshot
                        f"    invariant ({hp} ==> ({g} == ({c} != null)))",                    # no `!hp ==> !held`
                        f"    invariant ({hp} ==> ({g} == ({c} == null))) && ((!{hp}) ==> (!{g}))",
                        f"    invariant ({hp} ==> ({g} == ({c} != null))) && ((!{hp}) ==> {g})",
                        f"    invariant (true ==> ({g} == ({c} != null))) && ((!true) ==> (!{g}))",
                        f"    invariant ({hp} ==> ({g} == ({c} != null))) && ((!{hp}) ==> (!{g})) || true"]
            for inv in bad_invs:
                with self.subTest(g=g, inv=inv):
                    probs = self.carry(g, c, inv=inv)
                    self.assertTrue(any("is compared with something" in p or "snapshot is read only" in p for p in probs), probs)

    def test_the_rows_name_one_cell_and_one_ghost(self):
        for g, c in self.both():
            o = self.L1 + ".v_ptr_int"
            good = self.row(g, c)
            for row in (self.row(g, o), good.replace(f"(!{g})", f"{g}"), good.replace("> none", ">= none", 1),
                        self.row("held_v_t__0_0", c), good.replace(f"(perm({c}) > none) ==>", f"(perm({o}) > none) ==>", 1)):
                with self.subTest(g=g, row=row):
                    probs = self.carry(g, c, rows=[row])
                    self.assertTrue(any("is compared with something" in p for p in probs), probs)

    def test_a_snapshot_is_read_only_in_its_invariant_and_after_its_assignment(self):
        g, c = self.both()[1]
        hp = f"hp_{g}__0"
        probs = self.carry(g, c, before=[f"  assert {hp}"])
        self.assertTrue(any("snapshot is read only" in p for p in probs), probs)
        body = ["  var v_s__0: Ref", f"  var {hp}: Bool", "  var i: Int := 0", "  while (i < 3)", Check(self.inv(hp, g, c), INV, "internal"),
                Check("    decreases 3 - i", "termination.failed", "termination"), "  {", "    i := i + 1", "  }", f"  {hp} := perm({c}) > none"]
        probs = self.probs(body=body)
        self.assertTrue(any("snapshot is read only" in p or "is compared with something" in p for p in probs), probs)

    # a leaf's own block in main's exemption ---------------------------------------------------------------------------------------
    OWN0 = f"  exm_0 := held_GB_g__0_0 && (perm({GCELL}) > none)"
    OWN_TERM = f"exm_0 && ((offset_of({GCELL}) == 0) && (t == loc(block_of({GCELL}), (-1))))"
    OWN1 = f"  exm_1 := (exm_0 && ({GCELL} != null)) && (perm(loc(member({GCELL}, 0), 0).v_ptr_int) > none)"
    OWN1_CELL = f"loc(member({GCELL}, 0), 0).v_ptr_int"
    OWN_CHAIN_TERM = f"(exm_0 && ((offset_of({GCELL}) == 0) && (t == loc(block_of({GCELL}), (-1))))) || (exm_1 && ((offset_of({OWN1_CELL}) == 0) && (t == loc(block_of({OWN1_CELL}), (-1)))))"

    def test_a_leafs_own_block_and_a_chain_rooted_at_it_are_clean(self):
        self.assertEqual(self.exm_probs([self.OWN0], term=self.OWN_TERM), [])
        self.assertEqual(self.exm_probs([self.OWN0, self.OWN1], term=self.OWN_CHAIN_TERM), [])

    def test_the_leaf_form_needs_a_tagged_leafs_ghost(self):
        for g in ("held_v_s__0_0", "held_GB_g__0", "held_GB_g__0_1", "held_v"):
            with self.subTest(g=g):
                probs = self.exm_probs([self.OWN0.replace("held_GB_g__0_0", g)], term=self.OWN_TERM)
                self.assertTrue(any("`exm_0` is assigned" in p and "owning leaf tagged" in p for p in probs), probs)

    def test_the_leaf_form_reads_the_permission_to_its_own_cell(self):
        for cell in (self.L1 + ".v_ptr_int", "loc(GB_g__0(), 0).v_ptr_int", self.OWN1_CELL, "loc(member(loc(GB_h__0(), 0), 0), 0).v_ptr_int"):
            with self.subTest(cell=cell):
                probs = self.exm_probs([self.OWN0.replace(f"perm({self.GCELL})", f"perm({cell})")], term=self.OWN_TERM)
                self.assertTrue(any("reads the permission to its own leaf's cell" in p for p in probs), probs)
        probs = self.exm_probs([self.OWN0.replace("> none", ">= none")], term=self.OWN_TERM)
        self.assertTrue(any("`exm_0` is assigned" in p for p in probs), probs)

    def test_the_leaf_forms_disjunct_names_that_very_cell(self):
        for cell in (self.OWN1_CELL, self.L1 + ".v_ptr_int", "loc(GB_g__0(), 0).v_ptr_int"):
            with self.subTest(cell=cell):
                probs = self.exm_probs([self.OWN0], term=f"exm_0 && ((offset_of({cell}) == 0) && (t == loc(block_of({cell}), (-1))))")
                self.assertTrue(any("the exemption term guarded by `exm_0`" in p for p in probs), probs)

    def test_a_chain_rooted_at_a_leafs_own_block_guards_that_cell(self):
        bad = self.OWN1.replace(f"({self.GCELL} != null)", f"({self.L1}.v_ptr_int != null)")
        probs = self.exm_probs([self.OWN0, bad], term=self.OWN_CHAIN_TERM)
        self.assertTrue(any("that it read, and no other" in p for p in probs), probs)
        probs = self.exm_probs([self.OWN1, self.OWN0], term=self.OWN_CHAIN_TERM)
        self.assertTrue(any("is not assigned before it" in p for p in probs), probs)


class MarkedKeptRowTests(unittest.TestCase):
    """Decision 0058 item 4, audit B1: the kept-flag row of a marked closure site read through a load is only emitted for
    a tied site, and the return assertion that holds it is one atomic `assert !(!(CLAUSE))`.  Each check has its twin
    (the mutant that must fail) and a shape that must stay clean."""

    E = "loc(member(p_p__0, 0), 0).v_ptr_int"
    ROW = "((old(!({e} == null)) && old({e}.d_int)) ==> {e}.d_int)"

    def row(self, e=None):
        e = e or self.E
        return self.ROW.format(e=e)

    def method(self, ensures_text, assert_text=None):
        lines = ["method m__9(p_p__0: Ref)", Check("  requires p_p__0 != null", WF, "internal"),
                 Check("  ensures " + ensures_text, P, "postcondition"), "  decreases", "{"]
        if assert_text is not None:
            lines.append(Check("  assert " + assert_text, A, "postcondition"))
        lines.append("}")
        return lines

    def probs(self, ensures_text, assert_text=None):
        return [p for p in lint(self.method(ensures_text, assert_text)) if "marked closure" in p or "kept-flag" in p]

    def tied(self, e=None):
        e = e or self.E
        return f"(true && ({e} == old({e}))) && {self.row(e)}"

    def test_a_tied_kept_row_is_clean_wherever_the_tie_stands(self):
        e = self.E
        self.assertEqual(self.probs(self.tied()), [])
        # the tie after the row (a written tie lands at the end of the clause)
        self.assertEqual(self.probs(f"({self.row()} && ({e} == old({e})))"), [])

    def test_an_untied_kept_row_through_a_load_is_refused(self):
        probs = self.probs(f"(true && {self.row()})")
        self.assertTrue(any("is in a clause without" in p and "== old(" in p for p in probs), probs)

    def test_the_tie_must_be_the_one_of_the_rows_pointer(self):
        other = "loc(member(p_p__0, 1), 0).v_ptr_int"
        probs = self.probs(f"(({other} == old({other})) && {self.row()})")
        self.assertTrue(any("is in a clause without" in p for p in probs), probs)
        # twin: the equality of the pointer itself
        self.assertEqual(self.probs(f"(({self.E} == old({self.E})) && {self.row()})"), [])

    def test_a_tie_in_another_clause_does_not_count(self):
        # the tie belongs to the clause that holds the row: a second ensures line with it, beside an untied row
        lines = self.method(f"(true && {self.row()})")
        lines.insert(3, Check(f"  ensures ({self.E} == old({self.E}))", P, "postcondition"))
        probs = [p for p in lint(lines) if "kept-flag" in p]
        self.assertTrue(any("is in a clause without" in p for p in probs), probs)
        # twin: with the tie in the row's own clause the same two lines are clean
        lines = self.method(self.tied())
        lines.insert(3, Check(f"  ensures ({self.E} == old({self.E}))", P, "postcondition"))
        self.assertEqual([p for p in lint(lines) if "kept-flag" in p], [])

    def test_a_formal_or_a_result_needs_no_tie(self):
        # decision 0042 item 5: a formal's pointer cannot change, so its row stands untied
        self.assertEqual(self.probs(f"(true && {self.row('p_p__0')})"), [])
        self.assertEqual(self.probs(f"(true && {self.row('res')})"), [])

    def test_a_member_cell_of_the_pointees_block_needs_the_tie_of_its_pointer(self):
        # a marked field's closure keeps the flags of the cells of its pointee: the cell is loc(member(Q, k), 0), the
        # pointer read through a load is Q, and the tie is Q's (found when the first corpus run flagged these rows)
        cell = f"loc(member({self.E}, 1), 0)"
        row = f"((old(!({self.E} == null)) && old({cell}.d_int)) ==> {cell}.d_int)"
        probs = self.probs(f"(true && {row})")
        self.assertTrue(any("is in a clause without" in p and self.E + " == old(" in p for p in probs), probs)
        self.assertEqual(self.probs(f"(true && ({self.E} == old({self.E})) && {row})"), [])
        # twin: a cell of a formal's own block (member of the parameter) needs no tie
        formal = "loc(member(p_p__0, 1), 0)"
        formal_row = f"((old(!(p_p__0 == null)) && old({formal}.d_int)) ==> {formal}.d_int)"
        self.assertEqual(self.probs(f"(true && {formal_row})"), [])
        # and a nested cell is tied by the innermost pointer's load, at any depth
        nested = f"loc(member(loc(member({self.E}, 0), 0).v_ptr_struct_0_U, 2), 0)"
        inner = f"loc(member({self.E}, 0), 0).v_ptr_struct_0_U"
        nested_row = f"((old(!({inner} == null)) && old({nested}.d_int)) ==> {nested}.d_int)"
        probs = self.probs(f"(true && {nested_row})")
        self.assertTrue(any(inner + " == old(" in p for p in probs), probs)
        self.assertEqual(self.probs(f"(true && ({inner} == old({inner})) && {nested_row})"), [])

    def test_a_row_is_found_in_a_deeper_pointer_too(self):
        deep = self.E + ".v_ptr_int"
        probs = self.probs(f"(true && {self.row(deep)})")
        self.assertTrue(any("is in a clause without" in p for p in probs), probs)
        self.assertEqual(self.probs(self.tied(deep)), [])

    def test_the_return_assertion_with_a_kept_row_is_atomic(self):
        clause = self.tied()
        self.assertEqual(self.probs(clause, f"!(!({clause}))"), [])
        for mutant in (clause, f"!({clause})", f"({clause})", f"!(!(true)) && ({clause})"):
            with self.subTest(mutant=mutant[:40]):
                probs = self.probs(clause, mutant)
                self.assertTrue(any("one atomic `assert !(!(CLAUSE))`" in p for p in probs), probs)

    def test_an_assert_without_a_kept_row_is_left_alone(self):
        self.assertEqual(self.probs(self.tied(), "true"), [])
        self.assertEqual(self.probs(self.tied(), f"{self.E} == old({self.E})"), [])


class NotInvalidatingRowTests(unittest.TestCase):
    """Decision 0058 item 2: the `f__wf` row of `cerastes::not_invalidating`, `ANT ==> (GP ==> GE)`, the converse of the
    existing partner row `ANT' ==> (GE ==> GP)`, found by its map message."""

    NEW_MSG = ("the contract of `f` takes `*p` on a path where it does not return it, but `p` is "
               "`cerastes::not_invalidating`")
    OLD_MSG = "the contract of `f` returns `*p` on a path where its `requires` did not take it"
    REQ = "(p_c__0 != 0) ? (p_p__1 != null) : true"
    ENS = "(p_c__0 > 0) ? (p_p__1 != null) : true"

    def old_row(self, ge="(p_c__0 > 0)", gp="(p_c__0 != 0)", ant=None):
        return Check(f"  assert ({ant or self.REQ}) ==> ({ge} ==> {gp})", A, "permission", self.OLD_MSG)

    def new_row(self, gp="(p_c__0 != 0)", ge="(p_c__0 > 0)", ant=None, name_text=None):
        a = ant or f"(({self.REQ}) && ({self.ENS}))"
        return Check(name_text or f"  assert {a} ==> ({gp} ==> {ge})", A, "permission", self.NEW_MSG)

    def probs(self, *rows, name="f_f__0__wf", base="bodiless"):
        lines = []
        if base == "bodiless":
            lines += ["// cerastes:trusted-contract a contract nothing verifies", "method f_f__0(p_c__0: Int, p_p__1: Ref)", "  decreases _"]
        elif base == "body":
            lines += ["method f_f__0(p_c__0: Int, p_p__1: Ref)", "  decreases", "{", "}"]
        lines += [f"method {name}(p_c__0: Int, p_p__1: Ref)", "  decreases", "{", *rows, "}"]
        return [p for p in lint(lines) if "not_invalidating" in p]

    def test_the_converse_of_the_partner_row_is_clean(self):
        self.assertEqual(self.probs(self.old_row(), self.new_row()), [])

    def test_an_unguarded_requires_site_has_the_consequent_alone(self):
        # twin of the swap check: with no requires guard the consequent is the ensures guard alone, and there is no partner row
        self.assertEqual(self.probs(self.new_row(name_text=f"  assert (({self.REQ}) && ({self.ENS})) ==> (p_c__0 > 0)")), [])

    def test_a_row_that_restates_the_partner_row_is_refused(self):
        probs = self.probs(self.old_row(), self.new_row(gp="(p_c__0 > 0)", ge="(p_c__0 != 0)"))
        self.assertTrue(any("two guards swapped" in p for p in probs), probs)

    def test_a_row_with_no_partner_row_is_refused_for_a_bodiless_contract(self):
        probs = self.probs(self.new_row())
        self.assertTrue(any("two guards swapped" in p for p in probs), probs)

    def test_a_function_with_a_body_has_no_partner_row(self):
        # twin of the check above: the partner row and the tie are a bodiless contract's (decision 0054 item 2), so the
        # row of a function with a body stands alone; its form is still checked
        self.assertEqual(self.probs(self.new_row(), base="body"), [])
        probs = self.probs(self.new_row(name_text="  assert true"), base="body")
        self.assertTrue(any("is not an `assert ANT ==> CONSEQUENT` line" in p for p in probs), probs)
        probs = self.probs(self.new_row(), name="f_f__0", base="body")
        self.assertTrue(any("is not in a `__wf` method" in p for p in probs), probs)

    def test_the_partner_row_must_be_earlier(self):
        probs = self.probs(self.new_row(), self.old_row())
        self.assertTrue(any("two guards swapped" in p for p in probs), probs)

    def test_the_antecedent_holds_the_partner_rows(self):
        probs = self.probs(self.old_row(), self.new_row(ant="(p_c__0 != 5)"))
        self.assertTrue(any("antecedent" in p for p in probs), probs)

    def test_the_row_is_in_a_wf_method(self):
        probs = self.probs(self.old_row(), self.new_row(), name="f_f__0")
        self.assertTrue(any("is not in a `__wf` method" in p for p in probs), probs)
        self.assertEqual(self.probs(self.old_row(), self.new_row(), name="f_g__3__wf"), [])

    def test_the_row_is_an_assert_of_the_form_ant_implies_consequent(self):
        for text in ("  assert true", "  assert (a ==> b) ==> (c ==> d) ==> e"):
            with self.subTest(text=text):
                probs = self.probs(self.old_row(), self.new_row(name_text=text))
                self.assertTrue(any("is not an `assert ANT ==> CONSEQUENT` line" in p for p in probs), probs)

    def test_only_a_row_with_the_marker_message_is_checked(self):
        # twin: the same lines under another message are not this rule's business
        lines = ["method f_f__0__wf(p_c__0: Int, p_p__1: Ref)", "  decreases", "{",
                 Check("  assert (a) ==> (b ==> c)", A, "permission", self.OLD_MSG), "}"]
        self.assertEqual([p for p in lint(lines) if "not_invalidating" in p], [])


MARKED = "assert.failed[insufficient.permission]"
R0059 = "decision 0059"


class ArrayMapReasonTests(unittest.TestCase):
    """Decision 0059 item 7: the second column of a map line is `<id>`, or `<id>[<reason>]` on a marked range row; the id is read bare
    (the kind rules and the duplicate-id rule read it) and the only reason is `insufficient.permission`, only on `assert.failed`."""

    @staticmethod
    def line(ident, vpr_line=3):
        return "\t".join([str(vpr_line), ident, "permission", "t.c", "9", "5", "message"])

    def parse(self, ident):
        return lv.parse_map(self.line(ident) + "\n", "t.vpr.map")

    def test_a_marked_row_is_read_as_its_id_and_its_reason(self):
        entries, problems = self.parse(MARKED)
        self.assertEqual(problems, [])
        self.assertEqual((entries[0].error_id, entries[0].expected_reason), ("assert.failed", "insufficient.permission"))

    def test_an_unmarked_row_has_no_reason(self):
        entries, problems = self.parse("assert.failed")
        self.assertEqual(problems, [])
        self.assertEqual((entries[0].error_id, entries[0].expected_reason), ("assert.failed", ""))

    def test_any_other_reason_is_a_problem(self):
        for ident in ("assert.failed[assertion.false]", "assert.failed[]", "assert.failed[qp.not.injective]",
                      "assert.failed[Insufficient.permission]"):
            with self.subTest(ident=ident):
                entries, problems = self.parse(ident)
                self.assertEqual(entries, [])
                self.assertTrue(any("unknown expected reason" in p for p in problems), problems)

    def test_a_reason_is_only_on_assert_failed(self):
        for ident in ("call.precondition[insufficient.permission]", "postcondition.violated[insufficient.permission]",
                      "invariant.not.preserved[insufficient.permission]"):
            with self.subTest(ident=ident):
                entries, problems = self.parse(ident)
                self.assertEqual(entries, [])
                self.assertTrue(any("only on `assert.failed`" in p for p in problems), problems)

    def test_a_malformed_id_is_unknown(self):
        for ident in ("assert.failed[insufficient.permission", "assert.failed insufficient.permission", "assert.failed[a][b]",
                      "assert.failed[insufficient.permission]x", "[insufficient.permission]", "assert.failed(insufficient.permission)"):
            with self.subTest(ident=ident):
                entries, problems = self.parse(ident)
                self.assertEqual(entries, [])
                self.assertTrue(any("unknown Viper error id" in p for p in problems), problems)

    def test_the_bare_id_still_counts_for_the_duplicate_id_rule(self):
        # two entries with one id on one line are refused whether or not one is marked
        lines = [Check("  assert true", A, "overflow")]
        text, _ = build(["method f__1()", "  decreases", "{", *lines, "}"])
        map_text = self.line(MARKED, 4) + "\n" + self.line("assert.failed", 4) + "\n"
        probs = [p for p in lv.lint_vpr(text, map_text, "t.vpr", "t.vpr.map") if "two entries" in p]
        self.assertEqual(len(probs), 1, probs)


class ArrayAllocationTests(unittest.TestCase):
    """Decision 0059 items 3 and 12: `alloc_arr_<t>` and `zero_arr_<t>` are trusted, so each is pinned clause by clause beside
    `alloc_<t>`'s `len == 1` rule: S43's text for the allocation and the zero fill (the zero literal follows the field's type).
    Each mutant breaks one clause and must be refused; the permission-conjuring ones are the dangerous ones."""

    def fields(self, t="int", ty="Int"):
        return [f"field v_{t}: {ty}", f"field d_{t}: Bool", "field lv: Bool"]

    @staticmethod
    def q(body, hi="n", lo="0"):
        return f"forall k: Int :: {{ loc(block_of(r), k) }} (({lo} <= k) && (k < {hi})) ==> {body}"

    def alloc_clauses(self, t="int"):
        return [("requires", "1 <= n"),
                ("ensures", "((r != null) && (r == loc(block_of(r), 0))) && (len(block_of(r)) == n)"),
                ("ensures", "(lvloc(block_of(r)) == loc(block_of(r), (-2))) && acc(lvloc(block_of(r)).lv)"),
                ("ensures", self.q(f"acc(loc(block_of(r), k).v_{t})")),
                ("ensures", self.q(f"acc(loc(block_of(r), k).d_{t})")),
                ("ensures", self.q(f"(!loc(block_of(r), k).d_{t})"))]

    def zero_clauses(self, t="int", zero="0"):
        return [("requires", "((1 <= n) && (r == loc(block_of(r), 0))) && (len(block_of(r)) == n)"),
                ("requires", self.q(f"acc(loc(block_of(r), k).v_{t})")),
                ("requires", self.q(f"acc(loc(block_of(r), k).d_{t})")),
                ("ensures", self.q(f"acc(loc(block_of(r), k).v_{t})")),
                ("ensures", self.q(f"acc(loc(block_of(r), k).d_{t})")),
                ("ensures", self.q(f"(loc(block_of(r), k).v_{t} == {zero})")),
                ("ensures", self.q(f"loc(block_of(r), k).d_{t}"))]

    def method(self, name, clauses, header=None, tail="  decreases _", t="int", ty="Int", fields=True):
        head = header or (f"method {name}(n: Int) returns (r: Ref)" if name.startswith("alloc_arr_") else f"method {name}(r: Ref, n: Int)")
        body = [Check(f"  {kw} {text}", WF if kw == "requires" else P, "internal") for kw, text in clauses]
        return [*(self.fields(t, ty) if fields else []), "// cerastes:trusted-contract array allocation", head, *body, tail]

    def probs(self, lines):
        return [p for p in lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map") if "decision 0059 item 3" in p]

    def good(self, t="int", ty="Int", zero="0"):
        return (self.method(f"alloc_arr_{t}", self.alloc_clauses(t), t=t, ty=ty)
                + self.method(f"zero_arr_{t}", self.zero_clauses(t, zero), t=t, ty=ty, fields=False))

    def test_the_two_contracts_are_clean_for_each_field_type(self):
        for t, ty, zero in (("int", "Int", "0"), ("bool", "Bool", "false"), ("ptr_int", "Ref", "null")):
            with self.subTest(t=t):
                self.assertEqual(self.probs(self.good(t, ty, zero)), [])

    def test_the_allocation_is_not_judged_by_the_scalar_rule(self):
        probs = lv.lint_vpr(*build(self.good()), "t.vpr", "t.vpr.map")
        self.assertEqual([p for p in probs if "decision 0032 item 4" in p], [], probs)
        # twin: a scalar allocation named like an array's still gets the scalar rule's judgement
        bad = self.method("alloc_arr", self.alloc_clauses(), header="method alloc_arr() returns (r: Ref)")
        self.assertTrue(any("decision 0032 item 4" in p for p in lv.lint_vpr(*build(bad), "t.vpr", "t.vpr.map")))

    def test_n_ge_1_is_the_same_requirement_as_1_le_n(self):
        clauses = self.alloc_clauses()
        clauses[0] = ("requires", "n >= 1")
        self.assertEqual(self.probs(self.method("alloc_arr_int", clauses)), [])

    def mutants(self, base, name, header=None, **kw):
        out = {}
        for i, (kw_, text) in enumerate(base):
            out[f"drop clause {i}: {text[:40]}"] = base[:i] + base[i + 1:]
        out["extra ensures false"] = base + [("ensures", "false")]
        out["extra ensures a fact"] = base + [("ensures", "len(block_of(r)) == 1")]
        out["extra requires"] = base + [("requires", "n < 100")]
        for i, (kw_, text) in enumerate(base):
            if "k < n" in text:
                out[f"range k <= n in clause {i}"] = base[:i] + [(kw_, text.replace("(k < n)", "(k <= n)"))] + base[i + 1:]
            if "0 <= k" in text:
                out[f"range -1 <= k in clause {i}"] = base[:i] + [(kw_, text.replace("(0 <= k)", "(-1 <= k)"))] + base[i + 1:]
            if "{ loc(block_of(r), k) }" in text:
                out[f"no trigger in clause {i}"] = base[:i] + [(kw_, text.replace("{ loc(block_of(r), k) } ", ""))] + base[i + 1:]
        return out

    def test_every_clause_of_the_allocation_is_pinned(self):
        for label, clauses in self.mutants(self.alloc_clauses(), "alloc_arr_int").items():
            with self.subTest(mutant=label):
                self.assertTrue(self.probs(self.method("alloc_arr_int", clauses)), label)

    def test_every_clause_of_the_zero_fill_is_pinned(self):
        # the dangerous ones: an `ensures acc` wider than the `requires acc` (or with none) conjures permission
        for label, clauses in self.mutants(self.zero_clauses(), "zero_arr_int").items():
            with self.subTest(mutant=label):
                self.assertTrue(self.probs(self.method("zero_arr_int", clauses)), label)

    def test_the_zero_literal_follows_the_fields_type(self):
        for t, ty, zero, wrong in (("int", "Int", "0", "1"), ("bool", "Bool", "false", "true"), ("ptr_int", "Ref", "null", "r")):
            with self.subTest(t=t):
                self.assertTrue(self.probs(self.method(f"zero_arr_{t}", self.zero_clauses(t, wrong), t=t, ty=ty)))
        # a Bool field with the Int literal is wrong too
        self.assertTrue(self.probs(self.method("zero_arr_bool", self.zero_clauses("bool", "0"), t="bool", ty="Bool")))

    def test_the_wrapper_is_pinned(self):
        base = self.alloc_clauses()
        for label, kw in (("a body", {"tail": "  decreases _\n{\n}"}),
                          ("no decreases _", {"tail": "  decreases"}),
                          ("a wider header", {"header": "method alloc_arr_int(n: Int, m: Int) returns (r: Ref)"}),
                          ("no result", {"header": "method alloc_arr_int(n: Int)"}),
                          ("an Int result", {"header": "method alloc_arr_int(n: Int) returns (r: Int)"})):
            with self.subTest(mutant=label):
                self.assertTrue(self.probs(self.method("alloc_arr_int", base, **kw)), label)
        zero = self.zero_clauses()
        for label, kw in (("a result", {"header": "method zero_arr_int(r: Ref, n: Int) returns (s: Ref)"}),
                          ("swapped formals", {"header": "method zero_arr_int(n: Int, r: Ref)"})):
            with self.subTest(mutant=label):
                self.assertTrue(self.probs(self.method("zero_arr_int", zero, **kw)), label)

    def test_a_non_clause_line_is_refused(self):
        lines = self.method("alloc_arr_int", self.alloc_clauses())
        lines.insert(len(lines) - 1, "  assert true")
        self.assertTrue(any("not a `requires` or `ensures` clause" in p for p in self.probs(lines)))

    def test_the_fields_must_be_declared(self):
        lines = self.method("alloc_arr_long", self.alloc_clauses("long"), t="int")  # fields declared for int only
        self.assertTrue(any("is not declared" in p for p in self.probs(lines)))
        self.assertEqual(self.probs(self.method("alloc_arr_long", self.alloc_clauses("long"), t="long")), [])

    def test_a_name_that_only_starts_like_the_prefix_is_judged_as_an_allocation(self):
        # `zero_arr_` and `alloc_arr_` followed by anything are the emitter's names: no escape by an odd suffix
        for name in ("zero_arr_int_2", "alloc_arr_in_t"):
            with self.subTest(name=name):
                self.assertTrue(self.probs(self.method(name, [("ensures", "false")])))


class ArrayQuantifierTests(unittest.TestCase):
    """Decision 0059 items 7, 8 and 12: a permission check over a range is a marked row (`assert forall k :: { loc(B, k) } lo <= k &&
    k < hi ==> acc(loc(B, k).f)`, one field, B, lo, hi and any amount reading no heap) whose map entry carries
    `assert.failed[insufficient.permission]`; `forall` stands only in the rows, the range `exhale`, the two trusted contracts and
    item 8's four loop conjuncts; `old[label]` only in two of those, and only for a label defined earlier in the method."""

    B = "block_of(v_a)"

    @staticmethod
    def quant(body, k="k", b="block_of(v_a)", lo="0", hi="3", extra=None):
        ante = f"(({lo} <= {k}) && ({k} < {hi}))"
        if extra:
            ante = f"({ante} && {extra})"
        return f"forall {k}: Int :: {{ loc({b}, {k}) }} {ante} ==> {body}"

    def acc(self, f="v_int", k="k", b="block_of(v_a)", amount=""):
        return f"acc(loc({b}, {k}).{f}{amount})"

    def row(self, text=None, ident=MARKED, kind="assert"):
        return Check(f"  {kind} {text or self.quant(self.acc())}", ident, "permission")

    def method(self, body, loop=None):
        lines = ["method m__1()", "  decreases", "{", "  var v_a: Ref", *body]
        if loop is not None:
            lines += ["  label lh_0", "  while (true)", *loop, "  {", "  }"]
        return lines + ["}"]

    def probs(self, lines, keep=R0059):
        return [p for p in lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map") if keep in p]

    def test_marked_rows_the_range_exhale_and_a_plain_assert_are_clean(self):
        d = self.quant(self.acc("d_int"))
        lines = self.method([self.row(), self.row(d),
                             Check("  assert !(perm(lvloc(block_of(v_a)).lv) != write)", A, "lifetime"),
                             f"  exhale {self.quant(self.acc())}", f"  exhale {d}", "  exhale acc(lvloc(block_of(v_a)).lv)"])
        self.assertEqual(self.probs(lines), [])

    def test_a_row_may_carry_an_amount_that_reads_no_heap(self):
        self.assertEqual(self.probs(self.method([self.row(self.quant(self.acc(amount=", write")))])), [])
        self.assertEqual(self.probs(self.method([self.row(self.quant(self.acc(amount=", 1 / 2")))])), [])
        # twin: an amount that reads the heap is not a marked row
        for amount in (", perm(x.v_int)", ", loc(v_a, 0).v_int", ", old(kv)"):
            with self.subTest(amount=amount):
                probs = self.probs(self.method([self.row(self.quant(self.acc(amount=amount)))]))
                self.assertTrue(any("not a marked range row" in p for p in probs), probs)

    def test_a_marked_entry_on_a_row_is_needed_and_enough(self):
        probs = self.probs(self.method([self.row(ident="assert.failed")]))
        self.assertTrue(any("has no expected reason" in p for p in probs), probs)
        self.assertEqual(self.probs(self.method([self.row()])), [])

    def test_a_marked_entry_on_anything_else_is_a_problem(self):
        for text in ("!(perm(lvloc(block_of(v_a)).lv) != write)", "true", "perm(loc(block_of(v_a), 0).v_int) > none"):
            with self.subTest(text=text):
                probs = self.probs(self.method([Check(f"  assert {text}", MARKED, "permission")]))
                self.assertTrue(any("carries the expected reason" in p for p in probs), probs)
        # an `exhale` of the row's shape is not an assert either
        probs = self.probs(self.method([Check(f"  exhale {self.quant(self.acc())}", MARKED, "permission")]))
        self.assertTrue(any("carries the expected reason" in p for p in probs), probs)

    def test_a_marked_entry_on_a_requires_is_refused_by_the_map(self):
        lines = ["method m__2(v_a: Ref)", Check("  requires true", "not.wellformed[insufficient.permission]", "internal"), "  decreases", "{", "}"]
        text, map_text = build(lines)
        probs = lv.lint_vpr(text, map_text, "t.vpr", "t.vpr.map")
        self.assertTrue(any("only on `assert.failed`" in p for p in probs), probs)

    def test_a_row_must_have_exactly_the_shape(self):
        two = self.quant(self.acc() + " && " + self.acc("d_int"))
        other_k = self.quant(self.acc(k="j"))
        other_b = self.quant(self.acc(b="block_of(v_b)"))
        no_trigger = self.quant(self.acc()).replace("{ loc(block_of(v_a), k) } ", "")
        other_trigger = self.quant(self.acc()).replace("{ loc(block_of(v_a), k) }", "{ offset_of(loc(block_of(v_a), k)) }")
        not_acc = self.quant("loc(block_of(v_a), k).d_int")
        reversed_range = self.quant(self.acc()).replace("((0 <= k) && (k < 3))", "((k < 3) && (0 <= k))")
        implication_lost = f"forall k: Int :: {{ loc(block_of(v_a), k) }} {self.acc()}"
        one_conjunct = self.quant(self.acc()).replace("((0 <= k) && (k < 3))", "(k < 3)")
        for label, text in (("two fields", two), ("another k", other_k), ("another block", other_b), ("no trigger", no_trigger),
                            ("another trigger", other_trigger), ("not an acc", not_acc), ("range reversed", reversed_range),
                            ("no implication", implication_lost), ("one range conjunct", one_conjunct)):
            with self.subTest(mutant=label):
                probs = self.probs(self.method([self.row(text)]))
                self.assertTrue(probs, label)

    def test_the_block_the_bounds_and_the_trigger_read_no_heap(self):
        for label, kw in (("a heap read in B", {"b": "block_of(v_a.v_ptr_int)"}), ("perm in the low bound", {"lo": "perm(x.v_int)"}),
                          ("a field in the high bound", {"hi": "loc(v_a, 0).v_int"}), ("old in the high bound", {"hi": "old(n)"})):
            with self.subTest(mutant=label):
                kw2 = dict(kw)
                body = self.acc(b=kw.get("b", "block_of(v_a)"))
                probs = self.probs(self.method([self.row(self.quant(body, **kw2))]))
                self.assertTrue(any("not a marked range row" in p for p in probs), probs)
        # twin: variables and arithmetic are fine
        self.assertEqual(self.probs(self.method([self.row(self.quant(self.acc(), lo="n - n", hi="v_n + 1"))])), [])

    def test_the_range_exhale_has_the_shape_too(self):
        for label, text in (("two fields", self.quant(self.acc() + " && " + self.acc("d_int"))),
                            ("an amount", self.quant(self.acc(amount=", write"))),
                            ("no trigger", self.quant(self.acc()).replace("{ loc(block_of(v_a), k) } ", ""))):
            with self.subTest(mutant=label):
                self.assertTrue(self.probs(self.method([f"  exhale {text}"])), label)

    def test_forall_stands_nowhere_else(self):
        q = self.quant(self.acc())
        cases = {
            "a requires": ["method m__2(v_a: Ref)", Check(f"  requires {q}", WF, "internal"), "  decreases", "{", "}"],
            "an ensures": ["method m__2(v_a: Ref)", Check(f"  ensures {q}", P, "postcondition"), "  decreases", "{", "}"],
            "an assume-free assert in a conjunction": self.method([Check(f"  assert true && ({q})", A, "check")]),
            "an inhale": self.method([f"  inhale {q} // cerastes:inhale-ok test"]),
            "a function body": ["function f_f(a: Int): Bool", "{", f"  {q}", "}"],
            "a Bool variable": self.method([f"  var b: Bool := {q}"]),
            "a while condition": self.method([], loop=None)[:-1] + [f"  while ({q})", "  {", "  }", "}"],
        }
        for label, lines in cases.items():
            with self.subTest(case=label):
                self.assertTrue(any("`forall` outside the shapes" in p for p in self.probs(lines)), label)

    def test_a_forall_in_domain_block_is_the_axioms_business(self):
        lines = ["// cerastes:domain-ok 0028", "domain Block {", "  function loc(b: Block, i: Int): Ref",
                 "  // cerastes:axiom-ok 0033", "  " + lv.BLOCK_AXIOMS["loc_inv"][1], "}"]
        self.assertEqual(self.probs(lines), [])

    # --- item 8's loop conjuncts and old[label]
    def conjuncts(self):
        cell = "loc(block_of(v_a), k)"
        rng = "((0 <= k) && (k < 3))"
        return {
            "acc": f"    invariant forall k: Int :: {{ {cell} }} {rng} ==> acc({cell}.v_int)",
            "flags": f"    invariant forall k: Int :: {{ {cell} }} ({rng} && old[lh_0]({cell}.d_int)) ==> {cell}.d_int",
            "range": f"    invariant forall k: Int :: {{ {cell} }} ({rng} && {cell}.d_int) ==> "
                     f"(((-2147483648) <= {cell}.v_int) && ({cell}.v_int <= 2147483647))",
            "values": f"    invariant forall k: Int :: {{ {cell} }} {rng} ==> ({cell}.v_int == old[lh_0]({cell}.v_int))",
        }

    def loop(self, *invariants):
        return [Check(i, INV, "internal") for i in invariants]

    def loop_lines(self, *invariants, label=True):
        lines = ["method m__1()", "  decreases", "{", "  var v_a: Ref"]
        if label:
            lines.append("  label lh_0")
        lines += ["  while (true)", *self.loop(*invariants), "  {", "  }", "}"]
        return lines

    def test_the_four_loop_conjuncts_are_clean(self):
        for name, inv in self.conjuncts().items():
            with self.subTest(conjunct=name):
                self.assertEqual(self.probs(self.loop_lines(inv)), [])

    def test_a_loop_conjunct_must_keep_its_shape(self):
        c = self.conjuncts()
        cell, other = "loc(block_of(v_a), k)", "loc(block_of(v_a), 0)"
        mutants = {
            "acc with an amount": c["acc"].replace(".v_int)", ".v_int, kl)"),
            "flags of another cell": c["flags"].replace(f"old[lh_0]({cell}.d_int)", f"old[lh_0]({other}.d_int)"),
            "flags with another consequent": c["flags"].replace(f"==> {cell}.d_int", f"==> {cell}.d_bool"),
            "flags without old": c["flags"].replace("old[lh_0](", "(").replace(".d_int)) ==>", ".d_int)) ==>"),
            "range with a non-literal bound": c["range"].replace("2147483647", "n"),
            "range with one bound": c["range"].replace(f"(((-2147483648) <= {cell}.v_int) && ({cell}.v_int <= 2147483647))",
                                                       f"({cell}.v_int <= 2147483647)"),
            "values of another field": c["values"].replace("== old[lh_0](loc(block_of(v_a), k).v_int)",
                                                           "== old[lh_0](loc(block_of(v_a), k).v_bool)"),
            "values against the current state": c["values"].replace("old[lh_0](loc(block_of(v_a), k).v_int)", "loc(block_of(v_a), k).v_int"),
            "an implication to anything": f"    invariant forall k: Int :: {{ {cell} }} ((0 <= k) && (k < 3)) ==> {cell}.v_int > 0",
            "two extra conjuncts": c["flags"].replace(f"&& old[lh_0]({cell}.d_int))", f"&& old[lh_0]({cell}.d_int) && true)"),
        }
        for label, inv in mutants.items():
            with self.subTest(mutant=label):
                self.assertTrue(self.probs(self.loop_lines(inv)), label)

    def ptr_range(self, consequent=None, field="ptr_int", antecedent_flag=True, cell="loc(block_of(v_a), k)"):
        e = f"{cell}.v_{field}"
        cons = consequent if consequent is not None else lv._view_wf(e)
        flag = f" && {cell}.d_{field}" if antecedent_flag else ""
        return (f"    invariant forall k: Int :: {{ loc(block_of(v_a), k) }} (((0 <= k) && (k < 3)){flag}) ==> ({cons})")

    def test_a_pointer_elements_range_conjunct_is_wf_of_its_value(self):
        # S43 `s43_c_ptr_elem`: ((0 <= k < 3) && E.d_ptr_int) ==> WF(E) with E = loc(B, k).v_ptr_int
        self.assertEqual(self.probs(self.loop_lines(self.ptr_range())), [])
        e = "loc(block_of(v_a), k).v_ptr_int"
        mutants = {
            "no null alternative": f"(({e} == loc(block_of({e}), offset_of({e}))) && ((0 <= offset_of({e})) && (offset_of({e}) <= len(block_of({e})))))",
            "not a WF": f"{e} != null",
            "true": "true",
            "WF of another cell": lv._view_wf("loc(block_of(v_a), 0).v_ptr_int"),
            "a strict upper bound": lv._view_wf(e).replace(f"offset_of({e}) <= len", f"offset_of({e}) < len"),
            "no lower bound": lv._view_wf(e).replace(f" && (0 <= offset_of({e}))", ""),
        }
        for label, cons in mutants.items():
            with self.subTest(mutant=label):
                self.assertTrue(self.probs(self.loop_lines(self.ptr_range(cons))), label)
        # the antecedent must hold the flag, and WF is only the range of a pointer element
        self.assertTrue(self.probs(self.loop_lines(self.ptr_range(antecedent_flag=False))))
        self.assertTrue(self.probs(self.loop_lines(self.ptr_range(field="int"))))

    def test_old_label_stands_only_in_two_conjuncts(self):
        c = self.conjuncts()
        cell = "loc(block_of(v_a), k)"
        # not in the permission or the range conjunct
        for name in ("acc", "range"):
            inv = c[name].replace(".v_int)", ".v_int)", 1) + " && old[lh_0](true)"
            with self.subTest(conjunct=name):
                self.assertTrue(any("`old[label]` outside" in p for p in self.probs(self.loop_lines(inv))))
        # not in an assert, an ensures or a plain statement
        for lines in (self.method([Check(f"  assert old[lh_0]({cell.replace('k', '0')}.v_int) == 1", A, "check")]),
                      ["method m__2()", Check("  ensures old[lh_0](x) == 1", P, "postcondition"), "  decreases", "{", "}"],
                      self.method(["  v_a := old[lh_0](v_a)"])):
            self.assertTrue(any("`old[label]` outside" in p for p in self.probs(lines)), lines)
        # twin: the plain `old(e)` of a postcondition is not this rule's
        self.assertEqual(self.probs(["method m__2(x: Int)", Check("  ensures x == old(x)", P, "postcondition"), "  decreases", "{", "}"]), [])

    def test_the_label_must_be_defined_earlier_in_the_method(self):
        c = self.conjuncts()
        probs = self.probs(self.loop_lines(c["flags"], label=False))
        self.assertTrue(any("no earlier `label lh_0`" in p for p in probs), probs)
        # a label of another method does not count
        lines = ["method m__0()", "  decreases", "{", "  label lh_0", "}"] + self.loop_lines(c["flags"], label=False)
        self.assertTrue(any("no earlier `label lh_0`" in p for p in self.probs(lines)))
        # twin: with the label before the loop it is clean
        self.assertEqual(self.probs(self.loop_lines(c["flags"])), [])
        # a label after the loop does not count either
        lines = self.loop_lines(c["flags"], label=False)
        lines.insert(len(lines) - 1, "  label lh_0")
        self.assertTrue(any("no earlier `label lh_0`" in p for p in self.probs(lines)))


class AggregateArrayAllocationTests(unittest.TestCase):
    """Decision 0064 items 3, 6 and 17: `alloc_arr_<E>` and `zero_arr_<E>` of an array whose element is a structure are trusted, so they are pinned by
    their clause sets, read from the clauses themselves (the layout is not in the name): one quantifier of length facts with the slot of element `k`
    as its trigger, then per leaf of the element three (the allocation) or six (the zero fill) quantified rows with the leaf receiver as trigger, each
    (receiver, field) once.  Each mutant breaks one thing and must be refused with its own reason; the two dangerous ones are a permission the zero
    fill gives and never took, and a leaf the length facts do not state."""

    SLOT = "loc(block_of(r), k)"
    ROOT = "((r != null) && (r == loc(block_of(r), 0))) && (len(block_of(r)) == n)"
    LIVE = "(lvloc(block_of(r)) == loc(block_of(r), (-2))) && acc(lvloc(block_of(r)).lv)"
    ZERO_ROOT = "((1 <= n) && (r == loc(block_of(r), 0))) && (len(block_of(r)) == n)"

    def recv(self, chain):
        x = self.SLOT
        for j in chain:
            x = f"loc(member({x}, {j}), 0)"
        return x

    def member(self, chain):
        return f"member({self.recv(chain[:-1])}, {chain[-1]})"

    @staticmethod
    def q(trigger, body, hi="n", lo="0", k="k"):
        return f"forall {k}: Int :: {{ {trigger} }} (({lo} <= {k}) && ({k} < {hi})) ==> {body}"

    def lens(self, members, facts=()):
        parts = [f"(len({self.member(m)}) == 1)" for m in members] + [f"({a} != {b})" for a, b in facts]
        return self.q(self.SLOT, " && ".join(parts))

    def leaf_rows(self, chain, t="int"):
        r = self.recv(chain)
        return [("ensures", self.q(r, f"acc({r}.v_{t})")), ("ensures", self.q(r, f"acc({r}.d_{t})")), ("ensures", self.q(r, f"(!{r}.d_{t})"))]

    def alloc(self, leaves=((0,), (1,)), members=None, facts=(), t="int"):
        members = members if members is not None else [c for c in leaves]
        out = [("requires", "1 <= n"), ("ensures", self.ROOT), ("ensures", self.lens(members, facts))]
        for c in leaves:
            out += self.leaf_rows(c, t)
        return out + [("ensures", self.LIVE)]

    def zero(self, leaves=((0,), (1,)), t="int", zero="0"):
        out = [("requires", self.ZERO_ROOT)]
        for c in leaves:
            r = self.recv(c)
            out += [("requires", self.q(r, f"acc({r}.v_{t})")), ("requires", self.q(r, f"acc({r}.d_{t})")),
                    ("ensures", self.q(r, f"acc({r}.v_{t})")), ("ensures", self.q(r, f"acc({r}.d_{t})")),
                    ("ensures", self.q(r, f"({r}.v_{t} == {zero})")), ("ensures", self.q(r, f"{r}.d_{t}"))]
        return out

    def method(self, name, clauses, header=None, tail="  decreases _", fields=(("int", "Int"),), declare=True):
        head = header or (f"method {name}(n: Int) returns (r: Ref)" if name.startswith("alloc_arr_") else f"method {name}(r: Ref, n: Int)")
        body = [Check(f"  {kw} {text}", WF if kw == "requires" else P, "internal") for kw, text in clauses]
        decls = [x for t, ty in fields for x in (f"field v_{t}: {ty}", f"field d_{t}: Bool")] + ["field lv: Bool"] if declare else []
        return [*decls, "// cerastes:trusted-contract array allocation", head, *body, tail]

    def probs(self, lines):
        return [p for p in lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map") if "decision 0064 items 3 and 6" in p]

    def refused(self, lines, reason):
        probs = self.probs(lines)
        self.assertTrue(any(reason in p for p in probs), (reason, probs))

    def clean(self, lines):
        self.assertEqual(lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map"), [])

    NESTED = dict(leaves=((0, 0), (1,)), members=[(0,), (0, 0), (1,)])

    def nested(self, **kw):
        a, b = self.recv(()), self.recv((0,))
        return self.alloc(facts=[(a, b)], **self.NESTED, **kw)

    def test_the_allocation_is_clean_for_each_shape_of_element(self):
        self.clean(self.method("alloc_arr_P", self.alloc()))
        self.clean(self.method("alloc_arr_P", self.alloc(leaves=((0,),))))
        self.clean(self.method("alloc_arr_P", self.alloc(leaves=((0,), (1,), (2,), (3,)))))
        # a structure member that is itself a structure: its own member blocks and the slot fact of decision 0043 item 4
        self.clean(self.method("alloc_arr_P", self.nested()))
        for t, ty in (("bool", "Bool"), ("ptr_int", "Ref")):
            self.clean(self.method("alloc_arr_P", self.alloc(t=t), fields=((t, ty),)))

    def test_the_order_of_the_clauses_is_not_part_of_the_contract(self):
        clauses = self.alloc()
        self.clean(self.method("alloc_arr_P", list(reversed(clauses))))

    def test_the_zero_fill_is_clean_and_agrees_with_its_allocation(self):
        both = self.method("alloc_arr_P", self.alloc()) + self.method("zero_arr_P", self.zero(), declare=False)
        self.clean(both)
        self.clean(self.method("zero_arr_P", self.zero()))
        self.clean(self.method("alloc_arr_P", self.nested()) + self.method("zero_arr_P", self.zero(leaves=((0, 0), (1,))), declare=False))
        for t, ty, zero in (("bool", "Bool", "false"), ("ptr_int", "Ref", "null")):
            self.clean(self.method("zero_arr_P", self.zero(t=t, zero=zero), fields=((t, ty),)))

    def test_every_clause_of_the_allocation_is_needed(self):
        base = self.alloc()
        reasons = {0: "a clause is missing", 1: "a clause is missing", 2: "a clause is missing", len(base) - 1: "a clause is missing"}
        for i, (kw, text) in enumerate(base):
            with self.subTest(dropped=i):
                self.refused(self.method("alloc_arr_P", base[:i] + base[i + 1:]), reasons.get(i, "lacks its"))

    def test_a_clause_given_twice_is_refused(self):
        base = self.alloc()
        for i, reason in ((0, "twice"), (1, "twice"), (2, "twice"), (3, "twice"), (4, "twice"), (5, "twice"), (len(base) - 1, "twice")):
            with self.subTest(clause=i):
                self.refused(self.method("alloc_arr_P", base + [base[i]]), reason)

    def test_the_leaves_are_the_leaves_the_length_facts_state(self):
        # a leaf with permissions and no member block (a permission to a cell the facts never give a block): the dangerous direction
        self.refused(self.method("alloc_arr_P", self.alloc(leaves=((0,), (1,)), members=[(0,)])), "are not the leaves")
        # a member block the facts state and no permission for it
        self.refused(self.method("alloc_arr_P", self.alloc(leaves=((0,),), members=[(0,), (1,)])), "are not the leaves")
        # a nested structure's member without its slot fact, and the slot fact alone
        a, b = self.recv(()), self.recv((0,))
        self.refused(self.method("alloc_arr_P", self.alloc(**self.NESTED)), "the length facts")
        self.refused(self.method("alloc_arr_P", self.alloc(facts=[(a, b)])), "the length facts")

    def test_the_length_facts_state_one_slot_each_and_nothing_else(self):
        for label, lens in (("a count of 4 (an array member: Part E)", self.q(self.SLOT, f"(len({self.member((0,))}) == 4) && (len({self.member((1,))}) == 1)")),
                            ("a count of 0", self.q(self.SLOT, f"(len({self.member((0,))}) == 0) && (len({self.member((1,))}) == 1)")),
                            ("a member twice", self.q(self.SLOT, f"(len({self.member((0,))}) == 1) && (len({self.member((0,))}) == 1) && (len({self.member((1,))}) == 1)")),
                            ("an extra fact", self.q(self.SLOT, f"(len({self.member((0,))}) == 1) && (len({self.member((1,))}) == 1) && (n > 5)")),
                            ("a member of another block", self.q(self.SLOT, f"(len({self.member((0,))}) == 1) && (len(member(loc(block_of(s), k), 1)) == 1)"))):
            with self.subTest(mutant=label):
                clauses = self.alloc()
                clauses[2] = ("ensures", lens)
                self.refused(self.method("alloc_arr_P", clauses), "the length facts")

    def test_the_length_facts_stand_in_an_ensures_over_the_slot_only(self):
        clauses = self.alloc()
        self.refused(self.method("alloc_arr_P", clauses[:2] + [("requires", clauses[2][1])] + clauses[3:]),
                     "is not `1 <= n`")  # a `requires` of the allocation is only `1 <= n`
        # the facts over a range other than the leaves' (the same range in every clause)
        wide = self.alloc()
        wide[2] = ("ensures", self.q(self.SLOT, wide[2][1].split("==> ")[1], hi="n + 1"))
        self.refused(self.method("alloc_arr_P", wide), "is not one of the clauses")

    def test_a_quantified_clause_has_the_range_and_the_binder_of_the_record(self):
        base = self.alloc()
        r = self.recv((0,))
        for label, text, reason in (
                ("k <= n", self.q(r, f"acc({r}.v_int)").replace("(k < n)", "(k <= n)"), "is not one of the clauses"),
                ("-1 <= k", self.q(r, f"acc({r}.v_int)").replace("(0 <= k)", "(-1 <= k)"), "is not one of the clauses"),
                ("a literal count", self.q(r, f"acc({r}.v_int)", hi="3"), "over `0 <= k && k < n`"),
                ("a binder j", self.q(r, f"acc({r}.v_int)", k="j").replace("loc(block_of(r), k)", "loc(block_of(r), j)"), "binder `k`"),
                ("a start at 1", self.q(r, f"acc({r}.v_int)", lo="1"), "over `0 <= k && k < n`"),
                ("no trigger", self.q(r, f"acc({r}.v_int)").replace(f"{{ {r} }} ", ""), "is not one of the clauses"),
                ("a second binder", self.q(r, f"acc({r}.v_int)").replace("forall k: Int ::", "forall k: Int, j: Int ::"), "is not one of the clauses")):
            with self.subTest(mutant=label):
                self.refused(self.method("alloc_arr_P", base[:3] + [("ensures", text)] + base[4:]), reason)

    def test_the_trigger_the_receiver_and_the_cell_are_one_term(self):
        base = self.alloc()
        r0, r1 = self.recv((0,)), self.recv((1,))
        for label, text, reason in (
                ("an acc of another leaf than the trigger", self.q(r0, f"acc({r1}.v_int)"), "the consequent is not one of the leaf's rows"),
                ("an acc of the slot's own cell", self.q(r0, "acc(loc(block_of(r), k).v_int)"), "the consequent is not one of the leaf's rows"),
                ("a trigger rooted at another variable", self.q(r0.replace("block_of(r)", "block_of(s)"), f"acc({r0}.v_int)"), "the trigger is not the slot"),
                ("a trigger with arithmetic", self.q(r0.replace("k)", "k + 1)"), f"acc({r0}.v_int)"), "the trigger is not the slot"),
                ("a trigger with a variable member index", self.q(r0.replace("), 0), 0)", "), j), 0)"), f"acc({r0}.v_int)"), "the trigger is not the slot"),
                ("a trigger that skips the cell step", self.q(self.member((0,)), f"acc({r0}.v_int)"), "the trigger is not the slot"),
                ("a flag that is not negated", self.q(r0, f"{r0}.d_int"), "the consequent is not one of the leaf's rows"),
                ("a value row", self.q(r0, f"({r0}.v_int == 0)"), "the consequent is not one of the leaf's rows"),
                ("a fraction", self.q(r0, f"acc({r0}.v_int, 1 / 2)"), "the consequent is not one of the leaf's rows")):
            with self.subTest(mutant=label):
                self.refused(self.method("alloc_arr_P", base[:3] + [("ensures", text)] + base[4:]), reason)

    def test_a_leaf_row_in_the_requires_of_an_allocation_is_refused(self):
        r0 = self.recv((0,))
        self.refused(self.method("alloc_arr_P", self.alloc() + [("requires", self.q(r0, f"acc({r0}.v_int)"))]), "is not `1 <= n`")

    def test_each_leaf_has_one_field_suffix_and_it_is_declared(self):
        base = self.alloc()
        r0 = self.recv((0,))
        mixed = base[:4] + [("ensures", self.q(r0, f"acc({r0}.d_long)"))] + base[5:]
        self.refused(self.method("alloc_arr_P", mixed, fields=(("int", "Int"), ("long", "Int"))), "more than one field suffix")
        self.refused(self.method("alloc_arr_P", self.alloc(t="long"), fields=(("int", "Int"),)), "is not declared")
        self.clean(self.method("alloc_arr_P", self.alloc(t="long"), fields=(("long", "Int"),)))

    def test_extra_contract_is_refused(self):
        base = self.alloc()
        for label, extra in (("ensures false", ("ensures", "false")), ("an ensures fact", ("ensures", "len(block_of(r)) == 1")),
                             ("a bigger requires", ("requires", "n < 100")), ("an old", ("ensures", "n == old(n)"))):
            with self.subTest(mutant=label):
                self.refused(self.method("alloc_arr_P", base + [extra]), "is not")

    def test_the_wrapper_is_pinned(self):
        base = self.alloc()
        for label, kw, reason in (("a body", {"tail": "  decreases _\n{\n}"}, "has a body"),
                                  ("no decreases _", {"tail": "  decreases"}, "does not end with `decreases _`"),
                                  ("a wider header", {"header": "method alloc_arr_P(n: Int, m: Int) returns (r: Ref)"}, "its header is not"),
                                  ("no result", {"header": "method alloc_arr_P(n: Int)"}, "its header is not"),
                                  ("an Int result", {"header": "method alloc_arr_P(n: Int) returns (r: Int)"}, "its header is not")):
            with self.subTest(mutant=label):
                self.refused(self.method("alloc_arr_P", base, **kw), reason)
        lines = self.method("alloc_arr_P", base)
        lines.insert(len(lines) - 1, "  assert true")
        self.refused(lines, "not a `requires` or `ensures` clause")

    def test_a_clause_without_a_member_is_the_scalar_rules(self):
        # no `member(` in any clause: the scalar rule judges it (and refuses a name whose fields are not declared, decision 0059 item 3)
        scalar = [c for c in ArrayAllocationTests().alloc_clauses("int")]
        lines = self.method("alloc_arr_P", scalar)
        probs = [p for p in lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map") if "decision 0059 item 3" in p]
        self.assertTrue(any("is not declared" in p for p in probs), probs)
        self.assertEqual([p for p in probs if "decision 0064" in p], [])

    def test_every_clause_of_the_zero_fill_is_needed(self):
        base = self.zero(leaves=((0,),))
        reasons = {0: "the `requires` of the root facts is missing"}
        for i in range(len(base)):
            with self.subTest(dropped=i):
                self.refused(self.method("zero_arr_P", base[:i] + base[i + 1:]), reasons.get(i, "lacks its"))

    def test_the_zero_fill_cannot_give_a_permission_it_did_not_take(self):
        base = self.zero(leaves=((0,),))
        r1 = self.recv((1,))
        # an `ensures acc` of a second leaf with no `requires acc`: it would conjure the permission
        self.refused(self.method("zero_arr_P", base + [("ensures", self.q(r1, f"acc({r1}.v_int)"))]), "lacks its")
        # the same with every row of the second leaf in the ensures only
        more = base + [("ensures", self.q(r1, f"acc({r1}.v_int)")), ("ensures", self.q(r1, f"acc({r1}.d_int)")),
                       ("ensures", self.q(r1, f"({r1}.v_int == 0)")), ("ensures", self.q(r1, f"{r1}.d_int"))]
        self.refused(self.method("zero_arr_P", more), "lacks its `requires acc_v`")

    def test_the_zero_literal_follows_the_fields_type(self):
        for t, ty, zero, wrong in (("int", "Int", "0", "1"), ("bool", "Bool", "false", "true"), ("ptr_int", "Ref", "null", "r")):
            with self.subTest(t=t):
                self.refused(self.method("zero_arr_P", self.zero(t=t, zero=wrong), fields=((t, ty),)), "the zero literal")
        self.refused(self.method("zero_arr_P", self.zero(t="bool", zero="0"), fields=(("bool", "Bool"),)), "the zero literal")

    def test_a_zero_fill_states_the_leaves_of_its_allocation(self):
        alloc = self.method("alloc_arr_P", self.alloc(leaves=((0,), (1,))))
        for leaves in (((0,),), ((0,), (1,), (2,)), ((0,), (2,))):
            with self.subTest(leaves=leaves):
                probs = [p for p in lv.lint_vpr(*build(alloc + self.method("zero_arr_P", self.zero(leaves=leaves), declare=False)), "t.vpr", "t.vpr.map")
                         if "not the leaves of" in p]
                self.assertTrue(any("not the leaves of `alloc_arr_P`" in p for p in probs), probs)

    def test_the_zero_fill_has_its_own_header_and_no_length_facts(self):
        base = self.zero()
        self.refused(self.method("zero_arr_P", base, header="method zero_arr_P(r: Ref, n: Int) returns (s: Ref)"), "its header is not")
        self.refused(self.method("zero_arr_P", base, header="method zero_arr_P(n: Int, r: Ref)"), "its header is not")
        self.refused(self.method("zero_arr_P", base + [("ensures", self.lens([(0,), (1,)]))]), "the length facts stand in the `ensures` of an allocation only")


class LeafRowTests(unittest.TestCase):
    """Decision 0064 item 17: the rows of a local array of structures in code.  A row over a LEAF has the receiver `loc(member(<chain>, j), 0)` of the
    canonical chain rooted at the slot `loc(block_of(X), k)` of element `k` (X a variable) for its trigger and for the cell of its `acc`; it is a marked
    `assert` row (with its expected reason), a range `exhale` or one of item 8's loop conjuncts, and nowhere else (a contract's atom rows and the pure
    rows of a call are the part that lifts a range atom over structures).  The older rows keep their rules."""

    SLOT = "loc(block_of(v_a), k)"

    def recv(self, chain=(0,), block="block_of(v_a)", k="k"):
        x = f"loc({block}, {k})"
        for j in chain:
            x = f"loc(member({x}, {j}), 0)"
        return x

    def quant(self, body, chain=(0,), trigger=None, lo="0", hi="3", k="k", block="block_of(v_a)", binders=None):
        return (f"forall {binders or f'{k}: Int'} :: {{ {trigger or self.recv(chain, block, k)} }} (({lo} <= {k}) && ({k} < {hi})) ==> {body}")

    def acc(self, f="v_int", chain=(0,), amount="", block="block_of(v_a)"):
        return f"acc({self.recv(chain, block)}.{f}{amount})"

    def method(self, body):
        return ["method m__1()", "  decreases", "{", "  var v_a: Ref", *body, "}"]

    def row(self, text=None, ident=MARKED, kind="assert"):
        return Check(f"  {kind} {text or self.quant(self.acc())}", ident, "permission")

    def probs(self, lines, keep=R0059):
        return [p for p in lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map") if keep in p]

    def refused(self, lines, reason=None):
        probs = self.probs(lines)
        self.assertTrue(probs and (reason is None or any(reason in p for p in probs)), (reason, probs))

    def test_marked_rows_and_the_range_exhale_over_leaves_are_clean(self):
        rows = [self.row(self.quant(self.acc(f, c), c)) for c in ((0,), (1,), (0, 1)) for f in ("v_int", "d_int")]
        exhales = [f"  exhale {self.quant(self.acc(f, c), c)}" for c in ((0,), (1,)) for f in ("v_int", "d_int")]
        lines = self.method([*rows, Check("  assert !(perm(lvloc(block_of(v_a)).lv) != write)", A, "lifetime"), *exhales, "  exhale acc(lvloc(block_of(v_a)).lv)"])
        self.assertEqual(lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map"), [])

    def test_a_row_may_carry_an_amount_and_a_heap_free_range(self):
        self.assertEqual(self.probs(self.method([self.row(self.quant(self.acc(amount=", write")))])), [])
        self.assertEqual(self.probs(self.method([self.row(self.quant(self.acc(), lo="offset_of(t4)", hi="offset_of(t4) + t5"))])), [])
        for label, kw in (("a heap read in the low bound", {"lo": "loc(t4, 0).v_int"}), ("perm in the high bound", {"hi": "perm(x.v_int)"})):
            with self.subTest(mutant=label):
                self.refused(self.method([self.row(self.quant(self.acc(), **kw))]), "not a marked range row")
        for amount in (", perm(x.v_int)", ", loc(v_a, 0).v_int"):
            with self.subTest(amount=amount):
                self.refused(self.method([self.row(self.quant(self.acc(amount=amount)))]), "not a marked range row")

    def test_a_marked_entry_on_a_leaf_row_is_needed_and_enough(self):
        probs = self.probs(self.method([self.row(ident="assert.failed")]))
        self.assertTrue(any("has no expected reason" in p for p in probs), probs)
        self.assertEqual(self.probs(self.method([self.row()])), [])

    def test_a_marked_entry_on_a_leaf_row_that_is_no_acc_is_a_problem(self):
        flag = self.quant(self.recv() + ".d_int")
        probs = self.probs(self.method([Check(f"  assert {flag}", MARKED, "permission")]))
        self.assertTrue(any("carries the expected reason" in p for p in probs), probs)

    def test_the_receiver_is_the_canonical_chain_rooted_at_the_slot(self):
        r0 = self.recv()
        for label, text in (
                ("a cell of another leaf under the trigger", self.quant(self.acc("v_int", (1,)), trigger=r0)),
                ("the slot as trigger and a leaf cell", self.quant(self.acc(), trigger=self.SLOT)),
                ("a trigger that stops at the member block", self.quant(self.acc(), trigger=f"member({self.SLOT}, 0)")),
                ("a variable member index", self.quant(self.acc()).replace("loc(member(loc(block_of(v_a), k), 0), 0)", "loc(member(loc(block_of(v_a), k), j), 0)")),
                ("an offset in the slot", self.quant(self.acc()).replace("k), 0), 0)", "k + 1), 0), 0)")),
                ("a block that is not a variable", self.quant(self.acc(block="block_of(f(v_a))"), block="block_of(f(v_a))")),
                ("two roots", self.quant(self.acc()).replace("loc(member(loc(block_of(v_a), k), 0), 0).", "loc(member(loc(block_of(v_b), k), 0), 0).")),
                ("a second binder", self.quant(self.acc(), binders="k: Int, j: Int")),
                ("no trigger", self.quant(self.acc()).replace(f"{{ {r0} }} ", "")),
                ("a trigger of another term", self.quant(self.acc(), trigger=f"offset_of({r0})"))):
            with self.subTest(mutant=label):
                self.refused(self.method([self.row(text)]))
                self.refused(self.method([f"  exhale {text}"]))

    def test_a_leaf_row_has_exactly_the_shape_of_a_marked_row(self):
        two = self.quant(self.acc() + " && " + self.acc("d_int"))
        not_acc = self.quant(self.recv() + ".d_int")
        reversed_range = self.quant(self.acc()).replace("((0 <= k) && (k < 3))", "((k < 3) && (0 <= k))")
        one_conjunct = self.quant(self.acc()).replace("((0 <= k) && (k < 3))", "(k < 3)")
        extra = self.quant(self.acc()).replace("((0 <= k) && (k < 3))", "(((0 <= k) && (k < 3)) && true)")
        guarded = f"(x > 0) ==> {self.quant(self.acc())}"
        for label, text in (("two fields", two), ("not an acc", not_acc), ("range reversed", reversed_range), ("one range conjunct", one_conjunct),
                            ("an extra antecedent", extra), ("a guard", guarded)):
            with self.subTest(mutant=label):
                self.refused(self.method([self.row(text)]))

    def test_the_range_exhale_has_the_shape_too(self):
        for label, text in (("two fields", self.quant(self.acc() + " && " + self.acc("d_int"))),
                            ("an amount", self.quant(self.acc(amount=", write"))),
                            ("a flag", self.quant(self.recv() + ".d_int"))):
            with self.subTest(mutant=label):
                self.refused(self.method([f"  exhale {text}"]))

    def test_a_leaf_row_stands_nowhere_else(self):
        q = self.quant(self.acc())
        cases = {
            "a requires": ["method m__2(v_a: Ref)", Check(f"  requires {q}", WF, "internal"), "  decreases", "{", "}"],
            "an ensures": ["method m__2(v_a: Ref)", Check(f"  ensures {q}", P, "postcondition"), "  decreases", "{", "}"],
            "a pure assert": self.method([Check(f"  assert {self.quant(self.recv() + '.d_int')}", A, "check")]),
            "an assert in a conjunction": self.method([Check(f"  assert true && ({q})", A, "check")]),
            "an inhale": self.method([f"  inhale {q} // cerastes:inhale-ok test"]),
            "a Bool variable": self.method([f"  var b: Bool := {q}"]),
        }
        for label, lines in cases.items():
            with self.subTest(case=label):
                self.refused(lines, "`forall` outside the shapes")
        # the contract's and the pure rows say whose they are
        self.refused(cases["a requires"], "a row over a leaf of an array of structures")
        self.refused(cases["a pure assert"], "a row over a leaf of an array of structures")

    # --- item 8's loop conjuncts over a leaf
    def conjuncts(self, chain=(0,)):
        cell = self.recv(chain)
        rng = "((0 <= k) && (k < 3))"
        return {
            "acc": f"    invariant forall k: Int :: {{ {cell} }} {rng} ==> acc({cell}.v_int)",
            "acc flag": f"    invariant forall k: Int :: {{ {cell} }} {rng} ==> acc({cell}.d_int)",
            "flags": f"    invariant forall k: Int :: {{ {cell} }} ({rng} && old[lh_0]({cell}.d_int)) ==> {cell}.d_int",
            "range": f"    invariant forall k: Int :: {{ {cell} }} ({rng} && {cell}.d_int) ==> "
                     f"(((-2147483648) <= {cell}.v_int) && ({cell}.v_int <= 2147483647))",
            "values": f"    invariant forall k: Int :: {{ {cell} }} {rng} ==> ({cell}.v_int == old[lh_0]({cell}.v_int))",
        }

    def loop_lines(self, *invariants, label=True):
        lines = ["method m__1()", "  decreases", "{", "  var v_a: Ref"]
        if label:
            lines.append("  label lh_0")
        lines += ["  while (true)", *[Check(i, INV, "internal") for i in invariants], "  {", "  }", "}"]
        return lines

    def test_the_loop_conjuncts_over_a_leaf_are_clean(self):
        for chain in ((0,), (1,), (0, 1)):
            for name, inv in self.conjuncts(chain).items():
                with self.subTest(chain=chain, conjunct=name):
                    self.assertEqual(self.probs(self.loop_lines(inv)), [])
        self.assertEqual(self.probs(self.loop_lines(*self.conjuncts().values()), keep=""), [p for p in self.probs(self.loop_lines(*self.conjuncts().values()), keep="`while` loop without")])

    def test_a_loop_conjunct_over_a_leaf_must_keep_its_shape(self):
        c = self.conjuncts()
        cell, other = self.recv(), self.recv((1,))
        mutants = {
            "acc with an amount": c["acc"].replace(".v_int)", ".v_int, kl)"),
            "acc of another leaf than the trigger": c["acc"].replace(f"acc({cell}.v_int)", f"acc({other}.v_int)"),
            "flags of another leaf": c["flags"].replace(f"old[lh_0]({cell}.d_int)", f"old[lh_0]({other}.d_int)"),
            "flags with another consequent": c["flags"].replace(f"==> {cell}.d_int", f"==> {other}.d_int"),
            "range of another leaf": c["range"].replace(f"{cell}.v_int", f"{other}.v_int"),
            "range with a non-literal bound": c["range"].replace("2147483647", "n"),
            "values of another leaf": c["values"].replace(f"old[lh_0]({cell}.v_int)", f"old[lh_0]({other}.v_int)"),
            "values against the current state": c["values"].replace(f"old[lh_0]({cell}.v_int)", f"{cell}.v_int"),
            "an implication to anything": f"    invariant forall k: Int :: {{ {cell} }} ((0 <= k) && (k < 3)) ==> {cell}.v_int > 0",
            "the slot as trigger": c["acc"].replace(f"{{ {cell} }}", f"{{ {self.SLOT} }}"),
        }
        for label, inv in mutants.items():
            with self.subTest(mutant=label):
                self.assertTrue(self.probs(self.loop_lines(inv)), label)

    def test_old_label_over_a_leaf_stands_only_in_two_conjuncts(self):
        c = self.conjuncts()
        for name in ("acc", "range"):
            inv = c[name] + " && old[lh_0](true)"
            with self.subTest(conjunct=name):
                self.assertTrue(any("`old[label]` outside" in p for p in self.probs(self.loop_lines(inv))))
        probs = self.probs(self.loop_lines(c["flags"], label=False))
        self.assertTrue(any("no earlier `label lh_0`" in p for p in probs), probs)

    def test_the_older_rows_keep_their_rules(self):
        # the scalar element's marked row and loop conjunct are the old shapes, with the old receiver
        old = f"forall k: Int :: {{ loc(block_of(v_a), k) }} ((0 <= k) && (k < 3)) ==> acc(loc(block_of(v_a), k).v_int)"
        self.assertEqual(self.probs(self.method([self.row(old)])), [])
        # twin: a scalar row whose cell is a leaf cell is neither
        mixed = f"forall k: Int :: {{ loc(block_of(v_a), k) }} ((0 <= k) && (k < 3)) ==> {self.acc()}"
        self.refused(self.method([self.row(mixed)]))


class QuantifiedLoopHeaderTests(unittest.TestCase):
    """A loop over an array has quantified invariants whose trigger sets `{ loc(b, k) }` stand in the loop header: the rule that every
    `while` has a `decreases` clause (decision 0013 item 5) must not take a trigger's `{` for the body's (it did, as a false positive
    on every array loop of Part C), and must still see a missing clause."""

    TRIGGERED = ["    invariant forall k: Int :: { loc(b, k) } ((0 <= k) && (k < 3)) ==> acc(loc(b, k).v_int)",
                 "    invariant forall k: Int :: { loc(b, k) } { loc(b, k + 1) } ((0 <= k) && (k < 3)) ==> acc(loc(b, k).d_int)"]

    def lines(self, with_decreases):
        out = ["method m__1(b: Block)", "  decreases", "{", "  var i: Int := 0", "  while (i < 3)"]
        out += [Check(t, INV, "internal") for t in self.TRIGGERED]
        if with_decreases:
            out.append(Check("    decreases 3 - i", "termination.failed", "termination"))
        return out + ["  {", "    i := i + 1", "  }", "}"]

    def probs(self, with_decreases):
        return [p for p in lv.lint_vpr(*build(self.lines(with_decreases)), "t.vpr", "t.vpr.map") if "`while` loop without a `decreases`" in p]

    def test_trigger_sets_in_the_header_do_not_hide_the_decreases_clause(self):
        self.assertEqual(self.probs(True), [])

    def test_a_missing_decreases_is_still_found_behind_trigger_sets(self):
        probs = self.probs(False)
        self.assertEqual(len(probs), 1, probs)


class CarriedTermFamilyTests(unittest.TestCase):
    """Decision 0059 item 10 restates the exemption terms of the heap's owners with `offset_of(P) == 0`, and leaves the loop-carried terms of
    decisions 0050 and 0056 as they were (reached only through owners, whose updates hold it).  A term without the conjunct is accepted only as
    exactly one of three families over a pointer variable (`temp`, `kt`, `replaced`); a carried shape that is none of them is refused."""

    LOOP_ROW = "an iteration of this loop may end while it still owns an allocation that the loop does not carry to the next iteration"
    END_ROW = "`p` may return while it still owns an allocation that its caller does not hand back"

    def probs(self, body, message=None):
        """The `forperm` problems of a leak row with this exemption: by default the row of a loop's back edge (where the lowering emits
        the carried terms), or the row with `message` (a function's end)."""
        lines = HeapTokenTests("test_the_emitters_shape_is_clean").fixture(
            body=[Check("  assert forperm t: Ref [t.tok] :: " + body, A, "leak", message or self.LOOP_ROW)])
        return [p for p in lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map") if "forperm" in p and "offset_of" in p]

    T = "(t == loc(block_of({p}), (-1)))"

    def term(self, p="v"):
        return self.T.format(p=p)

    def test_each_family_is_clean(self):
        t = self.term
        for label, body in (
                ("temp", f"t__9 && (t == loc(block_of(t__8), (-1)))"),
                ("kt", f"((v != null) && (kt_h0__0 > none)) && {t()}"),
                ("kt with an alias guard", f"(((v != null) && (block_of(v) != block_of(w))) && (kt_h0__0 > none)) && {t()}"),
                ("kt with two alias guards", f"((((v != null) && (block_of(v) != block_of(w))) && (block_of(v) != block_of(u))) && (kt_h0__0 > none)) && {t()}"),
                ("kt with a loop alias flag", f"(((v != null) && ((!ad_h1_w__0) && (!(((v != null) && (w != null)) && (block_of(v) == block_of(w)))))) && (kt_h1__0 > none)) && {t()}"),
                ("replaced", f"(v != null) && {t()}"),
                ("replaced with a nullness guard", f"((!(v == null)) && (v != null)) && {t()}"),
                ("replaced with the ternary guard", f"((!(((v == null) ? 1 : 0) != 0)) && (v != null)) && {t()}"),
                ("a disjunction of two families", f"(t__9 && (t == loc(block_of(t__8), (-1)))) || (((v != null) && (kt_h0__0 > none)) && {t()})")):
            with self.subTest(family=label):
                self.assertEqual(self.probs(body), [], label)

    def test_a_carry_shape_that_is_not_a_family_is_refused(self):
        t = self.term
        for label, body in (
                ("a temp guard on a non-temp pointer", "t__9 && (t == loc(block_of(res), (-1)))"),
                ("a non-temp guard on a temp pointer", "w && (t == loc(block_of(t__8), (-1)))"),
                ("a held ghost as the guard", f"held_v && {t()}"),
                ("a temp guard with a second guard", f"(t__9 && (v != null)) && {t()}"),
                ("kt without the nullness of its pointer", f"((w != null) && (kt_h0__0 > none)) && {t()}"),
                ("kt with no nullness at all", f"(kt_h0__0 > none) && {t()}"),
                ("two kt compares", f"(((v != null) && (kt_h0__0 > none)) && (kt_h0__1 > none)) && {t()}"),
                ("kt compared with something else", f"((v != null) && (kt_h0__0 >= none)) && {t()}"),
                ("kt with an alias guard on another pointer", f"(((v != null) && (block_of(w) != block_of(u))) && (kt_h0__0 > none)) && {t()}"),
                ("kt with an unknown guard", f"(((v != null) && (n > 0)) && (kt_h0__0 > none)) && {t()}"),
                ("kt with an alias flag of another shape", f"(((v != null) && (!ad_h1)) && (kt_h0__0 > none)) && {t()}".replace("!ad_h1", "ad_h1")),
                ("replaced with an unknown guard", f"((v != null) && (n > 0)) && {t()}"),
                ("replaced with the nullness of another pointer", f"((!(w == null)) && (v != null)) && {t()}"),
                ("a carry shape over a chain pointer", "((loc(member(s, 0), 0).v_ptr_int != null) && (kt_h0__0 > none)) && (t == loc(block_of(loc(member(s, 0), 0).v_ptr_int), (-1)))"),
                ("a bare term", t()),
                ("a guard that is not about the pointer", f"(w != null) && {t()}")):
            with self.subTest(shape=label):
                probs = self.probs(body)
                self.assertTrue(any("loop-carry families" in p for p in probs), (label, probs))

    def test_a_family_is_accepted_only_in_the_row_of_a_loops_back_edge(self):
        # the exact shape of a restated ensures-site term with its offset_of conjunct dropped by a regression: `[g &&] P != null && t == loc(block_of(P), -1)`
        # for an identifier pointer is text-identical to the `replaced` family, so only the row's origin tells them apart (the map message)
        t = self.term
        shapes = (f"(res != null) && {t('res')}", f"(p_x__0 != null) && {t('p_x__0')}",
                  f"((!(res == null)) && (res != null)) && {t('res')}",
                  f"((!(((res == null) ? 1 : 0) != 0)) && (res != null)) && {t('res')}",
                  f"(held_v && (res != null)) && {t('res')}",
                  "t__9 && (t == loc(block_of(t__8), (-1)))",
                  f"((v != null) && (kt_h0__0 > none)) && {t()}")
        for body in shapes:
            with self.subTest(shape=body):
                self.assertTrue(any("loop's back edge" in p or "loop-carry families" in p for p in self.probs(body, self.END_ROW)), body)
        # twins: the family shapes are clean in the loop row (the rest of the family tests), and a restated term is clean in the end row
        for body in (f"(res != null) && {t('res')}", f"((v != null) && (kt_h0__0 > none)) && {t()}"):
            self.assertEqual(self.probs(body, None), [], body)
        self.assertEqual(self.probs(f"((res != null) && (offset_of(res) == 0)) && {t('res')}", self.END_ROW), [])
        # a map entry that is not the loop row's, or no map entry, keeps the term refused
        text, _ = build(HeapTokenTests("test_the_emitters_shape_is_clean").fixture(
            body=[Check("  assert forperm t: Ref [t.tok] :: " + f"(res != null) && {t('res')}", A, "leak")]))
        probs = [p for p in lv.lint_vpr(text, None, "t.vpr", "t.vpr.map") if "offset_of" in p]
        self.assertTrue(probs, probs)

    def test_a_family_is_not_a_way_around_the_restated_terms(self):
        # the restated families keep their conjunct: a term over an expression pointer or with a ghost guard needs it even next to a carry term
        t = self.term
        body = f"(held_v && {t()}) || ((v != null) && {t()})"
        self.assertEqual(len(self.probs(body)), 1, self.probs(body))


class ElementFlagTests(unittest.TestCase):
    """Decision 0059 item 12: the flag of an array element is set through a `Ref` temporary, `t.v_<t> := ...` then `t.d_<t> := true`;
    the old flag rule (decision 0019 item 5, 0032 item 11) already reads that shape.  Writing it through `loc(block_of(a), i)` gets its
    own message (S43's hand-written fixtures do, so they are not lint-clean by design)."""

    def lines(self, body):
        return ["method m__1()", "  decreases", "{", "  var v_a: Ref", "  var t__1: Ref", *body, "}"]

    def probs(self, body):
        return [p for p in lv.lint_vpr(*build(self.lines(body)), "t.vpr", "t.vpr.map") if "flag" in p]

    CELL = "loc(block_of(v_a), offset_of(v_a) + 1)"

    def test_a_flag_through_a_temporary_is_clean(self):
        body = [f"  t__1 := {self.CELL}", "  t__1.v_int := 2", "  t__1.d_int := true"]
        self.assertEqual(self.probs(body), [])

    def test_a_flag_through_the_element_address_has_its_own_message(self):
        for body in ([f"  {self.CELL}.v_int := 2", f"  {self.CELL}.d_int := true"],
                     [f"  t__1 := {self.CELL}", "  t__1.v_int := 2", f"  {self.CELL}.d_int := true"]):
            probs = self.probs(body)
            self.assertTrue(any("through a `Ref` temporary" in p and "decision 0059 item 12" in p for p in probs), probs)

    def test_a_flag_through_a_temporary_still_needs_the_write_before_it(self):
        probs = self.probs([f"  t__1 := {self.CELL}", "  t__1.d_int := true"])
        self.assertTrue(any("does not directly follow a write" in p for p in probs), probs)
        probs = self.probs([f"  t__1 := {self.CELL}", "  t__1.v_int := 2", "  t__1.d_int := false"])
        self.assertTrue(any("is assigned `false`" in p for p in probs), probs)

    def test_a_flag_of_a_global_cell_keeps_its_own_receiver(self):
        body = ["  loc(GB_g(), 0).v_int := 2", "  loc(GB_g(), 0).d_int := true"]
        self.assertEqual([p for p in self.probs(body) if "through a `Ref` temporary" in p], [])


class RangeAtomQuantifierTests(unittest.TestCase):
    """Decision 0060 item 16 (S44's shapes): a range atom's quantifiers in a method's `requires` and `ensures`, under an optional guard
    `g ==> forall k :: { loc(block_of(X), k) } offset_of(X) <= k && k < offset_of(X) + n ==> ROW`; the pure `assert` rows over a range
    (the flags, the type's range, the flag implication, `old(flag) ==> flag`); the marked rows over a `Ref` local's block and inside
    `if (g) { }`; the loop conjuncts with the method's own amount and a guard; the amount of a call row.  Nothing else that holds a
    `forall` is admitted: every twin changes one thing of a clean fixture, and the clean fixtures show the shapes pass."""

    INT_RANGE = "(((-2147483648) <= {c}.v_int) && ({c}.v_int <= 2147483647))"

    @staticmethod
    def cell(x="v_p", k="k"):
        return f"loc(block_of({x}), {k})"

    def quant(self, cons, x="v_p", n="v_n", k="k", lo=None, hi=None, trigger=None, extra=None, binder=None, wrap=False):
        lo = f"offset_of({x})" if lo is None else lo
        hi = f"offset_of({x}) + {n}" if hi is None else hi
        ante = f"(({lo} <= {k}) && ({k} < {hi}))"
        if extra:
            ante = f"({ante} && {extra})"
        trigger = f"loc(block_of({x}), {k})" if trigger is None else trigger
        text = f"forall {binder or k + ': Int'} :: {{ {trigger} }} {ante} ==> {cons}"
        return f"({text})" if wrap else text

    def acc(self, f="v_int", amount="", x="v_p", k="k"):
        return f"acc({self.cell(x, k)}.{f}{amount})"

    def flag(self, x="v_p", k="k", t="int"):
        return f"{self.cell(x, k)}.d_{t}"

    def rng(self, x="v_p", k="k", t="int"):
        return self.INT_RANGE.format(c=self.cell(x, k))

    def flag_rng(self, x="v_p", k="k"):
        return f"({self.flag(x, k)} ==> {self.rng(x, k)})"

    def old_flag(self, x="v_p", k="k"):
        return f"(old({self.flag(x, k)}) ==> {self.flag(x, k)})"

    def method(self, requires=(), ensures=(), body=(), formals="v_p: Ref, v_n: Int, amt: Perm"):
        lines = [f"method m__1({formals})", Check("  requires none < amt && amt < write", WF, "internal")]
        lines += [Check(f"  requires {r}", WF, "internal") for r in requires]
        lines += [Check(f"  ensures {e}", P, "postcondition") for e in ensures]
        return lines + ["  decreases", "{", "  var ra1: Ref", *body, "}"]

    @staticmethod
    def probs(lines):
        return [p for p in lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")
                if any(w in p for w in ("`forall`", "quantified", "old[", "marked range row", "expected reason", "`Perm` local"))]

    def clean(self, lines):
        self.assertEqual(self.probs(lines), [])
        self.assertEqual(lint_vpr_other(*build(lines), "t.vpr", "t.vpr.map"), [])

    def refused(self, lines, fragment="`forall` outside the shapes"):
        probs = self.probs(lines)
        self.assertTrue(any(fragment in p for p in probs), probs)

    # --- a method's requires and ensures
    def test_the_rows_of_an_atom_in_requires_and_ensures_are_clean(self):
        rows = [self.quant(self.acc()), self.quant(self.acc("d_int")), self.quant(self.flag()), self.quant(self.rng()),
                self.quant(self.flag_rng())]
        self.clean(self.method(requires=rows, ensures=rows + [self.quant(self.old_flag())]))

    def test_a_permission_row_may_carry_a_perm_formal_as_its_amount(self):
        rows = [self.quant(self.acc(amount=", amt")), self.quant(self.acc("d_int", amount=", amt"))]
        self.clean(self.method(requires=rows, ensures=rows))

    def test_a_guard_stands_outside_the_quantifier_in_either_spelling(self):
        for guard in ("(v_n != 0)", "v_n != 0", "(v_p != null)", "(v_n != 0 && v_p != null)"):
            for wrap in (False, True):
                with self.subTest(guard=guard, wrap=wrap):
                    row = self.quant(self.acc(), wrap=wrap)
                    flag = self.quant(self.flag_rng(), wrap=wrap)
                    self.clean(self.method(requires=[f"{guard} ==> {row}", f"{guard} ==> {flag}"],
                                           ensures=[f"{guard} ==> {row}", f"{guard} ==> {self.quant(self.old_flag(), wrap=wrap)}"]))

    def test_an_element_type_is_any_field_suffix(self):
        t = "ptr_int"
        view = lv._view_wf(f"{self.cell()}.v_{t}")
        rows = [self.quant(self.acc(f"v_{t}")), self.quant(self.flag(t=t)), self.quant(f"({self.flag(t=t)} ==> ({view}))")]
        self.clean(self.method(requires=rows, ensures=rows))

    def test_the_range_must_be_the_atoms(self):
        mutants = {
            "a low bound of zero": self.quant(self.acc(), lo="0"),
            "a low bound of another base": self.quant(self.acc(), lo="offset_of(v_q)"),
            "a high bound of a literal": self.quant(self.acc(), hi="3"),
            "a high bound of another base": self.quant(self.acc(), hi="offset_of(v_q) + v_n"),
            "a high bound that is the count alone": self.quant(self.acc(), hi="v_n"),
            "a high bound that names k": self.quant(self.acc(), hi="offset_of(v_p) + k"),
            "an extra conjunct in the antecedent": self.quant(self.acc(), extra="v_n > 1"),
            "a heap read in the count": self.quant(self.acc(), n="v_p.v_int"),
            "a perm in the count": self.quant(self.acc(), n="perm(v_p.v_int)"),
            "an old in the count": self.quant(self.acc(), n="old(v_n)"),
            "the quantifier over Bool": self.quant(self.acc(), binder="k: Bool"),
        }
        for label, row in mutants.items():
            with self.subTest(mutant=label):
                self.refused(self.method(requires=[row]))
                self.refused(self.method(ensures=[row]))

    def test_the_trigger_is_loc_of_the_block_of_a_variable(self):
        mutants = {
            "no trigger": self.quant(self.acc()).replace("{ loc(block_of(v_p), k) } ", ""),
            "another function": self.quant(self.acc(), trigger="offset_of(loc(block_of(v_p), k))"),
            "an offset index": self.quant(self.acc(), trigger="loc(block_of(v_p), k + 1)"),
            "another variable": self.quant(self.acc(), trigger="loc(block_of(v_p), j)"),
            "two triggers": self.quant(self.acc()).replace("{ loc(block_of(v_p), k) }", "{ loc(block_of(v_p), k) } { loc(block_of(v_p), k + 1) }"),
            "a base that is not a variable": self.quant(self.acc(x="loc(block_of(v_p), offset_of(v_p) + 1)"),
                                                        x="loc(block_of(v_p), offset_of(v_p) + 1)", lo="offset_of(v_p)",
                                                        hi="offset_of(v_p) + v_n"),
            "a base that is not a block_of": self.quant(self.acc(), trigger="loc(v_p, k)"),
        }
        for label, row in mutants.items():
            with self.subTest(mutant=label):
                self.refused(self.method(requires=[row]))

    def test_the_consequent_is_one_of_the_rows(self):
        c = self.cell()
        mutants = {
            "two fields": self.quant(f"{self.acc()} && {self.acc('d_int')}"),
            "a value comparison": self.quant(f"{c}.v_int == 0"),
            "a value inequality": self.quant(f"{c}.v_int > 0"),
            "a flag and another": self.quant(f"{self.flag()} && {self.flag()}"),
            "a range of a smaller type that is not standard": self.quant(f"({c}.d_int ==> ((0 <= {c}.v_int) && ({c}.v_int <= 5)))"),
            "a range with one bound": self.quant(f"{c}.v_int <= 2147483647"),
            "a flag of another type than the range": self.quant(f"({self.flag(t='bool')} ==> {self.rng()})"),
            "an implication from the value": self.quant(f"({c}.v_int > 0 ==> {self.flag()})"),
            "a flag of another cell": self.quant(self.flag(k="j")),
            "an acc of another cell": self.quant(self.acc(k="j")),
            "an acc of another base": self.quant(self.acc(x="v_q")),
            "an acc with an amount that is not a formal": self.quant(self.acc(amount=", kl")),
            "an acc with the amount write": self.quant(self.acc(amount=", write")),
            "an acc with a computed amount": self.quant(self.acc(amount=", amt / 2")),
            "an acc with a heap amount": self.quant(self.acc(amount=", perm(v_p.v_int)")),
            "a pointer range that is not WF": self.quant(f"({self.flag(t='ptr_int')} ==> ({c}.v_ptr_int != null))"),
        }
        for label, row in mutants.items():
            with self.subTest(mutant=label):
                self.refused(self.method(requires=[row]))
                self.refused(self.method(ensures=[row]))

    def test_old_of_a_flag_stands_in_an_ensures_and_an_assert_only(self):
        row = self.quant(self.old_flag())
        self.refused(self.method(requires=[row]))
        self.clean(self.method(ensures=[row]))
        mutants = {
            "the flag of another cell": self.quant(f"(old({self.flag(k='j')}) ==> {self.flag()})"),
            "the consequent of another cell": self.quant(f"(old({self.flag()}) ==> {self.flag(k='j')})"),
            "the value, not the flag": self.quant(f"(old({self.cell()}.v_int) ==> {self.flag()})"),
            "old of the permission": self.quant(f"(old({self.acc()}) ==> {self.flag()})"),
        }
        for label, r in mutants.items():
            with self.subTest(mutant=label):
                self.refused(self.method(ensures=[r]))

    def test_a_guard_is_a_plain_condition(self):
        row = self.quant(self.acc())
        for label, guard in (("a quantifier", f"({self.quant(self.flag())})"), ("a permission", "(acc(v_p.v_int))"),
                             ("a perm", "(perm(v_p.v_int) > none)"), ("a forperm", "(forperm t: Ref [t.tok] :: true)"),
                             ("a labelled old", "(old[lh_0](v_n) > 0)")):
            with self.subTest(guard=label):
                self.refused(self.method(requires=[f"{guard} ==> {row}"]))
        # twin: a plain condition is fine
        self.clean(self.method(requires=[f"(v_n != 0) ==> {row}"]))

    def test_forall_stands_nowhere_else_in_the_clauses(self):
        row = self.quant(self.acc())
        cases = {
            "a function's requires": ["function f_f(v_p: Ref, v_n: Int): Bool", f"  requires {row}", "{", "  true", "}"],
            "a conjunction in a requires": self.method(requires=[f"true && ({row})"]),
            "a disjunction in an ensures": self.method(ensures=[f"v_n == 0 || ({row})"]),
            "a negation": self.method(requires=[f"!({row})"]),
            "an inhale": self.method(body=[f"  inhale {row} // cerastes:inhale-ok test"]),
            "an if condition": self.method(body=[f"  if ({self.quant(self.flag())}) {{", "  }"]),
            "a variable initializer": self.method(body=[f"  var b: Bool := {self.quant(self.flag())}"]),
        }
        for label, lines in cases.items():
            with self.subTest(case=label):
                self.refused(lines)

    def typed_range(self, t, lo, hi, flagged=True, x="v_p", k="k"):
        c = self.cell(x, k)
        rng = f"((({lo}) <= {c}.v_{t}) && ({c}.v_{t} <= {hi}))"
        return f"({self.flag(x, k, t)} ==> {rng})" if flagged else rng

    def test_the_emitters_parenthesized_bounds_are_read(self):
        # the emitter writes every sum under a comparison in parentheses: k < (offset_of(p) + n)
        rows = [self.quant(self.acc(), hi="(offset_of(v_p) + v_n)"), self.quant(self.flag(), lo="(offset_of(v_p))", hi="(offset_of(v_p) + v_n)"),
                self.quant(self.flag_rng(), hi="((offset_of(v_p) + v_n))")]
        self.clean(self.method(requires=rows, ensures=rows))
        self.clean(self.method(body=[Check(f"  assert {r}", A, "uninitialized") for r in rows[1:]]))
        # twins: the parentheses do not let a wrong bound through
        mutants = {
            "another base": self.quant(self.acc(), hi="(offset_of(v_q) + v_n)"),
            "the count alone": self.quant(self.acc(), hi="(v_n)"),
            "a count that names k": self.quant(self.acc(), hi="(offset_of(v_p) + k)"),
            "a low bound of zero": self.quant(self.acc(), lo="(0)", hi="(offset_of(v_p) + v_n)"),
            "a high bound that reads the heap": self.quant(self.acc(), hi="(offset_of(v_p) + v_p.v_int)"),
        }
        for label, row in mutants.items():
            with self.subTest(mutant=label):
                self.refused(self.method(requires=[row]))
                self.refused(self.method(ensures=[row]))

    def test_an_enumerations_range_is_its_own_as_a_cells_row_reads_it(self):
        t = "enum_1_color"
        rows = [self.quant(self.typed_range(t, 0, 2)), self.quant(self.typed_range(t, 0, 2, flagged=False)),
                self.quant(self.typed_range(t, -1, 3)), self.quant(self.acc(f"v_{t}")), self.quant(self.flag(t=t))]
        self.clean(self.method(requires=rows, ensures=rows))
        self.clean(self.method(body=[Check(f"  assert {self.quant(self.typed_range(t, 0, 2))}", A, "permission")]))
        mutants = {
            "a range that excludes zero": self.quant(self.typed_range(t, 1, 2)),
            "a range below zero": self.quant(self.typed_range(t, -3, -1)),
            "a bare range that excludes zero": self.quant(self.typed_range(t, 1, 2, flagged=False)),
            "a flag of another field": self.quant(f"({self.flag(t='int')} ==> {self.typed_range(t, 0, 2, flagged=False)})"),
            "one bound only": self.quant(f"({self.flag(t=t)} ==> ({self.cell()}.v_{t} <= 2))"),
            "a non-literal bound": self.quant(self.typed_range(t, "v_n", 2)),
        }
        for label, row in mutants.items():
            with self.subTest(mutant=label):
                self.refused(self.method(requires=[row]))
                self.refused(self.method(ensures=[row]))

    def test_an_integer_field_takes_its_own_types_range_as_a_cells_row_does(self):
        ok = {"int": (-2**31, 2**31 - 1), "uint": (0, 2**32 - 1), "schar": (-128, 127), "ulong": (0, 2**64 - 1), "i128": (-2**127, 2**127 - 1)}
        for t, (lo, hi) in ok.items():
            with self.subTest(field=t):
                self.clean(self.method(requires=[self.quant(self.typed_range(t, lo, hi))]))
        # twins: another type's standard range, or a range that is no integer type's
        bad = {"int": (0, 2**32 - 1), "uint": (-2**31, 2**31 - 1), "schar": (0, 255), "short": (-128, 127), "int ": (0, 5), "i128": (0, 5)}
        for t, (lo, hi) in bad.items():
            with self.subTest(field=t, mutant=True):
                self.refused(self.method(requires=[self.quant(self.typed_range(t.strip(), lo, hi))]))

    # --- the pure assert rows and the marked rows
    def test_the_pure_assert_rows_are_clean_unguarded_and_guarded(self):
        pure = [self.quant(self.flag()), self.quant(self.rng()), self.quant(self.flag_rng()), self.quant(self.old_flag())]
        body = [Check(f"  assert {r}", A, "uninitialized") for r in pure]
        body += [Check(f"  assert (v_n != 0) ==> ({r})", A, "uninitialized") for r in pure]
        body += [Check(f"  assert v_n != 0 ==> {r}", A, "uninitialized") for r in pure]
        body += [Check(f"  assert ({r})", A, "uninitialized") for r in pure]
        self.clean(self.method(body=body))

    def test_a_pure_assert_row_has_the_shape_too(self):
        c = self.cell()
        mutants = {
            "a guard that reads the heap": f"(v_p.v_int != 0) ==> ({self.quant(self.flag())})",
            "a guard with perm": f"(perm(v_p.v_int) > none) ==> ({self.quant(self.flag())})",
            "a guarded permission row": f"(v_n != 0) ==> ({self.quant(self.acc())})",
            "a wrapped permission row": f"({self.quant(self.acc())})",
            "an equality": self.quant(f"{c}.v_int == 0"),
            "a bound that reads the heap": self.quant(self.flag(), hi="offset_of(v_p) + v_p.v_int"),
            "a zero-based range": self.quant(self.flag(), lo="0", hi="3"),
            "a trigger of another shape": self.quant(self.flag(), trigger="loc(block_of(v_p), k + 1)"),
            "a base that is not a variable": self.quant(self.flag(x="loc(block_of(v_p), 1)"), x="loc(block_of(v_p), 1)"),
        }
        for label, row in mutants.items():
            with self.subTest(mutant=label):
                probs = self.probs(self.method(body=[Check(f"  assert {row}", A, "uninitialized")]))
                self.assertTrue(any("`forall` outside the shapes" in p or "not a marked range row" in p for p in probs), probs)
        # a permission row that is guarded or wrapped is not the marked row of decision 0059 item 7 (that one is unguarded, and a guard
        # stands as `if (g) { }` around it), and it is no pure row either
        for label in ("a guarded permission row", "a wrapped permission row"):
            with self.subTest(mutant=label):
                probs = self.probs(self.method(body=[Check(f"  assert {mutants[label]}", A, "uninitialized")]))
                self.assertTrue(any("`forall` outside the shapes" in p for p in probs), probs)
                self.assertFalse(any("has no expected reason" in p for p in probs), probs)

    def test_a_marked_row_over_a_local_base_inside_a_guard_is_clean(self):
        acc, dacc = self.quant(self.acc(x="ra1"), x="ra1"), self.quant(self.acc("d_int", x="ra1"), x="ra1")
        body = ["  ra1 := loc(block_of(v_p), offset_of(v_p) + 1)", "  if (v_n != 0) {",
                Check(f"    assert {acc}", MARKED, "permission"), Check(f"    assert {dacc}", MARKED, "permission"), "  }"]
        self.clean(self.method(body=body))
        # twin: a row whose base is the address expression is not a trigger Silicon accepts (S44 F-b1)
        bad = self.quant(self.acc(x="loc(block_of(v_p), offset_of(v_p) + 1)"), x="loc(block_of(v_p), offset_of(v_p) + 1)", lo="offset_of(v_p)",
                         hi="offset_of(v_p) + v_n")
        self.assertTrue(self.probs(self.method(body=[Check(f"  assert {bad}", MARKED, "permission")])))

    def test_a_marked_row_may_carry_the_sum_form_and_an_amount_that_reads_no_heap(self):
        sums = [", amt", ", amt + ((block_of(v_q) == block_of(v_p) && offset_of(v_q) <= k && k < offset_of(v_q) + v_m) ? amt : none)",
                ", l1 + (inq ? l2 : none)"]
        for amount in sums:
            with self.subTest(amount=amount):
                body = [Check(f"  assert {self.quant(self.acc(amount=amount))}", MARKED, "permission")]
                self.assertEqual(self.probs(self.method(body=body)), [])
        for amount in (", amt + (v_q.v_int > 0 ? amt : none)", ", amt + (perm(v_q.v_int) > none ? amt : none)"):
            with self.subTest(amount=amount):
                body = [Check(f"  assert {self.quant(self.acc(amount=amount))}", MARKED, "permission")]
                self.assertTrue(any("not a marked range row" in p for p in self.probs(self.method(body=body))))

    def test_an_unmarked_row_with_the_reason_of_a_marked_one_is_refused(self):
        # a pure row whose map entry says insufficient.permission would let a read of a flag without permission pass for a user error
        for row in (self.quant(self.flag()), self.quant(self.flag_rng()), self.quant(self.old_flag()), f"(v_n != 0) ==> ({self.quant(self.flag())})"):
            with self.subTest(row=row):
                probs = self.probs(self.method(body=[Check(f"  assert {row}", MARKED, "permission")]))
                self.assertTrue(any("carries the expected reason" in p for p in probs), probs)
        # and a marked row without the reason is refused, as the older rule says
        probs = self.probs(self.method(body=[Check(f"  assert {self.quant(self.acc())}", "assert.failed", "permission")]))
        self.assertTrue(any("has no expected reason" in p for p in probs), probs)

    # --- loops
    def loop_lines(self, *invariants, label=True):
        lines = ["method m__1(v_p: Ref, v_n: Int, amt: Perm)", Check("  requires none < amt && amt < write", WF, "internal"),
                 "  decreases", "{", "  var v_a: Ref"]
        if label:
            lines.append("  label lh_0")
        return lines + ["  while (true)", *[Check(i, INV, "internal") for i in invariants], "  {", "  }", "}"]

    def conjuncts(self, guard=None, amount=", amt"):
        g = f"{guard} ==> " if guard else ""
        c = self.cell()
        rng = f"((offset_of(v_p) <= k) && (k < offset_of(v_p) + v_n))"

        def inv(ante, cons):
            return f"    invariant {g}(forall k: Int :: {{ {c} }} {ante} ==> {cons})"
        return {
            "acc": inv(rng, f"acc({c}.v_int{amount})"),
            "dacc": inv(rng, f"acc({c}.d_int{amount})"),
            "flags": inv(f"({rng} && old[lh_0]({c}.d_int))", f"{c}.d_int"),
            "range": inv(f"({rng} && {c}.d_int)", self.rng()),
            "values": inv(rng, f"({c}.v_int == old[lh_0]({c}.v_int))"),
        }

    def test_the_loop_conjuncts_with_an_amount_and_a_guard_are_clean(self):
        for guard in (None, "(v_n != 0)", "v_p != null"):
            for amount in ("", ", amt"):
                with self.subTest(guard=guard, amount=amount):
                    self.assertEqual(self.probs(self.loop_lines(*self.conjuncts(guard, amount).values())), [])

    def test_a_loop_amount_is_the_methods_own_perm_formal(self):
        for amount in (", kl", ", write", ", amt / 2", ", perm(v_p.v_int)", ", other"):
            with self.subTest(amount=amount):
                probs = self.probs(self.loop_lines(self.conjuncts(None, amount)["acc"]))
                self.assertTrue(any("item 8's four conjuncts" in p for p in probs), probs)
        # a formal of another method does not count
        lines = ["method m__0(v_p: Ref, other: Perm)", "  decreases", "{", "}"] + self.loop_lines(self.conjuncts(None, ", other")["acc"])
        self.assertTrue(any("item 8's four conjuncts" in p for p in self.probs(lines)))

    def test_a_loop_guard_is_a_plain_condition_and_old_label_is_still_read(self):
        c = self.conjuncts("(acc(v_p.v_int))")
        for name, inv in c.items():
            with self.subTest(conjunct=name):
                self.assertTrue(self.probs(self.loop_lines(inv)), name)
        c = self.conjuncts("(v_n != 0)")
        probs = self.probs(self.loop_lines(c["flags"], label=False))
        self.assertTrue(any("no earlier `label lh_0`" in p for p in probs), probs)
        inv = c["acc"] + " && old[lh_0](true)"
        self.assertTrue(any("`old[label]` outside" in p for p in self.probs(self.loop_lines(inv))))

    def test_a_loop_conjunct_under_a_guard_keeps_its_shape(self):
        c = self.conjuncts("(v_n != 0)")
        cell = self.cell()
        mutants = {
            "flags without old": c["flags"].replace(f"old[lh_0]({cell}.d_int)", f"{cell}.d_int"),
            "values of another field": c["values"].replace("old[lh_0](loc(block_of(v_p), k).v_int)", "old[lh_0](loc(block_of(v_p), k).v_bool)"),
            "a base that is not a variable": c["acc"].replace("block_of(v_p)", "block_of(loc(block_of(v_p), 1))"),
            "a consequent of any kind": c["acc"].replace(f"acc({cell}.v_int, amt)", f"{cell}.v_int > 0"),
        }
        for label, inv in mutants.items():
            with self.subTest(mutant=label):
                self.assertTrue(self.probs(self.loop_lines(inv)), label)

    # --- the amount of a call row
    SLOT = "loc(block_of(ra1), offset_of(ra1))"
    LIVE = "perm(lvloc(block_of(ra1)).lv)"

    def reads_amount(self, count="v_n", t="int", slot=None, live=None, live2=None):
        slot, live = slot or self.SLOT, live or self.LIVE
        live2 = live2 or live
        m = f"(perm({slot}.v_{t}) < perm({slot}.d_{t}) ? perm({slot}.v_{t}) : perm({slot}.d_{t}))"
        return f"(({count} > 0) ? ({m} < {live} ? {m} : {live}) : {live2})"

    def amount_lines(self, rhs):
        return self.method(body=["  var l1: Perm", "  ra1 := v_p", f"  l1 := {rhs}"])

    def test_the_amount_of_a_range_call_row_is_a_quotient_by_a_literal(self):
        for label, rhs in {
            "owns": f"{self.LIVE} / 2",
            "owns, more atoms": f"{self.LIVE} / 3",
            "owns, a parenthesized divisor": f"{self.LIVE} / (3)",
            "owns, guarded": f"(v_n != 0 ? {self.LIVE} : write) / 2",
            "reads": f"{self.reads_amount()} / 2",
            "reads, a counted expression": f"{self.reads_amount('v_n - 1')} / 4",
            "reads, guarded": f"(v_p != null ? {self.reads_amount()} : write) / 2",
            "reads, another element type": f"{self.reads_amount(t='bool')} / 2",
        }.items():
            with self.subTest(amount=label):
                self.clean(self.amount_lines(rhs))

    def test_any_other_quotient_is_refused(self):
        slot2 = "loc(block_of(v_q), offset_of(v_q))"
        live2 = "perm(lvloc(block_of(v_q)).lv)"
        for label, rhs in {
            "a divisor of one": f"{self.LIVE} / 1",
            "a divisor of zero": f"{self.LIVE} / 0",
            "a divisor that is a variable": f"{self.LIVE} / v_n",
            "a divisor that is an expression": f"{self.LIVE} / (v_n + 1)",
            "a product": f"{self.LIVE} * 2",
            "a sum": f"{self.LIVE} + {self.LIVE}",
            "a cell's permission": "perm(v_p.v_int) / 2",
            "none": "none / 2",
            "write": "write / 2",
            "a liveness over an address expression": "perm(lvloc(block_of(loc(block_of(v_p), offset_of(v_p) + 1))).lv) / 2",
            "a guard that reads the heap": f"(v_p.v_int != 0 ? {self.LIVE} : write) / 2",
            "a guard with perm": f"(perm(v_p.v_int) > none ? {self.LIVE} : write) / 2",
            "a guard that is the liveness": f"({self.LIVE} ? {self.LIVE} : write) / 2",
            "a guard whose else is not write": f"(v_n != 0 ? {self.LIVE} : none) / 2",
            "a count that reads the heap": f"{self.reads_amount('v_p.v_int')} / 2",
            "a minimum over another base": f"{self.reads_amount(slot=slot2)} / 2",
            "a minimum with the other liveness": f"{self.reads_amount(live2=live2)} / 2",
            "a minimum with the other element types": self.reads_amount().replace(".d_int", ".d_bool", 1) + " / 2",
            "a minimum compared with the other liveness": self.reads_amount().replace(f"< {self.LIVE} ?", f"< {live2} ?", 1) + " / 2",
            "a minimum that falls back to the other liveness": self.reads_amount().replace(f": {self.LIVE})", f": {live2})", 1) + " / 2",
            "a minimum not over the first slot": self.reads_amount(slot="loc(block_of(ra1), offset_of(ra1) + 1)") + " / 2",
            "a conditional count of anything": f"(v_n > 0 ? {self.LIVE} : none) / 2",
        }.items():
            with self.subTest(amount=label):
                probs = self.probs(self.amount_lines(rhs))
                self.assertTrue(any("`Perm` local `l1` is assigned" in p for p in probs), probs)

    # --- the minimum of item 6 with the caller's own read amounts (audit round 2, S5)
    def fold(self, leaves):
        """`a < b ? a : b` nested to the left over the leaves, as a minimum written in Viper."""
        cur = leaves[0]
        for x in leaves[1:]:
            cur = f"({cur} < {x} ? {cur} : {x})"
        return cur

    def own_min_lines(self, leaves, formals="v_p: Ref, v_n: Int, amt: Perm, amt2: Perm", count="v_n", live=None):
        live = live or self.LIVE
        rhs = f"(({count} > 0) ? {self.fold(leaves)} : {live}) / 2"
        return [f"method m__1({formals})", Check("  requires none < amt && amt < write", WF, "internal"),
                Check("  requires none < amt2 && amt2 < write", WF, "internal"), "  decreases", "{", "  var ra1: Ref", "  var kl: Perm",
                "  kl := perm(lvloc(block_of(v_p)).lv)", "  ra1 := v_p", "  var l1: Perm", f"  l1 := {rhs}", "}"]

    def test_the_minimum_may_hold_the_methods_own_perm_formals(self):
        v, d = f"perm({self.SLOT}.v_int)", f"perm({self.SLOT}.d_int)"
        for label, leaves in {
            "the three of item 6": [v, d, self.LIVE],
            "one formal": [v, d, self.LIVE, "amt"],
            "two formals": [v, d, self.LIVE, "amt", "amt2"],
            "formals first": ["amt", "amt2", v, d, self.LIVE],
            "a formal repeated": [v, d, self.LIVE, "amt", "amt"],
        }.items():
            with self.subTest(minimum=label):
                self.clean(self.own_min_lines(leaves))

    def test_the_minimum_takes_nothing_but_the_slot_the_liveness_and_the_formals(self):
        v, d = f"perm({self.SLOT}.v_int)", f"perm({self.SLOT}.d_int)"
        slot2 = "loc(block_of(ra1), offset_of(ra1) + 1)"
        other = "perm(lvloc(block_of(v_q)).lv)"
        for label, leaves in {
            "a Perm local": [v, d, self.LIVE, "kl"],
            "a formal of another method": [v, d, self.LIVE, "other"],
            "the literal write": [v, d, self.LIVE, "write"],
            "a quotient": [v, d, self.LIVE, "(amt / 2)"],
            "a perm of a second slot": [v, d, self.LIVE, f"perm({slot2}.v_int)"],
            "a perm of another cell": [v, d, self.LIVE, "perm(v_q.v_int)"],
            "another liveness cell": [v, d, self.LIVE, other],
            "no liveness cell": [v, d, "amt"],
            "no flag permission": [v, self.LIVE, "amt"],
            "no value permission": [d, self.LIVE, "amt"],
            "the flag of another type": [v, f"perm({self.SLOT}.d_bool)", self.LIVE, "amt"],
        }.items():
            with self.subTest(minimum=label):
                probs = self.probs(self.own_min_lines(leaves))
                self.assertTrue(any("`Perm` local `l1` is assigned" in p for p in probs), probs)
        # a maximum is no minimum: the comparison must be of the two branches in this order
        bad = f"(({v} < {d} ? {d} : {v}) < {self.LIVE} ? {self.LIVE} : {v})"
        lines = self.own_min_lines([v, d, self.LIVE])
        lines[-2] = f"  l1 := (((v_n > 0) ? {bad} : {self.LIVE})) / 2"
        self.assertTrue(any("`Perm` local `l1` is assigned" in p for p in self.probs(lines)))

    # --- decision 0063 item 8: the amount of a `reads(*p)` or `reads(p->a)` site with array parts (cells, first slots of the array parts, the liveness)
    SITE_LIVE = "perm(lvloc(block_of(v_p)).lv)"

    def site_lines(self, leaves, guard=None, divisor=2, bind="  ra1 := loc(member(v_p, 1), 0)"):
        lines = self.own_min_lines([self.LIVE], live=self.LIVE)
        lines = [bind if l == "  ra1 := v_p" else l for l in lines]
        rhs = f"({self.fold(leaves)}) / {divisor}"
        lines[-2] = f"  l1 := {rhs if guard is None else f'({guard} ? ({self.fold(leaves)}) : write) / {divisor}'}"
        return lines

    def test_the_amount_of_a_structure_site_with_array_parts_is_a_minimum_over_its_permissions(self):
        cv, cd = "perm(loc(member(v_p, 0), 0).v_int)", "perm(loc(member(v_p, 0), 0).d_int)"
        nv, nd = "perm(loc(member(loc(member(v_p, 0), 0), 1), 0).v_int)", "perm(loc(member(loc(member(v_p, 0), 0), 1), 0).d_int)"
        sv, sd = f"perm({self.SLOT}.v_int)", f"perm({self.SLOT}.d_int)"
        live = self.SITE_LIVE
        for label, kw in {
            "one cell leaf and one array part (marr_reads_call.vpr:161)": dict(leaves=[cv, cd, sv, sd, live]),
            "an array part alone (reads(p->a))": dict(leaves=[sv, sd, live]),
            "a nested cell leaf": dict(leaves=[cv, cd, nv, nd, sv, sd, live]),
            "the caller's own amounts": dict(leaves=[cv, cd, sv, sd, live, "amt", "amt2"]),
            "guarded": dict(leaves=[cv, cd, sv, sd, live], guard="v_p != null"),
            "a divisor of 3": dict(leaves=[cv, cd, sv, sd, live], divisor=3),
        }.items():
            with self.subTest(site=label):
                self.clean(self.site_lines(**kw))

    def test_a_structure_sites_amount_that_leaves_out_a_permission_is_refused(self):
        cv, cd = "perm(loc(member(v_p, 0), 0).v_int)", "perm(loc(member(v_p, 0), 0).d_int)"
        sv, sd = f"perm({self.SLOT}.v_int)", f"perm({self.SLOT}.d_int)"
        live = self.SITE_LIVE
        other = "perm(lvloc(block_of(v_q)).lv)"
        for label, leaves in {
            "no liveness": [cv, cd, sv, sd],
            "no cell flag": [cv, sv, sd, live],
            "no cell value": [cd, sv, sd, live],
            "no slot flag": [cv, cd, sv, live],
            "no slot value": [cv, cd, sd, live],
            "a cell of another type's flag": [cv, "perm(loc(member(v_p, 0), 0).d_bool)", sv, sd, live],
            "the liveness of another variable": [cv, cd, sv, sd, live, other],
            "a cell rooted at another variable": [cv, cd, sv, sd, live, "perm(loc(member(v_q, 0), 0).v_int)", "perm(loc(member(v_q, 0), 0).d_int)"],
            "a cell that is not a member slot": [cv, cd, sv, sd, live, "perm(v_p.v_int)", "perm(v_p.d_int)"],
            "a cell at another slot of the chain": [cv, cd, sv, sd, live, "perm(loc(member(v_p, 0), 1).v_int)", "perm(loc(member(v_p, 0), 1).d_int)"],
            "a cell through block_of": [cv, cd, sv, sd, live, "perm(loc(member(loc(block_of(v_p), 0), 0), 0).v_int)", "perm(loc(member(loc(block_of(v_p), 0), 0), 0).d_int)"],
            "a second slot of an array": [cv, cd, sv, sd, live, f"perm(loc(block_of(ra1), offset_of(ra1) + 1).v_int)"],
            "a Perm local": [cv, cd, sv, sd, live, "kl"],
            "the literal write": [cv, cd, sv, sd, live, "write"],
        }.items():
            with self.subTest(minimum=label):
                probs = self.probs(self.site_lines(leaves))
                self.assertTrue(any("`Perm` local `l1` is assigned" in p for p in probs), (label, probs))
        # the slot's base is the array part's bound local: a local assigned once `loc(member(<chain rooted at the liveness variable>, k), 0)`
        leaves = [cv, cd, sv, sd, live]
        for label, bind in {
            "the base is the structure's own pointer, not a member's first slot": "  ra1 := v_p",
            "the base is bound to a member of another variable": "  ra1 := loc(member(v_q, 1), 0)",
            "the base is bound to another slot of the member": "  ra1 := loc(member(v_p, 1), 1)",
            "the base is bound twice": "  ra1 := loc(member(v_p, 1), 0)\n  ra1 := loc(member(v_p, 2), 0)",
        }.items():
            with self.subTest(base=label):
                first, *more = bind.split("\n")
                lines = self.site_lines(leaves, bind=first)
                lines[lines.index(first):lines.index(first) + 1] = [first, *more]
                probs = self.probs(lines)
                self.assertTrue(any("`Perm` local `l1` is assigned" in p for p in probs), (label, probs))
        # a divisor of one, a guard whose else is not write
        for label, kw in {"a divisor of one": dict(divisor=1), "a divisor of zero": dict(divisor=0)}.items():
            with self.subTest(minimum=label):
                probs = self.probs(self.site_lines([cv, cd, sv, sd, live], **kw))
                self.assertTrue(any("`Perm` local `l1` is assigned" in p for p in probs), (label, probs))

    def test_the_amount_of_a_plain_scalar_cell_site_is_a_minimum_over_the_root_cell_and_its_liveness(self):
        # audit round 3 S2 (il-analyst-4): a call with array parts binds a scalar cell site's amount (marr_alias_reads.vpr:194)
        rv, rd = "perm(v_p.v_int)", "perm(v_p.d_int)"
        live = self.SITE_LIVE
        for label, kw in {
            "value, flag and liveness, divided by 3 (marr_alias_reads.vpr)": dict(leaves=[rv, rd, live], divisor=3),
            "the value and flag in the other order": dict(leaves=[rd, rv, live], divisor=2),
            "with the caller's own amounts": dict(leaves=[rv, rd, live, "amt", "amt2"], divisor=2),
            "guarded": dict(leaves=[rv, rd, live], guard="v_p != null", divisor=2),
        }.items():
            with self.subTest(site=label):
                self.clean(self.site_lines(**kw))

    def test_a_scalar_cell_sites_amount_that_leaves_out_or_changes_a_permission_is_refused(self):
        rv, rd = "perm(v_p.v_int)", "perm(v_p.d_int)"
        live = self.SITE_LIVE
        cv, cd = "perm(loc(member(v_p, 0), 0).v_int)", "perm(loc(member(v_p, 0), 0).d_int)"
        for label, leaves in {
            "the value only": [rv, live],
            "the flag only": [rd, live],
            "the value and the flag of another type": [rv, "perm(v_p.d_bool)", live],
            "the value of one type and the flag of another, both complete": [rv, rd, "perm(v_p.v_bool)", "perm(v_p.d_bool)", live],
            "a cell that is another variable than the liveness one": ["perm(v_q.v_int)", "perm(v_q.d_int)", live],
            "the liveness of another variable": [rv, rd, "perm(lvloc(block_of(v_q)).lv)"],
            "no liveness": [rv, rd],
            "a second liveness cell": [rv, rd, live, "perm(lvloc(block_of(v_q)).lv)"],
            "the root cell beside a structure cell": [rv, rd, cv, cd, live],
            "the root cell and another variable's cell": [rv, rd, "perm(v_q.v_int)", "perm(v_q.d_int)", live],
            "a Perm local": [rv, rd, live, "kl"],
            "the literal write": [rv, rd, live, "write"],
        }.items():
            with self.subTest(minimum=label):
                probs = self.probs(self.site_lines(leaves, divisor=3))
                self.assertTrue(any("`Perm` local `l1` is assigned" in p for p in probs), (label, probs))
        for label, kw in {"a divisor of one": dict(divisor=1), "a divisor of zero": dict(divisor=0)}.items():
            with self.subTest(minimum=label):
                probs = self.probs(self.site_lines([rv, rd, live], **kw))
                self.assertTrue(any("`Perm` local `l1` is assigned" in p for p in probs), (label, probs))

    def test_a_quotient_is_still_not_assigned_in_a_loop_that_uses_it(self):
        lines = self.method(body=["  var l1: Perm", "  ra1 := v_p", f"  l1 := {self.LIVE} / 2", "  label lh_0", "  while (true)",
                                  "    invariant acc(lvloc(block_of(ra1)).lv, l1)", "  {", f"    l1 := {self.LIVE} / 2", "  }"])
        probs = [p for p in lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map") if "assigned inside the loop" in p]
        self.assertTrue(probs)



class UserQuantifierTests(unittest.TestCase):
    """Decision 0061 item 11 (M4.5e1c): a user quantifier (`forall q0: Int, ... :: { loc(block_of(X), q0), ... } body`) in a method's
    `requires`, `ensures`, `invariant` and `assert`, and item 10's marked `wildcard` row: every twin changes one thing of a clean
    fixture and must be refused, so the clean fixtures show the shapes pass and a weaker rule fails here."""

    RANGE = "(-2147483648 <= (q0 - offset_of(v_p)) && (q0 - offset_of(v_p)) <= 2147483647)"
    READ = "(!(0 <= (q0 - offset_of(v_p)) && (q0 - offset_of(v_p)) < v_n) || (loc(block_of(v_p), q0).v_int >= 0))"
    REACH = "(0 <= (q0 - offset_of(v_p)) && (q0 - offset_of(v_p)) < v_n)"
    MARK = "assert.failed[insufficient.permission]"

    @staticmethod
    def uq(body=None, binders="q0: Int", trigger="{ loc(block_of(v_p), q0) }", ante=None):
        body = UserQuantifierTests.READ if body is None else body
        ante = UserQuantifierTests.RANGE if ante is None else ante
        return f"forall {binders} :: {trigger} {ante} ==> {body}"

    def wild(self, trigger="{ loc(block_of(v_p), q0) }", cons="acc(loc(block_of(v_p), q0).v_int, wildcard)", binders="q0: Int", ante=None):
        return f"forall {binders} :: {trigger} {self.RANGE if ante is None else ante} && {self.REACH} ==> {cons}"

    def method(self, requires=(), ensures=(), body=(), formals="v_p: Ref, v_n: Int, amt: Perm"):
        lines = [f"method m__1({formals})", Check("  requires none < amt && amt < write", WF, "internal")]
        lines += [Check(f"  requires {r}", WF, "internal") for r in requires]
        lines += [Check(f"  ensures {e}", P, "postcondition") for e in ensures]
        return lines + ["  decreases", "{", "  var ra1: Ref", *body, "}"]

    def loop(self, invariants):
        return ["  var i__1: Int := 0", "  while (i__1 < v_n)", *[Check(f"    invariant {i}", INV, "invariant-preserved") for i in invariants],
                Check("    decreases v_n - i__1", "termination.failed", "termination"), "  {", "    i__1 := i__1 + 1", "  }"]

    @staticmethod
    def probs(lines):
        return [p for p in lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")
                if any(w in p for w in ("decision 0061", "`forall`", "wildcard", "marked range row", "expected reason", "old["))]

    def clean(self, lines):
        self.assertEqual(self.probs(lines), [])
        self.assertTrue(all(ok for _n, ok in self.verdicts(lines)))

    @staticmethod
    def verdicts(lines):
        text, _ = build(lines)
        src = lv.split_source(text)
        return [(n, v[0]) for n, v in sorted(lv.classify_quantified_lines(src, lv.declarations(src)).verdicts.items())]

    def refused(self, lines, fragment="decision 0061"):
        probs = self.probs(lines)
        self.assertTrue(any(fragment in p for p in probs), probs)

    def refused_by_the_rule(self, lines, label=""):
        """The rule's own verdict is `refused` for a line (not only some other rule of the lint)."""
        self.assertTrue(self.probs(lines), label)
        self.assertTrue(any(not ok for _n, ok in self.verdicts(lines)), label)

    def row(self, text, kind="assert", ident=None, cat="check"):
        return Check(f"  {kind} {text}", ident or A, cat)

    # --- the user quantifier
    def test_a_user_quantifier_is_clean_in_requires_ensures_invariant_and_assert(self):
        q = self.uq()
        self.clean(self.method(requires=[q], ensures=[q], body=[self.row(q)]))
        self.clean(self.method(body=self.loop([q])))

    def test_the_guard_stands_outside_in_either_spelling(self):
        q = self.uq()
        for guard in ("(v_n != 0)", "v_n != 0", "(v_n != 0 && v_p != null)"):
            for wrap in (False, True):
                with self.subTest(guard=guard, wrap=wrap):
                    text = f"{guard} ==> ({q})" if wrap else f"{guard} ==> {q}"
                    self.clean(self.method(requires=[text], ensures=[text], body=[self.row(text)]))

    def test_two_classes_are_one_quantifier_with_one_trigger_set_of_both_terms(self):
        two = self.uq(binders="q0: Int, q1: Int", trigger="{ loc(block_of(v_p), q0), loc(block_of(v_q), q1) }",
                      ante=f"({self.RANGE} && (q1 == offset_of(v_q) + (q0 - offset_of(v_p))))")
        self.clean(self.method(requires=[two], ensures=[two], formals="v_p: Ref, v_q: Ref, v_n: Int, amt: Perm"))

    def test_old_without_a_label_and_the_result_may_stand_in_the_body(self):
        q = self.uq(body="(loc(block_of(v_p), q0).v_int == old(loc(block_of(v_p), q0).v_int))")
        self.clean(self.method(ensures=[q]))

    def test_a_read_the_binder_does_not_index_may_be_a_computed_loc(self):
        q = self.uq(body="(loc(block_of(v_p), q0).v_int <= loc(block_of(v_p), offset_of(v_p) + v_n).v_int)")
        self.clean(self.method(requires=[q]))

    def test_an_earlier_invariant_clause_may_stand_in_the_prior_of_a_loops_rows(self):
        """Question 258 (0037 item 7's evaluation-order prior): a loop's obligation `invariant` line and its framing `assert` are `PRIOR ==> CONCLUSION` with
        the loop's earlier quantified clauses among the conjuncts of the prior."""
        q = self.uq()
        q2 = self.uq(body="(loc(block_of(v_p), q0).d_int)")
        prior = f"((v_n >= 0) && ({q}))"
        loop = self.loop(["0 <= i__1 && i__1 <= v_n", q, f"{prior} ==> ({q2})", f"{prior} ==> (!((v_n - i__1) < (-2147483648)))"])
        self.clean(self.method(body=[self.row(f"{prior} ==> ({q2})"), self.row(f"{prior} ==> (!((v_n - 0) < (-2147483648)))"), *loop]))

    def test_the_twins_of_the_quantifier_in_a_loop_rows_prior(self):
        q = self.uq()
        q2 = self.uq(body="(loc(block_of(v_p), q0).d_int)")
        acc_q = self.uq(body="acc(loc(block_of(v_p), q0).v_int)")
        old_q = self.uq(body="(loc(block_of(v_p), q0).v_int == old[lh_0](loc(block_of(v_p), q0).v_int))")
        two_q = self.uq(trigger="{ loc(block_of(v_p), q0) } { loc(block_of(v_p), q0) }")
        other = self.uq(body="(loc(block_of(v_p), q0).v_int >= 5)")
        def prior(x):
            return f"((v_n >= 0) && ({x}))"
        def with_loop(invs, rows=()):
            return self.method(body=[*rows, *self.loop(invs)])
        cases = {
            # a quantifier with acc in the prior of an obligation line (it is no clause of the loop either)
            "a prior quantifier with acc": with_loop(["0 <= i__1 && i__1 <= v_n", q, f"{prior(acc_q)} ==> ({q2})"]),
            "a prior quantifier with a labelled old": with_loop(["0 <= i__1 && i__1 <= v_n", old_q, f"{prior(old_q)} ==> ({q2})"]),
            "a prior quantifier with two trigger sets": with_loop(["0 <= i__1 && i__1 <= v_n", two_q, f"{prior(two_q)} ==> ({q2})"]),
            "a prior quantifier that is no clause of a loop": with_loop(["0 <= i__1 && i__1 <= v_n", q, f"{prior(other)} ==> ({q2})"]),
            "a prior quantifier on a non-loop assert": self.method(body=[self.row(f"{prior(q)} ==> ({q2})")]),
            "a prior quantifier in a requires": self.method(requires=[f"{prior(q)} ==> ({q2})"]),
            "a prior quantifier in an ensures": self.method(ensures=[f"{prior(q)} ==> ({q2})"]),
            "a prior quantifier in a requires beside a loop that has the clause": self.method(requires=[f"{prior(q)} ==> ({q2})"], body=self.loop(["0 <= i__1 && i__1 <= v_n", q])),
            "a prior quantifier in an ensures beside a loop that has the clause": self.method(ensures=[f"{prior(q)} ==> ({q2})"], body=self.loop(["0 <= i__1 && i__1 <= v_n", q])),
            "a prior condition that holds a permission": with_loop(["0 <= i__1 && i__1 <= v_n", q, f"((perm(v_p.v_int) > none) && ({q})) ==> ({q2})"]),
            "an assert prior condition that reads the heap": with_loop(["0 <= i__1 && i__1 <= v_n", q], rows=[self.row(f"((v_p.v_int > 0) && ({q})) ==> ({q2})")]),
            "a conclusion with acc": with_loop(["0 <= i__1 && i__1 <= v_n", q, f"{prior(q)} ==> {acc_q}"]),
            "a conclusion that is a quantifier with two trigger sets": with_loop(["0 <= i__1 && i__1 <= v_n", q, f"{prior(q)} ==> ({two_q})"]),
            "a conclusion that holds a nested quantifier": with_loop(["0 <= i__1 && i__1 <= v_n", q, f"{prior(q)} ==> (true && ({q2}))"]),
            "a prior mixing && and || without parentheses": with_loop(["0 <= i__1 && i__1 <= v_n", q, f"((v_n >= 0) && (v_n >= 1) || ({q})) ==> ({q2})"]),
        }
        for label, lines in cases.items():
            with self.subTest(mutant=label):
                self.assertTrue(self.probs(lines), label)
                self.assertTrue(any(not ok for _n, ok in self.verdicts(lines)), label)

    def loop_of(self, var, invariants, back_edge=()):
        """A `while` loop over `var` with its entry run's label, the invariants and `back_edge` asserts at the end of its body."""
        return [f"  var {var}: Int := 0", f"  label lh_{var}", f"  while ({var} < v_n)",
                *[Check(f"    invariant {i}", INV, "invariant-preserved") for i in invariants],
                Check(f"    decreases v_n - {var}", "termination.failed", "termination"), "  {", f"    {var} := {var} + 1", *back_edge, "  }"]

    def test_a_loops_prior_names_the_clauses_of_its_own_loop_only(self):
        """Audit round 3, issue 3 (question 258): the quantifiers a prior may hold are the EARLIER `invariant` lines of the loop the line belongs to, and
        for a framing `assert` the clauses of the loop it frames (its entry run, its body's last asserts); no other line has one.  An obligation
        conditioned on another loop's clause is vacuous wherever that clause is false."""
        q = self.uq()
        q2 = self.uq(body="(loc(block_of(v_p), q0).d_int)")
        other = self.uq(body="(loc(block_of(v_p), q0).v_int >= 5)")
        other2 = self.uq(body="(loc(block_of(v_p), q0).v_int >= 6)")
        base = "0 <= i__1 && i__1 <= v_n"
        base2 = "0 <= j__2 && j__2 <= v_n"

        def prior(x):
            return f"((v_n >= 0) && ({x}))"
        row = lambda x, y=q2: self.row(f"{prior(x)} ==> ({y})")  # noqa: E731
        two_loops = lambda inv1, inv2, between=(), rows1=(), rows2=(), back1=(), back2=(): self.method(  # noqa: E731
            body=[*rows1, *self.loop_of("i__1", inv1, back1), *between, *rows2, *self.loop_of("j__2", inv2, back2)])
        # each loop with its own clauses, entry and back-edge asserts and obligation lines: clean
        self.clean(two_loops([base, q, f"{prior(q)} ==> ({q2})"], [base2, other, f"{prior(other)} ==> ({other2})"],
                             rows1=[row(q)], back1=[row(q)], rows2=[row(other, other2)], back2=[row(other, other2)]))
        cases = {
            "an obligation line of loop 2 conditioned on loop 1's clause": two_loops([base, q], [base2, other, f"{prior(q)} ==> ({other2})"]),
            "a flag line of loop 2 conditioned on loop 1's clause": two_loops([base, q], [base2, other, f"{prior(q)} ==> (!((v_n - j__2) < (-2147483648)))"]),
            "an obligation line conditioned on a LATER clause of its own loop": self.method(body=self.loop(
                [base, f"{prior(q)} ==> ({q2})", q])),
            "the entry assert of loop 2 conditioned on loop 1's clause": two_loops([base, q], [base2, other], rows2=[row(q, other2)]),
            "the back-edge assert of loop 2 conditioned on loop 1's clause": two_loops([base, q], [base2, other], back2=[row(q, other2)]),
            "the entry assert of loop 1 conditioned on loop 2's clause": two_loops([base, q], [base2, other], rows1=[row(other, q2)]),
            "an assert after the loops conditioned on a loop's clause": two_loops([base, q], [base2, other], between=[self.row("v_n >= 0"), "  exhale v_n >= 0"],
                                                                                  rows2=[row(q)]),
            "an assert after the last loop conditioned on its own clause": self.method(body=[*self.loop_of("i__1", [base, q]), row(q)]),
            "an assert between statements that are no framing run": self.method(body=[*self.loop_of("i__1", [base, q]), "  exhale v_n >= 0", row(q)]),
            "an assert inside a branch of the loop's body, not its last asserts": self.method(body=self.loop_of("i__1", [base, q], back_edge=[
                row(q), "    if (v_n > 0) {", "      ra1 := v_p", "    }"])),
        }
        for label, lines in cases.items():
            with self.subTest(mutant=label):
                self.assertTrue(self.probs(lines), label)
                self.assertTrue(any(not ok for _n, ok in self.verdicts(lines)), label)

    def test_a_guard_that_holds_a_permission_a_quantifier_or_the_heap_is_refused(self):
        q = self.uq()
        for label, guard in (("a permission", "(perm(v_p.v_int) > none)"), ("a quantifier", f"({self.uq(binders='q1: Int', trigger='{ loc(block_of(v_p), q1) }')})"),
                             ("a labelled old", "(old[lh_0](v_n) != 0)"), ("an empty guard", " ")):
            with self.subTest(guard=label):
                self.refused(self.method(requires=[f"{guard} ==> {q}"]))
        # an assert's guard reads no heap, a clause's may
        self.refused(self.method(body=[self.row(f"(v_p.v_int != 0) ==> {q}")]))

    def test_the_twins_of_the_binders_the_trigger_and_the_body(self):
        cases = {
            "a binder named q without a number": self.uq(binders="q: Int", trigger="{ loc(block_of(v_p), q) }").replace("q0", "q"),
            "a binder named i": self.uq(binders="i: Int", trigger="{ loc(block_of(v_p), i) }").replace("q0", "i"),
            "a binder named k": self.uq(binders="k: Int", trigger="{ loc(block_of(v_p), k) }").replace("q0", "k"),
            "a second binder not named q": self.uq(binders="q0: Int, x: Int", trigger="{ loc(block_of(v_p), q0), loc(block_of(v_p), x) }"),
            "a binder that is not Int": self.uq(binders="q0: Bool"),
            "a binder twice": self.uq(binders="q0: Int, q0: Int", trigger="{ loc(block_of(v_p), q0), loc(block_of(v_p), q0) }"),
            "no trigger set": self.uq(trigger=""),
            "two trigger sets": self.uq(trigger="{ loc(block_of(v_p), q0) } { loc(block_of(v_p), q0) }"),
            "an empty trigger set": self.uq(trigger="{ }"),
            "a trigger with an offset": self.uq(trigger="{ loc(block_of(v_p), q0 + 1) }"),
            "a trigger that is not a cell": self.uq(trigger="{ offset_of(loc(block_of(v_p), q0)) }"),
            "a trigger base that is not a variable": self.uq(trigger="{ loc(block_of(loc(block_of(v_p), 1)), q0) }"),
            "a trigger base that is not block_of(X)": self.uq(trigger="{ loc(v_p, q0) }"),
            "a trigger over another variable": self.uq(trigger="{ loc(block_of(v_p), v_n) }"),
            "a trigger with a term for one class of two": self.uq(binders="q0: Int, q1: Int", trigger="{ loc(block_of(v_p), q0) }"),
            "a trigger with a term twice for one class of two": self.uq(binders="q0: Int, q1: Int", trigger="{ loc(block_of(v_p), q0), loc(block_of(v_p), q0) }"),
            "a trigger with a term that is no binder's": self.uq(trigger="{ loc(block_of(v_p), q0), loc(block_of(v_p), q1) }"),
            "no type range": self.uq(ante="(0 <= (q0 - offset_of(v_p)))"),
            "no antecedent": f"forall q0: Int :: {{ loc(block_of(v_p), q0) }} {UserQuantifierTests.READ}",
            "a range that is no integer type's": self.uq(ante="(0 <= (q0 - offset_of(v_p)) && (q0 - offset_of(v_p)) <= 2)"),
            "a range of another variable": self.uq(ante="(-2147483648 <= v_n && v_n <= 2147483647)"),
            "a range over two expressions": self.uq(ante="(-2147483648 <= (q0 - offset_of(v_p)) && (q0 - offset_of(v_q)) <= 2147483647)"),
            "a computed loc in the body": self.uq(body="(loc(block_of(v_p), q0 + 1).v_int >= 0)"),
            "a computed loc in the antecedent": self.uq(ante=f"({UserQuantifierTests.RANGE} && (loc(block_of(v_p), q0 - 1).d_int))"),
            "acc in the body": self.uq(body="acc(loc(block_of(v_p), q0).v_int, amt)"),
            "acc in the antecedent": self.uq(ante="acc(loc(block_of(v_p), q0).v_int)"),
            "perm in the body": self.uq(body="(perm(loc(block_of(v_p), q0).v_int) > none)"),
            "forperm in the body": self.uq(body="(forperm k: Ref [k.v_int] :: true)"),
            "a forall in the body": self.uq(body=f"({self.uq(binders='q1: Int', trigger='{ loc(block_of(v_p), q1) }')})"),
            "a forall in the antecedent": self.uq(ante=f"({self.uq(binders='q1: Int', trigger='{ loc(block_of(v_p), q1) }')})"),
        }
        older = ("a binder named q without a number", "a binder named i", "a binder named k")  # not a user binder: the older rules refuse them
        for label, q in cases.items():
            for kind in ("requires", "ensures", "assert"):
                with self.subTest(mutant=label, kind=kind):
                    lines = self.method(requires=[q] if kind == "requires" else (), ensures=[q] if kind == "ensures" else (),
                                        body=[self.row(q)] if kind == "assert" else ())
                    self.assertTrue(self.probs(lines), label)
                    if label not in older:
                        self.refused_by_the_rule(lines, label)
        with self.subTest(mutant="a binder not named q in a loop invariant"):
            self.assertTrue(self.probs(self.method(body=self.loop([cases["a binder named i"]]))))

    def test_a_user_quantifier_stands_only_alone_in_a_clause(self):
        q = self.uq()
        cases = {
            "a conjunction": self.method(requires=[f"true && ({q})"]),
            "a disjunction": self.method(ensures=[f"v_n == 0 || ({q})"]),
            "a negation": self.method(requires=[f"!({q})"]),
            "a conjunct of an assert": self.method(body=[self.row(f"true && ({q})")]),
            "an exhale": self.method(body=[f"  exhale {q}"]),
            "an inhale": self.method(body=[f"  inhale {q} // cerastes:inhale-ok test"]),
            "a variable initializer": self.method(body=[f"  var b: Bool := {q}"]),
            "an if condition": self.method(body=[f"  if ({q}) {{", "  }"]),
            "a function's requires": ["function f_f(v_p: Ref, v_n: Int): Bool", f"  requires {q}", "{", "  true", "}"],
            "a function body": ["function f_f(v_p: Ref, v_n: Int): Bool", "{", f"  {q}", "}"],
        }
        for label, lines in cases.items():
            with self.subTest(case=label):
                self.refused(lines, "`forall`")
        for label in ("a conjunction", "a disjunction", "a negation", "a conjunct of an assert", "an exhale"):
            with self.subTest(rule=label):
                self.refused_by_the_rule(cases[label], label)

    def test_old_with_a_label_stays_the_loop_conjuncts_alone(self):
        q = self.uq(body="(loc(block_of(v_p), q0).v_int == old[lh_0](loc(block_of(v_p), q0).v_int))")
        self.refused(self.method(body=self.loop([q])), "old[")

    # --- the marked wildcard row
    def test_the_marked_wildcard_row_is_clean_as_an_unguarded_assert_with_its_marked_entry(self):
        self.clean(self.method(body=[self.row(self.wild(), ident=self.MARK, cat="permission")]))
        d = self.wild(cons="acc(loc(block_of(v_p), q0).d_int, wildcard)")
        self.clean(self.method(body=[self.row(d, ident=self.MARK, cat="permission")]))

    def test_the_wildcard_row_needs_its_marked_entry_and_only_it_may_have_one(self):
        self.refused(self.method(body=[self.row(self.wild(), ident="assert.failed", cat="permission")]), "has no expected reason")
        self.refused(self.method(body=[self.row(self.uq(), ident=self.MARK, cat="permission")]), "carries the expected reason")

    def test_the_twins_of_the_wildcard_row(self):
        w = self.wild()
        cases = {
            "in a requires": self.method(requires=[w]),
            "in an ensures": self.method(ensures=[w]),
            "in an invariant": self.method(body=self.loop([w])),
            "in an exhale": self.method(body=[f"  exhale {w}"]),
            "guarded": self.method(body=[self.row(f"(v_n != 0) ==> ({w})", ident=self.MARK, cat="permission")]),
            "wrapped": self.method(body=[self.row(f"({w})", ident=self.MARK, cat="permission")]),
            "a second permission": self.method(body=[self.row(self.wild(cons="acc(loc(block_of(v_p), q0).v_int, wildcard) && acc(loc(block_of(v_p), q0).d_int, wildcard)"), ident=self.MARK, cat="permission")]),
            "another amount": self.method(body=[self.row(self.wild(cons="acc(loc(block_of(v_p), q0).v_int, write)"), ident=self.MARK, cat="permission")]),
            "another base in the acc": self.method(body=[self.row(self.wild(cons="acc(loc(block_of(v_q), q0).v_int, wildcard)"), ident=self.MARK, cat="permission")]),
            "a reach that reads the heap": self.method(body=[self.row(self.wild(ante=f"({self.RANGE} && loc(block_of(v_p), q0).d_int)"), ident=self.MARK, cat="permission")]),
            "a reach that holds a permission": self.method(body=[self.row(self.wild(ante=f"({self.RANGE} && (perm(v_p.v_int) > none))"), ident=self.MARK, cat="permission")]),
            "a binder not named q": self.method(body=[self.row(self.wild(binders="x: Int", trigger="{ loc(block_of(v_p), x) }", cons="acc(loc(block_of(v_p), x).v_int, wildcard)").replace("q0", "x"), ident=self.MARK, cat="permission")]),
            "a heap trigger of another shape": self.method(body=[self.row(self.wild(trigger="{ loc(block_of(v_p), q0 + 1) }"), ident=self.MARK, cat="permission")]),
            "no type range": self.method(body=[self.row(f"forall q0: Int :: {{ loc(block_of(v_p), q0) }} {self.REACH} ==> acc(loc(block_of(v_p), q0).v_int, wildcard)", ident=self.MARK, cat="permission")]),
            "two binders": self.method(body=[self.row(self.wild(binders="q0: Int, q1: Int", trigger="{ loc(block_of(v_p), q0), loc(block_of(v_p), q1) }"), ident=self.MARK, cat="permission")]),
            "two trigger sets": self.method(body=[self.row(self.wild(trigger="{ loc(block_of(v_p), q0) } { loc(block_of(v_p), q0) }"), ident=self.MARK, cat="permission")]),
            "an implication in the consequent": self.method(body=[self.row(self.wild(cons="(v_n > 0 ==> acc(loc(block_of(v_p), q0).v_int, wildcard))"), ident=self.MARK, cat="permission")]),
        }
        for label, lines in cases.items():
            with self.subTest(mutant=label):
                self.assertTrue(self.probs(lines), label)
                if label not in ("a binder not named q", "in an exhale"):  # the older rules refuse these two
                    self.refused_by_the_rule(lines, label)
        # wildcard anywhere else stays the ghost-amount rule's
        for label, line in (("a Perm local", "  kq := wildcard"), ("a wildcard acc", "  exhale acc(v_p.v_int, wildcard)")):
            with self.subTest(case=label):
                self.refused(self.method(body=["  var kq: Perm", line]), "wildcard")


GHOST_FIXTURE_DOMAIN = ["// cerastes:domain-ok 0061", "domain Hvq_a {", "  function hvq_m__1__a_pre(s: Int): Int", "  function hvq_m__1__a_post(s: Int): Int", "}"]


class GhostDomainTests(unittest.TestCase):
    """Decision 0061 item 11: one domain per `f__wf` method, tagged `// cerastes:domain-ok 0061`, holding bodiless ghost functions and no axiom."""

    def lines(self, domain=None, wf=("m__1__wf", "hvq_m__1__a_pre(0)"), extra=()):
        domain = GHOST_FIXTURE_DOMAIN if domain is None else domain
        out = [*domain]
        if wf:
            name, use = wf
            out += [f"method {name}(v_a: Ref, v_n: Int)", "  decreases", "{", f"  var t: Int := {use}", "}"]
        return out + list(extra)

    @staticmethod
    def probs(lines):
        return [p for p in lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map") if "domain" in p or "ghost function" in p or "axiom" in p]

    def test_a_tagged_ghost_domain_used_by_one_wf_method_is_clean(self):
        self.assertEqual(self.probs(self.lines()), [])

    def test_the_twins_of_the_ghost_domain(self):
        d = GHOST_FIXTURE_DOMAIN
        cases = {
            "no tag": self.lines(domain=d[1:]),
            "another record": self.lines(domain=["// cerastes:domain-ok 0062"] + d[1:]),
            "a tag with no record": self.lines(domain=["// cerastes:domain-ok"] + d[1:]),
            "the memory model's record": self.lines(domain=["// cerastes:domain-ok 0028"] + d[1:]),
            "an axiom": self.lines(domain=d[:-1] + ["  axiom hvq_m__1__range { forall s: Int :: { hvq_m__1__a_pre(s) } hvq_m__1__a_pre(s) == 0 }", "}"]),
            "a tagged axiom": self.lines(domain=d[:-1] + ["  // cerastes:axiom-ok 0061", "  axiom hvq_m__1__range { forall s: Int :: { hvq_m__1__a_pre(s) } hvq_m__1__a_pre(s) == 0 }", "}"]),
            "a function with a body": self.lines(domain=d[:2] + ["  function hvq_m__1__a_pre(s: Int): Int { s }", "  function hvq_m__1__a_post(s: Int): Int", "}"]),
            "a function with a requires": self.lines(domain=d[:3] + ["    requires s > 0", "  function hvq_m__1__a_post(s: Int): Int", "}"]),
            "a function with an ensures": self.lines(domain=d[:3] + ["    ensures result > 0", "  function hvq_m__1__a_post(s: Int): Int", "}"]),
            "a Perm result": self.lines(domain=d[:2] + ["  function hvq_m__1__a_pre(s: Int): Perm", "  function hvq_m__1__a_post(s: Int): Int", "}"]),
            "a Rational result": self.lines(domain=d[:2] + ["  function hvq_m__1__a_pre(s: Int): Rational", "  function hvq_m__1__a_post(s: Int): Int", "}"]),
            "a Ref argument": self.lines(domain=d[:2] + ["  function hvq_m__1__a_pre(s: Ref): Int", "  function hvq_m__1__a_post(s: Int): Int", "}"]),
            "no argument": self.lines(domain=d[:2] + ["  function hvq_m__1__a_pre(): Int", "  function hvq_m__1__a_post(s: Int): Int", "}"]),
            "a unique function": self.lines(domain=d[:2] + ["  unique function hvq_m__1__a_pre(s: Int): Int", "  function hvq_m__1__a_post(s: Int): Int", "}"]),
            "a field inside": self.lines(domain=d[:-1] + ["  field f: Int", "}"]),
            "no function": self.lines(domain=["// cerastes:domain-ok 0061", "domain Hvq_a {", "}"], wf=None),
            "an unused domain": self.lines(wf=None),
            "a user in a method that is no __wf": self.lines(wf=("m__1", "hvq_m__1__a_pre(0)")),
            "a user in a function": self.lines(wf=None, extra=["function f_g(x: Int): Int", "{", "  hvq_m__1__a_pre(x)", "}"]),
            "two methods": self.lines(extra=["method m__2__wf(v_a: Ref, v_n: Int)", "  decreases", "{", "  var t: Int := hvq_m__1__a_post(1)", "}"]),
            "a ghost function named after another method": self.lines(domain=d[:2] + ["  function hvq_other__a_pre(s: Int): Int", "  function hvq_m__1__a_post(s: Int): Int", "}"], wf=("m__1__wf", "hvq_m__1__a_post(0)")),
            "an unqualified ghost function name": self.lines(domain=d[:2] + ["  function hvq_a_pre(s: Int): Int", "  function hvq_m__1__a_post(s: Int): Int", "}"], wf=("m__1__wf", "hvq_m__1__a_post(0)")),
            "a header with content": self.lines(domain=["// cerastes:domain-ok 0061", "domain Hvq_a { function hvq_m__1__a_pre(s: Int): Int",
                                                        "  function hvq_m__1__a_post(s: Int): Int", "}"], wf=("m__1__wf", "hvq_m__1__a_post(0)")),
            "a closing brace with content": self.lines(domain=d[:-1] + ["} method evil()"]),
            "a doubled closing brace": self.lines(domain=d[:-1] + ["}}"]),
            "two domains in one method": self.lines(
                domain=d + ["// cerastes:domain-ok 0061", "domain Hvq_b {", "  function hvq_m__1__b_pre(s: Int): Int", "}"],
                wf=("m__1__wf", "hvq_m__1__a_pre(0) + hvq_m__1__b_pre(0)")),
        }
        for label, lines in cases.items():
            with self.subTest(mutant=label):
                self.assertTrue(self.probs(lines), label)

    def test_ghost_functions_return_int_bool_or_ref(self):
        d = GHOST_FIXTURE_DOMAIN
        self.assertEqual(self.probs(self.lines(domain=d[:2] + ["  function hvq_m__1__a_pre(s: Int): Bool", "  function hvq_m__1__a_post(s: Int): Ref", "}"])), [])

    def test_a_tag_whose_record_is_missing_is_refused(self):
        with tempfile.TemporaryDirectory() as tmp:
            with mock.patch.object(lv, "DECISIONS_DIR", Path(tmp)):
                self.assertTrue(any("needs internal/decisions/0061" in p for p in self.probs(self.lines())))

    def test_a_ghost_domain_used_by_two_methods_of_one_domain_each_is_clean(self):
        d2 = ["// cerastes:domain-ok 0061", "domain Hvq_b {", "  function hvq_m__2__b_pre(s: Int): Int", "}"]
        lines = self.lines(domain=GHOST_FIXTURE_DOMAIN + d2, extra=["method m__2__wf(v_b: Ref, v_n: Int)", "  decreases", "{", "  var t: Int := hvq_m__2__b_pre(1)", "}"])
        self.assertEqual(self.probs(lines), [])


class WfQuantifierTests(unittest.TestCase):
    """Decision 0061 item 11, variant M of item 7: the heap-free `f__wf` method.  Its assumed facts (`requires` with a ghost function) have exactly four
    shapes, each checked against the atoms of the method it belongs to (audit round 1, B2): a false fact is a vacuous prior or a wrong pass.  Its
    `assert` rows are `[PRIOR ==>] (forall q<j> :: { G(q<j>), ... } ...)` over ghost applications, the prior standing outside the binder.  Every twin
    changes one thing of a clean fixture."""

    FUNCTIONS = [("a", "Int"), ("b", "Int"), ("g", "Int"), ("o", "Int"), ("w", "Int"), ("q", "Int"), ("z", "Int"), ("p", "Ref"), ("pi", "Int"),
                 ("bb", "Bool"), ("u", "Int"), ("m", "Int"), ("big", "Int"), ("oc", "Int")]
    DOMAIN = (["// cerastes:domain-ok 0061", "domain Hvq_all {"]
              + [f"  function hvq_m__1__{x}_{st}(s: Int): {ret}" for x, ret in FUNCTIONS for st in ("pre", "post")]
              + ["}"])
    FORMALS = "a: Ref, b: Ref, g: Ref, o: Ref, w: Ref, q: Ref, z: Ref, oc: Ref, p: Ref, pi: Ref, bb: Ref, u: Ref, m: Ref, big: Ref, v_n: Int, c: Int, amt: Perm"
    GUARD = "(c != 0)"

    @staticmethod
    def atom(x, field="v_int", amount="amt", guard=None, count="v_n"):
        acc = f"acc(loc(block_of({x}), k).{field}" + (f", {amount})" if amount else ")")
        row = f"forall k: Int :: {{ loc(block_of({x}), k) }} ((offset_of({x}) <= k) && (k < offset_of({x}) + {count})) ==> {acc}"
        return f"{guard} ==> ({row})" if guard else row

    @classmethod
    def base(cls, guard_g=GUARD, extra_req=(), extra_ens=()):
        """The method an `f__wf` belongs to: reads atoms on a, b, g (guarded), w (long), p, pi (pointers), bb (bool), an owns atom on o, a requires
        atom without a partner on q, a cell atom on z."""
        both = [cls.atom("a"), cls.atom("b"), cls.atom("g", guard=guard_g), cls.atom("w", "v_long"), cls.atom("p", "v_ptr_int"),
                cls.atom("pi", "v_ptr_int"), cls.atom("bb", "v_bool"), cls.atom("o", amount=None), cls.atom("big", "v_i128"), "acc(z.v_int, amt)", "acc(oc.v_int)"]
        reqs = [*both, cls.atom("q"), cls.atom("m"), *extra_req]  # m: a requires atom of one field whose partner is of another
        ens = [*both, cls.atom("m", "v_long"), *extra_ens]
        lines = ["// cerastes:trusted-contract test", f"method m__1({cls.FORMALS})", Check("  requires none < amt && amt < write", WF, "internal")]
        lines += [Check(f"  requires {r}", WF, "internal") for r in reqs]
        lines += [Check(f"  ensures {e}", P, "postcondition") for e in ens]
        return lines + ["  decreases _"]

    RANGE_A = "forall s: Int :: { hvq_m__1__a_pre(s) } (-2147483648 <= hvq_m__1__a_pre(s)) && (hvq_m__1__a_pre(s) <= 2147483647)"
    TIE = "hvq_m__1__a_post(offset_of(a) + 0) == hv_0_post"
    SAME = "(block_of(a) == block_of(b)) ==> (forall s: Int :: { hvq_m__1__a_pre(s) } { hvq_m__1__b_pre(s) } hvq_m__1__a_pre(s) == hvq_m__1__b_pre(s))"
    SHAPE = "((a != null) && (0 <= v_n))"
    CLAUSE = "forall s: Int :: { hvq_m__1__a_pre(s) } (-2147483648 <= (s - offset_of(a)) && (s - offset_of(a)) <= 2147483647) ==> (!(0 <= (s - offset_of(a)) && (s - offset_of(a)) < v_n) || (hvq_m__1__a_pre(s) < 100))"
    OBL = "!(((hvq_m__1__a_pre(q0) + 1) < -2147483648) || (2147483647 < (hvq_m__1__a_pre(q0) + 1)))"

    @staticmethod
    def keep(x="a", n="v_n", lo=None, post=None, pre=None):
        lo = f"offset_of({x})" if lo is None else lo
        post = f"hvq_m__1__{x}_post" if post is None else post
        pre = f"hvq_m__1__{x}_pre" if pre is None else pre
        return (f"forall s: Int :: {{ {post}(s) }} {{ {pre}(s) }} (({lo} <= s) && (s < offset_of({x}) + {n})) ==> ({post}(s) == {pre}(s))")

    def oq(self, obl=None, trigger="{ hvq_m__1__a_pre(q0) }", binders="q0: Int", ante=None):
        ante = ("((-2147483648 <= (q0 - offset_of(a)) && (q0 - offset_of(a)) <= 2147483647) && "
                "(0 <= (q0 - offset_of(a)) && (q0 - offset_of(a)) < v_n))") if ante is None else ante
        return f"forall {binders} :: {trigger} {ante} ==> {self.OBL if obl is None else obl}"

    def wf(self, requires=(), rows=(), ensures=(), base=None, tie=True):
        lines = [*self.DOMAIN, *(self.base() if base is None else base),
                 f"method m__1__wf({self.FORMALS}, res: Int, hv_0_post: Int, hv_0_pre: Int, hv_1_pre: Int)"]
        # every fixture uses the domain (a ghost function in a fact), so that a twin is refused for its own shape and never for an unused domain
        lines += [Check(f"  requires {r}", WF, "internal") for r in ((self.TIE,) if tie else ()) + tuple(requires)]
        lines += [Check(f"  ensures {e}", P, "postcondition") for e in ensures]
        lines += ["  decreases", "{"] + [Check(f"  assert {r}", A, "overflow") for r in rows] + ["}"]
        return lines

    @staticmethod
    def probs(lines):
        return [p for p in lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")
                if any(w in p for w in ("decision 0061", "`forall`", "quantified", "domain", "ghost function", "axiom"))]

    def clean(self, lines):
        self.assertEqual(self.probs(lines), [])

    @staticmethod
    def verdicts(lines):
        text, _ = build(lines)
        src = lv.split_source(text)
        return [(n, v[0]) for n, v in sorted(lv.classify_quantified_lines(src, lv.declarations(src)).verdicts.items())]

    def refused(self, lines, older=False):
        """Refused, and (unless the older rules own the shape) by item 11's own verdict on a line."""
        self.assertTrue(self.probs(lines), lines[-4:])
        if not older:
            self.assertTrue(any(not ok for _n, ok in self.verdicts(lines)))

    # --- the four assumed facts, each against the atoms of the method
    def test_the_assumed_facts_are_clean(self):
        guard = f"({self.GUARD} && {self.GUARD})"
        self.clean(self.wf(requires=[
            self.RANGE_A, self.RANGE_A.replace("hvq_m__1__a_pre", "hvq_m__1__a_post"),
            f"forall s: Int :: {{ hvq_m__1__p_pre(s) }} {lv._view_wf('hvq_m__1__p_pre(s)')}",
            # (b) the scalar tie of a state: any index over the formals
            "hvq_m__1__a_pre(offset_of(a) + 0) == hv_0_pre", "hvq_m__1__a_pre(offset_of(a)) == hv_0_pre", "hvq_m__1__a_pre(offset_of(a) + (v_n - 1)) == hv_1_pre",
            "hvq_m__1__b_post(offset_of(b) + (c + 1)) == hv_0_post",
            # (c) two parameters on one block with the same cell field
            self.SAME, self.SAME.replace("_pre", "_post"),
            # (d) a reads place keeps its value: unguarded, and under the two atoms' guards
            self.keep(), f"{guard} ==> ({self.keep('g')})", f"hvq_m__1__z_post(offset_of(z)) == hvq_m__1__z_pre(offset_of(z))".replace("hvq_m__1__z_post(offset_of(z)) == hvq_m__1__z_pre(offset_of(z))", "hvq_m__1__z_post(offset_of(z)) == hvq_m__1__z_pre(offset_of(z))"),
        ]))

    def test_the_type_range_is_the_element_types_not_any_range_with_zero(self):
        def fact(ghost, lo, hi):
            return f"forall s: Int :: {{ {ghost}(s) }} ({lo} <= {ghost}(s)) && ({ghost}(s) <= {hi})"
        # the element field of `a` is v_int: its range is int's, whatever the ghost function's other uses
        self.clean(self.wf(requires=[fact("hvq_m__1__a_pre", -2**31, 2**31 - 1), fact("hvq_m__1__w_pre", -2**63, 2**63 - 1), fact("hvq_m__1__big_pre", -2**127, 2**127 - 1)]))
        for label, f in {
            "a range that holds zero but is no int range": fact("hvq_m__1__a_pre", 0, 2),
            "a range that excludes zero": fact("hvq_m__1__a_pre", 1, 5),
            "another type's standard range": fact("hvq_m__1__a_pre", 0, 2**32 - 1),
            "a narrower integer type's range": fact("hvq_m__1__a_pre", -128, 127),
            "the int range for a long field": fact("hvq_m__1__w_pre", -2**31, 2**31 - 1),
            "one bound": "forall s: Int :: { hvq_m__1__a_pre(s) } hvq_m__1__a_pre(s) <= 5",
            "an equality": "forall s: Int :: { hvq_m__1__a_pre(s) } hvq_m__1__a_pre(s) == 0",
            "a bound that is no literal": "forall s: Int :: { hvq_m__1__a_pre(s) } (-2147483648 <= hvq_m__1__a_pre(s)) && (hvq_m__1__a_pre(s) <= v_n)",
            "another function in the body": "forall s: Int :: { hvq_m__1__a_pre(s) } (-2147483648 <= hvq_m__1__b_pre(s)) && (hvq_m__1__b_pre(s) <= 2147483647)",
            "a trigger that is not the application": "forall s: Int :: { hvq_m__1__a_post(s) } (-2147483648 <= hvq_m__1__a_pre(s)) && (hvq_m__1__a_pre(s) <= 2147483647)",
            "a trigger that is no ghost function": "forall s: Int :: { len(s) } (-2147483648 <= len(s)) && (len(s) <= 2147483647)",
            "a guard on a range": f"(c != 0) ==> ({self.RANGE_A})",
            "an antecedent": "forall s: Int :: { hvq_m__1__a_pre(s) } (0 <= s) ==> ((-2147483648 <= hvq_m__1__a_pre(s)) && (hvq_m__1__a_pre(s) <= 2147483647))",
            "a pointer element with an integer range": fact("hvq_m__1__p_pre", -2**31, 2**31 - 1),
            "a Bool element with a range": fact("hvq_m__1__bb_pre", 0, 1),
            "a Bool element with an integer range": fact("hvq_m__1__bb_pre", -2**31, 2**31 - 1),
            "a 128-bit field with a range that holds zero": fact("hvq_m__1__big_pre", 0, 2),
            "a pointer ghost declared Int": f"forall s: Int :: {{ hvq_m__1__pi_pre(s) }} {lv._view_wf('hvq_m__1__pi_pre(s)')}",
            "a base with no atom": fact("hvq_m__1__u_pre", -2**31, 2**31 - 1),
            "a function that is no hvq function": fact("g_pre", -2**31, 2**31 - 1),
        }.items():
            with self.subTest(mutant=label):
                self.refused(self.wf(requires=[f]))

    def test_a_scalar_tie_joins_one_state_over_an_index_of_the_formals(self):
        for label, f in {
            # the auditor's state-crossing tie (audit1/abl/tie_state_cross_vacuous.vpr): a pre function tied to a POST scalar ghost
            "a pre function against a post ghost": "hvq_m__1__a_pre(offset_of(a) + 0) == hv_0_post",
            "a post function against a pre ghost": "hvq_m__1__a_post(offset_of(a) + 0) == hv_0_pre",
            "a ghost that is no formal": "hvq_m__1__a_post(offset_of(a) + 0) == hv_9_post",
            "a variable that is no scalar ghost": "hvq_m__1__a_post(offset_of(a) + 0) == v_n",
            "a ghost with no state suffix": "hvq_m__1__a_post(offset_of(a) + 0) == res",
            "a slot of another base": "hvq_m__1__a_post(offset_of(b) + 0) == hv_0_post",
            "an index that is no offset": "hvq_m__1__a_post(v_n) == hv_0_post",
            "an index over a name that is no formal": "hvq_m__1__a_post(offset_of(a) + zzz) == hv_0_post",
            "an index that reads the heap": "hvq_m__1__a_post(offset_of(a) + a.v_int) == hv_0_post",
            "an index with a quantifier": f"hvq_m__1__a_post(offset_of(a) + ({self.RANGE_A})) == hv_0_post",
            "a base with no atom": "hvq_m__1__u_post(offset_of(u) + 0) == hv_0_post",
            "a value that is a literal": "hvq_m__1__a_post(offset_of(a) + 0) == 5",
            "an inequality": "hvq_m__1__a_post(offset_of(a) + 0) <= hv_0_post",
            "a guarded tie": "(c != 0) ==> (hvq_m__1__a_post(offset_of(a) + 0) == hv_0_post)",
            "an application outside a fact": "hvq_m__1__a_post(v_n) > 0",
        }.items():
            with self.subTest(mutant=label):
                self.refused(self.wf(requires=[f]))
        # the crossing tie beside the right one is what makes a prior vacuous: the lint refuses the pair
        self.refused(self.wf(requires=["hvq_m__1__a_pre(offset_of(a) + 0) == hv_0_pre", "hvq_m__1__a_pre(offset_of(a) + 0) == hv_0_post"]))

    def test_a_keep_fact_is_tied_to_a_reads_atom_its_partner_and_their_guards(self):
        guard = f"({self.GUARD} && {self.GUARD})"
        for label, f in {
            # the unsound one S45 proved (s45_d_m_unsound_evidence_unguarded_keep): the guarded atoms, an unguarded fact
            "an unguarded fact on guarded atoms": self.keep("g"),
            "a guard of one atom on guarded atoms": f"{self.GUARD} ==> ({self.keep('g')})",
            "a guard on unguarded atoms": f"{guard} ==> ({self.keep()})",
            "another guard": f"((c != 1) && (c != 1)) ==> ({self.keep('g')})",
            "a wider guard": f"({guard} || (v_n == 0)) ==> ({self.keep('g')})",
            "a guard that reads the heap": f"(a.v_int != 0) ==> ({self.keep()})",
            "a guard with a permission": f"(perm(a.v_int) > none) ==> ({self.keep()})",
            "a guard with a quantifier": f"({self.RANGE_A}) ==> ({self.keep()})",
            "an owns atom": self.keep("o"),
            "an atom with no partner": self.keep("q"),
            "an atom whose partner is of another field": self.keep("m"),
            "a base with no atom": self.keep("u"),
            "a count other than the atoms'": self.keep(n="(v_n + 5)"),
            "a count that is another variable": self.keep(n="c"),
            "a low bound of zero": self.keep(lo="0"),
            "another base": self.keep().replace("offset_of(a) <= s", "offset_of(b) <= s"),
            "the states of two bases": "forall s: Int :: { hvq_m__1__a_post(s) } { hvq_m__1__b_pre(s) } ((offset_of(a) <= s) && (s < offset_of(a) + v_n)) ==> (hvq_m__1__a_post(s) == hvq_m__1__b_pre(s))",
            "two pre states": "forall s: Int :: { hvq_m__1__a_pre(s) } { hvq_m__1__b_pre(s) } ((offset_of(a) <= s) && (s < offset_of(a) + v_n)) ==> (hvq_m__1__a_pre(s) == hvq_m__1__b_pre(s))",
            "one trigger set": "forall s: Int :: { hvq_m__1__a_post(s) } ((offset_of(a) <= s) && (s < offset_of(a) + v_n)) ==> (hvq_m__1__a_post(s) == hvq_m__1__a_pre(s))",
            "the pre set only": "forall s: Int :: { hvq_m__1__a_pre(s) } ((offset_of(a) <= s) && (s < offset_of(a) + v_n)) ==> (hvq_m__1__a_post(s) == hvq_m__1__a_pre(s))",
            "a set holding both": "forall s: Int :: { hvq_m__1__a_post(s), hvq_m__1__a_pre(s) } ((offset_of(a) <= s) && (s < offset_of(a) + v_n)) ==> (hvq_m__1__a_post(s) == hvq_m__1__a_pre(s))",
            "a set twice": "forall s: Int :: { hvq_m__1__a_post(s) } { hvq_m__1__a_post(s) } ((offset_of(a) <= s) && (s < offset_of(a) + v_n)) ==> (hvq_m__1__a_post(s) == hvq_m__1__a_pre(s))",
            "a count that names s": self.keep(n="s"),
            "a count that reads the heap": self.keep(n="a.v_int"),
            "an addition": self.keep().replace("hvq_m__1__a_post(s) == hvq_m__1__a_pre(s))", "hvq_m__1__a_post(s) == hvq_m__1__a_pre(s) + 1)"),
            "an inequality": self.keep().replace("hvq_m__1__a_post(s) == hvq_m__1__a_pre(s))", "hvq_m__1__a_post(s) <= hvq_m__1__a_pre(s))"),
            "a heap trigger": self.keep().replace("{ hvq_m__1__a_post(s) } { hvq_m__1__a_pre(s) }", "{ loc(block_of(a), s) }"),
            "a binder named k": self.keep().replace("s: Int", "k: Int").replace("(s)", "(k)").replace("<= s", "<= k").replace("(s <", "(k <"),
            "functions that are not hvq functions": "forall s: Int :: { g_post(s) } { g_pre(s) } ((offset_of(a) <= s) && (s < offset_of(a) + v_n)) ==> (g_post(s) == g_pre(s))",
        }.items():
            with self.subTest(mutant=label):
                self.refused(self.wf(requires=[f]), older=label == "a binder named k")

    def test_a_kept_cell_is_tied_to_a_reads_atom_its_partner_and_their_guards(self):
        z = "hvq_m__1__z_post(offset_of(z)) == hvq_m__1__z_pre(offset_of(z))"
        self.clean(self.wf(requires=[z, z.replace("(offset_of(z))", "(offset_of(z) + 0)")]))
        for label, f in {
            "a guard the atoms do not have": f"({self.GUARD} && {self.GUARD}) ==> ({z})",
            "the pre function first": "hvq_m__1__z_pre(offset_of(z)) == hvq_m__1__z_post(offset_of(z))",
            "another slot": "hvq_m__1__z_post(offset_of(z) + 1) == hvq_m__1__z_pre(offset_of(z) + 1)",
            "two slots": "hvq_m__1__z_post(offset_of(z)) == hvq_m__1__z_pre(offset_of(z) + 1)",
            "two bases": "hvq_m__1__z_post(offset_of(z)) == hvq_m__1__a_pre(offset_of(z))",
            "two post functions": "hvq_m__1__z_post(offset_of(z)) == hvq_m__1__z_post(offset_of(z))",
            "a base with no cell atom": "hvq_m__1__a_post(offset_of(a)) == hvq_m__1__a_pre(offset_of(a))",
            "an owns cell": "hvq_m__1__o_post(offset_of(o)) == hvq_m__1__o_pre(offset_of(o))",
        }.items():
            with self.subTest(mutant=label):
                self.refused(self.wf(requires=[f]))

    def test_a_formals_field_comes_from_its_cell_atoms_too(self):
        """`reads(*p)` and `owns(*p)` atoms give the element field of a formal whose only atoms are cells (decision-owner change after Part B's probes)."""
        def fact(ghost, lo, hi):
            return f"forall s: Int :: {{ {ghost}(s) }} ({lo} <= {ghost}(s)) && ({ghost}(s) <= {hi})"
        guard = f"({self.GUARD} && {self.GUARD})"
        self.clean(self.wf(requires=[
            fact("hvq_m__1__z_pre", -2**31, 2**31 - 1), fact("hvq_m__1__z_post", -2**31, 2**31 - 1), fact("hvq_m__1__oc_pre", -2**31, 2**31 - 1),
            "hvq_m__1__z_post(offset_of(z) + 0) == hv_0_post", "hvq_m__1__z_pre(offset_of(z)) == hv_0_pre", "hvq_m__1__oc_post(offset_of(oc)) == hv_0_post",
            "(block_of(a) == block_of(z)) ==> (forall s: Int :: { hvq_m__1__a_pre(s) } { hvq_m__1__z_pre(s) } hvq_m__1__a_pre(s) == hvq_m__1__z_pre(s))",
            "(block_of(z) == block_of(oc)) ==> (forall s: Int :: { hvq_m__1__z_post(s) } { hvq_m__1__oc_post(s) } hvq_m__1__z_post(s) == hvq_m__1__oc_post(s))",
            "hvq_m__1__z_post(offset_of(z)) == hvq_m__1__z_pre(offset_of(z))"]))
        for label, f in {
            "a range that is no int range for a cell of an int field": fact("hvq_m__1__z_pre", 0, 2),
            "the long range for a cell of an int field": fact("hvq_m__1__oc_pre", -2**63, 2**63 - 1),
            "a cell and a long field on one block": "(block_of(z) == block_of(w)) ==> (forall s: Int :: { hvq_m__1__z_pre(s) } { hvq_m__1__w_pre(s) } hvq_m__1__z_pre(s) == hvq_m__1__w_pre(s))",
            "a cell and a pointer field on one block": "(block_of(oc) == block_of(p)) ==> (forall s: Int :: { hvq_m__1__oc_pre(s) } { hvq_m__1__p_pre(s) } hvq_m__1__oc_pre(s) == hvq_m__1__p_pre(s))",
            "a tie in the wrong state for a cell": "hvq_m__1__z_pre(offset_of(z)) == hv_0_post",
            "no keep for an owns cell": "hvq_m__1__oc_post(offset_of(oc)) == hvq_m__1__oc_pre(offset_of(oc))",
            "a keep for a cell under a guard its atoms do not have": f"{guard} ==> (hvq_m__1__z_post(offset_of(z)) == hvq_m__1__z_pre(offset_of(z)))",
        }.items():
            with self.subTest(mutant=label):
                self.refused(self.wf(requires=[f]))

    def test_a_formal_without_an_atom_takes_its_field_from_its_quantified_read(self):
        """Audit round 2, issue 1: `q == p` with a quantified read through q (q has no atom; p's owns_range frames it): the field of q's type range is the
        field of its read `loc(block_of(q), q0).v_int` in the method's own clause."""
        read = ("forall q0: Int :: { loc(block_of(al), q0) } (-2147483648 <= (q0 - offset_of(al)) && (q0 - offset_of(al)) <= 2147483647) ==> "
                "(!(0 <= (q0 - offset_of(al)) && (q0 - offset_of(al)) < v_n) || (loc(block_of(al), q0).v_int >= 0))")
        base = self.base(extra_req=[read])
        base[1] = base[1].replace("amt: Perm)", "amt: Perm, al: Ref)")
        lines = lambda req: [l.replace("amt: Perm,", "amt: Perm, al: Ref,") if isinstance(l, str) and l.startswith("method m__1__wf") else l
                             for l in self.wf(requires=req, base=base)]
        dom = lambda l: [x.replace("function hvq_m__1__u_pre(s: Int): Int", "function hvq_m__1__al_pre(s: Int): Int\n  function hvq_m__1__u_pre(s: Int): Int") if isinstance(x, str) else x for x in l]
        good = "forall s: Int :: { hvq_m__1__al_pre(s) } (-2147483648 <= hvq_m__1__al_pre(s)) && (hvq_m__1__al_pre(s) <= 2147483647)"
        bad = good.replace("-2147483648", "0").replace("2147483647", "2")
        self.assertEqual(self.probs(dom(lines([good]))), [])
        self.assertTrue(self.probs(dom(lines([bad]))))

    def test_a_ghost_function_is_named_after_its_own_method(self):
        range_fact = "forall s: Int :: {{ {g}(s) }} (-2147483648 <= {g}(s)) && ({g}(s) <= 2147483647)"
        self.clean(self.wf(requires=[range_fact.format(g="hvq_m__1__a_pre")]))
        for label, g in {
            "another method's name": "hvq_m__2__a_pre",
            "no method name (the unqualified form collides between functions)": "hvq_a_pre",
            "a method name that is a prefix": "hvq_m__a_pre",
        }.items():
            with self.subTest(mutant=label):
                lines = self.wf(requires=[range_fact.format(g=g)])
                lines = [l.replace("function hvq_m__1__a_pre(", f"function {g}(") if isinstance(l, str) else l for l in lines]
                self.refused(lines)

    def test_two_parameters_on_one_block_need_the_same_cell_field(self):
        for label, f in {
            "another field (int and long)": "(block_of(a) == block_of(w)) ==> (forall s: Int :: { hvq_m__1__a_pre(s) } { hvq_m__1__w_pre(s) } hvq_m__1__a_pre(s) == hvq_m__1__w_pre(s))",
            "a pointer and an integer field": "(block_of(a) == block_of(p)) ==> (forall s: Int :: { hvq_m__1__a_pre(s) } { hvq_m__1__p_pre(s) } hvq_m__1__a_pre(s) == hvq_m__1__p_pre(s))",
            "a base with no atom": "(block_of(a) == block_of(u)) ==> (forall s: Int :: { hvq_m__1__a_pre(s) } { hvq_m__1__u_pre(s) } hvq_m__1__a_pre(s) == hvq_m__1__u_pre(s))",
            "one trigger set": "(block_of(a) == block_of(b)) ==> (forall s: Int :: { hvq_m__1__a_pre(s) } hvq_m__1__a_pre(s) == hvq_m__1__b_pre(s))",
            "two states": "(block_of(a) == block_of(b)) ==> (forall s: Int :: { hvq_m__1__a_pre(s) } { hvq_m__1__b_post(s) } hvq_m__1__a_pre(s) == hvq_m__1__b_post(s))",
            "a guard of other bases": "(block_of(a) == block_of(v_n)) ==> (forall s: Int :: { hvq_m__1__a_pre(s) } { hvq_m__1__b_pre(s) } hvq_m__1__a_pre(s) == hvq_m__1__b_pre(s))",
            "no guard": "forall s: Int :: { hvq_m__1__a_pre(s) } { hvq_m__1__b_pre(s) } hvq_m__1__a_pre(s) == hvq_m__1__b_pre(s)",
            "a body that is no equality": "(block_of(a) == block_of(b)) ==> (forall s: Int :: { hvq_m__1__a_pre(s) } { hvq_m__1__b_pre(s) } hvq_m__1__a_pre(s) <= hvq_m__1__b_pre(s))",
            "a nested quantifier": f"(block_of(a) == block_of(b)) ==> (forall s: Int :: {{ hvq_m__1__a_pre(s) }} {{ hvq_m__1__b_pre(s) }} ({self.RANGE_A}))",
            "an acc": "forall s: Int :: { hvq_m__1__a_pre(s) } acc(a.v_int)",
        }.items():
            with self.subTest(mutant=label):
                self.refused(self.wf(requires=[f]))
        # the same two parameters, the same field: clean (the clean list holds the pair; here with the post states of the long field)
        self.clean(self.wf())

    def test_a_ghost_application_or_a_quantifier_in_a_clause_that_is_no_fact_is_refused(self):
        self.refused(self.wf(ensures=["hvq_m__1__a_pre(0) > 0"]))
        self.refused(self.wf(ensures=[self.RANGE_A]))
        self.refused(self.wf(requires=["hvq_m__1__a_pre(0) > 0"]))

    # --- the obligation rows
    def test_the_obligation_rows_are_clean_alone_or_after_a_prior(self):
        prior = f"({self.SHAPE} && ({self.CLAUSE}))"
        guarded = f"({self.SHAPE} && ((c != 0) ==> ({self.CLAUSE})))"
        two = self.oq(binders="q0: Int, q1: Int", trigger="{ hvq_m__1__a_pre(q0), hvq_m__1__b_pre(q1) }",
                      ante="((-2147483648 <= (q0 - offset_of(a)) && (q0 - offset_of(a)) <= 2147483647) && (q1 == offset_of(b) + (q0 - offset_of(a))))")
        prior_two = f"({self.SHAPE} && ({self.CLAUSE}) && (forall s0: Int, s1: Int :: {{ hvq_m__1__a_pre(s0), hvq_m__1__b_pre(s1) }} (-2147483648 <= (s0 - offset_of(a)) && (s0 - offset_of(a)) <= 2147483647) && (s1 == offset_of(b) + (s0 - offset_of(a))) ==> (hvq_m__1__a_pre(s0) == hvq_m__1__b_pre(s1))))"
        inside = self.oq(ante=f"(({self.SHAPE}) && (-2147483648 <= (q0 - offset_of(a)) && (q0 - offset_of(a)) <= 2147483647) && (0 <= (q0 - offset_of(a)) && (q0 - offset_of(a)) < v_n))")
        self.clean(self.wf(rows=[self.oq(), f"{self.SHAPE} ==> ({self.oq()})", f"{prior} ==> ({self.oq()})", f"{guarded} ==> ({self.oq()})",
                                 f"{prior} ==> !((hv_0_post + 1) < -2147483648)", f"{self.SHAPE} ==> ({two})", f"{prior_two} ==> ({two})",
                                 f"{self.SHAPE} ==> ((c != 0) ==> ({self.oq()}))", f"{self.SHAPE} ==> !(hv_0_post < 0)", inside]))

    def test_a_class_read_in_both_states_has_one_trigger_set_per_state(self):
        """Question 257: `a[i]` and `old(a[i])` in one clause: the clause's rows (and the prior's quantifiers) carry `{ pre } { post }`; a row whose body reads
        one state or neither (a framing row) keeps both sets: triggers restrict instantiation and never prove anything."""
        both = "{ hvq_m__1__a_pre(q0) } { hvq_m__1__a_post(q0) }"
        prior = f"({self.SHAPE} && (forall s: Int :: {{ hvq_m__1__a_pre(s) }} {{ hvq_m__1__a_post(s) }} (-2147483648 <= (s - offset_of(a)) && (s - offset_of(a)) <= 2147483647) ==> (hvq_m__1__a_post(s) == hvq_m__1__a_pre(s))))"
        two = self.oq(binders="q0: Int, q1: Int", trigger="{ hvq_m__1__a_pre(q0), hvq_m__1__b_pre(q1) } { hvq_m__1__a_post(q0), hvq_m__1__b_pre(q1) }",
                      ante="((-2147483648 <= (q0 - offset_of(a)) && (q0 - offset_of(a)) <= 2147483647) && (q1 == offset_of(b) + (q0 - offset_of(a))))",
                      obl="(hvq_m__1__a_post(q0) <= hvq_m__1__b_pre(q1) + hvq_m__1__a_pre(q0))")
        self.clean(self.wf(rows=[self.oq(obl="(hvq_m__1__a_post(q0) == hvq_m__1__a_pre(q0))", trigger=both), self.oq(obl="true", trigger=both),
                                 self.oq(obl="(hvq_m__1__a_post(q0) >= 0)", trigger=both), f"{prior} ==> ({self.oq(trigger=both)})", two]))

    def test_the_twins_of_the_obligation_rows(self):
        for label, row in {
            "a heap trigger": self.oq(trigger="{ loc(block_of(a), q0) }"),
            "a trigger that is not an application of a ghost function": self.oq(trigger="{ offset_of(q0) }"),
            "an application of a function that is no ghost function": self.oq(trigger="{ len(q0) }"),
            "an application to a non-binder": self.oq(trigger="{ hvq_m__1__a_pre(v_n) }"),
            "a trigger with an offset": self.oq(trigger="{ hvq_m__1__a_pre(q0 + 1) }"),
            "no trigger set": self.oq(trigger=""),
            "two trigger sets naming different formals": self.oq(trigger="{ hvq_m__1__a_pre(q0) } { hvq_m__1__b_post(q0) }"),
            "two trigger sets naming different methods": self.oq(trigger="{ hvq_m__1__a_pre(q0) } { hvq_m__2__a_post(q0) }"),
            "a second trigger set with a computed term": self.oq(trigger="{ hvq_m__1__a_pre(q0) } { hvq_m__1__a_post(q0 + 1) }"),
            "a second trigger set of the same state": self.oq(trigger="{ hvq_m__1__a_pre(q0) } { hvq_m__1__a_pre(q0) }"),
            "a second trigger set that is no ghost function": self.oq(trigger="{ hvq_m__1__a_pre(q0) } { len(q0) }"),
            "a trigger set with a term twice for its binder": self.oq(trigger="{ hvq_m__1__a_pre(q0), hvq_m__1__a_pre(q0) }"),
            "a second trigger set that misses a binder": self.oq(binders="q0: Int, q1: Int", trigger="{ hvq_m__1__a_pre(q0), hvq_m__1__b_pre(q1) } { hvq_m__1__a_post(q0) }"),
            "a second trigger set with a term twice": self.oq(binders="q0: Int, q1: Int", trigger="{ hvq_m__1__a_pre(q0), hvq_m__1__b_pre(q1) } { hvq_m__1__a_post(q0), hvq_m__1__a_post(q0) }"),
            "a term for one class of two": self.oq(binders="q0: Int, q1: Int", trigger="{ hvq_m__1__a_pre(q0) }"),
            "a binder named k": self.oq(binders="k: Int", trigger="{ hvq_m__1__a_pre(k) }", obl="true", ante="(0 <= k)"),
            "a binder that is not Int": self.oq(binders="q0: Bool"),
            "a body that reads the heap": self.oq(obl="(loc(block_of(a), q0).v_int >= 0)"),
            "a body with old": self.oq(obl="(old(hvq_m__1__a_pre(q0)) >= 0)"),
            "a body with acc": self.oq(obl="acc(a.v_int)"),
            "a body with perm": self.oq(obl="(perm(a.v_int) > none)"),
            "a nested quantifier in the body": self.oq(obl=f"({self.CLAUSE})"),
            "the prior's quantifier inside the antecedent": self.oq(ante=f"(({self.CLAUSE}) && (0 <= (q0 - offset_of(a))))"),
            "no type range in the antecedent": self.oq(ante="(0 <= (q0 - offset_of(a)))"),
            "a type range that is no integer type's": self.oq(ante="((0 <= (q0 - offset_of(a)) && (q0 - offset_of(a)) <= 2) && (0 <= (q0 - offset_of(a))))"),
            "a type range over another variable": self.oq(ante="((-2147483648 <= v_n && v_n <= 2147483647) && (0 <= (q0 - offset_of(a))))"),
            "a computed application in the body": self.oq(obl="(hvq_m__1__a_pre(q0 + 1) >= 0)"),
            "a quantifier as a conjunct of the conclusion": f"{self.SHAPE} ==> (true && ({self.oq()}))",
            "a quantifier in a disjunction of the prior": f"({self.SHAPE} || ({self.CLAUSE})) ==> ({self.oq()})",
            "a quantifier under a negation in the prior": f"(!({self.CLAUSE})) ==> ({self.oq()})",
            "a prior quantifier with acc in its body": f"({self.SHAPE} && (forall s: Int :: {{ hvq_m__1__a_pre(s) }} acc(a.v_int))) ==> ({self.oq()})",
            "a prior quantifier over the heap": f"({self.SHAPE} && (forall s: Int :: {{ loc(block_of(a), s) }} loc(block_of(a), s).v_int >= 0)) ==> ({self.oq()})",
            "a prior condition that reads the heap": f"(a.v_int > 0) ==> ({self.oq()})",
            "a prior mixing && and || without parentheses": f"({self.SHAPE} && (c != 0) || ({self.CLAUSE})) ==> ({self.oq()})",
            "a quantifier outside a conjunction in a plain row": f"true && ({self.oq()})",
            "a prior quantifier with a guard that holds a quantifier": f"({self.SHAPE} && (({self.CLAUSE}) ==> ({self.CLAUSE}))) ==> ({self.oq()})",
        }.items():
            with self.subTest(mutant=label):
                self.refused(self.wf(rows=[row]), older=label == "a binder named k")
        # a quantifier elsewhere in an f__wf method
        self.refused(self.wf(ensures=[self.oq()]))
        lines = self.wf()
        lines[-1:-1] = ["  while (false)", f"    invariant {self.oq()}", "  {", "  }"]
        self.refused(lines)

    def test_a_user_quantifier_over_the_heap_in_a_wf_method_is_refused_and_a_ghost_one_outside_it(self):
        heap = "forall q0: Int :: { loc(block_of(a), q0) } (0 <= q0) ==> (loc(block_of(a), q0).v_int >= 0)"
        self.refused(self.wf(rows=[heap]))
        # the same ghost quantifier in a method that is no __wf is refused (the domain's functions are used there)
        outer = [*self.DOMAIN, "method m__1(a: Ref, v_n: Int)", "  decreases", "{", f"  assert {self.oq()}", "}"]
        self.refused(outer)


class Row:
    """A line of a fixture with several position-map entries (an `ensures` of a `pure` function has two: its user row and its internal one)."""

    def __init__(self, text, *entries):
        self.text, self.entries = text, entries


def build_rows(lines, wf_stubs=True):
    """(vpr text, map text) of a fixture of strings, `Check`s (one entry) and `Row`s (several).  Decision 0062 item 9 (S3): every `pf_<name>__<id>`
    function has its `__wf` method; a fixture that is not about that rule gets an empty one appended after its last line (`wf_stubs=False` leaves the
    fixture as written)."""
    if wf_stubs:
        texts = [i if isinstance(i, str) else i.text for i in lines]
        have = {m.group(1) for t in texts for m in [re.match(r"method\s+(pf_[\w$']+)__wf\b", t)] if m}
        stubs = []
        for t in texts:
            m = re.match(r"function\s+(pf_[\w$']+)\s*\(", t)
            if m and m.group(1) not in have:
                have.add(m.group(1))
                stubs += [f"method {m.group(1)}__wf()", "  decreases", "{", "}"]
        lines = [*lines, *stubs]
    text, entries = [], []
    for i, item in enumerate(lines, start=1):
        if isinstance(item, Check):
            text.append(item.text)
            pairs = [(item.error_id, item.category)]
        elif isinstance(item, Row):
            text.append(item.text)
            pairs = list(item.entries)
        else:
            text.append(item)
            continue
        for error_id, category in pairs:
            entries.append("\t".join([str(i), error_id, category, "t.c", str(i + 100), "5", f"message of line {i}"]))
    return "\n".join(text) + "\n", "\n".join(entries) + "\n"


def sub(lines, old, new, keep_entry=True):
    """`lines` with the one line that holds `old` rewritten to `new` (its entries stay)."""
    hits = [i for i, l in enumerate(lines) if old in (l if isinstance(l, str) else l.text)]
    assert len(hits) == 1, (old, hits)
    out = list(lines)
    item = out[hits[0]]
    if isinstance(item, str):
        out[hits[0]] = item.replace(old, new)
    else:
        item = type(item).__new__(type(item))
        item.__dict__.update(lines[hits[0]].__dict__)
        item.text = item.text.replace(old, new)
        out[hits[0]] = item
    return out


def drop(lines, fragment):
    hits = [i for i, l in enumerate(lines) if fragment in (l if isinstance(l, str) else l.text)]
    assert len(hits) == 1, (fragment, hits)
    return lines[:hits[0]] + lines[hits[0] + 1:]


def at(lines, fragment, *more):
    """`lines` with `more` inserted right after the one line that holds `fragment`."""
    hits = [i for i, l in enumerate(lines) if fragment in (l if isinstance(l, str) else l.text)]
    assert len(hits) == 1, (fragment, hits)
    return lines[:hits[0] + 1] + list(more) + lines[hits[0] + 1:]


TERM = ("termination.failed", "termination")
POSTCONDITION_ROW = (("postcondition.violated", "postcondition"), ("not.wellformed", "internal"))


class PureFunctionTests(unittest.TestCase):
    """Decision 0062 item 12 (M4.5e1d): the Viper function of a `pure` function, `pf_<name>__<id>`.  Bodied; one `decreases` line (a measure, or
    empty); `ensures` and body without permissions, `old`, `inhale` or `assume`; the applications of itself begin a line; the only cycle is such a
    self-application; the positions of an application (a `pf_` function, a method's `requires`, `ensures`, `invariant`, `assert`; not a `__wf`
    method, a `decreases`, a prelude or other function, a method's statement); `HEAP_READ_RE` counts an application as a read of memory; the map
    rows of a function.  Every twin changes one thing of the clean program."""

    FUNCTION = [
        "function pf_tri__1(n: Int): Int",
        Check("  requires -2147483648 <= n && n <= 2147483647", WF, "internal"),
        Check("  requires 0 <= n && n <= 1000", WF, "internal"),
        Row("  ensures 0 <= result && result <= 500500", *POSTCONDITION_ROW),
        Check("  decreases n", *TERM[:1], category="termination"),
        "{",
        "  (n <= 0) ? 0 :",
        Check("  pf_tri__1(n - 1) + n", *TERM[:1], category="termination"),
        "}",
    ]
    METHOD = [
        "method m__1(n: Int) returns (res: Int)",
        Check("  requires -2147483648 <= n && n <= 2147483647", WF, "internal"),
        Check("  requires 0 <= n && n <= 1000", WF, "internal"),
        Check("  ensures res == pf_tri__1(n)", P, "postcondition"),
        "  decreases",
        "{",
        "  var i__1: Int := 0",
        Check("  assert pf_tri__1(n) <= 500500", A, "check"),
        "  while (i__1 < n)",
        Check("    invariant pf_tri__1(i__1) <= 500500", INV, "invariant-preserved"),
        Check("    decreases 1000 - i__1", *TERM[:1], category="termination"),
        "  {",
        "    i__1 := i__1 + 1",
        "  }",
        "  res := n",
        "}",
    ]

    @classmethod
    def program(cls):
        return [*cls.FUNCTION, *cls.METHOD]

    @staticmethod
    def problems(lines):
        return lv.lint_vpr(*build_rows(lines), "t.vpr", "t.vpr.map")

    def refused(self, lines, fragment, label=""):
        probs = self.problems(lines)
        self.assertTrue(any(fragment in p for p in probs), (label, fragment, probs))
        return probs

    def test_the_clean_program_is_clean_under_every_rule(self):
        self.assertEqual(self.problems(self.program()), [])

    def test_a_non_recursive_function_has_an_empty_decreases_and_no_entry(self):
        lines = [
            "function pf_two__2(x: Int): Int",
            Check("  requires -2147483648 <= x && x <= 2147483647", WF, "internal"),
            Row("  ensures result == x + x", *POSTCONDITION_ROW),
            "  decreases",
            "{",
            "  x + x",
            "}",
        ]
        self.assertEqual(self.problems(lines), [])
        # the function needs the decreases line even with an empty measure
        self.refused(drop(lines, "decreases"), "exactly one `decreases` line")

    # --- the `decreases` line
    def test_the_decreases_line_is_exactly_one(self):
        lines = self.program()
        self.refused(drop(lines, "  decreases n"), "exactly one `decreases` line", "none")
        twice = at(lines, "  decreases n", Check("  decreases n", *TERM[:1], category="termination"))
        self.refused(twice, "exactly one `decreases` line", "two")

    def test_decreases_underscore_and_star_are_refused_on_a_function(self):
        lines = self.program()
        for measure, fragment in (("_", "`decreases _` assumes termination"), ("*", "`decreases *` assumes termination"),
                                  ("n if n > 0", "conditional `decreases")):
            with self.subTest(measure=measure):
                self.refused(sub(lines, "  decreases n", f"  decreases {measure}"), fragment)

    def test_a_self_application_needs_a_measure(self):
        lines = sub(self.program(), "  decreases n", "  decreases")
        probs = self.refused(lines, "so its `decreases` has a measure")
        self.assertFalse(any("exactly one" in p for p in probs))

    # --- the contents of an ensures and of the body
    def test_an_ensures_holds_no_permission_old_or_inhale(self):
        for text in ("acc(loc(block_of(n), 0).v_int)", "perm(n.v_int) > none", "old(n) == n", "forperm x: Ref [x.f] :: true"):
            with self.subTest(ensures=text):
                lines = sub(self.program(), "ensures 0 <= result && result <= 500500", f"ensures {text}")
                self.refused(lines, "an `ensures` of a `pure` function reads no permission")

    def test_the_body_holds_no_permission_old_forperm_inhale_or_assume(self):
        for text in ("perm(n.v_int) > none ? 0 : 1", "old(n)", "acc(n.v_int) ? 0 : 1", "forperm x: Ref [x.f] :: true ? 0 : 1"):
            with self.subTest(body=text):
                lines = sub(self.program(), "  (n <= 0) ? 0 :", f"  {text} :")
                self.refused(lines, "the body of a `pure` function holds no `acc`")
        for word in ("inhale", "assume"):
            with self.subTest(word=word):
                # a tagged `inhale`/`assume` is still refused in a function's body (the generic rule is for methods that carry the tag)
                lines = sub(self.program(), "  (n <= 0) ? 0 :", f"  {word} true // cerastes:inhale-ok test")
                self.refused(lines, "the body of a `pure` function holds no `acc`")

    def test_words_that_only_contain_the_keywords_are_fine(self):
        lines = sub(self.program(), "  (n <= 0) ? 0 :", "  (n <= 0) ? hold : accent :")
        self.assertFalse(any("holds no `acc`" in p for p in self.problems(lines)))

    # --- the shape of the application of itself
    def test_an_application_of_itself_begins_a_line_of_its_own(self):
        lines = sub(self.program(), "  (n <= 0) ? 0 :", "  (n <= 0) ? pf_tri__1(0) :")
        self.refused(lines, "begins a line of its own")
        both = sub(self.program(), "  pf_tri__1(n - 1) + n", "  pf_tri__1(n - 1) + pf_tri__1(n - 2)")
        self.refused(both, "begins a line of its own")
        parens = sub(self.program(), "  pf_tri__1(n - 1) + n", "  (pf_tri__1(n - 1)) + n")
        self.assertEqual(self.problems(parens), [])  # a leading group does not move the application off its line

    # --- the cycles
    def test_mutual_recursion_is_refused(self):
        a = ["function pf_a__3(n: Int): Int", "  decreases n", "{", "  (n <= 0) ? 0 :", Check("  pf_b__4(n - 1)", *TERM[:1], category="termination"), "}"]
        b = ["function pf_b__4(n: Int): Int", "  decreases n", "{", "  (n <= 0) ? 0 :", Check("  pf_a__3(n - 1)", *TERM[:1], category="termination"), "}"]
        probs = self.refused(a + b, "part of a cycle of `pure` functions")
        self.assertEqual(len([p for p in probs if "part of a cycle" in p]), 2)
        # the same two functions, one calling the other only: clean
        c = ["function pf_c__5(n: Int): Int", "  decreases", "{", "  n", "}"]
        d = ["function pf_d__6(n: Int): Int", "  decreases", "{", "  pf_c__5(n)", "}"]
        self.assertEqual(self.problems(c + d), [])

    def test_a_function_that_reaches_itself_through_a_contract_is_refused(self):
        for where, lines in (
                ("an ensures", sub(self.FUNCTION, "ensures 0 <= result && result <= 500500", "ensures result <= pf_tri__1(n)")),
                ("a requires", sub(self.FUNCTION, "requires 0 <= n && n <= 1000", "requires pf_tri__1(n) <= 1000")),
                ("a measure", sub(self.FUNCTION, "  decreases n", "  decreases pf_tri__1(n)"))):
            with self.subTest(where=where):
                self.refused(lines, "applies itself in a `requires`, an `ensures` or a `decreases`")
        # through another function's contract
        other = ["function pf_e__7(n: Int): Int", "  decreases", Check("  ensures result == pf_tri__1(n)", "postcondition.violated", "postcondition"), "{", "  n", "}"]
        through = sub(self.FUNCTION, "ensures 0 <= result && result <= 500500", "ensures result <= pf_e__7(n)")
        self.refused(other + through, "part of a cycle of `pure` functions")

    # --- the name
    def test_a_name_that_starts_pf_is_pf_name_id(self):
        for name in ("pf_x", "pf_x__", "pf_x__a", "pf__1"):
            with self.subTest(name=name):
                lines = [f"function {name}(n: Int): Int", "  decreases", "{", "  n", "}"]
                self.refused(lines, "a function whose name starts `pf_` is a `pf_<name>__<id>`")

    # --- the positions of an application
    def test_where_an_application_may_stand(self):
        method = [
            "method m__2(n: Int) returns (res: Int)",
            Check("  requires pf_tri__1(n) <= 1000", WF, "internal"),
            Check("  ensures res == pf_tri__1(n)", P, "postcondition"),
            "  decreases",
            "{",
            Check("  assert pf_tri__1(n) <= 500500", A, "check"),
            "  res := n",
            "}",
        ]
        self.assertEqual(self.problems(self.FUNCTION + method), [])

    def test_an_application_in_a_wf_method_a_decreases_or_a_statement_is_refused(self):
        lines = self.program()
        cases = {
            "a __wf method": (lines + ["method m__9__wf(n: Int)", "  decreases", "{", Check("  assert pf_tri__1(n) <= 500500", A, "check"), "}"],
                              "stands in no `__wf` method"),
            "a method's decreases": (sub(lines, "    decreases 1000 - i__1", "    decreases pf_tri__1(n) - i__1"), "stands in no `decreases`"),
            "a method statement": (sub(lines, "  res := n", "  res := pf_tri__1(n)"), "on the line of its keyword"),
            "the function's own decreases": (sub(lines, "  decreases n", "  decreases pf_tri__1(n)"), "stands in no `decreases`"),
            "a prelude function": (lines + ["function c_abs(x: Int): Int", "{", "  pf_tri__1(x)", "}"], "stands in a `pf_` function"),
        }
        for label, (program, fragment) in cases.items():
            with self.subTest(case=label):
                self.refused(program, fragment, label)

    def test_the_wf_method_of_a_pure_function_is_no_application_of_it(self):
        # S46 (b): the definition check of `pf_<name>__<id>` is a method `pf_<name>__<id>__wf`; its declaration line is no application
        wf = ["method pf_tri__1__wf(n: Int)", "  decreases", "{", "}"]
        probs = self.problems(self.program() + wf)
        self.assertFalse(any("an application of" in p for p in probs), probs)
        self.assertFalse(any("pf_tri__1__wf" in p for p in probs), probs)

    def test_the_callee_is_a_pf_function_of_the_file(self):
        self.refused(sub(self.program(), "assert pf_tri__1(n) <= 500500", "assert pf_nope__9(n) <= 500500"), "the callee is a `pf_<name>__<id>` function of this file")
        self.refused(sub(self.program(), "assert pf_tri__1(n) <= 500500", "assert pf_nope(n) <= 500500"), "the callee is a `pf_<name>__<id>` function of this file")

    # --- HEAP_READ_RE
    def test_an_application_counts_as_a_heap_read_in_every_heap_free_position(self):
        self.assertFalse(lv._heap_free("pf_tri__1(v_n)"))
        self.assertFalse(lv._heap_free("v_n + pf_tri__1(v_n) + 1"))
        self.assertTrue(lv._heap_free("v_n + 1"))
        self.assertTrue(lv._heap_free("hva_m__1__pf_tri__1_post(v_n)"))  # a ghost function is not an application
        ok = "forall k: Int :: { loc(block_of(v_a), k) } ((offset_of(v_a) <= k) && (k < offset_of(v_a) + %s)) ==> acc(loc(block_of(v_a), k).v_int, %s)"
        self.assertIsNotNone(lv.parse_range_quantifier_full(ok % ("v_n", "wildcard")))
        self.assertIsNone(lv.parse_range_quantifier_full(ok % ("pf_tri__1(v_n)", "wildcard")))  # the count
        self.assertIsNone(lv.parse_range_quantifier_full(
            "forall k: Int :: { loc(block_of(v_a), k) } ((pf_tri__1(v_n) <= k) && (k < offset_of(v_a) + v_n)) ==> acc(loc(block_of(v_a), k).v_int)"))  # the bound
        self.assertIsNone(lv.parse_range_quantifier_full(
            "forall k: Int :: { loc(block_of(pf_tri__1(v_a)), k) } ((offset_of(v_a) <= k) && (k < offset_of(v_a) + v_n)) ==> acc(loc(block_of(v_a), k).v_int)"))  # the base

    def test_an_application_in_a_guard_of_an_assert_is_refused(self):
        guard = "(pf_tri__1(n) > 0) ==> (forall q0: Int :: { loc(block_of(v_p), q0) } (-2147483648 <= (q0 - offset_of(v_p)) && (q0 - offset_of(v_p)) <= 2147483647) ==> (loc(block_of(v_p), q0).v_int >= 0))"
        lines = [*self.FUNCTION, "method m__3(v_p: Ref, n: Int)", "  decreases", "{", Check(f"  assert {guard}", A, "check"), "}"]
        self.assertTrue(any("`forall`" in p or "decision 0061" in p for p in self.problems(lines)))

    # --- quantifiers
    def test_a_user_quantifier_stands_in_the_requires_and_ensures_of_a_pure_function_only(self):
        q = ("forall q0: Int :: { loc(block_of(a), q0) } (-2147483648 <= (q0 - offset_of(a)) && (q0 - offset_of(a)) <= 2147483647) ==> "
             "(!(0 <= (q0 - offset_of(a)) && (q0 - offset_of(a)) < n) || (loc(block_of(a), q0).v_int >= 0))")
        lines = ["function pf_q__8(a: Ref, n: Int): Int", Check(f"  requires {q}", WF, "internal"),
                 Row(f"  ensures {q}", *POSTCONDITION_ROW), "  decreases", "{", "  n", "}"]
        self.assertEqual(self.problems(lines), [])
        in_body = ["function pf_q__8(a: Ref, n: Int): Int", "  decreases", "{", f"  ({q}) ? n : 0", "}"]
        self.refused(in_body, "`forall` outside")
        bad_shape = sub(lines, "ensures forall q0: Int :: { loc(block_of(a), q0) }", "ensures forall q0: Int :: { n }")
        self.refused(bad_shape, "decision 0061")

    # --- the position map
    def test_the_rows_of_a_function(self):
        lines = self.program()
        ensures_row = next(l for l in lines if isinstance(l, Row))

        def with_entries(*entries):
            return [Row(l.text, *entries) if l is ensures_row else l for l in lines]

        self.refused(with_entries(("not.wellformed", "internal")), "needs a position-map entry with the Viper error id `postcondition.violated`")
        self.refused(with_entries(("postcondition.violated", "postcondition")), "needs a position-map entry with the Viper error id `not.wellformed`")
        self.refused(with_entries(("postcondition.violated", "overflow"), ("not.wellformed", "internal")), "its category is `postcondition`, found `overflow`")
        self.refused(with_entries(("postcondition.violated", "postcondition"), ("not.wellformed", "check")), "its category is `internal`, found `check`")

        def retarget(fragment, error_id, category):
            return [Check(l.text, error_id, category) if isinstance(l, Check) and fragment in l.text else l for l in lines]

        # a requires is checked for well-formedness only
        self.refused(retarget("requires 0 <= n && n <= 1000", WF, "check"), "a `requires` of a `pure` function is checked for well-formedness only")
        # a measure's entry is a termination row
        self.refused(retarget("  decreases n", "termination.failed", "overflow"), "the entry of a `decreases` measure is `termination`")
        # the self-application's entry, missing, on another body line, and with another id or category
        self.refused([l.text if isinstance(l, Check) and "pf_tri__1(n - 1) + n" in l.text else l for l in lines], "needs the `termination.failed` entry of that call")
        self.refused([Check(l, A, "overflow") if l == "  (n <= 0) ? 0 :" else l for l in lines], "has an entry only when it begins an application of the function itself")
        self.refused(retarget("pf_tri__1(n - 1) + n", "assert.failed", "termination"), "has an entry only when it begins an application")
        self.refused(retarget("pf_tri__1(n - 1) + n", "termination.failed", "overflow"), "has an entry only when it begins an application")


class ApplicationGhostTests(unittest.TestCase):
    """Decision 0062 item 12, item 8: the application ghosts `hva_<method>__<callee>_<pre|post>` of an `f__wf` method (`Int`, `Bool` and `Ref`
    parameters, the callee's signature) and the three kinds of fact the method assumes over them: (i) the type range of an application, unguarded;
    (ii) the callee's `ensures` at the arguments, under exactly the callee's `requires` that read no memory (the guard is what makes it sound: S46
    (c) (2u)); (iii) the keep fact under the guards of a `reads_range` atom and its `ensures` partner and the interval inclusion (S45 / S46 (c) (4)).
    Each unguarded or weaker form is a must-fail twin."""

    APP = "hva_m__1__pf_tri__1_post"
    DOMAIN = ["// cerastes:domain-ok 0061", "domain Hvq_m {", f"  function {APP}(a: Int): Int", "}"]
    RANGE = f"(-9223372036854775808 <= {APP}(n)) && ({APP}(n) <= 9223372036854775807)"
    ENSURES = f"((-2147483648 <= n) && (n <= 2147483647) && (0 <= n) && (n <= 1000)) ==> ((0 <= {APP}(n)) && ({APP}(n) <= 500500))"

    @classmethod
    def program(cls, requires=None, domain=None, rows=("assert " + "hva_m__1__pf_tri__1_post(n) <= 500500",), tag=(-2**63, 2**63 - 1)):
        requires = (cls.RANGE, cls.ENSURES) if requires is None else requires
        out = [*PureFunctionTests.FUNCTION, *PureFunctionTests.METHOD, *(cls.DOMAIN if domain is None else domain),
               "method m__1__wf(n: Int, res: Int)"]
        out[0] += " // cerastes:result %d %d" % tag  # the callee's result tag, which fact (i) is checked against (decision 0062 item 12)
        out += [Check(f"  requires {r}", WF, "internal") for r in requires]
        out += ["  decreases", "{"] + [Check(f"  {r}", A, "check") for r in rows] + ["}"]
        return out

    @staticmethod
    def problems(lines):
        return lv.lint_vpr(*build_rows(lines), "t.vpr", "t.vpr.map")

    def refused(self, lines, fragment="decision 0062", label=""):
        probs = self.problems(lines)
        self.assertTrue(any(fragment in p for p in probs), (label, fragment, probs))
        return probs

    def test_the_clean_program_is_clean_under_every_rule(self):
        self.assertEqual(self.problems(self.program()), [])

    def test_each_fact_is_clean_on_its_own(self):
        for facts in ((self.RANGE,), (self.ENSURES,), ()):
            with self.subTest(facts=facts):
                self.assertEqual(self.problems(self.program(requires=facts)), [])

    # --- (i) the type range
    def test_the_type_range_of_an_application(self):
        a = self.APP
        for lo, hi in ((-2147483648, 2147483647), (0, 4294967295), (-128, 127), (-(2**127), 2**127 - 1)):
            with self.subTest(range=(lo, hi)):
                self.assertEqual(self.problems(self.program(requires=(f"({lo} <= {a}(n)) && ({a}(n) <= {hi})",), tag=(lo, hi))), [])

    def test_the_twins_of_the_type_range(self):
        a = self.APP
        cases = {
            "guarded": f"(n > 0) ==> ((-9223372036854775808 <= {a}(n)) && ({a}(n) <= 9223372036854775807))",
            "a range that excludes zero": f"(1 <= {a}(n)) && ({a}(n) <= 5)",
            "one bound only": f"(-9223372036854775808 <= {a}(n))",
            "two applications": f"(-9223372036854775808 <= {a}(n)) && ({a}(n + 1) <= 9223372036854775807)",
            "not heap free": f"(-9223372036854775808 <= {a}(n)) && ({a}(v_p.v_int) <= 9223372036854775807)",
        }
        for label, fact in cases.items():
            with self.subTest(case=label):
                self.refused(self.program(requires=(fact,)), label=label)

    def test_a_bool_function_has_no_type_range(self):
        function = ["function pf_ok__3(x: Int): Bool", "  decreases", "{", "  x > 0", "}"]
        domain = ["// cerastes:domain-ok 0061", "domain Hvq_m {", "  function hva_m__1__pf_ok__3_post(a: Int): Bool", "}"]
        method = ["method m__1(n: Int) returns (res: Int)", "  decreases", "{", "  res := n", "}"]
        lines = [*function, *method, *domain, "method m__1__wf(n: Int, res: Int)",
                 Check("  requires (-9223372036854775808 <= hva_m__1__pf_ok__3_post(n)) && (hva_m__1__pf_ok__3_post(n) <= 9223372036854775807)", WF, "internal"),
                 "  decreases", "{", "}"]
        self.refused(lines, "a `Bool` result has no type range")

    # --- (ii) the callee's ensures
    def test_an_ensures_fact_with_arguments_that_are_expressions(self):
        a = self.APP
        fact = (f"((-2147483648 <= (n - 1)) && ((n - 1) <= 2147483647) && (0 <= (n - 1)) && ((n - 1) <= 1000)) ==> "
                f"((0 <= {a}(n - 1)) && ({a}(n - 1) <= 500500))")
        self.assertEqual(self.problems(self.program(requires=(fact,))), [])

    def test_the_twins_of_the_ensures_fact(self):
        a = self.APP
        v = "(-2147483648 <= n) && (n <= 2147483647) && (0 <= n) && (n <= 1000)"
        e = f"(0 <= {a}(n)) && ({a}(n) <= 500500)"
        cases = {
            "unguarded (the ensures holds only under the requires: S46 (c) (2u))": e,
            "a guard that lacks a conjunct": f"((-2147483648 <= n) && (n <= 2147483647) && (0 <= n)) ==> ({e})",
            "a guard with one conjunct too many": f"({v} && (n <= 10)) ==> ({e})",
            "a guard that is not the callee's": f"(n > 5) ==> ({e})",
            "a conclusion that is not the callee's ensures": f"({v}) ==> ({a}(n) <= 500)",
            "a second guard": f"(n > 0) ==> (({v}) ==> ({e}))",
            "arguments the guard does not substitute": f"({v}) ==> ((0 <= {a}(n - 1)) && ({a}(n - 1) <= 500500))",
            "two applications": f"({v}) ==> ((0 <= {a}(n)) && ({a}(n - 1) <= 500500))",
            "the guard reads the heap": f"({v} && (v_p.v_int > 0)) ==> ({e})",
        }
        for label, fact in cases.items():
            with self.subTest(case=label):
                self.refused(self.program(requires=(fact,)), label=label)

    def test_the_guard_of_fact_ii_is_a_set_of_conjuncts(self):
        # the callee's requires write `0 <= n` and the well-formedness twice (the clause's and the atom's): a guard that repeats or omits the repeat is the same set
        a = self.APP
        e = f"(0 <= {a}(n)) && ({a}(n) <= 500500)"
        for v in ("(-2147483648 <= n) && (n <= 2147483647) && (0 <= n) && (n <= 1000) && (0 <= n)", "(n <= 1000) && (0 <= n) && (n <= 2147483647) && (-2147483648 <= n)"):
            with self.subTest(guard=v):
                self.assertEqual(self.problems(self.program(requires=(f"({v}) ==> ({e})",))), [])

    def test_a_function_without_requires_has_an_unguarded_ensures_fact(self):
        function = ["function pf_cl__3(x: Int): Int", Row("  ensures -100 <= result && result <= 100", *POSTCONDITION_ROW), "  decreases", "{", "  x", "}"]
        domain = ["// cerastes:domain-ok 0061", "domain Hvq_m {", "  function hva_m__1__pf_cl__3_post(a: Int): Int", "}"]
        fact = "(-100 <= hva_m__1__pf_cl__3_post(n)) && (hva_m__1__pf_cl__3_post(n) <= 100)"
        method = ["method m__1(n: Int) returns (res: Int)", "  decreases", "{", "  res := n", "}"]
        lines = [*function, *method, *domain, "method m__1__wf(n: Int, res: Int)", Check(f"  requires {fact}", WF, "internal"),
                 "  decreases", "{", "}"]
        self.assertEqual(self.problems(lines), [])
        guarded = sub(lines, f"requires {fact}", f"requires (n > 0) ==> ({fact})")
        self.refused(guarded, label="a guard where the callee has no requires")

    # --- the ghost functions
    def test_the_twins_of_the_ghost_function(self):
        a = self.APP
        decl = f"  function {a}(a: Int): Int"
        cases = {
            "another method's name": ("hva_other__pf_tri__1_post", f"  function hva_other__pf_tri__1_post(a: Int): Int"),
            "a callee that is no pf_ function": ("hva_m__1__nope_post", "  function hva_m__1__nope_post(a: Int): Int"),
            "no state": ("hva_m__1__pf_tri__1", "  function hva_m__1__pf_tri__1(a: Int): Int"),
            "a parameter of another type than the callee's": (a, f"  function {a}(a: Bool): Int"),
            "a parameter too many": (a, f"  function {a}(a: Int, b: Int): Int"),
            "no parameter": (a, f"  function {a}(): Int"),
            "a Perm parameter": (a, f"  function {a}(a: Perm): Int"),
            "a result of another type": (a, f"  function {a}(a: Int): Bool"),
            "a Ref result": (a, f"  function {a}(a: Int): Ref"),
        }
        for label, (name, line) in cases.items():
            with self.subTest(case=label):
                domain = sub(self.DOMAIN, decl, line)
                lines = sub(self.program(domain=domain), f"requires {self.RANGE}", f"requires {self.RANGE}".replace(a, name))
                lines = sub(lines, f"requires {self.ENSURES}", f"requires {self.ENSURES}".replace(a, name))
                lines = sub(lines, "assert hva_m__1__pf_tri__1_post(n) <= 500500", f"assert {name}(n) <= 500500")
                probs = self.problems(lines)
                self.assertTrue(any("decision 0062" in p or "ghost function" in p for p in probs), (label, probs))

    def test_the_domain_holds_int_bool_and_ref_parameters_only(self):
        domain = sub(self.DOMAIN, f"  function {self.APP}(a: Int): Int", f"  function {self.APP}(a: Perm): Int")
        probs = self.problems(self.program(domain=domain))
        self.assertTrue(any("has `Int`, `Bool` or `Ref` parameters" in p for p in probs), probs)

    def test_an_application_ghost_stands_in_a_wf_method_only(self):
        lines = sub(self.program(), "  res := n", "  res := hva_m__1__pf_tri__1_post(n)")
        probs = self.problems(lines)
        self.assertTrue(any("outside a `__wf` method" in p for p in probs), probs)

    def test_a_line_with_both_kinds_of_ghost_is_refused(self):
        domain = ["// cerastes:domain-ok 0061", "domain Hvq_m {", f"  function {self.APP}(a: Int): Int", "  function hvq_m__1__a_pre(s: Int): Int", "}"]
        lines = self.program(requires=(self.RANGE, f"hvq_m__1__a_pre(0) == {self.APP}(n)"), domain=domain)
        self.refused(lines, "applies both an `hvq_` and an `hva_` ghost function")

    def test_a_quantified_fact_over_application_ghosts_waits_for_cut_2(self):
        fact = f"forall s: Int :: {{ {self.APP}(s) }} (-2147483648 <= {self.APP}(s)) && ({self.APP}(s) <= 2147483647)"
        self.refused(self.program(requires=(fact,)), "decision 0062 item 7, cut 2")

    # --- the rows of the method
    def test_a_row_over_an_application_ghost_reads_no_heap(self):
        self.assertEqual(self.problems(self.program(rows=("assert hva_m__1__pf_tri__1_post(n) <= 500500",))), [])
        self.refused(self.program(rows=("assert hva_m__1__pf_tri__1_post(n) <= v_p.v_int",)), label="a heap read in a row")
        probs = self.problems(self.program(rows=("exhale hva_m__1__pf_tri__1_post(n) <= 500500",)))
        self.assertTrue(any("holds no application ghost" in p for p in probs), probs)


class KeepFactTests(unittest.TestCase):
    """Decision 0062 item 8 (iii): the keep fact `G ==> post(args) == pre(args)` of an application whose callee interval lies inside a `reads_range`
    atom of the method that has an `ensures` partner.  `G` is the atoms' guards and the inclusion of the interval; an unguarded fact lets a wrong
    contract pass (an `owns_range` alias under a false guard), as the unguarded keep fact of 0061 does (S45)."""

    PRE, POST = "hva_m__2__pf_sum__2_pre", "hva_m__2__pf_sum__2_post"
    INCLUSION = "(block_of(a) == block_of(a)) && (offset_of(a) <= offset_of(a)) && ((offset_of(a) + v_n) <= (offset_of(a) + v_n))"
    SHAPE = "(a == null) || (((a == loc(block_of(a), offset_of(a))) && (0 <= offset_of(a))) && (offset_of(a) <= len(block_of(a))))"

    @classmethod
    def program(cls, keep=None, guard=None):
        atom = WfQuantifierTests.atom
        keep = f"({cls.INCLUSION}) ==> ({cls.POST}(a, v_n) == {cls.PRE}(a, v_n))" if keep is None else keep
        return [
            "function pf_sum__2(a: Ref, n: Int): Int",
            Check("  requires (-2147483648 <= n) && (n <= 2147483647)", WF, "internal"),
            Check(f"  requires {cls.SHAPE}", WF, "internal"),
            Check("  requires a != null", WF, "internal"),
            Check(f"  requires {cls.SHAPE}", WF, "internal"),
            Check("  requires 0 <= n", WF, "internal"),
            Check("  requires (offset_of(a) + n) <= len(block_of(a))", WF, "internal"),
            Check("  requires forall k: Int :: { loc(block_of(a), k) } ((offset_of(a) <= k) && (k < offset_of(a) + n)) ==> acc(loc(block_of(a), k).v_int, wildcard)", WF, "internal"),
            Check("  requires forall k: Int :: { loc(block_of(a), k) } ((offset_of(a) <= k) && (k < offset_of(a) + n)) ==> ((-2147483648 <= loc(block_of(a), k).v_int) && (loc(block_of(a), k).v_int <= 2147483647))", WF, "internal"),
            "  decreases",
            "{",
            "  n",
            "}",
            "// cerastes:trusted-contract test",
            "method m__2(a: Ref, v_n: Int, c: Int, amt: Perm)",
            Check("  requires none < amt && amt < write", WF, "internal"),
            Check(f"  requires {atom('a', guard=guard)}", WF, "internal"),
            Check(f"  ensures {atom('a', guard=guard)}", P, "postcondition"),
            "  decreases _",
            "// cerastes:domain-ok 0061",
            "domain Hvq_m2 {",
            f"  function {cls.PRE}(a: Ref, n: Int): Int",
            f"  function {cls.POST}(a: Ref, n: Int): Int",
            "}",
            "method m__2__wf(a: Ref, v_n: Int, c: Int, amt: Perm)",
            Check("  requires none < amt && amt < write", WF, "internal"),
            Check(f"  requires {keep}", WF, "internal"),
            "  decreases",
            "{",
            "}",
        ]

    @staticmethod
    def problems(lines):
        return lv.lint_vpr(*build_rows(lines), "t.vpr", "t.vpr.map")

    def refused(self, lines, label=""):
        probs = self.problems(lines)
        self.assertTrue(any("decision 0062" in p for p in probs), (label, probs))

    def test_the_keep_fact_under_the_inclusion_is_clean(self):
        self.assertEqual(self.problems(self.program()), [])

    def test_the_keep_fact_under_the_atoms_guard_and_the_inclusion_is_clean(self):
        keep = f"((c != 0) && {self.INCLUSION}) ==> ({self.POST}(a, v_n) == {self.PRE}(a, v_n))"
        self.assertEqual(self.problems(self.program(keep=keep, guard="(c != 0)")), [])

    def test_the_twins_of_the_keep_fact(self):
        eq = f"{self.POST}(a, v_n) == {self.PRE}(a, v_n)"
        cases = {
            "unguarded (the fact holds on every path: a wrong contract passes)": (eq, None),
            "only the inclusion's first half": (f"(offset_of(a) <= offset_of(a)) ==> ({eq})", None),
            "no inclusion, only a plain condition": (f"(v_n >= 0) ==> ({eq})", None),
            "the atom's guard missing": (f"({self.INCLUSION}) ==> ({eq})", "(c != 0)"),
            "a guard of another atom": (f"((c == 0) && {self.INCLUSION}) ==> ({eq})", "(c != 0)"),
            "the states swapped": (f"({self.INCLUSION}) ==> ({self.PRE}(a, v_n) == {self.POST}(a, v_n))", None),
            "two pre-state applications": (f"({self.INCLUSION}) ==> ({self.PRE}(a, v_n) == {self.PRE}(a, v_n))", None),
            "other arguments in the two applications": (f"({self.INCLUSION}) ==> ({self.POST}(a, v_n) == {self.PRE}(a, v_n + 1))", None),
            "an equality that is no keep fact": (f"({self.INCLUSION}) ==> ({self.POST}(a, v_n) == 0)", None),
        }
        for label, (keep, guard) in cases.items():
            with self.subTest(case=label):
                self.refused(self.program(keep=keep, guard=guard), label)

    def test_a_method_with_no_reads_range_atom_has_no_keep_fact(self):
        lines = self.program()
        lines = [l for l in lines if not (isinstance(l, Check) and "acc(loc" in l.text and "ensures" in l.text)]
        self.refused(lines, "no partner")

    def test_the_atom_needs_its_ensures_partner(self):
        lines = [l for l in self.program() if not (isinstance(l, Check) and l.text.startswith("  requires forall"))]
        self.refused(lines, "no reads atom")


UDIV = ["function c_udiv(a: Int, b: Int): Int", "{", r"  ((a >= 0 && b > 0) ? (a \ b) : 0)", "}"]


class InertFunctionTests(unittest.TestCase):
    """Decision 0062 item 12 (the owner's H', question 273): a top-level `function` that is neither a prelude function nor `pf_<name>__<id>` applies
    only prelude functions, so it can neither recurse nor reach a `pf_` function."""

    @staticmethod
    def problems(lines):
        return lv.lint_vpr(*build_rows(lines), "t.vpr", "t.vpr.map")

    def refused(self, lines, label=""):
        probs = self.problems(lines)
        self.assertTrue(any("so it applies only prelude functions" in p for p in probs), (label, probs))

    def test_a_function_that_applies_only_prelude_functions_is_admitted(self):
        self.assertEqual(self.problems([*UDIV, "function other(x: Int): Int", "{", "  c_udiv(x, 3) + c_udiv(x, 5)", "}"]), [])
        self.assertEqual(self.problems(["function other(x: Int): Int", "{", "  x < 0 ? -x : x", "}"]), [])  # a function that applies nothing
        # a domain function or a built-in is not a function of the file
        self.assertEqual(self.problems(["function other(x: Int): Int", "{", "  len(block_of(loc(x, 0)))", "}"]), [])

    def test_the_twins_of_the_inert_function(self):
        cases = {
            "it applies itself": ["function other(x: Int): Int", "{", "  x == 0 ? 0 : other(x - 1)", "}"],
            "it applies itself on its signature line": ["function other(x: Int): Int { other(x) }"],
            "it applies a pf_ function": [*PureFunctionTests.FUNCTION, "function other(x: Int): Int", "{", "  pf_tri__1(x)", "}"],
            "it applies another such function": ["function one(x: Int): Int", "{", "  x", "}", "function other(x: Int): Int", "{", "  one(x)", "}"],
            "it applies another in a requires": ["function one(x: Int): Int", "{", "  x", "}", "function other(x: Int): Int", "  requires one(x) > 0", "{", "  x", "}"],
        }
        for label, lines in cases.items():
            with self.subTest(case=label):
                self.refused(lines, label)

    def test_a_pf_function_and_a_prelude_function_are_not_inert_functions(self):
        # a `pf_` function may apply anything the application rules admit; a prelude function by name is the prelude's
        self.assertFalse(any("applies only prelude functions" in p for p in self.problems([*PureFunctionTests.FUNCTION])))
        mutual = ["function one(x: Int): Int", "{", "  x", "}", *UDIV]
        self.assertEqual(self.problems(mutual), [])


class PreludeDecreasesTests(unittest.TestCase):
    """Decision 0062 item 12 (S46 (a)): lowering prints an empty `decreases` line on every prelude function of a program that holds a `pf_` function,
    and only there; in any other program an empty `decreases` on a prelude function is refused."""

    @staticmethod
    def problems(lines):
        return lv.lint_vpr(*build_rows(lines), "t.vpr", "t.vpr.map")

    @staticmethod
    def udiv(decreases="  decreases"):
        return ["function c_udiv(a: Int, b: Int): Int", decreases, "{", r"  ((a >= 0 && b > 0) ? (a \ b) : 0)", "}"]

    def test_an_empty_decreases_is_admitted_in_a_program_that_holds_a_pf_function(self):
        self.assertEqual(self.problems([*self.udiv(), *PureFunctionTests.FUNCTION]), [])
        applies = ["function pf_use__4(n: Int): Int", "  decreases", "{", "  c_udiv(n, 3)", "}"]
        self.assertEqual(self.problems([*self.udiv(), *applies]), [])

    def test_an_empty_decreases_is_refused_in_a_program_without_one(self):
        probs = self.problems(self.udiv())
        self.assertTrue(any("only stands in a program that holds a `pf_` function" in p for p in probs), probs)
        # a bitwise and a mask function too
        bit = ["function c_and_32(a: Int, b: Int): Int", "  ensures (-2147483648) <= result && result <= 2147483647", "  decreases", "{", "  a", "}"]
        self.assertTrue(any("only stands in a program that holds a `pf_` function" in p for p in self.problems(bit)))
        # a function whose name only starts `pf_` is not one
        fake = ["function pf_x(n: Int): Int", "  decreases", "{", "  n", "}"]
        self.assertTrue(any("only stands in a program" in p for p in self.problems([*self.udiv(), *fake])))

    def test_a_measure_on_a_prelude_function_is_still_the_position_map_rules(self):
        probs = self.problems([*self.udiv("  decreases a"), *PureFunctionTests.FUNCTION])
        self.assertTrue(any("`decreases` line has no entry in the position map" in p for p in probs), probs)

    def test_a_prelude_function_needs_no_decreases_either_way(self):
        plain = ["function c_udiv(a: Int, b: Int): Int", "{", r"  ((a >= 0 && b > 0) ? (a \ b) : 0)", "}"]
        self.assertEqual(self.problems(plain), [])
        self.assertEqual(self.problems([*plain, *PureFunctionTests.FUNCTION]), [])


class PureRequiresMinTests(unittest.TestCase):
    """Decision 0062 item 6, form Min (S46 (a)): the `requires` lines of a `pf_` function, as `tests/viper-smoke/memory/s46_a_min.vpr` has them: a
    parameter's type range, the value clauses, the pure lines of an atom, its `acc(..., wildcard)` line and its element range; no `d_<t>` line, no
    `lv` line, no flag line; `wildcard` stands there and on 0061's marked row only."""

    WF = "(a == null) || (((a == loc(block_of(a), offset_of(a))) && (0 <= offset_of(a))) && (offset_of(a) <= len(block_of(a))))"
    QUANT = "forall k: Int :: {{ loc(block_of(a), k) }} ((offset_of(a) <= k) && (k < offset_of(a) + n)) ==> {}"
    ACC = QUANT.format("acc(loc(block_of(a), k).v_int, wildcard)")
    ELEMENT = QUANT.format("((-2147483648 <= loc(block_of(a), k).v_int) && (loc(block_of(a), k).v_int <= 2147483647))")
    CELL_WF = "(p == null) || (((p == loc(block_of(p), offset_of(p))) && (0 <= offset_of(p))) && (offset_of(p) <= len(block_of(p))))"

    @classmethod
    def requires(cls):
        return ["(-2147483648 <= n) && (n <= 2147483647)", cls.WF, "0 <= n", "a != null", cls.WF, "0 <= n", "(offset_of(a) + n) <= len(block_of(a))", cls.ACC, cls.ELEMENT]

    @classmethod
    def function(cls, requires=None, name="pf_sum__3"):
        requires = cls.requires() if requires is None else requires
        return ([f"function {name}(a: Ref, n: Int): Int"] + [Check(f"  requires {r}", WF, "internal") for r in requires]
                + [Row("  ensures (-2147483648 * n <= result) && (result <= 2147483647 * n)", *POSTCONDITION_ROW),
                   Check("  decreases n", *TERM[:1], category="termination"), "{", "  (n == 0) ? 0 :",
                   Check(f"  {name}(a, n - 1)", *TERM[:1], category="termination"), "}"])

    @classmethod
    def cell_function(cls, requires=None):
        requires = (["(-2147483648 <= n) && (n <= 2147483647)", cls.CELL_WF, "p != null", cls.CELL_WF, "offset_of(p) < len(block_of(p))",
                     "acc(p.v_int, wildcard)", "((-2147483648 <= p.v_int) && (p.v_int <= 2147483647))"]) if requires is None else requires
        return (["function pf_cell__5(p: Ref, n: Int): Int"] + [Check(f"  requires {r}", WF, "internal") for r in requires]
                + ["  decreases", "{", "  n", "}"])

    @staticmethod
    def problems(lines):
        return lv.lint_vpr(*build_rows(lines), "t.vpr", "t.vpr.map")

    def refused(self, lines, fragment="decision 0062 item 6, form Min", label=""):
        probs = self.problems(lines)
        self.assertTrue(any(fragment in p for p in probs), (label, fragment, probs))
        return probs

    def test_the_range_atom_of_form_min_is_clean_under_every_rule(self):
        self.assertEqual(self.problems(self.function()), [])

    def test_the_cell_atom_of_form_min_is_clean_under_every_rule(self):
        self.assertEqual(self.problems(self.cell_function()), [])

    def test_the_combined_pure_line_is_read_too(self):
        combined = ("(((a != null) && ((a == loc(block_of(a), offset_of(a))) && (0 <= offset_of(a)) && (offset_of(a) <= len(block_of(a)))) && (0 <= n)"
                    " && (offset_of(a) + n <= len(block_of(a)))))")
        reqs = ["(-2147483648 <= n) && (n <= 2147483647)", combined, self.ACC, self.ELEMENT]
        self.assertEqual(self.problems(self.function(reqs)), [])

    def test_value_clauses_and_a_pure_function_in_a_requires(self):
        other = ["function pf_g__8(x: Int): Int", "  decreases", "{", "  x", "}"]
        lines = self.function(self.requires() + ["n < 1000 && n != 7", "pf_g__8(n) > 0"])
        self.assertEqual(self.problems(other + lines), [])

    def test_the_twins_of_the_atom_lines(self):
        base = self.requires()
        drop_at = lambda i: base[:i] + base[i + 1:]  # noqa: E731
        cases = {
            "the flag line (form Full)": base[:-1] + [self.QUANT.format("acc(loc(block_of(a), k).d_int, wildcard)"), base[-1]],
            "a flag line without a permission": base[:-1] + [self.QUANT.format("loc(block_of(a), k).d_int"), base[-1]],
            "the liveness line (form Full)": base + ["acc(lvloc(block_of(a)).lv, wildcard)"],
            "no a != null": drop_at(3),
            "no well-formedness line (the parameter's and the atom's)": [base[0], base[2], base[3]] + base[5:],
            "no 0 <= n (the clause's and the atom's)": [base[0], base[1], base[3], base[4]] + base[6:],
            "no upper bound line": drop_at(6),
            "the acc line before the pure lines": [base[0], base[7]] + base[1:7] + [base[8]],
            "no element-range line": base[:-1],
            "an element range of another type": base[:-1] + [base[-1].replace("2147483647", "2147483646")],
            "the element range before the acc line": base[:7] + [base[8], base[7]],
            "an amount that is not wildcard": base[:7] + [self.QUANT.format("acc(loc(block_of(a), k).v_int, write)")] + base[8:],
            "the acc line of another pointer": base[:7] + [self.ACC.replace("block_of(a)", "block_of(b)").replace("offset_of(a)", "offset_of(b)")] + base[8:],
            "a guard over the atom": base[:7] + [f"(n > 0) ==> ({self.ACC})"] + base[8:],
            "a value clause that reads memory": base + ["loc(block_of(a), 0).v_int > 0"],
            "an old in a requires": base + ["old(n) > 0"],
            "an owns-style permission": base + ["acc(loc(block_of(a), 0).v_int)"],
        }
        for label, requires in cases.items():
            with self.subTest(case=label):
                probs = self.problems(self.function(requires))
                self.assertTrue(any("decision 0062 item 6, form Min" in p for p in probs), (label, probs))

    def test_a_flag_or_liveness_line_is_named_as_form_full(self):
        flag = self.QUANT.format("acc(loc(block_of(a), k).d_int, wildcard)")
        for label, line in (("a flag permission", flag), ("a flag", self.QUANT.format("loc(block_of(a), k).d_int")),
                            ("the liveness", "acc(lvloc(block_of(a)).lv, wildcard)")):
            with self.subTest(case=label):
                self.refused(self.function(self.requires() + [line]), "is no line of form Min", label)

    def test_the_amount_of_the_acc_line_is_wildcard_alone(self):
        for amount in ("wildcard + write", "(wildcard * 2)", "wildcard ? write : write"):
            with self.subTest(amount=amount):
                line = self.QUANT.format(f"acc(loc(block_of(a), k).v_int, {amount})")
                self.refused(self.function(self.requires()[:7] + [line] + self.requires()[8:]), "`wildcard` stands only in", amount)

    def test_the_twins_of_the_cell_atom(self):
        base = ["(-2147483648 <= n) && (n <= 2147483647)", self.CELL_WF, "p != null", self.CELL_WF, "offset_of(p) < len(block_of(p))",
                "acc(p.v_int, wildcard)", "((-2147483648 <= p.v_int) && (p.v_int <= 2147483647))"]
        cases = {
            "no slot bound": base[:4] + base[5:],
            "no p != null": base[:2] + base[3:],
            "the flag": base + ["acc(p.d_int, wildcard)"],
            "no element range": base[:-1],
            "the element range before the acc": base[:5] + [base[6], base[5]],
            "an element range of another type": base[:-1] + [base[-1].replace("2147483647", "2147483646")],
            "an amount that is not wildcard": base[:5] + ["acc(p.v_int, write)"] + base[6:],
        }
        for label, requires in cases.items():
            with self.subTest(case=label):
                probs = self.problems(self.cell_function(requires))
                self.assertTrue(any("decision 0062 item 6, form Min" in p for p in probs) or any("`wildcard`" in p for p in probs), (label, probs))

    def test_wildcard_stands_in_the_acc_line_and_on_the_marked_row_only(self):
        # the Min acc line is exempt from the ghost-amount rule; the same text in a method, an ensures, or as a Perm local is not
        method = ["method m__1(a: Ref, n: Int)", Check(f"  requires {self.ACC}", WF, "internal"), "  decreases", "{", "}"]
        self.assertTrue(any("`wildcard`: an amount Silicon chooses" in p for p in self.problems(method)))
        ensures = self.function() + ["function pf_e__6(a: Ref, n: Int): Int",
                                     Row(f"  ensures {self.ACC}", *POSTCONDITION_ROW), "  decreases", "{", "  n", "}"]
        self.assertTrue(any("`wildcard`: an amount Silicon chooses" in p for p in self.problems(ensures)))
        other = self.function(self.requires() + ["wildcard == n"])
        self.assertTrue(any("`wildcard`" in p for p in self.problems(other)))

    def test_a_user_quantifier_still_stands_in_a_requires(self):
        q = ("forall q0: Int :: { loc(block_of(a), q0) } (-2147483648 <= (q0 - offset_of(a)) && (q0 - offset_of(a)) <= 2147483647) ==> "
             "(!(0 <= (q0 - offset_of(a)) && (q0 - offset_of(a)) < n) || (loc(block_of(a), q0).v_int >= 0))")
        self.assertEqual(self.problems(self.function(self.requires() + [q])), [])

    def test_a_callee_whose_requires_applies_a_function_has_no_ensures_fact(self):
        # fact (ii) is guarded by the callee's memory-free requires; one that holds an application cannot be compared, so the fact is not admitted
        a = "hva_m__1__pf_tri__1_post"
        other = ["function pf_g__8(x: Int): Int", "  decreases", "{", "  x", "}"]
        function = [
            *other,
            "function pf_tri__1(n: Int): Int",
            Check("  requires pf_g__8(n) > 0", WF, "internal"),
            Row("  ensures 0 <= result", *POSTCONDITION_ROW),
            "  decreases",
            "{", "  n", "}",
        ]
        fact = f"(n > 0) ==> (0 <= {a}(n))"
        lines = [*function, "method m__1(n: Int) returns (res: Int)", "  decreases", "{", "  res := n", "}", "// cerastes:domain-ok 0061",
                 "domain Hvq_m {", f"  function {a}(a: Int): Int", "}", "method m__1__wf(n: Int, res: Int)", Check(f"  requires {fact}", WF, "internal"),
                 "  decreases", "{", "}"]
        probs = self.problems(lines)
        self.assertTrue(any("applies a `pure` function: its value clauses are not comparable" in p for p in probs), probs)


class QuantifiedGuardTests(unittest.TestCase):
    """Decision 0062 item 8 and item 12 (audit round 1, B1): fact (ii)'s guard V of an application of a callee with a user quantifier (cut 2) in its
    `requires` holds that quantifier over the method's `hvq` ghosts at the arguments, with the callee's other value clauses and pure lines, as a set of
    conjuncts.  A V without it proves what the callee never promised (the program of the audit, `cx1`: the `m + 1` overflow row passes under V without Q
    and fails under V with Q).  The program is the audit's `cx1b_vq_sound.vpr`; the twins change the fact line only."""

    V = '(-2147483648 <= n) && (n <= 2147483647) && (-2147483648 <= m) && (m <= 2147483647) && ((a == null) || (((a == loc(block_of(a), offset_of(a))) && (0 <= offset_of(a))) && (offset_of(a) <= len(block_of(a))))) && (1 <= n) && (a != null) && (0 <= n) && ((offset_of(a) + n) <= len(block_of(a)))'
    Q = '(forall q0: Int :: { hvq_f_f__2__a_post(q0) } (((-2147483648) <= (q0 - offset_of(a))) && ((q0 - offset_of(a)) <= 2147483647) && ((0 <= (q0 - offset_of(a))) && ((q0 - offset_of(a)) < n))) ==> (hvq_f_f__2__a_post(q0) > m))'
    BODY = '((hva_f_f__2__pf_g__1_post(a, n, m) == 0) && (m < 2147483647))'
    PROGRAM = "field v_int: Int\nfield d_int: Bool\nfield v_ptr_int: Ref\nfield d_ptr_int: Bool\nfield tok: Bool\nfield lv: Bool\n\n// cerastes:domain-ok 0028\ndomain Block {\n  function loc(b: Block, i: Int): Ref\n  function len(b: Block): Int\n  function block_of(r: Ref): Block\n  function offset_of(r: Ref): Int\n  function member(r: Ref, k: Int): Block\n  function parent_of(b: Block): Ref\n  function index_of(b: Block): Int\n  function lvloc(b: Block): Ref\n  // cerastes:axiom-ok 0033\n  axiom loc_inv { forall b: Block, i: Int :: { loc(b, i) } block_of(loc(b, i)) == b && offset_of(loc(b, i)) == i }\n  // cerastes:axiom-ok 0034\n  axiom loc_nonnull { forall b: Block, i: Int :: { loc(b, i) } loc(b, i) != null }\n  // cerastes:axiom-ok 0035\n  axiom member_inv { forall r: Ref, k: Int :: { member(r, k) } parent_of(member(r, k)) == r && index_of(member(r, k)) == k }\n  // cerastes:axiom-ok 0036\n  axiom member_lv { forall r: Ref, k: Int :: { lvloc(member(r, k)) } lvloc(member(r, k)) == lvloc(block_of(r)) }\n}\n\n\n// The callee: g(a, n, m) requires 1 <= n, reads_range(a, n), cerastes_forall(gt, a, n, m) (every element > m); ensures r == 0 && m < INT_MAX.\n// Silicon proves the ensures: a[0] > m and a[0] <= INT_MAX.\nfunction pf_g__1(a: Ref, n: Int, m: Int): Int\n  requires (-2147483648 <= n) && (n <= 2147483647)\n  requires (-2147483648 <= m) && (m <= 2147483647)\n  requires (a == null) || (((a == loc(block_of(a), offset_of(a))) && (0 <= offset_of(a))) && (offset_of(a) <= len(block_of(a))))\n  requires 1 <= n\n  requires a != null\n  requires (a == null) || (((a == loc(block_of(a), offset_of(a))) && (0 <= offset_of(a))) && (offset_of(a) <= len(block_of(a))))\n  requires 0 <= n\n  requires (offset_of(a) + n) <= len(block_of(a))\n  requires forall k: Int :: { loc(block_of(a), k) } ((offset_of(a) <= k) && (k < offset_of(a) + n)) ==> acc(loc(block_of(a), k).v_int, wildcard)\n  requires forall k: Int :: { loc(block_of(a), k) } ((offset_of(a) <= k) && (k < offset_of(a) + n)) ==> ((-2147483648 <= loc(block_of(a), k).v_int) && (loc(block_of(a), k).v_int <= 2147483647))\n  requires forall q0: Int :: { loc(block_of(a), q0) } (((-2147483648) <= (q0 - offset_of(a))) && ((q0 - offset_of(a)) <= 2147483647) && ((0 <= (q0 - offset_of(a))) && ((q0 - offset_of(a)) < n))) ==> (loc(block_of(a), q0).v_int > m)\n  ensures result == 0\n  ensures m < 2147483647\n  decreases\n{\n  0\n}\n\n// The caller f(a, n, m): requires 1 <= n && reads_range(a, n) (no quantifier, m unconstrained);\n// ensures(r, reads_range(a, n) && m + 1 > m && (m == INT_MAX ? g(a, n, m) == 0 : 1)).\n// In C, f(arr, 1, INT_MAX) is a legal call and the ensures overflows at m + 1.\nmethod f_f__2(a: Ref, n: Int, m: Int, amt: Perm) returns (res: Int)\n  requires ((-2147483648) <= n) && (n <= 2147483647)\n  requires ((-2147483648) <= m) && (m <= 2147483647)\n  requires none < amt && amt < write\n  requires (a == null) || (((a == loc(block_of(a), offset_of(a))) && (0 <= offset_of(a))) && (offset_of(a) <= len(block_of(a))))\n  requires 1 <= n\n  requires a != null\n  requires (offset_of(a) + n) <= len(block_of(a))\n  requires forall k: Int :: { loc(block_of(a), k) } ((offset_of(a) <= k) && (k < offset_of(a) + n)) ==> acc(loc(block_of(a), k).v_int, amt)\n  requires forall k: Int :: { loc(block_of(a), k) } ((offset_of(a) <= k) && (k < offset_of(a) + n)) ==> ((-2147483648 <= loc(block_of(a), k).v_int) && (loc(block_of(a), k).v_int <= 2147483647))\n  ensures ((-2147483648) <= res) && (res <= 2147483647)\n  ensures forall k: Int :: { loc(block_of(a), k) } ((offset_of(a) <= k) && (k < offset_of(a) + n)) ==> acc(loc(block_of(a), k).v_int, amt)\n  ensures forall k: Int :: { loc(block_of(a), k) } ((offset_of(a) <= k) && (k < offset_of(a) + n)) ==> ((-2147483648 <= loc(block_of(a), k).v_int) && (loc(block_of(a), k).v_int <= 2147483647))\n  ensures m + 1 > m\n  ensures (m == 2147483647) ==> (((((-2147483648) <= n) && (n <= 2147483647)) && (((-2147483648) <= m) && (m <= 2147483647)) && (1 <= n) && (a != null) && ((offset_of(a) + n) <= len(block_of(a))) && (forall q0: Int :: { loc(block_of(a), q0) } (((-2147483648) <= (q0 - offset_of(a))) && ((q0 - offset_of(a)) <= 2147483647) && ((0 <= (q0 - offset_of(a))) && ((q0 - offset_of(a)) < n))) ==> (loc(block_of(a), q0).v_int > m)) ? pf_g__1(a, n, m) : 0) == 0)\n  decreases _\n{\n  res := 0\n}\n\n// cerastes:domain-ok 0061\ndomain Hvq_f_f__2 {\n  function hvq_f_f__2__a_post(s: Int): Int\n  function hva_f_f__2__pf_g__1_post(a: Ref, n: Int, m: Int): Int\n}\n\n// well-formedness of the contract of `f` (variant M), with fact (ii) guarded by V WITHOUT the callee's quantified clause\n// (the only guard scripts/lint_viper.py admits for fact (ii): it refuses a quantifier in a fact)\nmethod f_f__2__wf(a: Ref, n: Int, m: Int, amt: Perm, res: Int)\n  requires none < amt && amt < write\n  requires (a == null) || (((a == loc(block_of(a), offset_of(a))) && (0 <= offset_of(a))) && (offset_of(a) <= len(block_of(a))))\n  requires ((-2147483648) <= n) && (n <= 2147483647)\n  requires ((-2147483648) <= m) && (m <= 2147483647)\n  requires ((-2147483648) <= res) && (res <= 2147483647)\n  requires forall s: Int :: { hvq_f_f__2__a_post(s) } (-2147483648 <= hvq_f_f__2__a_post(s)) && (hvq_f_f__2__a_post(s) <= 2147483647)\n  requires ((-2147483648) <= hva_f_f__2__pf_g__1_post(a, n, m)) && (hva_f_f__2__pf_g__1_post(a, n, m) <= 2147483647)\n@@FACT@@\n  decreases\n{\n  // the overflow row of `m + 1` in the ensures, under the requires\n  assert ((1 <= n) && (a != null) && ((offset_of(a) + n) <= len(block_of(a)))) ==> !(2147483647 < m + 1) //ROW:ens:overflow\n  // the rows of g(a, n, m) under the path m == INT_MAX: the framing, the value clause and the quantified clause Q over the post ghosts\n  assert ((1 <= n) && (a != null) && ((offset_of(a) + n) <= len(block_of(a))) && (m == 2147483647)) ==> ((block_of(a) == block_of(a)) && (offset_of(a) <= offset_of(a)) && ((offset_of(a) + n) <= (offset_of(a) + n))) //ROW:app:frame\n  assert ((1 <= n) && (a != null) && ((offset_of(a) + n) <= len(block_of(a))) && (m == 2147483647)) ==> (1 <= n) //ROW:app:value\n  assert ((1 <= n) && (a != null) && ((offset_of(a) + n) <= len(block_of(a))) && (m == 2147483647)) ==> (forall q0: Int :: { hvq_f_f__2__a_post(q0) } (((-2147483648) <= (q0 - offset_of(a))) && ((q0 - offset_of(a)) <= 2147483647) && ((0 <= (q0 - offset_of(a))) && ((q0 - offset_of(a)) < n))) ==> (hvq_f_f__2__a_post(q0) > m)) //ROW:app:q\n}\n"

    def fact_problems(self, fact):
        text = self.PROGRAM.replace("@@FACT@@", "  requires " + fact)
        probs = lv.lint_vpr(text, None, "t.vpr", "t.vpr.map")
        marker = "hva_f_f__2__pf_g__1_post(a, n, m) == 0"
        line = [i for i, l in enumerate(text.split("\n"), start=1) if l.startswith("  requires") and l.endswith(fact)]
        self.assertEqual(len(line), 1, line)
        return [p for p in probs if p.startswith(f"t.vpr:{line[0]}:")]

    def test_v_with_the_quantified_clause_is_clean(self):
        self.assertEqual(self.fact_problems(f"({self.V} && {self.Q}) ==> {self.BODY}"), [])

    def test_v_without_the_quantified_clause_is_refused(self):
        probs = self.fact_problems(f"({self.V}) ==> {self.BODY}")
        self.assertTrue(any("decision 0062" in p and "quantified clauses over the method's `hvq` ghosts" in p for p in probs), probs)

    def test_the_twins_of_the_quantified_clause(self):
        q = self.Q
        cases = {
            "a weaker element condition": q.replace("> m))", ">= m))"),
            "a shorter range": q.replace("< n)))", "< n - 1)))"),
            "the pre-state ghost for a post-state application": q.replace("hvq_f_f__2__a_post", "hvq_f_f__2__a_pre"),
            "a ghost of no formal of the method": q.replace("hvq_f_f__2__a_post", "hvq_f_f__2__b_post"),
            "a quantifier of another shape": q.replace("(q0 - offset_of(a)) < n", "(q0 - offset_of(a)) < m"),
        }
        for label, quant in cases.items():
            self.assertNotEqual(quant, q, label)
            with self.subTest(case=label):
                probs = self.fact_problems(f"({self.V} && {quant}) ==> {self.BODY}")
                self.assertTrue(any("decision 0062" in p for p in probs), (label, probs))

    def test_the_quantified_clause_only_stands_in_the_guard_of_fact_ii(self):
        cases = {
            "in the body of the fact": f"({self.V}) ==> ({self.Q} && {self.BODY})",
            "in an unguarded fact": f"{self.Q} && {self.BODY}",
            "next to the keep fact": f"({self.V} && {self.Q}) ==> (hva_f_f__2__pf_g__1_post(a, n, m) == hva_f_f__2__pf_g__1_pre(a, n, m))",
        }
        for label, fact in cases.items():
            with self.subTest(case=label):
                self.assertTrue(self.fact_problems(fact), label)

    def test_another_conjunct_in_v_is_refused(self):
        self.assertTrue(self.fact_problems(f"({self.V} && {self.Q} && (n > 5)) ==> {self.BODY}"))
        self.assertTrue(self.fact_problems(f"({self.V} && {self.Q} && hvq_f_f__2__a_post(0) > 0) ==> {self.BODY}"))


class ApplicationFactAuditTests(unittest.TestCase):
    """Decision 0062 item 12 (audit round 1: S1 the keep fact's guard read from the callee's own atoms, S2 fact (i)'s range against the callee's result
    tag, S3 the definition-check method of every `pf_` function, N9 the prelude's closure)."""

    @staticmethod
    def problems(lines, stubs=True):
        return lv.lint_vpr(*build_rows(lines, wf_stubs=stubs), "t.vpr", "t.vpr.map")

    def refused(self, lines, fragment="decision 0062", stubs=True):
        probs = self.problems(lines, stubs)
        self.assertTrue(any(fragment in p for p in probs), (fragment, probs))
        return probs

    # --- S1: the keep fact is guarded by the inclusion of the callee's own intervals
    def test_a_keep_fact_over_a_narrower_interval_is_clean(self):
        k = KeepFactTests
        inclusion = "(block_of(a) == block_of(a)) && (offset_of(a) <= offset_of(a)) && ((offset_of(a) + (v_n - 1)) <= (offset_of(a) + v_n))"
        keep = f"({inclusion}) ==> ({k.POST}(a, v_n - 1) == {k.PRE}(a, v_n - 1))"
        self.assertEqual(self.problems(k.program(keep=keep)), [])

    def test_a_keep_fact_over_a_wider_interval_than_the_atom_is_refused(self):
        k = KeepFactTests
        keep = f"((0 <= offset_of(a)) && (0 <= v_n)) ==> ({k.POST}(a, v_n + 5) == {k.PRE}(a, v_n + 5))"
        self.refused(k.program(keep=keep))
        keep = f"({k.INCLUSION}) ==> ({k.POST}(a, v_n + 5) == {k.PRE}(a, v_n + 5))"
        self.refused(k.program(keep=keep))

    def test_a_keep_fact_over_a_callee_with_no_atom_is_refused(self):
        k = KeepFactTests
        lines = k.program()
        end = next(i for i, l in enumerate(lines) if isinstance(l, str) and l.startswith("// cerastes:trusted-contract"))
        lines = [l for i, l in enumerate(lines) if not (i < end and isinstance(l, Check) and "forall k" in l.text)]
        self.refused(lines, "has no range or cell atom")

    def test_a_keep_fact_without_the_same_block_conjunct_is_refused(self):
        k = KeepFactTests
        keep = f"((offset_of(a) <= offset_of(a)) && ((offset_of(a) + v_n) <= (offset_of(a) + v_n))) ==> ({k.POST}(a, v_n) == {k.PRE}(a, v_n))"
        self.refused(k.program(keep=keep))

    # --- S2: fact (i)'s range is the callee's result tag
    TAG = "function pf_tri__1(n: Int): Int // cerastes:result %d %d"

    def tagged(self, lo, hi, fact_lo, fact_hi):
        a = ApplicationGhostTests.APP
        lines = ApplicationGhostTests.program(requires=(f"({fact_lo} <= {a}(n)) && ({a}(n) <= {fact_hi})",))
        lines[0] = self.TAG % (lo, hi)
        return lines

    def test_fact_i_equal_to_the_result_tag_is_clean(self):
        for lo, hi in ((-2**63, 2**63 - 1), (-2**31, 2**31 - 1), (0, 2**32 - 1)):
            with self.subTest(tag=(lo, hi)):
                self.assertEqual(self.problems(self.tagged(lo, hi, lo, hi)), [])

    def test_fact_i_narrower_or_wider_than_the_result_tag_is_refused(self):
        self.refused(self.tagged(-2**63, 2**63 - 1, -128, 127), "cerastes:result")
        self.refused(self.tagged(-2**31, 2**31 - 1, -2**63, 2**63 - 1), "cerastes:result")
        self.refused(self.tagged(0, 2**32 - 1, -2**31, 2**31 - 1), "cerastes:result")

    def test_a_result_tag_is_a_standard_c_integer_range(self):
        self.refused(self.tagged(-100, 100, -100, 100), "no standard C integer range")
        self.refused(self.tagged(0, 1000, 0, 1000), "no standard C integer range")

    # --- S3: every pf_ function has its __wf method
    def test_a_pf_function_without_its_wf_method_is_refused(self):
        lines = ApplicationGhostTests.program()
        self.refused(lines, "has no definition-check method", stubs=False)
        self.assertEqual(self.problems(lines, stubs=True), [])

    def test_a_wf_method_of_another_name_is_no_wf_method(self):
        lines = [*PureFunctionTests.program(), "method pf_other__9__wf()", "  decreases", "{", "}"]
        self.refused(lines, "has no definition-check method `pf_tri__1__wf`", stubs=False)

    # --- N9: the prelude applies only prelude functions, without a cycle
    def test_a_prelude_function_that_applies_a_pf_function_is_refused(self):
        lines = [*PureFunctionTests.program(), "function c_div(a: Int, b: Int): Int", "{", "  pf_tri__1(a)", "}"]
        self.refused(lines, "no prelude function")

    def test_prelude_functions_in_a_cycle_are_refused(self):
        lines = ["function c_div(a: Int, b: Int): Int", "{", "  c_rem(a, b)", "}", "function c_rem(a: Int, b: Int): Int", "{", "  c_div(a, b)", "}"]
        self.refused(lines, "in a cycle")
        self.refused(["function c_div(a: Int, b: Int): Int", "{", "  c_div(a, b)", "}"], "in a cycle")

    def test_a_prelude_function_that_applies_a_prelude_function_is_no_cycle(self):
        lines = ["function c_div(a: Int, b: Int): Int", "{", "  c_rem(a, b)", "}", "function c_rem(a: Int, b: Int): Int", "{", "  a", "}"]
        probs = self.problems(lines)
        self.assertFalse(any("in a cycle" in p or "no prelude function" in p for p in probs), probs)


class PfEmittedShapeTests(unittest.TestCase):
    """Decision 0062 items 8 and 12 on the shapes lowering emits (cut 2, `smallsum`: `sum_to` with `cerastes_forall(small_at, a, n)` in its `requires`,
    which applies itself): (1) the guard of an application in a `pf_` body holds the applied callee's quantified `requires` clause at the arguments, its
    element reads in the function's own contract form; (2) the `hvq` facts of the function's own `__wf` read their atoms from the function's `requires`;
    (4) fact (ii)'s V holds the callee's quantified clause over the `hvq` ghosts.  The fixture is the emitted function with its `__wf` up to fact (ii);
    the twins change one line."""

    FIXTURE = '\n// the memory model (decision 0032 item 5, decision 0028 item 2): one value field and one flag field per cell type, the liveness cell\nfield v_int: Int\nfield d_int: Bool\nfield lv: Bool\n\n// cerastes:domain-ok 0028\ndomain Block {\n  function loc(b: Block, i: Int): Ref\n  function len(b: Block): Int\n  function block_of(r: Ref): Block\n  function offset_of(r: Ref): Int\n  function member(r: Ref, k: Int): Block\n  function parent_of(b: Block): Ref\n  function index_of(b: Block): Int\n  function lvloc(b: Block): Ref\n  // cerastes:axiom-ok 0033\n  axiom loc_inv { forall b: Block, i: Int :: { loc(b, i) } block_of(loc(b, i)) == b && offset_of(loc(b, i)) == i }\n  // cerastes:axiom-ok 0034\n  axiom loc_nonnull { forall b: Block, i: Int :: { loc(b, i) } loc(b, i) != null }\n  // cerastes:axiom-ok 0035\n  axiom member_inv { forall r: Ref, k: Int :: { member(r, k) } parent_of(member(r, k)) == r && index_of(member(r, k)) == k }\n  // cerastes:axiom-ok 0036\n  axiom member_lv { forall r: Ref, k: Int :: { lvloc(member(r, k)) } lvloc(member(r, k)) == lvloc(block_of(r)) }\n}\n\n// C/C++ function `smallsum` (a `pure` function, record 0062)\nfunction pf_smallsum__0(p_a__0: Ref, p_n__1: Int): Int // cerastes:result -2147483648 2147483647\n  requires ((-2147483648) <= p_n__1) && (p_n__1 <= 2147483647)\n  requires (p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0))))\n  requires p_n__1 >= 0\n  requires p_n__1 <= 1000\n  requires p_a__0 != null\n  requires (p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0))))\n  requires 0 <= p_n__1\n  requires (offset_of(p_a__0) + p_n__1) <= len(block_of(p_a__0))\n  requires forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> acc(loc(block_of(p_a__0), k).v_int, wildcard)\n  requires forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> (((-2147483648) <= loc(block_of(p_a__0), k).v_int) && (loc(block_of(p_a__0), k).v_int <= 2147483647))\n  requires forall q0: Int :: { loc(block_of(p_a__0), q0) } (((-2147483648) <= (q0 - offset_of(p_a__0))) && ((q0 - offset_of(p_a__0)) <= 2147483647)) ==> ((!(((q0 - offset_of(p_a__0)) >= 0) && ((q0 - offset_of(p_a__0)) < p_n__1))) || ((((((block_of(loc(block_of(p_a__0), q0)) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(p_a__0), q0)))) && (offset_of(loc(block_of(p_a__0), q0)) < (offset_of(p_a__0) + p_n__1))) ? loc(block_of(p_a__0), q0).v_int : 0) >= (-1000)) && (((((block_of(loc(block_of(p_a__0), q0)) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(p_a__0), q0)))) && (offset_of(loc(block_of(p_a__0), q0)) < (offset_of(p_a__0) + p_n__1))) ? loc(block_of(p_a__0), q0).v_int : 0) <= 1000)))\n  ensures (((-1000) * p_n__1) <= result) && (result <= (1000 * p_n__1))\n  decreases p_n__1\n{\n  (p_n__1 == 0) ? 0 : ((((((((((((((-2147483648) <= (p_n__1 - 1)) && ((p_n__1 - 1) <= 2147483647)) && ((p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0)))))) && ((p_n__1 - 1) >= 0)) && ((p_n__1 - 1) <= 1000)) && (p_a__0 != null)) && ((p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0)))))) && (0 <= (p_n__1 - 1))) && ((offset_of(p_a__0) + (p_n__1 - 1)) <= len(block_of(p_a__0)))) && (forall q0: Int :: { loc(block_of(p_a__0), q0) } (((-2147483648) <= (q0 - offset_of(p_a__0))) && ((q0 - offset_of(p_a__0)) <= 2147483647)) ==> ((!(((q0 - offset_of(p_a__0)) >= 0) && ((q0 - offset_of(p_a__0)) < (p_n__1 - 1)))) || ((((((block_of(loc(block_of(p_a__0), q0)) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(p_a__0), q0)))) && (offset_of(loc(block_of(p_a__0), q0)) < (offset_of(p_a__0) + p_n__1))) ? loc(block_of(p_a__0), q0).v_int : 0) >= (-1000)) && (((((block_of(loc(block_of(p_a__0), q0)) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(p_a__0), q0)))) && (offset_of(loc(block_of(p_a__0), q0)) < (offset_of(p_a__0) + p_n__1))) ? loc(block_of(p_a__0), q0).v_int : 0) <= 1000))))) && (((block_of(p_a__0) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(p_a__0))) && ((offset_of(p_a__0) + (p_n__1 - 1)) <= (offset_of(p_a__0) + p_n__1)))) ? \n  pf_smallsum__0(p_a__0, p_n__1 - 1) : 0) + ((((block_of(loc(block_of(p_a__0), offset_of(p_a__0) + (p_n__1 - 1))) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(p_a__0), offset_of(p_a__0) + (p_n__1 - 1))))) && (offset_of(loc(block_of(p_a__0), offset_of(p_a__0) + (p_n__1 - 1))) < (offset_of(p_a__0) + p_n__1))) ? loc(block_of(p_a__0), offset_of(p_a__0) + (p_n__1 - 1)).v_int : 0))\n}\n\n// cerastes:domain-ok 0061\ndomain Hvq_pf_smallsum__0 {\n  function hvq_pf_smallsum__0__p_a__0_pre(s: Int): Int\n  function hva_pf_smallsum__0__pf_smallsum__0_pre(x0: Ref, x1: Int): Int\n}\n\n// well-formedness of the contract of C/C++ function `smallsum`\nmethod pf_smallsum__0__wf(p_a__0: Ref, p_n__1: Int, res: Int, hv_e0_pre: Int)\n  requires (p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0))))\n  requires ((-2147483648) <= p_n__1) && (p_n__1 <= 2147483647)\n  requires ((-2147483648) <= res) && (res <= 2147483647)\n  requires ((-2147483648) <= hv_e0_pre) && (hv_e0_pre <= 2147483647)\n  requires forall s: Int :: { hvq_pf_smallsum__0__p_a__0_pre(s) } ((-2147483648) <= hvq_pf_smallsum__0__p_a__0_pre(s)) && (hvq_pf_smallsum__0__p_a__0_pre(s) <= 2147483647)\n  requires hvq_pf_smallsum__0__p_a__0_pre(offset_of(p_a__0) + (p_n__1 - 1)) == hv_e0_pre\n  requires ((-2147483648) <= hva_pf_smallsum__0__pf_smallsum__0_pre(p_a__0, p_n__1 - 1)) && (hva_pf_smallsum__0__pf_smallsum__0_pre(p_a__0, p_n__1 - 1) <= 2147483647)\n  requires (((((((((((-2147483648) <= (p_n__1 - 1)) && ((p_n__1 - 1) <= 2147483647)) && ((p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0)))))) && ((p_n__1 - 1) >= 0)) && ((p_n__1 - 1) <= 1000)) && (p_a__0 != null)) && ((p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0)))))) && (0 <= (p_n__1 - 1))) && ((offset_of(p_a__0) + (p_n__1 - 1)) <= len(block_of(p_a__0)))) && (forall q0: Int :: { hvq_pf_smallsum__0__p_a__0_pre(q0) } (((-2147483648) <= (q0 - offset_of(p_a__0))) && ((q0 - offset_of(p_a__0)) <= 2147483647)) ==> ((!(((q0 - offset_of(p_a__0)) >= 0) && ((q0 - offset_of(p_a__0)) < (p_n__1 - 1)))) || ((hvq_pf_smallsum__0__p_a__0_pre(q0) >= (-1000)) && (hvq_pf_smallsum__0__p_a__0_pre(q0) <= 1000))))) ==> ((((-1000) * (p_n__1 - 1)) <= hva_pf_smallsum__0__pf_smallsum__0_pre(p_a__0, p_n__1 - 1)) && (hva_pf_smallsum__0__pf_smallsum__0_pre(p_a__0, p_n__1 - 1) <= (1000 * (p_n__1 - 1))))\n'

    def lint(self, text):
        lines = text.split("\n")
        probs = lv.lint_vpr(text + "  decreases\n{\n}\n", None, "t.vpr", "t.vpr.map")
        return lines, probs

    def at(self, lines, probs, predicate):
        hits = [i for i, l in enumerate(lines, start=1) if predicate(l)]
        self.assertTrue(hits)
        return [p for p in probs if any(p.startswith(f"t.vpr:{i}:") for i in hits)]

    @staticmethod
    def body_quantifier(l):
        return "forall q0" in l and not l.strip().startswith("requires")

    @staticmethod
    def fact_ii(l):
        return l.startswith("  requires") and "forall q0" in l and "hva_pf_smallsum__0__pf_smallsum__0_pre" in l

    def mutate(self, predicate, old, new):
        lines = self.FIXTURE.split("\n")
        hits = [i for i, l in enumerate(lines) if predicate(l)]
        self.assertEqual(len(hits), 1, hits)
        self.assertIn(old, lines[hits[0]])
        lines[hits[0]] = lines[hits[0]].replace(old, new)
        return "\n".join(lines)

    def test_the_emitted_function_and_its_wf_facts_are_clean(self):
        lines, probs = self.lint(self.FIXTURE)
        self.assertEqual([p for p in probs if "no position map" not in p], [])

    # --- (1) the quantifier of a body
    def test_the_twins_of_the_body_quantifier(self):
        cases = {
            "another bound": ("(p_n__1 - 1)", "(p_n__1 - 2)"),
            "a weaker element condition": (">= (-1000)", ">= (-999)"),
            "a stronger element condition": ("<= 1000)", "<= 999)"),
            "reads over an interval that is neither the site's nor the callee's": ("(offset_of(p_a__0) + p_n__1)", "(offset_of(p_a__0) + (p_n__1 - 2))"),
        }
        for label, (old, new) in cases.items():
            with self.subTest(case=label):
                lines, probs = self.lint(self.mutate(self.body_quantifier, old, new))
                self.assertTrue(any("decision 0061 item 11" in p and "applied there" in p for p in self.at(lines, probs, self.body_quantifier)), (label, probs))

    def test_a_body_quantifier_with_no_application_of_a_function_that_has_one_is_refused(self):
        text = self.FIXTURE.replace("pf_smallsum__0(p_a__0, p_n__1 - 1)", "(p_n__1 - 1)")
        lines, probs = self.lint(text)
        self.assertTrue(any("applied there" in p for p in self.at(lines, probs, self.body_quantifier)), probs)

    # --- (2) and (3): the hvq facts of the function's own __wf
    def test_the_twin_of_the_atoms_of_a_function_wf(self):
        lines = [l for l in self.FIXTURE.split("\n") if not (l.startswith("  requires") and ".v_int" in l and "hvq_" not in l and "hva_" not in l)]
        lines, probs = self.lint("\n".join(lines))
        self.assertTrue(any("decision 0061 item 11" in p and "place atom" in p for p in probs), probs)

    # --- (4) fact (ii)'s V
    def test_the_twins_of_the_emitted_v(self):
        cases = {
            "a weaker quantified element condition": (">= (-1000)", ">= (-999)"),
            "another bound of the quantifier": ("< (p_n__1 - 1)", "< p_n__1"),
        }
        for label, (old, new) in cases.items():
            with self.subTest(case=label):
                lines, probs = self.lint(self.mutate(self.fact_ii, old, new))
                self.assertTrue(any("decision 0062" in p for p in self.at(lines, probs, self.fact_ii)), (label, probs))


class MixedRowAndTagTests(unittest.TestCase):
    """Decision 0062 item 12 (audit follow-up, il-analyst-2's emitted shapes): an obligation row of an `f__wf` method may hold both kinds of ghost (the
    overflow row of a body that adds an application and an element read), never an assumed fact other than fact (ii)'s V; fact (i) is checked against
    the callee's result tag, which an integer, non-enumeration result always has."""

    def program(self, extra):
        k = QuantifiedGuardTests
        text = k.PROGRAM.replace("@@FACT@@", f"  requires ({k.V} && {k.Q}) ==> {k.BODY}")
        lines = text.rstrip("\n").split("\n")
        assert lines[-1] == "}"
        lines[-1:-1] = [extra]
        return lines

    def problems_at(self, extra):
        lines = self.program(extra)
        probs = lv.lint_vpr("\n".join(lines) + "\n", None, "t.vpr", "t.vpr.map")
        line = len(lines) - 1
        return [p for p in probs if p.startswith(f"t.vpr:{line}:")]

    MIXED = "hva_f_f__2__pf_g__1_post(a, n, m) + hvq_f_f__2__a_post(0) <= 4294967294"

    def test_a_row_over_both_kinds_of_ghost_is_clean(self):
        self.assertEqual(self.problems_at(f"  assert {self.MIXED}"), [])
        self.assertEqual(self.problems_at(f"  assert (n > 0) ==> ({self.MIXED})"), [])

    def test_the_twins_of_the_mixed_row(self):
        cases = {
            "an assumed fact": f"  requires {self.MIXED}",
            "an exhale": f"  exhale {self.MIXED}",
            "an assume": f"  assume {self.MIXED}",
            "a row that reads the heap": "  assert hva_f_f__2__pf_g__1_post(a, n, m) + hvq_f_f__2__a_post(0) <= loc(block_of(a), 0).v_int",
        }
        for label, line in cases.items():
            with self.subTest(case=label):
                self.assertTrue(self.problems_at(line), label)

    def test_fact_i_of_a_callee_without_a_result_tag_is_refused(self):
        a = ApplicationGhostTests.APP
        lines = ApplicationGhostTests.program(requires=(f"(-9223372036854775808 <= {a}(n)) && ({a}(n) <= 9223372036854775807)",))
        self.assertTrue(lines[0].endswith("9223372036854775807"))
        lines[0] = lines[0][:lines[0].index(" // cerastes:result")]
        probs = lv.lint_vpr(*build_rows(lines), "t.vpr", "t.vpr.map")
        self.assertTrue(any("carries no `// cerastes:result" in p for p in probs), probs)


class SiteGuardQuantifierTests(unittest.TestCase):
    """Decision 0062 item 7 (cut 2), the positions of Part C: the quantified `requires` clause of an applied callee is a conjunct of the application's
    guard in a METHOD's `requires` and `ensures` and in the `assert` row of a call, its element reads in the site's contract form (membership in the
    interval of an atom of the site, or of the callee at the arguments).  The fixtures are the emitted `sq` (a trusted contract whose `ensures` applies
    `smallsum` of `requires cerastes_forall(small_at, a, n)`) and the emitted caller `sqc` up to its call; the twins change one line."""

    CONTRACT = '\n// the memory model (decision 0032 item 5, decision 0028 item 2): one value field and one flag field per cell type, the liveness cell\nfield v_int: Int\nfield d_int: Bool\nfield lv: Bool\n\n// cerastes:domain-ok 0028\ndomain Block {\n  function loc(b: Block, i: Int): Ref\n  function len(b: Block): Int\n  function block_of(r: Ref): Block\n  function offset_of(r: Ref): Int\n  function member(r: Ref, k: Int): Block\n  function parent_of(b: Block): Ref\n  function index_of(b: Block): Int\n  function lvloc(b: Block): Ref\n  // cerastes:axiom-ok 0033\n  axiom loc_inv { forall b: Block, i: Int :: { loc(b, i) } block_of(loc(b, i)) == b && offset_of(loc(b, i)) == i }\n  // cerastes:axiom-ok 0034\n  axiom loc_nonnull { forall b: Block, i: Int :: { loc(b, i) } loc(b, i) != null }\n  // cerastes:axiom-ok 0035\n  axiom member_inv { forall r: Ref, k: Int :: { member(r, k) } parent_of(member(r, k)) == r && index_of(member(r, k)) == k }\n  // cerastes:axiom-ok 0036\n  axiom member_lv { forall r: Ref, k: Int :: { lvloc(member(r, k)) } lvloc(member(r, k)) == lvloc(block_of(r)) }\n}\n\n// C/C++ function `smallsum` (a `pure` function, record 0062)\nfunction pf_smallsum__0(p_a__0: Ref, p_n__1: Int): Int // cerastes:result -2147483648 2147483647\n  requires ((-2147483648) <= p_n__1) && (p_n__1 <= 2147483647)\n  requires (p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0))))\n  requires p_n__1 >= 0\n  requires p_n__1 <= 1000\n  requires p_a__0 != null\n  requires (p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0))))\n  requires 0 <= p_n__1\n  requires (offset_of(p_a__0) + p_n__1) <= len(block_of(p_a__0))\n  requires forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> acc(loc(block_of(p_a__0), k).v_int, wildcard)\n  requires forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> (((-2147483648) <= loc(block_of(p_a__0), k).v_int) && (loc(block_of(p_a__0), k).v_int <= 2147483647))\n  requires forall q0: Int :: { loc(block_of(p_a__0), q0) } (((-2147483648) <= (q0 - offset_of(p_a__0))) && ((q0 - offset_of(p_a__0)) <= 2147483647)) ==> ((!(((q0 - offset_of(p_a__0)) >= 0) && ((q0 - offset_of(p_a__0)) < p_n__1))) || ((((((block_of(loc(block_of(p_a__0), q0)) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(p_a__0), q0)))) && (offset_of(loc(block_of(p_a__0), q0)) < (offset_of(p_a__0) + p_n__1))) ? loc(block_of(p_a__0), q0).v_int : 0) >= (-1000)) && (((((block_of(loc(block_of(p_a__0), q0)) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(p_a__0), q0)))) && (offset_of(loc(block_of(p_a__0), q0)) < (offset_of(p_a__0) + p_n__1))) ? loc(block_of(p_a__0), q0).v_int : 0) <= 1000)))\n  ensures (((-1000) * p_n__1) <= result) && (result <= (1000 * p_n__1))\n  decreases p_n__1\n{\n  0\n}\n\n// C/C++ function `sq`\n// cerastes:trusted-contract declared here, defined in another unit\nmethod f_sq__1(p_a__0: Ref, p_n__1: Int, amt_0: Perm) returns (res: Int)\n  requires ((-2147483648) <= p_n__1) && (p_n__1 <= 2147483647)\n  requires (none < amt_0) && (amt_0 < write)\n  requires (p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0))))\n  requires p_n__1 >= 0\n  requires p_n__1 <= 1000\n  requires p_a__0 != null\n  requires (p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0))))\n  requires 0 <= p_n__1\n  requires (offset_of(p_a__0) + p_n__1) <= len(block_of(p_a__0))\n  requires forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> acc(loc(block_of(p_a__0), k).v_int, amt_0)\n  requires forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> acc(loc(block_of(p_a__0), k).d_int, amt_0)\n  requires forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> loc(block_of(p_a__0), k).d_int\n  requires forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> (((-2147483648) <= loc(block_of(p_a__0), k).v_int) && (loc(block_of(p_a__0), k).v_int <= 2147483647))\n  requires acc(lvloc(block_of(p_a__0)).lv, amt_0)\n  requires forall q0: Int :: { loc(block_of(p_a__0), q0) } (((-2147483648) <= (q0 - offset_of(p_a__0))) && ((q0 - offset_of(p_a__0)) <= 2147483647)) ==> ((!(((q0 - offset_of(p_a__0)) >= 0) && ((q0 - offset_of(p_a__0)) < p_n__1))) || ((((((block_of(loc(block_of(p_a__0), q0)) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(p_a__0), q0)))) && (offset_of(loc(block_of(p_a__0), q0)) < (offset_of(p_a__0) + p_n__1))) ? loc(block_of(p_a__0), q0).v_int : 0) >= (-1000)) && (((((block_of(loc(block_of(p_a__0), q0)) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(p_a__0), q0)))) && (offset_of(loc(block_of(p_a__0), q0)) < (offset_of(p_a__0) + p_n__1))) ? loc(block_of(p_a__0), q0).v_int : 0) <= 1000)))\n  ensures ((-2147483648) <= res) && (res <= 2147483647)\n  ensures p_a__0 != null\n  ensures (p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0))))\n  ensures 0 <= p_n__1\n  ensures (offset_of(p_a__0) + p_n__1) <= len(block_of(p_a__0))\n  ensures forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> acc(loc(block_of(p_a__0), k).v_int, amt_0)\n  ensures forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> acc(loc(block_of(p_a__0), k).d_int, amt_0)\n  ensures forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> loc(block_of(p_a__0), k).d_int\n  ensures forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> (((-2147483648) <= loc(block_of(p_a__0), k).v_int) && (loc(block_of(p_a__0), k).v_int <= 2147483647))\n  ensures acc(lvloc(block_of(p_a__0)).lv, amt_0)\n  ensures res == (((((((((((((-2147483648) <= p_n__1) && (p_n__1 <= 2147483647)) && ((p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0)))))) && (p_n__1 >= 0)) && (p_n__1 <= 1000)) && (p_a__0 != null)) && ((p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0)))))) && (0 <= p_n__1)) && ((offset_of(p_a__0) + p_n__1) <= len(block_of(p_a__0)))) && (forall q0: Int :: { loc(block_of(p_a__0), q0) } (((-2147483648) <= (q0 - offset_of(p_a__0))) && ((q0 - offset_of(p_a__0)) <= 2147483647)) ==> ((!(((q0 - offset_of(p_a__0)) >= 0) && ((q0 - offset_of(p_a__0)) < p_n__1))) || ((((((block_of(loc(block_of(p_a__0), q0)) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(p_a__0), q0)))) && (offset_of(loc(block_of(p_a__0), q0)) < (offset_of(p_a__0) + p_n__1))) ? loc(block_of(p_a__0), q0).v_int : 0) >= (-1000)) && (((((block_of(loc(block_of(p_a__0), q0)) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(p_a__0), q0)))) && (offset_of(loc(block_of(p_a__0), q0)) < (offset_of(p_a__0) + p_n__1))) ? loc(block_of(p_a__0), q0).v_int : 0) <= 1000))))) && (((block_of(p_a__0) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(p_a__0))) && ((offset_of(p_a__0) + p_n__1) <= (offset_of(p_a__0) + p_n__1)))) ? pf_smallsum__0(p_a__0, p_n__1) : 0)\n  decreases _\n'
    CALLER = '\n// the memory model (decision 0032 item 5, decision 0028 item 2): one value field and one flag field per cell type, the liveness cell\nfield v_int: Int\nfield d_int: Bool\nfield lv: Bool\n\n// cerastes:domain-ok 0028\ndomain Block {\n  function loc(b: Block, i: Int): Ref\n  function len(b: Block): Int\n  function block_of(r: Ref): Block\n  function offset_of(r: Ref): Int\n  function member(r: Ref, k: Int): Block\n  function parent_of(b: Block): Ref\n  function index_of(b: Block): Int\n  function lvloc(b: Block): Ref\n  // cerastes:axiom-ok 0033\n  axiom loc_inv { forall b: Block, i: Int :: { loc(b, i) } block_of(loc(b, i)) == b && offset_of(loc(b, i)) == i }\n  // cerastes:axiom-ok 0034\n  axiom loc_nonnull { forall b: Block, i: Int :: { loc(b, i) } loc(b, i) != null }\n  // cerastes:axiom-ok 0035\n  axiom member_inv { forall r: Ref, k: Int :: { member(r, k) } parent_of(member(r, k)) == r && index_of(member(r, k)) == k }\n  // cerastes:axiom-ok 0036\n  axiom member_lv { forall r: Ref, k: Int :: { lvloc(member(r, k)) } lvloc(member(r, k)) == lvloc(block_of(r)) }\n}\n\n// C/C++ function `smallsum` (a `pure` function, record 0062)\nfunction pf_smallsum__0(p_a__0: Ref, p_n__1: Int): Int // cerastes:result -2147483648 2147483647\n  requires ((-2147483648) <= p_n__1) && (p_n__1 <= 2147483647)\n  requires (p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0))))\n  requires p_n__1 >= 0\n  requires p_n__1 <= 1000\n  requires p_a__0 != null\n  requires (p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0))))\n  requires 0 <= p_n__1\n  requires (offset_of(p_a__0) + p_n__1) <= len(block_of(p_a__0))\n  requires forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> acc(loc(block_of(p_a__0), k).v_int, wildcard)\n  requires forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> (((-2147483648) <= loc(block_of(p_a__0), k).v_int) && (loc(block_of(p_a__0), k).v_int <= 2147483647))\n  requires forall q0: Int :: { loc(block_of(p_a__0), q0) } (((-2147483648) <= (q0 - offset_of(p_a__0))) && ((q0 - offset_of(p_a__0)) <= 2147483647)) ==> ((!(((q0 - offset_of(p_a__0)) >= 0) && ((q0 - offset_of(p_a__0)) < p_n__1))) || ((((((block_of(loc(block_of(p_a__0), q0)) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(p_a__0), q0)))) && (offset_of(loc(block_of(p_a__0), q0)) < (offset_of(p_a__0) + p_n__1))) ? loc(block_of(p_a__0), q0).v_int : 0) >= (-1000)) && (((((block_of(loc(block_of(p_a__0), q0)) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(p_a__0), q0)))) && (offset_of(loc(block_of(p_a__0), q0)) < (offset_of(p_a__0) + p_n__1))) ? loc(block_of(p_a__0), q0).v_int : 0) <= 1000)))\n  ensures (((-1000) * p_n__1) <= result) && (result <= (1000 * p_n__1))\n  decreases p_n__1\n{\n  0\n}\n\nmethod f_sqc__2(p_a__0: Ref, p_n__1: Int, amt_0: Perm)\n  requires ((-2147483648) <= p_n__1) && (p_n__1 <= 2147483647)\n  requires (none < amt_0) && (amt_0 < write)\n  requires (p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0))))\n  requires p_n__1 >= 0\n  requires p_n__1 <= 1000\n  requires p_a__0 != null\n  requires (p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0))))\n  requires 0 <= p_n__1\n  requires (offset_of(p_a__0) + p_n__1) <= len(block_of(p_a__0))\n  requires forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> acc(loc(block_of(p_a__0), k).v_int, amt_0)\n  requires forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> acc(loc(block_of(p_a__0), k).d_int, amt_0)\n  requires forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> loc(block_of(p_a__0), k).d_int\n  requires forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> (((-2147483648) <= loc(block_of(p_a__0), k).v_int) && (loc(block_of(p_a__0), k).v_int <= 2147483647))\n  requires acc(lvloc(block_of(p_a__0)).lv, amt_0)\n  requires forall q0: Int :: { loc(block_of(p_a__0), q0) } (((-2147483648) <= (q0 - offset_of(p_a__0))) && ((q0 - offset_of(p_a__0)) <= 2147483647)) ==> ((!(((q0 - offset_of(p_a__0)) >= 0) && ((q0 - offset_of(p_a__0)) < p_n__1))) || ((((((block_of(loc(block_of(p_a__0), q0)) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(p_a__0), q0)))) && (offset_of(loc(block_of(p_a__0), q0)) < (offset_of(p_a__0) + p_n__1))) ? loc(block_of(p_a__0), q0).v_int : 0) >= (-1000)) && (((((block_of(loc(block_of(p_a__0), q0)) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(p_a__0), q0)))) && (offset_of(loc(block_of(p_a__0), q0)) < (offset_of(p_a__0) + p_n__1))) ? loc(block_of(p_a__0), q0).v_int : 0) <= 1000)))\n  requires (((((((((((((-2147483648) <= p_n__1) && (p_n__1 <= 2147483647)) && ((p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0)))))) && (p_n__1 >= 0)) && (p_n__1 <= 1000)) && (p_a__0 != null)) && ((p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0)))))) && (0 <= p_n__1)) && ((offset_of(p_a__0) + p_n__1) <= len(block_of(p_a__0)))) && (forall q0: Int :: { loc(block_of(p_a__0), q0) } (((-2147483648) <= (q0 - offset_of(p_a__0))) && ((q0 - offset_of(p_a__0)) <= 2147483647)) ==> ((!(((q0 - offset_of(p_a__0)) >= 0) && ((q0 - offset_of(p_a__0)) < p_n__1))) || ((((((block_of(loc(block_of(p_a__0), q0)) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(p_a__0), q0)))) && (offset_of(loc(block_of(p_a__0), q0)) < (offset_of(p_a__0) + p_n__1))) ? loc(block_of(p_a__0), q0).v_int : 0) >= (-1000)) && (((((block_of(loc(block_of(p_a__0), q0)) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(p_a__0), q0)))) && (offset_of(loc(block_of(p_a__0), q0)) < (offset_of(p_a__0) + p_n__1))) ? loc(block_of(p_a__0), q0).v_int : 0) <= 1000))))) && (((block_of(p_a__0) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(p_a__0))) && ((offset_of(p_a__0) + p_n__1) <= (offset_of(p_a__0) + p_n__1)))) ? pf_smallsum__0(p_a__0, p_n__1) : 0) <= 1000\n  ensures p_a__0 != null\n  ensures (p_a__0 == null) || (((p_a__0 == loc(block_of(p_a__0), offset_of(p_a__0))) && (0 <= offset_of(p_a__0))) && (offset_of(p_a__0) <= len(block_of(p_a__0))))\n  ensures 0 <= p_n__1\n  ensures (offset_of(p_a__0) + p_n__1) <= len(block_of(p_a__0))\n  ensures forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> acc(loc(block_of(p_a__0), k).v_int, amt_0)\n  ensures forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> acc(loc(block_of(p_a__0), k).d_int, amt_0)\n  ensures forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> loc(block_of(p_a__0), k).d_int\n  ensures forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> (((-2147483648) <= loc(block_of(p_a__0), k).v_int) && (loc(block_of(p_a__0), k).v_int <= 2147483647))\n  ensures acc(lvloc(block_of(p_a__0)).lv, amt_0)\n  decreases\n{\n  var v_a__0: Ref\n  var v_n__1: Int\n  var t__0: Ref\n  var t__1: Int\n  var t__2: Perm\n  var t__3: Int\n  v_a__0 := p_a__0\n  v_n__1 := p_n__1\n  t__0 := v_a__0\n  t__1 := v_n__1\n  assert (((t__0 != null) && ((t__0 == null) || (((t__0 == loc(block_of(t__0), offset_of(t__0))) && (0 <= offset_of(t__0))) && (offset_of(t__0) <= len(block_of(t__0)))))) && (0 <= t__1)) && ((offset_of(t__0) + t__1) <= len(block_of(t__0)))\n  assert !((t__1 > 0) && ((perm(loc(block_of(t__0), offset_of(t__0)).v_int) == none) || (perm(loc(block_of(t__0), offset_of(t__0)).d_int) == none)))\n  assert perm(lvloc(block_of(t__0)).lv) > none\n  t__2 := ((t__1 > 0) ? ((((((perm(loc(block_of(t__0), offset_of(t__0)).v_int) < perm(loc(block_of(t__0), offset_of(t__0)).d_int)) ? perm(loc(block_of(t__0), offset_of(t__0)).v_int) : perm(loc(block_of(t__0), offset_of(t__0)).d_int)) < perm(lvloc(block_of(t__0)).lv)) ? ((perm(loc(block_of(t__0), offset_of(t__0)).v_int) < perm(loc(block_of(t__0), offset_of(t__0)).d_int)) ? perm(loc(block_of(t__0), offset_of(t__0)).v_int) : perm(loc(block_of(t__0), offset_of(t__0)).d_int)) : perm(lvloc(block_of(t__0)).lv)) < amt_0) ? ((((perm(loc(block_of(t__0), offset_of(t__0)).v_int) < perm(loc(block_of(t__0), offset_of(t__0)).d_int)) ? perm(loc(block_of(t__0), offset_of(t__0)).v_int) : perm(loc(block_of(t__0), offset_of(t__0)).d_int)) < perm(lvloc(block_of(t__0)).lv)) ? ((perm(loc(block_of(t__0), offset_of(t__0)).v_int) < perm(loc(block_of(t__0), offset_of(t__0)).d_int)) ? perm(loc(block_of(t__0), offset_of(t__0)).v_int) : perm(loc(block_of(t__0), offset_of(t__0)).d_int)) : perm(lvloc(block_of(t__0)).lv)) : amt_0) : perm(lvloc(block_of(t__0)).lv)) / 2\n  assert none < t__2\n  assert t__2 < write\n  assert forall k: Int :: { loc(block_of(t__0), k) } ((offset_of(t__0) <= k) && (k < (offset_of(t__0) + t__1))) ==> acc(loc(block_of(t__0), k).v_int, t__2)\n  assert forall k: Int :: { loc(block_of(t__0), k) } ((offset_of(t__0) <= k) && (k < (offset_of(t__0) + t__1))) ==> acc(loc(block_of(t__0), k).d_int, t__2)\n  assert forall k: Int :: { loc(block_of(t__0), k) } ((offset_of(t__0) <= k) && (k < (offset_of(t__0) + t__1))) ==> loc(block_of(t__0), k).d_int\n  assert forall k: Int :: { loc(block_of(t__0), k) } ((offset_of(t__0) <= k) && (k < (offset_of(t__0) + t__1))) ==> (loc(block_of(t__0), k).d_int ==> (((-2147483648) <= loc(block_of(t__0), k).v_int) && (loc(block_of(t__0), k).v_int <= 2147483647)))\n  assert perm(lvloc(block_of(t__0)).lv) >= t__2\n  assert (v_a__0 != null) ==> (perm(lvloc(block_of(v_a__0)).lv) > none)\n  assert v_n__1 >= 0\n  assert v_n__1 <= 1000\n  assert forall q0: Int :: { loc(block_of(t__0), q0) } (((-2147483648) <= (q0 - offset_of(t__0))) && ((q0 - offset_of(t__0)) <= 2147483647)) ==> ((!(((q0 - offset_of(t__0)) >= 0) && ((q0 - offset_of(t__0)) < v_n__1))) || ((((((block_of(loc(block_of(t__0), q0)) == block_of(t__0)) && (offset_of(t__0) <= offset_of(loc(block_of(t__0), q0)))) && (offset_of(loc(block_of(t__0), q0)) < (offset_of(t__0) + v_n__1))) ? loc(block_of(t__0), q0).v_int : 0) >= (-1000)) && (((((block_of(loc(block_of(t__0), q0)) == block_of(t__0)) && (offset_of(t__0) <= offset_of(loc(block_of(t__0), q0)))) && (offset_of(loc(block_of(t__0), q0)) < (offset_of(t__0) + v_n__1))) ? loc(block_of(t__0), q0).v_int : 0) <= 1000)))\n  assert (((((((((((((-2147483648) <= v_n__1) && (v_n__1 <= 2147483647)) && ((t__0 == null) || (((t__0 == loc(block_of(t__0), offset_of(t__0))) && (0 <= offset_of(t__0))) && (offset_of(t__0) <= len(block_of(t__0)))))) && (v_n__1 >= 0)) && (v_n__1 <= 1000)) && (t__0 != null)) && ((t__0 == null) || (((t__0 == loc(block_of(t__0), offset_of(t__0))) && (0 <= offset_of(t__0))) && (offset_of(t__0) <= len(block_of(t__0)))))) && (0 <= v_n__1)) && ((offset_of(t__0) + v_n__1) <= len(block_of(t__0)))) && (forall q0: Int :: { loc(block_of(t__0), q0) } (((-2147483648) <= (q0 - offset_of(t__0))) && ((q0 - offset_of(t__0)) <= 2147483647)) ==> ((!(((q0 - offset_of(t__0)) >= 0) && ((q0 - offset_of(t__0)) < v_n__1))) || ((((((block_of(loc(block_of(t__0), q0)) == block_of(t__0)) && (offset_of(t__0) <= offset_of(loc(block_of(t__0), q0)))) && (offset_of(loc(block_of(t__0), q0)) < (offset_of(t__0) + v_n__1))) ? loc(block_of(t__0), q0).v_int : 0) >= (-1000)) && (((((block_of(loc(block_of(t__0), q0)) == block_of(t__0)) && (offset_of(t__0) <= offset_of(loc(block_of(t__0), q0)))) && (offset_of(loc(block_of(t__0), q0)) < (offset_of(t__0) + v_n__1))) ? loc(block_of(t__0), q0).v_int : 0) <= 1000))))) && (((block_of(t__0) == block_of(t__0)) && (offset_of(t__0) <= offset_of(t__0))) && ((offset_of(t__0) + v_n__1) <= (offset_of(t__0) + v_n__1)))) ? pf_smallsum__0(t__0, v_n__1) : 0) <= 1000\n  t__3 := f_sq__1(t__0, t__1, t__2)\n}\n'

    @staticmethod
    def quantifier_line(l):
        return "forall q0" in l and "requires" in l.split("forall q0")[0] or ("forall q0" in l and l.lstrip().startswith(("ensures", "assert")) and "hva_" not in l and "pf_smallsum__0(" in l)

    def lint(self, text, closing=""):
        return text.split("\n"), lv.lint_vpr(text + closing, None, "t.vpr", "t.vpr.map")

    @staticmethod
    def applying(l):
        # a method clause or row that applies the callee under a guard that holds the quantifier
        return "forall q0" in l and "pf_smallsum__0(" in l and l.lstrip().split(" ")[0] in ("requires", "ensures", "assert")

    def at(self, lines, probs, predicate):
        hits = [i for i, l in enumerate(lines, start=1) if predicate(l)]
        self.assertTrue(hits)
        return hits, [p for p in probs if any(p.startswith(f"t.vpr:{i}:") for i in hits)]

    def mutate(self, text, old, new):
        lines = text.split("\n")
        hits = [i for i, l in enumerate(lines) if self.applying(l)]
        self.assertEqual(len(hits), 1, hits)
        self.assertIn(old, lines[hits[0]])
        lines[hits[0]] = lines[hits[0]].replace(old, new)
        return "\n".join(lines)

    CASES = {
        "a weaker element condition": (">= (-1000)", ">= (-999)"),
        "another bound": ("< p_n__1)", "< (p_n__1 - 1))"),
        "reads over an interval that is neither the site's nor the callee's": ("(offset_of(p_a__0) + p_n__1)", "(offset_of(p_a__0) + (p_n__1 - 3))"),
    }

    def test_the_emitted_ensures_with_the_quantified_guard_is_clean(self):
        lines, probs = self.lint(self.CONTRACT)
        hits, got = self.at(lines, probs, self.applying)
        self.assertEqual([p for p in got if "decision 0061 item 11" in p], [])

    def test_the_twins_of_an_ensures(self):
        for label, (old, new) in self.CASES.items():
            with self.subTest(case=label):
                text = self.mutate(self.CONTRACT, old, new)
                lines, probs = self.lint(text)
                hits, got = self.at(lines, probs, self.applying)
                self.assertTrue(any("decision 0061 item 11" in p and "applied there" in p for p in got), (label, got))

    def test_the_emitted_requires_and_call_row_are_clean(self):
        lines, probs = self.lint(self.CALLER)
        hits, got = self.at(lines, probs, self.applying)
        self.assertGreaterEqual(len(hits), 2)
        self.assertEqual([p for p in got if "decision 0061 item 11" in p], [])

    def test_the_twins_of_a_requires_and_a_call_row(self):
        lines = self.CALLER.split("\n")
        hits = [i for i, l in enumerate(lines) if self.applying(l)]
        self.assertEqual(len(hits), 2)
        for which in hits:
            for label, (old, new) in self.CASES.items():
                with self.subTest(line=which + 1, case=label):
                    if lines[which].lstrip().startswith("assert"):  # the call's row names the caller's variables
                        old, new = (x.replace("p_n__1", "v_n__1").replace("p_a__0", "t__0") for x in (old, new))
                    mutated = list(lines)
                    self.assertIn(old, mutated[which])
                    mutated[which] = mutated[which].replace(old, new)
                    text = "\n".join(mutated)
                    probs = lv.lint_vpr(text, None, "t.vpr", "t.vpr.map")
                    got = [p for p in probs if p.startswith(f"t.vpr:{which + 1}:")]
                    self.assertTrue(any("decision 0061 item 11" in p and "applied there" in p for p in got), (label, got))

    def test_a_quantifier_in_a_line_that_applies_nothing_is_a_clause_of_its_own(self):
        lines = self.CONTRACT.split("\n")
        hits = [i for i, l in enumerate(lines) if self.applying(l)]
        mutated = list(lines)
        mutated[hits[0]] = mutated[hits[0]].replace("pf_smallsum__0(", "pf_other__9(")
        probs = lv.lint_vpr("\n".join(mutated), None, "t.vpr", "t.vpr.map")
        self.assertTrue(any(p.startswith(f"t.vpr:{hits[0] + 1}:") for p in probs), probs)


class InvariantGuardQuantifierTests(unittest.TestCase):
    """Decision 0062 item 7 (cut 2) in a loop's `invariant` line (Part D; the emitted `ctd_forall_sum`): the user's conjunct `s == (G ? pf(a, i) : 0)` whose
    guard G holds the callee's quantified `requires` clause at the arguments (`a`, `i`), its element reads' membership the interval of the method's own atom
    (the formal `p_a` with the count `p_n`) while the read names the loop's variable `v_a`; and the lines that restate it under `prior ==>`.  The fixture
    is `SiteGuardQuantifierTests.CONTRACT` (the emitted callee and trusted `sq`) plus a method with a loop whose invariant lines are made from `sq`'s `ensures`
    application by renaming (the arguments to the loop's variables, the read's membership kept on the formal).  Twins: another element condition, another
    bound, an interval that is neither the site's nor the callee's, a read pointer that is not the callee's argument, the cross form outside an `invariant`,
    an unparenthesized quantifier, a line that applies nothing."""

    MEMBER = ("(block_of(loc(block_of(p_a__0), q0)) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(p_a__0), q0)))) && "
              "(offset_of(loc(block_of(p_a__0), q0)) < (offset_of(p_a__0) + p_n__1))")
    MEMBER_CROSS = ("(block_of(loc(block_of(v_a__0), q0)) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(v_a__0), q0)))) && "
                    "(offset_of(loc(block_of(v_a__0), q0)) < (offset_of(p_a__0) + p_n__1))")
    INCLUSION = ("(((block_of(p_a__0) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(p_a__0))) && "
                 "((offset_of(p_a__0) + p_n__1) <= (offset_of(p_a__0) + p_n__1)))")
    INCLUSION_LOOP = ("(((block_of(v_a__0) == block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(v_a__0))) && "
                      "((offset_of(v_a__0) + v_i__3) <= (offset_of(p_a__0) + p_n__1)))")

    @classmethod
    def value(cls):
        base = SiteGuardQuantifierTests.CONTRACT
        lines = base.split("\n")
        ens = next(l for l in lines if l.startswith("  ensures res == ") and "forall q0" in l)
        v = ens[len("  ensures res == "):]
        assert cls.MEMBER in v and cls.INCLUSION in v and v.endswith("? pf_smallsum__0(p_a__0, p_n__1) : 0)")
        v = v.replace(cls.MEMBER, "@MEMBER@").replace(cls.INCLUSION, "@INCLUSION@")
        v = v.replace("p_a__0", "v_a__0").replace("p_n__1", "v_i__3")
        return v.replace("@MEMBER@", cls.MEMBER_CROSS).replace("@INCLUSION@", cls.INCLUSION_LOOP)

    @classmethod
    def program(cls, invariants=None, kind="invariant"):
        base = SiteGuardQuantifierTests.CONTRACT
        lines = base.split("\n")
        start = next(i for i, l in enumerate(lines) if l.startswith("method f_sq__1("))
        header = []
        for l in lines[start:]:
            if l.startswith("  ensures") or l.startswith("  decreases"):
                break
            header.append(l)
        header[0] = header[0].replace("f_sq__1", "f_lp__2")
        value = cls.value()
        user = "((v_i__3 >= 0) && (v_i__3 <= v_n__1)) && (v_s__2 == " + value + ")"
        invs = invariants if invariants is not None else [user, "(" + user + ") ==> (!(((v_n__1 - v_i__3) < (-2147483648)) || (2147483647 < (v_n__1 - v_i__3))))"]
        body = ["  ensures ((-2147483648) <= res) && (res <= 2147483647)", "  decreases _", "{",
                "  var v_a__0: Ref", "  var v_n__1: Int", "  var v_i__3: Int", "  var v_s__2: Int", "  v_a__0 := p_a__0", "  v_n__1 := p_n__1",
                "  v_i__3 := 0", "  v_s__2 := 0",
                "  while (v_i__3 < v_n__1)"]
        body += [f"    {kind} {x}" for x in invs]
        body += ["    decreases v_n__1 - v_i__3", "  {", "    v_i__3 := v_i__3 + 1", "  }", "  res := v_s__2", "}"]
        return base + "\n" + "\n".join(header + body) + "\n"

    def probs(self, text):
        return text.split("\n"), lv.lint_vpr(text, None, "t.vpr", "t.vpr.map")

    @staticmethod
    def user_lines(lines):
        return [i for i, l in enumerate(lines, start=1) if l.startswith("    ") and "forall q0" in l and "pf_smallsum__0(v_a__0, v_i__3)" in l]

    def item11(self, text):
        lines, probs = self.probs(text)
        hits = self.user_lines(lines)
        self.assertEqual(len(hits), 2, hits)
        return {n: [p for p in probs if p.startswith(f"t.vpr:{n}:") and "decision 0061 item 11" in p] for n in hits}

    def test_the_emitted_invariant_lines_are_clean(self):
        for n, got in self.item11(self.program()).items():
            self.assertEqual(got, [], (n, got))

    def test_the_twins_of_the_user_invariant_line(self):
        cases = {
            "a weaker element condition": (">= (-1000)", ">= (-999)"),
            "another bound": ("(offset_of(p_a__0) + p_n__1))", "(offset_of(p_a__0) + (p_n__1 - 1)))"),
            "an interval that is neither the site's nor the callee's": ("< (offset_of(p_a__0) + p_n__1))", "< (offset_of(p_a__0) + (p_n__1 - 3)))"),
            "a read pointer that is not the callee's argument": ("loc(block_of(v_a__0), q0)) == block_of(p_a__0)", "loc(block_of(p_a__0), q0)) == block_of(p_a__0)"),
            "a membership pointer that is not an atom of the site": ("== block_of(p_a__0)) && (offset_of(p_a__0) <= offset_of(loc(block_of(v_a__0), q0)))",
                                                                  "== block_of(v_a__0)) && (offset_of(v_a__0) <= offset_of(loc(block_of(v_a__0), q0)))"),
        }
        for label, (old, new) in cases.items():
            for which in (0, 1):
                with self.subTest(case=label, line=which):
                    text = self.program()
                    lines = text.split("\n")
                    n = self.user_lines(lines)[which]
                    self.assertIn(old, lines[n - 1], label)
                    lines[n - 1] = lines[n - 1].replace(old, new)
                    got = self.item11("\n".join(lines))
                    self.assertTrue(any("applied there" in p for p in got[n]), (label, n, got[n]))

    def test_the_other_line_of_a_twin_stays_clean(self):
        text = self.program()
        lines = text.split("\n")
        n = self.user_lines(lines)[0]
        lines[n - 1] = lines[n - 1].replace(">= (-1000)", ">= (-999)")
        got = self.item11("\n".join(lines))
        other = [k for k in got if k != n]
        self.assertTrue(got[n])
        self.assertEqual(got[other[0]], [])

    def test_the_cross_form_is_for_an_invariant_only(self):
        # the same conjunct in a method's `ensures` or a call's `assert` stays refused: a contract's read membership is the read pointer's own interval
        for kind in ("ensures", "assert"):
            with self.subTest(kind=kind):
                text = self.program(kind=kind)
                lines, probs = self.probs(text)
                hits = self.user_lines(lines)
                self.assertTrue(hits)
                for n in hits:
                    self.assertTrue(any(p.startswith(f"t.vpr:{n}:") and "applied there" in p for p in probs), (kind, n))

    def test_an_unparenthesized_quantifier_and_a_line_that_applies_nothing(self):
        text = self.program()
        lines = text.split("\n")
        n = self.user_lines(lines)[0]
        bare = list(lines)
        bare[n - 1] = bare[n - 1].replace("&& (forall q0", "&& forall q0", 1)
        probs = lv.lint_vpr("\n".join(bare), None, "t.vpr", "t.vpr.map")
        self.assertTrue(any(p.startswith(f"t.vpr:{n}:") and "parenthesized" in p for p in probs), probs)
        nothing = list(lines)
        nothing[n - 1] = nothing[n - 1].replace("pf_smallsum__0(v_a__0, v_i__3)", "pf_other__9(v_a__0, v_i__3)")
        probs = lv.lint_vpr("\n".join(nothing), None, "t.vpr", "t.vpr.map")
        self.assertTrue(any(p.startswith(f"t.vpr:{n}:") for p in probs), probs)

    def test_an_invariant_quantifier_of_the_old_shapes_is_unchanged(self):
        # a loop conjunct of decision 0059 item 8 in an invariant line that applies no pure function keeps its own verdict
        text = self.program(invariants=["forall k: Int :: { loc(block_of(p_a__0), k) } ((offset_of(p_a__0) <= k) && (k < (offset_of(p_a__0) + p_n__1))) ==> acc(loc(block_of(p_a__0), k).v_int, amt_0)"])
        lines, probs = self.probs(text)
        n = next(i for i, l in enumerate(lines, start=1) if l.startswith("    invariant forall k"))
        self.assertEqual([p for p in probs if p.startswith(f"t.vpr:{n}:")], [])


class KeepFactCellTests(unittest.TestCase):
    """Decision 0062 item 8 (iii), cell atoms (audit round 3, R3-S1): the keep fact over a method's `reads(*p)` atom and its `ensures` partner, as lowering
    emits it (the audit's `k2_heap_guard_cell`: `flip` reads `*a` under `*flag != 0` in the requires and in the ensures, and applies `get(a)`, whose
    `requires` reads the cell): the guard is the entry-state condition over the read ghost `hv_0_pre`, the partner's over `hv_0_post`, and the one-slot
    inclusion of the callee's cell.  Twins: unguarded, a guard of one state only, the wrong state, another cell, a weaker condition."""

    FIXTURE = 'field v_int: Int\nfield d_int: Bool\nfield lv: Bool\n\n// cerastes:domain-ok 0028\ndomain Block {\n  function loc(b: Block, i: Int): Ref\n  function len(b: Block): Int\n  function block_of(r: Ref): Block\n  function offset_of(r: Ref): Int\n  function member(r: Ref, k: Int): Block\n  function parent_of(b: Block): Ref\n  function index_of(b: Block): Int\n  function lvloc(b: Block): Ref\n  // cerastes:axiom-ok 0033\n  axiom loc_inv { forall b: Block, i: Int :: { loc(b, i) } block_of(loc(b, i)) == b && offset_of(loc(b, i)) == i }\n  // cerastes:axiom-ok 0034\n  axiom loc_nonnull { forall b: Block, i: Int :: { loc(b, i) } loc(b, i) != null }\n  // cerastes:axiom-ok 0035\n  axiom member_inv { forall r: Ref, k: Int :: { member(r, k) } parent_of(member(r, k)) == r && index_of(member(r, k)) == k }\n  // cerastes:axiom-ok 0036\n  axiom member_lv { forall r: Ref, k: Int :: { lvloc(member(r, k)) } lvloc(member(r, k)) == lvloc(block_of(r)) }\n}\n\n// C/C++ function `get` (a `pure` function, record 0062)\nfunction pf_get__0(p_p__0: Ref): Int // cerastes:result -2147483648 2147483647\n  requires (p_p__0 == null) || (((p_p__0 == loc(block_of(p_p__0), offset_of(p_p__0))) && (0 <= offset_of(p_p__0))) && (offset_of(p_p__0) <= len(block_of(p_p__0))))\n  requires p_p__0 != null\n  requires (p_p__0 == null) || (((p_p__0 == loc(block_of(p_p__0), offset_of(p_p__0))) && (0 <= offset_of(p_p__0))) && (offset_of(p_p__0) <= len(block_of(p_p__0))))\n  requires offset_of(p_p__0) < len(block_of(p_p__0))\n  requires acc(p_p__0.v_int, wildcard)\n  requires ((-2147483648) <= p_p__0.v_int) && (p_p__0.v_int <= 2147483647)\n  decreases\n{\n  p_p__0.v_int\n}\n\n// cerastes:trusted-contract declared here, defined in another unit\nmethod f_flip__2(p_flag__0: Ref, p_a__1: Ref, p_b__2: Ref, amt_0: Perm, amt_1: Perm, amt_2: Perm) returns (res: Int)\n  requires (none < amt_0) && (amt_0 < write)\n  requires (none < amt_1) && (amt_1 < write)\n  requires (none < amt_2) && (amt_2 < write)\n  requires (p_flag__0 == null) || (((p_flag__0 == loc(block_of(p_flag__0), offset_of(p_flag__0))) && (0 <= offset_of(p_flag__0))) && (offset_of(p_flag__0) <= len(block_of(p_flag__0))))\n  requires (p_a__1 == null) || (((p_a__1 == loc(block_of(p_a__1), offset_of(p_a__1))) && (0 <= offset_of(p_a__1))) && (offset_of(p_a__1) <= len(block_of(p_a__1))))\n  requires (p_b__2 == null) || (((p_b__2 == loc(block_of(p_b__2), offset_of(p_b__2))) && (0 <= offset_of(p_b__2))) && (offset_of(p_b__2) <= len(block_of(p_b__2))))\n  requires (((((((((((p_flag__0 != null) && ((p_flag__0 == null) || (((p_flag__0 == loc(block_of(p_flag__0), offset_of(p_flag__0))) && (0 <= offset_of(p_flag__0))) && (offset_of(p_flag__0) <= len(block_of(p_flag__0)))))) && (offset_of(p_flag__0) < len(block_of(p_flag__0)))) && acc(p_flag__0.v_int)) && acc(p_flag__0.d_int)) && p_flag__0.d_int) && (((-2147483648) <= p_flag__0.v_int) && (p_flag__0.v_int <= 2147483647))) && acc(lvloc(block_of(p_flag__0)).lv, amt_0)) && (((p_b__2 == p_a__1) ? 1 : 0) != 0)) && ((!(p_flag__0.v_int == 0)) ==> ((((((((p_a__1 != null) && ((p_a__1 == null) || (((p_a__1 == loc(block_of(p_a__1), offset_of(p_a__1))) && (0 <= offset_of(p_a__1))) && (offset_of(p_a__1) <= len(block_of(p_a__1)))))) && (offset_of(p_a__1) < len(block_of(p_a__1)))) && acc(p_a__1.v_int, amt_1)) && acc(p_a__1.d_int, amt_1)) && p_a__1.d_int) && (((-2147483648) <= p_a__1.v_int) && (p_a__1.v_int <= 2147483647))) && acc(lvloc(block_of(p_a__1)).lv, amt_1)))) && ((!(p_flag__0.v_int != 0)) ==> ((((((((p_b__2 != null) && ((p_b__2 == null) || (((p_b__2 == loc(block_of(p_b__2), offset_of(p_b__2))) && (0 <= offset_of(p_b__2))) && (offset_of(p_b__2) <= len(block_of(p_b__2)))))) && (offset_of(p_b__2) < len(block_of(p_b__2)))) && acc(p_b__2.v_int)) && acc(p_b__2.d_int)) && p_b__2.d_int) && (((-2147483648) <= p_b__2.v_int) && (p_b__2.v_int <= 2147483647))) && acc(lvloc(block_of(p_b__2)).lv, amt_2)))) && ((((((((p_a__1 == null) || (((p_a__1 == loc(block_of(p_a__1), offset_of(p_a__1))) && (0 <= offset_of(p_a__1))) && (offset_of(p_a__1) <= len(block_of(p_a__1))))) && (p_a__1 != null)) && ((p_a__1 == null) || (((p_a__1 == loc(block_of(p_a__1), offset_of(p_a__1))) && (0 <= offset_of(p_a__1))) && (offset_of(p_a__1) <= len(block_of(p_a__1)))))) && (offset_of(p_a__1) < len(block_of(p_a__1)))) && (((((block_of(p_a__1) == block_of(p_flag__0)) && (offset_of(p_flag__0) <= offset_of(p_a__1))) && ((offset_of(p_a__1) + 1) <= (offset_of(p_flag__0) + 1))) || ((((!(p_flag__0.v_int == 0)) && (block_of(p_a__1) == block_of(p_a__1))) && (offset_of(p_a__1) <= offset_of(p_a__1))) && ((offset_of(p_a__1) + 1) <= (offset_of(p_a__1) + 1)))) || ((((!(p_flag__0.v_int != 0)) && (block_of(p_a__1) == block_of(p_b__2))) && (offset_of(p_b__2) <= offset_of(p_a__1))) && ((offset_of(p_a__1) + 1) <= (offset_of(p_b__2) + 1))))) ? pf_get__0(p_a__1) : 0) < 100)\n  ensures ((-9223372036854775808) <= res) && (res <= 9223372036854775807)\n  ensures ((((((((((p_flag__0 != null) && ((p_flag__0 == null) || (((p_flag__0 == loc(block_of(p_flag__0), offset_of(p_flag__0))) && (0 <= offset_of(p_flag__0))) && (offset_of(p_flag__0) <= len(block_of(p_flag__0)))))) && (offset_of(p_flag__0) < len(block_of(p_flag__0)))) && acc(p_flag__0.v_int)) && acc(p_flag__0.d_int)) && p_flag__0.d_int) && (((-2147483648) <= p_flag__0.v_int) && (p_flag__0.v_int <= 2147483647))) && acc(lvloc(block_of(p_flag__0)).lv, amt_0)) && (p_flag__0.v_int == 1)) && ((!(p_flag__0.v_int == 0)) ==> ((((((((p_a__1 != null) && ((p_a__1 == null) || (((p_a__1 == loc(block_of(p_a__1), offset_of(p_a__1))) && (0 <= offset_of(p_a__1))) && (offset_of(p_a__1) <= len(block_of(p_a__1)))))) && (offset_of(p_a__1) < len(block_of(p_a__1)))) && acc(p_a__1.v_int, amt_1)) && acc(p_a__1.d_int, amt_1)) && p_a__1.d_int) && (((-2147483648) <= p_a__1.v_int) && (p_a__1.v_int <= 2147483647))) && acc(lvloc(block_of(p_a__1)).lv, amt_1)))) && (res == ((((((((p_a__1 == null) || (((p_a__1 == loc(block_of(p_a__1), offset_of(p_a__1))) && (0 <= offset_of(p_a__1))) && (offset_of(p_a__1) <= len(block_of(p_a__1))))) && (p_a__1 != null)) && ((p_a__1 == null) || (((p_a__1 == loc(block_of(p_a__1), offset_of(p_a__1))) && (0 <= offset_of(p_a__1))) && (offset_of(p_a__1) <= len(block_of(p_a__1)))))) && (offset_of(p_a__1) < len(block_of(p_a__1)))) && ((((block_of(p_a__1) == block_of(p_flag__0)) && (offset_of(p_flag__0) <= offset_of(p_a__1))) && ((offset_of(p_a__1) + 1) <= (offset_of(p_flag__0) + 1))) || ((((!(p_flag__0.v_int == 0)) && (block_of(p_a__1) == block_of(p_a__1))) && (offset_of(p_a__1) <= offset_of(p_a__1))) && ((offset_of(p_a__1) + 1) <= (offset_of(p_a__1) + 1))))) ? pf_get__0(p_a__1) : 0) + 9223372036854775000))\n  decreases _\n\n\n// cerastes:domain-ok 0061\ndomain Hvq_f_flip__2 {\n  function hva_f_flip__2__pf_get__0_pre(x0: Ref): Int\n  function hva_f_flip__2__pf_get__0_post(x0: Ref): Int\n}\nmethod f_flip__2__wf(p_flag__0: Ref, p_a__1: Ref, p_b__2: Ref, amt_0: Perm, amt_1: Perm, amt_2: Perm, res: Int, hv_0_pre: Int, hv_0_post: Int)\n  requires (none < amt_0) && (amt_0 < write)\n  requires (none < amt_1) && (amt_1 < write)\n  requires (none < amt_2) && (amt_2 < write)\n  requires (p_flag__0 == null) || (((p_flag__0 == loc(block_of(p_flag__0), offset_of(p_flag__0))) && (0 <= offset_of(p_flag__0))) && (offset_of(p_flag__0) <= len(block_of(p_flag__0))))\n  requires (p_a__1 == null) || (((p_a__1 == loc(block_of(p_a__1), offset_of(p_a__1))) && (0 <= offset_of(p_a__1))) && (offset_of(p_a__1) <= len(block_of(p_a__1))))\n  requires (p_b__2 == null) || (((p_b__2 == loc(block_of(p_b__2), offset_of(p_b__2))) && (0 <= offset_of(p_b__2))) && (offset_of(p_b__2) <= len(block_of(p_b__2))))\n  requires ((-9223372036854775808) <= res) && (res <= 9223372036854775807)\n  requires ((-2147483648) <= hv_0_pre) && (hv_0_pre <= 2147483647)\n  requires ((-2147483648) <= hv_0_post) && (hv_0_post <= 2147483647)\n  requires (((!(hv_0_pre == 0)) && (!(hv_0_post == 0))) && (((block_of(p_a__1) == block_of(p_a__1)) && (offset_of(p_a__1) <= offset_of(p_a__1))) && ((offset_of(p_a__1) + 1) <= (offset_of(p_a__1) + 1)))) ==> (hva_f_flip__2__pf_get__0_post(p_a__1) == hva_f_flip__2__pf_get__0_pre(p_a__1))\n'
    TAIL = "  decreases\n{\n}\nmethod pf_get__0__wf()\n  decreases\n{\n}\n"

    @staticmethod
    def keep_line(l):
        return l.startswith("  requires") and "hva_f_flip__2__pf_get__0_post(p_a__1) == hva_f_flip__2__pf_get__0_pre(p_a__1)" in l

    def problems_at_fact(self, text):
        lines = text.split("\n")
        hits = [i for i, l in enumerate(lines, start=1) if self.keep_line(l)]
        self.assertEqual(len(hits), 1, hits)
        probs = lv.lint_vpr(text + self.TAIL, None, "t.vpr", "t.vpr.map")
        return [p for p in probs if p.startswith(f"t.vpr:{hits[0]}:")]

    def mutate(self, old, new):
        lines = self.FIXTURE.split("\n")
        hits = [i for i, l in enumerate(lines) if self.keep_line(l)]
        self.assertIn(old, lines[hits[0]])
        lines[hits[0]] = lines[hits[0]].replace(old, new)
        return "\n".join(lines)

    def test_the_emitted_cell_keep_fact_is_clean(self):
        self.assertEqual(self.problems_at_fact(self.FIXTURE), [])

    def test_the_twins_of_the_cell_keep_fact(self):
        cases = {
            "unguarded (inclusion only)": ("(!(hv_0_pre == 0)) && (!(hv_0_post == 0))", "(offset_of(p_a__1) < len(block_of(p_a__1)))"),
            "the entry guard only": ("(!(hv_0_pre == 0)) && (!(hv_0_post == 0))", "(!(hv_0_pre == 0))"),
            "the partner's guard only": ("(!(hv_0_pre == 0)) && (!(hv_0_post == 0))", "(!(hv_0_post == 0))"),
            "the post-state ghost in the entry guard": ("(!(hv_0_pre == 0)) && (!(hv_0_post == 0))", "(!(hv_0_post == 0)) && (!(hv_0_post == 0))"),
            "a weaker condition": ("(!(hv_0_pre == 0)) &&", "(!(hv_0_pre == 1)) &&"),
            "the pre-state ghost in the partner's guard": ("(!(hv_0_post == 0))", "(!(hv_0_pre == 0))"),
            "another cell": ("(offset_of(p_a__1) <= offset_of(p_a__1))", "(offset_of(p_a__1) <= offset_of(p_b__2))"),
        }
        for label, (old, new) in cases.items():
            with self.subTest(case=label):
                probs = self.problems_at_fact(self.mutate(old, new))
                self.assertTrue(any("decision 0062" in p for p in probs), (label, probs))


class ArrayGlobalRowTests(unittest.TestCase):
    """Decision 0063 items 9 and 15 (M4.5e1e1): the row group of an array global in `program_start`, in the order of item 9 (the two quantified
    permissions and the flags over `[0, N)`, one conjunct per listed element then one quantifier for the rest, `acc(lvloc(GB_g()).lv)`), the facts
    `len(GB_g()) == N` the same in the three places the facts are pinned, and the block-term rows of an array atom in a method's contract.  The shapes are
    tests/viper-smoke/memory/s47_a_global.vpr and s47_b_calls.vpr's; each twin breaks one thing."""

    G = GlobalsTests("test_the_emitters_shape_is_clean")
    NAMES = ["GB_g__1", "GB_h__2"]
    H_ROW = GlobalsTests.row("GB_h__2", "int", "(loc(GB_h__2(), 0).v_int == 3)")
    INT_RANGE = "(((-2147483648) <= {c}.v_int) && ({c}.v_int <= 2147483647))"

    @staticmethod
    def q(b, lo, hi, body, k="k"):
        return f"forall {k}: Int :: {{ loc({b}, {k}) }} (({lo} <= {k}) && ({k} < {hi})) ==> {body}"

    @classmethod
    def group(cls, g="GB_g__1", n=4, t="int", listed=("1", "2"), rest="zero", lv=True):
        """The rows of an array global of n elements: `listed` are the written values, `rest` the zero rest ("zero"), an unknown value's range
        ("unknown") or nothing (None)."""
        b = f"{g}()"
        rows = [cls.q(b, 0, n, f"acc(loc({b}, k).v_{t})"), cls.q(b, 0, n, f"acc(loc({b}, k).d_{t})"), cls.q(b, 0, n, f"loc({b}, k).d_{t}")]
        rows += [f"loc({b}, {i}).v_{t} == {v}" for i, v in enumerate(listed)]
        zero = {"int": "0", "bool": "false", "ptr_int": "null"}[t]
        if rest == "zero":
            rows.append(cls.q(b, len(listed), n, f"loc({b}, k).v_{t} == {zero}"))
        elif rest == "unknown":
            if t == "ptr_int":
                v = f"loc({b}, k).v_{t}"
                body = f"(({v} == null) || ((({v} == loc(block_of({v}), offset_of({v}))) && (0 <= offset_of({v}))) && (offset_of({v}) <= len(block_of({v})))))"
            else:
                body = cls.INT_RANGE.format(c=f"loc({b}, k)")
            rows.append(cls.q(b, 0, n, body))
        if lv:
            rows.append(f"acc(lvloc({b}).lv)")
        return rows

    @staticmethod
    def facts(n=4, g="GB_g__1", h="GB_h__2"):
        return " && ".join(f"((len({x}()) == {c}) && (lvloc({x}()) == loc({x}(), (-2))))" for x, c in ((g, n), (h, 1)))

    def fixture(self, rows=None, facts=None, f_requires=None, names=None, **kw):
        rows = [*self.group(), self.H_ROW] if rows is None else rows
        facts = self.facts() if facts is None else facts
        lines = self.G.fixture(names=self.NAMES if names is None else names, facts=facts, rows=rows,
                               f_requires=facts if f_requires is None else f_requires, **kw)
        lines[0:0] = ["field v_bool: Bool", "field d_bool: Bool"]
        return lines

    strict = staticmethod(GlobalsTests.strict)

    def some(self, lines, fragment):
        probs = self.strict(lines)
        self.assertTrue(any(fragment in p for p in probs), probs)

    GROUP = "the row group of an array global in `program_start`"

    def test_the_emitters_group_is_clean(self):
        self.assertEqual(self.strict(self.fixture()), [])

    def test_each_form_of_the_values_is_clean(self):
        forms = {
            "every element listed, no rest": self.group(listed=("1", "2", "3", "4"), rest=None),
            "nothing listed, the zero rest from 0": self.group(listed=(), rest="zero"),
            "an unknown value's range": self.group(listed=(), rest="unknown"),
            "a negative literal": self.group(listed=("(-5)",), rest="zero"),
            "a bool array of unknown value states no value": self.group(t="bool", listed=(), rest=None),
            "a bool array's listed elements and zero rest": self.group(t="bool", listed=("true",), rest="zero"),
            "a pointer array: null and the address of a global": self.group(t="ptr_int", listed=("null", "loc(GB_h__2(), 0)"), rest="zero"),
            "a pointer array of unknown value (WF)": self.group(t="ptr_int", listed=(), rest="unknown"),
        }
        for what, rows in forms.items():
            with self.subTest(what):
                self.assertEqual(self.strict(self.fixture(rows=[*rows, self.H_ROW])), [])

    def test_an_array_of_one_element_is_clean(self):
        self.assertEqual(self.strict(self.fixture(rows=[*self.group(n=1, listed=("7",), rest=None), self.H_ROW], facts=self.facts(1))), [])

    def test_two_array_globals_and_a_scalar_are_clean(self):
        rows = [*self.group(), *self.group(g="GB_h__2", n=3, listed=("5",), rest="zero")]
        facts = " && ".join(f"((len({x}()) == {c}) && (lvloc({x}()) == loc({x}(), (-2))))" for x, c in (("GB_g__1", 4), ("GB_h__2", 3)))
        self.assertEqual(self.strict(self.fixture(rows=rows, facts=facts)), [])

    def test_the_facts_state_the_count(self):
        for what, facts in {"len 1 for an array global of 4": self.facts(1), "len 5 for an array global of 4": self.facts(5),
                            "the length of the scalar changed": self.facts().replace("(len(GB_h__2()) == 1)", "(len(GB_h__2()) == 2)")}.items():
            with self.subTest(what):
                self.some(self.fixture(facts=facts, f_requires=facts), "its first `ensures` row must be the global facts")

    def test_the_first_requires_of_every_method_has_the_same_count(self):
        # the second of the three places the facts are pinned: a method that takes the facts of a scalar `g` is refused
        self.some(self.fixture(f_requires=self.facts(1)), "must take the global facts as its first `requires`")
        self.assertEqual(self.strict(self.fixture(f_requires=self.facts(4))), [])

    def test_a_group_that_breaks_the_order_or_the_ranges_is_refused(self):
        g = self.group()
        b = "GB_g__1()"
        bad = {
            "the flags before the flag permission": [g[0], g[2], g[1], *g[3:]],
            "the flag permission before the value permission": [g[1], g[0], *g[2:]],
            "the flags missing": [g[0], g[1], *g[3:]],
            "the flags row without the flag": [g[0], g[1], self.q(b, 0, 4, "loc(GB_g__1(), k).v_int == 0"), *g[3:]],
            "a permission over [1, N)": [self.q(b, 1, 4, f"acc(loc({b}, k).v_int)"), *g[1:]],
            "a permission over [0, N + 1)": [self.q(b, 0, 5, f"acc(loc({b}, k).v_int)"), *g[1:]],
            "the flags over another count": [g[0], g[1], self.q(b, 0, 3, f"loc({b}, k).d_int"), *g[3:]],
            "a computed bound": [g[0].replace("(k < 4)", "(k < (2 + 2))"), *g[1:]],
            "the liveness cell missing": self.group(lv=False),
            "the liveness cell twice": [*g, g[-1]],
            "a block that is not a declared global": [r.replace("GB_g__1", "GB_x__9") for r in g],
            "a flag permission of another field": [g[0], g[1].replace("d_int", "d_bool"), *g[2:]],
            "a value permission of an undeclared field": [r.replace("v_int", "v_long") if i == 0 else r for i, r in enumerate(g)],
            "a value permission under an extra antecedent": [g[0].replace("(k < 4))", "(k < 4)) && (k != 2)"), *g[1:]],
        }
        for what, rows in bad.items():
            with self.subTest(what):
                probs = self.strict(self.fixture(rows=[*rows, self.H_ROW]))
                self.assertTrue(any(self.GROUP in p or "`program_start` row" in p for p in probs), (what, probs))

    def test_a_duplicate_group_is_refused(self):
        self.some(self.fixture(rows=[*self.group(), *self.group(listed=("9",)), self.H_ROW]), "a second `program_start` row for `GB_g__1`")

    def test_a_group_and_a_scalar_row_for_one_global_are_refused(self):
        rows = [*self.group(g="GB_h__2"), self.H_ROW, *self.group()]
        self.some(self.fixture(rows=rows), "a second `program_start` row for `GB_h__2`")

    def test_the_values_are_pinned(self):
        g = self.group()  # listed 1, 2, zero rest from 2
        b = "GB_g__1()"
        bad = {
            "a missing zero rest": g[:5] + g[6:],
            "a zero rest that starts early": g[:5] + [self.q(b, 1, 4, f"loc({b}, k).v_int == 0"), g[-1]],
            "a zero rest that starts late (a slot without a value)": g[:5] + [self.q(b, 3, 4, f"loc({b}, k).v_int == 0"), g[-1]],
            "a zero rest of another value": g[:5] + [self.q(b, 2, 4, f"loc({b}, k).v_int == 1"), g[-1]],
            "a zero rest of the wrong type's zero": [r for r in self.group(t="bool", listed=("true",), rest=None)][:4] + [self.q(b, 1, 4, f"loc({b}, k).v_bool == true"), g[-1]],
            "a value conjunct at the listed prefix's end": g[:5] + ["loc(GB_g__1(), 2).v_int == 3", g[5], g[-1]],
            "a value conjunct past the listed prefix": g[:5] + ["loc(GB_g__1(), 3).v_int == 3", g[5], g[-1]],
            "a listed element twice": g[:4] + ["loc(GB_g__1(), 1).v_int == 2", *g[4:]],
            "a listed element out of order": [*g[:3], "loc(GB_g__1(), 1).v_int == 2", "loc(GB_g__1(), 0).v_int == 1", *g[5:]],
            "a conjunct after the zero rest": g[:6] + ["loc(GB_g__1(), 3).v_int == 5", g[-1]],
            "a second quantifier after the zero rest": g[:6] + [self.q(b, 2, 4, f"loc({b}, k).v_int == 0"), g[-1]],
            "a listed element that is not a literal": g[:3] + ["loc(GB_g__1(), 0).v_int == loc(GB_g__1(), 1).v_int", *g[4:]],
            "a listed element that is a range": g[:3] + ["((0 <= loc(GB_g__1(), 0).v_int) && (loc(GB_g__1(), 0).v_int <= 9))", *g[4:]],
            "a listed element over the count": self.group(n=2, listed=("1", "2", "3"), rest=None),
            "an unknown range beside a listed element": self.group(listed=("1",), rest=None)[:4] + [self.q(b, 0, 4, self.INT_RANGE.format(c=f"loc({b}, k)")), g[-1]],
            "an unknown range over [1, N)": self.group(listed=(), rest=None)[:3] + [self.q(b, 1, 4, self.INT_RANGE.format(c=f"loc({b}, k)")), g[-1]],
            "an unlisted int array with no rest": self.group(listed=(), rest=None),
            "a pointer array whose rest is the address of a global": self.group(t="ptr_int", listed=(), rest=None)[:3]
                + [self.q(b, 0, 4, f"loc({b}, k).v_ptr_int == loc(GB_h__2(), 0)"), g[-1]],
            "a listed pointer that points at an undeclared global": self.group(t="ptr_int", listed=("loc(GB_x__9(), 0)",), rest="zero"),
        }
        for what, rows in bad.items():
            with self.subTest(what):
                lines = self.fixture(rows=[*rows, self.H_ROW], facts=self.facts(2) if what == "a listed element over the count" else None)
                probs = self.strict(lines)
                self.assertTrue(any(self.GROUP in p or "points at" in p for p in probs), (what, probs))

    def test_a_pointer_element_must_point_at_a_global_of_the_pointee_type(self):
        rows = self.group(t="ptr_int", listed=("loc(GB_g__1(), 0)",), rest="zero")  # g itself: its elements are pointers, not ints
        self.some(self.fixture(rows=[*rows, self.H_ROW]), "points at the array `GB_g__1`")

    def test_an_array_global_may_be_pointed_at_through_its_first_element(self):
        rows = [*self.group(g="GB_g__1", n=4, listed=("1",), rest="zero"), *self.group(g="GB_h__2", n=2, t="ptr_int", listed=("loc(GB_g__1(), 0)", "null"), rest=None)]
        facts = " && ".join(f"((len({x}()) == {c}) && (lvloc({x}()) == loc({x}(), (-2))))" for x, c in (("GB_g__1", 4), ("GB_h__2", 2)))
        self.assertEqual(self.strict(self.fixture(rows=rows, facts=facts)), [])

    def test_a_view_in_a_program_with_an_array_global_takes_the_same_count(self):
        # the third place the facts are pinned: the leading `requires` of a typed view (decision 0047 item 4, 0041 item 4), over the emitted view of
        # scripts/testdata/heap_views/malloc_struct_global.vpr whose first global `x` is made an array global of 3 elements
        text = HeapViewTests("test_every_emitted_view_is_clean").text("malloc_struct_global")
        row = next(l for l in text.split("\n") if l.startswith("  ensures (((acc(loc(GB_x__0(), 0).v_int)"))
        group = self.group(g="GB_x__0", n=3, listed=(), rest="zero")
        text3 = text.replace(row, "\n".join("  ensures " + r for r in group)).replace("(len(GB_x__0()) == 1)", "(len(GB_x__0()) == 3)")
        self.assertEqual(HeapViewTests.problems(text3), [])
        probs = []
        src = lv.split_source(text3)
        decls = lv.declarations(src)
        has_globals, facts = lv.lint_globals(src, decls, lambda n, why: probs.append(why))
        self.assertEqual(probs, [])
        self.assertIn("len(GB_x__0())==3", facts)
        # the view still has the scalar's count: refused
        stale = text3.replace("  requires ((len(GB_x__0()) == 3)", "  requires ((len(GB_x__0()) == 1)")
        self.assertEqual(len(HeapViewTests.problems(stale)), 1)
        self.assertIn("global facts of the program", HeapViewTests.problems(stale)[0])

    def test_an_array_global_s_group_that_names_a_member_block_without_its_structure_is_refused(self):
        # a group over `member(<chain>, k)` needs the structure global's own cell row first (StructGlobalArrayLeafTests): with none it is refused
        b = "member(loc(GB_g__1(), 0), 1)"
        rows = [*self.group(), *[r.replace("GB_h__2()", b) for r in self.group(g="GB_h__2")]]
        self.assertTrue(self.strict(self.fixture(rows=rows)))

    # --- the flag of an element goes through a Ref temporary (lint_flags is unchanged) ---

    def test_an_element_flag_is_written_through_a_ref_temporary(self):
        start = Check("  program_start()", PRE, "internal")
        ok = ["  var t: Ref", start, "  t := loc(GB_g__1(), 1)", "  t.v_int := 5", "  t.d_int := true"]
        self.assertEqual(self.strict(self.fixture(main_lines=ok)), [])
        direct = ["  var t: Ref", start, "  loc(GB_g__1(), 1).v_int := 5", "  loc(GB_g__1(), 1).d_int := true"]
        self.some(self.fixture(main_lines=direct), "the flag of an array element is written through a `Ref` temporary")
        # a flag set without the write of its value, through the temporary, is the flag rule's as before
        orphan = ["  var t: Ref", start, "  t := loc(GB_g__1(), 1)", "  t.d_int := true"]
        self.some(self.fixture(main_lines=orphan), "does not directly follow a write `t.v_int := ...`")


class ArrayStructAllocationTests(unittest.TestCase):
    """Decision 0063 items 6 and 15: `alloc_struct_<n>_<tag>` of a structure with an array member states, for the member block, exactly `len(member(X,
    k)) == N` and then three quantifiers (the value permission, the flag permission, every flag false), each over the block `member(X, k)` with the
    trigger `{ loc(member(X, k), j) }` and the range `0 <= j && j < N`, each (receiver, field) once.  The shape is s47_a_struct_local.vpr's and
    s47_e_nested.vpr's; each twin breaks one thing."""

    M = MemoryModelTests("test_the_emitters_struct_allocations_are_clean")

    ROOT = "ensures ((r != null) && (r == loc(block_of(r), 0))) && (len(block_of(r)) == 1)"
    CELL0 = "ensures (((len(member(r, 0)) == 1) && acc(loc(member(r, 0), 0).v_int)) && acc(loc(member(r, 0), 0).d_int)) && (!loc(member(r, 0), 0).d_int)"
    LIVE = "ensures (lvloc(block_of(r)) == loc(block_of(r), (-2))) && acc(lvloc(block_of(r)).lv)"

    @staticmethod
    def arr(x="r", k=1, n=4, t="int"):
        b = f"member({x}, {k})"
        q = lambda body: f"ensures forall j: Int :: {{ loc({b}, j) }} ((0 <= j) && (j < {n})) ==> {body}"  # noqa: E731
        return [f"ensures len({b}) == {n}", q(f"acc(loc({b}, j).v_{t})"), q(f"acc(loc({b}, j).d_{t})"), q(f"(!loc({b}, j).d_{t})")]

    def alloc(self, ensures, name="alloc_struct_0_S"):
        return self.M.with_struct_alloc(ensures, name=name)

    def problems(self, ensures, name="alloc_struct_0_S"):
        return self.M.strict(self.alloc(ensures, name))

    def test_a_structure_with_an_array_member_is_clean(self):
        # struct S { int n; int a[4]; }
        self.assertEqual(self.problems([self.ROOT, self.CELL0, *self.arr(), self.LIVE]), [])
        # struct S { int a[4]; int n; }: the array member first
        cell1 = self.CELL0.replace("member(r, 0)", "member(r, 1)")
        self.assertEqual(self.problems([self.ROOT, *self.arr(k=0), cell1, self.LIVE]), [])
        # two array members, one of them of one element
        self.assertEqual(self.problems([self.ROOT, *self.arr(k=0), *self.arr(k=1, n=1), self.LIVE]), [])

    def test_a_nested_structure_with_an_array_member_is_clean(self):
        # struct T { struct S { int n; int a[4]; } s; int z; }: the slot of s is loc(member(r, 0), 0)
        x = "loc(member(r, 0), 0)"
        cell_n = f"ensures (((len(member({x}, 0)) == 1) && acc(loc(member({x}, 0), 0).v_int)) && acc(loc(member({x}, 0), 0).d_int)) && (!loc(member({x}, 0), 0).d_int)"
        z = "ensures (((len(member(r, 1)) == 1) && acc(loc(member(r, 1), 0).v_int)) && acc(loc(member(r, 1), 0).d_int)) && (!loc(member(r, 1), 0).d_int)"
        good = [self.ROOT, "ensures len(member(r, 0)) == 1", "ensures r != loc(member(r, 0), 0)", cell_n, *self.arr(x=x, k=1), z, self.LIVE]
        self.assertEqual(self.problems(good, name="alloc_struct_1_T"), [])
        # the array member of a nested slot whose struct member block is not stated
        bad = [self.ROOT, cell_n, *self.arr(x=x, k=1), z, self.LIVE]
        self.assertTrue(any("is not the trusted struct allocation" in p for p in self.problems(bad, name="alloc_struct_1_T")))

    def test_each_departure_from_the_array_member_rows_is_refused(self):
        base = self.arr()
        len_, v, d, nd = base
        bad = {
            "the length without its quantifiers": [len_],
            "the length with two of the three quantifiers": [len_, v, d],
            "the quantifiers without the length": [v, d, nd],
            "the quantifiers in another order": [len_, d, v, nd],
            "no `!flag`": [len_, v, d, d],
            "the value permission twice": [len_, v, v, nd],
            "a fourth quantifier (a second set)": [*base, v],
            "a length of 0": [*self.arr(n=0)],
            "a quantifier over another count": [len_, v.replace("(j < 4)", "(j < 3)"), d, nd],
            "a quantifier over [1, N)": [len_, v.replace("(0 <= j)", "(1 <= j)"), d, nd],
            "a computed bound": [len_, v.replace("(j < 4)", "(j < (2 + 2))"), d, nd],
            "a trigger over another block": [len_, v.replace("{ loc(member(r, 1), j) }", "{ loc(member(r, 0), j) }"), d, nd],
            "a quantifier over another member": [len_, v.replace("member(r, 1)", "member(r, 0)"), d, nd],
            "a quantifier over a block of the root": [len_, v.replace("loc(member(r, 1), j)", "loc(block_of(r), j)"), d, nd],
            "an unrooted chain": [len_.replace("member(r, 1)", "member(q, 1)"), v.replace("member(r, 1)", "member(q, 1)"), d.replace("member(r, 1)", "member(q, 1)"), nd.replace("member(r, 1)", "member(q, 1)")],
            "a flag of another suffix": [len_, v, d.replace("d_int", "d_bool"), nd],
            "a value and a flag of two suffixes": [len_, v, d.replace("_int", "_long"), nd.replace("_int", "_long")],
            "an undeclared suffix": [x.replace("_int", "_long") if i else x for i, x in enumerate(base)],
            "the flag not negated": [len_, v, d, nd.replace("(!loc(member(r, 1), j).d_int)", "loc(member(r, 1), j).d_int")],
            "an extra conjunct": [len_, v.replace("acc(loc(member(r, 1), j).v_int)", "acc(loc(member(r, 1), j).v_int) && loc(member(r, 1), j).v_int == 0"), d, nd],
            "a permission amount": [len_, v.replace("v_int)", "v_int, 1 / 2)"), d, nd],
            "an antecedent beyond the range": [len_, v.replace("(j < 4))", "(j < 4)) && (j != 2)"), d, nd],
            "the length twice": [len_, len_, v, d, nd],
        }
        for what, rows in bad.items():
            with self.subTest(what):
                probs = self.problems([self.ROOT, self.CELL0, *rows, self.LIVE])
                self.assertEqual(len(probs), 1, (what, probs))
                self.assertIn("`method alloc_struct_0_S` is not the trusted struct allocation", probs[0])

    def test_an_array_member_inside_a_struct_slot_that_is_not_stated_is_refused(self):
        x = "loc(member(r, 0), 0)"
        probs = self.problems([self.ROOT, *self.arr(x=x, k=0), self.LIVE])
        self.assertTrue(any("is not inside a nested struct member" in p for p in probs), probs)

    def test_a_length_other_than_1_without_quantifiers_stays_refused(self):
        # the older rule: a cell member's `len == 2` (the quantifier-free `len(member(r, 1)) == 4` is read as an array member and needs its rows)
        probs = self.problems([self.ROOT, self.CELL0, "ensures len(member(r, 1)) == 4", self.LIVE])
        self.assertTrue(any("is not followed by its three quantified permissions" in p for p in probs), probs)

    # --- struct_lens: the WF_S conjunct of an unknown pointer's row states the count of the allocation ---

    G = GlobalsTests("test_a_struct_pointer_row_with_wf_s_is_clean")
    V = GlobalsTests.V

    def pointer_fixture(self, lens, ensures=None, name="alloc_struct_0_S"):
        lines = self.G.struct_fixture(lens)
        if ensures is not None:
            at = next(i for i, line in enumerate(lines) if isinstance(line, str) and line.startswith("// cerastes:trusted-contract program start"))
            lines[at:at] = ["// cerastes:trusted-contract allocation of a local of a struct type", f"method {name}() returns (r: Ref)",
                            *[Check("  " + t, P, "internal") for t in ensures], "  decreases _"]
        return lines

    def test_the_wf_s_of_a_pointer_to_a_structure_with_an_array_member_states_the_count(self):
        V = self.V
        alloc = [self.ROOT, self.CELL0, *self.arr(), self.LIVE]
        good = f"(len(member({V}, 0)) == 1) && (len(member({V}, 1)) == 4)"
        self.assertEqual(self.G.strict(self.pointer_fixture(good, alloc)), [])
        bad = {
            "the count 1 for the array member": f"(len(member({V}, 0)) == 1) && (len(member({V}, 1)) == 1)",
            "another count": f"(len(member({V}, 0)) == 1) && (len(member({V}, 1)) == 5)",
            "the count 4 for a cell member": f"(len(member({V}, 0)) == 4) && (len(member({V}, 1)) == 4)",
        }
        for what, lens in bad.items():
            with self.subTest(what):
                self.G.some(self.pointer_fixture(lens, alloc), "the value of the `program_start` row of `GB_gp__1`")
        # no allocation of that structure in the unit: a count other than 1 has no count to equal
        self.G.some(self.pointer_fixture(good), "the value of the `program_start` row of `GB_gp__1`")
        # an allocation of a structure whose member is a cell: the array count is refused there
        cells = [self.ROOT, self.CELL0, self.CELL0.replace("member(r, 0)", "member(r, 1)"), self.LIVE]
        self.G.some(self.pointer_fixture(good, cells), "the value of the `program_start` row of `GB_gp__1`")


class BlockTermRowTests(unittest.TestCase):
    """Decision 0063 item 15's block grammar: where a line has no Viper local (a method's `requires` and `ensures`), a quantified row of an array atom is
    `forall k: Int :: { loc(B, k) } 0 <= k && k < N ==> C` with B `GB_<g>()` of a declared array global or `member(<chain>, k)` over the canonical chain rooted
    at a formal, a result or `loc(GB_<g>(), 0)`, the bounds `0` and the count the facts state, and C one of the atom's consequents.  Nothing else.  The
    shapes are s47_b_calls.vpr's (a global) and s47_c_owns_member.vpr's (a member through a pointer)."""

    A = ArrayGlobalRowTests
    INT = "(((-2147483648) <= loc({b}, k).v_int) && (loc({b}, k).v_int <= 2147483647))"
    WF_P = "(p == null) || (((p == loc(block_of(p), offset_of(p))) && (0 <= offset_of(p))) && (offset_of(p) <= len(block_of(p))))"

    def rows(self, b, n, kw, amount="amt_0", extra=()):
        q = self.A.q
        return [Check(f"  {kw} " + t, WF if kw == "requires" else P, "internal") for t in (
            q(b, 0, n, f"acc(loc({b}, k).v_int, {amount})" if amount else f"acc(loc({b}, k).v_int)"),
            q(b, 0, n, f"acc(loc({b}, k).d_int, {amount})" if amount else f"acc(loc({b}, k).d_int)"),
            q(b, 0, n, f"loc({b}, k).d_int"),
            q(b, 0, n, self.INT.format(b=b)), *extra)]

    def global_method(self, rows=None, facts=None):
        facts = self.A.facts() if facts is None else facts
        rows = rows if rows is not None else [*self.rows("GB_g__1()", 4, "requires"), *self.rows("GB_g__1()", 4, "ensures")]
        req = [r for r in rows if r.text.lstrip().startswith("requires")]
        ens = [r for r in rows if not r.text.lstrip().startswith("requires")]
        return ["method f_arr__3(amt_0: Perm)", Check("  requires " + facts, WF, "internal"), Check("  requires (none < amt_0) && (amt_0 < write)", WF, "internal"),
                *req, Check("  requires acc(lvloc(GB_g__1()).lv, amt_0)", WF, "internal"), *ens,
                Check("  ensures acc(lvloc(GB_g__1()).lv, amt_0)", P, "internal"), "  decreases", "{", "}"]

    def fixture(self, method=None, facts=None):
        lines = ArrayGlobalRowTests("test_the_emitters_group_is_clean").fixture(facts=facts)
        return lines + (method if method is not None else self.global_method(facts=facts))

    def strict(self, lines):
        return GlobalsTests.strict(lines)

    def some(self, lines, fragment):
        probs = self.strict(lines)
        self.assertTrue(any(fragment in p for p in probs), probs)

    def test_the_rows_of_a_global_array_atom_are_clean(self):
        self.assertEqual(self.strict(self.fixture()), [])

    def test_the_member_rows_over_a_formal_are_clean(self):
        lens = Check("  requires (len(member(p, 0)) == 1) && (len(member(p, 1)) == 4)", WF, "internal")
        method = ["method f_mem__3(p: Ref, amt_0: Perm)", Check("  requires " + self.A.facts(), WF, "internal"),
                  Check("  requires (none < amt_0) && (amt_0 < write)", WF, "internal"), Check("  requires p != null", WF, "internal"), lens,
                  *self.rows("member(p, 1)", 4, "requires"), Check("  requires acc(lvloc(block_of(p)).lv, amt_0)", WF, "internal"),
                  *self.rows("member(p, 1)", 4, "ensures"), Check("  ensures acc(lvloc(block_of(p)).lv, amt_0)", P, "internal"), "  decreases", "{", "}"]
        self.assertEqual(self.strict(self.fixture(method)), [])
        # a nested slot's array member
        x = "loc(member(p, 0), 0)"
        lens2 = Check("  requires (len(member(p, 0)) == 1) && (len(member(loc(member(p, 0), 0), 1)) == 4)", WF, "internal")
        method2 = [*method[:4], lens2, *self.rows(f"member({x}, 1)", 4, "requires"),
                   Check("  requires acc(lvloc(block_of(p)).lv, amt_0)", WF, "internal"), *self.rows(f"member({x}, 1)", 4, "ensures"),
                   Check("  ensures acc(lvloc(block_of(p)).lv, amt_0)", P, "internal"), "  decreases", "{", "}"]
        self.assertEqual(self.strict(self.fixture(method2)), [])

    def member_method(self, b="member(p, 1)", n=4, lens="(len(member(p, 0)) == 1) && (len(member(p, 1)) == 4)", rows=None, header="method f_mem__3(p: Ref, amt_0: Perm)"):
        rows = rows if rows is not None else self.rows(b, n, "requires")
        return [header, Check("  requires " + self.A.facts(), WF, "internal"), Check("  requires (none < amt_0) && (amt_0 < write)", WF, "internal"),
                Check("  requires p != null", WF, "internal"), *([Check("  requires " + lens, WF, "internal")] if lens else []), *rows,
                Check("  requires acc(lvloc(block_of(p)).lv, amt_0)", WF, "internal"), "  decreases", "{", "}"]

    def test_the_count_is_read_from_a_guarded_length_fact_after_another_len(self):
        # what lowering writes for `owns(p->a)` (Part C, j4's struct_array_owns_array.c): `WF_S`'s conjunct and a guarded fact, each with an earlier
        # `len(block_of(p))` in the same clause; the count of the member's rows is the one the guarded `len(member(p, 1)) == 4` states
        guard = "((p != null) && (offset_of(p) < len(block_of(p)))) ==> "
        for lens in (guard + "((len(member(p, 0)) == 1) && (len(member(p, 1)) == 4))", guard + "(len(member(p, 1)) == 4)"):
            with self.subTest(lens=lens):
                self.assertEqual(self.strict(self.fixture(self.member_method(lens=lens))), [])
        # twins: a count the fact does not state, and a fact for another member
        for what, (lens, n) in {"a count of 5": (guard + "(len(member(p, 1)) == 4)", 5), "the fact of another member": (guard + "(len(member(p, 0)) == 4)", 4)}.items():
            with self.subTest(what):
                probs = self.strict(self.fixture(self.member_method(lens=lens, rows=self.rows("member(p, 1)", n, "requires"))))
                self.assertTrue(any("the method states no `len(member(p,1)) ==" in p for p in probs), probs)

    def ptr_rows(self, b, n, kw, wf=None, field="ptr_int"):
        """The rows of an array member of POINTER elements (audit round 4 S1): `acc` of the value and the flag, the flag, and the range of the value, which
        is `WF` of the pointer (null, or its own block with 0 <= offset <= len)."""
        q = self.A.q
        v = f"loc({b}, k).v_{field}"
        wf = wf or f"(({v} == null) || ((({v} == loc(block_of({v}), offset_of({v}))) && (0 <= offset_of({v}))) && (offset_of({v}) <= len(block_of({v})))))"
        return [Check(f"  {kw} " + x, WF if kw == "requires" else P, "internal") for x in (
            q(b, 0, n, f"acc(loc({b}, k).v_{field}, amt_0)"), q(b, 0, n, f"acc(loc({b}, k).d_{field}, amt_0)"), q(b, 0, n, f"loc({b}, k).d_{field}"),
            q(b, 0, n, wf))]

    def ptr_fixture(self, rows):
        lines = self.fixture(self.member_method(rows=rows))
        lines[0:0] = ["field v_ptr_int: Ref", "field d_ptr_int: Bool"]
        return lines

    def test_the_range_of_a_pointer_element_in_a_member_row_is_wf_of_the_pointer(self):
        # `struct S { int *a[4]; int n; }` under owns(*p): the range consequent of the member's rows is the pointer's WF, the same text as a view's
        self.assertEqual(self.strict(self.ptr_fixture(self.ptr_rows("member(p, 1)", 4, "requires"))), [])
        b = "member(p, 1)"
        v = f"loc({b}, k).v_ptr_int"
        blk = f"loc(block_of({v}), offset_of({v}))"
        bad = {
            "no `null` disjunct": f"((({v} == {blk}) && (0 <= offset_of({v}))) && (offset_of({v}) <= len(block_of({v}))))",
            "no lower bound on the offset": f"(({v} == null) || (({v} == {blk}) && (offset_of({v}) <= len(block_of({v})))))",
            "no upper bound on the offset": f"(({v} == null) || (({v} == {blk}) && (0 <= offset_of({v}))))",
            "the offset past the length by one": f"(({v} == null) || ((({v} == {blk}) && (0 <= offset_of({v}))) && (offset_of({v}) <= (len(block_of({v})) + 1))))",
            "another pointer than the element": f"(({v} == null) || ((({v} == {blk}) && (0 <= offset_of({v}))) && (offset_of({v}) <= len(block_of(loc({b}, 0).v_ptr_int)))))",
            "an integer range": "(((-2147483648) <= " + v + ") && (" + v + " <= 2147483647))",
            "just `true`": "true",
            "a disjunct that is always true": f"(({v} == null) || true)",
        }
        for what, wf in bad.items():
            with self.subTest(what):
                self.some(self.ptr_fixture(self.ptr_rows(b, 4, "requires", wf=wf)), "the consequent is not `acc(F.f)`")

    def test_a_row_that_leaves_the_grammar_is_refused(self):
        q = self.A.q
        b = "GB_g__1()"
        ok = self.rows(b, 4, "requires")
        mem = lambda **kw: self.member_method(**kw)  # noqa: E731
        count, scalar, neither = "is not the `len(GB_g__1()) == 4` of the facts", "is not an array global", "is neither `GB_g()` of a declared global nor `member("
        generic, perm_formal, consequent, states = "their pure `assert` rows)", "the amount of an `acc` is a `Perm` formal", "the consequent is not `acc(F.f)`", "the method states no `len("
        bad_global = {
            "another count": (self.rows(b, 3, "requires"), count),
            "a count that is the scalar's": (self.rows(b, 1, "requires"), count),
            "a block of a scalar global": (self.rows("GB_h__2()", 1, "requires"), scalar),
            "a block of an undeclared global": (self.rows("GB_x__9()", 4, "requires"), neither),
            "a block through block_of": (self.rows("block_of(loc(GB_g__1(), 0))", 4, "requires"), neither),
            "a range from 1": ([Check("  requires " + q(b, 1, 4, f"acc(loc({b}, k).v_int, amt_0)"), WF, "internal"), *ok[1:]], "the range starts at 1"),
            "a computed upper bound": ([Check("  requires " + q(b, 0, "(2 + 2)", f"acc(loc({b}, k).v_int, amt_0)"), WF, "internal"), *ok[1:]], generic),
            "an amount that is not a Perm formal": (self.rows(b, 4, "requires", amount="1 / 2"), perm_formal),
            "an amount that is a local": (self.rows(b, 4, "requires", amount="zz"), perm_formal),
            "an extra antecedent": ([Check("  requires " + q(b, 0, 4, f"acc(loc({b}, k).v_int, amt_0)").replace("(k < 4))", "(k < 4)) && (k != 2)"), WF, "internal"), *ok[1:]], generic),
            "a consequent that is not an atom's": ([Check("  requires " + q(b, 0, 4, f"loc({b}, k).v_int == 0"), WF, "internal"), *ok[1:]], consequent),
            "a consequent with two permissions": ([Check("  requires " + q(b, 0, 4, f"acc(loc({b}, k).v_int, amt_0) && acc(loc({b}, k).d_int, amt_0)"), WF, "internal"), *ok[1:]], consequent),
            "a trigger over another term": ([Check("  requires " + q(b, 0, 4, f"acc(loc({b}, k).v_int, amt_0)").replace("{ loc(GB_g__1(), k) }", "{ loc(GB_h__2(), k) }"), WF, "internal"), *ok[1:]], scalar),
            "a guard that holds a permission": ([Check("  requires acc(loc(GB_g__1(), 0).v_int) ==> " + q(b, 0, 4, f"acc(loc({b}, k).v_int, amt_0)"), WF, "internal"), *ok[1:]], "the guard holds a permission"),
        }
        for what, (rows, fragment) in bad_global.items():
            with self.subTest(what):
                self.some(self.fixture(self.global_method(rows=[*rows, *self.rows(b, 4, "ensures")])), fragment)
        bad_member = {
            "a count the method does not state": (mem(lens="(len(member(p, 0)) == 1) && (len(member(p, 1)) == 5)"), states),
            "no length fact at all": (mem(lens=None), states),
            "a chain rooted at a local": (mem(rows=self.rows("member(zz, 1)", 4, "requires")), neither),
            "a chain with block_of": (mem(rows=self.rows("member(loc(block_of(p), 0), 1)", 4, "requires")), neither),
            "a chain with parent_of": (mem(rows=self.rows("member(parent_of(member(p, 0)), 1)", 4, "requires")), neither),
            "a negative member index": (mem(rows=self.rows("member(p, -1)", 4, "requires")), neither),
            "a block of a local (the older range atoms' form: it has no count)": (mem(rows=self.rows("block_of(p)", 4, "requires")), "the range does not start at `offset_of(p)`"),
            "a member of the slot of an undeclared global": (mem(rows=self.rows("member(loc(GB_x__9(), 0), 1)", 4, "requires")), neither),
        }
        for what, (method, fragment) in bad_member.items():
            with self.subTest(what):
                self.some(self.fixture(method), fragment)

    def test_a_row_over_a_block_term_is_only_a_requires_or_an_ensures(self):
        # in a method's body the rows are over a local (`block_of(v)`): the block-term form there is refused
        b = "GB_g__1()"
        body = ["  assert " + self.A.q(b, 0, 4, f"acc(loc({b}, k).v_int)")]
        lines = self.fixture(["method f_body__3()", "  decreases", "{", *body, "}"])
        self.some(lines, "a quantified `assert` that is not a marked range row of decision 0059 item 7")

    def test_an_ensures_may_state_that_the_flags_are_kept(self):
        b = "GB_g__1()"
        keep = Check("  ensures " + self.A.q(b, 0, 4, f"old(loc({b}, k).d_int) ==> loc({b}, k).d_int"), P, "internal")
        rows = [*self.rows(b, 4, "requires"), *self.rows(b, 4, "ensures"), keep]
        self.assertEqual(self.strict(self.fixture(self.global_method(rows=rows))), [])
        # `old` in a `requires` has no pre-state
        bad = Check("  requires " + self.A.q(b, 0, 4, f"old(loc({b}, k).d_int) ==> loc({b}, k).d_int"), WF, "internal")
        self.some(self.fixture(self.global_method(rows=[bad, *self.rows(b, 4, "requires"), *self.rows(b, 4, "ensures")])), "`old` in a `requires`")


class BoundLocalRowTests(unittest.TestCase):
    """Decision 0063 item 15's bound local: a `Ref` local of a method body assigned exactly `loc(GB_<g>(), 0)` or `loc(member(<chain>, k), 0)` is e1a's array
    local, and its rows (the pure `assert` rows of a call or a scope exit) are `{ loc(block_of(t), k) } 0 <= k && k < N ==> ...` with N the count the
    block's length fact states.  The accepting cases are S47's lines (s47_a_global.vpr:65, s47_c_owns_member.vpr:114-115, s47_g_copy_locals.vpr:108's
    copy of a bound local); every twin breaks one thing: another start, a local bound to another slot, a block outside the grammar, a count that is not
    the fact's.  The block-term form in a body stays refused (the lowering writes a row in code over the local)."""

    B = BlockTermRowTests
    A = ArrayGlobalRowTests
    FLAG = "assert forall k: Int :: {{ loc(block_of({x}), k) }} (({lo} <= k) && (k < {hi})) ==> loc(block_of({x}), k).d_int"
    RANGE = ("assert forall k: Int :: {{ loc(block_of({x}), k) }} (({lo} <= k) && (k < {hi})) ==> "
             "(loc(block_of({x}), k).d_int ==> (((-2147483648) <= loc(block_of({x}), k).v_int) && (loc(block_of({x}), k).v_int <= 2147483647)))")
    ALLOC_S = ["// cerastes:trusted-contract allocation of a local of a struct type", "method alloc_struct_0_S() returns (r: Ref)",
               *[Check("  " + t, P, "internal") for t in (
                   "ensures ((r != null) && (r == loc(block_of(r), 0))) && (len(block_of(r)) == 1)",
                   "ensures (((len(member(r, 0)) == 1) && acc(loc(member(r, 0), 0).v_int)) && acc(loc(member(r, 0), 0).d_int)) && (!loc(member(r, 0), 0).d_int)",
                   *ArrayStructAllocationTests.arr(),
                   "ensures (lvloc(block_of(r)) == loc(block_of(r), (-2))) && acc(lvloc(block_of(r)).lv)")],
               "  decreases _"]

    _X = "loc(member(r, 0), 0)"
    ALLOC_T = ["// cerastes:trusted-contract allocation of a local of a struct type", "method alloc_struct_1_T() returns (r: Ref)",
               *[Check("  " + t, P, "internal") for t in (
                   ArrayStructAllocationTests.ROOT, "ensures len(member(r, 0)) == 1", "ensures r != loc(member(r, 0), 0)",
                   f"ensures (((len(member({_X}, 0)) == 1) && acc(loc(member({_X}, 0), 0).v_int)) && acc(loc(member({_X}, 0), 0).d_int)) && (!loc(member({_X}, 0), 0).d_int)",
                   *ArrayStructAllocationTests.arr(x=_X, k=1),
                   "ensures (((len(member(r, 1)) == 1) && acc(loc(member(r, 1), 0).v_int)) && acc(loc(member(r, 1), 0).d_int)) && (!loc(member(r, 1), 0).d_int)",
                   ArrayStructAllocationTests.LIVE)],
               "  decreases _"]

    def rows(self, x, lo=0, hi=4):
        return [Check("  " + t.format(x=x, lo=lo, hi=hi), A, "internal") for t in (self.FLAG, self.RANGE)]

    def with_body(self, method, body):
        assert method[-2:] == ["{", "}"], method[-4:]
        return [*method[:-2], "{", *body, "}"]

    BT = BlockTermRowTests("test_the_rows_of_a_global_array_atom_are_clean")

    def global_case(self, body):
        return self.BT.fixture(self.with_body(self.BT.global_method(), body))

    def member_case(self, body, lens="(len(member(p, 0)) == 1) && (len(member(p, 1)) == 4)", alloc=None):
        # the count of a member's bound local is the literal of the member in an `alloc_struct_*` of the unit (item 15): S { int n; int a[4]; } by default
        method = self.with_body(self.BT.member_method(lens=lens), body)
        base = self.BT.fixture(method)
        return [*base[:len(base) - len(method)], *(self.ALLOC_S if alloc is None else alloc), *method]

    def alloc_case(self, body):
        # the allocation's trusted method stands before the method that calls it
        method = ["method f_loc__3()", Check("  requires " + self.A.facts(), WF, "internal"), "  decreases", "{", *body, "}"]
        base = self.BT.fixture(method)
        return [*base[:len(base) - len(method)], *self.ALLOC_S, *method]

    @staticmethod
    def strict(lines):
        return GlobalsTests.strict(lines)

    def some(self, lines, fragment):
        probs = self.strict(lines)
        self.assertTrue(any(fragment in p for p in probs), (fragment, probs))

    def test_a_local_bound_to_a_global_array_has_the_rows_over_zero_to_the_count(self):
        # s47_a_global.vpr:65
        body = ["  var v_g: Ref", "  v_g := loc(GB_g__1(), 0)", *self.rows("v_g")]
        self.assertEqual(self.strict(self.global_case(body)), [])
        # e1a's own start, `offset_of(t)`, is still accepted for the same local
        e1a = Check("  " + self.FLAG.format(x="v_g", lo="offset_of(v_g)", hi="(offset_of(v_g) + 4)"), A, "internal")
        self.assertEqual(self.strict(self.global_case(["  var v_g: Ref", "  v_g := loc(GB_g__1(), 0)", e1a])), [])

    def test_a_local_bound_to_a_member_of_a_formal_has_the_rows_over_zero_to_the_count(self):
        # s47_c_owns_member.vpr:114-115 (there the local's block is `member(p, 1)` of the formal)
        body = ["  var v_a: Ref", "  v_a := loc(member(p, 1), 0)", *self.rows("v_a")]
        self.assertEqual(self.strict(self.member_case(body)), [])
        # a nested chain, the facts stating the nested member's length
        lens = "(len(member(p, 0)) == 1) && (len(member(loc(member(p, 0), 0), 1)) == 4)"
        body = ["  var v_a: Ref", "  v_a := loc(member(loc(member(p, 0), 0), 1), 0)", *self.rows("v_a")]
        nested = self.with_body(self.BT.member_method(lens=lens, rows=self.BT.rows("member(loc(member(p, 0), 0), 1)", 4, "requires")), body)
        base = self.BT.fixture(nested)
        # the count is the allocation's: struct T { struct S { int n; int a[4]; } s; int z; }, the array at the key (0, 1)
        self.assertEqual(self.strict([*base[:len(base) - len(nested)], *self.ALLOC_T, *nested]), [])
        self.some([*base[:len(base) - len(nested)], *self.ALLOC_S, *nested], "no `alloc_struct_*` of the unit has an array member at (0, 1)")

    def test_a_local_bound_to_a_member_of_an_allocated_structure_takes_the_count_of_the_allocation(self):
        # s47_c_owns_member.vpr:98-115, s47_g_copy_locals.vpr: `v_s := alloc_struct_0_S()`, `v_a := loc(member(v_s, 1), 0)`
        body = ["  var v_s: Ref", "  var v_a: Ref", Check("  v_s := alloc_struct_0_S()", "call.precondition", "internal"), "  v_a := loc(member(v_s, 1), 0)", *self.rows("v_a")]
        lines = self.alloc_case(body)
        self.assertEqual(self.strict(lines), [])
        bad = ["  var v_s: Ref", "  var v_a: Ref", Check("  v_s := alloc_struct_0_S()", "call.precondition", "internal"), "  v_a := loc(member(v_s, 1), 0)", *self.rows("v_a", hi=5)]
        self.some(self.alloc_case(bad), "is not the `len(...) == N` its block's facts state (4)")

    def test_a_copy_of_a_bound_local_is_not_a_bound_local(self):
        # the ruling is "assigned exactly `loc(...)`": s47_g_copy_locals.vpr:94 and :108 (`v_sa := v_a`, then rows over `v_sa`) are fixture differences,
        # refused until lowering is shown to write them (the coordinator's question: should a copy of a bound local count?)
        body = ["  var v_a: Ref", "  var v_sa: Ref", "  v_a := loc(member(p, 1), 0)", "  v_sa := v_a", *self.rows("v_sa")]
        self.some(self.member_case(body), "the range does not start at `offset_of(v_sa)`")

    def test_a_declaration_with_its_value_is_an_assignment(self):
        body = ["  var v_a: Ref := loc(member(p, 1), 0)", *self.rows("v_a")]
        self.assertEqual(self.strict(self.member_case(body)), [])
        body = ["  var v_a: Ref := loc(member(p, 1), 1)", *self.rows("v_a")]
        self.some(self.member_case(body), "is bound to slot 1")

    def test_a_multi_target_call_can_reassign_a_bound_local(self):
        # `v_a, v_b := f()` assigns v_a something that is no `loc(B, 0)`
        body = ["  var v_a: Ref", "  var v_b: Ref", "  v_a := loc(member(p, 1), 0)", "  v_a, v_b := f_two()", *self.rows("v_a")]
        self.some(self.member_case(body), "is not assigned one `loc(B, 0)`")
        body = ["  var v_a: Ref", "  var v_b: Ref", "  v_b, v_a := f_two()", "  v_a := loc(member(p, 1), 0)", *self.rows("v_a")]
        self.some(self.member_case(body), "is not assigned one `loc(B, 0)`")

    def test_a_local_that_is_an_allocation_result_and_something_else_is_no_root(self):
        # `v_s := alloc_struct_0_S()` roots the chain only when it is the local's one assignment
        ok = ["  var v_s: Ref", "  var v_a: Ref", Check("  v_s := alloc_struct_0_S()", "call.precondition", "internal"), "  v_a := loc(member(v_s, 1), 0)", *self.rows("v_a")]
        self.assertEqual(self.strict(self.alloc_case(ok)), [])
        twice = ["  var v_s: Ref", "  var v_a: Ref", Check("  v_s := alloc_struct_0_S()", "call.precondition", "internal"), "  v_s := p", "  v_a := loc(member(v_s, 1), 0)", *self.rows("v_a")]
        self.some(self.alloc_case(twice), "is neither `GB_g()` of a declared array global nor `member(<chain>, k)`")

    def test_the_rows_of_a_bound_local_are_refused_when_one_thing_is_wrong(self):
        def mem(binding, rows, lens="(len(member(p, 0)) == 1) && (len(member(p, 1)) == 4)"):
            return self.member_case(["  var v_a: Ref", f"  v_a := {binding}", *rows], lens)

        def glob(binding, rows):
            return self.global_case(["  var v_a: Ref", f"  v_a := {binding}", *rows])

        bound = "loc(member(p, 1), 0)"
        cases = {
            "another start than 0 or offset_of(t)": (mem(bound, self.rows("v_a", lo=1)), "the range does not start at `offset_of(v_a)`"),
            "a start at 0 of a local that is not bound": (mem("p", self.rows("v_a")), "the range does not start at `offset_of(v_a)`"),
            "a local bound to another slot": (mem("loc(member(p, 1), 1)", self.rows("v_a")), "is bound to slot 1 of `member(p,1)`, not the first slot"),
            "a local bound to the slot before": (mem("loc(member(p, 1), -1)", self.rows("v_a")), "is bound to slot -1"),
            "a count past the fact's": (mem(bound, self.rows("v_a", hi=5)), "is not the `len(...) == N` its block's facts state (4)"),
            "a count short of the fact's": (mem(bound, self.rows("v_a", hi=3)), "is not the `len(...) == N` its block's facts state (4)"),
            "a computed count": (mem(bound, self.rows("v_a", hi="(2 + 2)")), "is not the `len(...) == N` its block's facts state (4)"),
            "no allocation states an array at the member (the method's own fact does not count)": (mem("loc(member(p, 0), 0)", self.rows("v_a", hi=1)), "no `alloc_struct_*` of the unit has an array member at (0,)"),
            "a block outside the grammar (a member of an unknown local)": (mem("loc(member(zz, 1), 0)", self.rows("v_a")), "is neither `GB_g()` of a declared array global nor `member("),
            "a block outside the grammar (through block_of)": (mem("loc(member(loc(block_of(p), 0), 1), 0)", self.rows("v_a")), "is neither `GB_g()` of a declared array global"),
            "a block of an undeclared global": (glob("loc(GB_x__9(), 0)", self.rows("v_a")), "is neither `GB_g()` of a declared array global"),
            "a scalar global's slot as an array": (glob("loc(GB_h__2(), 0)", self.rows("v_a", hi=1)), "is not an array global"),
            "an array global's rows of another count": (glob("loc(GB_g__1(), 0)", self.rows("v_a", hi=3)), "is not the `len(...) == N` its block's facts state (4)"),
        }
        for what, (lines, fragment) in cases.items():
            with self.subTest(what):
                self.some(lines, fragment)

    def test_a_local_assigned_two_different_things_is_not_bound(self):
        body = ["  var v_a: Ref", "  v_a := loc(member(p, 1), 0)", "  v_a := p", *self.rows("v_a")]
        self.some(self.member_case(body), "is not assigned one `loc(B, 0)`")
        body = ["  var v_a: Ref", "  v_a := loc(member(p, 1), 0)", "  v_a := loc(member(p, 0), 0)", *self.rows("v_a")]
        self.some(self.member_case(body), "is not assigned one `loc(B, 0)`")

    def test_a_members_bound_local_is_rooted_at_a_viper_variable_only(self):
        # Part C's lowering (`ma_<n>`): `ma_n := loc(member(<chain>, k), 0)` with the chain rooted at a formal, `r`, a structure local, or a call-row
        # temporary bound from the argument first (assigned once); never an expression
        def mem(root_lines, root, rows=None):
            return self.member_case(["  var t: Ref", "  var ma_0: Ref", *root_lines, f"  ma_0 := loc(member({root}, 1), 0)", *(rows or self.rows("ma_0"))])

        # the temporary bound from the argument first: `t := p`, and a pointer a call returned
        self.assertEqual(self.strict(mem(["  t := p"], "t")), [])
        self.assertEqual(self.strict(mem([Check("  t := f_get(p)", "call.precondition", "internal")], "t")), [])
        # a temporary that is the address of a nested slot (`f(&s.t)`: `t := loc(member(p, 0), 0)`) is a root too, not an array
        self.assertEqual(self.strict(mem(["  t := loc(member(p, 0), 0)"], "t")), [])
        # twins, one thing each: the root assigned twice (not "bound from the argument first"), a root that is no variable of the method, an expression
        # as the root, and a chain rooted at another bound local (a first slot is no structure)
        self.some(mem(["  t := p", "  t := null"], "t"), "is neither `GB_g()` of a declared array global nor `member(<chain>, k)`")
        self.some(mem(["  t := p"], "zz"), "is neither `GB_g()` of a declared array global nor `member(<chain>, k)`")
        self.some(mem(["  t := p"], "loc(block_of(p), 0)"), "is neither `GB_g()` of a declared array global nor `member(<chain>, k)`")
        self.some(mem(["  t := p"], "null"), "is neither `GB_g()` of a declared array global nor `member(<chain>, k)`")
        other = self.member_case(["  var ma_0: Ref", "  var ma_1: Ref", "  ma_0 := loc(member(p, 1), 0)", "  ma_1 := loc(member(ma_0, 1), 0)", *self.rows("ma_1")])
        self.some(other, "is neither `GB_g()` of a declared array global nor `member(<chain>, k)`")

    def test_a_members_count_is_the_literal_of_any_allocation_with_an_array_member_there(self):
        # item 15: the count of the member at that path and index in ANY `alloc_struct_*` of a structure with an array member there
        def other(x):
            if isinstance(x, Check):
                text = x.text.replace("len(member(r, 1)) == 4", "len(member(r, 1)) == 8").replace("j < 4", "j < 8")
                return Check(text, x.error_id, x.category, x.message)
            return x.replace("alloc_struct_0_S", "alloc_struct_2_U")

        t8 = [other(x.replace("alloc_struct_0_S", "alloc_struct_2_U") if isinstance(x, str) else x) for x in self.ALLOC_S]
        both = [*self.ALLOC_S, *t8]
        body = lambda hi: ["  var ma_0: Ref", "  ma_0 := loc(member(p, 1), 0)", *self.rows("ma_0", hi=hi)]  # noqa: E731
        for n in (4, 8):
            with self.subTest(n=n):
                self.assertEqual(self.strict(self.member_case(body(n), alloc=both)), [])
        for n in (3, 5, 9):
            with self.subTest(n=n):
                self.some(self.member_case(body(n), alloc=both), "is not the `len(...) == N` its block's facts state")
        # the count is looked up by the member's path and index: another structure's array at ANOTHER index does not lend its count
        asx = ArrayStructAllocationTests
        cell1 = asx.CELL0.replace("member(r, 0)", "member(r, 1)")
        v5 = ["// cerastes:trusted-contract allocation of a local of a struct type", "method alloc_struct_3_V() returns (r: Ref)",
              *[Check("  " + t, P, "internal") for t in (asx.ROOT, *asx.arr(k=0, n=5), cell1, asx.LIVE)], "  decreases _"]
        self.assertEqual(self.strict(self.member_case(["  var ma_0: Ref", "  ma_0 := loc(member(p, 0), 0)", *self.rows("ma_0", hi=5)], alloc=[*self.ALLOC_S, *v5])), [])
        self.some(self.member_case(body(5), alloc=[*self.ALLOC_S, *v5]), "is not the `len(...) == N` its block's facts state (4)")
        # the method's own `len(member(p, 1)) == 4` is no source: with only the 8-element allocation the 4 rows are refused, and with no
        # allocation of a structure with an array member there is no count at all
        self.some(self.member_case(body(4), alloc=t8), "is not the `len(...) == N` its block's facts state (8)")
        self.some(self.member_case(body(4), alloc=[]), "no `alloc_struct_*` of the unit has an array member at (1,)")

    def test_the_block_term_form_stays_refused_in_a_body(self):
        # s47_c_owns_member.vpr:71-74 write the rows over `member(p, 1)` itself in the body: lowering writes them over the bound local
        row = self.A.q("member(p, 1)", 0, 4, "loc(member(p, 1), k).d_int")
        body = [Check("  assert " + row, A, "internal")]
        self.some(self.member_case(body), "a quantified `assert` that is not a marked range row of decision 0059 item 7")


class StructGlobalArrayLeafTests(unittest.TestCase):
    """Decision 0063 items 9 and 15: a structure global with an array member in `program_start`.  The shape is il-analyst-3's (marr_global_read_write.vpr
    of the VIR self-test): the facts row states `len(member(loc(GB_g(), 0), k)) == N` for the array member (`== 1` for a cell member), the cell row
    is the structure's cell leaves ending with `acc(lvloc(GB_g()).lv)` (decision 0044's row, unchanged), and each array member has its own group over
    `member(loc(GB_g(), 0), k)` in item 9's order WITHOUT the liveness row (the cell row has it).  A structure with no cell leaf has no place for the
    liveness and is refused until the lowering shows one; every twin breaks one thing."""

    A = ArrayGlobalRowTests
    S = StructGlobalTests
    G = "GB_g__1"
    B = "member(loc(GB_g__1(), 0), 1)"   # struct { int n; int a[4]; }: n is member 0, a member 1

    @classmethod
    def cell_row(cls, v="7"):
        return cls.S.srow(cls.G, [cls.S.int_leaf(cls.G, (0,), v)])

    @classmethod
    def group(cls, b=None, n=4, **kw):
        b = b or cls.B
        return [r for r in ArrayGlobalRowTests.group(g="GB_g__1", n=n, lv=False, **kw)] and [
            r.replace("GB_g__1()", b) for r in ArrayGlobalRowTests.group(g="GB_g__1", n=n, lv=False, **kw)]

    @classmethod
    def facts(cls, n=4, member_one=1):
        r = cls.S.slot(cls.G)
        parts = [f"((len({cls.G}()) == 1) && (lvloc({cls.G}()) == loc({cls.G}(), (-2))))", f"(len(member({r}, 0)) == {member_one})", f"(len(member({r}, 1)) == {n})",
                 "((len(GB_h__2()) == 1) && (lvloc(GB_h__2()) == loc(GB_h__2(), (-2))))"]
        return " && ".join(parts)

    def fixture(self, rows=None, facts=None):
        rows = [self.cell_row(), *self.group(), ArrayGlobalRowTests.H_ROW] if rows is None else rows
        f = self.facts() if facts is None else facts
        lines = ArrayGlobalRowTests("test_the_emitters_group_is_clean").G.fixture(names=["GB_g__1", "GB_h__2"], facts=f, rows=rows, f_requires=f)
        lines[0:0] = ["field v_bool: Bool", "field d_bool: Bool"]
        return lines

    strict = staticmethod(GlobalsTests.strict)

    def some(self, lines, fragment):
        probs = self.strict(lines)
        self.assertTrue(any(fragment in p for p in probs), (fragment, probs))

    def test_the_lowerings_shape_is_clean(self):
        self.assertEqual(self.strict(self.fixture()), [])

    def test_each_form_of_the_values_is_clean(self):
        for what, kw in {"a full list": dict(listed=("1", "2", "3", "4"), rest=None), "an unknown value": dict(listed=(), rest="unknown"),
                         "a partial list and the zero rest": dict(listed=("1",), rest="zero")}.items():
            with self.subTest(what):
                self.assertEqual(self.strict(self.fixture(rows=[self.cell_row(), *self.group(**kw), ArrayGlobalRowTests.H_ROW])), [])

    def test_two_array_members_are_two_groups(self):
        # struct { int n; int a[4]; int b[2]; }: members 1 and 2
        b2 = "member(loc(GB_g__1(), 0), 2)"
        r = self.S.slot(self.G)
        facts = self.facts() + f" && (len(member({r}, 2)) == 2)"
        rows = [self.cell_row(), *self.group(), *self.group(b2, n=2, listed=("5",)), ArrayGlobalRowTests.H_ROW]
        self.assertEqual(self.strict(self.fixture(rows=rows, facts=facts)), [])
        # the facts must state the second member's count too
        self.some(self.fixture(rows=rows), "its first `ensures` row must be the global facts")

    def test_a_group_that_breaks_the_order_is_refused(self):
        g = self.group()
        cell = self.cell_row()
        bad = {
            "the groups before the cell row": [*g, cell],
            "the liveness row in the group (the cell row has it)": [cell, *g, f"acc(lvloc({self.G}()).lv)"],
            "a duplicated group": [cell, *g, *g],
            "the flags before the flag permission": [cell, g[0], g[2], g[1], *g[3:]],
            "the value of a slot past the list's end listed twice": [cell, *g[:3], "loc(member(loc(GB_g__1(), 0), 1), 0).v_int == 1", "loc(member(loc(GB_g__1(), 0), 1), 0).v_int == 1", *g[4:]],
            "a zero rest from the wrong slot": [cell, *g[:3], g[3], g[4].replace("(2 <= k)", "(1 <= k)")],
            "no zero rest and no range for the elements past the list": [cell, *g[:5]],
            "a count that is not the facts'": [cell, *self.group(n=3)],
        }
        for what, rows in bad.items():
            with self.subTest(what):
                probs = self.strict(self.fixture(rows=[*rows, ArrayGlobalRowTests.H_ROW]))
                self.assertTrue(probs, what)

    def test_a_group_with_no_cell_row_and_no_lone_liveness_row_is_refused(self):
        self.some(self.fixture(rows=[*self.group(), ArrayGlobalRowTests.H_ROW]), "before the structure's cell row, or the structure has no cell leaf")

    LONE = "acc(lvloc(GB_g__1()).lv)"

    def only_facts(self, n=4):
        r = self.S.slot(self.G)
        return (f"((len({self.G}()) == 1) && (lvloc({self.G}()) == loc({self.G}(), (-2)))) && (len(member({r}, 0)) == {n}) && "
                "((len(GB_h__2()) == 1) && (lvloc(GB_h__2()) == loc(GB_h__2(), (-2))))")

    def only_fixture(self, rows):
        return self.fixture(rows=[*rows, ArrayGlobalRowTests.H_ROW], facts=self.only_facts())

    def only_group(self, **kw):
        return self.group(b="member(loc(GB_g__1(), 0), 0)", **kw)

    def test_an_array_only_structure_global_is_its_liveness_cell_and_the_group(self):
        # struct W { int a[4]; } w = {{3}} (marr_global_array_only.vpr): the cell row is the liveness cell alone, exactly once
        self.assertEqual(self.strict(self.only_fixture([self.LONE, *self.only_group(listed=("3",))])), [])
        self.assertEqual(self.strict(self.only_fixture([self.LONE, *self.only_group(listed=(), rest="unknown")])), [])

    def test_a_lone_liveness_row_is_refused_when_anything_else_stands_with_it(self):
        g = self.only_group(listed=("3",))
        bad = {
            "two liveness rows": [self.LONE, self.LONE, *g],
            "the group before the lone row": [*g, self.LONE],
            "an extra conjunct in the row": [f"{self.LONE} && true", *g],
            "a permission amount": ["acc(lvloc(GB_g__1()).lv, 1 / 2)", *g],
            "the liveness of another global": ["acc(lvloc(GB_h__2()).lv)", *g],
            "the liveness row after the group": [*g[:3], self.LONE, *g[3:]],
            "the lone row with no group": [self.LONE],
            "the lone row and a cell row for the same global": [self.LONE, self.cell_row(), *g],
        }
        for what, rows in bad.items():
            with self.subTest(what):
                self.assertTrue(self.strict(self.only_fixture(rows)), what)

    def test_the_facts_state_the_members_counts(self):
        for what, facts in {"the array member as a cell (len 1)": self.facts(n=1), "the array member's count 5": self.facts(n=5),
                            "the cell member's length 2": self.facts(member_one=2)}.items():
            with self.subTest(what):
                self.some(self.fixture(facts=facts), "its first `ensures` row must be the global facts")

    def test_a_group_over_a_block_that_is_no_member_of_a_declared_global_is_refused(self):
        for what, b in {"a member of an undeclared global": "member(loc(GB_x__9(), 0), 1)", "a block_of in the chain": "member(loc(block_of(loc(GB_g__1(), 0)), 0), 1)",
                        "a chain not rooted at the global's first slot": "member(GB_g__1(), 1)"}.items():
            with self.subTest(what):
                self.assertTrue(self.strict(self.fixture(rows=[self.cell_row(), *self.group(b), ArrayGlobalRowTests.H_ROW])), what)


class ArrayCopyTests(unittest.TestCase):
    """Decision 0063 item 13: `copy_arr_<t>` is trusted (the tag rule), so it is pinned clause by clause like `alloc_arr_<t>` and `zero_arr_<t>`: the shape
    of s47_g_copy_*.vpr, with the destination's value `old(source value)` and the source unchanged.  Each mutant weakens or strengthens one clause (a
    permission conjured, a value not copied, the distinctness of the blocks dropped, an amount past `write`) and must be refused; with `alloc_*` and
    `zero_arr_*` the method is exempt from the global facts and from the ghost-amount bounds of a method's formals."""

    A = ArrayAllocationTests("test_the_two_contracts_are_clean_for_each_field_type")
    HEADER = "method copy_arr_int(d: Ref, s: Ref, n: Int, rd: Perm)"

    def lines(self, clauses=None, name="copy_arr_int", header=None, t="int", ty="Int", tail="  decreases _"):
        clauses = lv.array_copy_clauses("int") if clauses is None else clauses
        header = header or f"method {name}(d: Ref, s: Ref, n: Int, rd: Perm)"
        body = [Check(f"  {kw} {text}", WF if kw == "requires" else P, "internal") for kw, text in clauses]
        return [*self.A.fields(t, ty), "// cerastes:trusted-contract array copy", header, *body, tail]

    def probs(self, lines):
        return [p for p in lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map") if "trusted array copy" in p or "ghost amount" in p or "global facts" in p]

    def test_the_contract_is_clean_for_each_field_type(self):
        for t, ty in (("int", "Int"), ("bool", "Bool"), ("ptr_int", "Ref")):
            with self.subTest(t=t):
                clauses = lv.array_copy_clauses(t)
                name = f"copy_arr_{t}"
                self.assertEqual(self.probs(self.lines(clauses, name=name, t=t, ty=ty)), [])

    def test_a_plain_spelling_of_the_same_clauses_is_clean(self):
        plain = [(kw, text.replace("((0 <= k) && (k < n))", "0 <= k && k < n")) for kw, text in lv.array_copy_clauses("int")]
        self.assertEqual(self.probs(self.lines(plain)), [])

    def test_every_clause_of_the_copy_is_pinned(self):
        base = lv.array_copy_clauses("int")
        out = {}
        for i, (kw, text) in enumerate(base):
            out[f"drop clause {i}: {text[:50]}"] = base[:i] + base[i + 1:]
        out["extra ensures false"] = base + [("ensures", "false")]
        out["extra ensures a fact"] = base + [("ensures", "len(block_of(d)) == 1")]
        out["extra requires"] = base + [("requires", "n < 100")]
        out["the blocks not distinct"] = [c for c in base if c[1] != "block_of(d) != block_of(s)"] + [("requires", "block_of(d) == block_of(s)")]
        out["the amount up to a bound past write"] = [(kw, t.replace("rd <= write", "rd <= 2 * write")) for kw, t in base]
        out["the amount without a lower bound"] = [(kw, t.replace("(none < rd) && ", "")) for kw, t in base]
        out["the amount strict below write"] = [(kw, t.replace("rd <= write", "rd < write")) for kw, t in base]
        out["the destination at an amount"] = [(kw, t.replace("acc(loc(block_of(d), k).v_int)", "acc(loc(block_of(d), k).v_int, rd)")) for kw, t in base]
        out["the source at full permission"] = [(kw, t.replace("acc(loc(block_of(s), k).v_int, rd)", "acc(loc(block_of(s), k).v_int)")) for kw, t in base]
        out["a destination value that is not the source's"] = [(kw, t.replace("== old(loc(block_of(s), k).v_int)", "== 0") if "block_of(d), k).v_int ==" in t else t) for kw, t in base]
        out["a destination value of the destination's own old value"] = [(kw, t.replace("old(loc(block_of(s), k).v_int)", "old(loc(block_of(d), k).v_int)") if "block_of(d), k).v_int ==" in t else t) for kw, t in base]
        out["a destination value stated of the source's cell (the source's value twice)"] = [(kw, t.replace("loc(block_of(d), k).v_int == old", "loc(block_of(s), k).v_int == old") if "block_of(d), k).v_int ==" in t else t) for kw, t in base]
        out["the range to n + 1"] = base[:3] + [(base[3][0], base[3][1].replace("(k < n)", "(k < (n + 1))"))] + base[4:]
        out["a trigger over the other block"] = base[:3] + [(base[3][0], base[3][1].replace("{ loc(block_of(d), k) }", "{ loc(block_of(s), k) }"))] + base[4:]
        src_flag = "loc(block_of(s), k).d_int"
        out["a source flag requirement dropped (an unwritten source element copied)"] = [c for c in base if not (c[0] == "requires" and c[1].endswith(src_flag) and "acc(" not in c[1])]
        out["the source flags not kept (the source's next read fails its `uninitialized` row)"] = [c for c in base if not (c[0] == "ensures" and c[1].endswith(src_flag) and "acc(" not in c[1])]
        out["the source flags kept as a requires only"] = [c if not (c[0] == "ensures" and c[1].endswith(src_flag) and "acc(" not in c[1]) else ("requires", c[1]) for c in base]
        out["the destination flags not set"] = [c for c in base if not (c[0] == "ensures" and c[1].endswith("loc(block_of(d), k).d_int") and "acc(" not in c[1])]
        out["the source value's ensures dropped (the copy may change the source)"] = [c for c in base if not (c[0] == "ensures" and "block_of(s), k).v_int ==" in c[1])]
        out["the source value's ensures stated of the destination"] = [(kw, t.replace("loc(block_of(s), k).v_int == old", "loc(block_of(d), k).v_int == old")) if "block_of(s), k).v_int == old" in t else (kw, t) for kw, t in base]
        for what, clauses in out.items():
            with self.subTest(what):
                probs = self.probs(self.lines(clauses))
                self.assertTrue(any("is not the trusted array copy of decision 0063 item 13" in p for p in probs), (what, probs))

    def test_the_header_the_body_and_the_tail_are_pinned(self):
        bad = {
            "a result": self.lines(header=self.HEADER + " returns (r: Ref)"),
            "swapped blocks": self.lines(header="method copy_arr_int(s: Ref, d: Ref, n: Int, rd: Perm)"),
            "no amount": self.lines(header="method copy_arr_int(d: Ref, s: Ref, n: Int)"),
            "an Int amount": self.lines(header="method copy_arr_int(d: Ref, s: Ref, n: Int, rd: Int)"),
            "a body": [*self.lines(tail="  decreases _"), "{", "}"],
            "a measure": self.lines(tail="  decreases n"),
            "an undeclared suffix": self.lines(name="copy_arr_long"),
        }
        for what, lines in bad.items():
            with self.subTest(what):
                self.assertTrue(any("trusted array copy" in p for p in self.probs(lines)), what)

    def test_the_name_is_the_emitters_and_no_wider(self):
        # `copy_arr_` followed by anything is the emitter's name; a method that merely looks like it is an ordinary method (here: bodyless, untagged)
        probs = lv.lint_vpr(*build(self.lines(name="copy_array_int")), "t.vpr", "t.vpr.map")
        self.assertFalse(any("trusted array copy" in p for p in probs), probs)
        probs = [p for p in lv.lint_vpr(*build(self.lines(name="copy_arr_int_2")), "t.vpr", "t.vpr.map") if "trusted array copy" in p]
        self.assertTrue(probs)

    def test_the_copy_is_exempt_from_the_global_facts_and_a_method_that_looks_like_it_is_not(self):
        g = GlobalsTests("test_the_emitters_shape_is_clean")
        lines = g.fixture() + self.lines()
        self.assertEqual([p for p in lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map") if "global facts" in p or "ghost amount" in p], [])
        # the same clauses under a name that is not the prefix's are a method of a program with globals: refused
        other = self.lines(name="copy_array_int")
        lines = g.fixture() + other
        self.assertTrue(any("must take the global facts" in p for p in lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")))

    # the trusted copy as S47 (g) passed on the box and as the emitter writes it, verbatim (the rule is not checked against its own generator only)
    EMITTED = """method copy_arr_int(d: Ref, s: Ref, n: Int, rd: Perm)
  requires (1 <= n) && (d == loc(block_of(d), 0)) && (len(block_of(d)) == n) && (s == loc(block_of(s), 0)) && (len(block_of(s)) == n)
  requires block_of(d) != block_of(s)
  requires (none < rd) && (rd <= write)
  requires forall k: Int :: { loc(block_of(d), k) } ((0 <= k) && (k < n)) ==> acc(loc(block_of(d), k).v_int)
  requires forall k: Int :: { loc(block_of(d), k) } ((0 <= k) && (k < n)) ==> acc(loc(block_of(d), k).d_int)
  requires forall k: Int :: { loc(block_of(s), k) } ((0 <= k) && (k < n)) ==> acc(loc(block_of(s), k).v_int, rd)
  requires forall k: Int :: { loc(block_of(s), k) } ((0 <= k) && (k < n)) ==> acc(loc(block_of(s), k).d_int, rd)
  requires forall k: Int :: { loc(block_of(s), k) } ((0 <= k) && (k < n)) ==> loc(block_of(s), k).d_int
  ensures forall k: Int :: { loc(block_of(d), k) } ((0 <= k) && (k < n)) ==> acc(loc(block_of(d), k).v_int)
  ensures forall k: Int :: { loc(block_of(d), k) } ((0 <= k) && (k < n)) ==> acc(loc(block_of(d), k).d_int)
  ensures forall k: Int :: { loc(block_of(s), k) } ((0 <= k) && (k < n)) ==> acc(loc(block_of(s), k).v_int, rd)
  ensures forall k: Int :: { loc(block_of(s), k) } ((0 <= k) && (k < n)) ==> acc(loc(block_of(s), k).d_int, rd)
  ensures forall k: Int :: { loc(block_of(d), k) } ((0 <= k) && (k < n)) ==> loc(block_of(d), k).v_int == old(loc(block_of(s), k).v_int)
  ensures forall k: Int :: { loc(block_of(d), k) } ((0 <= k) && (k < n)) ==> loc(block_of(d), k).d_int
  ensures forall k: Int :: { loc(block_of(s), k) } ((0 <= k) && (k < n)) ==> loc(block_of(s), k).v_int == old(loc(block_of(s), k).v_int)
  ensures forall k: Int :: { loc(block_of(s), k) } ((0 <= k) && (k < n)) ==> loc(block_of(s), k).d_int
  decreases _
"""

    def test_the_emitted_text_of_the_copy_is_clean(self):
        lines = [*self.A.fields("int", "Int"), "// cerastes:trusted-contract array copy", *[Check(l, WF if l.lstrip().startswith("requires") else P, "internal") if l.startswith("  ") else l for l in self.EMITTED.strip().split("\n")]]
        self.assertEqual(self.probs(lines), [])
        # the same text with the source's flags not kept, and with the destination's value row reading the source on both sides (the typo), is refused
        flags = [l for l in self.EMITTED.strip().split("\n") if not (l.strip().startswith("ensures") and l.endswith("loc(block_of(s), k).d_int"))]
        typo = [l.replace("loc(block_of(d), k).v_int == old", "loc(block_of(s), k).v_int == old") for l in self.EMITTED.strip().split("\n")]
        for what, text in (("flags not kept", flags), ("the typo", typo)):
            with self.subTest(what):
                ls = [*self.A.fields("int", "Int"), "// cerastes:trusted-contract array copy", *[Check(l, WF if l.lstrip().startswith("requires") else P, "internal") if l.startswith("  ") else l for l in text]]
                self.assertTrue(any("is not the trusted array copy of decision 0063 item 13" in p for p in self.probs(ls)), what)

    def test_the_exemptions_are_granted_by_the_clause_set_not_by_the_name(self):
        # a method named `copy_arr_int` whose clauses are not the pinned ones is no exempt method: it takes the global facts and the ghost-amount bounds like any
        # other (its `rd` has no `none < rd && rd < write`), beside the refusal as a copy
        g = GlobalsTests("test_the_emitters_shape_is_clean")
        pinned = lv.array_copy_clauses("int")
        for what, clauses in {"an extra ensures": pinned + [("ensures", "len(block_of(d)) == 1")],
                              "a clause dropped": pinned[:-1],
                              "the amount up to 2 * write": [(kw, t.replace("rd <= write", "rd <= 2 * write")) for kw, t in pinned]}.items():
            with self.subTest(what):
                probs = lv.lint_vpr(*build(g.fixture() + self.lines(clauses)), "t.vpr", "t.vpr.map")
                self.assertTrue(any("must take the global facts" in p for p in probs), (what, probs))
                self.assertTrue(any("is not the trusted array copy of decision 0063 item 13" in p for p in probs), (what, probs))
                self.assertTrue(any("ghost amount" in p or "`rd`" in p for p in probs), (what, probs))
        # the pinned clause set is exempt from both (the other test) and so is a clean program
        self.assertEqual([p for p in lv.lint_vpr(*build(g.fixture() + self.lines()), "t.vpr", "t.vpr.map") if "global facts" in p or "ghost amount" in p or "`rd`" in p], [])


class ArrayCopyCallTests(unittest.TestCase):
    """Decision 0063 item 13, Part E: a call of the trusted `copy_arr_<t>` is preceded by an explicit row for each of its `requires` (the amount `rd` as the
    minimum over the source's first slot, `none < rd`, the source's value, flag and flags rows, the destination's value and flag rows) and, immediately
    before the call, the internal row `block_of(d) != block_of(s)`; a copy that may be a self-copy sits alone in `if (d != s) {`.  Each mutant drops or
    changes one row and must be refused by this rule (the shapes are those of il-analyst-4's `earr_copy_*` outputs)."""

    A = ArrayCopyTests("test_the_contract_is_clean_for_each_field_type")

    @staticmethod
    def rows(d="t__1", s="t__2", rd="t__3", n=3, t="int"):
        def q(b, tail):
            return f"  assert forall k: Int :: {{ loc(block_of({b}), k) }} ((0 <= k) && (k < {n})) ==> {tail}"
        cs, cd = f"loc(block_of({s}), k)", f"loc(block_of({d}), k)"
        return {
            "rd": f"  {rd} := (perm({s}.v_{t}) < perm({s}.d_{t})) ? perm({s}.v_{t}) : perm({s}.d_{t})",
            "pos": f"  assert none < {rd}",
            "sv": q(s, f"acc({cs}.v_{t}, {rd})"), "sd": q(s, f"acc({cs}.d_{t}, {rd})"), "sf": q(s, f"{cs}.d_{t}"),
            "dv": q(d, f"acc({cd}.v_{t})"), "dd": q(d, f"acc({cd}.d_{t})"),
            "int": f"  assert block_of({d}) != block_of({s})",
        }

    def program(self, rows=None, guard=False, drop=(), after=(), call="  copy_arr_int(t__1, t__2, 3, t__3)", guard_text=None):
        r = dict(self.rows() if rows is None else rows)
        for k in drop:
            r.pop(k)
        order = ["rd", "pos", "sv", "sd", "sf", "dv", "dd"]
        body = [r[k] for k in order if k in r]
        tail = [r["int"]] if "int" in r else []
        tail = tail + list(after)
        call_lines = [guard_text or "  if (t__1 != t__2) {", *tail, call, "  }"] if guard else [*tail, call]
        return [*self.A.A.fields("int", "Int"), "// cerastes:trusted-contract array copy", *self.A.lines()[len(self.A.A.fields("int", "Int")) + 1:],
                "method m__1(v_a: Ref)", "  decreases", "{", "  var t__1: Ref", "  var t__2: Ref", "  var t__3: Perm", *body, *call_lines, "}"]

    def probs(self, lines):
        return [p for p in lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map") if "is called without its rows" in p]

    def test_the_rows_of_the_emitter_are_clean(self):
        self.assertEqual(self.probs(self.program()), [])
        self.assertEqual(self.probs(self.program(guard=True)), [])
        for t in ("bool", "ptr_int"):
            with self.subTest(t=t):
                self.assertEqual(self.probs(self.program(rows=self.rows(t=t), call=f"  copy_arr_{t}(t__1, t__2, 3, t__3)")), [])  # the callee is absent: not this rule's

    def test_a_call_without_a_row_of_its_requires_is_refused(self):
        for what, drop in {
            "no amount": ("rd",), "no `none < rd`": ("pos",), "no source value permission": ("sv",), "no source flag permission": ("sd",),
            "no source flags": ("sf",), "no destination value permission": ("dv",), "no destination flag permission": ("dd",),
            "no internal row": ("int",),
        }.items():
            with self.subTest(what):
                self.assertTrue(self.probs(self.program(drop=drop)), what)
                self.assertTrue(self.probs(self.program(drop=drop, guard=True)), what + " (guarded)")

    def test_a_row_that_is_not_the_rows_of_this_call_is_refused(self):
        good = self.rows()
        other = {"the amount of the destination": {"rd": good["rd"].replace("t__2", "t__1")},
                 "the amount of the value permission alone": {"rd": good["rd"].replace("perm(t__2.v_int) < perm(t__2.d_int)) ? perm(t__2.v_int) : perm(t__2.d_int)", "perm(t__2.v_int)")},
                 "the amount of the flag alone": {"rd": good["rd"].replace(" : perm(t__2.d_int)", " : perm(t__2.v_int)")},
                 "a count other than the call's": {"sv": good["sv"].replace("(k < 3)", "(k < 2)")},
                 "the source's rows at full permission": {"sv": good["sv"].replace(", t__3)", ")")},
                 "the destination's rows over the source": {"dv": good["dv"].replace("block_of(t__1)", "block_of(t__2)")},
                 "the internal row on other blocks": {"int": "  assert block_of(t__1) != block_of(t__3)"},
                 "the internal row reversed": {"int": "  assert block_of(t__2) != block_of(t__1)"},
                 "the amount of another suffix": {"dd": good["dd"].replace(".d_int", ".d_bool")}}
        for what, change in other.items():
            with self.subTest(what):
                self.assertTrue(self.probs(self.program(rows={**good, **change})), what)

    def test_the_internal_row_is_the_line_before_the_call_and_the_guard_is_the_pinned_one(self):
        self.assertTrue(self.probs(self.program(after=["  assert none < t__3"])), "a row between the internal row and the call")
        self.assertTrue(self.probs(self.program(guard=True, guard_text="  if (t__2 != t__1) {")), "the guard reversed")
        self.assertTrue(self.probs(self.program(guard=True, guard_text="  if (t__1 == t__2) {")), "the guard is an equality")
        self.assertTrue(self.probs(self.program(guard=True, guard_text="  if (t__1 != t__2 && t__3 == none) {")), "the guard has another conjunct")
        lines = self.program(guard=True)
        i = lines.index("  }", lines.index("  copy_arr_int(t__1, t__2, 3, t__3)"))
        lines.insert(i, "  t__3 := none")  # a second statement in the guarded block
        self.assertTrue(self.probs(lines), "the guarded block holds more than the call")
        self.assertTrue(self.probs(self.program(call="  copy_arr_int(t__1, t__2, n, t__3)")), "a count that is not a literal")
        self.assertTrue(self.probs(self.program(call="  copy_arr_int(loc(t__1, 0), t__2, 3, t__3)")), "an argument that is not a variable")

    def test_a_method_that_only_carries_the_name_is_not_a_pinned_copy_and_its_calls_are_not_this_rules(self):
        lines = self.program()
        i = lines.index("method copy_arr_int(d: Ref, s: Ref, n: Int, rd: Perm)")
        lines[i + 1] = "  requires true"
        self.assertEqual(self.probs(lines), [])  # refused as a copy by the clause rule instead
        self.assertTrue([p for p in lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map") if "not the trusted array copy" in p])


if __name__ == "__main__":
    unittest.main()
