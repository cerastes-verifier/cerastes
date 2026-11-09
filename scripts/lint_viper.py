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
* the flags of locals without an initializer (decision 0019 item 5; `held_<v> := ...` and `tv_<v> := ...` lines of an owning local, decision 0052 item 3, may stand between a write and its flag: only a write sets a flag, and a
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
* the ghost amounts of the memory model (decision 0037 items 6 and 14): every `Perm` formal of a method has exactly one
  `requires none < a && a < write` (parentheses and spaces ignored) and no other clause of the method mentions `a`
  before it (a clause that reads the amount before its bounds is not well-formed for Silicon, and an amount without
  the bounds could be `write` or `none`); `wildcard` (an amount Silicon chooses, which is an unchecked assumption) is
  refused anywhere.  The case split `if (<condition>) { }` is plain Viper and needs no rule;
* the ghost amount results of a method (decision 0052 item 5: the output amount of a partnerless `ensures` atom): a result of type
  `Perm` has exactly one `ensures none < o && o <= write` (parentheses and spaces ignored) and no clause of the method mentions
  it before that line (the postcondition that keeps the amount positive and at most `write` whatever the body, or a trusted
  contract, says); a call's `Perm` target (`t, o := f(...)`) is allowed only where `f` is a method of the file whose result at
  that position is such a `Perm` (otherwise the rule of the next paragraph flags it);
* the ghost amounts of loops (decision 0038 item 9, amending 0037 item 14): a `Perm` **local** (`var k: Perm`; the
  snapshots `kv := perm(x.v_int)` that a loop's invariant uses as the amount of an `acc`) is assigned only `perm(...)`,
  `none`, `write` or a conditional `c ? a : b` of those (never `wildcard`, never arithmetic, never another variable:
  the snapshot is what the method holds), and is never assigned inside a loop whose invariant mentions it (a
  havocked amount could exceed `write` and make the body vacuous); `acc(x.f, k)` with such an amount is allowed in
  an invariant, `perm(...)` in an invariant is refused (Viper refuses it; the snapshot is the way);
* the heap's free token (decision 0047 items 6, 7 and 10): `field tok: Bool` is declared exactly in a program that uses it (a heap
  program, which has a view call or a block site: a file that declares it and never uses it, uses it undeclared or declares it
  with another type is a problem); `tok` appears only as `<slot>.tok` where the slot is `loc(block_of(...), -1)` (the token's slot
  of 0028 item 6), and only in a `requires`, `ensures`, `invariant` or `assert` line (the conjunction of a view or a block site, an
  explicit row, a loop place's invariant or its back-edge row) or inside the `perm(...)` of a snapshot `kt := perm(...)` (a loop
  place's amount); never in an `exhale`, an assignment to `<slot>.tok`, a condition, a `var`, a `fold` or any other statement
  (a token is created only by a view's or a function's postcondition and consumed only by a call whose `requires` has the block
  site: nothing in the body may make or take one); `forperm` appears only as the statement `assert forperm t: Ref [t.tok] :: <E>`
  (one binder, the field `tok`, no `tok` in `<E>`) whose position-map entries are all category `leak`: the one assertion that
  reads every token the method holds (the rest of the heap rule is not checked here: the exemption `<E>` is the encoder's);
* the owner ghosts of an owning local (decision 0052 item 3): `tv_<v> := (<x> != null) ? loc(block_of(<x>), -1).tok : tv_<v>` is the
  one assignment that reads a token outside a `perm(...)` snapshot (the same ghost on both sides, the same pointer in the guard and
  the slot); `held_<v> := ... perm(loc(block_of(<o>), -1).tok) > none` reads it inside `perm(...)`, and the ending rows
  `if (!(<row>)) { assert false }` read no token;
* the allocator ghost of an owning loop variable in a vlibc heap program (decision 0052 item 3, loops): a `tve_<...>` is assigned only
  `(<x> != null) ? tv_<y> : true` and never inside a loop whose invariant mentions it; the entry and back-edge rows of an owner
  (`if (!(<row>)) { assert false }`, which read the token under `perm(...)` and compare it with `tve_...`) and its invariants (`held_v
  == (v != null)`, `(v != null) ==> (acc(<slot>.tok) && <slot>.tok == tve_...)`) need no rule of their own: they are the ending rows
  and the conjuncts the token rules above already read;
* the typed views of the allocation functions (decision 0047 items 2, 4 and 7): a bodyless method whose trusted-contract tag is
  vlibc's `malloc`, `calloc` or `free` with `typed view:` in its reason is the trusted view of one allocation type, and its text is
  pinned as `alloc_<t>`'s is, built from the type's leaves: `method f_<routine>__<n>() returns (res: Ref)` (`(p_ptr__0: Ref)` for
  `free`), exactly two clauses and `decreases _`; the first clause the well-formedness of the pointer (and, for a structure, its member
  lengths under `offset_of < len`), the second `!(res == null) ==>` the block part (the pointer, its well-formedness, offset 0, the
  token `acc(loc(block_of(res), -1).tok) && ...tok`, the liveness equation and cell, `len == 1` with the structure's member blocks and
  slot facts in the grammar of decision 0043 item 8, then for each leaf, in member order, its value and flag permissions and the facts
  `owns` states under the flag: an integer's range of its field's type, a pointer's well-formedness (with the pointee's member blocks),
  nothing for a `bool`) and, for `calloc` only, the flags and then the values 0, `false` or `null` of every leaf, and for no view a
  clause more or fewer: a view with an extra clause would be an unchecked assumption;
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


STRUCT_ALLOC_RE = re.compile(r"^struct_[0-9]+_[A-Za-z0-9_]+$")
FIELD_ANY_RE = re.compile(r"^\s*field\s+(?P<kind>[vd])_(?P<suffix>[A-Za-z0-9_$']+)\s*:")
_STRUCT_INDEX = r"(?:0|[1-9][0-9]*)"
_CHAIN_LOC_RE = re.compile(rf"^loc\(member\((?P<inner>.*),(?P<k>{_STRUCT_INDEX})\),0\)$")
_MEMBER_RE = re.compile(rf"^member\((?P<inner>.*),(?P<k>{_STRUCT_INDEX})\)$")


def parse_struct_chain(text: str, root: str = "r") -> tuple[int, ...] | None:
    """`root` (`r` for an allocation, the pointer's value for a `WF_S`) is (); `loc(member(<chain>, k), 0)` is the chain's tuple plus k,
    with a non-negative literal k and nothing else inside (no `block_of`, `parent_of`, other offset or arithmetic): the grammar of
    decision 0043 item 13."""
    if text == root:
        return ()
    m = _CHAIN_LOC_RE.match(text)
    if not m:
        return None
    inner = parse_struct_chain(m.group("inner"), root)
    return None if inner is None else inner + (int(m.group("k")),)


def parse_struct_member(text: str, root: str = "r") -> tuple[int, ...] | None:
    """`member(<chain>, k)` as the tuple of the chain plus k."""
    m = _MEMBER_RE.match(text)
    if not m:
        return None
    inner = parse_struct_chain(m.group("inner"), root)
    return None if inner is None else inner + (int(m.group("k")),)


def struct_wf_lens_problem(parts: list[str], root: str) -> str | None:
    """The guarded conjunct of `WF_S(v)` in a `program_start` row (decision 0043 item 8): under `v != null && offset_of(v) < len(block_of(v))`
    only `len(member(<chain>, k)) == 1` with a literal k, each member block once, a member of a member only inside a nested struct member,
    and the slot facts `A != B` exactly for the pairs of the root or a nested slot A and a nested slot B below it, chains rooted at the value.
    Returns the reason, or None."""
    members: list[tuple[int, ...]] = []
    facts: list[tuple[tuple[int, ...], tuple[int, ...]]] = []
    for part in parts:
        if "!=" in part:
            a, _, b = part.partition("!=")
            ca, cb = parse_struct_chain(a, root), parse_struct_chain(b, root)
            if ca is None or cb is None:
                return f"the slot fact `{part}` is not between canonical chains rooted at the value"
            facts.append((ca, cb))
            continue
        m = re.fullmatch(r"len\((?P<m>.*)\)==1", part)
        key = parse_struct_member(m.group("m"), root) if m else None
        if key is None:
            return f"`{part}` is not `len(member(<chain>, <literal>)) == 1` or a slot fact"
        if key in members:
            return f"the member block {key} twice"
        members.append(key)
    mset = set(members)
    if not mset:
        return "no member block"
    for key in mset:
        if key[:-1] and key[:-1] not in mset:
            return f"the member block {key} is not inside a nested struct member"
    structs = {k for k in mset if any(o[:-1] == k for o in mset)}
    expected = {(a, b) for b in structs for a in {()} | structs if len(b) > len(a) and b[:len(a)] == a}
    if len(facts) != len(set(facts)):
        return "a slot fact twice"
    if set(facts) != expected:
        return "the slot facts are not exactly the pairs of the root or a nested slot and a nested slot below it"
    return None


def struct_allocation_problem(src: Source, d: Decl, suffix: str) -> str | None:
    """Decision 0043 items 4 and 13: the trusted allocation `alloc_struct_<n>_<tag>` is pinned by its shape: bodyless, the header
    and `decreases _`; first the root facts of `alloc_<t>`; then clauses in which each member block has `len(member(X, k)) == 1`,
    each leaf exactly `acc(L.v_t) && acc(L.d_t) && !L.d_t` with `L = loc(member(X, k), 0)` and one declared suffix `t`, a nested struct
    member nothing else (its slot is the X of the clauses that follow), and the slot facts `A != B` of item 4, exactly the pairs
    (A, B) with B a nested slot and A the root or a nested slot whose chain B's chain strictly extends; last the liveness clause
    with exactly one `lv` permission on the literal `lvloc(block_of(r))`; every chain in the canonical grammar rooted at `r`, every
    member block and every (receiver, field) once; nothing else.  Returns the reason, or None."""
    if d.has_body:
        return "it has a body"
    lines = [" ".join(src.code[n - 1].split()) for n in range(d.line, d.end_line + 1) if src.code[n - 1].strip()]
    if len(lines) < 5 or lines[0] != f"method alloc_{suffix}() returns (r: Ref)" or lines[-1] != "decreases _":
        return "its header, `decreases _` or the number of clauses is not the emitter's"
    body = lines[1:-1]
    if not all(l.startswith("ensures ") for l in body):
        return "it has a clause that is not an `ensures`"
    root_text = alloc_contract("x")[1]
    if norm_viper(body[0][len("ensures "):]) != norm_viper(root_text[len("ensures "):]):
        return "its first `ensures` is not the root facts of `alloc_<t>`"
    live = norm_viper("lvloc(block_of(r)) == loc(block_of(r), -2) && acc(lvloc(block_of(r)).lv)")
    if norm_viper(body[-1][len("ensures "):]) != live:
        return "its last `ensures` is not exactly the liveness token (one `lv` permission on `lvloc(block_of(r))`)"
    declared = {"v": set(), "d": set()}
    for line in src.code:
        m = FIELD_ANY_RE.match(line)
        if m:
            declared[m.group("kind")].add(m.group("suffix"))
    members: set[tuple[int, ...]] = set()
    structs: set[tuple[int, ...]] = set()  # member blocks with no leaf: their slot is the next chain
    leaves: set[tuple[tuple[int, ...], str]] = set()
    facts: list[tuple[tuple[int, ...], tuple[int, ...]]] = []
    middle = body[1:-1]
    if not any("len(" in c for c in middle):
        return "it states no member block"
    for clause in middle:
        parts = norm_viper(clause[len("ensures "):]).split("&&")
        lens: list[tuple[int, ...]] = []
        accs: dict[str, tuple[tuple[int, ...], str]] = {}
        nots: list[tuple[tuple[int, ...], str]] = []
        for part in parts:
            if "!=" in part:
                a, _, b = part.partition("!=")
                ca, cb = parse_struct_chain(a), parse_struct_chain(b)
                if ca is None or cb is None:
                    return f"the slot fact `{part}` is not between canonical chains"
                facts.append((ca, cb))
                continue
            m = re.fullmatch(r"len\((?P<m>.*)\)==1", part)
            if m:
                key = parse_struct_member(m.group("m"))
                if key is None:
                    return f"`{part}` is not `len(member(<chain>, <literal>)) == 1`"
                lens.append(key)
                continue
            m = re.fullmatch(r"acc\((?P<l>.*)\.(?P<f>[vd])_(?P<t>[A-Za-z0-9_$']+)\)", part)
            if m:
                lc = parse_struct_chain(m.group("l"))
                if lc is None or not lc:
                    return f"`{part}` is not a permission to a canonical leaf"
                if m.group("f") in accs:
                    return f"`{part}`: a second {m.group('f')} permission in one clause"
                accs[m.group("f")] = (lc, m.group("t"))
                continue
            m = re.fullmatch(r"!(?P<l>.*)\.d_(?P<t>[A-Za-z0-9_$']+)", part)
            if m:
                lc = parse_struct_chain(m.group("l"))
                if lc is None or not lc:
                    return f"`{part}` is not `!L.d_<t>` of a canonical leaf"
                nots.append((lc, m.group("t")))
                continue
            return f"`{part}` is not one of the clauses of decision 0043 item 13"
        if not lens:
            if accs or nots:
                return "a leaf's permissions without its member block's `len(...) == 1`"
            continue  # the slot facts alone
        if len(lens) != 1:
            return "two member blocks in one clause"
        key = lens[0]
        if key in members:
            return f"the member block {key} twice"
        if key[:-1] and key[:-1] not in structs:
            return f"the member block {key} is not inside a nested struct member"
        members.add(key)
        if not accs and not nots:
            structs.add(key)
            continue
        if set(accs) != {"v", "d"} or len(nots) != 1:
            return "a leaf is not exactly `acc(L.v_t) && acc(L.d_t) && !L.d_t`"
        (lv_, tv), (ld_, td), (ln_, tn) = accs["v"], accs["d"], nots[0]
        if not (lv_ == ld_ == ln_ == key and tv == td == tn):
            return "a leaf's value, flag and `!d` are not one cell `loc(member(X, k), 0)` of one field suffix"
        if tv not in declared["v"] or tv not in declared["d"]:
            return f"the field suffix `{tv}` is not declared (`field v_{tv}` and `field d_{tv}`)"
        if (key, tv) in leaves:
            return f"the leaf {key} twice"
        leaves.add((key, tv))
    expected = {(a, b) for b in structs for a in {()} | structs if len(b) > len(a) and b[:len(a)] == a}
    if len(facts) != len(set(facts)):
        return "a slot fact twice"
    if set(facts) != expected:
        return ("the slot facts `A != B` are not exactly the pairs of the root or a nested slot A and a nested slot B below it "
                "(decision 0043 item 4)")
    return None


def lint_allocations(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0032 item 4: a bodyless method `alloc_<t>` is trusted (the tag rule), so its contract is pinned: a different
    postcondition (a dropped `!r.d_<t>`, an extra fact, `ensures false`, another suffix) is an unsound assumption.  The struct
    allocations `alloc_struct_<n>_<tag>` have their own shape (decision 0043 item 13)."""
    for d in decls:
        if d.kind != "method" or not d.name.startswith(ALLOC_PREFIX):
            continue
        suffix = d.name[len(ALLOC_PREFIX):]
        if STRUCT_ALLOC_RE.match(suffix):
            why = struct_allocation_problem(src, d, suffix)
            if why is not None:
                add(d.line, f"`method {d.name}` is not the trusted struct allocation of decision 0043 items 4 and 13: {why}")
            continue
        lines = [" ".join(src.code[n - 1].split()) for n in range(d.line, d.end_line + 1) if src.code[n - 1].strip()]
        if d.has_body or not ALLOC_SUFFIX_RE.match(suffix) or lines != alloc_contract(suffix or "_"):
            add(d.line, f"`method {d.name}` is not the trusted allocation of decision 0032 item 4: it must be bodyless with "
                        "exactly the header, the two `ensures` lines and `decreases _` that the emitter prints for "
                        f"`alloc_{suffix or '<t>'}` (`ensures ((r != null) && ...` with the fields `v_{suffix or '<t>'}`/`d_{suffix or '<t>'}`)")


# Globals and `static` locals (decision 0041 item 4): each is a `unique function GB_<ident>(): Block` of `domain Block`, and the trusted
# `program_start()` gives `main` their initial state.  That method is an assumption (invariant 4), so its contract is pinned row by row, as
# `alloc_<t>` is; the pure facts about the blocks (`len(GB_x()) == 1 && lvloc(GB_x()) == loc(GB_x(), -2)`, true in 0028 item 2's model) are
# a hypothesis that every other method takes as its first `requires` and its callers discharge, never an axiom.
UNIQUE_RE = re.compile(r"(?<![\w$'])unique(?![\w$'])")
GB_DECL_RE = re.compile(r"^\s*unique\s+function\s+(?P<name>GB_[A-Za-z0-9_$']+)\(\)\s*:\s*Block\s*$")
GB_FUNCTION_RE = re.compile(r"^\s*function\s+GB_")
FIELD_DECL_RE = re.compile(r"^\s*field\s+v_(?P<suffix>[A-Za-z0-9_$']+)\s*:\s*(?P<type>[A-Za-z]+)\s*$")
PROGRAM_START = "program_start"
PROGRAM_START_CALL_RE = re.compile(r"(?<![\w$'])program_start\s*\(")
INT_LITERAL_RE = r"-?[0-9]+"


def norm_viper(text: str) -> str:
    """`text` without whitespace and without grouping parentheses (a `(` that does not follow a name is a group; the
    parentheses of a call or of `GB_x()` stay), so that the emitter's full parenthesization and a plain spelling
    of the same `&&` chain compare equal."""
    out: list[str] = []
    stack: list[bool] = []  # True: a call's parenthesis, kept
    prev = ""
    for ch in text:
        if ch.isspace():
            continue
        if ch == "(":
            keep = bool(prev) and (prev.isalnum() or prev in "_$'")
            stack.append(keep)
            if keep:
                out.append(ch)
        elif ch == ")":
            if (stack.pop() if stack else True):
                out.append(ch)
        else:
            out.append(ch)
        prev = ch
    return "".join(out)


def global_facts_atoms(names: list[str]) -> list[str]:
    atoms: list[str] = []
    for n in names:
        atoms += [norm_viper(f"len({n}()) == 1"), norm_viper(f"lvloc({n}()) == loc({n}(), -2)")]
    return sorted(atoms)


def clause_texts(src: Source, d: Decl) -> list[tuple[str, int, str]]:
    """(keyword, first line, text after the keyword) of each clause of the header of the method `d`; a clause that runs over
    several lines is joined."""
    out: list[tuple[str, int, str]] = []
    end = d.body_line - 1 if d.has_body else d.end_line
    for n in range(d.line, max(d.line, end) + 1):
        code = src.code[n - 1]
        m = CLAUSE_START_RE.match(code)
        if m:
            out.append((m.group(1), n, code[m.end():]))
        elif out and code.strip() and not code.strip().startswith("{"):
            k, ln, text = out[-1]
            out[-1] = (k, ln, text + " " + code)
    return out


def first_statement(src: Source, d: Decl) -> tuple[str, int]:
    """The text and line of the first statement of the body of `d`: after the `var` declarations."""
    for n in range(d.body_line + 1, d.end_line + 1):
        code = src.code[n - 1].strip()
        if code and not code.startswith("var ") and not code.startswith("var\t"):
            return code, n
    return "", 0


def leaf_value(fields: dict[str, str], suffix: str, c: str, mid: str) -> tuple[bool, str | None]:
    """The value part of a `program_start` row for the cell `c` (normalized text) of field suffix `suffix` (decision 0041 item 4):
    a literal of the type, `null`, the range of an unknown value, `WF` / `WF_S` of an unknown pointer, or `loc(GB_h(), 0)` (returned
    as the target).  Returns (good, target)."""
    kind, target, good = fields.get(suffix), None, False
    if kind == "Int":
        good = re.fullmatch(re.escape(c) + rf"\.v_{suffix}==" + INT_LITERAL_RE, mid) is not None
        if not good and (r := re.fullmatch(rf"({INT_LITERAL_RE})<={re.escape(c)}\.v_{suffix}&&{re.escape(c)}\.v_{suffix}<=({INT_LITERAL_RE})", mid)):
            good = int(r.group(1)) <= int(r.group(2))
    elif kind == "Bool":
        good = mid in ("", f"{c}.v_{suffix}==true", f"{c}.v_{suffix}==false")
    elif kind == "Ref" and suffix.startswith("ptr_"):
        v = f"{c}.v_{suffix}"
        wf = f"{v}==null||{v}==loc(block_of({v}),offset_of({v}))&&0<=offset_of({v})&&offset_of({v})<=len(block_of({v}))"
        guard = rf"{re.escape(v)}!=null&&offset_of\({re.escape(v)}\)<len\(block_of\({re.escape(v)}\)\)"
        if mid in (f"{v}==null", wf):
            good = True
        elif mid.startswith(wf + "&&") and (r := re.fullmatch(guard + r"==>(?P<lens>.*)", mid[len(wf) + 2:])):
            # a pointer to a struct: WF followed by the guarded WF_S conjunct (decision 0043 item 8)
            good = struct_wf_lens_problem(r.group("lens").split("&&"), v) is None
        elif r := re.fullmatch(re.escape(v) + r"==loc\((GB_[\w$']+)\(\),0\)", mid):
            target, good = r.group(1), True
    return good, target


_LEAF_PERM_RE = re.compile(r"^(?P<l>loc\(member\(.*\),[0-9]+\),0\))\.v_(?P<t>[\w$']+)\)$")
_LEAF_FLAG_RE = re.compile(r"^(?P<l>loc\(member\(.*\),[0-9]+\),0\))\.d_(?P<t>[\w$']+)\)&&(?P=l)\.d_(?P=t)(?:&&(?P<value>.*))?$")


def struct_row(row: str, fields: dict[str, str]) -> tuple[str, list[tuple[tuple[int, ...], str, str | None]]] | str:
    """The `program_start` row of a struct global (decision 0044 items 8 and 13), normalized: `acc(L.v_t) && acc(L.d_t) && L.d_t && <value>`
    for every leaf L, then exactly one `acc(lvloc(GB_g()).lv)`, nothing else.  Every L is a canonical chain `loc(member(<chain>, k), 0)` rooted at
    `loc(GB_g(), 0)` (no `block_of`, `parent_of`, arithmetic), each leaf once and no leaf's chain a prefix of another's.  Returns (the global,
    [(leaf chain, field suffix, `GB_h` of an address value or None)]) or the reason."""
    m = re.search(r"&&acc\(lvloc\((GB_[\w$']+)\(\)\)\.lv\)$", row)
    if m is None:
        return "it does not end with exactly one `acc(lvloc(GB_x()).lv)`"
    g = m.group(1)
    root = f"loc({g}(),0)"
    parts = row.split("&&acc(")
    if not parts[0].startswith("acc("):
        return "it does not start with a permission"
    parts[0] = parts[0][len("acc("):]
    if parts[-1] != f"lvloc({g}()).lv)":
        return f"it does not end with exactly one `acc(lvloc({g}()).lv)`"
    body = parts[:-1]
    if not body or len(body) % 2:
        return "a leaf is not exactly `acc(L.v_t) && acc(L.d_t) && L.d_t && <value>`"
    leaves: list[tuple[tuple[int, ...], str, str | None]] = []
    for i in range(0, len(body), 2):
        pm, fm = _LEAF_PERM_RE.match(body[i]), _LEAF_FLAG_RE.match(body[i + 1])
        if pm is None or fm is None or pm.group("l") != fm.group("l") or pm.group("t") != fm.group("t"):
            return "a leaf is not exactly `acc(L.v_t) && acc(L.d_t) && L.d_t && <value>` over one cell `loc(member(X, k), 0)` and one field suffix"
        chain = parse_struct_chain(pm.group("l"), root)
        if chain is None or not chain:
            return f"the leaf `{pm.group('l')}` is not a canonical chain `loc(member(<chain>, k), 0)` rooted at `loc({g}(), 0)`"
        suffix = pm.group("t")
        good, target = leaf_value(fields, suffix, pm.group("l"), fm.group("value") or "")
        if not good:
            return (f"the value of the leaf `{pm.group('l')}` (field `v_{suffix}`{'' if suffix in fields else ', not declared'}) is not a literal of its "
                    "type, `null`, `loc(GB_h(), 0)` of a declared global, or an unknown value's range / `WF`")
        leaves.append((chain, suffix, target))
    chains = [c for c, _s, _t in leaves]
    for a in chains:
        for b in chains:
            if a != b and b[:len(a)] == a:
                return f"the leaf {a} is a prefix of the leaf {b}: a nested struct is its leaves, not a cell"
    if len(chains) != len(set(chains)):
        return "a leaf twice (two full permissions to one cell would make `main` vacuous)"
    return g, leaves


def slot_text(g: str, chain: tuple[int, ...]) -> str:
    out = f"loc({g}(),0)"
    for k in chain:
        out = f"loc(member({out},{k}),0)"
    return out


def struct_facts_atoms(g: str, leaves: list[tuple[tuple[int, ...], str, str | None]]) -> list[str]:
    """The member lengths `len(member(X, k)) == 1` of every member block of the row's struct (every prefix of a leaf chain) and its slot facts
    `X != Y` (Y a nested slot, X the root or a nested slot whose chain Y's chain strictly extends), normalized (decision 0044 item 8)."""
    blocks = {c[:i] for c, _s, _t in leaves for i in range(1, len(c) + 1)}
    slots = {c[:i] for c, _s, _t in leaves for i in range(1, len(c))}
    atoms = [f"len(member({slot_text(g, b[:-1])},{b[-1]}))==1" for b in sorted(blocks)]
    atoms += [f"{slot_text(g, a)}!={slot_text(g, b)}" for b in sorted(slots) for a in sorted({()} | slots) if len(b) > len(a) and b[:len(a)] == a]
    return atoms


def global_facts_atoms_of(names: list[str], structs: dict[str, list[tuple[tuple[int, ...], str, str | None]]]) -> list[str]:
    """The facts of the unit (sorted atoms): each global's scalar facts, and each struct global's member lengths and slot facts."""
    atoms = global_facts_atoms(names)
    for g, leaves in structs.items():
        atoms += struct_facts_atoms(g, leaves)
    return sorted(atoms)


def lint_globals(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> tuple[bool, list[str] | None]:
    ok_domains = [d for d in decls if d.kind == "domain" and lint_domain(src, d, lambda _l, _w: None)]
    in_domain: set[int] = set()
    for d in ok_domains:
        in_domain.update(range(d.line + 1, block_close_line(src, d) + 1))
    gbs: dict[str, int] = {}  # name -> line, in declaration order
    for lineno, code in enumerate(src.code, start=1):
        if not UNIQUE_RE.search(code):
            if GB_FUNCTION_RE.match(code) and lineno in in_domain:
                add(lineno, "a `GB_` function of `domain Block` must be `unique`: two distinct globals are two blocks "
                            "(decision 0041 item 4)")
            continue
        m = GB_DECL_RE.match(code)
        if m is None or lineno not in in_domain:
            add(lineno, "`unique`: admitted only as `unique function GB_<name>(): Block` inside `domain Block` "
                        "(decision 0041 item 4); distinctness anywhere else would be an assumption")
        elif m.group("name") in gbs:
            add(lineno, f"`unique function {m.group('name')}` declared twice")
        else:
            gbs[m.group("name")] = lineno
    ps = [d for d in decls if d.kind == "method" and d.name == PROGRAM_START]
    if not gbs:
        for d in ps:
            add(d.line, "`method program_start` in a program with no `unique function GB_...`: it exists only for globals "
                        "(decision 0041 item 4)")
        return False, None
    if len(ps) != 1:
        add(min(gbs.values()), "a program with globals (`unique function GB_...`) has exactly one `method program_start()` "
                               f"(decision 0041 item 4), found {len(ps)}")
    fields = {m.group("suffix"): m.group("type") for l in src.code if (m := FIELD_DECL_RE.match(l))}
    facts_atoms: list[str] | None = global_facts_atoms(list(gbs))  # replaced below by the unit's facts, with the struct globals' lengths and slot facts
    for d in ps:
        want_head = "method program_start()"
        header = " ".join(src.code[d.line - 1].split())
        clauses = clause_texts(src, d)
        problem = None
        # A row is `acc(<cell>.v_T) && acc(<cell>.d_T) && <cell>.d_T && <value> && acc(lvloc(GB_x()).lv)` of a scalar global, or the
        # rows of a struct global's leaves (decision 0044 items 8 and 13); first read every row, then the facts they imply.
        rows: dict[str, tuple[int, str, str | None]] = {}  # global -> (line, field suffix, target of a `loc(GB_h(), 0)` value)
        srows: dict[str, tuple[int, list[tuple[tuple[int, ...], str, str | None]]]] = {}  # struct global -> (line, leaves)
        row_problems: list[tuple[int, str]] = []
        if d.has_body or header != want_head:
            problem = f"it must be a bodyless `{want_head}`"
        elif trusted_tag(src, d.line) is None:
            problem = "it must carry `// cerastes:trusted-contract <reason>`"
        elif not clauses or clauses[-1][0] != "decreases" or norm_viper(clauses[-1][2]) != "_" \
                or any(k != "ensures" for k, _l, _t in clauses[:-1]):
            problem = "its clauses are `ensures` rows and a final `decreases _`, nothing else"
        else:
            for _k, ln, text in clauses[1:-1]:
                row = norm_viper(text)
                if row.startswith("acc(loc(member("):
                    r = struct_row(row, fields)
                    if isinstance(r, str):
                        row_problems.append((ln, f"a `program_start` struct row is wrong: {r}"))
                        continue
                    g, leaves = r
                    if g not in gbs:
                        row_problems.append((ln, f"a `program_start` struct row names `{g}`, which is not a declared `GB_` global"))
                    elif g in rows or g in srows:
                        row_problems.append((ln, f"a second `program_start` row for `{g}` (two full permissions would make `main` vacuous)"))
                    else:
                        srows[g] = (ln, leaves)
                    continue
                m = re.match(r"acc\(loc\((?P<g>GB_[\w$']+)\(\),0\)\.v_(?P<s>[\w$']+)\)&&acc\(loc\((?P=g)\(\),0\)\.d_(?P=s)\)&&"
                             r"loc\((?P=g)\(\),0\)\.d_(?P=s)&&(?P<rest>.*)$", row)
                g = m.group("g") if m else ""
                tail = f"acc(lvloc({g}()).lv)"
                if m is None or g not in gbs or not (m.group("rest") == tail or m.group("rest").endswith("&&" + tail)):
                    row_problems.append((ln, "a `program_start` row is `acc(<cell>.v_T) && acc(<cell>.d_T) && <cell>.d_T && <value> && "
                                             "acc(lvloc(GB_x()).lv)` for a declared `GB_x`, <cell> being `loc(GB_x(), 0)`"))
                    continue
                suffix, c = m.group("s"), f"loc({g}(),0)"
                mid = m.group("rest")[:-len(tail)].rstrip("&")
                good, target = leaf_value(fields, suffix, c, mid)
                if not good:
                    row_problems.append((ln, f"the value of the `program_start` row of `{g}` (field `v_{suffix}`{'' if suffix in fields else ', not declared'}) is not "
                                             "a literal of its type, `null`, `loc(GB_h(), 0)` of a declared global, or an unknown value's range / `WF`"))
                    continue
                if g in rows or g in srows:
                    row_problems.append((ln, f"a second `program_start` row for `{g}` (two full permissions would make `main` vacuous)"))
                    continue
                rows[g] = (ln, suffix, target)
            # the facts follow the rows: with a row that is wrong or missing the expected facts are unknown, and the row is the problem
            missing = [g for g in gbs if g not in rows and g not in srows]
            facts_atoms = None if row_problems or missing else global_facts_atoms_of(list(gbs), {g: lv for g, (_l, lv) in srows.items()})
            if facts_atoms is not None and (len(clauses) < 2 or sorted(norm_viper(clauses[0][2]).split("&&")) != facts_atoms):
                problem = ("its first `ensures` row must be the global facts: `len(GB_x()) == 1 && lvloc(GB_x()) == loc(GB_x(), -2)` "
                           "for each declared `GB_x`, each once, and for each struct global the member lengths `len(member(X, k)) == 1` of every "
                           "slot of its row and its slot facts `X != Y` (decision 0044 item 8), nothing else")
        if problem:
            add(d.line, f"`method program_start` is not the trusted start of decision 0041 item 4: {problem}")
            continue
        for ln, why in row_problems:
            add(ln, why)
        shapes: dict[str, frozenset] = {}  # pointer field suffix -> the shape of the struct globals it points at
        # every address value `loc(GB_h(), 0)`, of a scalar global's row or of a struct global's leaf: (owner, line, field suffix, target)
        addrs = [(g, ln, suffix, target) for g, (ln, suffix, target) in rows.items() if target is not None]
        addrs += [(g, ln, suffix, target) for g, (ln, leaves) in srows.items() for _c, suffix, target in leaves if target is not None]
        for g, ln, suffix, target in addrs:
            pointee = suffix[len("ptr_"):]
            if target in rows:
                if rows[target][1] != pointee:
                    add(ln, f"the row of `{g}` points at `{target}`, which is not a declared global of the pointee's field "
                            f"`v_{pointee}` (decision 0041 item 4)")
            elif target in srows:
                shape = frozenset((c, sfx) for c, sfx, _t in srows[target][1])
                if not pointee.startswith("struct_"):
                    add(ln, f"`{g}` has an address of the struct global `{target}` in a field `v_{suffix}` that is not a pointer to a struct "
                            "(decision 0044 item 13: the address of a struct global is the value of a `ptr_struct_<n>_<tag>` field)")
                elif shapes.setdefault(suffix, shape) != shape:
                    add(ln, f"the field `v_{suffix}` points at struct globals of two different shapes (`{target}` is not of the pointee's type)")
            else:
                add(ln, f"`{g}` points at `{target}`, which is not a declared global with a row (decision 0041 item 4)")
        for g in gbs:
            if g not in rows and g not in srows and not row_problems:
                add(d.line, f"`method program_start` has no row for `{g}` (decision 0041 item 4: one row per global)")
    # `program_start` is reached once, as the first statement of `main`; every other method takes the facts
    methods = [d for d in decls if d.kind == "method"]
    ps_lines = {d.line for d in ps}
    calls = [n for n, code in enumerate(src.code, start=1) if PROGRAM_START_CALL_RE.search(code) and n not in ps_lines]
    entries: list[Decl] = []
    for n in calls:
        owner = next((d for d in methods if d.line <= n <= d.end_line), None)
        if owner is not None and owner.has_body and first_statement(src, owner) == ("program_start()", n):
            entries.append(owner)
        else:
            add(n, "`program_start()` is called only as the first statement of `main`'s method (decision 0041 item 4)")
    if len(calls) > 1:
        add(calls[1], "`program_start()` is called once (a second call would hold every global's permission twice)")
    # The entry method is reached by the program's start only: a call of it (from another method or from itself, after `program_start()`) would
    # run `program_start()` again and inhale every global's permission twice, and it takes no `requires` (it holds the globals from `program_start`).
    decl_lines = {d.line for d in methods}
    for e in entries:
        if any(k == "requires" for k, _l, _t in clause_texts(src, e)):
            add(e.line, f"`method {e.name}` starts the program (its first statement is `program_start()`): it takes no `requires` "
                        "(decision 0041 item 4)")
        call = re.compile(r"(?<![\w$'.])" + re.escape(e.name) + r"\s*\(")
        for n, code in enumerate(src.code, start=1):
            if n not in decl_lines and call.search(code):
                add(n, f"a call of the entry method `{e.name}`: it runs `program_start()` once, as the program starts; a call would "
                       "inhale every global's permission again (decision 0041 item 4)")
    exempt = {PROGRAM_START} | {e.name for e in entries} | {e.name + "__wf" for e in entries}
    for d in methods:
        if d.name in exempt or d.name.startswith(ALLOC_PREFIX):
            continue
        reqs = [(ln, t) for k, ln, t in clause_texts(src, d) if k == "requires"]
        if facts_atoms is not None and (not reqs or sorted(norm_viper(reqs[0][1]).split("&&")) != facts_atoms):
            add(reqs[0][0] if reqs else d.line,
                f"`method {d.name}` of a program with globals must take the global facts as its first `requires` "
                "(`len(GB_x()) == 1 && lvloc(GB_x()) == loc(GB_x(), -2)` for each global, before the amounts' rows; "
                "decision 0041 item 4)")
    return True, facts_atoms


# The ghost amounts (decision 0037 items 6 and 14): a method's `Perm` formals are the amounts of the atoms of its contract.
WILDCARD_RE = re.compile(r"(?<![\w$'.])wildcard(?![\w$'])")
PERM_FORMAL_RE = re.compile(r"^\s*(?P<name>[A-Za-z_$][\w$']*)\s*:\s*Perm\s*$")
CLAUSE_START_RE = re.compile(r"^\s*(requires|ensures|invariant|decreases)\b")


def method_header_text(src: Source, d: Decl) -> str:
    """The header of a method: its lines up to the first clause or the opening brace, joined."""
    header = []
    for n in range(d.line, d.end_line + 1):
        line = src.code[n - 1]
        if CLAUSE_START_RE.match(line) or line.strip().startswith("{"):
            break
        header.append(line)
    return " ".join(header)


def parenthesized_list(text: str, start: int) -> list[str]:
    """The top-level comma-separated parts of the parenthesis that opens at `text[start]`."""
    depth, parts, cur = 0, [], []
    for ch in text[start:]:
        if ch == "(":
            depth += 1
            if depth == 1:
                continue
        elif ch == ")":
            depth -= 1
            if depth == 0:
                break
        if depth == 1 and ch == ",":
            parts.append("".join(cur))
            cur = []
        else:
            cur.append(ch)
    parts.append("".join(cur))
    return [x for x in parts if x.strip()]


def method_formals(src: Source, d: Decl) -> list[str]:
    """The text of each formal of a method (the first parenthesis of its header, split at top-level commas)."""
    text = method_header_text(src, d)
    start = text.find("(", text.find(d.name) + len(d.name))
    if start < 0:
        return []
    return parenthesized_list(text, start)


RESULT_PART_RE = re.compile(r"^\s*(?P<name>[A-Za-z_$][\w$']*)\s*:\s*(?P<type>.+?)\s*$")


def method_results(src: Source, d: Decl) -> list[tuple[str, str]]:
    """The (name, type) of each result of a method: the parenthesis after `returns` in its header."""
    text = method_header_text(src, d)
    after = text.find(d.name) + len(d.name)
    m = re.search(r"\)\s*returns\s*\(", text[after:])
    if m is None:
        return []
    start = after + m.end() - 1
    return [(r.group("name"), r.group("type")) for part in parenthesized_list(text, start) if (r := RESULT_PART_RE.match(part))]


def perm_result_rows(src: Source, d: Decl, o: str) -> tuple[list[int], int]:
    """(the lines of the `ensures none < o && o <= write` of a ghost `Perm` result, the first other clause that mentions it)."""
    word = re.compile(r"(?<![\w$'.])" + re.escape(o) + r"(?![\w$'])")
    bounds = re.sub(r"[\s()]", "", f"none<{o}&&{o}<=write")
    rows, first_mention = [], 0
    for n in range(d.line + 1, d.end_line + 1):
        line = src.code[n - 1]
        if not CLAUSE_START_RE.match(line):
            continue
        if line.strip().startswith("ensures") and re.sub(r"[\s()]", "", line.strip()[len("ensures"):]) == bounds:
            rows.append(n)
        elif word.search(line) and not first_mention:
            first_mention = n
    return rows, first_mention


def perm_result_has_row(src: Source, d: Decl, o: str) -> bool:
    """Whether the ghost `Perm` result `o` has exactly one `ensures none < o && o <= write` and no clause before it mentions it."""
    rows, first_mention = perm_result_rows(src, d, o)
    return len(rows) == 1 and not (first_mention and first_mention < rows[0])


def lint_ghost_amounts(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    for lineno, code in enumerate(src.code, start=1):
        if WILDCARD_RE.search(code):
            add(lineno, "`wildcard`: an amount Silicon chooses is an assumption (decision 0037 item 14); ghost amounts "
                        "are `Perm` formals with `requires none < a && a < write`")
    for d in decls:
        if d.kind != "method":
            continue
        amounts = [m.group("name") for f in method_formals(src, d) if (m := PERM_FORMAL_RE.match(f))]
        for a in amounts:
            word = re.compile(r"(?<![\w$'.])" + re.escape(a) + r"(?![\w$'])")
            bounds = re.sub(r"[\s()]", "", f"none<{a}&&{a}<write")
            bound_lines = []
            first_mention = 0
            for n in range(d.line + 1, d.end_line + 1):
                line = src.code[n - 1]
                if not CLAUSE_START_RE.match(line):
                    continue
                if re.sub(r"[\s()]", "", line.strip()[len("requires"):]) == bounds and line.strip().startswith("requires"):
                    bound_lines.append(n)
                elif word.search(line) and not first_mention:
                    first_mention = n
            if len(bound_lines) != 1:
                add(d.line, f"`method {d.name}`: the ghost amount `{a}` must have exactly one `requires none < {a} && {a} < write` "
                            f"(decision 0037 items 6 and 14), found {len(bound_lines)}")
            elif first_mention and first_mention < bound_lines[0]:
                add(first_mention, f"`method {d.name}`: a clause mentions the ghost amount `{a}` before its bounds "
                                   f"`requires none < {a} && {a} < write` (line {bound_lines[0]}; Silicon's well-formedness check "
                                   "needs the bounds first, decision 0037 item 6)")
        # Decision 0052 item 5: a ghost `Perm` *result* (the output amount of a partnerless `ensures` atom) has exactly one
        # `ensures none < o && o <= write`, before any clause that mentions it: the amount is positive and at most `write`
        # whatever the body or a trusted contract says, so a caller's `t, o := f(...)` receives a bounded amount.
        for o, typ in method_results(src, d):
            if typ != "Perm":
                continue
            rows, first_mention = perm_result_rows(src, d, o)
            if len(rows) != 1:
                add(d.line, f"`method {d.name}`: the ghost amount result `{o}` must have exactly one `ensures none < {o} && {o} <= write` "
                            f"(decision 0052 item 5), found {len(rows)}")
            elif first_mention and first_mention < rows[0]:
                add(first_mention, f"`method {d.name}`: a clause mentions the ghost amount result `{o}` before its bounds "
                                   f"`ensures none < {o} && {o} <= write` (line {rows[0]}; decision 0052 item 5)")


# The ghost amounts of loops (decision 0038 item 9): snapshots of `perm(...)` in `Perm` locals, used as amounts in invariants.
PERM_LOCAL_RE = re.compile(r"(?<![\w$'.])var\s+(?P<names>[A-Za-z_$][\w$']*(?:\s*,\s*[A-Za-z_$][\w$']*)*)\s*:\s*Perm\b"
                           r"(?P<rest>.*)$")
PERM_CALL_RE = re.compile(r"(?<![\w$'.])perm\s*\(")


def perm_local_names(src: Source, d: Decl) -> set[str]:
    """The `Perm` locals of a method with a body: its `var <name>: Perm` lines after the opening brace (the formals and
    the results of the header are not locals)."""
    names: set[str] = set()
    if not d.has_body:
        return names
    for n in range(d.body_line, d.end_line + 1):
        if m := PERM_LOCAL_RE.search(src.code[n - 1]):
            names |= {x.strip() for x in m.group("names").split(",")}
    return names


def _statement_rhs(code: str, start: int) -> str:
    """The text of an assignment's right-hand side that starts at `start`: up to a top-level `;` or `}` or the line's end."""
    depth = 0
    for i in range(start, len(code)):
        ch = code[i]
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        elif depth == 0 and ch in ";}":
            return code[start:i].strip()
    return code[start:].strip()


def _strip_outer_parens(text: str) -> str:
    text = text.strip()
    while text.startswith("(") and text.endswith(")"):
        depth = 0
        for i, ch in enumerate(text):
            depth += ch == "("
            depth -= ch == ")"
            if depth == 0 and i < len(text) - 1:
                return text
        text = text[1:-1].strip()
    return text


def perm_value_ok(text: str) -> bool:
    """Whether `text` is `perm(...)`, `none`, `write` or a conditional `c ? a : b` of those (parentheses anywhere)."""
    text = _strip_outer_parens(text)
    if text in ("none", "write"):
        return True
    m = PERM_CALL_RE.match(text)
    if m:
        depth = 0
        for i in range(m.end() - 1, len(text)):
            depth += text[i] == "("
            depth -= text[i] == ")"
            if depth == 0:
                return i == len(text) - 1
        return False
    depth, q = 0, -1
    for i, ch in enumerate(text):
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        elif ch == "?" and depth == 0:
            q = i
            break
    if q < 0:
        return False
    depth, nested, colon = 0, 0, -1
    for i in range(q + 1, len(text)):
        ch = text[i]
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        elif depth == 0 and ch == "?":
            nested += 1
        elif depth == 0 and ch == ":":
            if nested == 0:
                colon = i
                break
            nested -= 1
    if colon < 0:
        return False
    return perm_value_ok(text[q + 1:colon]) and perm_value_ok(text[colon + 1:])


def while_loops(src: Source, d: Decl):
    """[(line of `while`, the tokens of its `invariant` clauses, the tokens of its body)] of a method with a body; a token is
    (text, line).  The body is between the braces that open it and the matching `}` (a quantifier's trigger set `:: { f(x) }`
    is not a body)."""
    tokens: list[tuple[str, int]] = []
    for lineno in range(d.line, d.end_line + 1):
        tokens += [(m.group(), lineno) for m in TOKEN_RE.finditer(src.code[lineno - 1])]
    out = []
    n = len(tokens)
    for k, (text, lineno) in enumerate(tokens):
        if text != "while" or (k and tokens[k - 1][0] in ("::", "'")):
            continue
        i = k + 1
        if i < n and tokens[i][0] == "(":
            depth = 0
            while i < n:
                depth += tokens[i][0] == "("
                depth -= tokens[i][0] == ")"
                i += 1
                if depth == 0:
                    break
        inv: list[tuple[str, int]] = []
        clause = ""
        trigger_end = -2
        while i < n:
            t = tokens[i][0]
            prev = tokens[i - 1][0] if i else ""
            if t == "{":
                if prev == "::" or i - 1 == trigger_end:
                    level = 0
                    while i < n:
                        level += tokens[i][0] == "{"
                        level -= tokens[i][0] == "}"
                        if level == 0:
                            break
                        if clause == "invariant":
                            inv.append(tokens[i])
                        i += 1
                    trigger_end = i
                    i += 1
                    continue
                break
            if t in ("invariant", "decreases"):
                clause = t
            elif clause == "invariant":
                inv.append(tokens[i])
            i += 1
        body: list[tuple[str, int]] = []
        if i < n and tokens[i][0] == "{":
            level = 0
            j = i
            while j < n:
                level += tokens[j][0] == "{"
                level -= tokens[j][0] == "}"
                body.append(tokens[j])
                j += 1
                if level == 0:
                    break
        out.append((lineno, inv, body))
    return out


GHOST_REF_LOCAL_RE = re.compile(r"(?<![\w$'.])var\s+(?P<names>[A-Za-z_$][\w$']*(?:\s*,\s*[A-Za-z_$][\w$']*)*)\s*:\s*Ref\b")
EB_NAME_RE = re.compile(r"eb_\d+")
LB_NAME_RE = re.compile(r"lb_\d+__\d+")
AW_NAME_RE = re.compile(r"a[wb]_q\d+__\d+")
REF_FIELD_END_RE = re.compile(r"\.v_ptr_[\w$']+$")
LEAF_CELL_RE = re.compile(r"^loc\(member\((?P<base>.*), -?\d+\), 0\)$", re.S)


def ref_read_ok(text: str, conditional: bool) -> bool:
    """Whether `text` is `null` or a plain read of a pointer cell's field (`<place>.v_ptr_...`, a `?:` inside the place is
    part of the place), or, when `conditional` is set, a `c ? a : b` of those (the chain-guarded read of decision 0045 items 4
    and 6: `ite(<framing guards>, read, null)`)."""
    text = _strip_outer_parens(text)
    if text == "null":
        return True
    depth, q = 0, -1
    for i, ch in enumerate(text):
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        elif ch == "?" and depth == 0:
            q = i
            break
    if q < 0:
        return bool(REF_FIELD_END_RE.search(text))
    if not conditional:
        return False
    depth, nested, colon = 0, 0, -1
    for i in range(q + 1, len(text)):
        ch = text[i]
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        elif depth == 0 and ch == "?":
            nested += 1
        elif depth == 0 and ch == ":":
            if nested == 0:
                colon = i
                break
            nested -= 1
    if colon < 0:
        return False
    return ref_read_ok(text[q + 1:colon], True) and ref_read_ok(text[colon + 1:], True)


def lint_ghost_refs(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0045 item 14: the ghost `Ref` locals that snapshot the object a contract place designates.  `eb_<N>` (the
    method-entry snapshot of a load-rooted `requires` place) is assigned once, outside every loop, from `null` or a read of a
    pointer cell (or a conditional of those: the chain-guarded read); `lb_<N>__<L>` (the loop-entry snapshot) only from `null`
    or a plain read of a pointer cell, outside every loop body (the loop never assigns it, so the loop's automatic place keeps
    one object); `aw_q<j>__<L>` (a written atom's object at the top of an iteration) is assigned once, from such a read or a
    conditional of them, and only in the leading statements of a loop body, before anything else of the iteration.
    `ab_q<j>__<L>` (decision 0054 item 4: a written `owns_block` site's pointer at the top of an iteration) is the same ghost."""
    for d in decls:
        if d.kind != "method" or not d.has_body:
            continue
        names: set[str] = set()
        for n in range(d.body_line, d.end_line + 1):
            if m := GHOST_REF_LOCAL_RE.search(src.code[n - 1]):
                names |= {x.strip() for x in m.group("names").split(",")}
        ghosts = {x for x in names if EB_NAME_RE.fullmatch(x) or LB_NAME_RE.fullmatch(x) or AW_NAME_RE.fullmatch(x)}
        if not ghosts:
            continue
        loops = while_loops(src, d)
        bodies = [(b[0][1], b[-1][1], b) for _ln, _inv, b in loops if b]   # (line of `{`, line of `}`, tokens)
        assigns: dict[str, list[tuple[int, str]]] = {g: [] for g in ghosts}
        for n in range(d.body_line, d.end_line + 1):
            code = src.code[n - 1]
            for g in sorted(ghosts):
                for a in re.finditer(r"(?<![\w$'.])" + re.escape(g) + r"\s*:=", code):
                    assigns[g].append((n, _statement_rhs(code, a.end())))

        def in_loop(line: int) -> bool:
            return any(lo < line <= hi for lo, hi, _b in bodies)

        for g in sorted(ghosts):
            lst = assigns[g]
            if EB_NAME_RE.fullmatch(g):
                if len(lst) != 1:
                    add(d.line, f"ghost `Ref` local `{g}` is assigned {len(lst)} times: the method-entry snapshot is assigned once "
                                "(decision 0045 item 14)")
                for n, rhs in lst:
                    if in_loop(n):
                        add(n, f"ghost `Ref` local `{g}` is assigned inside a loop: the method-entry snapshot is assigned once at the "
                               "method's entry (decision 0045 item 14)")
                    elif not ref_read_ok(rhs, True):
                        add(n, f"ghost `Ref` local `{g}` is assigned `{rhs}`: a snapshot is `null`, a read of a pointer cell or a "
                               "conditional of those (decision 0045 item 14)")
            elif LB_NAME_RE.fullmatch(g):
                if not 1 <= len(lst) <= 2:
                    add(d.line, f"ghost `Ref` local `{g}` is assigned {len(lst)} times: `:= null`, then at most one guarded read, "
                                "right before its loop (decision 0045 item 14)")
                for k, (n, rhs) in enumerate(lst):
                    if in_loop(n):
                        add(n, f"ghost `Ref` local `{g}` is assigned inside a loop: the loop-entry snapshot is assigned only before "
                               "its loop (decision 0045 item 14)")
                    elif (k == 0 and rhs != "null") or (k > 0 and not ref_read_ok(rhs, False)):
                        add(n, f"ghost `Ref` local `{g}` is assigned `{rhs}`: `null` first, then a plain read of a pointer cell "
                               "(decision 0045 item 14)")
            else:
                if len(lst) != 1:
                    add(d.line, f"ghost `Ref` local `{g}` is assigned {len(lst)} times: a written atom's object is assigned once, "
                                "at the top of an iteration (decision 0045 item 14)")
                for n, rhs in lst:
                    host = [(lo, hi) for lo, hi, _b in bodies if lo < n <= hi]
                    if not host:
                        add(n, f"ghost `Ref` local `{g}` is assigned outside a loop body: it holds the written atom's object at the "
                               "top of each iteration (decision 0045 item 14)")
                        continue
                    lo, _hi = min(host, key=lambda h: h[1] - h[0])
                    # only ghost assignments between the loop's `{` and this one
                    for m in range(lo + 1, n):
                        if src.code[m - 1].strip() and not any(re.match(r"\s*" + re.escape(x) + r"\s*:=", src.code[m - 1]) for x in ghosts if AW_NAME_RE.fullmatch(x)):
                            add(n, f"ghost `Ref` local `{g}` is assigned after other statements of the loop body (line {m}): it is "
                                   "the first statement of an iteration (decision 0045 item 14)")
                            break
                    leaf = LEAF_CELL_RE.match(_strip_outer_parens(rhs))   # a struct atom's leaf cell: loc(member(<chain read>, k), 0)
                    if not ref_read_ok(leaf.group("base") if leaf else rhs, True):
                        add(n, f"ghost `Ref` local `{g}` is assigned `{rhs}`: a read of a pointer cell, `null`, a conditional of "
                               "those or, for a struct atom, a leaf cell `loc(member(<such a read>, k), 0)` (decision 0045 item 14)")



CALL_RHS_RE = re.compile(r"^(?P<callee>[A-Za-z_$][\w$']*)\s*\(.*\)$", re.S)


def perm_call_target_ok(src: Source, by_name: dict[str, Decl], code: str, start: int, end: int, rhs: str) -> bool:
    """Decision 0052 item 5 (amending 0038 item 9): a `Perm` local that is a target of a multi-target call `t, o := f(...)` is the
    output amount of a partnerless atom when the callee is a method of this file whose result at that position is a `Perm` with
    exactly one `ensures none < o && o <= write` before any clause that mentions it (the amount a caller receives is bounded
    whatever the callee does).  The target is `code[start:end]`; every other call target, and every other right-hand side, is
    the rule of decision 0038 item 9."""
    m = CALL_RHS_RE.match(rhs)
    callee = by_name.get(m.group("callee")) if m else None
    if callee is None:
        return False
    colon = code.index(":=", end)
    targets = re.search(r"((?:[A-Za-z_$][\w$']*\s*,\s*)*[A-Za-z_$][\w$']*)\s*$", code[:colon])
    if targets is None:
        return False
    names = [t.strip() for t in targets.group(1).split(",")]
    results = method_results(src, callee)
    position = len(re.findall(r"[A-Za-z_$][\w$']*", code[targets.start():start]))
    if len(results) != len(names) or position >= len(results):
        return False
    o, typ = results[position]
    return typ == "Perm" and perm_result_has_row(src, callee, o)


def lint_perm_locals(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    """Decision 0038 item 9: the amounts of a loop's invariant are `Perm` locals that snapshot what the method holds.  A
    local is assigned only `perm(...)`, `none`, `write` or a conditional of those (an assignment of anything else, `wildcard`
    included, could give the invariant an amount that the method does not hold or above `write`); it is never assigned inside
    a loop whose invariant mentions it (the loop head havocs what the body assigns, so the amount could exceed `write`
    and make the body vacuous); and `perm(...)` is not written in an invariant (Viper refuses it: the snapshot is the way)."""
    by_name = {x.name: x for x in decls if x.kind == "method"}
    for d in decls:
        if d.kind != "method" or not d.has_body:
            continue
        names = perm_local_names(src, d)
        for n in range(d.body_line, d.end_line + 1):
            code = src.code[n - 1]
            if (m := PERM_LOCAL_RE.search(code)) and ":=" in m.group("rest"):
                rhs = _statement_rhs(m.group("rest"), m.group("rest").index(":=") + 2)
                if not perm_value_ok(rhs) and not WILDCARD_RE.search(rhs):  # wildcard: reported by its own rule
                    add(n, f"`Perm` local `{m.group('names')}` is initialized with `{rhs}`: a snapshot is `perm(...)`, `none`, "
                           "`write` or a conditional of those (decision 0038 item 9)")
            for name in sorted(names):
                # `name` is a target when `:=` follows it directly or after more targets: `a, b := m(args)` assigns both (decision
                # 0044 item 13: a multi-target call is one assignment of every target; the first target must be seen too)
                for a in re.finditer(r"(?<![\w$'.])" + re.escape(name) + r"\s*(?=(?:,\s*[A-Za-z_$][\w$']*\s*)*:=)", code):
                    rhs = _statement_rhs(code, code.index(":=", a.end()) + 2)
                    if not perm_value_ok(rhs) and not WILDCARD_RE.search(rhs):  # wildcard: reported by its own rule
                        if perm_call_target_ok(src, by_name, code, a.start(), a.end(), rhs):
                            continue
                        add(n, f"`Perm` local `{name}` is assigned `{rhs}`: a snapshot is `perm(...)`, `none`, `write` or a "
                               "conditional of those, never arithmetic or another variable (decision 0038 item 9; `wildcard` has its own rule)")
        for line, inv, body in while_loops(src, d):
            used = {t for t, _ln in inv} & names
            for k, (t, ln) in enumerate(inv):
                if t == "perm" and k + 1 < len(inv) and inv[k + 1][0] == "(" and (k == 0 or inv[k - 1][0] != "."):
                    add(ln, "`perm(...)` in a loop invariant: Viper refuses it (decision 0038 item 9); the invariant uses a "
                            "`Perm` local snapshotted before the loop")
            for k, (t, ln) in enumerate(body):
                j = k + 1  # the targets after this one: `kv, other := m(args)` assigns kv as well
                while j + 1 < len(body) and body[j][0] == "," and re.fullmatch(r"[A-Za-z_$][\w$']*", body[j + 1][0]):
                    j += 2
                if t in used and j < len(body) and body[j][0] == ":=" and (k == 0 or body[k - 1][0] != "."):
                    add(ln, f"`Perm` local `{t}` is assigned inside the loop of line {line}, whose invariant uses it as an "
                            "amount: the loop head havocs what the body assigns (decision 0038 item 9)")


# The allocator ghost of an owning loop variable in a vlibc heap program (decision 0052 item 3 Loops, shapes L1-L3): `tve_<v>__<loop> :=
# (<x> != null) ? tv_<y> : true` is the token value the owner held at the loop's entry, a method-level `Bool` that the loop's invariant and
# its rows compare with the token.  It is assigned only that way (the value is the owner's own ghost, never a read of a token), and never
# inside a loop whose invariant mentions it (the loop head would havoc it and the invariant would no longer say which allocator's block
# the owner holds).
TVE_WORD_RE = re.compile(r"(?<![\w$'.])tve_[\w$']*")
TVE_ASSIGN_RE = re.compile(r"^\s*tve_[\w$']*\s*:=\s*\(\s*[A-Za-z_$][\w$']*\s*!=\s*null\s*\)\s*\?\s*tv_[\w$']*\s*:\s*true\s*$")


def lint_owner_loop_ghosts(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    for lineno, code in enumerate(src.code, start=1):
        m = re.match(r"^\s*(?P<name>tve_[\w$']*)\s*:=", code)
        if m and not TVE_ASSIGN_RE.match(code):
            add(lineno, f"`{m.group('name')}` is assigned something but `(<x> != null) ? tv_<y> : true`: the allocator ghost of an owning loop "
                        "variable is its own token value, never a read of a token or another expression (decision 0052 item 3)")
    for d in decls:
        if d.kind != "method" or not d.has_body:
            continue
        for line, inv, body in while_loops(src, d):
            used = {t for t, _ln in inv if TVE_WORD_RE.fullmatch(t)}
            for k, (t, ln) in enumerate(body):
                if t in used and k + 1 < len(body) and body[k + 1][0] == ":=" and (k == 0 or body[k - 1][0] != "."):
                    add(ln, f"`{t}` is assigned inside the loop of line {line}, whose invariant mentions it: the loop head havocs what "
                            "the body assigns (decision 0052 item 3)")


# The loop-entry ghosts of the loop carriers (decision 0054 items 3 and 4), each a local that the lowering assigns once before its loop
# and the loop never assigns, so the invariant that names it states a fact about the loop's entry and not an assumption:
#   `ev_<v>__<L>` (`Ref`): the owner `v`'s value at entry, `ev_<v>__<L> := <v>`; the value set `v == ev || v == null || v == x...` names it;
#   `hs_<v>__<L>` (`Bool`): `held_<v>` at entry, `hs_<v>__<L> := held_<v>`; the invariant is `held_<v> == hs_<v>__<L>`, its back-edge row
#       `if (!(hs_<v>__<L> ==> held_<v>)) {`, `assert false`, `}`;
#   `dup_...__<L>` and `ad_...__<L>` (`Bool`): whether two carriers shared a block at entry, `S := ((a != null) && (b != null)) &&
#       (block_of(a) == block_of(b))`; it is used negated (`!S`) as a carrier guard and, positively, only in the exclusivity clause
#       `invariant S || (!(<the same pair formula>))` and its back-edge row `if (!(S || (!(<formula>)))) {`.
# Each is assigned exactly once, before the `while` of its own loop (the number after the last `__`, in the order of the loops'
# `while`s; an inner loop's ghost may stand in the outer loop's body), from exactly that right-hand side.
LOOP_GHOST_LOCAL_RE = re.compile(r"(?<![\w$'.])var\s+(?P<names>[A-Za-z_$][\w$']*(?:\s*,\s*[A-Za-z_$][\w$']*)*)\s*:\s*(?P<type>[A-Za-z]+)\b")
EV_NAME_RE = re.compile(r"^ev_(?P<v>[A-Za-z_$][\w$']*)__(?P<l>\d+)$")
HS_NAME_RE = re.compile(r"^hs_(?P<v>[A-Za-z_$][\w$']*)__(?P<l>\d+)$")
SNAP_NAME_RE = re.compile(r"^(?:dup|ad)_[\w$']*__\d+$")
PAIR_FORMULA_RE = re.compile(r"^\(\((?P<a>[A-Za-z_$][\w$']*)!=null\)&&\((?P<b>[A-Za-z_$][\w$']*)!=null\)\)&&"
                             r"\(block_of\((?P=a)\)==block_of\((?P=b)\)\)$")
INV_LINE_RE = re.compile(r"^\s*invariant\b(?P<rest>.*)$")


def _nospace(text: str) -> str:
    return re.sub(r"\s+", "", text)


def lint_loop_entry_ghosts(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    for d in decls:
        if d.kind != "method" or not d.has_body:
            continue
        types: dict[str, str] = {}
        for n in range(d.body_line, d.end_line + 1):
            if m := LOOP_GHOST_LOCAL_RE.search(src.code[n - 1]):
                for x in (x.strip() for x in m.group("names").split(",")):
                    types[x] = m.group("type")
        ghosts: dict[str, str] = {}
        for x, t in types.items():
            kind = "ev" if EV_NAME_RE.match(x) else "hs" if HS_NAME_RE.match(x) else "snap" if SNAP_NAME_RE.match(x) else None
            if kind is None:
                continue
            ghosts[x] = kind
            if t != ("Ref" if kind == "ev" else "Bool"):
                add(d.line, f"loop-entry ghost `{x}` is a `{t}` local: `ev_...` is a `Ref`, `hs_...`, `dup_...` and `ad_...` are `Bool` "
                            "(decision 0054 item 3)")
        if not ghosts:
            continue
        loops = while_loops(src, d)   # numbered in the order of their `while`, as the lowering numbers them
        assigns: dict[str, list[tuple[int, str]]] = {g: [] for g in ghosts}
        for n in range(d.body_line, d.end_line + 1):
            code = src.code[n - 1]
            for g in ghosts:
                for a in re.finditer(r"(?<![\w$'.])" + re.escape(g) + r"\s*:=", code):
                    assigns[g].append((n, _statement_rhs(code, a.end())))
        rhs_of: dict[str, str] = {}
        for g, kind in sorted(ghosts.items()):
            lst = assigns[g]
            loop_no = int(re.search(r"__(\d+)$", g).group(1))
            if len(lst) != 1:
                add(d.line, f"loop-entry ghost `{g}` is assigned {len(lst)} times: it is assigned once, right before its loop "
                            "(decision 0054 item 3)")
            if loop_no >= len(loops):
                add(d.line, f"loop-entry ghost `{g}` names loop {loop_no}, which the method does not have (decision 0054 item 3)")
            for n, rhs in lst:
                if loop_no < len(loops) and n >= loops[loop_no][0]:
                    add(n, f"loop-entry ghost `{g}` is assigned at or after the `while` of its own loop: the loop head would havoc it and "
                           "the invariant that names it would no longer say anything about the entry (decision 0054 item 3)")
                if kind == "ev":
                    want = EV_NAME_RE.match(g).group("v")
                    ok = _nospace(rhs) == want
                    what = f"`{want}`, the owner's own value"
                elif kind == "hs":
                    want = "held_" + HS_NAME_RE.match(g).group("v")
                    ok = _nospace(rhs) == want
                    what = f"`{want}`"
                else:
                    ok = PAIR_FORMULA_RE.match(_nospace(rhs)) is not None
                    what = "`((a != null) && (b != null)) && (block_of(a) == block_of(b))`"
                    rhs_of[g] = _nospace(rhs)
                if not ok:
                    add(n, f"loop-entry ghost `{g}` is assigned `{rhs}`: its right-hand side is exactly {what} (decision 0054 item 3)")
        alt = "|".join(re.escape(g) for g in sorted(ghosts, key=len, reverse=True))
        word_re = re.compile(r"(?<![\w$'.])(" + alt + r")(?![\w$'])")
        for n in range(d.body_line, d.end_line + 1):
            code = src.code[n - 1]
            if not word_re.search(code):
                continue
            if re.match(r"^\s*(?:var\s+(?:" + alt + r")\s*:|(?:" + alt + r")\s*:=)", code):
                continue
            norm = _nospace(code)
            for g in sorted({m.group(1) for m in word_re.finditer(code)}):
                kind = ghosts[g]
                if kind == "hs":
                    v = HS_NAME_RE.match(g).group("v")
                    if norm == f"invariantheld_{v}=={g}":
                        continue
                    if is_ending_row(src, n) and norm == f"if(!({g}==>held_{v})){{":
                        continue
                    add(n, f"`{g}` appears outside `invariant held_{v} == {g}` and its back-edge row `if (!({g} ==> held_{v})) {{`: it is "
                           "the loop-entry value of the owner's flag and nothing else (decision 0054 item 3)")
                elif kind == "ev":
                    v = EV_NAME_RE.match(g).group("v")
                    m = INV_LINE_RE.match(code)
                    terms = _nospace(m.group("rest")).replace("(", "").replace(")", "").split("||") if m else []
                    if terms and terms[0] == f"{v}=={g}" and all(re.fullmatch(re.escape(v) + r"==[A-Za-z_$][\w$']*", t) for t in terms[1:]):
                        continue
                    add(n, f"`{g}` appears outside the value set `invariant {v} == {g} || {v} == null || {v} == x...`: it is the "
                           "owner's entry value and only the value set names it (decision 0054 item 3)")
                else:
                    f = rhs_of.get(g)
                    if f is not None and PAIR_FORMULA_RE.match(f):
                        if norm == f"invariant{g}||(!({f}))":
                            continue
                        if is_ending_row(src, n) and norm == f"if(!({g}||(!({f})))){{":
                            continue
                    bare = [m for m in word_re.finditer(code) if m.group(1) == g and not code[:m.start()].rstrip().endswith("!")]
                    if bare:
                        add(n, f"`{g}` is used positively outside the exclusivity clause `invariant {g} || (!(<its pair formula>))` and its "
                               f"back-edge row: a carrier guard is `!{g}` (decision 0054 item 3)")


# The token clause of a loop carrier (decision 0054 items 3 and 4): an `invariant` that holds `acc(loc(block_of(P), -1).tok ...)` is an
# implication whose antecedent names `P`, so a carrier never holds a token for a pointer its guard does not mention (a written site's
# `P != null ==> ...`, an owner's `held_o && ... ==> ...`, a place's `... && P != null ==> ...`).
TOK_ACC = "acc(loc(block_of("


def lint_carrier_clauses(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    for d in decls:
        if d.kind != "method" or not d.has_body:
            continue
        for n in range(d.body_line, d.end_line + 1):
            m = INV_LINE_RE.match(src.code[n - 1])
            if m is None:
                continue
            text = _nospace(m.group("rest"))
            i = text.find(TOK_ACC)
            while i >= 0:
                start = i + len("acc(loc(")
                end = _group_end(text, start + len("block_of"))
                arg = text[start + len("block_of("):end] if end >= 0 else ""
                tail = text[end + 1:] if end >= 0 else ""
                if end >= 0 and re.match(r",\(?-1\)?\)\.tok", tail):
                    depth, arrow = 0, -1
                    for k in range(len(text)):
                        c = text[k]
                        depth += c == "("
                        depth -= c == ")"
                        if depth == 0 and text.startswith("==>", k):
                            arrow = k
                            break
                    if arrow < 0 or arrow > i or arg not in text[:arrow]:
                        add(n, f"an `invariant` holds the token of `block_of({arg})` outside an implication whose antecedent names "
                               f"`{arg}`: a loop carrier is guarded by its pointer (decision 0054 items 3 and 4)")
                        break
                i = text.find(TOK_ACC, i + 1)


# Owning globals, `static` locals and address-taken owners (decision 0054 items 5, 6 and 8).  The lowering gives each such owner the
# method-level ghost `held_<o>` (for a global or `static` local `<o>` is its block's name, `GB_<name>`; an owning local also has `tv_<o>`), and `main`'s leak
# row exempts the block of an owning global by a term `held_GB_x && t == loc(block_of(loc(GB_x(), 0).v_ptr_T), -1)`.  The lint reads only
# the Viper, so:
#   * `program_start` carries one tag line `// cerastes:owning-global GB_<name>` per owning global (right after its head, before its
#     first clause), each naming a declared `unique function GB_<name>()` once; `held_GB_x` exists only for a tagged block;
#   * a `held_<o>` is assigned only: `false`; `<x> != null` (the owner received a value; its value row precedes); `held_<o> && ((<x> != null)
#     && (perm(tok(<x>)) > none))` (after a call or a free, local); `(held_<o> && (perm(<C>) > none)) && ((<C> != null) && (perm(tok(<C>))
#     > none))` (after a call or a free, global or address-taken owner, `<C>` the owner's own cell `loc(GB_x(), 0).v_ptr_T` or
#     `<o>.v_ptr_T`); `held_<o> || (((<x> != null) && (offset_of(<x>) == 0)) && (perm(tok(<x>)) == write))` (an address-taken owner that
#     received a block): it never becomes `true` without a read of the token's permission;
#   * a `tv_<o>` (an owning local's allocator ghost) is assigned only `(<x> != null) ? tok(<x>) : tv_<o>`;
#   * an exemption term of a `forperm` row with a `held_GB_x` guard names exactly `t == loc(block_of(loc(GB_x(), 0).v_ptr_T), -1)`;
#   * a comparison of `held_GB_x` with a value is `held_GB_x == (loc(GB_x(), 0).v_ptr_T != null)`, as the loop invariant and its rows say.
OWNING_GLOBAL_TAG_RE = re.compile(r"//\s*cerastes:owning-global\b[ \t]*(?P<name>\S+)?")
GB_CALL_RE = re.compile(r"(?<![\w$'])(?P<name>GB_[A-Za-z0-9_$']+)\s*\(\s*\)")
GB_GHOST_RE = re.compile(r"(?<![\w$'.])held_(?P<name>GB_[A-Za-z0-9_$']+)(?![\w$'])")
OWNER_ASSIGN_RE = re.compile(r"(?<![\w$'.])(?P<g>(?:held|tv)_[\w$']+)\s*:=")
_ID = r"[A-Za-z_$][\w$']*"
_SLOT = r"loc\(block_of\((?P<{n}>.+?)\),\(?-1\)?\)\.tok"


def owner_assignment_ok(g: str, rhs: str) -> bool:
    r = _nospace(rhs)
    e = re.escape(g)
    if g.startswith("tv_"):
        return re.fullmatch(rf"\((?P<x>{_ID})!=null\)\?loc\(block_of\((?P=x)\),\(?-1\)?\)\.tok:{e}", r) is not None
    if r == "false" or re.fullmatch(rf"{_ID}!=null", r):
        return True
    if re.fullmatch(rf"{e}&&\(\((?P<x>{_ID})!=null\)&&\(perm\(loc\(block_of\((?P=x)\),\(?-1\)?\)\.tok\)>none\)\)", r):
        return True
    if re.fullmatch(rf"{e}\|\|\(\(\((?P<x>{_ID})!=null\)&&\(offset_of\((?P=x)\)==0\)\)&&\(perm\(loc\(block_of\((?P=x)\),\(?-1\)?\)\.tok\)==write\)\)", r):
        return True
    m = re.fullmatch(rf"\({e}&&\(perm\((?P<c>.+?)\)>none\)\)&&\(\((?P=c)!=null\)&&\(perm\(loc\(block_of\((?P=c)\),\(?-1\)?\)\.tok\)>none\)\)", r)
    if m:
        c = m.group("c")
        if gm := re.fullmatch(r"loc\((?P<gb>GB_[\w$']+)\(\),0\)\.v_ptr_[\w$']+", c):
            return g == "held_" + gm.group("gb")
        return re.fullmatch(rf"{_ID}\.v_ptr_[\w$']+", c) is not None and c.split(".")[0] == g[len("held_"):]
    return False


def lint_owning_global_tags(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> None:
    declared: dict[str, int] = {}
    for code in src.code:
        if m := GB_DECL_RE.match(code):
            declared[m.group("name")] = declared.get(m.group("name"), 0) + 1
    start = next((d for d in decls if d.kind == "method" and d.name == PROGRAM_START), None)
    first_clause = last_clause = None   # `program_start` is a bodyless trusted method: its head, then its clauses
    if start is not None:
        clauses = [n for n in range(start.line + 1, start.end_line + 1) if re.match(r"^\s*(requires|ensures|decreases)\b", src.code[n - 1])]
        first_clause, last_clause = (clauses[0], clauses[-1]) if clauses else (None, start.line)
    tagged: set[str] = set()
    for lineno, comment in enumerate(src.comments, start=1):
        m = OWNING_GLOBAL_TAG_RE.search(comment) if comment else None
        if m is None:
            continue
        name = m.group("name")
        if start is None or not start.line <= lineno <= last_clause:
            add(lineno, "`// cerastes:owning-global` stands only inside `method program_start`: it names an owning global whose block "
                        "`main`'s leak row exempts (decision 0054 item 8)")
        elif name is None or declared.get(name, 0) != 1:
            add(lineno, f"`// cerastes:owning-global {name or ''}` names no single `unique function GB_...` declared in this file "
                        "(decision 0054 item 8)")
        elif name in tagged:
            add(lineno, f"`{name}` is tagged `// cerastes:owning-global` twice (decision 0054 item 8)")
        elif first_clause is not None and lineno > first_clause:
            add(lineno, "`// cerastes:owning-global` stands right after the head of `method program_start`, before its first clause "
                        "(decision 0054 item 8)")
        else:
            tagged.add(name)
    for lineno, code in enumerate(src.code, start=1):
        for gm in sorted({m.group("name") for m in GB_GHOST_RE.finditer(code)}):
            if gm not in tagged:
                add(lineno, f"the ownership ghost `held_{gm}` of `{gm}` in a program whose `program_start` does not tag "
                            f"`// cerastes:owning-global {gm}`: only an owning global has one (decision 0054 item 5)")
                break
        for a in OWNER_ASSIGN_RE.finditer(code):
            rhs = _statement_rhs(code, a.end())
            if not owner_assignment_ok(a.group("g"), rhs):
                add(lineno, f"`{a.group('g')}` is assigned `{rhs}`: an owner's ghost is assigned `false`, `<x> != null`, `held && ...` "
                            "(the update after a call or a free, which reads the token's permission), `held || ...` (a block received "
                            "through an address) or, for `tv_`, the conditional read of the token (decision 0054 items 5 and 6)")
        for gm in sorted({m.group("name") for m in re.finditer(r"(?<![\w$'.])held_(?P<name>GB_[\w$']+)\s*==(?!>)", code)}):
            n = _nospace(code)
            cell = rf"loc\({re.escape(gm)}\(\),0\)\.v_ptr_[\w$']+"
            if not (re.fullmatch(rf"invariantheld_{re.escape(gm)}==\({cell}!=null\)", n)
                    or re.fullmatch(rf"if\(!\(held_{re.escape(gm)}==\({cell}!=null\)\)\)\{{", n)):
                add(lineno, f"`held_{gm}` is compared with something but `(loc({gm}(), 0).v_ptr_T != null)`, in the loop invariant "
                            "or its row (decision 0054 item 5)")
    for lineno, code in enumerate(src.code, start=1):
        m = FORPERM_LINE_RE.match(code)
        if m is None:
            continue
        body = _strip_wrap(m.group("body").strip())
        for disjunct in _flatten_or(body):
            conjuncts = [_strip_wrap(c) for c in _split_top(_strip_wrap(disjunct), "&&")]
            helds = [c for c in conjuncts if re.fullmatch(r"held_[\w$']*", c)]
            if not helds:
                continue
            for c in conjuncts:
                t = re.match(r"^t\s*==\s*(?P<loc>loc\(.*\))$", c, re.S)
                if t is None:
                    continue
                gbs = {g.group("name") for g in GB_CALL_RE.finditer(t.group("loc"))}
                for g in sorted(gbs - tagged):
                    add(lineno, f"an exemption term `held_... && t == loc(block_of(...{g}()...), -1)` names the cell of `{g}`, which "
                                f"`program_start` does not tag `// cerastes:owning-global {g}`: only an owning global's block is exempt "
                                "at `main`'s end (decision 0054 item 8)")
                for h in helds:
                    gm = GB_GHOST_RE.fullmatch(h)
                    if gm is None:
                        continue
                    want = rf"loc\(block_of\(loc\({re.escape(gm.group('name'))}\(\),0\)\.v_ptr_[\w$']+\),\(?-1\)?\)"
                    if not re.fullmatch(want, _nospace(t.group("loc"))):
                        add(lineno, f"the exemption term guarded by `{h}` is exactly `t == loc(block_of(loc({gm.group('name')}(), 0).v_ptr_T), "
                                    "-1)`: a held owning global exempts its own block and no other (decision 0054 item 8)")


# The heap's free token (decision 0047 items 6, 7 and 10).  `field tok: Bool` is the ghost token of a heap block at slot -1: held at
# `write` and true by whoever owns the allocation.  It is created by a view's postcondition (a bodyless trusted method) or a
# function's `ensures`, consumed by a call whose `requires` has the block site (`free`), and read by the leak rows `forperm`.  Nothing
# in a method body may make or take one, so the lint refuses every statement form that could: an `exhale`, an assignment, an `inhale`,
# a condition.  The text of the view methods themselves is pinned by `lint_views` (above).
TOK_WORD_RE = re.compile(r"(?<![\w$'])tok(?![\w$'])")
FORPERM_WORD_RE = re.compile(r"(?<![\w$'])forperm(?![\w$'])")
FORPERM_LINE_RE = re.compile(r"^\s*assert\s+forperm\s+(?P<v>[A-Za-z_$][\w$']*)\s*:\s*Ref\s*\[\s*(?P=v)\.tok\s*\]\s*::\s*(?P<body>\S.*)$")
TOKEN_SLOT_RE = re.compile(r"loc\(\s*block_of\(.*\)\s*,\s*\(?\s*-\s*1\s*\)?\s*\)$", re.S)
TOK_FIELD_DECL = "field tok: Bool"
TOK_STATEMENT_RE = re.compile(r"^\s*(?P<kw>assert|requires|ensures|invariant)\b")
# The loop value snapshot of decision 0050 item 5: `ktv_<n> := loc(block_of(...), -1).tok`, taken when the loop's `kt_<n> > none`
# (one line `if (kt_<n> > none) { ktv_<n> := ... }`, or the assignment alone on a line right below `if (kt_<n> > none) {`).  It is the
# only read of a token's value that is stored: a local `Bool` that the loop's invariant and back-edge row compare with the cell.
KTV_ASSIGN = r"(?P<v>ktv_[\w$']*)\s*:=\s*(?P<slot>loc\(.*\))\.tok"
KTV_IF = r"if\s*\(\s*(?P<g>kt(?:_[\w$']*)?)\s*>\s*none\s*\)\s*\{"
KTV_ONE_LINE_RE = re.compile(rf"^\s*{KTV_IF}\s*{KTV_ASSIGN}\s*\}}\s*$", re.S)
KTV_ALONE_RE = re.compile(rf"^\s*{KTV_ASSIGN}\s*$", re.S)
KTV_IF_LINE_RE = re.compile(rf"^\s*{KTV_IF}\s*$")
# The ending rows of a heap place's back edge (decision 0047 item 10 as amended at M4.5d2v): `if (!(<row>)) {`, then `assert false`, then
# `}` stands for `assert <row>` whose failure ends the path (so the rows after it assume it).  The row is read like an assert's
# conjuncts (a `tok` in it only as `loc(block_of(...), -1).tok`); an `if` with anything else in its body, or a condition that is not
# the negation of one parenthesised row, is an ordinary `if` and may not read `tok`.
ENDING_IF_RE = re.compile(r"^\s*if\s*\(\s*!\s*(?P<row>\(.*\))\s*\)\s*\{\s*$", re.S)


# The owner's token value (decision 0052 item 3, 0050 item 5): `tv_<v> := (<x> != null) ? loc(block_of(<x>), -1).tok : tv_<v>`, taken
# at the initialization or assignment of an owning local right after its value row (which checks the whole block is held where `<x>`
# is not null).  It is the second read of a token's value that is stored: a method-level `Bool` that only the owner's ending rows and
# the loop's place compare with the cell.  Only this exact shape: the same ghost on both sides, the same pointer in the guard and the slot.
TV_ASSIGN_RE = re.compile(r"^\s*(?P<v>tv_[\w$']*)\s*:=\s*\(\s*(?P<x>[A-Za-z_$][\w$']*)\s*!=\s*null\s*\)\s*\?\s*"
                          r"loc\(\s*block_of\(\s*(?P=x)\s*\)\s*,\s*\(?\s*-\s*1\s*\)?\s*\)\.tok\s*:\s*(?P=v)\s*$")


def is_ending_row(src: Source, lineno: int) -> bool:
    """The line `lineno` (1-based) is `if (!(<row>)) {` and the next two lines are exactly `assert false` and `}`."""
    m = ENDING_IF_RE.match(src.code[lineno - 1])
    if m is None or _group_end(m.group("row"), 0) != len(m.group("row")) - 1:
        return False
    rest = [c.strip() for c in src.code[lineno:lineno + 2]]
    return rest == ["assert false", "}"]


def strip_perm_calls(text: str) -> str:
    """`text` without its `perm(...)` calls (balanced), for a snapshot's right-hand side."""
    out = []
    i = 0
    while i < len(text):
        m = PERM_CALL_RE.match(text, i)
        if m:
            depth = 1
            j = m.end()
            while j < len(text) and depth:
                depth += (text[j] == "(") - (text[j] == ")")
                j += 1
            out.append(" ")
            i = j
            continue
        out.append(text[i])
        i += 1
    return "".join(out)


# The typed views of the allocation functions (decision 0047 items 2, 4 and 7).  A view is a bodyless trusted method, so its contract is
# an assumption (invariant 4): it is pinned as `alloc_<t>` is, by rebuilding the expected clauses from the type's leaves, which are read
# from the text itself (a leaf is an `acc(C.v_t)` and `acc(C.d_t)` pair in member order; the member blocks and slot facts of a
# structure are checked with `struct_wf_lens_problem`).  Whatever is added to, missing from or reordered in what the rebuilt text says
# (a `true`-ed flag on a `malloc` cell, `ensures false`, a second token, a wider range) makes the two texts differ.
# Record 0050 item 9 (M4.5d2v): a view of vlibc's own `vlibc_malloc`, `vlibc_calloc` or `vlibc_free` has the same text with the
# token's value `false` (`!loc(block_of(...), -1).tok`): the block belongs to vlibc's heap (item 3), not the platform's.
VIEW_TAG_RE = re.compile(r"vlibc:\s+(?P<routine>(?:vlibc_)?(?:malloc|calloc|free))\b[^\n]*;\s*typed view:")
VIEW_NAME_RE = re.compile(r"^f_(?P<routine>(?:vlibc_)?(?:malloc|calloc|free))__[0-9]+$")
VIEW_INT_RANGES = {
    "schar": (-128, 127), "uchar": (0, 255), "short": (-32768, 32767), "ushort": (0, 65535),
    "int": (-2**31, 2**31 - 1), "uint": (0, 2**32 - 1), "long": (-2**63, 2**63 - 1), "ulong": (0, 2**64 - 1),
}
VIEW_STANDARD_RANGES = {(-(2**(n - 1)), 2**(n - 1) - 1) for n in (8, 16, 32, 64, 128)} | {(0, 2**n - 1) for n in (8, 16, 32, 64, 128)}


def _group_end(text: str, start: int) -> int:
    """The index of the `)` matching the `(` at `start`, or -1."""
    depth = 0
    for i in range(start, len(text)):
        if text[i] == "(":
            depth += 1
        elif text[i] == ")":
            depth -= 1
            if depth == 0:
                return i
    return -1


def _strip_wrap(text: str) -> str:
    text = text.strip()
    while text.startswith("(") and _group_end(text, 0) == len(text) - 1:
        text = text[1:-1].strip()
    return text


def _split_top(text: str, op: str) -> list[str]:
    parts, depth, start, i = [], 0, 0, 0
    while i < len(text):
        c = text[i]
        if c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
        elif depth == 0 and text.startswith(op, i):
            parts.append(text[start:i])
            start = i + len(op)
            i += len(op)
            continue
        i += 1
    parts.append(text[start:])
    return parts


def _flatten_and(text: str) -> list[str]:
    """The conjuncts of a fully parenthesized `&&` chain, as raw text (an operand with `||` or `==>` stays whole)."""
    text = _strip_wrap(text)
    parts = _split_top(text, "&&")
    if len(parts) == 1:
        return [text]
    if len(_split_top(text, "||")) > 1 or len(_split_top(text, "==>")) > 1:
        raise ValueError("an unparenthesized mix of `&&` with `||` or `==>`")
    out: list[str] = []
    for part in parts:
        out += _flatten_and(part)
    return out


def _view_wf(x: str) -> str:
    return f"({x} == null) || ((({x} == loc(block_of({x}), offset_of({x}))) && (0 <= offset_of({x}))) && (offset_of({x}) <= len(block_of({x}))))"


def _chain_cell(chain: tuple[int, ...], root: str) -> str:
    cell = root
    for k in chain:
        cell = f"loc(member({cell}, {k}), 0)"
    return cell


def _lens_members(parts: list[str], root: str) -> tuple[list[tuple[int, ...]], str | None]:
    why = struct_wf_lens_problem(parts, root)
    if why is not None:
        return [], why
    keys = []
    for part in parts:
        m = re.fullmatch(r"len\((?P<m>.*)\)==1", part)
        if m:
            keys.append(parse_struct_member(m.group("m"), root))
    return keys, None


def _leaf_kind(t: str) -> str:
    if t == "bool":
        return "bool"
    if t.startswith("ptr_"):
        return "ptr"
    if t.startswith("enum_"):
        return "enum"
    return "int"


def view_problem(src: Source, d: Decl, routine: str, declared: dict[str, set[str]], has_globals: bool = False,
                 global_facts: list[str] | None = None, vlibc: bool = False) -> str | None:
    """The reason why the view method `d` is not the text decision 0047 gives its allocation type, or None.  In a program with
    globals every method takes the global facts as its first `requires` (decision 0041 item 4), views included: exactly one such
    leading `requires`, equal to the facts of the program (`global_facts`, from `program_start`), and no other.  `routine` is the
    base name (`malloc`, `calloc`, `free`); `vlibc` says the view is of the `vlibc_` routine, whose token conjunct is `!tok`
    (decision 0050 items 3 and 9)."""
    if d.has_body:
        return "it has a body"
    shown = ("vlibc_" if vlibc else "") + routine
    name_m = VIEW_NAME_RE.match(d.name)
    if name_m is None or name_m.group("routine") != shown:
        return f"its name is not `f_{shown}__<n>`"
    lines = [" ".join(src.code[n - 1].split()) for n in range(d.line, d.end_line + 1) if src.code[n - 1].strip()]
    root = "p_ptr__0" if routine == "free" else "res"
    keyword = "requires" if routine == "free" else "ensures"
    header = f"method {d.name}(p_ptr__0: Ref)" if routine == "free" else f"method {d.name}() returns (res: Ref)"
    if has_globals and len(lines) == 5 and lines[1].startswith("requires "):
        facts_line = lines[1][len("requires "):]
        if global_facts is not None and sorted(norm_viper(facts_line).split("&&")) != global_facts:
            return "its leading `requires` is not the global facts of the program (`len(GB_x()) == 1 && lvloc(GB_x()) == loc(GB_x(), -2)` for each global, as every method takes them)"
        lines = lines[:1] + lines[2:]
    elif has_globals:
        return "a view in a program with globals takes exactly one leading `requires`, the global facts of the program (decision 0041 item 4)"
    if len(lines) != 4 or lines[0] != header or lines[3] != "decreases _":
        return f"its header, its two `{keyword}` clauses and `decreases _` are not the emitter's"
    if not (lines[1].startswith(keyword + " ") and lines[2].startswith(keyword + " ")):
        return f"its two clauses are not both `{keyword}`"
    first, second = lines[1][len(keyword) + 1:], lines[2][len(keyword) + 1:]
    try:
        implies = _split_top(_strip_wrap(second), "==>")
        if len(implies) != 2 or norm_viper(implies[0]) != norm_viper(f"!({root} == null)"):
            return f"its second clause is not `!({root} == null) ==> <the block>`"
        raw = _flatten_and(implies[1])
    except ValueError as e:
        return str(e)
    atoms = [norm_viper(a) for a in raw]
    token_value = ("!" if vlibc else "") + f"loc(block_of({root}), (-1)).tok"
    prefix = [f"{root} != null", _view_wf(root), f"offset_of({root}) == 0", f"acc(loc(block_of({root}), (-1)).tok)",
              token_value, f"lvloc(block_of({root})) == loc(block_of({root}), (-2))",
              f"acc(lvloc(block_of({root})).lv)", f"len(block_of({root})) == 1"]
    if atoms[:len(prefix)] != [norm_viper(a) for a in prefix]:
        return ("the block part does not begin with the pointer, its well-formedness, offset 0, the token and its value "
                f"({'`!tok`: a block of vlibc\'s heap' if vlibc else '`tok`: a block of the platform\'s heap'}), the liveness equation "
                "and cell and `len == 1`")
    i = len(prefix)
    lens: list[str] = []
    while i < len(atoms) and not atoms[i].startswith("acc("):
        lens.append(atoms[i])
        i += 1
    members: list[tuple[int, ...]] = []
    if lens:
        members, why = _lens_members(lens, root)
        if why is not None:
            return f"the structure's member blocks: {why}"
    structs = {k for k in members if any(o[:-1] == k for o in members)}
    leaf_chains = sorted(k for k in members if k not in structs) if lens else [()]
    # the leaves, in the text: `acc(C.v_t)`, `acc(C.d_t)` and the facts under the flag
    leaves: list[tuple[tuple[int, ...], str, str]] = []  # chain, suffix, the rebuilt facts
    for chain in leaf_chains:
        cell = _chain_cell(chain, root)
        m = re.fullmatch(r"acc\((?P<c>.*)\.v_(?P<t>[A-Za-z0-9_$']+)\)", atoms[i]) if i < len(atoms) else None
        if m is None or m.group("c") != norm_viper(cell):
            return f"leaf {chain or 'res'}: `acc({cell}.v_<t>)` is expected here"
        t = m.group("t")
        if t not in declared["v"] or t not in declared["d"]:
            return f"the field suffix `{t}` is not declared (`field v_{t}` and `field d_{t}`)"
        if i + 1 >= len(atoms) or atoms[i + 1] != norm_viper(f"acc({cell}.d_{t})"):
            return f"leaf {chain or 'res'}: `acc({cell}.d_{t})` is expected after its value permission"
        i += 2
        kind = _leaf_kind(t)
        value, flag = f"{cell}.v_{t}", f"{cell}.d_{t}"
        if kind == "bool":
            facts = ""
        elif kind == "ptr":
            fact_raw = raw[i] if i < len(raw) else ""
            rhs = _split_top(_strip_wrap(fact_raw), "==>")
            if len(rhs) != 2 or norm_viper(rhs[0]) != norm_viper(flag):
                return f"leaf {chain or 'res'}: the pointer's facts are not `{flag} ==> ...`"
            sub = _flatten_and(rhs[1])
            if len(sub) == 1:
                facts = f"{flag} ==> ({_view_wf(value)})"
            elif len(sub) == 2:
                g = _split_top(_strip_wrap(sub[1]), "==>")
                if len(g) != 2:
                    return f"leaf {chain or 'res'}: the pointee's member blocks are not `guard ==> lens`"
                pointee = [norm_viper(a) for a in _flatten_and(g[1])]
                _members, why = _lens_members(pointee, norm_viper(value))
                if why is not None:
                    return f"leaf {chain or 'res'}: the pointee's member blocks: {why}"
                facts = (f"{flag} ==> (({_view_wf(value)}) && (({value} != null && offset_of({value}) < len(block_of({value}))) "
                         f"==> ({' && '.join(pointee)})))")
            else:
                return f"leaf {chain or 'res'}: the pointer's facts are not its well-formedness and its pointee's member blocks"
            i += 1
        else:
            fact = atoms[i] if i < len(atoms) else ""
            m2 = re.fullmatch(re.escape(norm_viper(flag)) + r"==>(?P<lo>-?[0-9]+)<=" + re.escape(norm_viper(value)) + "&&"
                              + re.escape(norm_viper(value)) + r"<=(?P<hi>-?[0-9]+)", fact)
            if m2 is None:
                return f"leaf {chain or 'res'}: the facts under its flag are not `{flag} ==> lo <= value && value <= hi`"
            lo, hi = int(m2.group("lo")), int(m2.group("hi"))
            if kind == "enum":
                ok = lo <= 0 <= hi
            elif t in VIEW_INT_RANGES:
                ok = (lo, hi) == VIEW_INT_RANGES[t]
            else:
                ok = (lo, hi) in VIEW_STANDARD_RANGES
            if not ok:
                return f"leaf {chain or 'res'}: the range [{lo}, {hi}] is not the range of its field `{t}` (decision 0018)"
            facts = f"{flag} ==> (({lo}) <= {value} && {value} <= {hi})"
            i += 1
        leaves.append((chain, t, facts))
    # rebuild the whole text
    body = [*prefix, *lens]
    for chain, t, facts in leaves:
        cell = _chain_cell(chain, root)
        body += [f"acc({cell}.v_{t})", f"acc({cell}.d_{t})"] + ([facts] if facts else [])
    if routine == "calloc":
        zero = {"bool": "false", "ptr": "null"}
        body += [f"{_chain_cell(c, root)}.d_{t}" for c, t, _ in leaves]
        body += [f"{_chain_cell(c, root)}.v_{t} == {zero.get(_leaf_kind(t), '0')}" for c, t, _ in leaves]
    expected_second = f"!({root} == null) ==> ({' && '.join(body)})"
    if norm_viper(second) != norm_viper(expected_second):
        return ("the block part is not exactly what its type's leaves give" +
                (" (a `calloc` view adds the flags and then the zero values of every leaf, and nothing else)" if routine == "calloc"
                 else " (a `malloc` view states no flag and no value, a `free` view only what `owns_block` takes)"))
    expected_first = _view_wf(root)
    if lens:
        expected_first += f" && (({root} != null && offset_of({root}) < len(block_of({root}))) ==> ({' && '.join(lens)}))"
    if norm_viper(first) != norm_viper(expected_first):
        return "its first clause is not the pointer's well-formedness (and the member block lengths of a structure under `offset_of < len`)"
    return None


TYPED_VIEW_WORD_RE = re.compile(r";\s*typed view:")
VIEW_ROUTINE_TAG_RE = re.compile(r"vlibc:\s+(?:vlibc_)?(?:malloc|calloc|free)\b")


def lint_views(src: Source, decls: list[Decl], add: Callable[[int, str], None], has_globals: bool = False,
               global_facts: list[str] | None = None) -> None:
    declared = {"v": set(), "d": set()}
    for line in src.code:
        m = FIELD_ANY_RE.match(line)
        if m:
            declared[m.group("kind")].add(m.group("suffix"))
    for d in decls:
        if d.kind != "method" or d.has_body and not VIEW_NAME_RE.match(d.name):
            continue
        tag = trusted_tag(src, d.line)
        if tag is None:
            continue
        vt = VIEW_TAG_RE.search(tag.group(0))
        if vt is None:
            if not d.has_body and VIEW_ROUTINE_TAG_RE.search(tag.group(0)):
                add(d.line, f"`method {d.name}` is tagged as vlibc's malloc, calloc or free but not as a typed view "
                            "(`; typed view:` in the reason): the allocation functions, and the `vlibc_` ones of decision 0050 item 3, are "
                            "trusted only as the views of decision 0047 items 2 and 4, whose text is pinned")
            elif not d.has_body and TYPED_VIEW_WORD_RE.search(tag.group(0)):
                # decision 0050 item 9: a method whose tag claims to be a typed view that no pattern above recognises was skipped
                add(d.line, f"`method {d.name}` is tagged `; typed view:` but is not a view of `malloc`, `calloc`, `free` or their "
                            "`vlibc_` forms (decision 0047 items 2 and 4, decision 0050 item 3): a bodyless method may not claim to "
                            "be a pinned view without being one")
            continue
        routine = vt.group("routine")
        vlibc = routine.startswith("vlibc_")
        why = view_problem(src, d, routine[len("vlibc_"):] if vlibc else routine, declared, has_globals, global_facts, vlibc)
        if why is not None:
            add(d.line, f"`method {d.name}` is not the trusted typed view of decision 0047 items 2 and 4: {why}")


def _flatten_or(text: str) -> list[str]:
    """The disjuncts of a possibly nested `||` chain (`((A || B) || C)`, as the lowering nests it, or `A || (B || C)`), as raw text."""
    text = _strip_wrap(text.strip())
    parts = _split_top(text, "||")
    if len(parts) == 1:
        return [text]
    out: list[str] = []
    for part in parts:
        out += _flatten_or(part)
    return out


def forperm_exemption_problem(body: str) -> str | None:
    """Decision 0054 item 8: the exemption of a `forperm` leak row is `false` or a disjunction of terms, each a conjunction of guards
    with exactly one `t == loc(block_of(<pointer>), -1)`; `t` appears nowhere else, so an exemption only names the token of a block
    by its location (a carried token), never `true`, `t != ...` or a negated term."""
    body = _strip_wrap(body.strip())
    if body == "false":
        return None
    for disjunct in _flatten_or(body):
        conjuncts = [_strip_wrap(c) for c in _split_top(_strip_wrap(disjunct), "&&")]
        terms = [c for c in conjuncts if (m := re.match(r"^t\s*==\s*(?P<loc>loc\(.*\))$", c, re.S)) and TOKEN_SLOT_RE.search(m.group("loc"))
                 and _group_end(m.group("loc"), 3) == len(m.group("loc")) - 1]
        if len(terms) != 1:
            return ("an exemption term of a `forperm` leak row is `<guards> && t == loc(block_of(<pointer>), -1)` with exactly one "
                    "such comparison: it names a carried token by its location (decision 0054 item 8)")
        for c in conjuncts:
            if c not in terms and re.search(r"(?<![\w$'.])t(?![\w$'])", c):
                return ("`t` appears in a `forperm` exemption outside the comparison `t == loc(block_of(<pointer>), -1)`: a guard "
                        "that tests the token itself would exempt more than a carried block (decision 0054 item 8)")
    return None


def lint_heap_token(src: Source, decls: list[Decl], add: Callable[[int, str], None]) -> set[int]:
    """The rules of the free token (see above).  Returns the lines of the `forperm` assertions, whose map entries the caller
    checks against the category `leak`."""
    forperm_lines: set[int] = set()
    decl_lines = {d.line for d in decls if d.kind == "field" and d.name == "tok"}
    used: list[int] = []
    for lineno, code in enumerate(src.code, start=1):
        if lineno in decl_lines:
            if " ".join(code.split()) != TOK_FIELD_DECL:
                add(lineno, f"`field tok` must be exactly `{TOK_FIELD_DECL}` (the free token of a heap block, decision 0047)")
            continue
        has_forperm = bool(FORPERM_WORD_RE.search(code))
        tok_hits = list(TOK_WORD_RE.finditer(code))
        if not has_forperm and not tok_hits:
            continue
        used.append(lineno)
        if has_forperm:
            m = FORPERM_LINE_RE.match(code)
            if m is None:
                add(lineno, "`forperm` is only the statement `assert forperm t: Ref [t.tok] :: <exemption>` of a `leak` row "
                            "(decision 0047 item 7)")
                continue
            if TOK_WORD_RE.search(m.group("body")):
                add(lineno, "the exemption of a `forperm` leak row does not mention `tok`: it names the places the method "
                            "hands back (decision 0047 item 7)")
            elif (why := forperm_exemption_problem(m.group("body"))) is not None:
                add(lineno, why)
            forperm_lines.add(lineno)
            continue
        for h in tok_hits:
            before = code[:h.start()]
            if not before.endswith(".") or not TOKEN_SLOT_RE.search(before[:-1]):
                add(lineno, "`tok` outside the token slot of a block: it is only `loc(block_of(...), -1).tok` (decision 0047 "
                            "item 4)")
                break
        else:
            stripped = code.strip()
            if re.match(r"^exhale\b", stripped):
                add(lineno, "`exhale` of a `tok` place: a token is consumed only by a call whose `requires` has the block "
                            "site (decision 0047 item 6)")
            elif is_ending_row(src, lineno):
                pass  # an ending back-edge row: the token rules above have read its `tok`s as an assert's
            elif (km := KTV_ONE_LINE_RE.match(code) or KTV_ALONE_RE.match(code)) is not None:
                above = next((c for c in reversed(src.code[:lineno - 1]) if c.strip()), "")
                guard = km.group("g") if "g" in km.groupdict() else None
                if guard is None and (gm := KTV_IF_LINE_RE.match(above)) is not None:
                    guard = gm.group("g")
                # (the slot itself is the loop above's check: any `.tok` not at `loc(block_of(...), -1)` is reported there)
                if guard is None:
                    add(lineno, f"`{km.group('v')} := ....tok` outside `if (kt_<n> > none) {{ ... }}`: the loop value snapshot is taken "
                                "only where the loop holds the token (decision 0050 item 5)")
                elif not guard.startswith("kt_") or km.group("v") != "ktv" + guard[2:]:
                    add(lineno, f"`{km.group('v')}` is taken under the guard of `{guard}`: the snapshot of a place's value is taken where "
                                "that same place's `kt_<n>` is held, so its name is `ktv_<n>` for the guard `kt_<n> > none` "
                                "(decision 0050 item 5)")
            elif TV_ASSIGN_RE.match(code):
                pass  # an owner's token value (decision 0052 item 3): the one conditional read of a token that is stored
            elif re.search(r"\.tok\s*:=", code):
                add(lineno, "assignment to `tok`: a token is created only by a postcondition (decision 0047 item 2)")
            elif TOK_STATEMENT_RE.match(code):
                pass  # a conjunct of a view, a block site, an explicit row or a loop place
            elif re.match(r"^\s*(?:var\s+)?[A-Za-z_$][\w$']*\s*(?::\s*Perm\s*)?:=", code):
                rhs = code.split(":=", 1)[1]
                if TOK_WORD_RE.search(strip_perm_calls(rhs)):
                    add(lineno, "`tok` in the right-hand side of an assignment beyond the `perm(...)` snapshot of a loop "
                                "place's amount (decision 0047 item 10)")
            else:
                add(lineno, f"`tok` in `{stripped.split()[0] if stripped else ''}`: it is only a conjunct of a `requires`, "
                            "`ensures`, `invariant` or `assert`, or inside the `perm(...)` of a snapshot (decision 0047 items "
                            "4, 7 and 10)")
    if decl_lines and not used:
        add(min(decl_lines), "`field tok: Bool` in a program that never uses a token: only a heap program (one with a view "
                             "call or a block site) declares it (decision 0047 item 7)")
    if used and not decl_lines:
        add(used[0], "`tok` is used but `field tok: Bool` is not declared")
    return forperm_lines



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
    has_globals, global_facts = lint_globals(src, decls, add)
    lint_ghost_amounts(src, decls, add)
    lint_perm_locals(src, decls, add)
    lint_owner_loop_ghosts(src, decls, add)
    lint_loop_entry_ghosts(src, decls, add)
    lint_carrier_clauses(src, decls, add)
    lint_owning_global_tags(src, decls, add)
    lint_ghost_refs(src, decls, add)
    forperm_lines = lint_heap_token(src, decls, add)
    lint_views(src, decls, add, has_globals, global_facts)

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
    for lineno in sorted(forperm_lines):
        for e in by_line.get(lineno, []):
            if e.category != "leak":
                problems.append(f"{map_name or name + MAP_SUFFIX}:{e.map_line}: the entry for {name} line {lineno}, a `forperm` "
                                f"assertion, has the category `{e.category}`: `forperm` is only a `leak` row (decision 0047 item 7)")
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
# A global's cell is `loc(GB_<name>(), 0)` (decision 0041 item 4): the same rule, the receiver is that term.
GLOBAL_CELL_RE = r"loc\(\s*GB_[\w$']+\(\)\s*,\s*0\s*\)"
FIELD_FLAG_RE = re.compile(r"^\s*(?P<recv>[A-Za-z_$][\w$']*|" + GLOBAL_CELL_RE + r")\.d_(?P<name>[A-Za-z_$][\w$']*)\s*:=(?P<value>.*)$")


# The owner ghosts of an owning local (decision 0052 item 3) are assigned between its write and its flag: `held_<v> := ...` and
# `tv_<v> := ...` are writes of other variables, so a flag set after them still directly follows the write of its variable.
OWNER_GHOST_ASSIGN_RE = re.compile(r"^\s*(?:held|tv)_[\w$']*\s*:=")


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
            j = k - 1
            while j > 0 and OWNER_GHOST_ASSIGN_RE.match(lines[j][1]):  # the owner ghosts of an owning local (decision 0052 item 3)
                j -= 1
            prev = lines[j][1] if j >= 0 else ""
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
