#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Lint the Viper that `cerastes verify` emits (invariant 4; internal/encoding.md
section 6, decision 0012).

    python3 scripts/lint_viper.py [path ...]

A path is a directory (every `*.vpr` under it, and its `MANIFEST`) or a `.vpr`
file.  The default is `build/vpr`, where scripts/run_tests.py puts the
`--keep-vpr` output of the verify tests.  Each `<file>.vpr` is read with its
position map `<file>.vpr.map` (encoding.md section 5).  The lint fails (exit
status 1) on:

* `inhale` or `assume` without `// cerastes:inhale-ok <reason>` on its line
  (a reason is required);
* a bodyless `method` without `// cerastes:trusted-contract <reason>` on its
  `method` line or in the comment lines right above it (decision 0012 item 1:
  the contracts of functions defined in other units);
* any `domain` but `domain Block` preceded directly by `// cerastes:domain-ok 0028`, any `axiom` but the four of
  `BLOCK_AXIOMS` inside it (`loc_inv` 0033, `loc_nonnull` 0034, `member_inv` 0035, `member_lv` 0036: each once, with
  its exact text, whitespace-normalized, and `// cerastes:axiom-ok <its record>` directly above; the comment lines
  right above; the record has a file internal/decisions/NNNN-*.md: decision 0028 item 2, 0032 item 11), any method named `alloc_<t>`
  that is not the trusted allocation of decision 0032 item 4 (bodyless, exactly its header, two `ensures` lines and `decreases _`,
  as the emitter prints them), any bodyless
  top-level `function` (the prelude has bodies only, decision 0012 item 11; the functions inside the `Block`
  domain are not top-level), and Viper's `new(...)` anywhere (its results would not satisfy `WF`);
* an `assert`, `requires`, `ensures` or `invariant` line, or a call of a method
  declared in the file, with no entry in the position map for its `.vpr` line
  (every check is on its own line and has an entry, decision 0012 item 13).  The one exception
  (decision 0012 item 11 as amended for 0023, owner 2026-10-24): the range postcondition of a bitwise
  prelude function, `function c_[u]<and|or|xor>_<32|64|128>(a: Int, b: Int): Int` with exactly one
  header line `ensures <lo> <= result && result <= <hi>` (`(-2^(N-1)) ... 2^(N-1)-1` signed, `0 ...
  2^N-1` unsigned; Silicon proves it once per function, it is never assumed), which needs no map
  entry.  Any other `ensures`, a `requires` or a `decreases` in a prelude function, and an `ensures`
  of another shape in a bitwise one, still need an entry (so they are reported);
* a position map that is not in the format of encoding.md section 5, a map entry
  for a line that is not there (past the end, blank or a comment), the same
  Viper error id twice for one line, or a missing map;
* termination clauses (decision 0013 item 5): a method or `while` without a
  `decreases` clause, `decreases _` anywhere but on a bodyless method with the
  trusted-contract tag, `decreases *` (it assumes termination: invariant 4) and a
  conditional `decreases <measure> if <condition>`;
* jumps (decision 0014 item 9): a backward `goto` (a `goto L` whose `label L` is earlier in the
  same method: a cycle that Silicon does not check for termination), a `label` that carries
  `invariant` clauses (an unchecked loop) and a label defined twice in one method.  A forward
  `goto` and a label without invariants (`goto l_end` ... `label l_end`) are fine; and (decision 0015)
  a forward `goto` into a block: the `label` must sit in a block (a `{ ... }`: the method body, a
  `while` body, an `if` or `else` branch) that also encloses the `goto`, so a jump goes outward or
  stays in its block, never into a loop (which would skip the invariant check on entry) or a branch
  (which would skip its condition);
* the flags of locals without an initializer (decision 0019 item 5: only a write sets a flag, and a
  declaration, a block entry or a switch dispatch resets it): in a method, every `d_<name> := true` must
  directly follow (the previous line that is neither blank nor a comment) an assignment `<name> := ...` of
  the variable it flags, so a flag cannot be set without the write; every `d_<name> := false` (a reset) must
  directly precede `<name> := <dummy>`, an integer literal or `true`/`false` (the emitter writes the flag
  first, then the value: `d_v := false` and then `v := 0`), so a variable cannot be reset to a value that
  is then taken for a determinate one; and a flag is assigned nothing but `true` and `false`.  The
  snapshot of a loop (`ds_<name> := d_<name>`) and the reads of a flag are not assignments to it.  A dummy
  may also be `null` (the dummy of a `Ref` local).  The flag of an address-taken cell is a field (decision 0032
  item 11): `<recv>.d_<t> := true` standing alone on its line and directly after `<recv>.v_<t> := ...` (the
  same receiver and `<t>`), nothing else is assigned to a field flag (the allocation gives `false`);
* a `%` (Viper's modulo: the Euclidean remainder, never C's `%`) or a `\\` (Viper's floor division, never C's `/`)
  that is not Viper's exact mask or bit-function form (decision 0026 item 7: `a & (2^k-1)` is `a % 2^k`,
  `a & ~(2^k-1)` is `2^k * (a \\ 2^k)`): outside the bodies of the prelude functions that use the operator (`%`:
  `c_rem`, `c_urem`; `\\`: `c_div`, `c_udiv`, `c_shr`; by name), the divisor of every `%` and `\\` must be a plain
  decimal literal that is a power of two, 1 or more (`x % 256`, `16 * (x \\ 16)`).  The lint checks the form of the
  divisor only: that no C `%` or `/` is emitted as Viper's is guaranteed by `low::mod_pow2` and `low::div_pow2`
  (src/vir/lowered.cpp) being the only constructors of them, not by this rule (a C `/ 16` emitted as `\\ 16` would
  pass it); a `%` or `\\` whose divisor the rule cannot read as such a literal is reported too;
* an empty input set, and a MANIFEST that names a file which is not there.  The
  manifest is the set of files scripts/run_tests.py expects: its first line is
  `# run-id: <id>` (printed by the run), then one file per line.  Fewer files
  than it lists is a failure, so is an empty manifest, one without a run id,
  and a `.vpr` file that the manifest does not list (a stale file, or one from
  a partial run: the directory is cleared at the start of every run that
  selects a verify test).  With no MANIFEST and no
  `.vpr` file at all, the lint passes (`no emitted Viper to lint`) only when no
  verify test of the tree expects Viper (a `verified` or `error` test in C or
  C++): once one exists, an absent or empty `build/vpr` is a failure.

A comment is `//` to the end of the line, or `/* ... */`; a tag in a block
comment does not count.  Exit status: 0 clean, 1 problems, 2 usage error.
"""

from __future__ import annotations

import argparse
import dataclasses
import re
import sys
from pathlib import Path
from typing import Callable

sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_tests as rt  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
DEFAULT_DIR = Path("build") / "vpr"
MANIFEST = rt.VPR_MANIFEST
MAP_SUFFIX = ".map"

# The categories a map entry may carry: the driver's (decision 0011 item 10)
# and `internal`, for the checks whose failure is a bug of the encoder and exit
# status 3 (the automatic range invariants of decision 0012 item 8).
MAP_CATEGORIES = frozenset(rt.DRIVER_CATEGORIES | {"internal"})
# The Viper error ids Silicon's messages map to (the table of scripts/run_tests.py).
# `application.precondition` is not one of them: the prelude's functions have no preconditions
# (encoding.md section 5), so no emitted check can produce it.
MAP_ERROR_IDS = frozenset(viper_id for _text, viper_id, _cat in rt._VIPER_ERRORS
                          if viper_id != "application.precondition")

INHALE_TAG_RE = re.compile(r"//\s*cerastes:inhale-ok\b[ \t]*(?P<reason>\S.*)?$")
TRUSTED_TAG_RE = re.compile(r"//\s*cerastes:trusted-contract\b[ \t]*(?P<reason>\S.*)?$")
DOMAIN_TAG_RE = re.compile(r"//\s*cerastes:domain-ok\b[ \t]*(?P<record>\S+)?")
AXIOM_TAG_RE = re.compile(r"//\s*cerastes:axiom-ok\b[ \t]*(?P<record>\S+)?")
DECISIONS_DIR = ROOT / "internal" / "decisions"
BLOCK_DOMAIN_RE = re.compile(r"^\s*domain\s+Block\s*\{")
AXIOM_DECL_RE = re.compile(r"^\s*axiom\s+(?P<name>[A-Za-z_$][\w$']*)\s*\{")
NEW_RE = re.compile(r"(?<![\w$'.])new\s*\(")
INHALE_WORD_RE = re.compile(r"\b(inhale|assume)\b")
CHECK_LINE_RE = re.compile(r"^\s*(assert|requires|ensures|invariant|decreases)\b")
CHECK_WORD_RE = re.compile(r"\b(assert|requires|ensures|invariant|decreases)\b")
DECREASES_LINE_RE = re.compile(r"^\s*decreases\b(?P<measure>.*)$")
WHILE_WORD_RE = re.compile(r"(?<![\w$'])while\b")
# What ends a `decreases` clause that runs over several lines: a line that opens a block (`{`, `}`), starts
# another clause, a statement or a declaration (a line that starts with anything else continues the clause).
CLAUSE_END_RE = re.compile(r"^\s*(?:[{}]|(?:assert|requires|ensures|invariant|decreases|while|var|label|goto|inhale|"
                           r"exhale|assume|fold|unfold|apply|package|else|method|function|predicate|domain|field|"
                           r"define|import|axiom|adt|type)(?![\w$']))")
# The Viper error ids a position-map entry may have, by the kind of the line it is
# for (encoding.md section 5): an `assert` is `assert.failed`; a call of a method is
# `call.precondition`; an `invariant` clause is `invariant.not.established` and
# `invariant.not.preserved` (two entries on one line); a method `requires` is
# `not.wellformed` (a failed precondition is reported at the call, never at the clause);
# a method `ensures` is `not.wellformed` and `postcondition.violated`; a `decreases` line with a measure
# is `termination.failed` (a loop's measure is where Silicon reports a loop that does not terminate), and a
# call may also be `termination.failed` (a recursive call whose measure does not decrease, decision 0013).
KIND_IDS = {
    "assert": {"assert.failed"},
    "requires": {"not.wellformed"},
    "ensures": {"not.wellformed", "postcondition.violated"},
    "invariant": {"invariant.not.established", "invariant.not.preserved"},
    "call": {"call.precondition", "termination.failed"},
    "decreases": {"termination.failed"},
}
TOKEN_RE = re.compile(r"[A-Za-z_$][\w$']*|\d+|::|:=|\S")
TOP_KEYWORDS = ("method", "function", "predicate", "domain", "field", "define", "import")


# ---------------------------------------------------------------------------
# Reading Viper text
# ---------------------------------------------------------------------------

@dataclasses.dataclass
class Source:
    code: list[str]  # per line: the line without comments
    comments: list[str]  # per line: its `//` comment text ("" if none), with the `//`


def split_source(text: str) -> Source:
    """Split each line into code and `//` comment; block comments are blanked
    from the code (their text never counts as a tag)."""
    code: list[str] = []
    comments: list[str] = []
    in_block = False
    for raw in text.split("\n"):
        out = []
        comment = ""
        i = 0
        while i < len(raw):
            if in_block:
                j = raw.find("*/", i)
                if j < 0:
                    i = len(raw)
                else:
                    in_block = False
                    out.append(" ")
                    i = j + 2
                continue
            if raw.startswith("//", i):
                comment = raw[i:]
                break
            if raw.startswith("/*", i):
                in_block = True
                i += 2
                continue
            out.append(raw[i])
            i += 1
        code.append("".join(out))
        comments.append(comment)
    return Source(code, comments)


@dataclasses.dataclass
class Decl:
    kind: str
    name: str
    line: int  # 1-based line of the keyword
    has_body: bool = False
    body_line: int = 0  # 1-based line of the `{` that opens the body (0: none)
    end_line: int = 0  # 1-based last line of the declaration (set by declarations())


def declarations(src: Source) -> list[Decl]:
    """The top-level declarations, with whether each has a body."""
    tokens: list[tuple[str, int]] = []
    for lineno, line in enumerate(src.code, start=1):
        tokens += [(m.group(), lineno) for m in TOKEN_RE.finditer(line)]
    decls: list[Decl] = []
    depth = 0  # braces
    paren = 0
    i = 0
    n = len(tokens)
    trigger_end = -2  # index of the `}` that closed the last trigger set
    while i < n:
        text, lineno = tokens[i]
        prev = tokens[i - 1][0] if i else ""
        if text == "{":
            # a quantifier trigger set (`:: { f(x) }`, and the sets that follow it:
            # `{ f(x) } { g(x) }`), not a body: skip it
            if prev == "::" or i - 1 == trigger_end:
                level = 0
                while i < n:
                    if tokens[i][0] == "{":
                        level += 1
                    elif tokens[i][0] == "}":
                        level -= 1
                        if level == 0:
                            break
                    i += 1
                trigger_end = i
            else:
                if depth == 0 and paren == 0 and decls and not decls[-1].has_body:
                    decls[-1].has_body = True
                    decls[-1].body_line = lineno
                depth += 1
        elif text == "}":
            depth = max(0, depth - 1)
        elif text == "(":
            paren += 1
        elif text == ")":
            paren = max(0, paren - 1)
        elif depth == 0 and paren == 0 and text in TOP_KEYWORDS and prev != "::":
            name = tokens[i + 1][0] if i + 1 < n else ""
            decls.append(Decl(text, name, lineno))
        i += 1
    nlines = len(src.code)
    for k, d in enumerate(decls):
        d.end_line = decls[k + 1].line - 1 if k + 1 < len(decls) else nlines
    return decls


# ---------------------------------------------------------------------------
# Position maps (encoding.md section 5)
# ---------------------------------------------------------------------------

@dataclasses.dataclass(frozen=True)
class MapEntry:
    vpr_line: int
    error_id: str
    category: str
    file: str
    line: int
    column: int
    message: str
    note: str | None
    map_line: int  # line of the map file, for messages


def parse_map(text: str, name: str) -> tuple[list[MapEntry], list[str]]:
    """The entries of a position map and the problems with its format: one line
    per check, tab-separated `vpr line, viper error id, category, file, line,
    column, message [, note]`."""
    entries: list[MapEntry] = []
    problems: list[str] = []
    for lineno, raw in enumerate(text.split("\n"), start=1):
        if not raw.strip():
            continue

        def bad(why: str) -> None:
            problems.append(f"{name}:{lineno}: {why}")
        fields = raw.split("\t")
        if len(fields) not in (7, 8):
            bad(f"a position map line has 7 or 8 tab-separated fields, found {len(fields)}")
            continue
        try:
            vpr_line, line, column = int(fields[0]), int(fields[4]), int(fields[5])
        except ValueError:
            bad("the Viper line, line and column of a map entry must be integers")
            continue
        if vpr_line < 1 or line < 1 or column < 1:
            bad("the Viper line, line and column of a map entry start at 1")
            continue
        if fields[1] not in MAP_ERROR_IDS:
            bad(f"unknown Viper error id `{fields[1]}`")
            continue
        if fields[2] not in MAP_CATEGORIES:
            bad(f"unknown category `{fields[2]}` (known: "
                + ", ".join(sorted(MAP_CATEGORIES)) + ")")
            continue
        if not fields[3].strip() or not fields[6].strip():
            bad("a map entry needs a file and a message")
            continue
        if len(fields) == 8 and not fields[7].strip():
            bad("an empty note field")
            continue
        entries.append(MapEntry(vpr_line, fields[1], fields[2], fields[3], line, column,
                                fields[6], fields[7] if len(fields) == 8 else None, lineno))
    return entries, problems


# ---------------------------------------------------------------------------
# One .vpr file
# ---------------------------------------------------------------------------

class CallStatement:
    """`match(line)`: the line is exactly one call statement, `name(args)` or
    `targets := name(args)`, with the call's parentheses closing at the end of the
    line (so `p := h(); p := h()` is not one)."""

    PREFIX = re.compile(r"^\s*(?:[A-Za-z_$][\w$']*(?:\s*,\s*[A-Za-z_$][\w$']*)*\s*:=\s*)?"
                        r"(?P<name>[A-Za-z_$][\w$']*)\s*\(")

    def match(self, code: str):
        m = self.PREFIX.match(code)
        if m is None:
            return None
        depth = 1
        i = m.end()
        while i < len(code) and depth:
            depth += (code[i] == "(") - (code[i] == ")")
            i += 1
        return m if depth == 0 and code[i:].strip() in ("", ";") else None


def decreases_measures(src: Source) -> dict[int, str]:
    """The measure of every `decreases` clause, by the line of its keyword: the text after the keyword,
    and the lines that follow while they do not start a block, another clause, a statement or a
    declaration (`CLAUSE_END_RE`), so that a clause written over several lines (`decreases` on one line and
    `*` or `if c` on the next) is read whole.  Blank lines (and comments) inside a clause are skipped."""
    out: dict[int, str] = {}
    for lineno, code in enumerate(src.code, start=1):
        m = DECREASES_LINE_RE.match(code)
        if not m:
            continue
        parts = [m.group("measure")]
        for more in src.code[lineno:]:
            if more.strip() and CLAUSE_END_RE.match(more):
                break
            parts.append(more)
        out[lineno] = " ".join(" ".join(parts).split())
    return out


BIT_FUNCTION_RE = re.compile(r"^c_(?P<u>u?)(?P<op>and|or|xor)_(?P<width>32|64|128)$")


# The mask functions of `^` (decision 0027 item 3 as revised): `c_[u]xor_mask_N_k(a)` is a ^ (2^k - 1) and
# `c_[u]xor_cmask_N_k(a)` is a ^ ~(2^k - 1); their bodies are remainder forms, their one `ensures` is a range implication.
# Exactly what `parse_xor_mask_fn` (src/viper/prelude.cpp) reads, tightened: k is a decimal without leading zeros, with
# `0 <= k <= N` for the unsigned functions and `0 <= k <= N - 1` for the signed ones (a signed `2^N` is not a value of the type).
XOR_MASK_FUNCTION_RE = re.compile(r"^c_(?P<u>u?)xor_(?P<c>c?)mask_(?P<width>32|64|128)_(?P<k>0|[1-9][0-9]{0,2})$")


def xor_mask_function_ok(name: str) -> bool:
    m = XOR_MASK_FUNCTION_RE.match(name)
    return m is not None and int(m.group("k")) <= int(m.group("width")) - (0 if m.group("u") else 1)


def bit_range_ensures_lines(src: Source, decls: list[Decl]) -> set[int]:
    """The lines of the range postconditions that a bitwise prelude function may carry without a position map
    entry (decision 0012 item 11, amended for 0023): in `function c_[u]<op>_<N>(a: Int, b: Int): Int` the single
    header line `ensures <lo> <= result && result <= <hi>`, where `[lo, hi]` is the range of the computed type
    (`(-2^(N-1)) .. 2^(N-1)-1` for the signed functions, `0 .. 2^N-1` for the unsigned ones).  Any other
    `ensures` line, and a second one in the same function, is not exempt."""
    out: set[int] = set()
    for d in decls:
        if d.kind == "function" and d.has_body and xor_mask_function_ok(d.name):
            out |= xor_mask_range_ensures_line(src, d)
            continue
        m = BIT_FUNCTION_RE.match(d.name) if d.kind == "function" else None
        if m is None or not d.has_body:
            continue
        head = " ".join(src.code[d.line - 1].split())
        if not re.fullmatch(r"function " + re.escape(d.name) + r"\(a: Int, b: Int\): Int", head):
            continue
        width = int(m.group("width"))
        lo, hi = (0, 2 ** width - 1) if m.group("u") else (-2 ** (width - 1), 2 ** (width - 1) - 1)
        want = f"ensures {'(' + str(lo) + ')' if lo < 0 else lo} <= result && result <= {hi}"
        lines = [n for n in range(d.line + 1, d.body_line) if src.code[n - 1].strip().startswith("ensures")]
        if len(lines) == 1 and " ".join(src.code[lines[0] - 1].split()) == want:
            out.add(lines[0])
    return out


def xor_mask_range_ensures_line(src: Source, d: Decl) -> set[int]:
    """The single range postcondition of `function c_[u]xor_[c]mask_N_k(a: Int): Int`, exactly
    `ensures (LO <= a && a <= HI) ==> (LO <= result && result <= HI)` with `[LO, HI]` the range of the N-bit type
    (`LO` in parentheses when negative); empty when the header, the line or the count of `ensures` lines differs."""
    m = XOR_MASK_FUNCTION_RE.match(d.name)
    head = " ".join(src.code[d.line - 1].split())
    if m is None or not xor_mask_function_ok(d.name) or head != f"function {d.name}(a: Int): Int":
        return set()
    width = int(m.group("width"))
    lo, hi = (0, 2 ** width - 1) if m.group("u") else (-2 ** (width - 1), 2 ** (width - 1) - 1)
    lo_text = f"({lo})" if lo < 0 else str(lo)
    want = f"ensures ({lo_text} <= a && a <= {hi}) ==> ({lo_text} <= result && result <= {hi})"
    lines = [n for n in range(d.line + 1, d.body_line) if src.code[n - 1].strip().startswith("ensures")]
    if len(lines) == 1 and " ".join(src.code[lines[0] - 1].split()) == want:
        return {lines[0]}
    return set()


# The prelude functions whose bodies use Viper's `%` and `\` (src/viper/prelude.cpp), by operator.  `%`: `c_urem` guards
# `a >= 0 && b > 0` and `c_rem` calls it with the absolute values.  `\`: `c_div` and `c_udiv` divide by the variable `b`
# in the same guarded way, `c_shr` by `c_pow2(n)`.  The bitwise functions `c_[u]<and|or|xor>_<N>` also use `\`, but only
# with a power-of-two literal (`a \ 2`, `a \ 2^64`), so they need no exemption and are held to the rule.
DIVISION_PRELUDE = {"%": frozenset({"c_rem", "c_urem"}), "\\": frozenset({"c_div", "c_udiv", "c_shr"})}
DIVISOR_RE = re.compile(r"\s*(?P<lit>[0-9]+)(?![\w.$'])")
DIVISION_WHAT = {
    "%": "Viper's Euclidean remainder and is emitted only by a constant mask (`a % 2^k`)",
    "\\": "Viper's floor division and is emitted only by a bit function or a constant mask (`2^k * (a \\ 2^k)`)",
}


def lint_remainders(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0026 item 7: Viper's `%` and `\\` are the Euclidean remainder and the floor division (a remainder in
    `[0, divisor)` for a positive divisor), C's `%` and `/` truncate, and the two must never be confused: C's go
    through `c_rem`/`c_urem` and `c_div`/`c_udiv`.  The emitted ones outside those prelude bodies are the constant-mask
    rewrite (`a % 2^k`, `2^k * (a \\ 2^k)`) and the bit functions' `\\ 2^j`, whose divisor is a power of two.  So outside the
    bodies of the prelude functions that use the operator (`DIVISION_PRELUDE`, by name), every `%` and `\\` must be
    followed by a plain decimal literal 2^k (k >= 0, no sign, no parentheses, no leading zeros); anything the rule
    cannot read as that fails."""
    exempt: dict[str, set[int]] = {op: set() for op in DIVISION_PRELUDE}
    for d in decls:
        if d.kind == "function" and d.has_body:
            for op, names in DIVISION_PRELUDE.items():
                if d.name in names:
                    exempt[op].update(range(d.body_line, d.end_line + 1))
    for lineno, code in enumerate(src.code, start=1):
        for m in re.finditer(r"[%\\]", code):
            op = m.group()
            if lineno in exempt[op]:
                continue
            names = " and ".join(f"`{n}`" for n in sorted(DIVISION_PRELUDE[op]))
            d = DIVISOR_RE.match(code, m.end())
            if d is None:
                add(lineno, f"a `{op}` whose divisor is not a decimal literal: outside the prelude's {names}, "
                            f"`{op}` is {DIVISION_WHAT[op]}, with a power of two as divisor (decision 0026 item 7)")
                continue
            lit = d.group("lit")
            n = int(lit)
            if (len(lit) > 1 and lit[0] == "0") or n < 1 or n & (n - 1):
                add(lineno, f"a `{op}` with the divisor `{lit}`: outside the prelude's {names}, the divisor of "
                            f"`{op}` must be a power of two, 1 or more, written as a decimal literal "
                            "(decision 0026 item 7)")


def tag_above(src: Source, line: int, tag_re: re.Pattern):
    """The match of `tag_re` in the run of comment-only lines directly above `line` (1-based), or None.
    A blank line or a line with code ends the run."""
    i = line - 2
    while i >= 0 and not src.code[i].strip() and src.comments[i]:
        if m := tag_re.search(src.comments[i]):
            return m
        i -= 1
    return None


def record_exists(record: str | None) -> bool:
    """Whether `record` is a number NNNN with a decision record internal/decisions/NNNN-*.md."""
    if record is None or not re.fullmatch(r"[0-9]{4}", record):
        return False
    return any(DECISIONS_DIR.glob(f"{record}-*.md"))


def block_close_line(src: Source, d: Decl) -> int:
    """The line of the `}` that closes the body of `d` (the last line of the file if it never closes)."""
    depth = 0
    for lineno in range(d.body_line or d.line, len(src.code) + 1):
        for ch in src.code[lineno - 1]:
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return lineno
    return len(src.code)


# The memory model's domain and axioms, exactly (decision 0028 item 2, records 0033-0036): the domain is tagged
# `domain-ok 0028`, each of the four axioms has its own record and this text (whitespace-normalized).  Anything else
# in the domain's axioms is refused, a fifth axiom and an altered body included.
DOMAIN_RECORD = "0028"
BLOCK_AXIOMS: dict[str, tuple[str, str]] = {
    "loc_inv": ("0033", "axiom loc_inv { forall b: Block, i: Int :: { loc(b, i) } "
                        "block_of(loc(b, i)) == b && offset_of(loc(b, i)) == i }"),
    "loc_nonnull": ("0034", "axiom loc_nonnull { forall b: Block, i: Int :: { loc(b, i) } loc(b, i) != null }"),
    "member_inv": ("0035", "axiom member_inv { forall r: Ref, k: Int :: { member(r, k) } "
                           "parent_of(member(r, k)) == r && index_of(member(r, k)) == k }"),
    "member_lv": ("0036", "axiom member_lv { forall r: Ref, k: Int :: { lvloc(member(r, k)) } "
                          "lvloc(member(r, k)) == lvloc(block_of(r)) }"),
}


def lint_domain(src: Source, d: Decl, add: Callable[[int, str], None]) -> bool:
    """Whether the `domain` at `d` is the memory model's: named `Block`, with a body and with
    `// cerastes:domain-ok 0028` directly above (the record exists).  A tag that names another record
    is reported here (and the domain is then refused with it)."""
    if d.name != "Block" or not d.has_body or not BLOCK_DOMAIN_RE.match(src.code[d.line - 1]):
        return False
    tag = tag_above(src, d.line, DOMAIN_TAG_RE)
    if tag is None:
        return False
    if tag.group("record") != DOMAIN_RECORD or not record_exists(DOMAIN_RECORD):
        add(d.line, f"`// cerastes:domain-ok {tag.group('record') or ''}` must name decision {DOMAIN_RECORD} "
                    "(internal/decisions/0028-*.md must exist)")
        return False
    return True


def axiom_text(src: Source, lineno: int) -> str:
    """The text of the axiom that starts at `lineno`, up to its closing brace, whitespace-normalized."""
    depth = 0
    parts: list[str] = []
    for ln in range(lineno, len(src.code) + 1):
        parts.append(src.code[ln - 1])
        depth += src.code[ln - 1].count("{") - src.code[ln - 1].count("}")
        if depth <= 0:
            break
    return " ".join(" ".join(parts).split())


def tagged_axiom_lines(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> set[int]:
    """The lines of the axioms that are accepted: inside an accepted `domain Block`, exactly the four of
    `BLOCK_AXIOMS`, each once, each with its record in `// cerastes:axiom-ok <record>` directly above (the record
    exists) and its exact text.  A missing one is reported at the domain."""
    ok: set[int] = set()
    for d in decls:
        if d.kind != "domain" or not lint_domain(src, d, lambda _l, _w: None):
            continue
        seen: set[str] = set()
        for lineno in range(d.line + 1, block_close_line(src, d) + 1):
            m = AXIOM_DECL_RE.match(src.code[lineno - 1])
            if not m:
                continue
            name = m.group("name")
            if name not in BLOCK_AXIOMS:
                add(lineno, f"`axiom {name}` is not one of the memory model's four axioms "
                            f"({', '.join(BLOCK_AXIOMS)}; decision 0028 item 2)")
                continue
            record, text = BLOCK_AXIOMS[name]
            if name in seen:
                add(lineno, f"`axiom {name}` twice in the domain")
                continue
            seen.add(name)
            tag = tag_above(src, lineno, AXIOM_TAG_RE)
            if tag is None:
                continue  # the generic refusal of an untagged axiom reports it
            if tag.group("record") != record or not record_exists(record):
                add(lineno, f"`// cerastes:axiom-ok {tag.group('record') or ''}` must name decision {record} for "
                            f"`axiom {name}` (internal/decisions/{record}-*.md must exist)")
                continue
            if axiom_text(src, lineno) != text:
                add(lineno, f"`axiom {name}` is not the text of decision {record}: `{text}`")
                continue
            ok.add(lineno)
        for name in BLOCK_AXIOMS:
            if name not in seen:
                add(d.line, f"`domain Block` lacks `axiom {name}` (decision {BLOCK_AXIOMS[name][0]}): the emitter "
                            "writes all four")
    return ok


# The trusted allocation of an address-taken local of type `<t>` (decision 0032 item 4), exactly as the emitter prints it: any method named
# `alloc_<t>` has this header and these two postconditions (whitespace-normalized), `decreases _` and nothing else.
ALLOC_PREFIX = "alloc_"
ALLOC_SUFFIX_RE = re.compile(r"^[A-Za-z0-9_]+$")


def alloc_contract(suffix: str) -> list[str]:
    return [
        f"method alloc_{suffix}() returns (r: Ref)",
        "ensures ((r != null) && (r == loc(block_of(r), 0))) && (len(block_of(r)) == 1)",
        f"ensures ((((lvloc(block_of(r)) == loc(block_of(r), (-2))) && acc(r.v_{suffix})) && acc(r.d_{suffix})) && "
        f"(!r.d_{suffix})) && acc(lvloc(block_of(r)).lv)",
        "decreases _",
    ]


def lint_allocations(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0032 item 4: a bodyless method `alloc_<t>` is trusted (the tag rule), so its contract is pinned: a different
    postcondition (a dropped `!r.d_<t>`, an extra fact, `ensures false`, another suffix) is an unsound assumption."""
    for d in decls:
        if d.kind != "method" or not d.name.startswith(ALLOC_PREFIX):
            continue
        suffix = d.name[len(ALLOC_PREFIX):]
        lines = [" ".join(src.code[n - 1].split()) for n in range(d.line, d.end_line + 1) if src.code[n - 1].strip()]
        if d.has_body or not ALLOC_SUFFIX_RE.match(suffix) or lines != alloc_contract(suffix or "_"):
            add(d.line, f"`method {d.name}` is not the trusted allocation of decision 0032 item 4: it must be bodyless with "
                        "exactly the header, the two `ensures` lines and `decreases _` that the emitter prints for "
                        f"`alloc_{suffix or '<t>'}` (`ensures ((r != null) && ...` with the fields `v_{suffix or '<t>'}`/`d_{suffix or '<t>'}`)")


def lint_vpr(text: str, map_text: str | None, name: str = "<vpr>",
             map_name: str | None = None) -> list[str]:
    """Problems of one Viper file with its position map (None: the map is missing)."""
    src = split_source(text)
    problems: list[str] = []
    decls = declarations(src)

    def add(lineno: int, why: str) -> None:
        problems.append(f"{name}:{lineno}: {why}")

    for lineno, (code, comment) in enumerate(zip(src.code, src.comments), start=1):
        if m := INHALE_WORD_RE.search(code):
            tag = INHALE_TAG_RE.search(comment) if comment else None
            if tag is None:
                add(lineno, f"`{m.group(1)}` without `// cerastes:inhale-ok <reason>` "
                            "(invariant 4)")
            elif not tag.group("reason"):
                add(lineno, "`// cerastes:inhale-ok` needs a reason")
    for d in decls:
        if d.kind == "import":
            add(d.line, "`import`: an emitted file is self-contained (the lint reads only this "
                        "file, so an imported one would escape it)")
        elif d.kind == "domain":
            if not lint_domain(src, d, add):
                add(d.line, f"`domain {d.name}`: the prelude has no domains but the memory model's `domain Block` "
                            "with `// cerastes:domain-ok <record>` right above it (decision 0012 item 11, "
                            "decision 0028 item 2)")
        elif d.kind == "function" and not d.has_body:
            add(d.line, f"bodyless `function {d.name}`: functions need bodies "
                        "(decision 0012 item 11)")
        elif d.kind == "method" and not d.has_body:
            tag = trusted_tag(src, d.line)
            if tag is None:
                add(d.line, f"bodyless `method {d.name}` without `// cerastes:trusted-contract "
                            "<reason>` (decision 0012 item 1)")
            elif not tag.group("reason"):
                add(d.line, "`// cerastes:trusted-contract` needs a reason")
    ok_axioms = tagged_axiom_lines(src, decls, add)
    for lineno, code in enumerate(src.code, start=1):
        if re.search(r"\baxiom\b", code) and lineno not in ok_axioms:
            add(lineno, "`axiom`: the prelude has no axioms but the ones of the memory model's Block domain, "
                        "each with `// cerastes:axiom-ok <record>` right above it (decision 0012 item 11, "
                        "decision 0028 item 2)")
        if NEW_RE.search(code):
            add(lineno, "`new(...)`: its result is not `WF` (decision 0028 item 2); objects come from the "
                        "trusted `alloc_T` methods")

    methods = {d.name for d in decls if d.kind == "method"}
    call_re = CallStatement()
    check_lines: dict[int, str] = {}
    declaration_lines = {d.line for d in decls}
    measures = decreases_measures(src)
    exempt_ensures = bit_range_ensures_lines(src, decls)
    call_word_re = re.compile(r"(?<![\w$'])(" + "|".join(map(re.escape, sorted(methods))) + r")\s*\(") \
        if methods else None
    for lineno, code in enumerate(src.code, start=1):
        # every check is on its own line (decision 0012 item 13), so that its map entry
        # is the line: a keyword that does not start its line, or a second one, hides one
        words = list(CHECK_WORD_RE.finditer(code))
        for k, w in enumerate(words):
            if k > 0 or code[:w.start()].strip():
                add(lineno, f"`{w.group(1)}` does not start its line: every check is on its "
                            "own line (decision 0012 item 13)")
        if m := CHECK_LINE_RE.match(code):
            if lineno in exempt_ensures:
                pass  # the range postcondition of a bitwise prelude function: no map entry
            elif m.group(1) == "decreases" and measures[lineno] in ("", "_"):
                pass  # an empty measure (a method outside any recursion) or `_` (trusted) is no check
            else:
                check_lines[lineno] = m.group(1)
        elif lineno not in declaration_lines and call_word_re is not None \
                and (c := call_word_re.search(code)):
            if (m := call_re.match(code)) and m.group("name") in methods:
                check_lines[lineno] = f"call of {m.group('name')}"
            else:
                add(lineno, f"the call of method `{c.group(1)}` is not alone on its line: every "
                            "call is a statement of its own (decision 0012 item 13)")

    lint_termination_clauses(src, decls, add, measures)
    lint_jumps(src, decls, add)
    lint_flags(src, add)
    lint_remainders(src, decls, add)
    lint_allocations(src, decls, add)

    if map_text is None:
        problems.append(f"{name}: no position map ({map_name or name + MAP_SUFFIX})")
        return problems
    entries, map_problems = parse_map(map_text, map_name or name + MAP_SUFFIX)
    problems += map_problems
    by_line: dict[int, list[MapEntry]] = {}
    for e in entries:
        by_line.setdefault(e.vpr_line, []).append(e)
    for lineno, kind in sorted(check_lines.items()):
        if lineno not in by_line:
            add(lineno, f"`{kind}` line has no entry in the position map")
            continue
        allowed = KIND_IDS["call" if kind.startswith("call of ") else kind]
        for e in by_line[lineno]:
            if e.error_id not in allowed:
                problems.append(f"{map_name or name + MAP_SUFFIX}:{e.map_line}: the entry for "
                                f"{name} line {lineno} has the Viper error id `{e.error_id}`, "
                                f"which a `{kind}` line cannot produce (expected "
                                + " or ".join(f"`{i}`" for i in sorted(allowed)) + ")")
    nlines = len(src.code) - (1 if src.code and src.code[-1] == "" and text.endswith("\n") else 0)
    for lineno, group in sorted(by_line.items()):
        if lineno > nlines or not src.code[lineno - 1].strip():
            for e in group:
                problems.append(f"{map_name or name + MAP_SUFFIX}:{e.map_line}: entry for "
                                f"{name} line {lineno}, which is "
                                + ("past the end of the file" if lineno > nlines
                                   else "blank or a comment"))
        ids = [e.error_id for e in group]
        for error_id in sorted({i for i in ids if ids.count(i) > 1}):
            problems.append(f"{map_name or name + MAP_SUFFIX}: line {lineno} has two entries "
                            f"with the Viper error id `{error_id}`")
    return problems


def _top_level_items(measure: str) -> list[str]:
    """The comma-separated items of a `decreases` measure (commas inside brackets do not split)."""
    items, depth, cur = [], 0, ""
    for ch in measure:
        if ch in "([{":
            depth += 1
        elif ch in ")]}":
            depth -= 1
        if ch == "," and depth == 0:
            items.append(cur.strip())
            cur = ""
        else:
            cur += ch
    items.append(cur.strip())
    return items


def lint_termination_clauses(src: Source, decls: list[Decl], add: Callable[[int, str], None],
                             measures: dict[int, str] | None = None) -> None:
    """Decision 0013 item 5: Silicon checks termination only where a `decreases` clause is present, so every
    method (the `__wf` methods too, with an empty measure) and every `while` has one; and `decreases _`, which
    assumes termination (invariant 4), is allowed only on a bodyless method with the trusted-contract tag.
    A clause is read whole when it runs over several lines (`decreases_measures`)."""
    if measures is None:
        measures = decreases_measures(src)
    for d in decls:
        if d.kind != "method":
            continue
        header_end = d.body_line - 1 if d.has_body else d.end_line
        lines = range(d.line, max(d.line, header_end) + 1)
        clauses = [ln for ln in lines if DECREASES_LINE_RE.match(src.code[ln - 1])]
        if not clauses:
            add(d.line, f"`method {d.name}` has no `decreases` clause: Silicon checks termination only "
                        "where one is present (an empty `decreases` for a method outside any recursion, "
                        "decision 0013 item 5)")
    for lineno, measure in sorted(measures.items()):
        if measure == "_":
            owner = next((d for d in decls if d.line <= lineno <= d.end_line), None)
            ok = (owner is not None and owner.kind == "method" and not owner.has_body
                  and trusted_tag(src, owner.line) is not None)
            if not ok:
                add(lineno, "`decreases _` assumes termination (invariant 4): it is allowed only on a "
                            "bodyless method that carries `// cerastes:trusted-contract <reason>`")
        if "*" in _top_level_items(measure):
            add(lineno, "`decreases *` assumes termination (invariant 4, decision 0013): Silicon "
                        "does not check the termination of what carries it; the emitter produces "
                        "a measure, an empty `decreases`, or `decreases _` on a trusted bodyless method")
        if re.search(r"(?<![\w$'])if(?![\w$'])", measure):
            add(lineno, "conditional `decreases <measure> if <condition>` is not allowed (decision 0013): "
                        "termination is checked only where the condition holds, so the measure must be "
                        "unconditional")
    for lineno, code in enumerate(src.code, start=1):
        for _w in WHILE_WORD_RE.finditer(code):
            body = next((ln for ln in range(lineno, len(src.code) + 1) if "{" in src.code[ln - 1]), len(src.code))
            if not any(DECREASES_LINE_RE.match(src.code[ln - 1]) for ln in range(lineno, body)):
                add(lineno, "`while` loop without a `decreases` clause: Silicon checks termination only "
                            "where one is present (decision 0013 item 5)")


def lint_jumps(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0014 item 9: control flow is structured (a `while` for every cycle).  Silicon
    checks the termination of `while` loops only; a cycle made of a backward `goto`, or a loop
    that a `label ... invariant` introduces, is unchecked, so none is allowed.  Labels are
    per method: the same name in two methods is fine, in one method it is a problem."""
    tokens: list[tuple[str, int]] = []
    for lineno, line in enumerate(src.code, start=1):
        tokens += [(m.group(), lineno) for m in TOKEN_RE.finditer(line)]
    for d in decls:
        if d.kind != "method":
            continue
        labels: dict[str, int] = {}  # name -> line of its first definition
        body = [(t, ln) for t, ln in tokens if d.line <= ln <= d.end_line]
        label_blocks, goto_blocks = block_nesting(body)
        for target, lineno, k_goto, blocks in goto_blocks:
            if target not in label_blocks:
                continue  # an undefined label is Silicon's business
            k_label, inside = label_blocks[target]
            if k_label > k_goto and inside != blocks[:len(inside)]:
                add(lineno, f"`goto {target}` into a block: the label is inside a `{{ ... }}` (a `while` "
                            "body or an `if` or `else` branch) that does not enclose the `goto`; a jump "
                            "may only go outward or stay in its block (it would skip the loop's "
                            "invariant check or the branch's condition; decision 0015)")
        for k, (text, lineno) in enumerate(body):
            nxt = body[k + 1] if k + 1 < len(body) else ("", 0)
            if text == "label" and nxt[0]:
                name = nxt[0]
                if name in labels:
                    add(lineno, f"duplicate `label {name}` in `method {d.name}` (first on line "
                                f"{labels[name]}): a label is defined once per method")
                else:
                    labels[name] = lineno
                after = body[k + 2][0] if k + 2 < len(body) else ""
                if after == "invariant":
                    add(lineno, f"`label {name}` carries an `invariant`: a label with invariants "
                                "is a loop that Silicon does not check for termination "
                                "(decision 0014); loops are `while`")
            elif text == "goto" and nxt[0] in labels:
                add(lineno, f"backward `goto {nxt[0]}` (the label is on line {labels[nxt[0]]}): a "
                            "cycle that Silicon does not check for termination (decision 0014); "
                            "loops are `while`")


FLAG_ASSIGN_RE = re.compile(r"^\s*d_(?P<name>[A-Za-z_$][\w$']*)\s*:=(?P<value>.*)$")
FLAG_ANYWHERE_RE = re.compile(r"(?<![\w$'])d_[\w$']*\s*:=")
FLAG_DECL_INIT_RE = re.compile(r"(?<![\w$'])var\s+d_[\w$']*\s*:\s*Bool\s*:=")
DUMMY_VALUE_RE = re.compile(r"^\s*(?:-?\s*\d+|\(\s*-\s*\d+\s*\)|true|false|null)\s*;?\s*$")
# The flag of an address-taken cell is a field of the cell (decision 0032 item 11): `<recv>.d_<t> := <value>`.
FIELD_FLAG_RE = re.compile(r"^\s*(?P<recv>[A-Za-z_$][\w$']*)\.d_(?P<name>[A-Za-z_$][\w$']*)\s*:=(?P<value>.*)$")


def lint_flags(src: Source, add: Callable[[int, str], None]) -> None:
    """Decision 0019 item 5: the ghost flag `d_<name>` of a local without an initializer is set only by a
    write of that local, and a reset gives the local its dummy value.  A flag that could be set without
    the write would make an `uninitialized` obligation hold for a variable that has no value: the one
    place where a lowering bug turns into a wrong `verified`.  So `d_<name> := true` must follow
    `<name> := ...` directly, `d_<name> := false` must precede `<name> := <literal>` directly, and a flag
    is assigned nothing else.  Neighbours are the previous and next lines that are neither blank nor a
    comment (the emitter writes one statement per line)."""
    lines = [(ln, code) for ln, code in enumerate(src.code, start=1) if code.strip()]
    for k, (lineno, code) in enumerate(lines):
        if FLAG_DECL_INIT_RE.search(code):
            add(lineno, "a flag is declared without an initializer (`var d_<name>: Bool`): it holds a value only "
                        "after a reset (decision 0019 item 5)")
        fm = FIELD_FLAG_RE.match(code)
        if fm is not None and len(FLAG_ANYWHERE_RE.findall(code)) == 1:
            recv, name = fm.group("recv"), fm.group("name")
            value = fm.group("value").strip().rstrip(";").strip()
            if ";" in value:
                add(lineno, "an assignment to a flag does not stand alone on its line: the flag rule reads "
                            "one statement per line (decision 0019 item 5)")
            elif value != "true":
                add(lineno, f"`{recv}.d_{name}` is assigned `{value}`: the flag of a cell is assigned only `true`, "
                            "by the write of its value (the allocation gives `false`; decision 0032 item 11, "
                            "decision 0019 item 5)")
            else:
                prev = lines[k - 1][1] if k > 0 else ""
                if not re.match(r"^\s*" + re.escape(recv) + r"\.v_" + re.escape(name) + r"\s*:=", prev):
                    add(lineno, f"`{recv}.d_{name} := true` does not directly follow a write `{recv}.v_{name} := ...`: "
                                "a flag is set only by the write of its cell (decision 0032 item 11, "
                                "decision 0019 item 5)")
            continue
        m = FLAG_ASSIGN_RE.match(code)
        if len(FLAG_ANYWHERE_RE.findall(code)) > (1 if m else 0) or (m is None and FLAG_ANYWHERE_RE.search(code)):
            add(lineno, "an assignment to a flag does not stand alone on its line: the flag rule reads "
                        "one statement per line (decision 0019 item 5)")
            continue
        if m is None:
            continue
        name, value = m.group("name"), m.group("value").strip().rstrip(";").strip()
        if value == "true":
            prev = lines[k - 1][1] if k > 0 else ""
            if not re.match(r"^\s*" + re.escape(name) + r"\s*:=", prev):
                add(lineno, f"`d_{name} := true` does not directly follow a write `{name} := ...`: a flag is "
                            "set only by the write of its variable (decision 0019 item 5)")
        elif value == "false":
            nxt = lines[k + 1][1] if k + 1 < len(lines) else ""
            r = re.match(r"^\s*" + re.escape(name) + r"\s*:=(?P<rest>.*)$", nxt)
            if r is None or DUMMY_VALUE_RE.match(r.group("rest")) is None:
                add(lineno, f"`d_{name} := false` is not directly followed by `{name} := <literal>`: a reset "
                            "gives the variable its dummy value (decision 0019 item 5)")
        else:
            add(lineno, f"`d_{name}` is assigned `{value}`: a flag is assigned only `true`, by the write of "
                        "its variable, or `false`, by a reset (decision 0019 item 5)")


def block_nesting(body: list[tuple[str, int]]):
    """({label: (token index, the braces that enclose its first definition)}, [(label, line, token
    index, the braces that enclose the `goto`)]) for the tokens of one method.  A brace is the
    number of its `{` (outermost first); every `{ ... }` counts, the method body, a `while` body,
    an `if` or `else` branch, and also the balanced trigger sets of quantifiers, which are closed
    before any `label` or `goto` that follows them."""
    labels: dict[str, tuple[int, tuple[int, ...]]] = {}
    gotos: list[tuple[str, int, int, tuple[int, ...]]] = []
    stack: list[int] = []
    count = 0
    for k, (text, lineno) in enumerate(body):
        nxt = body[k + 1][0] if k + 1 < len(body) else ""
        if text == "{":
            count += 1
            stack.append(count)
        elif text == "}":
            if stack:
                stack.pop()
        elif text == "label" and nxt:
            labels.setdefault(nxt, (k, tuple(stack)))
        elif text == "goto" and nxt:
            gotos.append((nxt, lineno, k, tuple(stack)))
    return labels, gotos


def trusted_tag(src: Source, line: int):
    """The `cerastes:trusted-contract` tag for the method at `line` (1-based): on
    that line, or in the run of comment-only lines right above it."""
    m = TRUSTED_TAG_RE.search(src.comments[line - 1]) if src.comments[line - 1] else None
    i = line - 2
    while m is None and i >= 0 and not src.code[i].strip() and src.comments[i]:
        m = TRUSTED_TAG_RE.search(src.comments[i])
        i -= 1
    return m


# ---------------------------------------------------------------------------
# Directories, the manifest, the command line
# ---------------------------------------------------------------------------

class LintUsageError(Exception):
    pass


def verify_tests_expect_viper(root: Path | None = None) -> bool:
    """Whether a test of the tree (tests/ and the documentation examples) runs
    `cerastes verify` and must leave Viper behind (`verified` or `error`, in C
    or C++): then an absent or empty build/vpr is a failure."""
    base = ROOT if root is None else root
    paths = [str(base / p) for p in rt.DEFAULT_PATHS if (base / p).is_dir()]
    if not paths:
        return False
    for f in rt.discover(paths).tests:
        exp = rt.read_header(f)
        if isinstance(exp, rt.Expectation) and rt.is_verify_test(f, exp) and rt.vpr_expected(exp):
            return True
    return False


RUN_ID_RE = re.compile(r"^# run-id: (?P<id>\S+)$")


def read_manifest(directory: Path) -> tuple[list[str] | None, str | None, list[str]]:
    """(the manifest's entries or None if there is no MANIFEST, its run id,
    problems).  The first line is `# run-id: <id>`; other `#` lines are comments."""
    path = directory / MANIFEST
    if not path.is_file():
        return None, None, []
    lines = [l for l in path.read_text().split("\n") if l.strip()]
    problems = []
    run_id = None
    if lines and (m := RUN_ID_RE.match(lines[0])):
        run_id = m.group("id")
        lines = lines[1:]
    else:
        problems.append(f"{path}: the first line is not `# run-id: <id>`: the files cannot be "
                        "tied to a run")
    entries = [l for l in lines if not l.startswith("#")]
    seen: set[str] = set()
    for e in entries:
        if e.startswith("/") or ".." in Path(e).parts or not e.endswith(".vpr"):
            problems.append(f"{path}: `{e}` is not a path of a .vpr file below the directory")
        if e in seen:
            problems.append(f"{path}: `{e}` is listed twice")
        seen.add(e)
    return entries, run_id, problems


def lint_directory(directory: Path, expect_viper: Callable[[], bool] | None = None
                   ) -> tuple[list[str], int, str | None, str | None]:
    """(problems, number of .vpr files linted, a note, the run id) for a directory;
    `expect_viper` (default: verify_tests_expect_viper) says whether the tree has
    a verify test that must leave Viper behind."""
    if expect_viper is None:
        expect_viper = verify_tests_expect_viper
    problems: list[str] = []
    files = sorted(p for p in directory.rglob("*.vpr") if p.is_file()) if directory.is_dir() else []
    manifest, run_id, manifest_problems = read_manifest(directory)
    problems += manifest_problems
    if manifest is None and not files:
        if expect_viper():
            return [f"{directory}: no {MANIFEST} and no .vpr file, but a verify test of the "
                    "tree expects Viper (run scripts/run_tests.py first)"], 0, None, None
        return [], 0, "no emitted Viper to lint", None
    if manifest is not None:
        if not manifest:
            problems.append(f"{directory / MANIFEST}: the manifest is empty: an empty set "
                            "must not pass")
        for e in manifest:
            if not (directory / e).is_file():
                problems.append(f"{directory / MANIFEST}: `{e}` is listed but not there "
                                f"(fewer files than the {len(manifest)} the run expects)")
    elif files:
        problems.append(f"{directory}: .vpr files but no {MANIFEST}: they cannot be tied to the "
                        "current run (a stale or partial directory)")
    if manifest is not None:
        listed = set(manifest)
        for f in files:
            if f.relative_to(directory).as_posix() not in listed:
                problems.append(f"{f}: not listed in the current {MANIFEST} (run "
                                f"{run_id or '?'}): a stale file, or one from a partial run")
    if not files:
        problems.append(f"{directory}: no .vpr file to lint")
    for f in files:
        problems += lint_file(f)
    return problems, len(files), None, run_id


def lint_file(path: Path) -> list[str]:
    mapf = path.with_name(path.name + MAP_SUFFIX)
    try:
        text = path.read_text()
    except OSError as err:
        return [f"{path}: cannot read ({err})"]
    map_text = mapf.read_text() if mapf.is_file() else None
    return lint_vpr(text, map_text, str(path), str(mapf))


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("paths", nargs="*", help="directories or .vpr files (default: build/vpr)")
    args = parser.parse_args(argv)
    paths = [Path(p) for p in args.paths] or [ROOT / DEFAULT_DIR]
    problems: list[str] = []
    linted = 0
    notes: list[str] = []
    runs: list[str] = []
    for p in paths:
        if p.is_dir() or (not p.exists() and not args.paths):
            try:
                probs, n, note, run_id = lint_directory(p)
            except rt.UsageError as err:
                print(f"lint_viper: {err}", file=sys.stderr)
                return 2
            problems += probs
            linted += n
            if note:
                notes.append(note)
            if run_id:
                runs.append(run_id)
        elif p.is_file() and p.suffix == ".vpr":
            problems += lint_file(p)
            linted += 1
        else:
            print(f"lint_viper: {p}: not a directory or a .vpr file", file=sys.stderr)
            return 2
    for line in problems:
        print(line)
    if problems:
        print(f"lint_viper: {len(problems)} problems in {linted} files")
        return 1
    if notes and linted == 0:
        print(f"lint_viper: {notes[0]}")
    else:
        print(f"lint_viper: {linted} files clean" + (f" (run {', '.join(runs)})" if runs else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())
