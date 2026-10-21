#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Self-test for scripts/mutate.py.

The lexer and the mutant generator are checked against exact expected ids; every
rule of the generator has a twin that must not be mutated (a comment, a string,
a macro body, a declaration, `->`, `<<`, a hex literal, a check).  The run is
checked against a fake `cerastes verify` (a small script that exits with the
status a rule table gives for the mutant's text), so that killed, surviving,
stillborn and failing mutants, the equivalence list, per-mode outcomes and the
exit statuses are all exercised without the real driver.
"""

import contextlib
import hashlib
import io
import json
import os
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import mutate as mu  # noqa: E402
import run_tests as rt  # noqa: E402

SAMPLE = """\
#include <cerastes.h>
#define X(a) a < 3
int f(int x, int y) CERASTES_REQUIRES(x > 0 && y >= 1) CERASTES_ENSURES(r, r <= 100)
{
  int i = 0; // comment x < 5
  CERASTES_INVARIANT(i <= x && 0 <= i)
  while (i < x) { i++; }
  y = y + 10u;
  if (x == 3 || y != 4) return 1;
  CERASTES_CHECK(y > 0);
  return i << 2;
}
"""


def ids(src, path="t.c", op=None):
    return [m.id for m in mu.mutants_of(src, path) if op is None or m.op == op]


def body(statements, decl="int f(int x, int y)"):
    return f"{decl}\n{{\n{statements}\n}}\n"


class LexTests(unittest.TestCase):
    def texts(self, src):
        return [t.text for t in mu.lex(src)]

    def test_comments_strings_and_preprocessor_lines_produce_no_tokens(self):
        src = ('#include <x.h>\n#define M(a) \\\n   a < 3\nint x; // y < 2\n'
               '/* z */ char *s = "a < b"; char c = \'<\';\n')
        self.assertEqual(self.texts(src), ["int", "x", ";", "char", "*", "s", "=", '"a < b"', ";",
                                           "char", "c", "=", "'<'", ";"])

    def test_a_hash_that_is_not_at_the_start_of_a_line_is_an_operator(self):
        self.assertEqual(self.texts("a # b"), ["a", "#", "b"])
        self.assertEqual(self.texts("  # define X 1\nint y;"), ["int", "y", ";"])

    def test_longest_operator_wins(self):
        self.assertEqual(self.texts("a<<=b>>c<=d->e++&&f||g!=h"),
                         ["a", "<<=", "b", ">>", "c", "<=", "d", "->", "e", "++", "&&", "f", "||",
                          "g", "!=", "h"])

    def test_numbers(self):
        self.assertEqual(self.texts("10u 0x1F 1.5e+3 0777 1'000 .5"),
                         ["10u", "0x1F", "1.5e+3", "0777", "1'000", ".5"])

    def test_a_string_with_escapes_and_a_prefix(self):
        self.assertEqual(self.texts(r'u8"a\"b" L\'x\'' .replace("\\'", "'")),
                         [r'"a\"b"', "'x'"])

    def test_a_continued_line_comment(self):
        self.assertEqual(self.texts("a // x \\\n y < 3\nb"), ["a", "b"])

    def test_errors(self):
        for src in ("/* never closed", 'char *s = "open;\n', "int x = 'a;\n", 'R"(raw)"', "a @ b"):
            with self.subTest(src), self.assertRaises(mu.LexError):
                mu.lex(src)

    def test_brackets_must_balance(self):
        for src in ("int f( {", "int f() }", "int f(] {", "{ ( }"):
            with self.subTest(src), self.assertRaises(mu.LexError):
                mu.matching(mu.lex(src))
        pair = mu.matching(mu.lex("f(a[1]){}"))
        self.assertEqual(sorted(pair.items()), [(1, 6), (3, 5), (7, 8)])


class TargetTests(unittest.TestCase):
    """Where mutants are, and are not."""

    def test_the_sample_exactly(self):
        self.assertEqual(ids(SAMPLE), [
            "t.c:3:39:drop-conjunct:x > 0 &&->",
            "t.c:3:41:relop:>->>=",
            "t.c:3:43:literal:0->1",
            "t.c:3:45:drop-conjunct:&& y >= 1->",
            "t.c:3:45:swap-logic:&&->||",
            "t.c:3:50:relop:>=->>",
            "t.c:3:53:literal:1->0",
            "t.c:3:53:literal:1->2",
            "t.c:3:78:relop:<=-><",
            "t.c:3:81:literal:100->101",
            "t.c:3:81:literal:100->99",
            "t.c:5:11:literal:0->1",
            "t.c:6:22:drop-conjunct:i <= x &&->",
            "t.c:6:24:relop:<=-><",
            "t.c:6:29:drop-conjunct:&& 0 <= i->",
            "t.c:6:29:swap-logic:&&->||",
            "t.c:6:32:literal:0->1",
            "t.c:6:34:relop:<=-><",
            "t.c:7:12:relop:<-><=",
            "t.c:7:19:delete-stmt:i++;->;",
            "t.c:8:3:delete-stmt:y = y + 10u;->;",
            "t.c:8:11:literal:10u->11u",
            "t.c:8:11:literal:10u->9u",
            "t.c:9:9:relop:==->!=",
            "t.c:9:12:literal:3->2",
            "t.c:9:12:literal:3->4",
            "t.c:9:14:swap-logic:||->&&",
            "t.c:9:19:relop:!=->==",
            "t.c:9:22:literal:4->3",
            "t.c:9:22:literal:4->5",
            "t.c:9:32:literal:1->0",
            "t.c:9:32:literal:1->2",
            "t.c:10:20:relop:>->>=",
            "t.c:10:22:literal:0->1",
            "t.c:11:3:delete-stmt:return i << 2;->;",
            "t.c:11:15:literal:2->1",
            "t.c:11:15:literal:2->3",
        ])

    def test_every_mutant_is_real_and_well_formed(self):
        for m in mu.mutants_of(SAMPLE, "t.c"):
            mutated = m.apply(SAMPLE)
            self.assertNotEqual(mutated, SAMPLE, m.id)
            mu.matching(mu.lex(mutated))  # still lexes, brackets still balance
            self.assertTrue(m.id.startswith("t.c:"))
            self.assertNotIn("\n", m.id)
            self.assertNotIn("\t", m.id)
            self.assertRegex(m.id, mu.ID_RE)

    def test_the_position_is_the_first_changed_character(self):
        lines = SAMPLE.split("\n")
        for m in mu.mutants_of(SAMPLE, "t.c"):
            shown = lines[m.line - 1][m.column - 1:]
            if m.op in ("relop", "literal", "swap-logic"):
                self.assertTrue(shown.startswith(m.original), m.id)
            elif m.op == "drop-conjunct":
                self.assertTrue(shown.startswith(m.original.split()[0]), m.id)
            elif m.op == "delete-stmt":
                self.assertTrue(shown.startswith(m.original.split()[0]), m.id)

    def test_the_ids_are_unique_and_sorted_by_position(self):
        found = mu.mutants_of(SAMPLE, "t.c")
        self.assertEqual(len({m.id for m in found}), len(found))
        self.assertEqual([m.start for m in found], sorted(m.start for m in found))

    def test_the_path_is_part_of_the_id(self):
        self.assertTrue(all(i.startswith("tests/scalar/a.c:")
                            for i in ids(SAMPLE, "tests/scalar/a.c")))

    # -- where nothing is mutated ------------------------------------------------

    def test_nothing_outside_function_bodies_and_contracts(self):
        for src in ("int a = 3 < 4 && 5 > 2;\n", "int f(int x);\n", "int g(int x = 1 + 2);\n",
                    "struct s { int a[3]; };\n", "enum { A = 1 < 2 };\n",
                    "static_assert(1 < 2, \"x\");\n", "int h(void) { return 0; } int a[2 + 1];\n"
                    .replace("return 0;", "")):
            with self.subTest(src):
                self.assertEqual(ids(src), [], src)

    def test_a_contract_on_a_declaration_is_mutated(self):
        got = ids("int f(int x) CERASTES_REQUIRES(x > 0);\n")
        self.assertEqual(got, ["t.c:1:34:relop:>->>=", "t.c:1:36:literal:0->1"])

    def test_the_attribute_spelling_of_a_contract_is_mutated(self):
        got = ids("[[cerastes::requires(x > 0)]] int f(int x);\n")
        self.assertEqual([i.split(":", 3)[3] for i in got], ["relop:>->>=", "literal:0->1"])
        self.assertEqual(ids("[[other::requires(x > 0)]] int f(int x);\n"), [])
        self.assertEqual(ids("[[cerastes::predicate]] int f(int x);\n"), [])

    def test_other_macros_are_not_contracts(self):
        self.assertEqual(ids("int f(int x) MY_REQUIRES(x > 0);\n"), [])
        self.assertEqual(ids("int g = CERASTES_X(1 < 2);\n"), [i for i in ids(
            "int g = CERASTES_X(1 < 2);\n")])  # a CERASTES_ macro anywhere counts
        self.assertTrue(ids("int g = CERASTES_X(1 < 2);\n"))

    def test_comments_strings_and_macro_bodies_inside_a_body_are_not_mutated(self):
        src = body('  // x < 3\n  /* y > 4 */\n  char *s = "a < b && 1";\n'
                   "#define Q(a) (a < 7)\n  return 0;")
        self.assertEqual(ids(src, op="relop"), [])
        self.assertEqual(ids(src, op="swap-logic"), [])
        self.assertEqual([i for i in ids(src) if ":literal:" in i], [i for i in ids(src)
                         if ":literal:" in i and i.endswith(("0->1",))])  # only `return 0`

    def test_these_operators_are_not_relational(self):
        for operator in ("<<", ">>", "->", "<<=", ">>=", "<=>"):
            src = body(f"  return a {operator} b;")
            with self.subTest(operator):
                self.assertEqual(ids(src, op="relop"), [])

    def test_only_plain_decimal_integers_are_literals(self):
        for lit in ("0x10", "010", "1.5", "1e3", "0b11", "'a'", '"s"', "1'000", "2.f"):
            with self.subTest(lit):
                self.assertEqual(ids(body(f"  return {lit};"), op="literal"), [], lit)

    def test_literals_keep_their_suffix_and_zero_has_one_neighbour(self):
        got = ids(body("  return 7ul + 0u;"), op="literal")
        self.assertEqual([i.split(":", 4)[4] for i in got], ["7ul->6ul", "7ul->8ul", "0u->1u"])

    def test_a_negative_literal_is_the_literal_after_the_minus(self):
        got = ids(body("  return -1;"), op="literal")
        self.assertEqual([i.split(":", 4)[4] for i in got], ["1->0", "1->2"])

    # -- the operators -------------------------------------------------------------

    def test_relops(self):
        table = {"<": "<=", "<=": "<", ">": ">=", ">=": ">", "==": "!=", "!=": "=="}
        for op, flipped in table.items():
            got = ids(body(f"  return x {op} y;"), op="relop")
            self.assertEqual([i.split(":", 4)[4] for i in got], [f"{op}->{flipped}"], op)
        for op in ("+", "-", "*", "/", "%", "=", "&", "|", "^", "&&", "||", "!", "~"):
            self.assertEqual(ids(body(f"  y = x {op} y;"), op="relop"), [], op)

    def test_swap_logic_both_ways(self):
        got = ids(body("  return x && y || y && x;"), op="swap-logic")
        self.assertEqual([i.split(":", 4)[4] for i in got], ["&&->||", "||->&&", "&&->||"])

    def test_drop_conjunct_drops_each_side(self):
        got = ids(body("  return x > 0 && y > 0;"), op="drop-conjunct")
        self.assertEqual([i.split(":", 4)[4] for i in got], ["x > 0 &&->", "&& y > 0->"])
        self.assertEqual(self.returns("return x > 0 && y > 0;"), ["x>0", "y>0"])
    def test_a_chain_of_conjuncts_loses_one_at_a_time(self):
        # `a && b && c`: each `&&` loses its left and its right operand
        self.assertEqual(self.returns("return a && b && c;"),
                         ["a&&b", "a&&c", "a&&c", "b&&c"])
    def test_conjuncts_end_at_the_right_boundaries(self):
        cases = {
            "return a == 1 && f(b, c) || d;": ["a==1||d", "f(b,c)||d"],
            "return (a || b) && c ? d : e;": ["(a||b)?d:e", "c?d:e"],
            "return x[i + 1] > 0 && !y;": ["!y", "x[i+1]>0"],
            "z = a && b;": ["z=a", "z=b"],
        }
        for stmt, expected in cases.items():
            with self.subTest(stmt):
                self.assertEqual(self.returns(stmt, whole=True), sorted(expected))
    def test_a_conjunct_inside_a_call_or_group_stops_at_the_bracket(self):
        self.assertEqual(self.returns("return f(a && b, c);"), ["f(a,c)", "f(b,c)"])
    def returns(self, stmt, whole=False):
        """The statement `stmt` after each drop-conjunct mutation, spaces removed, sorted
        (only the expression of a `return` unless `whole`)."""
        src = body("  " + stmt)
        out = []
        for m in mu.mutants_of(src, "t.c"):
            if m.op == "drop-conjunct":
                line = m.apply(src).split("\n")[2].strip()
                if line.startswith("return "):
                    line = line[len("return "):]
                out.append("".join(line.rstrip(";").split()))
        return sorted(out)

    def statements(self, stmts, decl="int f(int x, int y)"):
        src = body(stmts, decl)
        return [m.original for m in mu.mutants_of(src, "t.c") if m.op == "delete-stmt"]

    def test_expression_statements_and_returns_are_deleted(self):
        got = self.statements("  y = x;\n  y += 2;\n  x++;\n  --y;\n  g(x);\n  (void)g(y);\n"
                              "  return y;")
        self.assertEqual(got, ["y = x;", "y += 2;", "x++;", "--y;", "g(x);", "(void)g(y);",
                               "return y;"])

    def test_what_is_never_deleted(self):
        for stmt in ("int a = 1;", "unsigned char c = 2;", "const int k = 3;", "static int s = 4;",
                     "uint8_t u = 5;", "struct p q = {1};", "CERASTES_CHECK(x > 0);",
                     "CERASTES_FOLD(p);", "static_assert(1, \"m\");", "_Static_assert(1, \"m\");",
                     ";", "if (x) y = 1;", "while (x) x--;", "for (;;) y = 1;",
                     "switch (x) { }", "goto *p;", "typedef int T;",
                     "do { } while (x);", "switch (x) { case 1: ; default: ; }"):
            with self.subTest(stmt):
                self.assertEqual(self.statements("  " + stmt), [], stmt)

    def test_statements_in_nested_blocks_are_found(self):
        got = self.statements("  if (x) {\n    y = 1;\n    { y = 2; }\n  }\n"
                              "  while (y) { y--; }\n  for (int i = 0; i < x; i++) { y += i; }")
        self.assertEqual(got, ["y = 1;", "y = 2;", "y--;", "y += i;"])

    def test_the_body_of_an_else_or_a_do_is_a_statement(self):
        self.assertEqual(self.statements("  if (x) { } else y = 2;"), ["y = 2;"])
        self.assertEqual(self.statements("  do x--; while (x);"), ["x--;"])

    def test_the_header_of_a_for_loop_is_not_three_statements(self):
        self.assertEqual(self.statements("  for (y = 0; y < x; y++) { }"), [])

    def test_a_deleted_statement_becomes_an_empty_one(self):
        src = body("  y = x;\n  return y;")
        m = [m for m in mu.mutants_of(src, "t.c") if m.op == "delete-stmt"][0]
        self.assertEqual(m.replacement, ";")
        self.assertEqual(m.apply(src), body("  ;\n  return y;"))
        self.assertEqual(m.new, ";")

    def test_a_long_statement_is_shortened_in_its_id(self):
        long_stmt = "  y = " + " + ".join(["x"] * 30) + ";"
        (got,) = self.statements(long_stmt)
        self.assertEqual(len(got), mu.STATEMENT_ID_CHARS)
        self.assertTrue(got.endswith("..."))

    def test_a_statement_spread_over_lines_has_one_line_ids(self):
        got = self.statements("  y =\n      x +\n      1;")
        self.assertEqual(got, ["y = x + 1;"])

    def test_statements_after_else_and_do(self):
        self.assertEqual(self.statements("  if (x) { y = 1; } else { y = 2; }"),
                         ["y = 1;", "y = 2;"])

    def test_function_bodies_only(self):
        src = "struct s { int a; };\nint g(void) { return 1; }\nint h = 3;\nint f(int x) { x = 1; }\n"
        got = [m.original for m in mu.mutants_of(src, "t.c") if m.op == "delete-stmt"]
        self.assertEqual(got, ["return 1;", "x = 1;"])

    def test_a_function_with_a_contract_between_parameters_and_body(self):
        src = "int f(int x) CERASTES_ENSURES(r, r > 0) { return x; }\n"
        self.assertEqual([m.original for m in mu.mutants_of(src, "t.c")
                          if m.op == "delete-stmt"], ["return x;"])

    # -- break, continue and the statements after case labels (decision 0014 item 9) --

    def test_break_and_continue_are_deleted(self):
        got = self.statements("  while (x) {\n    if (y) {\n      break;\n    }\n"
                              "    continue;\n  }")
        self.assertEqual(got, ["break;", "continue;"])

    def test_a_deleted_break_becomes_an_empty_statement_and_nothing_else_changes(self):
        src = body("  while (x) {\n    if (y) {\n      break;\n    }\n    x--;\n  }")
        (m,) = [m for m in mu.mutants_of(src, "t.c") if m.original == "break;"]
        self.assertEqual(m.apply(src), src.replace("break;", ";"))
        self.assertEqual((m.line, m.column), (5, 7))
        self.assertEqual(m.id, "t.c:5:7:delete-stmt:break;->;")

    def test_the_jump_that_is_the_body_of_a_head_is_deleted_but_no_other_body(self):
        # `if (x) break;` and its kin: the jump goes, the head stays.  Other bodies of a
        # head stay as M3 left them (their ids are in tests/mutants-equivalent.txt).
        got = self.statements("  while (x) {\n    if (y) break;\n    if (x) continue;\n"
                              "    for (;;) break;\n    if (x) if (y) continue;\n"
                              "    if (y) x = 1;\n    while (y) y--;\n  }")
        self.assertEqual(got, ["break;", "continue;", "break;", "continue;"])
        src = body("  while (x) {\n    if (y) break;\n  }")
        (m,) = [m for m in mu.mutants_of(src, "t.c") if m.op == "delete-stmt"]
        self.assertEqual(m.apply(src), src.replace("break;", ";"))
        self.assertEqual((m.line, m.column), (4, 12))

    def test_the_jump_after_else_and_do_is_deleted(self):
        got = self.statements("  while (x) {\n    if (y) { } else break;\n    do continue; while (y);\n  }")
        self.assertEqual(got, ["break;", "continue;"])

    def test_the_statement_after_a_case_or_default_label_is_deleted(self):
        got = self.statements("  switch (x) {\n  case 1: y = 1; break;\n  case 2: case 3: y = 2;\n"
                              "  default: return y;\n  }")
        self.assertEqual(got, ["y = 1;", "break;", "y = 2;", "return y;"])

    def test_a_label_is_never_deleted_only_the_statement_after_it(self):
        src = body("  switch (x) {\n  case 1: break;\n  default: y = 1;\n  }")
        mutants = [m for m in mu.mutants_of(src, "t.c") if m.op == "delete-stmt"]
        self.assertEqual([m.original for m in mutants], ["break;", "y = 1;"])
        self.assertEqual([m.apply(src).split("\n")[3:5] for m in mutants],
                         [["  case 1: ;", "  default: y = 1;"], ["  case 1: break;", "  default: ;"]])
        self.assertEqual([(m.line, m.column) for m in mutants], [(4, 11), (5, 12)])

    def test_labels_followed_by_no_deletable_statement_give_no_mutant(self):
        for stmt in ("switch (x) { case 1: ; default: ; }",
                     "switch (x) { case 1: int z = 1; break; }",
                     "switch (x) { case 1: CERASTES_CHECK(x > 0); }",
                     "switch (x) { case 1: if (y) { } }",
                     "switch (x) { case 1: while (y) y--; }",
                     "switch (x) { case 1: { } default: { } }"):
            with self.subTest(stmt):
                got = [m.original for m in mu.mutants_of(body("  " + stmt), "t.c")
                       if m.op == "delete-stmt"]
                self.assertEqual(got, ["break;"] if "break" in stmt else [], stmt)

    def test_case_values_with_colons_brackets_and_ternaries_end_at_the_right_colon(self):
        for label in ("case 1 ? 2 : 3:", "case (1 ? 2 : 3):", "case 'a':", "case ':':",
                      "case A::b:", "case 1 ? (2 ? 3 : 4) : 5:", "default:"):
            with self.subTest(label):
                self.assertEqual(self.statements(f"  switch (x) {{\n  {label} y = 1;\n  }}"),
                                 ["y = 1;"])

    def test_labels_chain_before_one_statement(self):
        self.assertEqual(self.statements("  switch (x) { case 1: case 2: default: y = 1; }"),
                         ["y = 1;"])

    def test_a_malformed_label_gives_no_mutant(self):
        # `case` with no colon in its statement: the label is not skipped, `case` is a keyword
        src = body("  switch (x) { case 1 y = 1; }")
        self.assertEqual([m for m in mu.mutants_of(src, "t.c") if m.op == "delete-stmt"], [])

    def test_a_switch_head_and_a_do_head_are_not_deleted_and_bodies_are(self):
        self.assertEqual(self.statements("  switch (x) { }"), [])
        self.assertEqual(self.statements("  do { y = 1; } while (x);"), ["y = 1;"])
        self.assertEqual(self.statements("  do { } while (x);"), [])
        self.assertEqual(self.statements("  while (x) ;"), [])

    def test_jump_statements_with_other_tokens_are_not_deleted(self):
        self.assertEqual(self.statements("  goto *p;"), [])  # a computed goto has no label to keep
        self.assertEqual(self.statements("  goto l, m;"), [])
        self.assertEqual(self.statements("  breaker = 1;"), ["breaker = 1;"])
        self.assertEqual(self.statements("  gotox l;"), [])  # a declaration-shaped statement
        self.assertEqual(self.statements("  goto_l = 1;"), ["goto_l = 1;"])

    # -- goto, labels, noreturn calls and assert (decision 0015) --

    def test_a_goto_is_deleted(self):
        got = self.statements("  if (y) {\n    goto done;\n  }\n  y = 1;\n  done: return y;")
        self.assertEqual(got, ["goto done;", "y = 1;", "return y;"])

    def test_a_deleted_goto_becomes_an_empty_statement_and_nothing_else_changes(self):
        src = body("  if (y) {\n    goto done;\n  }\n  y = 1;\n  done: return y;")
        (m,) = [m for m in mu.mutants_of(src, "t.c") if m.original == "goto done;"]
        self.assertEqual(m.apply(src), src.replace("goto done;", ";"))
        self.assertEqual((m.line, m.column), (4, 5))
        self.assertEqual(m.id, "t.c:4:5:delete-stmt:goto done;->;")

    def test_the_goto_that_is_the_body_of_a_head_is_deleted_but_no_other_body(self):
        got = self.statements("  if (y) goto out;\n  while (x) if (y) goto out;\n"
                              "  for (;;) goto out;\n  if (x) y = 1;\n  out: ;")
        self.assertEqual(got, ["goto out;", "goto out;", "goto out;"])
        src = body("  if (y) goto out;\n  out: ;")
        (m,) = [m for m in mu.mutants_of(src, "t.c") if m.op == "delete-stmt"]
        self.assertEqual(m.apply(src), src.replace("goto out;", ";"))
        self.assertEqual((m.line, m.column), (3, 10))

    def test_the_goto_after_else_and_a_label_is_deleted(self):
        got = self.statements("  if (y) { } else goto out;\n  out: goto out2;\n  out2: ;")
        self.assertEqual(got, ["goto out;", "goto out2;"])

    def test_a_user_label_is_never_deleted_only_the_statement_after_it(self):
        src = body("  if (y) goto done;\n  done: y = 1;")
        mutants = [m for m in mu.mutants_of(src, "t.c") if m.op == "delete-stmt"]
        self.assertEqual([m.original for m in mutants], ["goto done;", "y = 1;"])
        self.assertEqual(mutants[1].apply(src).split("\n")[3], "  done: ;")
        self.assertEqual((mutants[1].line, mutants[1].column), (4, 9))

    def test_labels_followed_by_no_deletable_statement_give_no_mutant(self):
        for stmt in ("l: ;", "l: int z = 1;", "l: CERASTES_CHECK(x > 0);", "l: { }",
                     "l: if (y) { }", "l: m: ;", "l: while (y) y--;"):
            with self.subTest(stmt):
                got = [m.original for m in mu.mutants_of(body("  " + stmt), "t.c")
                       if m.op == "delete-stmt"]
                self.assertEqual(got, [], stmt)

    def test_labels_chain_before_one_statement_with_case_labels(self):
        self.assertEqual(self.statements("  switch (x) { case 1: l: m: y = 1; }"), ["y = 1;"])

    def test_a_statement_that_merely_contains_a_colon_is_still_one_statement(self):
        # not a label: the `:` is not the second token, or is part of `::`
        self.assertEqual(self.statements("  x ? y : z;"), ["x ? y : z;"])
        self.assertEqual(self.statements("  y = x ? 1 : 2;"), ["y = x ? 1 : 2;"])
        self.assertEqual(self.statements("  ns::f(y);"), ["ns::f(y);"])

    def test_a_call_of_a_noreturn_function_is_deleted_like_any_call(self):
        got = self.statements("  if (x == 0) abort();\n  if (y == 0) {\n    exit(1);\n  }\n"
                              "  _Exit(2);\n  fail();\n  die(y);\n  return 0;")
        self.assertEqual(got, ["exit(1);", "_Exit(2);", "fail();", "die(y);", "return 0;"])
        # (the body of an `if` that is a call stays: only a jump is the body of a head)

    def test_an_assert_statement_is_deleted(self):
        got = self.statements("  assert(x > 0);\n  assert(x > 0 && y < 3);\n  return x;")
        self.assertEqual(got, ["assert(x > 0);", "assert(x > 0 && y < 3);", "return x;"])

    def test_an_assert_is_not_a_check_and_a_static_assert_is_never_deleted(self):
        self.assertEqual(self.statements("  static_assert(1, \"m\");\n  CERASTES_CHECK(x > 0);"), [])


def re_return(text):
    """The expression of the `return` of a one-function source."""
    return text.split("return ")[1].split(";")[0].strip()


class FunctionBodyTests(unittest.TestCase):
    """Which `{` start a function (auditor round 2, item 13)."""

    def names(self, src):
        return [n for n, _l, _s, _e in mu.function_spans(src)]

    def test_shapes_after_the_parameter_list(self):
        for what, src, name in (
                ("plain", "int f(int x) { return x; }", "f"),
                ("void parameters", "int f(void) { return 1; }", "f"),
                ("noexcept", "int f(int x) noexcept { return x; }", "f"),
                ("noexcept(true)", "int f(int x) noexcept(true) { return x; }", "f"),
                ("an attribute after the parameters", "int f(int x) [[maybe_unused]] { return x; }", "f"),
                ("a contract", "int f(int x) CERASTES_REQUIRES(x > 0) { return x; }", "f"),
                ("two contracts", "int f(int x) CERASTES_REQUIRES(x > 0)\n  CERASTES_ENSURES(r, r > 0)\n{ return x; }", "f"),
                ("a trailing return type", "auto f(int x) -> int { return x; }", "f"),
                ("a trailing return type and noexcept", "auto f(int x) noexcept -> int { return x; }", "f"),
                ("const and a reference", "int f(int x) const & { return x; }", "f"),
                ("a struct return type", "struct s { int a; };\nstruct s f(int x) { struct s v = {x}; return v; }", "f"),
                ("static inline", "static inline int f(int x) { return x; }", "f"),
                ("a pointer return type", "int *f(int x) { return 0; }", "f")):
            with self.subTest(what):
                self.assertEqual(self.names(src), [name], src)

    def test_what_is_not_a_function(self):
        for src in ("struct s { int a; };", "enum e { A, B };", "union u { int a; };",
                    "int a[] = {1, 2};", "int a[2] = { 1, 2 };", "auto l = [](int x) { return x; };",
                    "typedef struct { int a; } t;", "int x = (1 + 2);", "class c { int a; };",
                    "int a[] = { f(1), f(2) };"):
            with self.subTest(src):
                self.assertEqual(self.names(src), [], src)

    def test_several_functions_in_order(self):
        src = "int a(void) { return 1; }\nstruct s { int x; };\nint b(void) { return 2; }\n"
        self.assertEqual(self.names(src), ["a", "b"])

    def test_namespace_and_extern_c_blocks_are_entered(self):
        self.assertEqual(self.names("namespace n { int f(int x) { return x; } }"), ["f"])
        self.assertEqual(self.names('extern "C" { int f(int x) { return x; } }'), ["f"])
        self.assertEqual(self.names('extern "C" {\nint f(int x) { return x; }\nint g(int x) { return x; }\n}\nint h(void) { return 0; }'),
                         ["f", "g", "h"])

    def test_spans_and_lines(self):
        src = "int a(void)\n{\n  return 1;\n}\n"
        ((name, line, start, end),) = mu.function_spans(src)
        self.assertEqual((name, line), ("a", 2))
        self.assertEqual(src[start:end], "{\n  return 1;\n}")

    def test_nested_braces_do_not_end_a_function(self):
        src = "int f(int x)\n{\n  if (x) {\n    x++;\n  }\n  return x;\n}\nint g(void) { return 0; }\n"
        self.assertEqual(self.names(src), ["f", "g"])


class EquivalentsTests(unittest.TestCase):
    ID = "tests/scalar/a.c:5:9:relop:<-><="

    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.path = Path(tmp.name) / "eq.txt"

    def read(self, text):
        self.path.write_text(text)
        return mu.read_equivalents(self.path)

    def test_a_good_file(self):
        eq = self.read(f"# the reviewed equivalents\n# reviewed: Matt, 2026-10-16\n\n"
                       f"{self.ID}\tthe loop is never entered with x == 0\n")
        self.assertEqual(eq.problems, [])
        self.assertEqual(eq.entries, {self.ID: "the loop is never entered with x == 0"})
        self.assertEqual(eq.reviewed, "Matt, 2026-10-16")

    def test_a_missing_file_is_empty(self):
        eq = mu.read_equivalents(self.path.with_name("nowhere.txt"))
        self.assertEqual((eq.entries, eq.problems), ({}, []))

    def test_an_empty_file_and_comments_only_need_no_review(self):
        self.assertEqual(self.read("").problems, [])
        self.assertEqual(self.read("# nothing yet\n").problems, [])

    def test_entries_need_a_review_line(self):
        eq = self.read(f"{self.ID}\treason\n")
        self.assertEqual(len(eq.problems), 1)
        self.assertIn("no `# reviewed:", eq.problems[0])

    def test_bad_lines(self):
        for what, text in (("no tab", f"{self.ID} reason\n"),
                           ("no reason", f"{self.ID}\t\n"),
                           ("blank reason", f"{self.ID}\t   \n"),
                           ("three fields", f"{self.ID}\treason\textra\n"),
                           ("not an id", "something\treason\n"),
                           ("empty id", "\treason\n")):
            with self.subTest(what):
                eq = self.read("# reviewed: me\n" + text)
                self.assertEqual(eq.entries, {}, what)
                self.assertEqual(len(eq.problems), 1, (what, eq.problems))
                self.assertIn("eq.txt:2:", eq.problems[0].replace("/", "").replace(
                    str(self.path.parent).replace("/", ""), "") or "eq.txt:2:")

    def test_a_duplicate_entry(self):
        eq = self.read(f"# reviewed: me\n{self.ID}\ta\n{self.ID}\tb\n")
        self.assertEqual(len(eq.problems), 1)
        self.assertIn("listed twice", eq.problems[0])

    def test_staleness(self):
        eq = mu.Equivalents({self.ID: "r"}, "me", [])
        self.assertEqual(mu.check_equivalents(eq, {self.ID}, None), [])
        self.assertEqual(mu.check_equivalents(eq, {self.ID}, {self.ID: mu.SURVIVED}), [])
        gone = mu.check_equivalents(eq, set(), None)
        self.assertEqual(len(gone), 1)
        self.assertIn("no such mutant is generated", gone[0])
        for outcome in (mu.KILLED, mu.STILLBORN):
            stale = mu.check_equivalents(eq, {self.ID}, {self.ID: outcome})
            self.assertEqual(len(stale), 1, outcome)
            self.assertIn(f"the mutant is {outcome}, not a survivor", stale[0])
        self.assertIn("not run", mu.check_equivalents(eq, {self.ID}, {})[0])

    def test_format_problems_are_passed_on(self):
        eq = mu.Equivalents({}, None, ["bad line"])
        self.assertEqual(mu.check_equivalents(eq, set(), None), ["bad line"])


# A stand-in for `build/cerastes`: it exits with the status of the first rule whose
# text occurs in the file (and whose mode, if any, is the run's), else 0.  Rules are
# JSON `[[text, status, mode-or-null], ...]` in $FAKE_MUTATE_RULES; the arguments of
# every call are appended to $FAKE_MUTATE_LOG.  Status -9 kills the process, 99
# sleeps (for timeouts).
FAKE = r'''#!@PYTHON@
import json, os, sys, time
args = sys.argv[1:]
with open(os.environ["FAKE_MUTATE_LOG"], "a") as f:
    f.write(json.dumps(args) + "\n")
assert args[0] == "verify", args
src = open(args[1]).read()
std = [a for a in args[2:] if a.startswith("--std=")][0][len("--std="):]
status = 0
for text, code, mode in json.load(open(os.environ["FAKE_MUTATE_RULES"])):
    if text in src and mode in (None, std):
        status = code
        break
if status == -9:
    os.kill(os.getpid(), 9)
if status == 99:
    time.sleep(30)
sys.exit(status)
'''

GOOD = """\
// expect: verified
int f(int x)
{
  return x < 5;
}
"""


class RunCase(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.base = Path(tmp.name).resolve()
        self.dir = self.base / "t"
        self.dir.mkdir()
        fake = self.base / "fake_verify"
        fake.write_text(FAKE.replace("@PYTHON@", sys.executable))
        fake.chmod(0o755)
        self.log = self.base / "calls.log"
        self.rules_file = self.base / "rules.json"
        self.equivalents = self.base / "mutants-equivalent.txt"
        self.set_rules([])
        env_file = self.base / "env.sh"
        env_file.write_text("")
        # hermetic: nothing that `. build/env.sh` put in the shell may reach these tests
        inherited = mock.patch.dict(os.environ)
        inherited.start()
        self.addCleanup(inherited.stop)
        for name in [n for n in os.environ if n.startswith(("CERASTES_", "EDG_")) or n in ("SILICON_JAR", "Z3_EXE", "CPFE")]:
            del os.environ[name]
        for patcher in (mock.patch.object(rt, "VERIFY_COMMAND", [str(fake)]),
                        mock.patch.object(rt, "ENV_SH", env_file),
                        mock.patch.dict(os.environ, {"FAKE_MUTATE_LOG": str(self.log),
                                                     "FAKE_MUTATE_RULES": str(self.rules_file)})):
            patcher.start()
            self.addCleanup(patcher.stop)

    def set_rules(self, rules):
        self.rules_file.write_text(json.dumps(rules))

    def put(self, name, text=GOOD):
        path = self.dir / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)
        return path

    def mid(self, name, line, col, op, change):
        return f"{rt.rel(self.dir / name)}:{line}:{col}:{op}:{change}"

    def run_main(self, *args, equivalents=True):
        out, err = io.StringIO(), io.StringIO()
        extra = ["--equivalents", str(self.equivalents)] if equivalents else []
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            status = mu.main([*extra, *map(str, args)])
        return status, out.getvalue(), err.getvalue()

    def calls(self):
        if not self.log.exists():
            return []
        return [json.loads(l) for l in self.log.read_text().splitlines()]


# the mutants of GOOD: `x < 5` -> `x <= 5`, `5` -> `6`/`4`, `return x < 5;` deleted
GOOD_MUTANTS = ["4:12:relop:<-><=", "4:14:literal:5->6", "4:14:literal:5->4", "4:3:delete-stmt:return x < 5;->;"]


class RunTests(RunCase):
    def all_killed(self):
        self.set_rules([["x <= 5", 1, None], ["x < 6", 1, None], ["x < 4", 1, None],
                        ["  ;\n", 1, None]])

    def test_every_mutant_killed_is_clean(self):
        self.all_killed()
        t = self.put("a.c")
        status, out, _ = self.run_main(t)
        self.assertEqual(status, 0, out)
        self.assertIn("1 tests, 4 mutants: 4 killed, 0 equivalent (reviewed), 0 safe-direction, 0 unexplained "
                      "survivors, 0 stillborn, 0 tool failures", out)
        self.assertNotIn("SURVIVED", out)

    def test_the_command_line_of_a_run(self):
        self.all_killed()
        t = self.put("a.c")
        self.run_main(t)
        calls = self.calls()
        self.assertEqual(len(calls), 5)  # the original, then four mutants
        for call in calls:
            self.assertEqual((call[0], call[2]), ("verify", "--std=c23"))
            self.assertEqual(Path(call[1]).name, "a.c")
            self.assertNotEqual(Path(call[1]).parent, self.dir)  # a copy, never the test itself
            self.assertEqual(len(call), 4)
            # relative includes of the test resolve from the copy: -I<the test's directory>
            self.assertEqual(call[3], f"-I{self.dir}")

    def test_the_test_file_is_never_touched(self):
        self.all_killed()
        t = self.put("a.c")
        before = hashlib.sha256(t.read_bytes()).hexdigest()
        self.run_main(t)
        self.assertEqual(hashlib.sha256(t.read_bytes()).hexdigest(), before)
        self.assertEqual(sorted(p.name for p in self.dir.iterdir()), ["a.c"])

    def test_a_surviving_mutant_is_reported_with_its_id(self):
        self.set_rules([["x <= 5", 1, None], ["x < 6", 1, None], ["  ;\n", 1, None]])
        t = self.put("a.c")
        status, out, _ = self.run_main(t)
        self.assertEqual(status, 1, out)
        self.assertIn("SURVIVED " + self.mid("a.c", 4, 14, "literal", "5->4"), out)
        self.assertIn("1 unexplained survivors", out)

    def test_a_reviewed_equivalent_survivor_is_fine(self):
        self.set_rules([["x <= 5", 1, None], ["x < 6", 1, None], ["  ;\n", 1, None]])
        t = self.put("a.c")
        survivor = self.mid("a.c", 4, 14, "literal", "5->4")
        self.equivalents.write_text(f"# reviewed: me\n{survivor}\tx is never 4 here\n")
        status, out, _ = self.run_main(t)
        self.assertEqual(status, 0, out)
        self.assertNotIn("SURVIVED", out)
        self.assertIn("3 killed, 1 equivalent (reviewed), 0 safe-direction, 0 unexplained", out)

    def test_a_stale_entry_fails_the_run(self):
        self.all_killed()
        t = self.put("a.c")
        killed = self.mid("a.c", 4, 12, "relop", "<-><=")
        gone = self.mid("a.c", 99, 1, "relop", "<-><=")
        for entry, needle in ((killed, "the mutant is killed, not a survivor"),
                              (gone, "no such mutant is generated")):
            with self.subTest(needle):
                self.equivalents.write_text(f"# reviewed: me\n{entry}\tbecause\n")
                status, out, _ = self.run_main(t)
                self.assertEqual(status, 1, out)
                self.assertIn(needle, out)

    def test_stillborn_mutants_are_listed_and_not_counted(self):
        self.set_rules([["x <= 5", 2, None], ["x < 6", 1, None], ["x < 4", 1, None],
                        ["  ;\n", 1, None]])
        t = self.put("a.c")
        status, out, _ = self.run_main(t)
        self.assertEqual(status, 0, out)
        self.assertIn("stillborn (not counted): 1", out)
        self.assertIn(self.mid("a.c", 4, 12, "relop", "<-><="), out)
        self.assertIn("3 killed, 0 equivalent (reviewed), 0 safe-direction, 0 unexplained survivors, 1 stillborn", out)

    def test_a_stillborn_entry_in_the_list_is_stale(self):
        self.set_rules([["x <= 5", 2, None], ["x < 6", 1, None], ["x < 4", 1, None],
                        ["  ;\n", 1, None]])
        t = self.put("a.c")
        self.equivalents.write_text("# reviewed: me\n" + self.mid("a.c", 4, 12, "relop", "<-><=")
                                    + "\twhy\n")
        status, out, _ = self.run_main(t)
        self.assertEqual(status, 1, out)
        self.assertIn("the mutant is stillborn", out)

    # -- failures of the tool ------------------------------------------------------

    def test_a_tool_failure_fails_the_run_with_status_three(self):
        for code in (3, 4, 127):
            with self.subTest(code):
                self.set_rules([["x <= 5", code, None]])
                t = self.put("a.c")
                status, out, _ = self.run_main(t)
                self.assertEqual(status, 3, out)
                self.assertIn("TOOL FAILURE", out)
                self.assertIn(self.mid("a.c", 4, 12, "relop", "<-><="), out)

    def test_a_crash_and_a_timeout_are_tool_failures(self):
        t = self.put("a.c")
        self.set_rules([["x <= 5", -9, None]])
        status, out, _ = self.run_main(t)
        self.assertEqual(status, 3, out)
        self.assertIn("crashed (signal 9)", out)
        self.set_rules([["x <= 5", 99, None]])
        with mock.patch.object(rt, "VERIFY_TIMEOUT_S", 1):
            status, out, _ = self.run_main(t)
        self.assertEqual(status, 3, out)
        self.assertIn("timed out after 1s", out)

    def test_a_tool_failure_wins_over_survivors(self):
        self.set_rules([["x <= 5", 3, None]])  # the others survive
        status, out, _ = self.run_main(self.put("a.c"))
        self.assertEqual(status, 3, out)
        self.assertIn("SURVIVED", out)

    # -- the original --------------------------------------------------------------

    def test_an_original_that_does_not_verify_fails_and_is_not_mutated(self):
        self.set_rules([["return x < 5", 1, None]])
        status, out, _ = self.run_main(self.put("a.c"))
        self.assertEqual(status, 1, out)
        self.assertIn("BASELINE ", out)
        self.assertIn("the original does not verify (exit status 1", out)
        self.assertEqual(len(self.calls()), 1)  # no mutant was run

    def test_a_rejected_original_fails(self):
        self.set_rules([["return x < 5", 2, None]])
        status, out, _ = self.run_main(self.put("a.c"))
        self.assertEqual(status, 1, out)
        self.assertIn("rejected (exit status 2)", out)

    def test_a_tool_failure_on_the_original_is_status_three(self):
        self.set_rules([["return x < 5", 3, None]])
        status, out, _ = self.run_main(self.put("a.c"))
        self.assertEqual(status, 3, out)
        self.assertIn("(the original)", out)

    def test_other_tests_are_still_run_when_one_original_fails(self):
        self.set_rules([["BAD", 1, None], ["x <= 5", 1, None]])
        self.put("bad.c", GOOD.replace("int f", "int BAD"))
        self.put("good.c")
        status, out, _ = self.run_main(self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn("bad.c [c23]: the original does not verify", out)
        self.assertTrue(any(c[1].endswith("good.c") for c in self.calls()[1:]))

    # -- modes ---------------------------------------------------------------------

    def two_modes(self):
        return self.put("m.c", GOOD.replace("verified\n", "verified\n// mode: c++20, c23\n"))

    def test_the_original_and_every_mutant_run_in_every_mode(self):
        self.set_rules([["x <= 5", 1, None], ["x < 6", 1, None], ["x < 4", 1, None],
                        ["  ;\n", 1, None]])
        status, out, _ = self.run_main(self.two_modes())
        self.assertEqual(status, 0, out)
        calls = self.calls()
        self.assertEqual(len(calls), 10)
        self.assertEqual(sum(c[2] == "--std=c++20" for c in calls), 5)
        self.assertEqual(sum(c[2] == "--std=c23" for c in calls), 5)

    def test_a_mutant_that_survives_in_any_mode_is_a_survivor(self):
        self.set_rules([["x <= 5", 1, "c23"], ["x < 6", 1, None], ["x < 4", 1, None],
                        ["  ;\n", 1, None]])
        status, out, _ = self.run_main(self.two_modes())
        self.assertEqual(status, 1, out)
        self.assertIn("SURVIVED " + self.mid("m.c", 5, 12, "relop", "<-><="), out)
        self.assertIn("3 killed", out)
        self.assertIn("1 unexplained survivors", out)

    def test_stillborn_in_one_mode_and_killed_in_the_other_is_killed(self):
        self.set_rules([["x <= 5", 2, "c++20"], ["x <= 5", 1, "c23"], ["x < 6", 1, None],
                        ["x < 4", 1, None], ["  ;\n", 1, None]])
        status, out, _ = self.run_main(self.two_modes())
        self.assertEqual(status, 0, out)
        self.assertIn("4 killed", out)
        self.assertIn("0 stillborn", out)

    def test_stillborn_in_every_mode_is_stillborn(self):
        self.set_rules([["x <= 5", 2, None], ["x < 6", 1, None], ["x < 4", 1, None],
                        ["  ;\n", 1, None]])
        status, out, _ = self.run_main(self.two_modes())
        self.assertEqual(status, 0, out)
        self.assertIn("1 stillborn", out)

    def test_survived_in_one_mode_and_stillborn_in_the_other_is_a_survivor(self):
        self.set_rules([["x <= 5", 2, "c23"], ["x < 6", 1, None], ["x < 4", 1, None],
                        ["  ;\n", 1, None]])
        status, out, _ = self.run_main(self.two_modes())
        self.assertEqual(status, 1, out)

    # -- selection -----------------------------------------------------------------

    def test_only_verified_c_and_cpp_tests_are_mutated(self):
        self.all_killed()
        self.put("v.cpp")
        self.put("e.c", "// expect: error check @ L4\nint f(int x) { return x < 5; }\n")
        self.put("r.c", "// expect: rejected undefined_identifier @ L4\nint f() { return 1 < 5; }\n")
        self.put("g.c", "// expect: golden\nint f(int x) { return x < 5; }\n")
        self.put("o.vpr", "// expect: verified\nmethod m() {}\n")
        self.put("bad.c", "no header\nint f(int x) { return x < 5; }\n")
        status, out, _ = self.run_main(self.dir)
        self.assertEqual(status, 0, out)
        self.assertIn("1 tests, 4 mutants", out)
        self.assertEqual({Path(c[1]).name for c in self.calls()}, {"v.cpp"})
        self.assertEqual({c[2] for c in self.calls()}, {"--std=c++20"})

    def test_a_directory_is_searched_recursively_and_ids_name_the_files(self):
        self.all_killed()
        self.put("a.c")
        self.put("sub/b.c")
        status, out, _ = self.run_main(self.dir, "-j", 3)
        self.assertEqual(status, 0, out)
        self.assertIn("2 tests, 8 mutants: 8 killed", out)

    def test_list_prints_the_ids(self):
        self.all_killed()
        t = self.put("a.c")
        status, out, _ = self.run_main(t, "--list")
        self.assertEqual(status, 0, out)
        for m in GOOD_MUTANTS:
            self.assertIn(f"{rt.rel(t)}:{m}\n", out)

    def test_list_does_not_start_a_run(self):
        # --list prints the ids and exits: the verifier is never called, not even for the originals
        self.all_killed()
        t = self.put("a.c")
        status, out, _ = self.run_main(t, "--list")
        self.assertEqual(status, 0, out)
        self.assertEqual(self.calls(), [])
        self.assertNotIn("mutants:", out)  # the run's summary line
        self.assertEqual(len([l for l in out.split("\n") if l.strip()]), len(GOOD_MUTANTS), out)

    def test_list_needs_no_verifier_environment(self):
        # a run loads the environment (and needs the verifier); --list does not
        self.put("a.c")
        with mock.patch.object(rt, "VERIFY_COMMAND", None), \
                mock.patch.object(rt, "load_env", side_effect=AssertionError("--list loaded the environment")):
            status, out, err = self.run_main(self.dir, "--list")
        self.assertEqual((status, err), (0, ""), out)

    def test_list_of_a_test_that_does_not_lex_is_a_failure_without_a_run(self):
        self.put("a.c", "// expect: verified\nint f(int x) { return (x < 5; }\n")
        status, out, err = self.run_main(self.dir, "--list")
        self.assertEqual(status, 1, out)
        self.assertIn("cannot lex", err)
        self.assertEqual(self.calls(), [])

    def test_nothing_to_mutate_is_a_usage_error(self):
        self.put("e.c", "// expect: error check @ L4\nint f(int x) { return x < 5; }\n")
        status, _, err = self.run_main(self.dir)
        self.assertEqual(status, 2)
        self.assertIn("no verified tests under", err)

    def test_a_missing_path_is_a_usage_error(self):
        status, _, err = self.run_main(self.dir / "nowhere")
        self.assertEqual(status, 2)
        self.assertIn("not a test file or directory", err)

    def test_a_test_that_does_not_lex_fails_the_run(self):
        self.put("a.c", "// expect: verified\nint f(int x) { return (x < 5; }\n")
        status, out, _ = self.run_main(self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn("cannot lex", out)
        self.assertEqual(self.calls(), [])


    def test_a_test_with_no_killed_mutant_fails(self):
        # all survive, reviewed or not
        self.set_rules([])  # nothing is killed
        t = self.put("a.c")
        survivors = [self.mid("a.c", *m) for m in (
            (4, 12, "relop", "<-><="), (4, 14, "literal", "5->4"), (4, 14, "literal", "5->6"),
            (4, 3, "delete-stmt", "return x < 5;->;"))]
        self.equivalents.write_text("# reviewed: me\n" + "".join(f"{x}\twhy\n" for x in survivors))
        status, out, _ = self.run_main(t)
        self.assertEqual(status, 1, out)
        self.assertIn("NO KILLED MUTANT", out)
        self.assertIn("of 4 mutants 4 survived and 0 were stillborn", out)

    def test_an_all_stillborn_test_fails(self):
        self.set_rules([["x <= 5", 2, None], ["x < 6", 2, None], ["x < 4", 2, None],
                        ["  ;\n", 2, None]])  # every mutant is rejected
        status, out, _ = self.run_main(self.put("a.c"))
        self.assertEqual(status, 1, out)
        self.assertIn("NO KILLED MUTANT", out)
        self.assertIn("of 4 mutants 0 survived and 4 were stillborn", out)

    def test_one_killed_mutant_is_enough(self):
        self.set_rules([["x <= 5", 1, None], ["x < 6", 2, None], ["x < 4", 2, None],
                        ["  ;\n", 2, None]])
        status, out, _ = self.run_main(self.put("a.c"))
        self.assertEqual(status, 0, out)
        self.assertNotIn("NO KILLED MUTANT", out)

    def test_each_test_is_judged_alone(self):
        stillborn = ["y <= 5", "y < 6", "y < 4", "y = 2", "y = 4", "  ;\n  return y", "y = 3;\n  ;"]
        self.set_rules([[t, 2, None] for t in stillborn]
                       + [["x <= 5", 1, None], ["x < 6", 1, None], ["x < 4", 1, None],
                          ["  ;\n", 1, None]])
        self.put("good.c")
        self.put("bad.c", "// expect: verified\nint g(int y)\n{\n  y = 3;\n  return y < 5;\n}\n")
        status, out, _ = self.run_main(self.dir)
        self.assertEqual(status, 1, out)
        line = out.split("NO KILLED MUTANT")[1].split("\n")[0]
        self.assertIn("bad.c", line)
        self.assertEqual(out.count("NO KILLED MUTANT"), 1)


class EnvironmentTests(RunCase):
    def test_the_shell_environment_does_not_reach_the_tests(self):
        # the cases below must give the same answer with `. build/env.sh` sourced or not
        leaked = [n for n in os.environ if n.startswith(("CERASTES_", "EDG_")) or n in ("SILICON_JAR", "Z3_EXE", "CPFE")]
        self.assertEqual(leaked, [])

    def test_a_missing_driver_is_an_environment_error(self):
        t = self.put("a.c")
        with mock.patch.object(rt, "VERIFY_COMMAND", None):
            status, _, err = self.run_main(t)
        self.assertEqual(status, 2)
        self.assertIn("CERASTES_VERIFY not set", err)
        self.assertEqual(self.calls(), [])

    def test_a_stale_build_is_an_environment_error(self):
        t = self.put("a.c")
        env_file = self.base / "env.sh"
        env_file.write_text(f'export CERASTES_VERIFY="{self.base / "fake_verify"}"\n')
        with mock.patch.object(rt, "VERIFY_COMMAND", None), \
                mock.patch.object(rt, "edg_tree_problem", return_value="mismatch"):
            status, _, err = self.run_main(t)
        self.assertEqual(status, 2)
        self.assertIn("mismatch", err)
        with mock.patch.object(rt, "VERIFY_COMMAND", None), \
                mock.patch.object(rt, "edg_tree_problem", return_value=None), \
                mock.patch.object(rt, "edg_stamp_problem", return_value="stale binary"):
            status, _, err = self.run_main(t)
        self.assertEqual(status, 2)
        self.assertIn("stale binary", err)
        self.assertEqual(self.calls(), [])

    def test_the_real_driver_comes_from_the_environment(self):
        self.set_rules([["x <= 5", 1, None], ["x < 6", 1, None], ["x < 4", 1, None],
                        ["  ;\n", 1, None]])
        env_file = self.base / "env.sh"
        env_file.write_text(f'export CERASTES_VERIFY="{self.base / "fake_verify"}"\n')
        with mock.patch.object(rt, "VERIFY_COMMAND", None), \
                mock.patch.object(rt, "edg_tree_problem", return_value=None), \
                mock.patch.object(rt, "edg_stamp_problem", return_value=None):
            status, out, _ = self.run_main(self.put("a.c"))
        self.assertEqual(status, 0, out)



class ClauseRelationTests(unittest.TestCase):
    """The comparison that decides the direction of a contract mutant."""

    def test_the_four_relations(self):
        rel = mu.clause_relation
        self.assertEqual(rel("x >= 0", "x > 0"), "stronger")
        self.assertEqual(rel("x >= 0", "x >= -1"), "weaker")
        self.assertEqual(rel("x >= 0 && x <= 100", "x >= 0 && x <= 99"), "stronger")
        self.assertEqual(rel("a >= 0 && b >= 0", "a >= 0"), "weaker")
        self.assertEqual(rel("r == 0 || r == 1", "r == 0"), "stronger")
        self.assertEqual(rel("x >= 0", "x >= 0 || x < 0"), "weaker")
        self.assertEqual(rel("x > 0", "x < 0"), "incomparable")
        self.assertEqual(rel("x >= 0", "0 <= x"), "equal")

    def test_division_and_negation_are_modelled(self):
        self.assertEqual(mu.clause_relation("b != 0 && a / b > 1", "b != 0 || a / b > 1"), "weaker")
        self.assertEqual(mu.clause_relation("a % 2 == 0", "a % 2 != 1"), "weaker")  # -3 % 2 == -1
        self.assertEqual(mu.clause_relation("!(x > 0)", "x <= 0 && x >= -5"), "stronger")

    def test_a_clause_that_is_not_modelled_is_unknown_never_safe(self):
        self.assertEqual(mu.clause_relation("x ? 1 : 0", "x"), "unknown")
        self.assertEqual(mu.clause_relation("sizeof(x) == 4", "sizeof(x) == 8"), "unknown")
        self.assertEqual(mu.clause_relation("p->x > 0", "p->x >= 0"), "unknown")

    def test_a_name_that_is_not_defined_cannot_make_a_mutant_safe(self):
        # a call is a construct the evaluator does not know
        self.assertEqual(mu.clause_relation("f(x) > 0", "f(x) >= 0"), "unknown")


class SafeDirectionTests(RunCase):
    """Decision (owner, 2026-10-16): a contract mutant that makes the specification easier to meet
    (stronger requires, weaker ensures or check) is its own counted bucket, not a survivor to
    kill; everything else, a mutant of code included, still has to be killed or listed."""

    def program(self, requires="x >= 0 && x <= 100", ensures="r >= 0", check="x >= 0", body="return x + 1;"):
        return ("// expect: verified\n"
                f"int f(int x) CERASTES_REQUIRES({requires}) CERASTES_ENSURES(r, {ensures})\n"
                "{\n"
                f"  CERASTES_CHECK({check});\n"
                f"  {body}\n"
                "}\n")

    def mutants_surviving(self, text, survive):
        """Run the program with every mutant killed except those whose id has a part in `survive`."""
        t = self.put("a.c", text)
        from_ids = [m.id for m in mu.mutants_of(text, mu.display_path(t))]
        rules = []
        # the fake verifier kills a mutant unless the mutated text is one of the allowed ones
        self.allowed = []
        for m in mu.mutants_of(text, mu.display_path(t)):
            if any(part in m.id for part in survive):
                self.allowed.append(m.apply(text))
        return t, from_ids

    def run_with_survivors(self, text, survive_ids):
        t = self.put("a.c", text)
        mutants = mu.mutants_of(text, mu.display_path(t))
        keep = {m.apply(text) for m in mutants if m.id.split(":", 1)[1] in survive_ids}
        self.assertEqual(len(keep), len(survive_ids), (sorted(survive_ids), [m.id for m in mutants]))
        # a rule per mutant that must be killed: its distinguishing text is the whole mutated file
        rules = [[m.apply(text), 1, None] for m in mutants if m.apply(text) not in keep]
        rules = [[r[0], 1, None] for r in rules]
        self.set_rules(rules)
        return self.run_main(t)

    def pos(self, text, needle, line=None):
        """The `line:col:op:orig->new` suffix of the first mutant (in source order) whose id contains
        `needle`, on `line` if given."""
        ids = [m.id.split(":", 1)[1] for m in mu.mutants_of(text, "x.c")
               if needle in m.id and (line is None or m.line == line)]
        assert ids, (needle, line)
        return ids[0]

    def test_a_stronger_requires_that_survives_is_in_the_bucket(self):
        text = self.program()
        survivor = self.pos(text, "relop:>=->>")  # x >= 0 -> x > 0 in the requires
        status, out, _ = self.run_with_survivors(text, {survivor})
        self.assertEqual(status, 0, out)
        self.assertIn("1 safe-direction, 0 unexplained survivors", out)
        self.assertIn("safe-direction ", out)
        self.assertNotIn("SURVIVED", out)

    def test_it_is_reported_per_test_and_in_total(self):
        text = self.program()
        a = self.pos(text, "relop:>=->>")
        b = self.pos(text, "literal:100->99")
        status, out, _ = self.run_with_survivors(text, {a, b})
        self.assertEqual(status, 0, out)
        self.assertRegex(out, r"safe-direction \S*a\.c: 2\n")
        self.assertIn("2 safe-direction", out)

    def test_a_weaker_requires_that_survives_is_not(self):
        text = self.program()
        survivor = self.pos(text, "drop-conjunct:&& x <= 100->")
        status, out, _ = self.run_with_survivors(text, {survivor})
        self.assertEqual(status, 1, out)
        self.assertIn("SURVIVED " + mu.display_path(self.dir / "a.c") + ":" + survivor, out)
        self.assertIn("0 safe-direction, 1 unexplained survivors", out)

    def test_a_weaker_ensures_that_survives_is_in_the_bucket_a_stronger_is_not(self):
        text = self.program(ensures="r >= 0 && r <= 1000")
        weaker = self.pos(text, "drop-conjunct:&& r <= 1000->")
        status, out, _ = self.run_with_survivors(text, {weaker})
        self.assertEqual(status, 0, out)
        self.assertIn("1 safe-direction", out)
        stronger = self.pos(text, "literal:1000->999")
        status, out, _ = self.run_with_survivors(text, {stronger})
        self.assertEqual(status, 1, out)
        self.assertIn("0 safe-direction, 1 unexplained survivors", out)

    def test_a_weaker_check_that_survives_is_in_the_bucket_a_stronger_is_not(self):
        text = self.program(check="x >= 0 && x <= 100")
        weaker = self.pos(text, "literal:100->101", line=4)
        status, out, _ = self.run_with_survivors(text, {weaker})
        self.assertEqual(status, 0, out)
        self.assertIn("1 safe-direction", out)
        stronger = self.pos(text, "literal:100->99", line=4)
        status, out, _ = self.run_with_survivors(text, {stronger})
        self.assertEqual(status, 1, out)

    def test_a_mutant_of_code_is_never_in_the_bucket(self):
        text = self.program(body="return x < 5;")
        survivor = self.pos(text, "relop:<-><=")
        status, out, _ = self.run_with_survivors(text, {survivor})
        self.assertEqual(status, 1, out)
        self.assertIn("0 safe-direction, 1 unexplained survivors", out)

    def test_an_invariant_or_decreases_mutant_is_never_in_the_bucket(self):
        text = ("// expect: verified\n"
                "int f(int n) CERASTES_REQUIRES(n >= 0 && n <= 100)\n{\n  int i = 0;\n"
                "  CERASTES_INVARIANT(0 <= i && i <= n)\n  CERASTES_DECREASES(n - i)\n"
                "  while (i < n) {\n    i += 1;\n  }\n  return i;\n}\n")
        # line 5 is the invariant, line 6 the measure `n - i - 0 + 1`
        text = text.replace("CERASTES_DECREASES(n - i)", "CERASTES_DECREASES(n - i + 1)")
        for needle, line in (("relop:<=-><", 5), ("literal:0->1", 5), ("literal:1->2", 6)):
            ids = [m.id.split(":", 1)[1] for m in mu.mutants_of(text, "x.c") if needle in m.id and m.line == line]
            self.assertTrue(ids, (needle, line))
            with self.subTest(needle, line=line):
                status, out, _ = self.run_with_survivors(text, {ids[0]})
                self.assertEqual(status, 1, out)
                self.assertIn("0 safe-direction, 1 unexplained survivors", out)

    def test_a_mutant_that_does_not_change_the_meaning_is_not_in_the_bucket(self):
        text = self.program(requires="x >= 0 && x <= 100 && x < 1000")  # `x < 1000` is implied
        survivor = self.pos(text, "literal:1000->1001")
        status, out, _ = self.run_with_survivors(text, {survivor})
        self.assertEqual(status, 1, out)

    def test_a_stale_entry_for_a_bucketed_mutant_fails_the_run(self):
        text = self.program()
        survivor = self.pos(text, "relop:>=->>")
        self.equivalents.write_text("# reviewed: me\n" + mu.display_path(self.dir / "a.c") + ":" + survivor
                                    + "\tbecause\n")
        status, out, _ = self.run_with_survivors(text, {survivor})
        self.assertEqual(status, 1, out)
        self.assertIn("the safe-direction bucket covers this mutant", out)

    def test_validate_also_finds_the_stale_entry(self):
        text = self.program()
        self.put("a.c", text)
        survivor = self.pos(text, "relop:>=->>")
        self.equivalents.write_text("# reviewed: me\n" + mu.display_path(self.dir / "a.c") + ":" + survivor
                                    + "\tbecause\n")
        status, out, _ = self.run_main("--validate", self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn("the safe-direction bucket covers this mutant", out)

    def test_a_safe_direction_mutant_that_is_killed_is_killed(self):
        text = self.program()
        t = self.put("a.c", text)
        mutants = mu.mutants_of(text, mu.display_path(t))
        self.set_rules([[m.apply(text), 1, None] for m in mutants])
        status, out, _ = self.run_main(t)
        self.assertEqual(status, 0, out)
        self.assertIn("0 safe-direction", out)
        self.assertNotIn("safe-direction ", out.replace("0 safe-direction", ""))


class SafeDirectionModelTests(unittest.TestCase):
    """The safe-direction bucket only trusts the grid where it models C: every variable of the clause
    (original and mutant) a signed builtin integer parameter of the enclosing function, a linear
    clause, no `==`/`!=` flip, and for a stronger `requires` no use of the function in the file.
    Each rule has a twin that is bucketed, and a case that is not: removing a rule makes one fail."""

    def safe(self, src, needle, line=None):
        found = [m for m in mu.mutants_of(src, "x.c")
                 if needle in m.id and (line is None or m.line == line)]
        self.assertTrue(found, (needle, [m.id for m in mu.mutants_of(src, "x.c")]))
        results = {mu.is_safe_direction(m, src) for m in found}
        self.assertEqual(len(results), 1, (needle, results))
        return results.pop()

    @staticmethod
    def req(decl, clause, body="return 0;"):
        return f"{decl} CERASTES_REQUIRES({clause})\n{{\n  {body}\n}}\n"

    @staticmethod
    def ens(decl, clause, body="return 0;"):
        return f"{decl} CERASTES_ENSURES(r, {clause})\n{{\n  {body}\n}}\n"

    # (a) the types
    def test_signed_builtin_parameters_are_modelled(self):
        for decl in ("int f(int x)", "int f(const int x)", "int f(signed x)", "int f(signed char x)",
                     "int f(short x)", "int f(long x)", "int f(long long x)", "int f(long int x)",
                     "int f(signed long long x)", "int f(int const x)"):
            with self.subTest(decl):
                self.assertTrue(self.safe(self.req(decl, "x >= 1"), "relop:>=->>"))

    def test_any_other_parameter_type_is_not_modelled(self):
        # the old classifier bucketed all of these: `x >= 1` -> `x > 1` is `stronger` on the grid
        self.assertEqual(mu.clause_relation("x >= 1", "x > 1"), "stronger")
        for decl in ("int f(unsigned x)", "int f(unsigned int x)", "int f(unsigned long x)",
                     "int f(unsigned char x)", "int f(unsigned long long x)", "int f(bool x)",
                     "int f(_Bool x)", "int f(char x)", "int f(size_t x)", "int f(int8_t x)",
                     "int f(int *x)", "int f(int &x)", "int f(float x)", "int f(long double x)",
                     "int f(short long x)", "int f(int int x)"):
            with self.subTest(decl):
                self.assertFalse(self.safe(self.req(decl, "x >= 1"), "relop:>=->>"))

    def test_the_unsigned_parameter_of_another_function_does_not_matter(self):
        src = (self.req("int g(unsigned a)", "a >= 1") + self.req("int f(int a)", "a >= 1"))
        self.assertFalse(self.safe(src, "relop:>=->>", line=1))
        self.assertTrue(self.safe(src, "relop:>=->>", line=5))

    def test_a_variable_that_is_no_parameter_is_not_modelled(self):
        src = self.req("int f(int x)", "x >= 1 && g_limit > x")  # a global
        self.assertFalse(self.safe(src, "relop:>=->>"))
        src = self.req("int f(int x)", "x >= 1 && x > y")  # y is unknown
        self.assertFalse(self.safe(src, "relop:>=->>"))

    def test_one_unmodelled_parameter_in_the_clause_is_enough(self):
        src = self.req("int f(int x, unsigned y)", "x >= 1 && y >= 1")
        self.assertFalse(self.safe(src, "relop:>=->>", line=1))  # x's flip: y is in the clause
        src = self.req("int f(int x, int y)", "x >= 1 && y >= 1")
        self.assertTrue(self.safe(src, "relop:>=->>", line=1))

    def test_the_ensures_result_counts_only_for_a_signed_return_type(self):
        for ret, expected in (("int", True), ("long long", True), ("signed char", True), ("unsigned", False),
                              ("unsigned long", False), ("bool", False), ("char", False), ("size_t", False),
                              ("int *", False)):
            with self.subTest(ret):
                src = self.ens(f"{ret} f(int x)", "r <= 5")
                self.assertEqual(self.safe(src, "literal:5->6"), expected)

    def test_an_ensures_over_a_signed_parameter_and_an_unsigned_result_is_not_modelled(self):
        # `r` is in the clause; the parameter alone would be fine
        self.assertFalse(self.safe(self.ens("unsigned f(int x)", "r <= x + 5"), "literal:5->6"))
        self.assertTrue(self.safe(self.ens("unsigned f(int x)", "x <= 5"), "literal:5->6"))

    def test_a_check_over_a_local_or_a_shadowed_parameter_is_not_modelled(self):
        src = "int f(int x)\n{\n  int i = 0;\n  CERASTES_CHECK(i <= 5 && x >= 0);\n  return x;\n}\n"
        self.assertFalse(self.safe(src, "literal:5->6"))  # the local `i`
        src = "int f(int x)\n{\n  CERASTES_CHECK(x <= 5);\n  return x;\n}\n"
        self.assertTrue(self.safe(src, "literal:5->6"))
        src = ("int f(int x)\n{\n  { unsigned x = 3; (void)x; }\n  CERASTES_CHECK(x <= 5);\n"
               "  return x;\n}\n")
        self.assertFalse(self.safe(src, "literal:5->6"))  # a block-local `unsigned x` of the same name

    def test_a_contract_of_a_bodyless_declaration_is_not_modelled(self):
        src = "int f(int x) CERASTES_REQUIRES(x >= 1);\nint g(void) { return 0; }\n"
        # no body, so no mutant is generated in it, but the macro argument is a target all the same
        found = [m for m in mu.mutants_of(src, "x.c") if "relop" in m.id]
        self.assertEqual(len(found), 1)
        self.assertFalse(mu.is_safe_direction(found[0], src))

    # (b) linearity
    def test_a_product_of_two_variables_is_not_modelled(self):
        for clause in ("x * y >= 2", "x * x >= 4", "x * (y + 1) >= 2", "x * 2 * y >= 2", "2 * x * y >= 2"):
            with self.subTest(clause):
                # the grid alone would bucket it (`>=` to `>` is stronger): only linearity rejects it
                self.assertEqual(mu.clause_relation(clause, clause.replace(">=", ">")), "stronger")
                self.assertFalse(self.safe(self.req("int f(int x, int y)", clause), "relop:>=->>"))

    def test_a_product_with_a_literal_is_linear(self):
        for clause in ("2 * x >= 2", "x * 2 >= 2", "3 * 4 + x >= 13", "x - 2 * y >= 1"):
            with self.subTest(clause):
                self.assertTrue(self.safe(self.req("int f(int x, int y)", clause), "relop:>=->>"))

    def test_division_remainder_and_shifts_are_not_modelled(self):
        for clause in ("x / 2 >= 1", "x % 2 >= 1", "x << 1 >= 1", "x >> 1 >= 1"):
            with self.subTest(clause):
                self.assertFalse(self.safe(self.req("int f(int x)", clause), "relop:>=->>"))
        # the grid would have called the first two `stronger`
        self.assertEqual(mu.clause_relation("x / 2 >= 1", "x / 2 > 1"), "stronger")
        self.assertEqual(mu.clause_relation("x % 3 >= 1", "x % 3 > 1"), "stronger")

    def test_bit_operations_conditionals_casts_calls_and_subscripts_are_not_modelled(self):
        for clause in ("(x & 1) >= 1", "(x | 1) >= 1", "(x ^ 1) >= 1", "~x >= 1", "(x > 0 ? 1 : 2) >= 1",
                       "(long)x >= 1", "(unsigned)x >= 1", "abs(x) >= 1", "x[0] >= 1", "x++ >= 1"):
            with self.subTest(clause):
                self.assertFalse(self.safe(self.req("int f(int x)", clause), "relop:>=->>"))

    def test_an_unsigned_or_hex_literal_is_not_modelled(self):
        for clause in ("x >= 1u", "x >= 1U", "x >= 1ul", "x >= 0x1", "x >= 1.0"):
            with self.subTest(clause):
                self.assertFalse(self.safe(self.req("int f(int x)", clause), "relop:>=->>"))
        self.assertTrue(self.safe(self.req("int f(int x)", "x >= 1L"), "relop:>=->>"))  # signed suffix

    # (c) equality flips
    def test_an_equality_flip_is_never_bucketed(self):
        src = self.ens("int f(int x)", "r == 1 && r == 2")  # unsatisfiable: the flips are `weaker`
        flips = [m for m in mu.mutants_of(src, "x.c") if m.op == "relop" and m.original in ("==", "!=")]
        self.assertEqual(len(flips), 2)
        # the grid says `weaker` for the flip: the old classifier bucketed it
        self.assertEqual(mu.clause_relation("r == 1 && r == 2", "r != 1 && r == 2"), "weaker")
        for m in flips:
            self.assertFalse(mu.is_safe_direction(m, src), m.id)

    def test_other_mutants_of_a_clause_with_equality_are_still_bucketed(self):
        src = self.ens("int f(int x)", "r < 5 || r == 2")
        self.assertTrue(self.safe(src, "relop:<-><="))  # weaker
        self.assertTrue(self.safe(self.ens("int f(int x)", "r >= 0 && r == 2"), "drop-conjunct:&& r == 2->"))

    # (d) callers
    def test_a_stronger_requires_with_a_caller_in_the_file_is_not_bucketed(self):
        base = self.req("int f(int x)", "x >= 1")
        self.assertTrue(self.safe(base, "relop:>=->>"))
        for caller in ("int g(int y) { return f(y); }\n",  # a call
                       "int g(int y) { int (*p)(int) = f; return p(y); }\n",  # the address
                       "int g(int y) CERASTES_ENSURES(r, r == f(y)) { return y; }\n"):  # a contract
            with self.subTest(caller):
                self.assertFalse(self.safe(base + caller, "relop:>=->>", line=1))

    def test_a_recursive_call_is_a_caller(self):
        src = self.req("int f(int x)", "x >= 1", body="return x > 1 ? f(x - 1) : 1;")
        self.assertFalse(self.safe(src, "relop:>=->>", line=1))

    def test_a_caller_of_another_function_does_not_matter(self):
        src = (self.req("int f(int x)", "x >= 1") + self.req("int ff(int x)", "x >= 1")
               + "int g(int y) { return ff(y); }\n")
        self.assertTrue(self.safe(src, "relop:>=->>", line=1))  # f has none (ff is another name)
        self.assertFalse(self.safe(src, "relop:>=->>", line=5))  # ff has one

    def test_a_name_in_a_comment_or_a_string_is_no_caller(self):
        src = (self.req("int f(int x)", "x >= 1") + "// f(3)\n/* f(4) */\nint g(int y) { return 0; }\n"
               + 'const char *s = "f(5)";\n')
        self.assertTrue(self.safe(src, "relop:>=->>", line=1))

    def test_a_weaker_ensures_with_a_caller_in_the_file_is_not_bucketed(self):
        # a caller may rely on the postcondition that the mutant weakens
        base = self.ens("int f(int x)", "r >= 0 && r <= 5")
        self.assertTrue(self.safe(base, "drop-conjunct:&& r <= 5->", line=1))  # no caller: bucketed
        for caller in ("int g(int y) { return f(y); }\n",
                       "int g(int y) { int (*p)(int) = f; return p(y); }\n",
                       "int g(int y) CERASTES_ENSURES(r, r == f(y)) { return y; }\n"):
            with self.subTest(caller):
                self.assertFalse(self.safe(base + caller, "drop-conjunct:&& r <= 5->", line=1))
                self.assertFalse(self.safe(base + caller, "literal:5->6", line=1))

    def test_a_caller_does_not_matter_for_a_weaker_check(self):
        # a `check` has no callers: rule (d) is about `requires` and `ensures`
        src = ("int f(int x)\n{\n  CERASTES_CHECK(x >= 0 && x <= 5);\n  return x;\n}\n"
               "int g(int y) { return f(y); }\n")
        self.assertTrue(self.safe(src, "literal:5->6", line=3))


class AssertDeletionTests(RunCase):
    """Decision 0015: deleting an `assert(e);` only removes a check, so such a mutant that survives
    is in the safe-direction bucket (a correct program a sound verifier must accept).  Every other
    deleted statement, a call of `abort` included, is still killed or listed."""

    SRC = ("// expect: verified\n"
           "int f(int x)\n"
           "{\n"
           "  assert(x > 0);\n"
           "  x = x + 1;\n"
           "  return x;\n"
           "}\n")

    def safe(self, src, needle):
        found = [m for m in mu.mutants_of(src, "x.c") if "delete-stmt" in m.id and needle in m.id]
        self.assertEqual(len(found), 1, (needle, [m.id for m in mu.mutants_of(src, "x.c")]))
        return mu.is_safe_direction(found[0], src)

    def test_a_deleted_assert_is_safe_direction(self):
        self.assertTrue(self.safe(self.SRC, "assert(x > 0);"))

    def test_an_assert_with_a_message_or_nested_parentheses_is_safe_direction(self):
        for stmt in ('assert(x > 0 && "message (with parentheses)");', "assert((x > 0) == (x > 1));",
                     "assert (x > 0);", "assert(x >= 0 && x != 3 && x <= 100);"):
            with self.subTest(stmt):
                src = self.SRC.replace("assert(x > 0);", stmt)
                self.assertTrue(self.safe(src, "assert"), stmt)

    def test_other_deleted_statements_are_not_safe_direction(self):
        for stmt in ("x = x + 1;", "abort();", "exit(1);", "fail();", "return x;", "x++;"):
            with self.subTest(stmt):
                src = self.SRC.replace("assert(x > 0);", stmt)
                found = [m for m in mu.mutants_of(src, "x.c") if m.op == "delete-stmt"]
                self.assertTrue(found)
                for m in found:
                    self.assertFalse(mu.is_safe_direction(m, src), m.id)

    def test_a_statement_that_only_starts_with_an_assert_is_not_safe_direction(self):
        # the second operand of the comma has an effect that the deletion would also remove
        for stmt in ("assert(x > 0), x++;", "assert(x > 0) + 1;", "my_assert(x > 0);",
                     "assert_that(x > 0);", "(assert(x > 0));", "assert;", "assert x;"):
            with self.subTest(stmt):
                src = self.SRC.replace("assert(x > 0);", stmt)
                found = [m for m in mu.mutants_of(src, "x.c") if m.op == "delete-stmt"
                         and m.original.startswith(stmt[:6])]
                for m in found:
                    self.assertFalse(mu.is_safe_direction(m, src), m.id)

    def test_an_assert_with_a_call_or_an_effect_is_not_safe_direction(self):
        # `checked(x)` may end the program (exit) unless x > 0: deleting the assert loses what
        # follows from it, and the mutant can fail to verify: it is not the safe direction
        for stmt in ("assert(g(x, (1)));", "assert(checked(x) == 1);", "assert(x++ > 0);",
                     "assert(--x > 0);", "assert((x = 1) > 0);", "assert((x += 1) > 0);",
                     "assert((x, 1));", "assert(x <<= 1);", "assert(sizeof(x) > 0);"):
            with self.subTest(stmt):
                src = self.SRC.replace("assert(x > 0);", stmt)
                (m,) = [m for m in mu.mutants_of(src, "x.c")
                        if m.op == "delete-stmt" and m.original.startswith(stmt[:12])]
                self.assertFalse(mu.is_safe_direction(m, src), stmt)

    def test_comparison_operators_with_an_equals_sign_are_not_assignments(self):
        for stmt in ("assert(x == 1);", "assert(x != 1);", "assert(x <= 1);", "assert(x >= 1);"):
            with self.subTest(stmt):
                src = self.SRC.replace("assert(x > 0);", stmt)
                (m,) = [m for m in mu.mutants_of(src, "x.c")
                        if m.op == "delete-stmt" and m.original == stmt]
                self.assertTrue(mu.is_safe_direction(m, src), stmt)

    def test_a_deleted_goto_is_not_safe_direction(self):
        src = "int f(int x)\n{\n  if (x) goto out;\n  x = 1;\n  out: return x;\n}\n"
        (m,) = [m for m in mu.mutants_of(src, "x.c") if m.original == "goto out;"]
        self.assertFalse(mu.is_safe_direction(m, src))

    def run_with_survivors(self, text, survive):
        t = self.put("a.c", text)
        mutants = mu.mutants_of(text, mu.display_path(t))
        keep = {m.apply(text) for m in mutants if any(part in m.id for part in survive)}
        self.set_rules([[m.apply(text), 1, None] for m in mutants if m.apply(text) not in keep])
        return self.run_main(t)

    def test_a_surviving_deleted_assert_is_counted_in_the_bucket_not_unexplained(self):
        status, out, _ = self.run_with_survivors(self.SRC, ["delete-stmt:assert(x > 0);->;"])
        self.assertEqual(status, 0, out)
        self.assertIn("1 safe-direction, 0 unexplained survivors", out)
        self.assertNotIn("SURVIVED", out)

    def test_a_surviving_deleted_abort_is_an_unexplained_survivor(self):
        src = self.SRC.replace("assert(x > 0);", "abort();")
        status, out, _ = self.run_with_survivors(src, ["delete-stmt:abort();->;"])
        self.assertEqual(status, 1, out)
        self.assertIn("SURVIVED", out)
        self.assertIn("0 safe-direction", out)

    def test_a_killed_deleted_assert_is_killed(self):
        status, out, _ = self.run_with_survivors(self.SRC, [])
        self.assertEqual(status, 0, out)
        self.assertIn("0 safe-direction, 0 unexplained survivors", out)


class ZeroTargetTests(RunCase):
    """A test or a function with nothing to mutate is a failure (item 13)."""

    def test_a_function_without_a_target_fails_validation(self):
        t = self.put("a.c", "// expect: verified\nint f(int x)\n{\n  return x < 5;\n}\n"
                            "void empty(void)\n{\n}\n")
        status, out, _ = self.run_main("--validate", t)
        self.assertEqual(status, 1, out)
        self.assertIn("the body of `empty` has no mutation target", out)
        self.assertNotIn("the body of `f`", out)

    def test_a_test_without_a_function_fails_validation(self):
        t = self.put("a.c", "// expect: verified\nint x = 1 < 2;\nstruct s { int a; };\n")
        status, out, _ = self.run_main("--validate", t)
        self.assertEqual(status, 1, out)
        self.assertIn("no function body found", out)

    def test_a_test_that_yields_no_mutant_is_named_even_when_others_do(self):
        # the total is not zero (a.c has mutants), so only a per-test check notices b.c
        self.put("a.c")
        self.put("b.c", "// expect: verified\nvoid empty(void)\n{\n}\n")
        status, out, _ = self.run_main("--validate", self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn(f"{rt.rel(self.dir / 'b.c')}: no mutant is generated for this test", out)
        self.assertNotIn(f"{rt.rel(self.dir / 'a.c')}: no mutant", out)
        # and the full run refuses it before it runs the verifier
        status, out, _ = self.run_main(self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn("no mutant is generated for this test", out)
        self.assertEqual(self.calls(), [])

    def test_the_full_run_refuses_it_too(self):
        t = self.put("a.c", "// expect: verified\nint f(int x)\n{\n  return x < 5;\n}\n"
                            "void empty(void)\n{\n}\n")
        status, out, _ = self.run_main(t)
        self.assertEqual(status, 1, out)
        self.assertEqual(self.calls(), [])

    def test_a_function_with_mutants_in_every_body_passes(self):
        t = self.put("a.c", "// expect: verified\nint f(int x) noexcept\n{\n  return x < 5;\n}\n"
                            "auto g(int x) -> int\n{\n  return x + 1;\n}\n")
        status, out, _ = self.run_main("--validate", t)
        self.assertEqual(status, 0, out)


class ValidateTests(RunCase):
    def test_validate_generates_and_never_runs_the_verifier(self):
        t = self.put("a.c")
        with mock.patch.object(rt, "VERIFY_COMMAND", None):  # no driver needed
            status, out, _ = self.run_main("--validate", t)
        self.assertEqual(status, 0, out)
        self.assertIn("1 tests, 4 mutants generated, 0 reviewed equivalents", out)
        self.assertEqual(self.calls(), [])

    def test_validate_checks_the_equivalence_list(self):
        t = self.put("a.c")
        self.equivalents.write_text("# reviewed: me\n" + self.mid("a.c", 4, 12, "relop", "<-><=")
                                    + "\twhy\n")
        status, out, _ = self.run_main("--validate", t)
        self.assertEqual(status, 0, out)
        self.assertIn("1 reviewed equivalents", out)
        self.equivalents.write_text("# reviewed: me\n" + self.mid("a.c", 9, 9, "relop", "<-><=")
                                    + "\twhy\n")
        status, out, _ = self.run_main("--validate", t)
        self.assertEqual(status, 1, out)
        self.assertIn("no such mutant is generated", out)
        self.equivalents.write_text("garbage\n")
        status, out, _ = self.run_main("--validate", t)
        self.assertEqual(status, 1, out)

    def test_validate_fails_on_a_test_that_does_not_lex(self):
        self.put("a.c", "// expect: verified\nint f() { return 1 < (2; }\n")
        status, out, _ = self.run_main("--validate", self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn("cannot lex", out)

    def test_validate_fails_when_nothing_is_generated(self):
        self.put("a.c", "// expect: verified\n")
        status, out, _ = self.run_main("--validate", self.dir)
        self.assertEqual(status, 1, out)
        self.assertIn("an empty set must not pass", out)

    def test_validate_fails_on_a_mutant_equal_to_its_source(self):
        t = self.put("a.c")
        real = mu.mutants_of

        def identical(src, path):
            return [mu.Mutant(path, 1, 1, "relop", "<", "<", 0, 0, "")]
        with mock.patch.object(mu, "mutants_of", identical):
            status, out, _ = self.run_main("--validate", t)
        self.assertEqual(status, 1, out)
        self.assertIn("identical to the original", out)
        self.assertTrue(callable(real))

    def test_validate_fails_on_a_generated_id_twice(self):
        t = self.put("a.c")

        def twice(src, path):
            m = mu.Mutant(path, 1, 1, "relop", "<", "<=", 0, 0, "x")
            return [m, m]
        with mock.patch.object(mu, "mutants_of", twice):
            status, out, _ = self.run_main("--validate", t)
        self.assertEqual(status, 1, out)
        self.assertIn("generated twice", out)

    def test_validate_with_no_verified_tests_passes_only_while_the_tree_has_none(self):
        empty = self.base / "tree"
        (empty / "tests").mkdir(parents=True)
        (empty / "doc/modules/ROOT/examples").mkdir(parents=True)
        with mock.patch.object(mu, "ROOT", empty), mock.patch.object(rt, "ROOT", empty):
            status, out, _ = self.run_main("--validate", equivalents=False)
            self.assertEqual(status, 0, out)
            self.assertIn("no verified tests to mutate yet", out)
            (empty / "tests/scalar").mkdir()
            (empty / "tests/scalar/a.c").write_text(GOOD.replace(
                "verified\n", "verified\n// expect: native-ok\n// mode: c23, c++20\n"))
            (empty / "tests/other").mkdir()
            status, out, _ = self.run_main("--validate", empty / "tests/other", equivalents=False)
            self.assertEqual(status, 1, out)  # a verified test exists; the selection is empty
            status, out, _ = self.run_main("--validate", equivalents=False)
            self.assertEqual(status, 0, out)  # the default path now finds it
            self.assertIn("mutants generated", out)

    def test_validate_with_an_explicit_missing_path_is_a_usage_error(self):
        status, _, err = self.run_main("--validate", self.dir / "nowhere")
        self.assertEqual(status, 2)

    def test_the_default_path_is_tests_scalar(self):
        self.assertEqual(mu.DEFAULT_PATHS, ("tests/scalar",))
        self.assertEqual(mu.EQUIVALENT_FILE, Path("tests") / "mutants-equivalent.txt")

    def test_the_real_tree_validates(self):
        # the check.sh step: whatever tests/scalar holds generates well-formed mutants
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            status = mu.main(["--validate"])
        self.assertEqual(status, 0, out.getvalue() + err.getvalue())


class JobsTests(RunCase):
    """The default `-j` is CERASTES_JOBS, else half the CPUs; an explicit -j wins (decision 0016)."""

    def jobs_of_a_run(self, *args, variable=None, cpus=16):
        """The `jobs` that main() hands to run_all, or the exit status of a run that stopped early."""
        self.set_rules([["x <= 5", 1, None], ["x < 6", 1, None], ["x < 4", 1, None],
                        ["  ;\n", 1, None]])
        t = self.put("a.c")
        seen = []
        real = mu.run_all

        def spy(subjects, env, jobs, workdir):
            seen.append(jobs)
            return real(subjects, env, jobs, workdir)
        environ = {} if variable is None else {"CERASTES_JOBS": variable}
        # set after the scrub of CERASTES_* in setUp (that is what makes the first case "unset")
        with mock.patch.dict(os.environ, environ), mock.patch.object(mu, "run_all", spy), \
                mock.patch.object(rt.os, "cpu_count", return_value=cpus):
            status, out, err = self.run_main(*args, t)
        return status, seen, out, err

    def test_unset_is_half_the_cpus(self):
        status, seen, out, _ = self.jobs_of_a_run(cpus=16)
        self.assertEqual((status, seen), (0, [8]), out)
        status, seen, out, _ = self.jobs_of_a_run(cpus=3)
        self.assertEqual((status, seen), (0, [1]), out)

    def test_the_variable_is_the_default(self):
        for value in ("3", "5", "10"):
            with self.subTest(value):
                status, seen, out, _ = self.jobs_of_a_run(variable=value, cpus=16)
                self.assertEqual((status, seen), (0, [int(value)]), out)

    def test_an_explicit_j_wins(self):
        for variable in ("3", None, "x", "0"):
            for flag in ("-j", "--jobs"):
                with self.subTest(variable=variable, flag=flag):
                    status, seen, out, _ = self.jobs_of_a_run(flag, "2", variable=variable)
                    self.assertEqual((status, seen), (0, [2]), out)

    def test_an_invalid_value_is_an_environment_error_that_runs_nothing(self):
        for value in ("", "0", "-1", "x", " 3", "3 ", "03", "3\n"):
            with self.subTest(value=value):
                status, seen, _, err = self.jobs_of_a_run(variable=value)
                self.assertEqual((status, seen), (2, []))
                self.assertIn("CERASTES_JOBS", err)
                self.assertEqual(self.calls(), [])

    def test_validate_and_list_do_not_need_the_variable(self):
        t = self.put("a.c")
        with mock.patch.dict(os.environ, {"CERASTES_JOBS": "x"}):
            for flags in (["--validate"], ["--list"]):
                status, out, err = self.run_main(*flags, t)
                self.assertEqual(status, 0, out + err)

    def test_mutate_uses_the_harness_default(self):
        source = (rt.ROOT / "scripts" / "mutate.py").read_text()
        self.assertIn("args.jobs = rt.default_jobs()", source)
        self.assertNotIn("os.cpu_count", source)
        self.assertRegex(source, r'"-j", "--jobs", type=int, default=None')


if __name__ == "__main__":
    unittest.main()
