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


def lint_vpr_other(*args, **kwargs):
    """lv.lint_vpr without the termination rules (decision 0013 item 5): the fixtures of the older tests are
    about other rules, and the termination rules have their own tests (TerminationClauseTests), which call
    lv.lint_vpr itself."""
    return [p for p in lv.lint_vpr(*args, **kwargs) if not any(t in p for t in TERMINATION_RULES)]


class Check:
    """A line of a fixture that needs a position-map entry."""

    def __init__(self, text, error_id="assert.failed", category="overflow"):
        self.text, self.error_id, self.category = text, error_id, category


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
                                      "5", f"message of line {i}"]))
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
        return lv.lint_vpr(*build(lines), "t.vpr", "t.vpr.map")

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
        probs = self.strict(lines)
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
        probs = [p for p in self.strict(lines) if "without a `decreases`" not in p]
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
                    f"  assert forperm t: Ref [t.tok] :: t == loc(block_of(p), (-1)) || {self.SLOT}",
                    "  exhale forperm t: Ref [t.tok] :: false",
                    "  assert (forperm t: Ref [t.tok] :: false)",
                    "  var b: Bool := forperm t: Ref [t.tok] :: false"):
            with self.subTest(row=row):
                probs = self.strict(self.fixture(body=[Check(row, A, "leak")]))
                self.assertTrue(probs, row)

    def test_forperm_with_an_exemption_is_clean(self):
        row = "  assert forperm t: Ref [t.tok] :: t == loc(block_of(res), (-1)) || (p != null && t == loc(block_of(p), (-1)))"
        self.assertEqual(self.strict(self.fixture(body=[Check(row, A, "leak")])), [])

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
    HELD = f"  held_v := held_v && ((v != null) && (perm(loc(block_of(v), (-1)).tok) > none))"

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
            Check(f"  assert forperm t: Ref [t.tok] :: {exempt or '(p != null) && (t == loc(block_of(p), (-1)))'}", A, "leak"),
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
        for e in ("false", "t == loc(block_of(res), (-1))", "(v != null) && (t == loc(block_of(v), (-1)))",
                  "(held_v && (t == loc(block_of(v), (-1)))) || ((w != null) && (t == loc(block_of(w), (-1))))",
                  "((((loc(member(s, 0), 0) == q) && (k > none)) ? q.v_ptr_int : null) != null) && (t == loc(block_of(q.v_ptr_int), (-1)))",
                  "(held_v && (t == loc(block_of(v), -1)))"):
            with self.subTest(e=e):
                self.assertEqual(self.leak(e), [])

    def test_nested_disjunctions_of_exemption_terms_are_flattened(self):
        a, b, c = ("(held_%s && (t == loc(block_of(%s), (-1))))" % (n, n) for n in ("u", "v", "w"))
        d = "(w != null) && (t == loc(block_of(w), (-1)))"
        for e in (f"(({a} || {b}) || {c})", f"({a} || ({b} || {c}))", f"((({a} || {b}) || {c}) || ({d}))", f"{a} || {b} || {c}"):
            with self.subTest(e=e):
                self.assertEqual(self.leak(e), [])

    def test_a_nested_disjunct_without_a_token_term_is_still_a_problem(self):
        a, b = ("(held_%s && (t == loc(block_of(%s), (-1))))" % (n, n) for n in ("u", "v"))
        for e in (f"(({a} || {b}) || true)", f"(({a} || (v != null)) || {b})", f"(({a} || {b}) || (t == t))",
                  f"(({a} || {b}) || (t != loc(block_of(v), (-1))))", f"(({a} || {b}) || !(t == loc(block_of(v), (-1))))"):
            with self.subTest(e=e):
                probs = self.leak(e)
                self.assertEqual(len(probs), 1, probs)
                self.assertTrue("a `forperm` leak row is" in probs[0] or "in a `forperm` exemption" in probs[0], probs)

    def test_an_exemption_that_could_exempt_more_than_a_carried_block_is_a_problem(self):
        for e in ("true", "v != null", "t == loc(block_of(v), (-1)) || true", "(v != null) || (t == loc(block_of(v), (-1)))",
                  "!(t == loc(block_of(v), (-1)))", "t != loc(block_of(v), (-1))", "t == v", "t == loc(block_of(v), 0)",
                  "t == loc(block_of(v), (-2))", "(t == loc(block_of(v), (-1))) && (t == loc(block_of(w), (-1)))",
                  "(t == t) && (t == loc(block_of(v), (-1)))", "(t == null) && (t == loc(block_of(v), (-1)))"):
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
    guard names exactly `t == loc(block_of(loc(GB_x(), 0).v_ptr_T), -1)`; `held_GB_x == ...` only compares with the cell's
    nullness.  Each rule has a twin that breaks exactly it (only the messages of these rules are read)."""

    CELL = "loc(GB_g__0(), 0).v_ptr_int"
    TERM = f"held_GB_g__0 && (t == loc(block_of({CELL}), (-1)))"
    UPD_LOCAL = "  held_v := held_v && ((v != null) && (perm(loc(block_of(v), (-1)).tok) > none))"
    UPD_GLOBAL = (f"  held_GB_g__0 := (held_GB_g__0 && (perm({CELL}) > none)) && (({CELL} != null) && "
                  f"(perm(loc(block_of({CELL}), (-1)).tok) > none))")
    UPD_CELL = ("  held_v := (held_v && (perm(v.v_ptr_int) > none)) && ((v.v_ptr_int != null) && "
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
        other = "held_GB_g__0 && (t == loc(block_of(loc(GB_h__0(), 0).v_ptr_int), (-1)))"
        probs = self.probs(term=other, extra_gb=True)
        self.assertTrue(any("guarded by `held_GB_g__0`" in p for p in probs), probs)
        probs = self.probs(term="held_GB_g__0 && (t == loc(block_of(v), (-1)))")
        self.assertTrue(any("guarded by `held_GB_g__0`" in p for p in probs), probs)
        probs = self.probs(term="held_GB_g__0 && (t == loc(block_of(loc(GB_g__0(), 0).v_int), (-1)))")
        self.assertTrue(any("guarded by `held_GB_g__0`" in p for p in probs), probs)

    def test_the_terms_of_a_nested_disjunction_are_each_read(self):
        def term(n):
            return f"(held_GB_{n}__0 && (t == loc(block_of(loc(GB_{n}__0(), 0).v_ptr_int), (-1))))"
        lines = self.fixture(extra_gb=True, term=f"(({term('g')} || {term('h')}) || {term('g')})")
        probs = [p for p in self.strict(lines) if "decision 0054" in p]
        self.assertTrue(any("`GB_h__0`, which `program_start` does not tag" in p for p in probs), probs)
        lines = self.fixture(extra_gb=True, term=f"(({term('g')} || {term('g')}) || {term('g')})")
        self.assertEqual([p for p in self.strict(lines) if "decision 0054" in p], [])

    def test_an_untagged_block_in_a_held_exemption_term_is_a_problem(self):
        probs = self.probs(tag=None)
        self.assertTrue(any("exemption term" in p and "does not tag" in p for p in probs), probs)

    def test_a_term_without_a_held_guard_is_an_ordinary_block_exemption(self):
        probs = self.probs(tag=None, term=f"(t == loc(block_of({self.CELL}), (-1)))", body=["  held_v := false"])
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
    TERM = f"held_GB_g__0_0 && (t == loc(block_of({CELL}), (-1)))"
    TAG = "  // cerastes:owning-global GB_g__0/0"

    @staticmethod
    def upd(ghost, cell):
        return (f"  {ghost} := ({ghost} && (perm({cell}) > none)) && (({cell} != null) && "
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
        term = f"{ghost} && (t == loc(block_of({leaf}.v_ptr_int), (-1)))"
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
                term = f"held_GB_g__0_0 && (t == loc(block_of({cell}), (-1)))"
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
                             term=f"held_GB_g__0 && (t == loc(block_of({self.CELL}), (-1)))")
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
    EXM_TERM = (f"(held_GB_g__0_0 && (t == loc(block_of({L0CELL}), (-1)))) || (exm_0 && (t == loc(block_of({NEXT0}), (-1)))) || "
                f"(exm_1 && (t == loc(block_of({NEXT1}), (-1))))")
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
        for term in (self.EXM_TERM.replace(f"exm_0 && (t == loc(block_of({n0})", f"exm_0 && (t == loc(block_of({n1})"),
                     self.EXM_TERM.replace(f"exm_1 && (t == loc(block_of({n1})", f"exm_1 && (t == loc(block_of({n0})"),
                     self.EXM_TERM.replace(f"exm_0 && (t == loc(block_of({n0})", f"exm_0 && (t == loc(block_of({self.L0CELL})"),
                     self.EXM_TERM.replace(f"exm_0 && (t == loc(block_of({n0}), (-1)))", f"exm_0 && (t == loc(block_of({n0}), 0))")):
            with self.subTest(term=term):
                probs = self.exm_probs([self.EXM0, self.EXM1], term=term)
                self.assertTrue(any("the exemption term guarded by `exm_" in p for p in probs), probs)

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
        term = f"(held_GB_g__0 && (t == loc(block_of({cell}), (-1)))) || (exm_0 && (t == loc(block_of({nxt}), (-1))))"

        def probs(line, t=term):
            lines = base.fixture(body=["  var exm_0: Bool", *base.LEGAL, line], term=t)
            return [p for p in base.strict(lines) if "decision 0055" in p]
        self.assertEqual(probs(exm0), [])
        self.assertTrue(any("guards the cell of its own owner" in p for p in probs(exm0.replace(f"({cell} != null)", "(loc(GB_h__0(), 0).v_ptr_int != null)"))))
        self.assertTrue(any("not the ghost of an owner tagged" in p for p in probs(exm0.replace("held_GB_g__0 &&", "held_v &&"))))
        self.assertTrue(any("the exemption term guarded by `exm_0`" in p for p in probs(exm0, term.replace(f"exm_0 && (t == loc(block_of({nxt})", f"exm_0 && (t == loc(block_of({cell})"))))



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


if __name__ == "__main__":
    unittest.main()
