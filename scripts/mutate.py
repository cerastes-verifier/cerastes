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
  `return`) is replaced by `;`.  Declarations, checks (`CERASTES_*`), `static_assert`
  and compound statements are never deleted.

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
* the clause is linear: only `+ - < <= > >= == != && || !` and parentheses, decimal
  literals without a `u` suffix, and `*` only with a literal operand (no product of two
  variables, no `/`, `%`, `<<`, `>>`, bit operations, `?:`, casts, calls or subscripts);
* the mutation is not an `==` <-> `!=` flip (a flip of equality is `unknown`);
* for a stronger `requires` or a weaker `ensures` (not a `check`), the function is used
  nowhere else in the file (a call, a recursive call, its address, a mention in a
  contract): a use means that the mutant must be killed by a failure at the use, or be
  listed (a caller may fail the stronger precondition, or rely on the weaker
  postcondition).

A pair that cannot be evaluated (a construct the grid does not know) is `unknown` too.
Known gaps remain, listed in the handoff and deferred to the triage session: C precedence
of `!` and chained comparisons, literal coefficients of `*`, the well-definedness of a
contract after a dropped guard, and uses of the function inside a `#define` body or a
default argument.
The other contract mutants, a weaker `requires`, a
stronger `ensures` or `check`, any change of an `invariant` or a `decreases`, one
that leaves the clause's meaning unchanged, and every mutant of code, are never
bucketed: each must be killed or listed.

`tests/mutants-equivalent.txt` lists the survivors that were reviewed and are
equivalent (or intended), apart from those of the safe-direction bucket: lines `id<TAB>reason`, `#` comments, and a
`# reviewed: <who, when>` line whenever there are entries.  An entry that names
a mutant which is not generated, which does not survive, or which the
safe-direction bucket already covers, is stale and fails the run (so a fixed test or an edited source cannot leave a stale excuse).

`--list` generates, prints the id of every mutant (sorted) and exits 0 without running
anything: it needs no verifier and no environment (a source that cannot be generated is
reported on stderr and exits 1).  `--validate` only generates: it checks that the sources lex (balanced braces and
parentheses), that every mutant differs from its original, that the ids are
unique, and the equivalence list (format and staleness against the generated
ids).  It needs no verifier and runs in `scripts/check.sh`; the full run (about
six minutes for tests/scalar) is an acceptance step.  With no verified test under
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
OPS = ("relop", "literal", "drop-conjunct", "swap-logic", "delete-stmt")
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


def lex(src: str) -> list[Token]:
    """Tokens of `src` without comments and preprocessor lines (a `#` line, with
    its backslash continuations, produces nothing: a macro body is never mutated)."""
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
                      "break", "continue", "try", "catch"}


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
    out.sort(key=lambda m: (m.start, m.op, m.end, m.new))
    return out



# ---------------------------------------------------------------------------
# The safe-direction bucket
# ---------------------------------------------------------------------------

SAFE = "safe-direction"
_CONTRACT_RE = re.compile(r"CERASTES_(REQUIRES|ENSURES|CHECK|INVARIANT|DECREASES)\s*\(")


def contract_clause(src: str, offset: int):
    """(macro name, clause start, clause end, result name) of the `CERASTES_*(...)` call
    whose argument contains `offset`, with the result name of `ENSURES(r, ...)` left out
    of the clause (the name is the fourth item, None when there is none), or None when
    the offset is not in the argument of one."""
    for m in _CONTRACT_RE.finditer(src):
        i = m.end()
        depth, j = 1, i
        while j < len(src) and depth:
            depth += (src[j] == "(") - (src[j] == ")")
            j += 1
        if not (i <= offset < j - 1):
            continue
        start, end = i, j - 1
        binder = None
        if m.group(1) == "ENSURES":
            b = re.match(r"\s*([A-Za-z_]\w*)\s*,", src[start:end])
            if b:
                start += b.end()
                binder = b.group(1)
        return m.group(1), start, end, binder
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


def _as_python(clause: str) -> str:
    c = re.sub(r"(\d)[uUlL]+\b", r"\1", clause)
    if "?" in c or "sizeof" in c or "[" in c or "->" in c or "." in re.sub(r"\d\.\d", "", c):
        raise _Unknown(clause)
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


def _holds(code, env) -> bool:
    try:
        return bool(eval(code, {"_cdiv": _cdiv, "_cmod": _cmod, "__builtins__": {}}, env))
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
        orig_implies_mut = mut_implies_orig = True
        for combo in itertools.product(values, repeat=len(names)):
            env = dict(zip(names, combo))
            a, b = _holds(po, env), _holds(pm, env)
            orig_implies_mut &= not (a and not b)
            mut_implies_orig &= not (b and not a)
            if not orig_implies_mut and not mut_implies_orig:
                return "incomparable"
    except (_Unknown, SyntaxError, NameError, TypeError):
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
# unknown.  `*` is checked apart (a product needs a literal operand).
_LINEAR_OPS = {"+", "-", "*", "<", "<=", ">", ">=", "==", "!=", "&&", "||", "!", "(", ")"}
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
    may declare (a declaration `T name` can shadow a parameter in a nested block)."""
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
    return out


def has_caller(src: str, name: str) -> bool:
    """Whether the identifier `name` occurs in a function body or in a contract of the file (a
    call, a recursive call, the address taken: any use a stronger `requires` could break)."""
    tokens = lex(src)
    pair = matching(tokens)
    inside = [False] * len(tokens)
    for o, c in function_bodies(tokens, pair) + macro_argument_ranges(tokens, pair):
        for k in range(o + 1, c):
            inside[k] = True
    return any(t.kind == "ident" and t.text == name and inside[k] for k, t in enumerate(tokens))


def clause_is_modelled(clause: str, allowed: frozenset) -> bool:
    """Whether every identifier of `clause` is in `allowed` (or `true`/`false`) and the clause
    is linear over them: only the operators of `_LINEAR_OPS`, decimal literals without an
    unsigned suffix, and a `*` only between a literal and a variable or literal."""
    try:
        tokens = lex(clause)
    except LexError:
        return False
    for k, t in enumerate(tokens):
        if t.kind == "ident":
            if t.text not in allowed and t.text not in ("true", "false"):
                return False
        elif t.kind == "number":
            if not _DECIMAL_RE.match(t.text):
                return False
        elif t.kind == "op":
            if t.text not in _LINEAR_OPS:
                return False
            if t.text == "*":
                left = tokens[k - 1] if k > 0 else None
                right = tokens[k + 1] if k + 1 < len(tokens) else None
                if left is None or right is None:
                    return False
                if not ({left.kind, right.kind} <= {"ident", "number"} and "number" in (left.kind, right.kind)):
                    return False
                far_left = tokens[k - 2] if k > 1 else None
                far_right = tokens[k + 2] if k + 2 < len(tokens) else None
                if (far_left is not None and far_left.text == "*") or \
                        (far_right is not None and far_right.text == "*"):
                    return False  # `x * 2 * y`: a product of variables
        else:
            return False  # a string or character literal
    return True


def is_safe_direction(m: "Mutant", src: str) -> bool:
    """A mutant of a contract that makes it easier to meet: a stronger `requires`, a
    weaker `ensures` or `check`.  Never a mutant of code, and never an `invariant` or
    a `decreases`.  Bucketed only when the grid models the clause (see the module
    docstring): every variable a signed builtin integer parameter of the enclosing function (the
    `ensures` result only if the return type is one), a linear clause, no `==`/`!=` flip,
    and, for a stronger `requires` or a weaker `ensures`, no use of the function in the file."""
    found = contract_clause(src, m.start)
    if found is None or found[0] not in ("REQUIRES", "ENSURES", "CHECK"):
        return False
    if m.op == "relop" and m.original in ("==", "!="):
        return False  # a flip of equality is `unknown`: never bucketed
    sig = next((s for s in function_signatures(src) if s.decl_start <= found[1] < s.body_end), None)
    if sig is None:
        return False  # a contract of a bodyless declaration: no signature to read the types from
    allowed = set(sig.signed_params)
    if found[0] == "ENSURES" and sig.returns_signed and found[3] is not None:
        allowed.add(found[3])
    if found[0] == "CHECK":
        allowed -= sig.body_decl_names  # a local of the same name may shadow the parameter
    if found[0] in ("REQUIRES", "ENSURES") and has_caller(src, sig.name):
        # a caller may fail the stronger precondition, or rely on the postcondition that is now
        # weaker: the mutant must be killed by a failure at the use, or be listed
        return False
    if not clause_is_modelled(src[found[1]:found[2]], frozenset(allowed)):
        return False
    # the start of the clause is before the changed text (or is it), so it is inside the same call
    # in the mutated source, also when the whole tail of the clause was dropped
    msrc = m.apply(src)
    mutated = contract_clause(msrc, found[1])
    if mutated is None or mutated[0] != found[0]:
        return False
    mutant_clause = msrc[mutated[1]:mutated[2]]
    if not clause_is_modelled(mutant_clause, frozenset(allowed)):
        return False
    relation = clause_relation(src[found[1]:found[2]], mutant_clause)
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
            if is_deletable(tokens, start, i):
                line, col = position(tokens[start].start)
                text = src[tokens[start].start:t.end]
                short = one_line(text)
                if len(short) > STATEMENT_ID_CHARS:
                    short = short[:STATEMENT_ID_CHARS - 3] + "..."
                out.append(Mutant(path, line, col, "delete-stmt", short, ";",
                                  tokens[start].start, t.end, ";"))
            start = i + 1
        elif t.kind == "ident" and t.text in ("else", "do") and start == i:
            start = i + 1
        i += 1
    return out


def is_deletable(tokens: list[Token], first: int, semi: int) -> bool:
    """Whether tokens[first..semi] is a simple statement that may be deleted: not
    empty, not a declaration, a check, an attribute or a compound statement."""
    if first >= semi:
        return False
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

    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, jobs)) as pool:
        baseline = list(pool.map(run_one, work))
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
        for (kind, s, m, mode), (outcome, detail) in pool.map(run_one, mutant_work):
            per.setdefault(m.id, []).append(outcome)
            if outcome == FAILED:
                result.failures.append(f"{m.id} [{mode}]: {detail}")
        for mid, outs in per.items():
            result.outcomes[mid] = combine(outs)
    return result


def check_equivalents(eq: Equivalents, generated: set[str],
                      outcomes: dict[str, str] | None, safe: frozenset | set = frozenset()) -> list[str]:
    """Stale entries: not generated; covered by the safe-direction bucket; with `outcomes`
    also not surviving."""
    problems = list(eq.problems)
    for mid in sorted(eq.entries):
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
    parser.add_argument("-j", "--jobs", type=int, default=max(1, (os.cpu_count() or 2) // 2))
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
        problems += check_equivalents(eq, generated, None, safe_all)
        if total == 0:
            problems.append("no mutant is generated: an empty set must not pass")
        for p in problems:
            print(p)
        if problems:
            print(f"mutate: {len(problems)} problems in {len(subjects)} tests")
            return 1
        print(f"mutate: {len(subjects)} tests, {total} mutants generated, "
              f"{len(eq.entries)} reviewed equivalents, {len(safe_all)} in the safe-direction bucket "
              "if they survive")
        return 0

    if problems:
        for p in problems:
            print(p)
        print(f"mutate: {len(problems)} problems in the generated mutants")
        return 1
    env = rt.load_env()
    try:
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
    stale = check_equivalents(eq, generated, result.outcomes, safe_all)
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
