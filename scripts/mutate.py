#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Mutation testing of the must-verify tests (plan.md M3, decisions 0011, 0012).

    python3 scripts/mutate.py [--validate | --list] [-j N] [paths ...]     (default: tests/scalar)

Every `// expect: verified` test under the paths (C and C++, in each of its
modes) is mutated one place at a time, and each mutant is verified with
`build/cerastes verify <mutant> --std=<mode>`.  A mutant that still verifies
survived: the test does not notice the change, which is a hole in the test (or
a mutant that does not change the meaning: see the equivalence list below).

What is mutated.  A small C/C++ lexer finds the targets in function bodies and
in the arguments of `CERASTES_*` macros (and of `[[cerastes::...]]`
attributes), never in comments, strings, preprocessor lines or declarations:

* `relop`: a relational operator is flipped: `<` to `<=`, `<=` to `<`, `>` to
  `>=`, `>=` to `>`, `==` to `!=`, `!=` to `==`;
* `literal`: a decimal integer literal gets `+ 1` or `- 1` (a `0` only `+ 1`);
  its suffix stays (`10u` becomes `11u` and `9u`);
* `drop-conjunct`: one operand of an `&&` is dropped (`a && b` becomes `a`, and
  `b`);
* `swap-logic`: `&&` becomes `||` and `||` becomes `&&`;
* `delete-stmt`: an expression statement (an assignment, `++`, a call, a
  `return`, a `break;`, a `continue;`, a `goto L;`) is replaced by `;`.  A call of a
  function that does not return (`abort();`, `exit(0);`, a `[[noreturn]]` function) is a
  call like any other.  Declarations, checks
  (`CERASTES_*`), `static_assert` and compound statements are never deleted.  The
  statement after a `case <k>:` or `default:` label is deleted without its label (the label
  itself is never deleted, and neither is a `switch` or `do` head), and the `break;` or
  `continue;` that is the body of an `if (...)`, `while (...)` or `for (...)` is deleted
  (decision 0014 item 9), and so is the `goto L;` that is the body of one; a `goto` label
  `L:` is never deleted either (a dangling `goto` is no mutant of the program), only the
  statement after it.  Other bodies of those heads are not (that would change the ids
  of the tests of M3).
* `drop-store`, `swap-incdec` and `drop-comma` (side effects in expressions, M4.3a) are mutants of code
  only (never inside the arguments of `CERASTES_*` or `[[cerastes::...]]`, so never in the safe-direction
  bucket below), and only where the effect is part of an expression whose value is used:
  `drop-store` removes the store of an assignment used as a value (`(x = e)` becomes `(e)`, `y = x = a`
  becomes `y = a`, `f(x = e)` becomes `f(e)`; the id is `x =` -> empty), `swap-incdec` turns a used `x++`
  into `++x` and back (`x++` -> `++x`) and a used `++` into `--` and back (`++` -> `--`), and `drop-comma`
  drops the left operand of a comma operator whose left operand has an effect (an assignment, `++`, `--` or
  a call): `(a, b)` becomes `(b)`, `f(x), g(y);` becomes `g(y);`, and in a `for` init or step clause `i = 0,
  j = n` becomes `j = n` (the id is the dropped operand and its comma -> empty).  None is produced for the
  statements that discard the value: a whole expression statement (`x = 1;`, `i++;`), a whole `for` init or
  step clause or one of its comma-separated parts (`for (i = 0; ...; i++)`), or the operand of a cast to
  `void` (`(void)x++;`); the value-discarding comma operands of such a statement are skipped alike (`i = 0,
  j = n;`), while its comma is a `drop-comma` target.  A comma that is an argument separator (`f(a, b)`, also
  of a macro), the separator of declarators (`int i = 0, j = n;`, also in a `for` init) or of an
  initializer is never a target, and neither is an assignment that initializes a declarator.  An
  assignment's `x` is a plain identifier (not `p->x`, `a[i]`, `*p`); so is the operand of `++`/`--`.
* `swap-logical` and `swap-arms` (calls in expressions, M4.3b) are mutants of code only too.  `swap-logical`
  swaps the two operands of an `&&` or `||` when at least one of them contains a call (`a > 0 && pos(a) > 1`
  becomes `pos(a) > 1 && a > 0`): it pins that a call's guard is placed right.  The operands are the
  operands of the operator at its own precedence (in `a && b || c` the `&&` swaps `a` and `b`, the `||`
  swaps `a && b` and `c`), so a swap never changes how the other operators of the expression group.
  `swap-arms` swaps the two arms of a `?:` (`c ? a : b` becomes `c ? b : a`, the arms being the middle
  operand and the operand after the `:`, a nested conditional in the last arm included).  Neither is made
  for an arm or operand with an unparenthesized comma or assignment where that would change the grouping,
  for two equal operands, or inside the arguments of `CERASTES_*` and `[[cerastes::...]]`.

Mutant ids have the form `<path>:<line>:<column>:<op>:<original>-><new>`, with the
position of the first changed character and the changed text on one line (a
statement is shortened to 40 characters; `new` is empty for a dropped operand).

Outcomes, by the verifier's exit status (decision 0011): 1 killed; 0 survived;
2 stillborn (not accepted: listed, not counted); 3, any other status, a crash
or a timeout: a tool failure, which fails the run (exit status 3).  A mutant
that survives in any mode is a survivor; one that is killed in some mode and
survives in none is killed; one that is stillborn in every mode is stillborn.
The original must verify in every mode first, or the run fails.

The safe-direction bucket.  A mutant of a *contract* that makes the specification
easier to meet is a correct program, and a sound verifier must accept it: a
stronger `requires` (the body has less to prove; a caller in the file may fail it), a
weaker `ensures` (the body proves more than the clause says; a caller may rely on it) or
`check` (the body proves more than the clause says; nothing relies on a check).
Such a mutant that survives is counted in its own bucket, `safe-direction`, reported
per test and in total, and is not required to be killed and not listed.  The
direction is decided by comparing the original clause with the mutated one (the two
clauses are evaluated on a grid of values that holds every literal of either clause
and its neighbours, 0, +-1, 2 and 3, for every variable; `stronger` means the mutant
is true nowhere the original is false and false somewhere the original is true).  The
grid is plain mathematics on Python integers, so it is trusted only where it models C:
a mutant is bucketed *only when all of these hold*, and is `unknown` (never bucketed)
otherwise:

* every variable of the clause, original and mutant, is a parameter of the enclosing
  function (read from its signature) of a signed builtin integer type (`signed char`,
  `short`, `int`, `long`, `long long`, with an optional `signed` or `const`; not `bool`,
  plain `char`, anything unsigned, a typedef, a pointer or a reference); the result name
  of an `ensures` counts only if the return type is one; for a `check`, a name that the
  body declares again (a local that may shadow the parameter) does not count; a contract
  of a declaration without a body has no signature to read, so it is `unknown`;
* the clause is linear: only `+ - < <= > >= == != && || !` and parentheses, and decimal
  literals without a `u` suffix (no `*` at all, not even with a literal coefficient: `3 * x + y < 10`
  against `4 * x + y < 10` is incomparable, which a small grid may not show; no `/`, `%`, `<<`, `>>`,
  bit operations, `?:`, casts, calls or subscripts);
* Python and C read the clause alike (`c_semantics_ok`): at most one comparison between two `&&`/`||`
  or brackets (Python chains `a < b < c`, C does not; `r == x > 0` is `r == (x > 0)` in C), the
  operand of `!` is followed only by `&&`, `||`, `)` or the end (`!r < 1` is `(!r) < 1` in C), and a
  bracketed group with `&&` or `||` of its own is not an operand of a comparison or of arithmetic
  (Python's `and` yields an operand, C's `&&` yields 0 or 1);
* the mutation is not an `==` <-> `!=` flip (a flip of equality is `unknown`);
* for a stronger `requires` or a weaker `ensures` (not a `check`), the identifier of the
  function occurs nowhere else in the file (`has_caller`): a call, a recursive call, its
  address, a mention in a contract, in a `#define` body or other directive (not an `#include`
  line), in a default argument, in an initializer; only its own declarations and definitions
  (`declarator_indexes`: outside every body and bracket, directly after a type, followed by `(`, no `=`
  before it in the statement) are not uses, and neither are comments and strings.  A use means that
  the mutant must be killed by a failure at the use, or be listed (a caller may fail the stronger
  precondition, or rely on the weaker postcondition).  A directive that this lexer cannot read (an
  apostrophe in an `#error` line) makes every name used;

The bucket does not ask whether the mutated clause is well-defined, though a dropped conjunct, a swapped
`&&`/`||` or a relop or literal change that weakens a guard can leave arithmetic in a contract that overflows or
divides by zero (a mutant is a correct program only if its clause is well-defined).  It need not: a mutant that
survives has been accepted by cerastes, and cerastes checks the well-definedness of contract arithmetic itself
(decision 0010 item 11: contract arithmetic has its C meaning, overflow and division by zero included, checked
by the emitted `__wf` methods and the clause's own checks), so a surviving mutant whose clause is ill-defined
would be a bug of the verifier, not a hole in the test.  That check is pinned by must-fail tests in
tests/scalar/negative, each with the guarded original in tests/scalar/guard_kept.c as its must-verify twin:
`contract_wf_divzero.c` and `contract_wf_overflow.c` (twin `contract_wf.c`), and the tests that write a
contract exactly as a drop-conjunct or swap-logic mutant would leave it, one per clause kind:
`guard_dropped_requires.c` (a requires whose own guard is gone: division by zero), `guard_dropped_ensures_later.c`
(an ensures whose guard was in an earlier ensures that lost it: division by zero in the later one),
`guard_dropped_overflow.c` (an ensures with `r + 1` and the guard `r < 2147483647 &&` dropped: overflow),
`guard_dropped_ensures_later_overflow.c` (the same with an overflow in the later ensures),
`guard_swapped_requires_overflow.c` (`||` swapped to `&&` in a requires: `x + 1` evaluated for x == INT_MAX),
`guard_changed_invariant.c` (the guard of a loop invariant changed: division by zero at entry) and
`measure_loop_overflow_later.c` (a measure whose arithmetic overflows).  Invariants and measures are never
bucketed; they are pinned because they are clauses too.  A weaker `check` or `assert` never evaluates an operand
in a state where the original did not (it is judged in the state at its line, where what the original proved is
still known), so it cannot become ill-defined; `guard_swapped_check.c` and `guard_swapped_assert.c` (`||` swapped
to `&&`: division by zero) pin the same check for the stronger form that the bucket never accepts, as defence in
depth.  The argument rests on these tests: a verifier that stopped checking the well-definedness of contract
arithmetic would fail them, and the bucket would then hide the survivors.

A pair that cannot be evaluated (a construct the grid does not know), or whose grid is larger than
two million points, is `unknown` too.
`assert(e);` (the `<assert.h>` macro, decision 0015: a failing assert is a `check`) is a `check`
clause for the bucket: a mutant inside the argument of an `assert(...);` statement is bucketed when the
mutated argument is weaker, under exactly the rules of a `check` (every variable a signed builtin integer
parameter of the enclosing function that the body does not declare again, linear, no `==`/`!=` flip)
and only when the argument is pure by the rule of `is_assert_statement` (no call, `++`, `--`,
assignment or comma).  A stronger argument is never bucketed; `static_assert` and an identifier that
merely contains `assert` are not the macro; the statement must start a statement (after `;`, `{`, `}`, `)`,
`:`, `else` or `do`).  A *deleted* `assert(...);` statement whose argument is pure is in the bucket too: the
program only loses a check, so it is a correct program that a sound verifier must accept, and when it
survives (the assertion was redundant) that is no hole in the test.  With a call the assert is not only a
check (the callee may end the program, and what the assert established helps what follows), so it is never
bucketed.  The other contract mutants, a weaker `requires`, a stronger `ensures` or `check`, any change of
an `invariant` or a `decreases`, one that leaves the clause's meaning unchanged, and every mutant of code,
are never bucketed: each must be killed or listed.

Progress.  A run writes one line to stderr, flushed, as soon as each verifier run finishes (in the
order they finish): `mutate: [k/N] <mutant id or "<path> (original)"> [<mode>]: <outcome>`.  k and N count
the originals (outcome `verified`, `does not verify`, `rejected` or `tool failure`) and the mutants (outcome
`killed`, `survived`, `stillborn` or `tool failure`) apart.  A run that a time cap stops therefore still
leaves its outcomes in a log; stdout is the report, unchanged, in a deterministic order.

`tests/mutants-equivalent.txt` lists the survivors that were reviewed and are
equivalent (or intended), apart from those of the safe-direction bucket: lines `id<TAB>reason`, `#` comments, and a
`# reviewed: <who, when>` line whenever there are entries.  An entry that names
a mutant which is not generated, which does not survive, or which the
safe-direction bucket already covers, is stale and fails the run (so a fixed test or an edited source cannot leave a stale excuse).
A run over some of the tests checks only the entries of the files it selected (the others are not
generated, so nothing is known about them); `--validate` also generates every file that the list names
outside the selection (`check.sh` selects `tests/scalar`; the list also names doc examples) and checks
all entries against them: an entry whose file does not exist, or is not a `// expect: verified` C or
C++ test, is stale too.  A run does not check the entries of files it did not select, and `--validate` cannot
tell whether an entry survives: the acceptance run must therefore name every file that the list names (the doc
examples among them), or those entries are never checked for survival.

`--list` generates, prints the id of every mutant (sorted) and exits 0 without running
anything: it needs no verifier and no environment (a source that cannot be generated is
reported on stderr and exits 1).  `--validate` only generates: it checks that the sources lex (balanced braces and
parentheses), that every mutant differs from its original, that the ids are
unique, and the equivalence list (format and staleness against the generated
ids).  It needs no verifier and runs in `scripts/check.sh` (a few seconds for tests/scalar: each
clause is compiled once and each source is read once); the full run (45 to 90 minutes at `-j 8` for
tests/scalar) is an acceptance step.  With no verified test under
the paths and none in the tree, `--validate` passes with a note (the first
tests come with M3); once the tree has one, an empty selection fails.

Exit status: 0 clean, 1 unexplained survivors, stale entries, a baseline that
does not verify or a validation failure, 2 usage or environment error, 3 tool
failure.
"""

from __future__ import annotations

import argparse
import bisect
import concurrent.futures
import dataclasses
import functools
import itertools
import os
import re
import shutil
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_tests as rt  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
EQUIVALENT_FILE = Path("tests") / "mutants-equivalent.txt"
DEFAULT_PATHS = ("tests/scalar",)
REVIEWED_RE = re.compile(r"^#\s*reviewed:\s*(?P<who>\S.*)$")
ID_RE = re.compile(r"^(?P<path>.+?):(?P<line>\d+):(?P<col>\d+):(?P<op>[a-z][a-z-]*):"
                   r"(?P<orig>.*)->(?P<new>.*)$")
OPS = ("relop", "literal", "drop-conjunct", "swap-logic", "delete-stmt", "drop-store", "swap-incdec",
       "drop-comma", "swap-logical", "swap-arms")
# the operators of side effects and calls in expressions: mutants of code only, never bucketed
EFFECT_OPS = ("drop-store", "swap-incdec", "drop-comma", "swap-logical", "swap-arms")
STATEMENT_ID_CHARS = 40


# ---------------------------------------------------------------------------
# The lexer
# ---------------------------------------------------------------------------

@dataclasses.dataclass(frozen=True)
class Token:
    kind: str  # ident, number, string, char, op
    text: str
    start: int
    end: int


class LexError(Exception):
    pass


PUNCTUATORS = sorted([
    "<<=", ">>=", "...", "->*", "<=>", "&&", "||", "<<", ">>", "<=", ">=", "==", "!=", "->",
    "++", "--", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "::", ".*", "##",
    "<", ">", "=", "+", "-", "*", "/", "%", "&", "|", "^", "~", "!", "?", ":", ";", ",", ".",
    "(", ")", "[", "]", "{", "}", "#"], key=len, reverse=True)
_PUNCT_RE = re.compile("|".join(re.escape(p) for p in PUNCTUATORS))
_IDENT_RE = re.compile(r"[A-Za-z_$][A-Za-z0-9_$]*")
_NUMBER_RE = re.compile(r"\.?[0-9](?:[eEpP][+-]|[A-Za-z0-9_.'])*")


_INCLUDE_RE = re.compile(r"#[ \t]*(?:include|include_next|import)\b")


def lex(src: str, preprocessor: bool = False) -> list[Token]:
    """Tokens of `src` without comments and preprocessor lines (a `#` line, with
    its backslash continuations, produces nothing: a macro body is never mutated).
    With `preprocessor=True` the tokens of the directive lines are produced too (not the
    `#` that starts one, and not an `#include` line): `has_caller` reads the identifiers of
    a `#define` body with it."""
    tokens: list[Token] = []
    i, n = 0, len(src)
    line_start = True  # only whitespace seen since the last newline
    while i < n:
        c = src[i]
        if c == "\n":
            line_start = True
            i += 1
        elif c in " \t\r\f\v":
            i += 1
        elif src.startswith("//", i):
            j = src.find("\n", i)
            while j > 0 and src[j - 1] == "\\":  # a continued line comment
                j = src.find("\n", j + 1)
            i = n if j < 0 else j
        elif src.startswith("/*", i):
            j = src.find("*/", i + 2)
            if j < 0:
                raise LexError("unterminated /* comment")
            i = j + 2
        elif preprocessor and c == "\\" and src.startswith("\\\n", i):
            i += 2  # a backslash-newline inside a directive
        elif preprocessor and c == "#" and line_start and not _INCLUDE_RE.match(src, i):
            i += 1  # the directive's own tokens are lexed like any others
            line_start = False
        elif c == "#" and line_start:
            while i < n:  # to the end of the line, following backslash-newlines
                j = src.find("\n", i)
                if j < 0:
                    i = n
                    break
                if src[j - 1] == "\\":
                    i = j + 1
                    continue
                i = j
                break
        elif c == '"' or c == "'":
            j = i + 1
            while j < n and src[j] != c:
                if src[j] == "\\":
                    j += 1
                if j < n and src[j] == "\n":
                    raise LexError("unterminated string or character literal")
                j += 1
            if j >= n:
                raise LexError("unterminated literal")
            tokens.append(Token("string" if c == '"' else "char", src[i:j + 1], i, j + 1))
            i = j + 1
            line_start = False
        elif c.isdigit() or (c == "." and i + 1 < n and src[i + 1].isdigit()):
            m = _NUMBER_RE.match(src, i)
            assert m
            tokens.append(Token("number", m.group(), i, m.end()))
            i = m.end()
            line_start = False
        elif (m := _IDENT_RE.match(src, i)):
            # a string prefix (u8"..", L'x') is part of the literal
            if m.end() < n and src[m.end()] in "\"'" and m.group() in ("u8", "u", "U", "L", "R"):
                if m.group() == "R" and src[m.end()] == '"':
                    raise LexError("raw string literals are not supported")
                i = m.end()
                continue
            tokens.append(Token("ident", m.group(), i, m.end()))
            i = m.end()
            line_start = False
        elif (m := _PUNCT_RE.match(src, i)):
            tokens.append(Token("op", m.group(), i, m.end()))
            i = m.end()
            line_start = False
        else:
            raise LexError(f"unexpected character {c!r}")
    return tokens


# ---------------------------------------------------------------------------
# Finding the targets
# ---------------------------------------------------------------------------

_OPEN = {"(": ")", "[": "]", "{": "}"}
_CLOSE = {")", "]", "}"}
_DECL_KEYWORDS = {"int", "char", "short", "long", "unsigned", "signed", "bool", "_Bool", "void",
                  "float", "double", "const", "volatile", "static", "extern", "register", "auto",
                  "struct", "union", "enum", "typedef", "constexpr", "inline", "thread_local",
                  "static_assert", "_Static_assert", "using", "class", "typename", "template"}
_COMPOUND_KEYWORDS = {"if", "else", "while", "for", "do", "switch", "case", "default", "goto",
                      "try", "catch"}
_JUMPS = ("break", "continue")  # deletable: simple statements that move control
_HEADS = ("if", "while", "for")  # `if (c) break;`: the jump is the deletable statement


def matching(tokens: list[Token]) -> dict[int, int]:
    """Index of each opening bracket -> its closing bracket; raises LexError on
    unbalanced brackets."""
    stack: list[int] = []
    pair: dict[int, int] = {}
    for i, t in enumerate(tokens):
        if t.kind != "op":
            continue
        if t.text in _OPEN:
            stack.append(i)
        elif t.text in _CLOSE:
            if not stack or _OPEN[tokens[stack[-1]].text] != t.text:
                raise LexError(f"unbalanced `{t.text}`")
            pair[stack.pop()] = i
    if stack:
        raise LexError(f"unclosed `{tokens[stack[-1]].text}`")
    return pair


def function_bodies(tokens: list[Token], pair: dict[int, int]) -> list[tuple[int, int]]:
    """(open, close) token indexes of the function bodies: a `{` outside any braces
    whose statement so far has a `)` (a parameter list, a contract, `noexcept`, a
    post-parameter attribute, a trailing return type, `const`, `&` all come after
    one) and no `=` outside parentheses (an initializer: `int a[] = {...}`, a lambda).
    The bodies inside `namespace` and `extern "C"` blocks count."""
    bodies: list[tuple[int, int]] = []
    transparent: set[int] = set()  # closing braces of namespace / extern blocks
    start = 0  # first token of the statement being read
    i = 0
    while i < len(tokens):
        t = tokens[i]
        if t.kind == "op" and t.text == "{":
            stmt = tokens[start:i]
            paren, outer_eq, has_close = 0, False, False
            for u in stmt:
                if u.kind != "op":
                    continue
                if u.text == "(":
                    paren += 1
                elif u.text == ")":
                    paren -= 1
                    has_close = True
                elif u.text == "=" and paren == 0:
                    outer_eq = True
            first = stmt[0].text if stmt else ""
            if has_close and not outer_eq:
                bodies.append((i, pair[i]))
            elif first in ("namespace", "extern") and not has_close:
                transparent.add(pair[i])
                i += 1
                start = i
                continue
            i = pair[i] + 1
            start = i
        elif t.kind == "op" and t.text in ("(", "["):
            i = pair[i] + 1
        elif t.kind == "op" and t.text == ";":
            i += 1
            start = i
        elif i in transparent:
            i += 1
            start = i
        else:
            i += 1
    return bodies


def function_spans(src: str) -> list[tuple[str, int, int, int]]:
    """(name, line, start, end) of every function body of `src`: the name before its
    parameter list, the line of the opening brace, and the character range of the
    braces."""
    tokens = lex(src)
    pair = matching(tokens)
    line_starts = [0] + [m.end() for m in re.finditer("\n", src)]
    out = []
    for o, c in function_bodies(tokens, pair):
        name = ""  # the identifier before the first "(" of the statement before the `{`
        for u in range(o - 1, -1, -1):
            if tokens[u].kind == "op" and tokens[u].text in (";", "}", "{"):
                break
        else:
            u = -1
        for v in range(u + 1, o):
            if tokens[v].text == "(" and v > 0 and tokens[v - 1].kind == "ident":
                name = tokens[v - 1].text
                break
        line = bisect.bisect_right(line_starts, tokens[o].start)
        out.append((name, line, tokens[o].start, tokens[c].end))
    return out


def macro_argument_ranges(tokens: list[Token], pair: dict[int, int]) -> list[tuple[int, int]]:
    """(open, close) of the arguments of `CERASTES_*(...)` and of the
    `[[cerastes::name(...)]]` attributes."""
    out = []
    for i, t in enumerate(tokens):
        if t.kind == "ident" and t.text.startswith("CERASTES_") and i + 1 < len(tokens) \
                and tokens[i + 1].text == "(":
            out.append((i + 1, pair[i + 1]))
        elif t.kind == "ident" and t.text == "cerastes" and i + 3 < len(tokens) \
                and tokens[i + 1].text == "::" and tokens[i + 3].text == "(":
            out.append((i + 3, pair[i + 3]))
    return out


@dataclasses.dataclass(frozen=True)
class Mutant:
    path: str  # as it appears in the id
    line: int
    column: int
    op: str
    original: str
    new: str
    start: int  # the replaced range of the source
    end: int
    replacement: str

    @property
    def id(self) -> str:
        return f"{self.path}:{self.line}:{self.column}:{self.op}:{self.original}->{self.new}"

    def apply(self, src: str) -> str:
        return src[:self.start] + self.replacement + src[self.end:]


_RELOP = {"<": "<=", "<=": "<", ">": ">=", ">=": ">", "==": "!=", "!=": "=="}
_INT_RE = re.compile(r"^(?P<digits>0|[1-9][0-9]*)(?P<suffix>[uUlL]*)$")
_LOGIC_STOP = {"||", "?", ":", ",", ";", "=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=",
               "<<=", ">>="}
_LOGIC_STOP_WORDS = {"return", "case", "if", "while", "for", "switch", "else", "do"}


def one_line(text: str) -> str:
    return " ".join(text.split())


def _conjunct_end(tokens, pair, i):
    """Index after the last token of the operand that starts at token `i` and
    extends right of an `&&`: up to `&&`, `||`, `?`, `:`, `,`, `;`, an
    assignment or the closing bracket of the enclosing group."""
    j = i
    while j < len(tokens):
        t = tokens[j]
        if t.kind == "op":
            if t.text in _OPEN:
                j = pair[j] + 1
                continue
            if t.text in _CLOSE or t.text == "&&" or t.text in _LOGIC_STOP:
                break
        j += 1
    return j


def _conjunct_start(tokens, pair, rev_pair, i):
    """Index of the first token of the operand that ends just before token `i`
    (an `&&`), scanning left to `&&`, `||`, `?`, `:`, `,`, `;`, an assignment, a
    keyword such as `return`, or the opening bracket of the enclosing group."""
    j = i - 1
    while j >= 0:
        t = tokens[j]
        if t.kind == "op":
            if t.text in _CLOSE:
                j = rev_pair[j] - 1
                continue
            if t.text in _OPEN or t.text == "&&" or t.text in _LOGIC_STOP:
                break
        elif t.kind == "ident" and t.text in _LOGIC_STOP_WORDS:
            break
        j -= 1
    return j + 1


def mutants_of(src: str, path: str) -> list[Mutant]:
    """Every mutant of `src` (see the module docstring), in source order."""
    tokens = lex(src)
    pair = matching(tokens)
    rev_pair = {c: o for o, c in pair.items()}
    bodies = function_bodies(tokens, pair)
    ranges = macro_argument_ranges(tokens, pair)
    in_body = [False] * len(tokens)
    for o, c in bodies:
        for k in range(o + 1, c):
            in_body[k] = True
    in_macro = [False] * len(tokens)
    for o, c in ranges:
        for k in range(o + 1, c):
            in_macro[k] = True
    targets = [in_body[k] or in_macro[k] for k in range(len(tokens))]
    line_starts = [0] + [m.end() for m in re.finditer("\n", src)]

    def position(offset: int) -> tuple[int, int]:
        line = bisect.bisect_right(line_starts, offset) - 1
        return line + 1, offset - line_starts[line] + 1

    out: list[Mutant] = []

    def add(op, start, end, original, new, replacement):
        line, col = position(start)
        out.append(Mutant(path, line, col, op, one_line(original), one_line(new), start, end,
                          replacement))

    for k, t in enumerate(tokens):
        if not targets[k]:
            continue
        if t.kind == "op" and t.text in _RELOP:
            add("relop", t.start, t.end, t.text, _RELOP[t.text], _RELOP[t.text])
        elif t.kind == "number" and (m := _INT_RE.match(t.text)):
            value, suffix = int(m.group("digits")), m.group("suffix")
            add("literal", t.start, t.end, t.text, f"{value + 1}{suffix}", f"{value + 1}{suffix}")
            if value > 0:
                add("literal", t.start, t.end, t.text, f"{value - 1}{suffix}",
                    f"{value - 1}{suffix}")
        elif t.kind == "op" and t.text in ("&&", "||"):
            other = "||" if t.text == "&&" else "&&"
            add("swap-logic", t.start, t.end, t.text, other, other)
            if t.text == "&&":
                right_end = _conjunct_end(tokens, pair, k + 1)
                if right_end > k + 1:
                    text = src[t.start:tokens[right_end - 1].end]
                    add("drop-conjunct", t.start, tokens[right_end - 1].end, text, "", "")
                left_start = _conjunct_start(tokens, pair, rev_pair, k)
                if left_start < k:
                    text = src[tokens[left_start].start:t.end]
                    add("drop-conjunct", tokens[left_start].start, t.end, text, "", "")

    for o, c in bodies:
        out += statement_mutants(src, tokens, pair, o, c, position, path)
    out += effect_mutants(src, tokens, pair, rev_pair, bodies, in_macro, position, path)
    out.sort(key=lambda m: (m.start, m.op, m.end, m.new))
    return out



# ---------------------------------------------------------------------------
# The safe-direction bucket
# ---------------------------------------------------------------------------

SAFE = "safe-direction"
_CONTRACT_RE = re.compile(r"CERASTES_(REQUIRES|ENSURES|CHECK|INVARIANT|DECREASES)\s*\(")


@functools.lru_cache(maxsize=4)
def _contract_calls(src: str):
    """((macro name, clause start, clause end, result name), (argument start, argument end))
    of every `CERASTES_REQUIRES|ENSURES|CHECK|INVARIANT|DECREASES(...)` call of `src`, in
    order, with the result name of `ENSURES(r, ...)` left out of the clause (the name is the
    fourth item, None when there is none).  Read once per source."""
    out = []
    for m in _CONTRACT_RE.finditer(src):
        i = m.end()
        depth, j = 1, i
        while j < len(src) and depth:
            depth += (src[j] == "(") - (src[j] == ")")
            j += 1
        start, end = i, j - 1
        binder = None
        if m.group(1) == "ENSURES":
            b = re.match(r"\s*([A-Za-z_]\w*)\s*,", src[start:end])
            if b:
                start += b.end()
                binder = b.group(1)
        out.append(((m.group(1), start, end, binder), (i, j - 1)))
    return tuple(out)


def contract_clause(src: str, offset: int):
    """(macro name, clause start, clause end, result name) of the `CERASTES_*(...)` call
    whose argument contains `offset`, with the result name of `ENSURES(r, ...)` left out
    of the clause (the name is the fourth item, None when there is none), or None when
    the offset is not in the argument of one."""
    for clause, (i, end) in _contract_calls(src):
        if i <= offset < end:
            return clause
    return None


_STATEMENT_BEFORE = {";", "{", "}", ")", ":", "else", "do"}  # what may precede a statement


@functools.lru_cache(maxsize=4)
def _assert_calls(src: str):
    """(argument start, argument end) of every `assert(...);` statement of `src` (the
    `<assert.h>` macro: exactly the identifier `assert`, not `static_assert` or `my_assert`)
    whose argument is pure by the rule of `is_assert_statement`."""
    try:
        tokens = lex(src)
        pair = matching(tokens)
    except LexError:
        return ()
    out = []
    for k, t in enumerate(tokens):
        if not (t.kind == "ident" and t.text == "assert" and k + 1 < len(tokens)
                and tokens[k + 1].text == "(" and k > 0 and tokens[k - 1].text in _STATEMENT_BEFORE):
            continue
        close = pair[k + 1]
        if close + 1 < len(tokens) and tokens[close + 1].text == ";" \
                and is_assert_statement(src[t.start:tokens[close + 1].end]):
            out.append((tokens[k + 1].end, tokens[close].start))
    return tuple(out)


def assert_clause(src: str, offset: int):
    """("ASSERT", argument start, argument end, None) of the pure `assert(...);` statement whose
    argument contains `offset`, else None (decision 0015: a failing assert is a `check`)."""
    for start, end in _assert_calls(src):
        if start <= offset < end:
            return ("ASSERT", start, end, None)
    return None


def _cdiv(a, b):
    if b == 0:
        raise ZeroDivisionError
    q = abs(a) // abs(b)
    return q if (a < 0) == (b < 0) else -q


def _cmod(a, b):
    return a - b * _cdiv(a, b)


class _Unknown(Exception):
    pass


_COMPARISONS = {"<", "<=", ">", ">=", "==", "!="}


def c_semantics_ok(tokens: list[Token]) -> bool:
    """Whether Python reads the clause the way C does (the grid evaluates the Python reading).  Python
    chains comparisons (`a < b < c` is `a < b and b < c`; C compares the 0/1 of `a < b` with `c`, and
    puts `==` below `<`: `r == x > 0` is `r == (x > 0)`), `not` binds looser than a comparison (`!r < 1` is
    `(!r) < 1` in C), and `and`/`or` yield an operand (`(x && y) == 1` compares 1 in C).  Only the
    forms where the two readings agree are accepted (the rest is `unknown`, never bucketed):

    * at most one comparison operator between two `&&`/`||` or brackets of one level;
    * a bracketed group that has `&&` or `||` of its own (or holds nothing but such a group: `((x && y))`) is
      used only as an operand of `&&`, `||`, `!`, another bracket, or alone (never of a comparison or of
      arithmetic);
    * the operand of `!` (a bracketed group, an identifier or a literal, after any number of `!`) is
      followed only by `&&`, `||`, a closing bracket or the end of the clause."""
    n = len(tokens)
    closes: dict[int, int] = {}  # index of a `(` -> index of its `)`
    valued: set[int] = set()  # the `(` of the groups whose value is a `&&`/`||` result
    frames = [[0, False, -1]]  # [comparisons since the last `&&`/`||`, has a logic operator, index of its `(`]
    for k, t in enumerate(tokens):
        if t.kind != "op":
            continue
        frame = frames[-1]
        if t.text in _COMPARISONS:
            frame[0] += 1
            if frame[0] > 1:
                return False
        elif t.text in ("&&", "||"):
            frame[0] = 0
            frame[1] = True
        elif t.text == "(":
            frames.append([0, False, k])
        elif t.text == ")":
            if len(frames) == 1:
                return False
            _, logic, opened = frames.pop()
            closes[opened] = k
            child = opened + 1  # a bracket that holds nothing but a logic-valued group is logic-valued too
            if not logic and child < k and tokens[child].text == "(" and closes.get(child) == k - 1 \
                    and child in valued:
                logic = True
            if logic:
                valued.add(opened)
                before = tokens[opened - 1].text if opened > 0 else None
                after = tokens[k + 1].text if k + 1 < n else None
                if before not in (None, "&&", "||", "(", "!") or after not in (None, "&&", "||", ")"):
                    return False
    if len(frames) != 1:
        return False
    pair = {}
    stack: list[int] = []
    for k, t in enumerate(tokens):
        if t.kind == "op" and t.text == "(":
            stack.append(k)
        elif t.kind == "op" and t.text == ")":
            pair[stack.pop()] = k
    for k, t in enumerate(tokens):
        if not (t.kind == "op" and t.text == "!"):
            continue
        j = k + 1
        while j < n and tokens[j].kind == "op" and tokens[j].text == "!":
            j += 1
        if j >= n:
            return False
        if tokens[j].kind == "op" and tokens[j].text == "(":
            after = pair[j] + 1
        elif tokens[j].kind in ("ident", "number"):
            after = j + 1
        else:
            return False
        if after < n and tokens[after].text not in ("&&", "||", ")"):
            return False
    return True


def _as_python(clause: str) -> str:
    c = re.sub(r"(\d)[uUlL]+\b", r"\1", clause)
    if "?" in c or "sizeof" in c or "[" in c or "->" in c or "." in re.sub(r"\d\.\d", "", c):
        raise _Unknown(clause)
    try:
        if not c_semantics_ok(lex(clause)):
            raise _Unknown(clause)
    except LexError:
        raise _Unknown(clause) from None
    c = re.sub(r"\(\s*(?:unsigned\s+|signed\s+)?(?:char|short|int|long\s+long|long|bool)\s*\)", "", c)
    c = c.replace("&&", " and ").replace("||", " or ")
    c = re.sub(r"!(?!=)", " not ", c)
    c = re.sub(r"\btrue\b", "True", c)
    c = re.sub(r"\bfalse\b", "False", c)
    operand = r"(\w+|\([^()]*\))"
    for _ in range(8):
        n = re.sub(operand + r"\s*/\s*" + operand, r"_cdiv(\1,\2)", c)
        n = re.sub(operand + r"\s*%\s*" + operand, r"_cmod(\1,\2)", n)
        if n == c:
            break
        c = n
    if "/" in c or "%" in c:
        raise _Unknown(clause)
    return c


_EVAL_GLOBALS = {"_cdiv": _cdiv, "_cmod": _cmod, "__builtins__": {}}
_GRID_LIMIT = 2_000_000  # a grid of more points than this is not evaluated: `unknown`


def _holds(code, env) -> bool:
    """Whether the clause holds at `env`; `code` is a code object (`compile(..., "eval")`) so that the
    grid, which calls this thousands of times for one clause, compiles it once."""
    try:
        return bool(eval(code, _EVAL_GLOBALS, env))
    except ZeroDivisionError:
        return False  # the clause is undefined: it does not hold
    except Exception as err:  # anything else is a construct we do not model
        raise _Unknown(str(err))


def clause_relation(original: str, mutant: str) -> str:
    """`stronger`, `weaker`, `equal`, `incomparable` or `unknown` (the mutant relative to
    the original), by evaluating both on a grid of values (see the module docstring)."""
    try:
        po, pm = _as_python(original), _as_python(mutant)
        names = sorted(set(re.findall(r"\b[A-Za-z_]\w*\b", po + " " + pm))
                       - {"and", "or", "not", "True", "False", "_cdiv", "_cmod"})
        lits = {int(x) for x in re.findall(r"\d+", original + " " + mutant)}
        lits |= {-x for x in lits}
        values = {0, 1, -1, 2, 3}
        for lit in lits:
            values |= {lit - 1, lit, lit + 1}
        if len(names) > 3:
            values = {v for v in values if abs(v) < 10 or v in lits or v - 1 in lits or v + 1 in lits}
        values = sorted(values)
        if len(values) ** len(names) > _GRID_LIMIT:
            return "unknown"
        # eval() strips leading blanks and compile() does not: strip the same ones
        code_o = compile(po.strip(" \t"), "<original>", "eval")
        code_m = compile(pm.strip(" \t"), "<mutant>", "eval")
        orig_implies_mut = mut_implies_orig = True
        for combo in itertools.product(values, repeat=len(names)):
            env = dict(zip(names, combo))
            a, b = _holds(code_o, env), _holds(code_m, env)
            orig_implies_mut &= not (a and not b)
            mut_implies_orig &= not (b and not a)
            if not orig_implies_mut and not mut_implies_orig:
                return "incomparable"
    except (_Unknown, SyntaxError, NameError, TypeError, ValueError, RecursionError):
        return "unknown"
    if orig_implies_mut and mut_implies_orig:
        return "equal"
    return "weaker" if orig_implies_mut else "stronger"


# The types of a parameter (or a result) whose clauses the grid models: the signed builtin
# integer types, `const` allowed.  Not `bool`, plain `char`, anything unsigned, a typedef, a
# pointer or a reference: those wrap, are range-limited differently, or are not integers.
_SIGNED_TYPES = {tuple(sorted(t)) for t in (
    ("signed", "char"), ("short",), ("short", "int"), ("signed", "short"), ("signed", "short", "int"),
    ("int",), ("signed",), ("signed", "int"),
    ("long",), ("long", "int"), ("signed", "long"), ("signed", "long", "int"),
    ("long", "long"), ("long", "long", "int"), ("signed", "long", "long"),
    ("signed", "long", "long", "int"))}
_STORAGE_WORDS = {"static", "inline", "extern", "constexpr", "_Noreturn", "consteval"}
# The tokens a clause may have for the grid to model it: linear integer arithmetic over signed
# parameters, comparisons and logic.  Everything else (`/`, `%`, shifts, bit operations, `?:`,
# casts (their type words are identifiers that are no parameter), calls, subscripts, `++`) is
# unknown, and so is every `*`: a literal coefficient is not monotone (`3 * x + y < 10` against
# `4 * x + y < 10` is incomparable, which the grid may not show), so no product is modelled.
_LINEAR_OPS = {"+", "-", "<", "<=", ">", ">=", "==", "!=", "&&", "||", "!", "(", ")"}
_DECIMAL_RE = re.compile(r"^[0-9]+[lL]{0,2}$")  # no `u` suffix, no hex, no float
_NOT_A_DECLARATION_PREFIX = {"return", "else", "do", "case", "goto", "sizeof"}


def is_signed_builtin(words) -> bool:
    """Whether the type words (without `const`) name a signed builtin integer type."""
    words = [w for w in words if w != "const"]
    return tuple(sorted(words)) in _SIGNED_TYPES


@dataclasses.dataclass(frozen=True)
class Signature:
    name: str
    decl_start: int  # offset of the first token of the declaration (the contracts are after the `)`)
    body_end: int  # offset after the closing brace
    returns_signed: bool
    signed_params: frozenset  # names of the parameters of a signed builtin integer type
    body_decl_names: frozenset  # identifiers that follow another identifier in the body (`T x`)


def function_signatures(src: str) -> list[Signature]:
    """The signature of each function that has a body, and the identifiers that its body
    may declare (a declaration `T name` can shadow a parameter in a nested block).  Read
    once per source."""
    return list(_function_signatures(src))


@functools.lru_cache(maxsize=4)
def _function_signatures(src: str) -> tuple[Signature, ...]:
    tokens = lex(src)
    pair = matching(tokens)
    out = []
    for o, c in function_bodies(tokens, pair):
        u = o - 1
        while u >= 0 and not (tokens[u].kind == "op" and tokens[u].text in (";", "}", "{")):
            u -= 1
        v = next((k for k in range(u + 1, o)
                  if tokens[k].text == "(" and k > 0 and tokens[k - 1].kind == "ident"), None)
        if v is None:
            continue
        ret = [t.text for t in tokens[u + 1:v - 1] if t.text not in _STORAGE_WORDS]
        params: list[list[Token]] = [[]]
        depth = 0
        for t in tokens[v + 1:pair[v]]:
            if t.kind == "op" and t.text in _OPEN:
                depth += 1
            elif t.kind == "op" and t.text in _CLOSE:
                depth -= 1
            if depth == 0 and t.kind == "op" and t.text == ",":
                params.append([])
            else:
                params[-1].append(t)
        signed = set()
        for p in params:
            if len(p) >= 2 and p[-1].kind == "ident" and all(t.kind == "ident" for t in p) \
                    and is_signed_builtin([t.text for t in p[:-1]]):
                signed.add(p[-1].text)
        declared = {tokens[k].text for k in range(o + 1, c)
                    if tokens[k].kind == "ident" and tokens[k - 1].kind == "ident"
                    and tokens[k - 1].text not in _NOT_A_DECLARATION_PREFIX}
        out.append(Signature(tokens[v - 1].text, tokens[u + 1].start, tokens[c].end,
                             is_signed_builtin(ret) and all(w.isidentifier() for w in ret),
                             frozenset(signed), frozenset(declared)))
    return tuple(out)


def declarator_indexes(tokens: list[Token], pair: dict[int, int]) -> set[int]:
    """Indexes of the identifiers that are the name of a function being declared or defined:
    outside every function body, bracket and brace block except `namespace` and `extern`
    blocks, directly after a type (an identifier, `*`, `&`, `&&` or `>`), followed by `(`, and
    with no `=` earlier in the statement (`int v = f(2);` is a use).  Anything else is not."""
    bodies = {o for o, _c in function_bodies(tokens, pair)}
    out: set[int] = set()
    n = len(tokens)
    start, seen_eq, i = 0, False, 0
    while i < n:
        t = tokens[i]
        if t.kind == "op" and t.text == "{":
            stmt = tokens[start:i]
            first = stmt[0].text if stmt else ""
            has_close = any(u.kind == "op" and u.text == ")" for u in stmt)
            if i not in bodies and first in ("namespace", "extern") and not has_close:
                i += 1  # a transparent block: its declarations are at the top level
                start, seen_eq = i, False
                continue
            i = pair[i] + 1
            start, seen_eq = i, False
        elif t.kind == "op" and t.text in ("(", "["):
            i = pair[i] + 1
        elif t.kind == "op" and t.text in (";", "}"):
            i += 1
            start, seen_eq = i, False
        else:
            if t.kind == "op" and t.text == "=":
                seen_eq = True
            elif (t.kind == "ident" and not seen_eq and i > start and i + 1 < n
                    and tokens[i + 1].kind == "op" and tokens[i + 1].text == "("
                    and (tokens[i - 1].kind == "ident"
                         or (tokens[i - 1].kind == "op" and tokens[i - 1].text in ("*", "&", "&&", ">")))):
                out.add(i)
            i += 1
    return out


@functools.lru_cache(maxsize=4)
def _used_names(src: str):
    """The identifiers that occur anywhere in `src` other than as the name of a function being
    declared or defined (`declarator_indexes`): in a body, a contract, a default argument, an
    initializer, a `#define` body or a directive.  Comments and strings are not read.  None when
    the file does not lex with its directives (then everything counts as used)."""
    try:
        plain = lex(src)
        pair = matching(plain)
        full = lex(src, preprocessor=True)
    except LexError:
        return None
    plain_starts = {t.start for t in plain}
    names = {t.text for t in full if t.kind == "ident" and t.start not in plain_starts}
    declarators = declarator_indexes(plain, pair)
    names |= {t.text for k, t in enumerate(plain) if t.kind == "ident" and k not in declarators}
    return frozenset(names)


def has_caller(src: str, name: str) -> bool:
    """Whether the identifier `name` occurs anywhere in the file other than as the name of its own
    declaration or definition (a call, a recursive call, the address taken, a mention in a
    contract, a `#define` body, a default argument, an initializer: any use a stronger `requires`
    or a weaker `ensures` could break)."""
    used = _used_names(src)
    return used is None or name in used


def clause_is_modelled(clause: str, allowed: frozenset) -> bool:
    """Whether every identifier of `clause` is in `allowed` (or `true`/`false`) and the clause
    is linear over them: only the operators of `_LINEAR_OPS` (so no `*`), decimal literals without
    an unsigned suffix, and a reading in which Python and C agree (`c_semantics_ok`)."""
    try:
        tokens = lex(clause)
    except LexError:
        return False
    for t in tokens:
        if t.kind == "ident":
            if t.text not in allowed and t.text not in ("true", "false"):
                return False
        elif t.kind == "number":
            if not _DECIMAL_RE.match(t.text):
                return False
        elif t.kind == "op":
            if t.text not in _LINEAR_OPS:
                return False
        else:
            return False  # a string or character literal
    return c_semantics_ok(tokens)


_ASSERT_EFFECT_OPS = {"++", "--", ",", "="}


def is_assert_statement(text: str) -> bool:
    """Whether `text` is exactly one `assert(...);` statement (the `<assert.h>` macro) whose argument
    has no call, `++`, `--`, assignment or comma."""
    try:
        tokens = lex(text)
        pair = matching(tokens)
    except LexError:
        return False
    if not (len(tokens) >= 4 and tokens[0].kind == "ident" and tokens[0].text == "assert"
            and tokens[1].text == "(" and pair.get(1) == len(tokens) - 2
            and tokens[-1].text == ";"):
        return False
    # the argument must be pure: a call (it may end the program, so deleting the assert can make
    # what follows fail: `assert(checked(x) == 1)`), an increment, an assignment or a comma may
    # change what the rest of the function sees, so such a mutant is not the safe direction
    inner = tokens[2:-2]
    for k, t in enumerate(inner):
        if t.kind == "op" and (t.text in _ASSERT_EFFECT_OPS or t.text.endswith("=")
                               and t.text not in ("==", "!=", "<=", ">=")):
            return False
        if t.kind == "ident" and k + 1 < len(inner) and inner[k + 1].text == "(":
            return False
    return True


def is_safe_direction(m: "Mutant", src: str) -> bool:
    """A mutant of a contract that makes it easier to meet: a stronger `requires`, a
    weaker `ensures` or `check` (or a weaker argument of a pure `assert(...);`, which is a `check`,
    decision 0015).  Never a mutant of code, and never an `invariant` or
    a `decreases`.  Bucketed only when the grid models the clause (see the module
    docstring): every variable a signed builtin integer parameter of the enclosing function (the
    `ensures` result only if the return type is one), a linear clause, no `==`/`!=` flip,
    and no use of the function in the file for a stronger `requires` or a weaker `ensures`.  Whether the
    mutated clause is well-defined (a dropped guard) is not asked here: see the module docstring."""
    if m.op in EFFECT_OPS:
        return False  # a mutant of code: it must be killed or listed
    if m.op == "delete-stmt":
        # an `assert(e);` deleted: the program only loses a check, which a sound verifier must
        # accept (a survivor needs no kill and no entry in the equivalence list; decision 0015)
        return is_assert_statement(src[m.start:m.end])
    found = contract_clause(src, m.start)
    if found is None:
        found = assert_clause(src, m.start)  # `assert(e);` is a `check`
    if found is None or found[0] not in ("REQUIRES", "ENSURES", "CHECK", "ASSERT"):
        return False
    if m.op == "relop" and m.original in ("==", "!="):
        return False  # a flip of equality is `unknown`: never bucketed
    if not (found[1] <= m.start and m.end <= found[2]):
        return False  # the change reaches outside the clause (the clause is no longer the same call)
    sig = next((s for s in _function_signatures(src) if s.decl_start <= found[1] < s.body_end), None)
    if sig is None:
        return False  # a contract of a bodyless declaration: no signature to read the types from
    allowed = set(sig.signed_params)
    if found[0] == "ENSURES" and sig.returns_signed and found[3] is not None:
        allowed.add(found[3])
    if found[0] in ("CHECK", "ASSERT"):
        allowed -= sig.body_decl_names  # a local of the same name may shadow the parameter
    if found[0] in ("REQUIRES", "ENSURES") and has_caller(src, sig.name):
        # a caller may fail the stronger precondition, or rely on the postcondition that is now
        # weaker: the mutant must be killed by a failure at the use, or be listed
        return False
    clause = src[found[1]:found[2]]
    if not clause_is_modelled(clause, frozenset(allowed)):
        return False
    # the changed text lies inside the clause, so the mutated clause is the clause with it replaced
    mutant_clause = src[found[1]:m.start] + m.replacement + src[m.end:found[2]]
    if not clause_is_modelled(mutant_clause, frozenset(allowed)):
        return False
    relation = clause_relation(clause, mutant_clause)
    return relation == ("stronger" if found[0] == "REQUIRES" else "weaker")


def statement_mutants(src, tokens, pair, open_i, close_i, position, path) -> list[Mutant]:
    """Mutants that replace a simple expression statement by `;`, in the block
    `tokens[open_i..close_i]` and the blocks nested in it."""
    out: list[Mutant] = []
    i = open_i + 1
    start = i  # first token of the statement being read
    while i < close_i:
        t = tokens[i]
        if t.kind == "op" and t.text == "{":
            out += statement_mutants(src, tokens, pair, i, pair[i], position, path)
            i = pair[i] + 1
            start = i
            continue
        if t.kind == "op" and t.text in ("(", "["):
            i = pair[i] + 1
            continue
        if t.kind == "op" and t.text == "}":
            start = i + 1
        elif t.kind == "op" and t.text == ";":
            target = deletable_target(tokens, pair, start, i)
            if target is not None:
                line, col = position(tokens[target].start)
                text = src[tokens[target].start:t.end]
                short = one_line(text)
                if len(short) > STATEMENT_ID_CHARS:
                    short = short[:STATEMENT_ID_CHARS - 3] + "..."
                out.append(Mutant(path, line, col, "delete-stmt", short, ";",
                                  tokens[target].start, t.end, ";"))
            start = i + 1
        elif t.kind == "ident" and t.text in ("else", "do") and start == i:
            start = i + 1
        i += 1
    return out


# ---------------------------------------------------------------------------
# Side effects in expressions: drop-store, swap-incdec, drop-comma
# ---------------------------------------------------------------------------

_ASSIGN_OPS = {"=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="}
_NOT_CALLS = {"sizeof", "alignof", "_Alignof", "typeof", "typeof_unqual", "_Generic", "defined",
              "if", "while", "for", "switch", "return", "case", "else", "do", "goto", "decltype",
              "noexcept", "static_assert", "_Static_assert", "asm", "__asm__"} | _DECL_KEYWORDS
_GROUP_HEADS = ("if", "while", "switch")  # `if (a, b)`: the comma is an operator
_GROUP_WORDS = _GROUP_HEADS + ("return", "else", "do")  # a `(` after these groups an expression
_NO_EXPRESSION = ("goto", "break", "continue", "case", "default")
_OPERAND_BEFORE = {"(", ",", "=", "?", "[", "return"} | _ASSIGN_OPS  # what may precede an assignment's variable


def _is_declaration(tokens: list[Token], first: int, semi: int) -> bool:
    """Whether tokens[first..semi] (the statement without its labels and heads) is a declaration:
    it starts with a declaration keyword, or with `T name` / `T *name` (a typedef name)."""
    if first >= semi or tokens[first].kind != "ident":
        return False
    t = tokens[first].text
    if t in _DECL_KEYWORDS:
        return True
    if t in _COMPOUND_KEYWORDS or t == "return":
        return False
    j = first + 1
    while j < semi and tokens[j].kind == "op" and tokens[j].text in ("*", "&", "&&"):
        j += 1
    return j < semi and tokens[j].kind == "ident" and tokens[j].text not in _NOT_CALLS


def _has_effect(tokens: list[Token], lo: int, hi: int) -> bool:
    """Whether tokens[lo..hi) holds an assignment, `++`, `--` or a call."""
    for k in range(lo, hi):
        t = tokens[k]
        if t.kind == "op" and (t.text in _ASSIGN_OPS or t.text in ("++", "--")):
            return True
        if t.kind == "ident" and t.text not in _NOT_CALLS and k + 1 < hi \
                and tokens[k + 1].kind == "op" and tokens[k + 1].text == "(":
            return True
    return False


class _EffectContext:
    """What the effect operators need to know about the statements of one function body: the `=` that
    initialize a declarator (never an assignment), the element spans whose value is discarded (a whole
    expression statement, a `for` init or step clause, each operand of a comma that is itself
    discarded, the operand of a cast to `void`), and the comma operators (as the token index of the
    comma and the span of its left operand)."""

    def __init__(self, tokens, pair, rev_pair):
        self.tokens = tokens
        self.pair = pair
        self.rev_pair = rev_pair
        self.init_eq: set[int] = set()
        self.discarded: set[tuple[int, int]] = set()
        self.commas: list[tuple[int, int, int]] = []

    def split(self, lo, hi):
        """Spans of the elements of tokens[lo..hi) separated by commas at its own level."""
        tokens, pair = self.tokens, self.pair
        spans, start, i = [], lo, lo
        while i < hi:
            t = tokens[i]
            if t.kind == "op" and t.text in _OPEN:
                i = pair[i] + 1
                continue
            if t.kind == "op" and t.text == ",":
                spans.append((start, i))
                start = i + 1
            i += 1
        spans.append((start, hi))
        return spans

    def grouping_paren(self, i):
        """Whether the `(` at token `i` groups an expression (or is the head of an `if`, `while` or
        `switch`) rather than starting an argument list: its commas are comma operators."""
        tokens = self.tokens
        if i == 0:
            return True
        p = tokens[i - 1]
        if p.kind == "ident":
            return p.text in _GROUP_WORDS
        if p.kind == "op" and p.text == ")":
            o = self.rev_pair[i - 1]
            before = tokens[o - 1] if o > 0 else None
            if before is not None and before.kind == "ident" and before.text in _GROUP_HEADS + ("for",):
                return True  # `if (c) (a, b);`
            inner = tokens[o + 1:i - 1]  # a cast: `(int)(a, b)`
            return bool(inner) and any(u.kind == "ident" for u in inner) and all(
                u.kind == "ident" and u.text in _DECL_KEYWORDS or u.kind == "op" and u.text == "*"
                for u in inner)
        return not (p.kind == "op" and p.text in ("]", "::", ".", "->"))

    def scan_commas(self, lo, hi, operator):
        """Record the comma operators of tokens[lo..hi); `operator` says whether commas at this level
        are comma operators (they are separators in an argument list, an initializer or a declaration)."""
        tokens, pair = self.tokens, self.pair
        start, i = lo, lo
        while i < hi:
            t = tokens[i]
            if t.kind == "op" and t.text in _OPEN:
                self.scan_commas(i + 1, pair[i], t.text == "(" and self.grouping_paren(i))
                i = pair[i] + 1
                continue
            if t.kind == "op" and t.text == ",":
                if operator:
                    self.commas.append((i, start, i))
                start = i + 1
            i += 1

    def discard(self, lo, hi):
        """The value of tokens[lo..hi) is discarded: each comma element, after a cast to `void` and
        inside redundant parentheses, is a discarded element."""
        tokens, pair = self.tokens, self.pair
        for a, b in self.split(lo, hi):
            while b - a >= 3 and tokens[a].text == "(" and tokens[a + 1].text == "void" \
                    and tokens[a + 2].text == ")":
                a += 3
            if a >= b:
                continue
            self.discarded.add((a, b))
            if tokens[a].kind == "op" and tokens[a].text == "(" and pair[a] == b - 1:
                self.discard(a + 1, b - 1)

    def head(self, i):
        """The head `if|while|switch|for (...)` at token `i`; returns the index after its `)`."""
        tokens, pair = self.tokens, self.pair
        o, c = i + 1, pair[i + 1]
        if tokens[i].text != "for":
            self.scan_commas(o + 1, c, True)
            return c + 1
        clauses = []  # init, condition, step
        start, k = o + 1, o + 1
        while k < c:
            u = tokens[k]
            if u.kind == "op" and u.text in _OPEN:
                k = pair[k] + 1
                continue
            if u.kind == "op" and u.text == ";":
                clauses.append((start, k))
                start = k + 1
            k += 1
        clauses.append((start, c))
        if len(clauses) == 3:
            init, cond, step = clauses
            if _is_declaration(tokens, init[0], init[1]):
                self.declaration(init[0], init[1])
            else:
                self.scan_commas(init[0], init[1], True)
                self.discard(*init)
            self.scan_commas(cond[0], cond[1], True)
            self.scan_commas(step[0], step[1], True)
            self.discard(*step)
        else:  # a range-based for: no expression of ours
            self.scan_commas(o + 1, c, False)
        return c + 1

    def declaration(self, lo, hi):
        """tokens[lo..hi) is a declaration: the first `=` of each declarator initializes it (a later `=` of
        the same initializer, `int b = a = x;`, is an assignment), and its commas separate declarators."""
        tokens, pair = self.tokens, self.pair
        i = lo
        expect = True  # the next `=` initializes a declarator (the ones after it are assignments)
        while i < hi:
            t = tokens[i]
            if t.kind == "op" and t.text in _OPEN:
                i = pair[i] + 1
                continue
            if t.kind == "op" and t.text == ",":
                expect = True
            elif t.kind == "op" and t.text == "=" and expect:
                self.init_eq.add(i)
                expect = False
            i += 1
        self.scan_commas(lo, hi, False)

    def segment(self, start, end):
        """The statement tokens[start..end) (`end` is its `;`, or the `{` that follows heads and labels)."""
        tokens, pair = self.tokens, self.pair
        j = start
        while True:  # annotations (`CERASTES_INVARIANT(...)`, `[[cerastes::...]]`), labels, heads
            while j < end and (tokens[j].kind == "ident" and tokens[j].text.startswith("CERASTES_")
                               and j + 1 < end and tokens[j + 1].text == "("
                               or tokens[j].kind == "op" and tokens[j].text == "["):
                j = pair[j + 1 if tokens[j].kind == "ident" else j] + 1
            j = skip_labels(tokens, pair, j, end)
            if j < end and tokens[j].kind == "ident" and tokens[j].text in _HEADS + ("switch",) \
                    and j + 1 < end and tokens[j + 1].kind == "op" and tokens[j + 1].text == "(":
                j = self.head(j)
                continue
            break
        if j >= end:
            return
        t = tokens[j]
        if t.kind == "ident" and t.text in _NO_EXPRESSION:
            return
        if t.kind == "ident" and t.text == "return":
            self.scan_commas(j + 1, end, True)
        elif _is_declaration(tokens, j, end):
            self.declaration(j, end)
        else:
            self.scan_commas(j, end, True)
            self.discard(j, end)

    def walk(self, open_i, close_i):
        tokens, pair = self.tokens, self.pair
        i = open_i + 1
        start = i
        while i < close_i:
            t = tokens[i]
            if t.kind == "op" and t.text == "{":
                self.segment(start, i)  # the heads and labels before the block
                self.walk(i, pair[i])
                i = pair[i] + 1
                start = i
                continue
            if t.kind == "op" and t.text in ("(", "["):
                i = pair[i] + 1
                continue
            if t.kind == "op" and t.text == "}":
                start = i + 1
            elif t.kind == "op" and t.text == ";":
                self.segment(start, i)
                start = i + 1
            elif t.kind == "ident" and t.text in ("else", "do") and start == i:
                start = i + 1
            i += 1


def _short(text: str) -> str:
    text = one_line(text)
    return text if len(text) <= STATEMENT_ID_CHARS else text[:STATEMENT_ID_CHARS - 3] + "..."


_NO_CALL_WORDS = _NOT_CALLS | {"assert"}


def _has_call(tokens: list[Token], lo: int, hi: int) -> bool:
    """Whether tokens[lo..hi) holds a call (an identifier followed by `(`)."""
    for k in range(lo, hi - 1):
        if tokens[k].kind == "ident" and tokens[k].text not in _NO_CALL_WORDS \
                and tokens[k + 1].kind == "op" and tokens[k + 1].text == "(":
            return True
    return False


def _logical_operand_end(tokens, pair, i, is_and):
    """Index after the right operand that starts at token `i`: the operand of `&&` stops at `&&` and `||`,
    the operand of `||` only at `||` (an `&&` binds tighter), both at `?`, `:`, `,`, `;`, an assignment or the
    closing bracket of the enclosing group."""
    if is_and:
        return _conjunct_end(tokens, pair, i)
    j = i
    while j < len(tokens):
        t = tokens[j]
        if t.kind == "op":
            if t.text in _OPEN:
                j = pair[j] + 1
                continue
            if t.text in _CLOSE or t.text in _LOGIC_STOP:
                break
        j += 1
    return j


def _logical_operand_start(tokens, pair, rev_pair, i, is_and):
    """Index of the first token of the left operand of the `&&` or `||` at token `i`."""
    if is_and:
        return _conjunct_start(tokens, pair, rev_pair, i)
    j = i - 1
    while j >= 0:
        t = tokens[j]
        if t.kind == "op":
            if t.text in _CLOSE:
                j = rev_pair[j] - 1
                continue
            if t.text in _OPEN or t.text in _LOGIC_STOP:
                break
        elif t.kind == "ident" and t.text in _LOGIC_STOP_WORDS:
            break
        j -= 1
    return j + 1


def _conditional_arms(tokens, pair, q, limit):
    """For the `?` at token `q`: (a0, a1, b0, b1), the middle operand tokens[a0..a1) and the operand
    tokens[b0..b1) after the matching `:` (a nested conditional in it included), or None when the shape
    is not one a swap keeps in the same grouping (a comma at the level of the middle operand, an
    assignment in the last operand, no matching `:`)."""
    j, depth, colon = q + 1, 0, None
    while j < limit:
        u = tokens[j]
        if u.kind == "op":
            if u.text in _OPEN:
                j = pair[j] + 1
                continue
            if u.text in _CLOSE or u.text in (";", ","):
                return None
            if u.text == "?":
                depth += 1
            elif u.text == ":":
                if depth == 0:
                    colon = j
                    break
                depth -= 1
        j += 1
    if colon is None or colon == q + 1:
        return None
    j, depth = colon + 1, 0
    while j < limit:
        u = tokens[j]
        if u.kind == "op":
            if u.text in _OPEN:
                j = pair[j] + 1
                continue
            if u.text in _CLOSE or u.text in (";", ","):
                break
            if u.text in _ASSIGN_OPS:
                return None
            if u.text == "?":
                depth += 1
            elif u.text == ":":
                if depth == 0:
                    break
                depth -= 1
        j += 1
    if j == colon + 1:
        return None
    return q + 1, colon, colon + 1, j



def effect_mutants(src, tokens, pair, rev_pair, bodies, in_macro, position, path) -> list[Mutant]:
    """The `drop-store`, `swap-incdec`, `drop-comma`, `swap-logical` and `swap-arms` mutants of the function
    bodies (see the module docstring).  Only code: never an argument of a `CERASTES_*` macro or of a `[[cerastes::...]]`
    attribute, and only where the value of the expression is used (the statements that discard it, the
    `for` init and step clauses and a cast to `void` are skipped; a comma is a comma operator only where it
    is not an argument, an initializer or a declarator separator)."""
    out: list[Mutant] = []

    def add(op, a, end, original, new, replacement):
        line, col = position(tokens[a].start)
        out.append(Mutant(path, line, col, op, _short(original), new, tokens[a].start, end, replacement))

    for o, c in bodies:
        ctx = _EffectContext(tokens, pair, rev_pair)
        ctx.walk(o, c)
        for k in range(o + 1, c):
            if in_macro[k]:
                continue
            t = tokens[k]
            if t.kind != "op":
                continue
            if t.text == "=" and k not in ctx.init_eq and k > o + 1 and tokens[k - 1].kind == "ident" \
                    and k + 1 < c and tokens[k - 1].text not in _NOT_CALLS:
                before = tokens[k - 2]
                if before.kind == "ident" and before.text != "return" or before.kind == "op" \
                        and before.text not in _OPERAND_BEFORE:
                    continue
                end = k + 1  # the right operand ends at a `,` `;` or closing bracket (or the `:` of a `?:`)
                ternary = 0
                while end < c:
                    u = tokens[end]
                    if u.kind == "op":
                        if u.text in _OPEN:
                            end = pair[end] + 1
                            continue
                        if u.text in _CLOSE or u.text in (",", ";"):
                            break
                        if u.text == "?":
                            ternary += 1
                        elif u.text == ":":
                            if ternary == 0:
                                break
                            ternary -= 1
                    end += 1
                if end == k + 1 or (k - 1, end) in ctx.discarded:
                    continue
                add("drop-store", k - 1, tokens[k + 1].start, f"{tokens[k - 1].text} =", "", "")
            elif t.text in ("++", "--"):
                other = "--" if t.text == "++" else "++"
                prv = tokens[k - 1]
                nxt = tokens[k + 1] if k + 1 < c else None
                if prv.kind == "ident" and prv.text not in _NOT_CALLS and (
                        k < 2 or tokens[k - 2].text not in (".", "->", "::")):
                    span, name, postfix = (k - 1, k + 1), prv.text, True  # x++
                elif nxt is not None and nxt.kind == "ident" and nxt.text not in _NOT_CALLS and not (
                        k + 2 < len(tokens)
                        and tokens[k + 2].text in (".", "->", "[", "(", "::", "++", "--")):
                    span, name, postfix = (k, k + 2), nxt.text, False  # ++x
                else:
                    continue
                if span in ctx.discarded:
                    continue
                swapped = f"{t.text}{name}" if postfix else f"{name}{t.text}"
                add("swap-incdec", span[0], tokens[span[1] - 1].end, f"{name}{t.text}" if postfix
                    else f"{t.text}{name}", swapped, swapped)
                add("swap-incdec", k, t.end, t.text, other, other)
        for k, a, b in ctx.commas:
            if k <= o or k >= c or in_macro[k] or a >= b:
                continue
            if _has_effect(tokens, a, b):
                add("drop-comma", a, tokens[k + 1].start, src[tokens[a].start:tokens[k].end], "", "")
        for k in range(o + 1, c):
            t = tokens[k]
            if in_macro[k] or t.kind != "op":
                continue
            if t.text in ("&&", "||"):
                is_and = t.text == "&&"
                a = _logical_operand_start(tokens, pair, rev_pair, k, is_and)
                b = _logical_operand_end(tokens, pair, k + 1, is_and)
                if a <= o or b > c or a >= k or b <= k + 1:
                    continue
                if not (_has_call(tokens, a, k) or _has_call(tokens, k + 1, b)):
                    continue
                left = src[tokens[a].start:tokens[k - 1].end]
                right = src[tokens[k + 1].start:tokens[b - 1].end]
                if one_line(left) == one_line(right):
                    continue
                add("swap-logical", a, tokens[b - 1].end, f"{left} {t.text} {right}",
                    f"{right} {t.text} {left}", f"{right} {t.text} {left}")
            elif t.text == "?":
                arms = _conditional_arms(tokens, pair, k, c)
                if arms is None:
                    continue
                a0, a1, b0, b1 = arms
                left = src[tokens[a0].start:tokens[a1 - 1].end]
                right = src[tokens[b0].start:tokens[b1 - 1].end]
                if one_line(left) == one_line(right):
                    continue
                add("swap-arms", a0, tokens[b1 - 1].end, f"{left} : {right}", f"{right} : {left}",
                    f"{right} : {left}")
    return out


def is_user_label(tokens: list[Token], i: int, semi: int) -> bool:
    """Whether tokens[i] is the name of a `goto` label: `name :` (the `:` is a token of its own:
    `name::x` lexes as `::`), where `name` is not `case`, `default` or another keyword."""
    return (tokens[i].kind == "ident" and tokens[i].text not in _COMPOUND_KEYWORDS
            and tokens[i].text not in _DECL_KEYWORDS and i + 1 < semi
            and tokens[i + 1].kind == "op" and tokens[i + 1].text == ":")


def is_goto(tokens: list[Token], i: int, semi: int) -> bool:
    """Whether tokens[i..semi] is `goto name ;` (the `;` is tokens[semi]): a forward jump the
    mutation may delete.  A computed `goto *p;` is not (its target is no name)."""
    return (tokens[i].kind == "ident" and tokens[i].text == "goto" and i + 2 == semi
            and tokens[i + 1].kind == "ident")


def skip_labels(tokens: list[Token], pair: dict[int, int], first: int, semi: int) -> int:
    """Index of the first token after the labels that open tokens[first..semi]: `case <value>:`,
    `default:` and a `goto` label `name:` (`first` itself when there is none or a `case` label is
    malformed)."""
    i = first
    while i < semi and ((tokens[i].kind == "ident" and tokens[i].text in ("case", "default"))
                        or is_user_label(tokens, i, semi)):
        if is_user_label(tokens, i, semi):
            i += 2
            continue
        j = i + 1
        if tokens[i].text == "case":
            ternary = 0
            while j < semi:
                u = tokens[j]
                if u.kind == "op" and u.text in ("(", "["):
                    j = pair[j]
                elif u.kind == "op" and u.text == "?":
                    ternary += 1
                elif u.kind == "op" and u.text == ":":
                    if ternary == 0:
                        break
                    ternary -= 1
                j += 1
        if j >= semi or tokens[j].kind != "op" or tokens[j].text != ":":
            return first
        i = j + 1
    return i


def deletable_target(tokens: list[Token], pair: dict[int, int], first: int, semi: int):
    """The first token of the part of the statement tokens[first..semi] that is deleted, or
    None: the statement itself; the statement after its `case`/`default` labels; or the
    `break;`/`continue;` that is the body of `if (...)`, `while (...)` and `for (...)` heads
    (any number of them), after the labels."""
    body = skip_labels(tokens, pair, first, semi)
    if is_deletable(tokens, body, semi):
        return body
    i = body
    while (i < semi and tokens[i].kind == "ident" and tokens[i].text in _HEADS
           and i + 1 < semi and tokens[i + 1].kind == "op" and tokens[i + 1].text == "("):
        i = pair[i + 1] + 1
    if i != body and i == semi - 1 and tokens[i].kind == "ident" and tokens[i].text in _JUMPS:
        return i
    if i != body and is_goto(tokens, i, semi):
        return i
    return None


def is_deletable(tokens: list[Token], first: int, semi: int) -> bool:
    """Whether tokens[first..semi] is a simple statement that may be deleted: not
    empty, not a declaration, a check, an attribute or a compound statement."""
    if first >= semi:
        return False
    if is_goto(tokens, first, semi):
        return True
    t = tokens[first]
    if t.kind == "op" and t.text in ("[", "{", "}"):
        return False
    if t.kind != "ident":
        return t.kind == "op" and t.text in ("(", "++", "--", "*")
    if t.text.startswith("CERASTES_") or t.text in _DECL_KEYWORDS or t.text in _COMPOUND_KEYWORDS:
        return False
    nxt = tokens[first + 1] if first + 1 < semi else None
    if nxt is not None and nxt.kind == "ident" and t.text != "return":
        return False  # `T name ...`: a declaration with a typedef name
    if t.text in ("if", "while", "for", "switch"):
        return False
    return True


# ---------------------------------------------------------------------------
# The equivalence list
# ---------------------------------------------------------------------------

@dataclasses.dataclass
class Equivalents:
    entries: dict[str, str]  # id -> reason
    reviewed: str | None
    problems: list[str]


def read_equivalents(path: Path) -> Equivalents:
    """`id<TAB>reason` lines, `#` comments, a `# reviewed:` line when there are
    entries.  A missing file is an empty list."""
    entries: dict[str, str] = {}
    reviewed = None
    problems: list[str] = []
    if not path.is_file():
        return Equivalents(entries, None, problems)
    name = rt.rel(path)
    for lineno, raw in enumerate(path.read_text().split("\n"), start=1):
        if not raw.strip():
            continue
        if raw.lstrip().startswith("#"):
            if m := REVIEWED_RE.match(raw.strip()):
                reviewed = m.group("who")
            continue
        fields = raw.split("\t")
        if len(fields) != 2 or not fields[0].strip() or not fields[1].strip():
            problems.append(f"{name}:{lineno}: a line is `<mutant id><TAB><reason>`")
            continue
        mid, reason = fields[0].strip(), fields[1].strip()
        if not ID_RE.match(mid):
            problems.append(f"{name}:{lineno}: `{mid}` is not a mutant id "
                            "(`path:line:column:op:original->new`)")
            continue
        if mid in entries:
            problems.append(f"{name}:{lineno}: `{mid}` is listed twice")
            continue
        entries[mid] = reason
    if entries and reviewed is None:
        problems.append(f"{name}: has entries but no `# reviewed: <who, when>` line")
    return Equivalents(entries, reviewed, problems)


# ---------------------------------------------------------------------------
# Selecting tests, running the verifier
# ---------------------------------------------------------------------------

@dataclasses.dataclass
class Subject:
    path: Path
    modes: list[str]
    source: str
    mutants: list[Mutant]
    safe: frozenset = frozenset()  # ids of the mutants in the safe-direction bucket (when they survive)


class Problem(Exception):
    """A usage or environment error (exit status 2)."""


def display_path(path: Path) -> str:
    return rt.rel(path)


def select(paths: list[str]) -> list[tuple[Path, list[str]]]:
    """The verified C and C++ tests under `paths`, with their modes."""
    try:
        sel = rt.discover(paths)
    except rt.UsageError as err:
        raise Problem(str(err)) from None
    out = []
    for f in sel.tests:
        exp = rt.read_header(f)
        if isinstance(exp, rt.Expectation) and f.suffix != ".vpr" and exp.kind == "verified":
            out.append((f, exp.modes))
    return out


def tree_has_verified_tests() -> bool:
    paths = [str(ROOT / p) for p in rt.DEFAULT_PATHS if (ROOT / p).is_dir()]
    if not paths:
        return False
    try:
        return bool(select(paths))
    except Problem:
        return True  # a tree we cannot read is not known to have none


def generate(chosen: list[tuple[Path, list[str]]]) -> tuple[list[Subject], list[str]]:
    """The subjects with their mutants, and the problems found while generating."""
    subjects: list[Subject] = []
    problems: list[str] = []
    seen: dict[str, str] = {}
    for f, modes in chosen:
        name = display_path(f)
        src = rt.read_exact(f)
        try:
            muts = mutants_of(src, name)
        except LexError as err:
            problems.append(f"{name}: cannot lex: {err}")
            continue
        spans = function_spans(src)
        if not muts:
            problems.append(f"{name}: no mutant is generated for this test: a subject with none "
                            "would pass the mutation run without being tested")
        if not spans:
            problems.append(f"{name}: no function body found: a verified test has at least one, "
                            "and a subject without one would pass vacuously")
        for fname, fline, fstart, fend in spans:
            if not any(fstart <= m.start < fend for m in muts):
                problems.append(f"{name}:{fline}: the body of `{fname}` has no mutation target: "
                                "nothing in it would notice a change (add a check of its result "
                                "or give it code to mutate)")
        for m in muts:
            if m.apply(src) == src:
                problems.append(f"{m.id}: the mutant is identical to the original")
            if m.id in seen:
                problems.append(f"{m.id}: generated twice")
            seen[m.id] = name
        subjects.append(Subject(f, modes, src, muts, frozenset(m.id for m in muts if is_safe_direction(m, src))))
    return subjects, problems


def generate_listed(eq: Equivalents, selected: set[str]) -> tuple[list[Subject], list[str]]:
    """Subjects for the files that the equivalence list names and the selection does not contain,
    the problems (an entry whose file does not exist or is not a `// expect: verified` C or C++ test;
    the problems of generating the file)."""
    listed = sorted({entry_path(mid) for mid in eq.entries} - selected)
    chosen: list[tuple[Path, list[str]]] = []
    problems: list[str] = []
    for name in listed:
        path = Path(name) if Path(name).is_absolute() else ROOT / name
        why = None
        if not path.is_file():
            why = "the file does not exist"
        else:
            try:
                picked = select([str(path)])
            except Problem:
                picked = []
            if len(picked) != 1 or display_path(picked[0][0]) != name:
                why = "the file is not a `// expect: verified` C or C++ test"
            else:
                chosen.append(picked[0])
        if why is not None:
            for mid in sorted(m for m in eq.entries if entry_path(m) == name):
                problems.append(f"stale equivalence entry ({why}): {mid}")
    subjects, generated_problems = generate(chosen)
    return subjects, problems + generated_problems


def verify_base(env: dict[str, str]) -> list[str]:
    return rt.VERIFY_COMMAND if rt.VERIFY_COMMAND is not None else [env["CERASTES_VERIFY"]]


def run_verifier(env: dict[str, str], file: Path, mode: str, include_dir: Path) -> rt.ToolRun:
    """Verify `file`, a copy of a test: `-I <the test's directory>` makes the includes
    that are relative to the test (`#include "headers/h.h"`) resolve from the copy."""
    cmd = [*verify_base(env), "verify", str(file), rt.VERIFY_STD_OPTIONS[mode],
           f"-I{include_dir}"]
    return rt.run_tool(cmd, env, rt.VERIFY_TIMEOUT_S)


KILLED, SURVIVED, STILLBORN, FAILED = "killed", "survived", "stillborn", "tool failure"


def status_of(run: rt.ToolRun) -> tuple[str, str]:
    """(outcome, detail) of one verifier run."""
    if run.returncode is None:
        return FAILED, f"cerastes verify {run.error}"
    if run.returncode < 0:
        return FAILED, f"cerastes verify crashed (signal {-run.returncode})"
    code = run.returncode
    if code == rt.VERIFY_OK:
        return SURVIVED, ""
    if code == rt.VERIFY_ERRORS:
        return KILLED, ""
    if code == rt.VERIFY_REJECTED:
        return STILLBORN, ""
    tail = [l for l in (run.stderr or run.stdout).split("\n") if l.strip()]
    return FAILED, f"cerastes verify exited with status {code}: " + " | ".join(tail)[:200]


def combine(outcomes: list[str]) -> str:
    """One outcome for a mutant from its outcomes in each mode."""
    if FAILED in outcomes:
        return FAILED
    if SURVIVED in outcomes:
        return SURVIVED
    if KILLED in outcomes:
        return KILLED
    return STILLBORN


@dataclasses.dataclass
class Result:
    baseline_failures: list[str]
    outcomes: dict[str, str]  # mutant id -> outcome
    failures: list[str]  # tool failures
    baseline_failed_paths: list[Path] = dataclasses.field(default_factory=list)


def run_all(subjects: list[Subject], env: dict[str, str], jobs: int, workdir: Path) -> Result:
    result = Result([], {}, [])
    work: list[tuple[str, Subject, Mutant | None, str]] = []
    for s in subjects:
        for mode in s.modes:
            work.append(("baseline", s, None, mode))
    counter = itertools.count()

    def run_one(item):
        kind, s, m, mode = item
        d = workdir / str(next(counter))
        d.mkdir(parents=True, exist_ok=True)
        f = d / s.path.name
        rt.write_exact(f, s.source if m is None else m.apply(s.source))
        run = run_verifier(env, f, mode, s.path.parent)
        shutil.rmtree(d, ignore_errors=True)
        return item, status_of(run)

    def run_batch(pool, items):
        """The outcomes of `items`, in the order of `items`.  One line per finished verifier run goes
        to stderr as soon as it finishes (in completion order, flushed), so that a run stopped by a time
        cap still leaves its outcomes in a log: `mutate: [k/N] <mutant id or "<path> (original)"> [<mode>]:
        <outcome>`, k and N counting the originals and the mutants apart."""
        futures = {pool.submit(run_one, item): index for index, item in enumerate(items)}
        results: list = [None] * len(items)
        for done, future in enumerate(concurrent.futures.as_completed(futures), start=1):
            item, (outcome, detail) = results[futures[future]] = future.result()
            _kind, subject, mutant, mode = item
            if mutant is None:  # the original must verify: exit status 0 is "verified", anything else is a failure
                name = f"{display_path(subject.path)} (original)"
                outcome = {SURVIVED: "verified", KILLED: "does not verify", STILLBORN: "rejected"}.get(outcome, outcome)
            else:
                name = mutant.id
            print(f"mutate: [{done}/{len(items)}] {name} [{mode}]: {outcome}", file=sys.stderr, flush=True)
        return results

    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, jobs)) as pool:
        baseline = run_batch(pool, work)
        bad_files: set[Path] = set()
        for (kind, s, m, mode), (outcome, detail) in baseline:
            if outcome == FAILED:
                bad_files.add(s.path)
                result.failures.append(f"{display_path(s.path)} [{mode}] (the original): {detail}")
            elif outcome != SURVIVED:  # the original must verify (exit status 0)
                bad_files.add(s.path)
                result.baseline_failures.append(
                    f"{display_path(s.path)} [{mode}]: the original does not verify "
                    f"({'exit status 1: verification errors' if outcome == KILLED else detail or 'rejected (exit status 2)'})")
        result.baseline_failed_paths = sorted(bad_files)
        mutant_work = [("mutant", s, m, mode) for s in subjects if s.path not in bad_files
                       for m in s.mutants for mode in s.modes]
        per: dict[str, list[str]] = {}
        for (kind, s, m, mode), (outcome, detail) in run_batch(pool, mutant_work):
            per.setdefault(m.id, []).append(outcome)
            if outcome == FAILED:
                result.failures.append(f"{m.id} [{mode}]: {detail}")
        for mid, outs in per.items():
            result.outcomes[mid] = combine(outs)
    return result


def entry_path(mid: str) -> str:
    """The file of a mutant id (the id has the form `path:line:column:op:original->new`)."""
    m = ID_RE.match(mid)
    return m.group("path") if m else mid


def check_equivalents(eq: Equivalents, generated: set[str],
                      outcomes: dict[str, str] | None, safe: frozenset | set = frozenset(),
                      files: set[str] | frozenset[str] | None = None) -> list[str]:
    """Stale entries: not generated; covered by the safe-direction bucket; with `outcomes`
    also not surviving.  With `files`, only the entries of those files are checked: a run over some
    of the tests knows nothing about the mutants of the others."""
    problems = list(eq.problems)
    for mid in sorted(eq.entries):
        if files is not None and entry_path(mid) not in files:
            continue
        if mid in safe:
            problems.append(f"stale equivalence entry (the safe-direction bucket covers this mutant: "
                            f"a stronger requires, or a weaker ensures or check): {mid}")
        elif mid not in generated:
            problems.append(f"stale equivalence entry (no such mutant is generated): {mid}")
        elif outcomes is not None and outcomes.get(mid) != SURVIVED:
            problems.append(f"stale equivalence entry (the mutant is "
                            f"{outcomes.get(mid, 'not run')}, not a survivor): {mid}")
    return problems


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("paths", nargs="*", help=f"tests or directories (default: "
                        f"{', '.join(DEFAULT_PATHS)})")
    parser.add_argument("--validate", action="store_true",
                        help="only generate and check the mutants and the equivalence list")
    parser.add_argument("-j", "--jobs", type=int, default=None,
                        help=f"parallel mutants (default: ${rt.JOBS_VARIABLE} from the shell if set, "
                             "else half the CPUs; an explicit -j wins)")
    parser.add_argument("--list", action="store_true",
                        help="print every mutant id and exit (with --validate: and validate)")
    parser.add_argument("--equivalents", type=Path, default=ROOT / EQUIVALENT_FILE,
                        help=argparse.SUPPRESS)
    args = parser.parse_args(argv)
    paths = args.paths or [str(ROOT / p) for p in DEFAULT_PATHS]

    existing = [p for p in paths if Path(p).exists()]
    if args.validate and not args.paths and not existing:
        chosen: list[tuple[Path, list[str]]] = []
    else:
        try:
            chosen = select(paths)
        except Problem as err:
            if args.validate and not args.paths and "not a test file or directory" in str(err):
                chosen = []
            else:
                print(f"mutate: {err}", file=sys.stderr)
                return 2
    if not chosen:
        if args.validate and not tree_has_verified_tests():
            print("mutate: no verified tests to mutate yet")
            return 0
        print("mutate: no verified tests under " + ", ".join(paths), file=sys.stderr)
        return 1 if args.validate else 2

    subjects, problems = generate(chosen)
    generated = {m.id for s in subjects for m in s.mutants}
    selected_files = {display_path(s.path) for s in subjects}
    total = len(generated)
    eq = read_equivalents(args.equivalents)
    if args.list:
        for mid in sorted(generated):
            print(mid)
        if not args.validate:
            for p in problems:  # generation problems are still problems, but nothing is run
                print(p, file=sys.stderr)
            return 1 if problems else 0  # --list only lists: it never starts a run
    if args.validate:
        safe_all = {mid for s in subjects for mid in s.safe}
        # the list may name files outside the selection (the doc examples; the default selection is
        # tests/scalar): generate each one, so that its entries are checked for staleness too
        extra_subjects, extra_problems = generate_listed(eq, selected_files)
        problems += extra_problems
        generated |= {m.id for s in extra_subjects for m in s.mutants}
        safe_all |= {mid for s in extra_subjects for mid in s.safe}
        checked_files = selected_files | {display_path(s.path) for s in extra_subjects}
        problems += check_equivalents(eq, generated, None, safe_all, checked_files)
        if total == 0:
            problems.append("no mutant is generated: an empty set must not pass")
        for p in problems:
            print(p)
        if problems:
            print(f"mutate: {len(problems)} problems in {len(subjects)} tests")
            return 1
        print(f"mutate: {len(subjects)} tests, {total} mutants generated, "
              f"{len(eq.entries)} reviewed equivalents, {len(safe_all)} in the safe-direction bucket "
              "if they survive"
              + (f" ({len(extra_subjects)} more files named by the list were generated too)"
                 if extra_subjects else ""))
        return 0

    if problems:
        for p in problems:
            print(p)
        print(f"mutate: {len(problems)} problems in the generated mutants")
        return 1
    env = rt.load_env()
    try:
        if args.jobs is None:  # an explicit -j wins: the variable is consulted only here
            args.jobs = rt.default_jobs()
        if rt.VERIFY_COMMAND is None:
            rt.require(env, "CERASTES_VERIFY")
            if (problem := rt.edg_tree_problem()) is not None \
                    or (problem := rt.edg_stamp_problem()) is not None:
                raise rt.UsageError(problem)
    except rt.UsageError as err:
        print(f"mutate: {err}", file=sys.stderr)
        return 2
    parent = ROOT / "build"
    with tempfile.TemporaryDirectory(prefix="mutants-",
                                     dir=parent if parent.is_dir() else None) as tmp:
        result = run_all(subjects, env, args.jobs, Path(tmp))
    counts = {k: sum(1 for o in result.outcomes.values() if o == k)
              for k in (KILLED, SURVIVED, STILLBORN, FAILED)}
    survivors = sorted(mid for mid, o in result.outcomes.items() if o == SURVIVED)
    safe_all = {mid for s in subjects for mid in s.safe}
    safe_survivors = [mid for mid in survivors if mid in safe_all]
    unexplained = [mid for mid in survivors if mid not in eq.entries and mid not in safe_all]
    stale = check_equivalents(eq, generated, result.outcomes, safe_all, selected_files)
    # a test none of whose mutants was killed notices nothing: all survived, all stillborn,
    # or a mix; it fails the run even if each survivor were reviewed
    unmutated: list[str] = []
    for subject in subjects:
        if subject.path in result.baseline_failed_paths:
            continue  # reported as a baseline failure
        outs = [result.outcomes.get(m.id) for m in subject.mutants]
        if subject.mutants and KILLED not in outs:
            stillborn_n, survived_n = outs.count(STILLBORN), outs.count(SURVIVED)
            unmutated.append(f"NO KILLED MUTANT {display_path(subject.path)}: of {len(outs)} "
                             f"mutants {survived_n} survived and {stillborn_n} were stillborn")
    for line in result.baseline_failures:
        print("BASELINE " + line)
    for line in result.failures:
        print("TOOL FAILURE " + line)
    for mid in unexplained:
        print("SURVIVED " + mid)
    for line in stale:
        print(line)
    for line in unmutated:
        print(line)
    for subject in subjects:
        n = sum(1 for mid in safe_survivors if mid in subject.safe and any(m.id == mid for m in subject.mutants))
        if n:
            print(f"safe-direction {display_path(subject.path)}: {n}")
    stillborn = sorted(mid for mid, o in result.outcomes.items() if o == STILLBORN)
    if stillborn:
        print(f"stillborn (not counted): {len(stillborn)}")
        for mid in stillborn:
            print("  " + mid)
    print(f"mutate: {len(subjects)} tests, {total} mutants: {counts[KILLED]} killed, "
          f"{counts[SURVIVED] - len(unexplained) - len(safe_survivors)} equivalent (reviewed), "
          f"{len(safe_survivors)} safe-direction, "
          f"{len(unexplained)} unexplained survivors, {counts[STILLBORN]} stillborn, "
          f"{counts[FAILED]} tool failures")
    if result.failures:
        return 3
    return 1 if (unexplained or stale or unmutated or result.baseline_failures) else 0


if __name__ == "__main__":
    sys.exit(main())
