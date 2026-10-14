#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Lint the user documentation so that every code example is a test.

Invariant 8 (docs/decisions/0004-docs-site.md, item 7): a code example in
doc/ is a source block in a language cerastes reads (C, C++ or Viper), and it
is an include of a test file under doc/modules/ROOT/examples/ through a tagged
region, so the page never shows the expectation header.  This script reads
doc/modules/ROOT/nav.adoc, pages/**/*.adoc and partials/**/*.adoc and reports
`<file>:<line>: <message>` for:

  a. a source block in c, cpp, c++, cxx, h, hpp, vpr or viper (and a few
     spellings of those) whose body is anything but one
     `include::example$<test file>[tag=...]` line (a test file has a suffix of
     run_tests.TEST_SUFFIXES; a partial, an attachment or a non-test file is an
     error).  A line-based, best-effort reader of `[source,cpp]`, `[,cpp]`,
     `language=cpp`, `----`, `--`, fenced blocks, the delimiter-less paragraph
     form, blocks that start on a table cell line (`a|[source,cpp]`), block
     titles and anchors; `////` comment blocks and `//` lines are skipped.
     Because it cannot know every AsciiDoc form, --html is the backstop.
  a2. No code without a declared language: a `----` listing, an open source
     block, a fenced block or a source paragraph needs `[source,<language>]`,
     and the language must be a cerastes one (included from a test) or one of
     OUTPUT_LANGUAGES (sh, console, text).  Errors: `....` literal blocks, a
     paragraph that starts indented (a literal block, also after a list
     continuation or in a table cell), `[verse]` and `[pass]` styles, `l|` and
     `m|` cell styles and `cols` with l or m, `++++` passthrough blocks,
     `pass:[...]` with `<` (raw HTML), a `:source-language:` entry, and a
     `source-language` attribute in doc/antora.yml or a playbook in doc/.
  b. an `include::example$...[...]` without `tag=` or `tags=`, with `lines=`
     (which takes precedence over the tags and would show the header), with an
     empty tag, or with a tag that is not a plain name (`*`, `**` and `!x`
     select untagged lines, which include the header).
  c. an include whose target does not exist, is not under example$/partial$,
     is a test file whose header is invalid, or whose tag is not defined in the
     file; or whose tagged region contains an `// expect:` or `// mode:` line
     (the header would be shown).
     A tag directive inside `#if`/`#ifdef`/`#ifndef`, a `/* */` comment or
     unclosed parentheses (the arguments of a macro that discards them) is an
     error too: the region could be shown but compiled out.  So are a raw
     string literal anywhere in an example and a `//` comment ending in a
     backslash (it continues on the next line).
  d. an orphan: a test file under doc/modules/ROOT/examples/ that no page
     includes (an include inside a `////` comment block does not count).
  d2. A file under doc/modules/*/examples/ that is not a test or its `.golden`
     file, and any C/C++/Viper source (by suffix) elsewhere under doc/modules/.
     A module other than ROOT is a usage error (exit 2).
  e. a `++` in text outside `pass:[...]`: it opens an AsciiDoc unconstrained
     passthrough that silently swallows text up to the next `++` (a real build
     garbles a paragraph with two of them, `libc++`, `g++` and `-std=c++20`
     included, in backticks too; `+...+` does not protect it; one alone is left
     alone but is a trap).  A bare `C++` gets its own message (write
     `pass:[C++]`, `pass:[C++]20` for a version).  Checked in prose, titles,
     headings, list items, table cells, inline code, link text and attribute
     entries; not in `----`, fenced and `////` blocks, source/listing
     paragraphs, `//` comment lines and include directives.  A block attribute
     line is checked except for its language (`[source,c++]`).
  f. a doubled formatting mark (`__`, `**`, `##`) that occurs twice in a
     paragraph outside `pass:[...]` and `+...+` literal spans: the pair
     formats everything between them, across code spans (a real build turned
     two `__` code spans into italics; one alone is left alone).

`--html DIR` checks the built site instead of the sources (run it on build/doc
after scripts/build_docs.sh) and does not depend on the line reader above.  It
classifies every `<pre>` (and every `<code>` with a language class or
`data-lang`, `lang-X` included):
  - a cerastes language (c, cpp, ...) must equal, after unescaping and
    whitespace normalisation, the lines of some tags of a test under
    doc/modules/ROOT/examples/ (tag lines dropped), and there must be at least
    as many such blocks as the page sources declare (so a UI class-name change
    cannot make the check vacuous);
  - sh, console and text are fine;
  - anything else is an error: a `<pre>` without `<code>` (literal, verse, raw
    HTML), a `<code>` without a language, another language.
No page chrome is exempt (the default UI's own pages have no `<pre>`; a future
one would need a precise rule and a test).  It also reports broken markup: a
backtick in the text outside `<pre>`, and an inline element closed while one
inside it is still open (what a doubled formatting mark spanning code spans
renders to).  What it cannot see (a `++` pair that eats text, `**` pairs)
only the source lint catches.

The exit status is 0 when clean, 1 on problems and 2 on usage errors (for
example a missing doc component or a module other than ROOT).  `--root DIR`
checks another tree (the self-test uses it).  Stdlib only; the header rules come from run_tests.py.
"""

from __future__ import annotations

import argparse
import dataclasses
import html.parser
import itertools
import re
import sys
import textwrap
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_tests as rt  # noqa: E402

ROOT = rt.ROOT
COMPONENT = Path("doc/modules/ROOT")

# Languages of source blocks that cerastes reads (lower case).  A language that
# is an attribute reference (`{lang}`) cannot be checked and counts too.
CERASTES_LANGUAGES = {"c", "cpp", "c++", "cxx", "cc", "cp", "h", "hh", "hpp", "hxx", "h++",
                      "vpr", "viper", "sil"}

# Asciidoctor's include directive: `include::target[attrs]`, not escaped.
INCLUDE_RE = re.compile(r"^include::([^\s\[](?:[^\[]*[^\s\[])?)\[(.*)\]$")
COMMENT_DELIM_RE = re.compile(r"^/{4,}$")
COMMENT_LINE_RE = re.compile(r"^//(?:[^/].*)?$")
ATTR_ENTRY_RE = re.compile(r"^:(!?)([\w][\w-]*)(!?):(?:\s+(.*))?$")
ANCHOR_RE = re.compile(r"^\[\[.+\]\]$")
ATTR_LINE_RE = re.compile(r"^\[(?!\[)(.*)\]$")
TITLE_RE = re.compile(r"^\.[^\s.].*$")
HEADING_RE = re.compile(r"^=+\s+\S")
LISTING_RE = re.compile(r"^-{4,}$")
LITERAL_RE = re.compile(r"^\.{4,}$")
OPEN_RE = re.compile(r"^--$")
FENCE_RE = re.compile(r"^```([^`]*)$")
PASS_RE = re.compile(r"^\+{4,}$")
# Other delimiters: example, sidebar, quote, tables.  They only end paragraphs.
OTHER_DELIM_RE = re.compile(r"^(={4,}|\*{4,}|_{4,}|[|,:!]={3,})$")
PASS_SPAN_RE = re.compile(r"pass:[a-z,]*\[[^\]]*\]")
# A constrained `+literal+` span (it protects `__`, `**` and `##`, not `++`).
LITERAL_SPAN_RE = re.compile(r"(?<![\w+])\+(?=[^\s+])[^+\n]*?[^\s+]\+(?![\w+])|"
                             r"(?<![\w+])\+[^\s+]\+(?![\w+])")
PLUSPLUS_MESSAGE = ("write it inside pass:[...] (a ++ opens an AsciiDoc passthrough; "
                    "write C++ as pass:[C++], libc++ as pass:[libc++])")
FORMAT_MESSAGE = ("wrap it in a literal span (`+...+`): a doubled formatting mark opens "
                  "unconstrained formatting")
FORMAT_MARKS = ("__", "**", "##")
# Languages a documentation block may declare besides the cerastes ones, which
# must be included from tested examples.  --html checks the built site against
# the same list.  Extending it is a reviewed change.
OUTPUT_LANGUAGES = {"sh", "console", "text"}
BARE_CPP_RE = re.compile(r"C\+\+")
BARE_CPP_MESSAGE = "write C++ as pass:[C++] (a bare C++ opens an AsciiDoc passthrough)"
TAG_DIRECTIVE_RE = re.compile(r"\b(?:tag|(e)nd)::(\S+?)\[\](?=$|[ \r])")
TAG_NAME_RE = re.compile(r"^[A-Za-z0-9_][A-Za-z0-9_.-]*$")
# `[ROOT:]family$path`; only these families are accepted in an include.
TARGET_RE = re.compile(r"^(?P<prefix>(?:ROOT:)?)(?P<family>[a-z]+)\$(?P<path>.+)$")


@dataclasses.dataclass(frozen=True)
class Problem:
    file: str
    line: int
    message: str

    def __str__(self) -> str:
        return f"{self.file}:{self.line}: {self.message}"


@dataclasses.dataclass
class Include:
    line: int
    target: str
    attrs: str
    in_comment: bool


@dataclasses.dataclass
class PageScan:
    problems: list[tuple[int, str]] = dataclasses.field(default_factory=list)
    includes: list[Include] = dataclasses.field(default_factory=list)
    blocks: int = 0  # source blocks in a cerastes language, checked


# ---------------------------------------------------------------------------
# AsciiDoc attribute lists
# ---------------------------------------------------------------------------

def split_attrs(text: str) -> list[str]:
    """Split an attribute list at commas that are not inside quotes."""
    parts: list[str] = []
    cur: list[str] = []
    quote = None
    for ch in text:
        if quote:
            cur.append(ch)
            if ch == quote:
                quote = None
        elif ch in "\"'":
            quote = ch
            cur.append(ch)
        elif ch == ",":
            parts.append("".join(cur))
            cur = []
        else:
            cur.append(ch)
    parts.append("".join(cur))
    return [p.strip() for p in parts]


def unquote(value: str) -> str:
    value = value.strip()
    if len(value) >= 2 and value[0] == value[-1] and value[0] in "\"'":
        return value[1:-1]
    return value


def parse_attrs(text: str) -> tuple[list[str], dict[str, str]]:
    """(positional tokens in order, named attributes) of an attribute list.

    A token is named when it has the form `name=value`.
    """
    positional: list[str] = []
    named: dict[str, str] = {}
    for token in split_attrs(text):
        m = re.match(r"^([A-Za-z_][\w-]*)\s*=\s*(.*)$", token, re.DOTALL)
        if m:
            named[m.group(1).lower()] = unquote(m.group(2))
        else:
            positional.append(unquote(token))
    return positional, named


# ---------------------------------------------------------------------------
# Page scanning
# ---------------------------------------------------------------------------

@dataclasses.dataclass
class Pending:
    """Block attributes seen since the last block."""
    style: str | None = None  # `source`, `listing`, ... (None: not given)
    language: str | None = None
    language_line: int = 0  # the attribute line that gave it


def language_of(attr_line: str) -> tuple[str | None, str | None]:
    """(style, language) given by one block attribute line, lower case.

    The style is the first positional token up to its `#`, `.` or `%` shorthand
    (`''` for `[,cpp]` and `[.role,cpp]`).  The language is the second
    positional token or a `language=`/`lang=` attribute.  To stay on the safe
    side, a cerastes language in any later positional token counts too (the
    `[source,subs=x,cpp]` corner, where Asciidoctor itself would not read it
    as the language).
    """
    tokens = split_attrs(attr_line)
    _positional, named = parse_attrs(attr_line)
    style = None
    if not re.match(r"^[A-Za-z_][\w-]*\s*=", tokens[0]):
        style = re.split(r"[#.%]", unquote(tokens[0]))[0].strip().lower()
    later = [unquote(t).strip() for t in tokens[1:]
             if not re.match(r"^[A-Za-z_][\w-]*\s*=", t)]
    language = named.get("language") or named.get("lang") or (later[0] if later else None)
    for token in later:
        if token.lower() in CERASTES_LANGUAGES:
            language = token
            break
    return style, (language.strip().lower() if language else None)


def is_cerastes_language(language: str | None) -> bool:
    return bool(language) and (language in CERASTES_LANGUAGES or "{" in language)


def is_test_example_target(target: str) -> bool:
    """Whether an include target is `[ROOT:]example$<path of a test suffix>`."""
    m = TARGET_RE.match(target)
    return bool(m and m.group("family") == "example"
                and Path(m.group("path")).suffix in rt.TEST_SUFFIXES)


def has_bare_cpp(text: str) -> bool:
    """Whether `text` has a `C++` that is not exactly `pass:[C++]`."""
    for m in BARE_CPP_RE.finditer(text):
        if text[max(0, m.start() - 6):m.start()] != "pass:[" or text[m.end():m.end() + 1] != "]":
            return True
    return False


def has_bare_plusplus(text: str) -> bool:
    """Whether `text` has a `++` outside every `pass:[...]` (`+...+` does not
    protect it, as a real build shows)."""
    return "++" in PASS_SPAN_RE.sub("", text)


def pass_with_markup(text: str) -> bool:
    """A `pass:[...]` whose content has `<`: it can emit raw HTML."""
    return any("<" in m.group(0) for m in PASS_SPAN_RE.finditer(text))


def doubled_marks(texts: list[str]) -> list[int]:
    """Indices (into `texts`, the lines of one paragraph) of the lines where a
    formatting mark (`__`, `**`, `##`) occurs a second time, outside `pass:[]`
    and `+...+`: the pair formats text between them, across code spans too.  A
    single mark is safe (a real build leaves it alone)."""
    found: list[int] = []
    counts = {mark: 0 for mark in FORMAT_MARKS}
    for i, text in enumerate(texts):
        text = re.sub(r"^\s*(?:[*\-]+|\.+)\s+", "", text)  # a list marker is not a mark
        text = LITERAL_SPAN_RE.sub("", PASS_SPAN_RE.sub("", text))
        for mark in FORMAT_MARKS:
            here = text.count(mark)
            counts[mark] += here
            if here and counts[mark] >= 2 and i not in found:
                found.append(i)
    return found


def attr_line_text(attr_line: str) -> str:
    """The text of a block attribute line that can be rendered: without the
    language of a source block (`[source,c++]`, `[source,language=C++]`)."""
    kept = []
    for token in split_attrs(attr_line):
        m = re.match(r"^([A-Za-z_][\w-]*)\s*=\s*(.*)$", token, re.DOTALL)
        if m and m.group(1).lower() in ("language", "lang"):
            continue
        if not m and unquote(token).strip().lower() in CERASTES_LANGUAGES:
            continue
        kept.append(token)
    return ",".join(kept)


def is_verbatim(pending: "Pending | None") -> bool:
    """Whether the block that the pending attributes apply to keeps its text
    as it is (a source, listing or literal block or paragraph)."""
    if pending is None:
        return False
    if pending.style in ("source", "listing", "literal"):
        return True
    return pending.style in (None, "") and bool(pending.language)


LIST_ITEM_RE = re.compile(r"^\s*(?:[*\-]+|\.+|\d+\.|[A-Za-z]\.|[ivxIVX]+\))\s+\S|^\S.*(?:::|;;)(?:\s|$)")
CELL_A_RE = re.compile(r"(?:^|\s)[\d.+*<^>]*a\|")
CELL_START_RE = re.compile(r"^[\d.+*<^>a-z]*\|")
CELL_LITERAL_RE = re.compile(r"(?:^|\s)[\d.+*<^>]*[lm]\|")


def cell_content(line: str) -> str:
    """The AsciiDoc content that starts a table cell on `line`: after the last
    `a|` cell separator, or after the first `|` of a plain cell."""
    found = list(CELL_A_RE.finditer(line))
    if found:
        return line[found[-1].end():]
    if m := CELL_START_RE.match(line):
        return line[m.end():]
    return line


class PageScanner:
    """A line-based reader of one AsciiDoc page."""

    def __init__(self, text: str):
        self.lines = [l.rstrip() for l in text.replace("\r\n", "\n").split("\n")]
        if self.lines and self.lines[-1] == "":
            self.lines.pop()
        self.pos = 0
        self.in_comment = False
        self.in_table = False
        self.scan = PageScan()
        self.default_language: str | None = None  # `:source-language:`
        self.pending: Pending | None = None

    # -- reading ----------------------------------------------------------
    def eof(self) -> bool:
        return self.pos >= len(self.lines)

    def peek(self) -> str:
        return self.lines[self.pos]

    def advance(self) -> str:
        """Consume a line.  Include directives are processed in every context
        (as Asciidoctor's reader does), comment blocks included."""
        line = self.lines[self.pos]
        self.pos += 1
        if m := INCLUDE_RE.match(line):
            self.scan.includes.append(
                Include(self.pos, m.group(1), m.group(2), self.in_comment))
        return line

    def problem(self, lineno: int, message: str) -> None:
        self.scan.problems.append((lineno, message))

    def prose(self, lineno: int, text: str) -> None:
        """Rules e and f on one line of rendered text."""
        self.prose_lines([(lineno, text)])

    def prose_lines(self, items: list[tuple[int, str]]) -> None:
        """Rules e (a bare `C++` or `++`) and f (doubled formatting marks) on the
        lines of one paragraph (table cells and attribute lines: one line)."""
        for lineno, text in items:
            if has_bare_cpp(text):
                self.problem(lineno, BARE_CPP_MESSAGE)
            elif has_bare_plusplus(text):
                self.problem(lineno, PLUSPLUS_MESSAGE)
            if pass_with_markup(text):
                self.problem(lineno, "pass:[...] with markup (<) is not allowed: it can "
                             "emit raw HTML, including code that nothing tests")
        if self.in_table:
            scopes = [[(n, seg)] for n, text in items for seg in text.split("|")]
            for scope in scopes:
                for i in doubled_marks([scope[0][1]]):
                    self.problem(scope[0][0], FORMAT_MESSAGE)
        else:
            # One scope per list item or description-list entry: a mark in one
            # item cannot pair with a mark in the next (checked in a real build).
            groups: list[list[tuple[int, str]]] = []
            for item in items:
                if not groups or LIST_ITEM_RE.match(item[1]):
                    groups.append([])
                groups[-1].append(item)
            for group in groups:
                for i in doubled_marks([text for _n, text in group]):
                    self.problem(group[i][0], FORMAT_MESSAGE)

    # -- blocks -----------------------------------------------------------
    def run(self) -> PageScan:
        while not self.eof():
            self.step()
        return self.scan

    def step(self) -> None:
        line = self.peek()
        if self.in_table and line.strip() and not OTHER_DELIM_RE.match(line):
            # A table cell may hold AsciiDoc that starts on the cell's line
            # (`a|[source,cpp]`): read the cell content as a line of its own.
            starts_cell = bool(CELL_A_RE.search(line.lstrip())
                               or CELL_START_RE.match(line.lstrip()))
            if CELL_LITERAL_RE.search(line.lstrip()):
                self.problem(self.pos + 1, "table cell styles l and m show literal text, "
                             "which is code without a declared language: not allowed")
            if starts_cell:
                self.lines[self.pos] = line = cell_content(line.lstrip()).lstrip()
        if not line.strip():
            self.advance()
        elif line.strip() == "+":
            self.advance()  # a list continuation: the next block belongs to the item
        elif COMMENT_DELIM_RE.match(line):
            self.comment_block(line)
        elif COMMENT_LINE_RE.match(line):
            self.advance()
        elif m := ATTR_ENTRY_RE.match(line):
            self.prose(self.pos + 1, line)
            name = m.group(2).lower()
            if name == "source-language":
                unset = bool(m.group(1) or m.group(3))
                if not unset:
                    self.problem(self.pos + 1, "`:source-language:` is not allowed: declare "
                                 "the language of each block with [source,<language>]")
                self.default_language = None if unset else (m.group(4) or "").strip().lower()
            self.advance()
        elif ANCHOR_RE.match(line):
            self.advance()
        elif m := ATTR_LINE_RE.match(line):
            self.prose(self.pos + 1, attr_line_text(m.group(1)))
            self.advance()
            style, language = language_of(m.group(1))
            if style in ("verse", "pass"):
                self.problem(self.pos, f"the [{style}] style is not allowed: it shows text "
                             "verbatim (verse) or emits raw HTML (pass)")
            self.cols_problem(self.pos, m.group(1))
            if re.search(r"(?:^|,)\s*subs\s*=", m.group(1), re.I):
                self.problem(self.pos, "`subs=` is not allowed: it can turn off escaping and "
                             "emit raw HTML, including code that nothing tests")
            pending = self.pending or Pending()
            if style:
                pending.style = style
            if language:
                pending.language = language
                pending.language_line = self.pos  # the line just consumed
            self.pending = pending
        elif TITLE_RE.match(line):
            self.prose(self.pos + 1, line)
            self.advance()
        elif LISTING_RE.match(line):
            self.delimited(line, "listing")
        elif LITERAL_RE.match(line):
            self.delimited(line, "literal")
        elif OPEN_RE.match(line):
            if is_verbatim(self.pending):
                self.delimited(line, "open")
            else:
                self.advance()  # an open block holds ordinary content
                self.pending = None
        elif (m := FENCE_RE.match(line)):
            self.fenced(m.group(1))
        elif PASS_RE.match(line):
            self.problem(self.pos + 1, "passthrough blocks (++++) are not allowed: they "
                         "emit raw HTML, including code that nothing tests")
            self.advance()
            while not self.eof() and self.peek() != line:
                self.advance()
            if not self.eof():
                self.advance()
            self.pending = None
        elif OTHER_DELIM_RE.match(line):
            self.advance()
            self.pending = None
            if line[0] in "|,:!":
                self.in_table = not self.in_table
        elif HEADING_RE.match(line):
            self.prose(self.pos + 1, line)
            self.advance()
            self.pending = None
        else:
            self.paragraph()

    def cols_problem(self, lineno: int, attr_line: str) -> None:
        """`cols` specs with a literal (l) or monospace (m) column style."""
        _positional, named = parse_attrs(attr_line)
        cols = named.get("cols")
        if cols is None:
            return
        for spec in re.split(r"[,;]", cols):
            if spec.strip().lower().endswith(("l", "m")):
                self.problem(lineno, f"`cols` spec `{spec.strip()}`: column styles l and m "
                             "show literal text, which is code without a declared language")
                return

    def comment_block(self, delimiter: str) -> None:
        self.advance()
        self.in_comment = True
        while not self.eof() and self.peek() != delimiter:
            self.advance()
        if not self.eof():
            self.advance()
        self.in_comment = False

    def declared_language(self) -> str | None:
        """The language the block attributes declare: `[source,<lang>]`,
        `[,<lang>]` or `[source,language=<lang>]` (a `:source-language:`
        default does not count)."""
        pending = self.pending
        if pending and pending.style in (None, "", "source", "listing") and pending.language:
            return pending.language
        return None

    def source_language(self, listing: bool) -> str | None:
        """The language of the block about to start if it is a source block
        (the `language` may be anything), else None."""
        pending = self.pending
        style = pending.style if pending else None
        language = pending.language if pending else None
        if style not in (None, "", "source", "listing"):
            return None
        if language:
            return language
        if style == "source" or (style is None and listing):
            return self.default_language  # may be None: a plain listing
        return None

    UNDECLARED = ("{what} without a declared language: write [source,<language>] before it "
                  "(use [source,text] for output that is not code)")

    def delimited(self, delimiter: str, kind: str) -> None:
        language = self.source_language(kind == "listing")
        declared = self.declared_language()
        language_line = self.pending.language_line if self.pending else 0
        self.pending = None
        opening = self.pos + 1
        self.advance()
        body: list[tuple[int, str]] = []
        while not self.eof() and self.peek() != delimiter:
            body.append((self.pos + 1, self.advance()))
        if not self.eof():
            self.advance()
        if kind == "literal":
            self.problem(opening, "literal blocks (....) are not allowed: use "
                         "[source,<language>] with ---- (or [source,text])")
        elif declared is None:
            self.problem(opening, self.UNDECLARED.format(
                what="listing block" if kind == "listing" else "open source block"))
        self.check_body(language, body, skip_comments=False, language_line=language_line)

    def fenced(self, info: str) -> None:
        tokens = info.strip().split()
        language = re.sub(r"[{}.]", "", tokens[0]).lower() if tokens else None
        opening = self.pos + 1
        self.pending = None
        self.advance()
        body = []
        while not self.eof() and self.peek().rstrip() != "```":
            body.append((self.pos + 1, self.advance()))
        if not self.eof():
            self.advance()
        if not language:
            self.problem(opening, self.UNDECLARED.format(what="fenced block").replace(
                "write [source,<language>] before it", "write ```<language>"))
        self.check_body(language, body, skip_comments=False, language_line=opening)

    def paragraph(self) -> None:
        language = self.source_language(False)
        verbatim = is_verbatim(self.pending)
        declared = self.declared_language()
        language_line = self.pending.language_line if self.pending else 0
        first_line = self.peek()
        self.pending = None
        body = []
        while not self.eof():
            line = self.peek()
            if not line.strip() or (body and line.strip() == "+"):
                break
            if body and (ATTR_LINE_RE.match(line) or ANCHOR_RE.match(line)
                         or self.is_delimiter(line)):
                break
            if body and self.in_table and (CELL_A_RE.search(line) or CELL_START_RE.match(line)):
                break
            body.append((self.pos + 1, self.advance()))
        if verbatim and declared is None:
            self.problem(body[0][0], self.UNDECLARED.format(what="source paragraph"))
        if not verbatim:
            # An indented paragraph is a literal block, which shows code
            # without a declared language (also after a list continuation, in
            # a table cell and in a description list); a list item may be
            # indented.
            if first_line[:1] in (" ", "\t") and not LIST_ITEM_RE.match(first_line):
                self.problem(body[0][0], "an indented paragraph is a literal block: use "
                             "[source,<language>] and ----, or do not indent")
            self.prose_lines([(n, l) for n, l in body
                              if not INCLUDE_RE.match(l) and not COMMENT_LINE_RE.match(l)])
        self.check_body(language, body, skip_comments=True, language_line=language_line)

    @staticmethod
    def is_delimiter(line: str) -> bool:
        return bool(COMMENT_DELIM_RE.match(line) or LISTING_RE.match(line)
                    or LITERAL_RE.match(line) or OPEN_RE.match(line)
                    or PASS_RE.match(line) or OTHER_DELIM_RE.match(line)
                    or FENCE_RE.match(line))

    def check_body(self, language: str | None, body: list[tuple[int, str]],
                   skip_comments: bool, language_line: int = 0) -> None:
        """Rule a: the body of a cerastes-language block is exactly one
        `include::example$<test file>[tag=...]` line."""
        if language and not is_cerastes_language(language) and language not in OUTPUT_LANGUAGES:
            at = language_line or (body[0][0] - 1 if body else self.pos)
            self.problem(at, f"language `{language}` is not allowed in a documentation block: "
                         "use one of " + ", ".join(sorted(OUTPUT_LANGUAGES)) + ", or include a "
                         "tested example in a cerastes language (OUTPUT_LANGUAGES in "
                         "scripts/check_doc_examples.py)")
        if not is_cerastes_language(language):
            return
        self.scan.blocks += 1
        includes = 0
        for lineno, line in body:
            if not line.strip() or (skip_comments and COMMENT_LINE_RE.match(line)):
                continue
            if not (m := INCLUDE_RE.match(line)):
                self.problem(lineno, f"inline {language} source: a code example must be an "
                             "include::example$...[tag=...] of a tested file under "
                             "doc/modules/ROOT/examples/")
                return
            if not is_test_example_target(m.group(1)):
                self.problem(lineno, f"a {language} block may include only "
                             "example$<file ending in " + ", ".join(sorted(rt.TEST_SUFFIXES))
                             + ">; `" + m.group(1) + "` is something else (a partial, an "
                             "attachment or a file that is not a test)")
                return
            includes += 1
            if includes == 2:
                self.problem(lineno, f"a {language} block holds one include (use "
                             "tags=a;b to show several regions)")
        if not includes:
            first = body[0][0] if body else self.pos
            self.problem(first, f"empty {language} source block: it shows no example "
                         "(use include::example$...[tag=...])")


def scan_page(text: str) -> PageScan:
    """The rule-a problems and the include directives of one page."""
    return PageScanner(text).run()


# ---------------------------------------------------------------------------
# Includes
# ---------------------------------------------------------------------------

def tag_problems(attrs: str) -> tuple[list[str], list[str]]:
    """(tag names, problems) of the attribute list of an example include."""
    positional, named = parse_attrs(attrs)
    problems: list[str] = []
    names: list[str] = []
    if "lines" in named:
        problems.append("`lines=` takes precedence over the tags and would show the "
                        "expectation header; select the region with `tag=`")
    values = [named[k] for k in ("tag", "tags") if k in named]
    if not values:
        if "lines" in named:
            return names, problems  # already reported
        problems.append("untagged include of an example: the expectation header "
                        "would be shown; use `tag=<name>`")
        return names, problems
    for key in ("tag", "tags"):
        if key not in named:
            continue
        value = named[key]
        if "{" in value:
            problems.append(f"`{key}={value}`: an attribute reference cannot be checked")
            continue
        tokens = [t.strip() for t in re.split(r"[;,]", value)]
        if not value.strip() or any(not t for t in tokens):
            problems.append(f"`{key}=` has an empty tag name, which includes the whole "
                            "file with its expectation header")
            continue
        for token in tokens:
            if TAG_NAME_RE.fullmatch(token):
                names.append(token)
            else:
                problems.append(f"`{key}={value}`: `{token}` is not a plain tag name "
                                "(wildcards and negations select untagged lines, "
                                "which include the expectation header)")
    return names, problems


def file_lines(text: str) -> list[str]:
    """The lines of an example file as Asciidoctor's reader sees them: line
    ends removed and trailing whitespace stripped."""
    return [l.rstrip() for l in text.replace("\r\n", "\n").split("\n")]


def tagged_lines(text: str, name: str) -> tuple[bool, bool, list[str]]:
    """(tag opened, tag closed, the lines of the region) for tag `name`."""
    opened = closed = False
    active = False
    region: list[str] = []
    for line in file_lines(text):
        directives = list(TAG_DIRECTIVE_RE.finditer(line))
        if directives:
            for d in directives:
                if d.group(2) == name:
                    if d.group(1):
                        closed = closed or active
                        active = False
                    else:
                        opened = True
                        active = True
            continue
        if active:
            region.append(line)
    return opened, closed, region


@dataclasses.dataclass
class Report:
    problems: list[Problem] = dataclasses.field(default_factory=list)
    pages: int = 0
    blocks: int = 0
    includes: int = 0
    examples: int = 0


def resolve(base: Path, path: str) -> Path | None:
    """`base`/`path`, or None if it would leave `base`."""
    full = (base / path).resolve()
    try:
        full.relative_to(base.resolve())
    except ValueError:
        return None
    return full


def check_include(inc: Include, page: str, root: Path, report: Report,
                  used: set[Path]) -> None:
    def problem(message: str) -> None:
        report.problems.append(Problem(page, inc.line, message))

    component = root / COMPONENT
    target = inc.target
    m = TARGET_RE.match(target)
    if "{" in target:
        problem(f"include target `{target}` has an attribute reference; it cannot be "
                "checked")
        return
    if not m or m.group("family") not in ("example", "partial"):
        problem(f"include target `{target}`: only `example$` and `partial$` includes "
                "are allowed (a code example must come from examples/)")
        return
    family = m.group("family")
    base = component / ("examples" if family == "example" else "partials")
    full = resolve(base, m.group("path"))
    if full is None:
        problem(f"include target `{target}` leaves the {family}s directory")
        return
    if not full.is_file():
        problem(f"include target `{target}` does not exist "
                f"({full.relative_to(root) if full.is_relative_to(root) else full})")
        return
    if family == "partial":
        return
    report.includes += 1
    if not inc.in_comment:
        used.add(full)

    names, problems = tag_problems(inc.attrs)
    for p in problems:
        problem(p)
    if full.suffix in rt.TEST_SUFFIXES:
        exp = rt.read_header(full)
        if isinstance(exp, rt.HeaderError):
            problem(f"`{target}` is not a valid test: {exp}")
    else:
        problem(f"`{target}` is not a test file (its suffix must be one of "
                f"{', '.join(sorted(rt.TEST_SUFFIXES))}), so it would never run")
    text = rt.read_exact(full)
    risky = risky_tags(text, full.suffix)
    for name in names:
        opened, closed, region = tagged_lines(text, name)
        if not opened or not closed:
            problem(f"tag `{name}` is not defined in `{target}` (needs both "
                    f"`tag::{name}[]` and `end::{name}[]` lines)")
            continue
        if any(rt._EXPECT_RE.match(l.strip()) or rt._MODE_RE.match(l.strip())
               for l in region):
            problem(f"tag `{name}` in `{target}` covers an `// expect:` or `// mode:` "
                    "line, so the page would show the expectation header")
        if name in risky:
            problem(f"tag `{name}` in `{target}` has a tag directive inside an `#if`, "
                    "`#ifdef`, `#ifndef`, a `/* */` comment or unclosed parentheses (a "
                    "macro call): the region could be shown but never compiled; put the "
                    "tags outside conditionals, comments and macro arguments")


class UsageError(Exception):
    """The documentation tree cannot be checked (exit status 2)."""


def risky_tags(text: str, suffix: str) -> set[str]:
    """Tags with a directive at nonzero `#if` depth, inside a `/* */` comment or
    inside unclosed parentheses (the arguments of a macro that discards them).

    Such a region is shown on the page but may be compiled out, so the page
    could show code that the test never compiles.
    """
    risky: set[str] = set()
    depth = paren = 0
    in_block = False
    for line in file_lines(text):
        for d in TAG_DIRECTIVE_RE.finditer(line):
            if depth > 0 or paren > 0 or in_block or in_block_comment(line[:d.start()], in_block):
                risky.add(d.group(2))
        if not in_block and suffix != ".vpr":
            if re.match(r"\s*#\s*(?:if|ifdef|ifndef)\b", line):
                depth += 1
            elif re.match(r"\s*#\s*endif\b", line):
                depth = max(0, depth - 1)
        code, in_block = strip_code(line, in_block)
        paren = max(0, paren + code.count("(") - code.count(")"))
    return risky


TAG_LINE_RE = re.compile(r"^\s*//\s*(?:tag|end)::[A-Za-z0-9_][\w.-]*\[\]\s*$")
RAW_STRING_PREFIX_RE = re.compile(r"(?<![A-Za-z0-9_])(?:u8|u|U|L)?R$")
COMMENT_CONTINUATION_RE = re.compile(r"//.*\\\s*$")


def has_raw_string(line: str) -> bool:
    """Whether `line` opens a raw string literal (`R"(`, `u8R"x(`, ...), outside
    comments and ordinary string and character literals."""
    i = 0
    while i < len(line):
        c = line[i]
        if line.startswith("//", i):
            return False
        if line.startswith("/*", i):
            j = line.find("*/", i + 2)
            if j < 0:
                return False
            i = j + 2
        elif c == '"' and RAW_STRING_PREFIX_RE.search(line[:i]):
            return True
        elif c in "\"'":
            j = i + 1
            while j < len(line) and line[j] != c:
                j += 2 if line[j] == "\\" else 1
            i = j + 1
        else:
            i += 1
    return False


def example_file_problems(text: str, suffix: str) -> list[tuple[int, str]]:
    """Constructs of an example file that can hide a tag directive in code the
    compiler skips (a raw string, a `//` comment that continues on the next
    line); the checks of risky_tags cannot see through them."""
    problems = []
    previous = ""
    for lineno, line in enumerate(file_lines(text), start=1):
        if TAG_DIRECTIVE_RE.search(line):
            if not TAG_LINE_RE.match(line):
                problems.append((lineno, "a tag directive must be a line comment alone on "
                                 "its line: `// tag::name[]` or `// end::name[]` (anywhere "
                                 "else it can sit in a macro or a continued line that is "
                                 "shown but not compiled)"))
            elif previous.rstrip().endswith("\\"):
                problems.append((lineno, "a tag directive follows a line ending in a "
                                 "backslash, so it is part of that continued line"))
        previous = line
        if suffix != ".vpr" and has_raw_string(line):
            problems.append((lineno, "raw string literal (R\"...\") in an example: a tag "
                             "directive inside it would be shown but is not code; write the "
                             "string another way"))
        if COMMENT_CONTINUATION_RE.search(line):
            problems.append((lineno, "a `//` comment ends in a backslash, so it continues on "
                             "the next line, which would be shown but not compiled; remove "
                             "the backslash"))
    return problems


def strip_code(line: str, in_block: bool) -> tuple[str, bool]:
    """(`line` without comments, string and character literals; whether a
    `/* */` comment is open at its end)."""
    out: list[str] = []
    i = 0
    while i < len(line):
        if in_block:
            j = line.find("*/", i)
            if j < 0:
                return "".join(out), True
            in_block, i = False, j + 2
            continue
        c = line[i]
        if line.startswith("//", i):
            break
        if line.startswith("/*", i):
            in_block, i = True, i + 2
        elif c in "\"'":
            j = i + 1
            while j < len(line) and line[j] != c:
                j += 2 if line[j] == "\\" else 1
            i = j + 1
        else:
            out.append(c)
            i += 1
    return "".join(out), in_block


def in_block_comment(text: str, in_block: bool) -> bool:
    """Whether a `/* */` comment is open after `text`, given the state before
    it (a `//` comment ends the scan of the line; string literals are ignored)."""
    i = 0
    while i < len(text):
        if in_block:
            j = text.find("*/", i)
            if j < 0:
                return True
            in_block, i = False, j + 2
        else:
            j, k = text.find("/*", i), text.find("//", i)
            if k >= 0 and (j < 0 or k < j):
                return False
            if j < 0:
                return False
            in_block, i = True, j + 2
    return in_block


# ---------------------------------------------------------------------------
# The tree
# ---------------------------------------------------------------------------

SOURCE_LIKE = {s.lower() for s in rt.SOURCE_LIKE_SUFFIXES}
IGNORED_FILES = {".DS_Store"}


def page_files(component: Path) -> list[Path]:
    files = []
    nav = component / "nav.adoc"
    if nav.is_file():
        files.append(nav)
    for sub in ("pages", "partials"):
        d = component / sub
        if d.is_dir():
            files += sorted(d.rglob("*.adoc"))
    return files


def check_modules(root: Path) -> None:
    """ROOT is the only module: pages of another one would go unchecked."""
    modules = root / COMPONENT.parent
    if modules.is_dir():
        for d in sorted(modules.iterdir()):
            if d.is_dir() and d.name != "ROOT":
                raise UsageError(
                    f"{d.relative_to(root)}: doc/modules/ has a module other than ROOT; this "
                    "checker reads ROOT only (a new module is a reviewed change: extend "
                    "scripts/check_doc_examples.py first)")


def check_component_files(root: Path, report: Report) -> None:
    """No component-wide `source-language` attribute (antora.yml, playbooks):
    it would turn undeclared listings into source blocks."""
    doc = root / "doc"
    for f in sorted([doc / "antora.yml", *doc.glob("*playbook*.yml")]):
        if not f.is_file():
            continue
        for lineno, line in enumerate(rt.read_exact(f).split("\n"), start=1):
            if "source-language" in line and not line.lstrip().startswith("#"):
                report.problems.append(Problem(
                    str(f.relative_to(root)), lineno,
                    "`source-language` must not be set for the whole component or "
                    "site: declare the language of each block with [source,<language>]"))


def check_files(root: Path, report: Report) -> None:
    """Every file in an examples directory is a test or its golden file, and
    there is no other C/C++/Viper source under doc/modules/."""
    modules = root / COMPONENT.parent
    examples = (root / COMPONENT / "examples").resolve()
    for f in sorted(modules.rglob("*")):
        if not f.is_file() or f.name in IGNORED_FILES:
            continue
        rel = str(f.relative_to(root))
        in_examples = f.resolve().is_relative_to(examples) or "examples" in f.relative_to(
            modules).parts[1:2]
        if in_examples:
            if f.suffix not in rt.TEST_SUFFIXES and not f.name.endswith(rt.GOLDEN_SUFFIX):
                report.problems.append(Problem(
                    rel, 1, "not a test or a golden file: examples/ holds tests only "
                    f"({', '.join(sorted(rt.TEST_SUFFIXES))}, or a `.golden` file), because "
                    "anything else can be published without ever being run"))
        elif f.suffix.lower() in SOURCE_LIKE:
            report.problems.append(Problem(
                rel, 1, "C/C++/Viper source outside doc/modules/ROOT/examples/ would be "
                "published without being tested: move it there as a test, or remove it"))


def check_tree(root: Path) -> Report:
    """Run every rule on the documentation component under `root`.

    Raises UsageError if doc/modules/ has a module other than ROOT.
    """
    check_modules(root)
    component = root / COMPONENT
    report = Report()
    used: set[Path] = set()
    for page in page_files(component):
        rel = str(page.relative_to(root))
        report.pages += 1
        scan = scan_page(rt.read_exact(page))
        report.blocks += scan.blocks
        for line, message in scan.problems:
            report.problems.append(Problem(rel, line, message))
        for inc in scan.includes:
            check_include(inc, rel, root, report, used)

    check_component_files(root, report)
    if (root / COMPONENT.parent).is_dir():
        check_files(root, report)
    examples = component / "examples"
    if examples.is_dir():
        for f in sorted(examples.rglob("*")):
            if f.is_file() and f.suffix in rt.TEST_SUFFIXES:
                report.examples += 1
                for lineno, message in example_file_problems(rt.read_exact(f), f.suffix):
                    report.problems.append(Problem(str(f.relative_to(root)), lineno, message))
                if f.resolve() not in used:
                    report.problems.append(Problem(
                        str(f.relative_to(root)), 1,
                        "orphan example: no page includes it (a test nobody reads); "
                        "include it from a page through a tag, or delete it"))
    report.problems.sort(key=lambda p: (p.file, p.line, p.message))
    return report


# ---------------------------------------------------------------------------
# The built site (--html): the backstop that does not depend on the parser above
# ---------------------------------------------------------------------------

def normalize_code(text: str) -> str:
    """Code text compared without whitespace at line ends, common indentation or
    blank lines at its start and end."""
    lines = [l.rstrip() for l in text.replace("\r\n", "\n").split("\n")]
    text = textwrap.dedent("\n".join(lines))
    return "\n".join(text.strip("\n").split("\n")).strip("\n")


def region_texts(text: str) -> set[str]:
    """What an include with tag(s) selects from `text`: the lines of the regions
    of the chosen tags in file order, tag directive lines dropped, as
    Asciidoctor's include does, for every nonempty choice of tags (all of them
    if there are at most 10 tags, else single tags, pairs and all)."""
    lines = file_lines(text)
    names = sorted({d.group(2) for line in lines for d in TAG_DIRECTIVE_RE.finditer(line)})
    choices: list[frozenset[str]] = []
    if len(names) <= 10:
        for mask in range(1, 1 << len(names)):
            choices.append(frozenset(n for i, n in enumerate(names) if mask >> i & 1))
    else:
        choices = [frozenset([n]) for n in names] + [
            frozenset(pair) for pair in itertools.combinations(names, 2)] + [frozenset(names)]
    out: set[str] = set()
    for choice in choices:
        active: set[str] = set()
        picked: list[str] = []
        for line in lines:
            directives = list(TAG_DIRECTIVE_RE.finditer(line))
            if directives:
                for d in directives:
                    (active.discard if d.group(1) else active.add)(d.group(2))
                continue
            if active & choice:
                picked.append(line)
        if picked:
            out.add(normalize_code("\n".join(picked)))
    return out


VOID_TAGS = {"area", "base", "br", "col", "embed", "hr", "img", "input", "link", "meta",
             "param", "source", "track", "wbr"}
INLINE_TAGS = {"em", "strong", "code", "mark", "i", "b", "span", "a", "sub", "sup", "kbd",
               "samp", "u", "s", "small", "cite"}


@dataclasses.dataclass
class Block:
    """A code block of the built site: a `<pre>` (with its first `<code>`, if
    any) or a `<code>` outside `<pre>` that carries a language class."""
    line: int
    has_code: bool  # a <pre> without <code> has none
    languages: list[str]  # from `language-X`, `lang-X` classes and `data-lang`
    text: str


def language_tokens(attrs) -> list[str]:
    tokens = []
    for key, value in attrs:
        if key == "class":
            for token in (value or "").split():
                low = token.lower()
                for prefix in ("language-", "lang-"):
                    if low.startswith(prefix) and len(token) > len(prefix):
                        tokens.append(token[len(prefix):])
                        break
        elif key == "data-lang" and value:
            tokens.append(value)
    return tokens


class SiteScanner(html.parser.HTMLParser):
    """Every `<pre>` and language-classed `<code>` of a built page, plus markup
    that shows broken AsciiDoc: backticks in the text outside `<pre>`, and an
    inline element closed while another one inside it is still open (a
    formatting pair that spans code spans)."""

    def __init__(self) -> None:
        super().__init__(convert_charrefs=True)
        self.blocks: list[Block] = []
        self.problems: list[tuple[int, str]] = []
        self._skip = 0  # inside <script> or <style>
        self._pre: dict | None = None
        self._solo: dict | None = None  # a language-classed <code> outside <pre>
        self._stack: list[tuple[str, int]] = []  # open inline elements

    def _code_start(self, attrs):
        return {"languages": language_tokens(attrs), "text": [], "depth": 1,
                "line": self.getpos()[0]}

    def handle_starttag(self, tag, attrs):
        if tag in ("script", "style"):
            self._skip += 1
        elif tag == "pre" and self._pre is None:
            self._pre = {"line": self.getpos()[0], "code": None, "cur": None}
        elif tag == "code":
            if self._pre is not None:
                if self._pre["cur"] is not None:
                    self._pre["cur"]["depth"] += 1
                elif self._pre["code"] is None:
                    self._pre["cur"] = self._pre["code"] = self._code_start(attrs)
            elif self._solo is not None:
                self._solo["depth"] += 1
            elif language_tokens(attrs):
                self._solo = self._code_start(attrs)
        if tag in INLINE_TAGS and tag not in VOID_TAGS:
            self._stack.append((tag, self.getpos()[0]))

    def handle_endtag(self, tag):
        line = self.getpos()[0]
        if tag in ("script", "style"):
            self._skip = max(0, self._skip - 1)
            return
        if tag in INLINE_TAGS:
            names = [t for t, _ in self._stack]
            if names and names[-1] == tag:
                self._stack.pop()
            elif tag in names:
                open_tag, opened = self._stack[-1]
                self.problems.append((line, f"</{tag}> closes while <{open_tag}> (line "
                                      f"{opened}) is still open: broken AsciiDoc markup, "
                                      "such as a doubled formatting mark spanning code spans"))
                while self._stack.pop()[0] != tag:
                    pass
        elif self._stack and tag not in VOID_TAGS:
            open_tag, opened = self._stack[-1]
            self.problems.append((line, f"<{open_tag}> (line {opened}) is still open at "
                                  f"</{tag}>: broken AsciiDoc markup"))
            self._stack.clear()
        if tag == "code":
            if self._pre is not None and self._pre["cur"] is not None:
                self._pre["cur"]["depth"] -= 1
                if self._pre["cur"]["depth"] == 0:
                    self._pre["cur"] = None
            elif self._solo is not None:
                self._solo["depth"] -= 1
                if self._solo["depth"] == 0:
                    solo, self._solo = self._solo, None
                    self.blocks.append(Block(solo["line"], True, solo["languages"],
                                             "".join(solo["text"])))
        elif tag == "pre" and self._pre is not None:
            pre, self._pre = self._pre, None
            code = pre["code"]
            self.blocks.append(Block(pre["line"], code is not None,
                                     code["languages"] if code else [],
                                     "".join(code["text"]) if code else ""))

    def handle_data(self, data):
        if self._skip:
            return
        if self._pre is not None:
            if self._pre["cur"] is not None:
                self._pre["cur"]["text"].append(data)
        else:
            if self._solo is not None:
                self._solo["text"].append(data)
            if "`" in data:
                self.problems.append((self.getpos()[0], "a literal backtick in the text "
                                      "(outside <pre>): a code span was not rendered, so "
                                      "the markup is broken"))


@dataclasses.dataclass
class HtmlReport:
    problems: list[Problem] = dataclasses.field(default_factory=list)
    files: int = 0
    blocks: int = 0  # code blocks in a cerastes language


def classify_block(block: Block, expected: set[str]) -> str | None:
    """The problem with one code block of the site, or None."""
    langs = [l.lower() for l in block.languages]
    if not block.has_code:
        return ("a <pre> without a <code>: a literal block, verse or raw HTML; all code on a "
                "page must be a source block in a declared language")
    if not langs:
        return ("a code block without a language class: declare it with "
                "[source,<language>]")
    if any(l in CERASTES_LANGUAGES for l in langs):
        if normalize_code(block.text) not in expected:
            first = next((l.strip() for l in block.text.split("\n") if l.strip()), "")
            return (f"{block.languages[0]} code block `{first[:60]}` does not equal any "
                    "tagged region of a test under doc/modules/ROOT/examples/ (code on a "
                    "page must be included from a tested example)")
        return None
    if all(l in OUTPUT_LANGUAGES for l in langs):
        return None
    return (f"code block in language `{block.languages[0]}`, which is not one of "
            + ", ".join(sorted(OUTPUT_LANGUAGES)) + " or a cerastes language")


def check_html(root: Path, html_dir: Path) -> HtmlReport:
    """Every code block of the built site is classified.

    - a block in a cerastes language equals a tagged region of a test under
      doc/modules/ROOT/examples/, and there are at least as many as the pages'
      sources declare (so a UI change that renames the class cannot make the
      check pass without looking at anything);
    - a block in sh, console or text is fine;
    - anything else (a `<pre>` without `<code>`, a `<code>` without a language
      class, another language, `lang-cpp` and so on) is an error;
    - broken markup (a backtick in the text, misnested inline elements) is an
      error.
    """
    expected: set[str] = set()
    examples = root / COMPONENT / "examples"
    if examples.is_dir():
        for f in sorted(examples.rglob("*")):
            if f.is_file() and f.suffix in rt.TEST_SUFFIXES:
                expected |= region_texts(rt.read_exact(f))
    report = HtmlReport()
    try:
        source = check_tree(root)
    except UsageError as err:  # the blocks below are still checked
        report.problems.append(Problem("doc/modules", 0, str(err)))
        source = Report()
    for page in sorted(html_dir.rglob("*.html")):
        report.files += 1
        rel = str(page.relative_to(html_dir))
        scanner = SiteScanner()
        scanner.feed(rt.read_exact(page))
        scanner.close()
        for line, message in scanner.problems:
            report.problems.append(Problem(rel, line, message))
        for block in scanner.blocks:
            if any(l.lower() in CERASTES_LANGUAGES for l in block.languages):
                report.blocks += 1
            if message := classify_block(block, expected):
                report.problems.append(Problem(rel, block.line, message))
    need = max(source.blocks, 1 if source.includes else 0)
    if report.blocks < need:
        report.problems.append(Problem(
            str(html_dir), 0, f"found {report.blocks} code blocks in a cerastes language "
            f"but the pages' sources have {source.blocks} (and {source.includes} example "
            "includes): the check cannot see the code blocks, e.g. because the UI changed "
            "its class names"))
    return report


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--root", type=Path, default=ROOT,
                        help="repository root to check (default: this repository)")
    parser.add_argument("--html", type=Path, metavar="DIR",
                        help="instead of the page sources, check the built site in DIR "
                             "(after scripts/build_docs.sh, build/doc): every code block in "
                             "a cerastes language must equal a tagged region of a test")
    args = parser.parse_args(argv)
    root = args.root.resolve()
    component = root / COMPONENT
    if not (component / "pages").is_dir():
        print(f"check_doc_examples.py: {component / 'pages'} is missing; there is no "
              "documentation component to check", file=sys.stderr)
        return 2
    try:
        if args.html is not None:
            if not args.html.is_dir() or not any(args.html.rglob("*.html")):
                print(f"check_doc_examples.py: {args.html} has no HTML files; build the "
                      "docs first (scripts/build_docs.sh)", file=sys.stderr)
                return 2
            html_report = check_html(root, args.html)
            for p in html_report.problems:
                print(p)
            status = "FAILED" if html_report.problems else "OK"
            print(f"check_doc_examples --html: {status}: {html_report.files} html files, "
                  f"{html_report.blocks} code blocks in a cerastes language, "
                  f"{len(html_report.problems)} problems")
            return 1 if html_report.problems else 0
        report = check_tree(root)
    except UsageError as err:
        print(f"check_doc_examples.py: {err}", file=sys.stderr)
        return 2
    for p in report.problems:
        print(p)
    status = "FAILED" if report.problems else "OK"
    print(f"check_doc_examples: {status}: {report.pages} pages, {report.blocks} source "
          f"blocks, {report.includes} example includes, {report.examples} examples, "
          f"{len(report.problems)} problems")
    return 1 if report.problems else 0


if __name__ == "__main__":
    sys.exit(main())
