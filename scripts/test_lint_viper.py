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


if __name__ == "__main__":
    unittest.main()
