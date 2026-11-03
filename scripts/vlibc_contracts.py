#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""vlibc's trusted contracts as cerastes reads them (decision 0046, items 6 and 8).

vlibc (third_party/vlibc, decision 0030) holds C library headers whose bodyless
declarations carry cerastes contracts.  Every such declaration is trusted, and
trust is only as good as the list of what is trusted, so this script reads the
shim directory `<vlibc>/include/vlibc` (every `*.h` in it, not recursively) and
understands exactly one shape, the one vlibc's CLAUDE.md fixes:

    // cerastes:trusted-contract vlibc: <name> <reason text>
    <declaration of <name>, ending with ';' on one or more lines, with contracts>

Fail closed: any doubt is an error `file:line: message` and exit status 1.

  * a tag line (a `//` comment alone on its line) must be followed, on the very
    next line, by a bodyless declaration of a function whose declarator name (the
    identifier directly before the first `(`, after leaving out attributes and
    `CERASTES_*(...)` groups) is <name>, with one declarator and a contract
    (`CERASTES_*` macro or `[[cerastes::...]]`);
  * a declaration in a header that has a contract and no tag directly above it is
    an error (M4.6's verified implementations will get a marker of their own);
  * a tag whose prefix is not exactly `vlibc:`, a name tagged twice (in any
    header) and a tag with no reason are errors;
  * comments and preprocessor lines are not declarations.

Subcommands:

  names --include-dir DIR --out FILE
        write a C++ fragment for the encoder, one string literal per reserved
        name, sorted (reproducible: no timestamp, and the file is rewritten only
        when its contents change, since CMake runs this at configure time).
  list --include-dir DIR
        print the canonical listing of every trusted declaration, one line each,
        sorted by name:  name | header | declaration | reason
  check --include-dir DIR --list FILE
        compare `list`'s output with the committed file (lines starting with `#`
        and blank lines in the file are ignored); a difference prints a unified
        diff and exits 1.  A pin move that changes a trusted contract fails here
        until the file changes, and that change is a golden change (invariant 1)
        that the soundness-auditor reads (record 0030 item 5).
"""

from __future__ import annotations

import argparse
import dataclasses
import difflib
import re
import sys
from pathlib import Path

TAG_LINE_RE = re.compile(r"^[ \t]*//[ \t]*cerastes:trusted-contract(?P<rest>.*)$")
TAG_BLOCK_RE = re.compile(r"^[ \t]*/\*[ \t]*cerastes:trusted-contract")
TAG_REST_RE = re.compile(r"^[ \t]+vlibc:[ \t]+(?P<name>[A-Za-z_][A-Za-z0-9_]*)(?:[ \t]+(?P<reason>\S.*?))?[ \t]*$")
TAG_PREFIX_RE = re.compile(r"^[ \t]+(?P<prefix>\S+)")
CONTRACT_RE = re.compile(r"\bCERASTES_[A-Z0-9_]+\b|\bcerastes\s*::")
LINKAGE_BLOCK_RE = re.compile(r'^extern\s*"[^"\n]*"$')
NAMESPACE_BLOCK_RE = re.compile(r"^(?:inline\s+)?namespace\b[^;(){}=]*$")
IDENT_BEFORE_RE = re.compile(r"([A-Za-z_][A-Za-z0-9_]*)\s*$")


@dataclasses.dataclass
class Tag:
    file: str
    line: int
    name: str
    reason: str


@dataclasses.dataclass
class Statement:
    start: int
    end: int
    text: str          # comments and preprocessor lines blanked, whitespace not yet collapsed
    term: str          # ';' or '}' (a function body)
    has_top_comma: bool


@dataclasses.dataclass
class Trusted:
    name: str
    header: str
    declaration: str
    reason: str
    file: str
    line: int


class Problems:
    def __init__(self) -> None:
        self.items: list[tuple[str, int, str]] = []

    def add(self, file: str, line: int, message: str) -> None:
        self.items.append((file, line, message))

    def __bool__(self) -> bool:
        return bool(self.items)

    def report(self, out) -> None:
        for file, line, message in self.items:
            print(f"{file}:{line}: {message}", file=out)


# ---------------------------------------------------------------------------
# Lexing: comments, preprocessor lines, tags


def scan(text: str, file: str, problems: Problems) -> tuple[list[str], list[tuple[int, str]]]:
    """Blank the comments and the preprocessor lines of `text`.  Returns the code
    lines (same line numbering, comments and preprocessor lines replaced by
    spaces) and the tag lines found: (line number, text after `//`)."""
    lines: list[list[str]] = [[]]
    tags: list[tuple[int, str]] = []
    i, n = 0, len(text)
    lineno = 1
    in_block = False
    preproc = False
    line_has_code = False   # a code (non-space, non-comment) character on this line

    def emit(ch: str) -> None:
        lines[-1].append(ch)

    def newline() -> None:
        nonlocal lineno, line_has_code
        lines.append([])
        lineno += 1
        line_has_code = False

    while i < n:
        c = text[i]
        if in_block:
            if text.startswith("*/", i):
                in_block = False
                emit(" ")
                emit(" ")
                i += 2
            elif c == "\n":
                newline()
                i += 1
            else:
                emit(" ")
                i += 1
            continue
        if c == "\n":
            # a preprocessor line ends at a newline not preceded by a backslash
            if preproc and not (i > 0 and text[i - 1] == "\\"):
                preproc = False
            newline()
            i += 1
            continue
        if text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
            comment = text[i + 2:j].rstrip("\r")
            if not line_has_code and not preproc:
                m = TAG_LINE_RE.match("//" + comment)
                if m:
                    tags.append((lineno, m.group("rest")))
            for _ in range(j - i):
                emit(" ")
            i = j
            continue
        if text.startswith("/*", i):
            if not line_has_code and not preproc:
                end = text.find("\n", i)
                end = n if end < 0 else end
                if TAG_BLOCK_RE.match(text[i:end]):
                    problems.add(file, lineno, "a cerastes:trusted-contract tag must be a `//` comment "
                                               "alone on its line")
            in_block = True
            emit(" ")
            emit(" ")
            i += 2
            continue
        if c == "#" and not line_has_code and not preproc:
            preproc = True
        if preproc:
            emit(" ")
            i += 1
            continue
        if c == '"' or c == "'":
            # a string or character literal stays as code
            j = i + 1
            while j < n and text[j] != c and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            end = min(j + 1, n)
            for k in range(i, end):
                if text[k] == "\n":      # an unterminated literal: do not swallow lines
                    end = k
                    break
            for k in range(i, end):
                emit(text[k])
            line_has_code = True
            i = end
            continue
        emit(c)
        if not c.isspace():
            line_has_code = True
        i += 1
    if in_block:
        problems.add(file, lineno, "unterminated /* comment")
    return ["".join(ln) for ln in lines], tags


# ---------------------------------------------------------------------------
# Statements


def split_statements(code_lines: list[str], file: str, problems: Problems) -> list[Statement]:
    out: list[Statement] = []
    cur: list[str] = []
    start = 0
    paren = brace = 0
    transparent = 0
    fn_body = False
    top_comma = False

    def reset() -> None:
        nonlocal cur, start, paren, brace, fn_body, top_comma
        cur, start, paren, brace, fn_body, top_comma = [], 0, 0, 0, False, False

    def text() -> str:
        return "".join(cur)

    for lineno, line in enumerate(code_lines, 1):
        for c in line + "\n":
            if not cur and c.isspace():
                continue
            if not cur:
                start = lineno
            if c == "{":
                if brace == 0 and paren == 0:
                    pre = re.sub(r"\s+", " ", text().strip())
                    if LINKAGE_BLOCK_RE.match(pre) or NAMESPACE_BLOCK_RE.match(pre):
                        transparent += 1
                        reset()
                        continue
                    fn_body = pre.endswith(")")
                brace += 1
                cur.append(c)
                continue
            if c == "}":
                if brace == 0:
                    if cur and text().strip():
                        problems.add(file, lineno, "`}` inside a declaration that is not closed")
                    elif transparent > 0:
                        transparent -= 1
                    else:
                        problems.add(file, lineno, "unbalanced `}`")
                    reset()
                    continue
                brace -= 1
                cur.append(c)
                if brace == 0 and paren == 0 and fn_body:
                    out.append(Statement(start, lineno, text(), "}", top_comma))
                    reset()
                continue
            if c in "([":
                paren += 1
            elif c in ")]":
                paren = max(paren - 1, 0)
            elif c == "," and paren == 0 and brace == 0:
                top_comma = True
            elif c == ";" and paren == 0 and brace == 0:
                cur.append(c)
                out.append(Statement(start, lineno, text(), ";", top_comma))
                reset()
                continue
            cur.append(c)
    if cur and text().strip():
        problems.add(file, start, "a declaration that does not end (no `;` or function body before "
                                  "the end of the file)")
    return out


def strip_groups(text: str) -> str:
    """Leave out `[[...]]`, `__attribute__((...))` and `CERASTES_*(...)` so that
    the first `(` that remains belongs to the declarator."""
    out = []
    i, n = 0, len(text)
    while i < n:
        if text.startswith("[[", i):
            depth, j = 0, i
            while j < n:
                if text[j] == "[":
                    depth += 1
                elif text[j] == "]":
                    depth -= 1
                    if depth == 0:
                        break
                j += 1
            out.append(" ")
            i = j + 1
            continue
        m = re.match(r"(?:__attribute__|CERASTES_[A-Z0-9_]+)\s*\(", text[i:])
        if m and (i == 0 or not (text[i - 1].isalnum() or text[i - 1] == "_")):
            j = i + m.end()
            depth = 1
            while j < n and depth:
                depth += (text[j] == "(") - (text[j] == ")")
                j += 1
            out.append(" ")
            i = j
            continue
        out.append(text[i])
        i += 1
    return "".join(out)


def declarator_name(text: str) -> str | None:
    t = strip_groups(text)
    k = t.find("(")
    if k < 0:
        return None
    m = IDENT_BEFORE_RE.search(t[:k])
    return m.group(1) if m else None


def collapse(text: str) -> str:
    return re.sub(r"\s+", " ", text).strip()


# ---------------------------------------------------------------------------
# One header, a directory


def read_header(path: Path, shown: str, problems: Problems) -> list[Trusted]:
    try:
        text = path.read_text(encoding="utf-8")
    except (OSError, UnicodeDecodeError) as e:
        problems.add(shown, 1, f"cannot read the header: {e}")
        return []
    text = text.replace("\r\n", "\n")
    code, raw_tags = scan(text, shown, problems)
    statements = split_statements(code, shown, problems)

    tags: list[Tag] = []
    for lineno, rest in raw_tags:
        m = TAG_REST_RE.match(rest)
        if not m:
            p = TAG_PREFIX_RE.match(rest)
            if p and p.group("prefix") != "vlibc:":
                problems.add(shown, lineno, f"the tag's prefix is `{p.group('prefix')}`, "
                                            "it must be exactly `vlibc:`")
            else:
                problems.add(shown, lineno, "malformed tag, expected "
                                            "`// cerastes:trusted-contract vlibc: <name> <reason>`")
            continue
        if not m.group("reason"):
            problems.add(shown, lineno, f"the tag of `{m.group('name')}` has no reason text")
            continue
        tags.append(Tag(shown, lineno, m.group("name"), m.group("reason")))

    by_start: dict[int, Statement] = {}
    for st in statements:
        by_start.setdefault(st.start, st)

    tagged: set[int] = set()    # ids of the statements a tag claims
    result: list[Trusted] = []
    for tag in tags:
        inside = [st for st in statements if st.start < tag.line <= st.end]
        if inside:
            problems.add(shown, tag.line, f"the tag of `{tag.name}` is inside a declaration "
                                          f"(which starts at line {inside[0].start})")
            continue
        st = by_start.get(tag.line + 1)
        if st is None:
            problems.add(shown, tag.line, f"the tag of `{tag.name}` is not directly above a declaration "
                                          "(the next line must start it)")
            continue
        tagged.add(id(st))
        if st.term != ";":
            problems.add(shown, st.start, f"the declaration of `{tag.name}` has a body: a trusted "
                                          "declaration is bodyless")
            continue
        if st.has_top_comma:
            problems.add(shown, st.start, f"the declaration under the tag of `{tag.name}` has more "
                                          "than one declarator")
            continue
        got = declarator_name(st.text)
        if got != tag.name:
            problems.add(shown, st.start, f"the tag names `{tag.name}` but the declaration below it "
                                          f"declares `{got if got else '(no function)'}`")
            continue
        if not CONTRACT_RE.search(st.text):
            problems.add(shown, st.start, f"the declaration of `{tag.name}` is tagged trusted but carries "
                                          "no contract")
            continue
        result.append(Trusted(tag.name, Path(shown).name, collapse(st.text), tag.reason, shown, tag.line))

    for st in statements:
        if id(st) not in tagged and CONTRACT_RE.search(st.text):
            problems.add(shown, st.start, "a contract without a cerastes:trusted-contract tag directly "
                                          "above its declaration (M4.6 will add verified implementations "
                                          "with a marker of their own)")
    return result


def read_dir(include_dir: Path, problems: Problems) -> list[Trusted]:
    if not include_dir.is_dir():
        problems.add(str(include_dir), 0, "not a directory")
        return []
    headers = sorted(p for p in include_dir.iterdir() if p.is_file() and p.suffix == ".h")
    if not headers:
        problems.add(str(include_dir), 0, "no *.h files: not vlibc's shim directory?")
        return []
    trusted: list[Trusted] = []
    seen: dict[str, Trusted] = {}
    for h in headers:
        for t in read_header(h, str(h), problems):
            if t.name in seen:
                first = seen[t.name]
                problems.add(t.file, t.line, f"`{t.name}` is tagged twice (first at "
                                             f"{first.file}:{first.line})")
                continue
            seen[t.name] = t
            trusted.append(t)
    return sorted(trusted, key=lambda t: t.name)


def listing(trusted: list[Trusted]) -> list[str]:
    return [f"{t.name} | {t.header} | {t.declaration} | {t.reason}" for t in trusted]


# ---------------------------------------------------------------------------
# Subcommands


def load(include_dir: str) -> list[Trusted] | None:
    problems = Problems()
    trusted = read_dir(Path(include_dir), problems)
    if problems:
        problems.report(sys.stderr)
        return None
    return trusted


def cmd_list(args) -> int:
    trusted = load(args.include_dir)
    if trusted is None:
        return 1
    for line in listing(trusted):
        print(line)
    return 0


def names_fragment(trusted: list[Trusted], include_dir: str) -> str:
    out = [f"// Generated by scripts/vlibc_contracts.py from {include_dir}; do not edit.\n"]
    out += [f'"{t.name}",\n' for t in trusted]
    return "".join(out)


def cmd_names(args) -> int:
    trusted = load(args.include_dir)
    if trusted is None:
        return 1
    content = names_fragment(trusted, args.include_dir)
    out = Path(args.out)
    try:
        if out.exists() and out.read_text(encoding="utf-8") == content:
            return 0
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(content, encoding="utf-8")
    except OSError as e:
        print(f"{out}: cannot write: {e}", file=sys.stderr)
        return 1
    return 0


def committed_lines(path: Path) -> list[str]:
    out = []
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        out.append(line.rstrip())
    return out


def cmd_check(args) -> int:
    trusted = load(args.include_dir)
    if trusted is None:
        return 1
    path = Path(args.list)
    try:
        have = committed_lines(path)
    except (OSError, UnicodeDecodeError) as e:
        print(f"{path}: cannot read the committed list: {e}", file=sys.stderr)
        return 1
    want = listing(trusted)
    if have == want:
        return 0
    diff = difflib.unified_diff(have, want, fromfile=str(path), tofile=f"{args.include_dir} (current)",
                                lineterm="")
    for line in diff:
        print(line, file=sys.stderr)
    print(f"a trusted vlibc contract changed: review it (record 0030 item 5: the soundness-auditor "
          f"reads every trusted contract diff) and update {path}", file=sys.stderr)
    return 1


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description="vlibc's trusted contracts (decision 0046)")
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("names", help="write the encoder's reserved-name fragment")
    p.add_argument("--include-dir", required=True)
    p.add_argument("--out", required=True)
    p.set_defaults(fn=cmd_names)
    p = sub.add_parser("list", help="print the canonical listing of the trusted declarations")
    p.add_argument("--include-dir", required=True)
    p.set_defaults(fn=cmd_list)
    p = sub.add_parser("check", help="compare the listing with the committed file")
    p.add_argument("--include-dir", required=True)
    p.add_argument("--list", required=True)
    p.set_defaults(fn=cmd_check)
    args = ap.parse_args(argv)
    return args.fn(args)


if __name__ == "__main__":
    sys.exit(main())
