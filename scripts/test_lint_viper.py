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
        self.assertEqual(self.strict(lines), [])

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


if __name__ == "__main__":
    unittest.main()
