#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""cerastes test harness.

Every test file starts with a header of `//` comment lines carrying its
expectations (grammar in tests/README.md):

    // expect: verified
    // expect: error <category> @ L<line>                (one line per error; .vpr: <category>[:<reason>])
    // expect: rejected <diag> @ L<line>                 (one line per diagnostic; C and C++ only)
    // expect: frontend-ok
    // expect: frontend-error <diag> @ L<line>           (one line per diagnostic)
    // expect: frontend-warning <diag> @ L<line>
    // expect: frontend-refused <substring>              (needs cerastes-fe-options)
    // cerastes-fe-options: <options>                    (only with frontend-refused)
    // expect: golden
    // expect: native-ok                                 (accompanies an outcome)
    // mode: c++20 | c23                                 (C and C++ only)
    // twin: <repo-relative test path>                   (tests/annotations/negative/)

`.vpr` files run directly through Silicon.  `.c`/`.cpp` files with
`frontend-ok` run through the stock EDG front end (scripts/cpfe.sh).
`.c`/`.cpp` files with `golden` run through the cerastes front end with its IL
walker (scripts/cerastes-fe.sh <mode option> --cerastes_walk <file>, from the
repository root), and its output must equal the golden file byte for byte:
stdout, then, if stderr is not empty, a line `--- stderr ---` and stderr.
The golden file is `<test>.golden` for a test with one mode and
`<test>.<mode>.golden` for each mode of a test with several.
`verified`, `error` and `rejected` tests (C and C++) run `cerastes verify`
(decision 0011) once per mode, from the repository root:

    build/cerastes verify <file> --std=<c++20|c23> --keep-vpr=build/vpr/<file>.<mode>.vpr

Exit status 0 means verified (a `<file>: verified (...)` line), 1 means
verification errors (`<file>:<line>: error: <category>: <message>` lines, each
possibly followed by `note:` lines), 2 means the program was not accepted (EDG
format diagnostics on stderr), 3 or anything else a tool failure, which fails
whatever the test expects (so does a timeout or a crash).  The expectation must
match exactly, as for `.vpr` tests and for `frontend-error` tests; the
categories are DRIVER_CATEGORIES.  Each run must leave the `--keep-vpr` file and
its `.map`; the harness writes `build/vpr/MANIFEST` for scripts/lint_viper.py.

`frontend-error`/`frontend-warning` tests run the cerastes front end with
`--brief_diagnostics --display_error_number --no_colors` and compare the
multisets of (EDG diagnostic tag, line) it prints; the tag table is the
generated err_data.h/err_codes.h of the active EDG build ($CERASTES_EDG_GEN).
`frontend-refused` tests run the cerastes front end with the options of
`// cerastes-fe-options:` instead of a mode option and require that the driver
refuses them: see tests/README.md.
`native-ok` also compiles the file with Apple clang and, when installed, GCC,
with -Wall -Wextra -Werror and no __CERASTES__; each compiler has an
anti-vacuity canary.  `// twin:` ties a must-fail test under
tests/annotations/negative/ to the positive test it was derived from.  Every
build/env.sh consumer checks that CERASTES_EDG_SRC selects the tree env.sh was
written for (decision 0005).  All of this is documented in tests/README.md.

A test passes only if the outcome matches exactly: every expected error is
reported with the expected category (and reason, if given) at the expected
line, and nothing else is reported; a golden test's output is identical to
its golden file and the front end exits with status 0.

`--update-golden` writes the golden files of the selected golden tests from
the current output instead of comparing.  A golden file is an expectation
(invariant 1 applies): regenerate it only for an intended change, review the
diff, and state the reason in the commit.  scripts/check.sh never uses it.

The walker's stdout must end with the line `end walk: <R> routines reported,
<U> routines with bodies in non-system files outside the primary file`; a run
without it (a crash, or a run that printed only diagnostics) fails whatever
the golden holds, and --update-golden never writes it.

A directory containing a file named `.harness-skip` is skipped, with all its
subdirectories, even when a file inside it is named on the command line.  The
marker holds a one-line reason, which is printed; a skipped directory is
counted as skipped, never as passed.  A file in a skipped directory that
starts with an `// expect:` header is a real test that would never run, and is
an error.  A marker is permitted only in the
directories listed in SKIP_ALLOWED (tests/differential/verifast); one anywhere
else is an error that names the file.

A file under a scanned directory whose suffix says C, C++ or Viper source but
is not a test suffix (.cc, .cxx, .c++, .CPP, .C, .hpp, .h, ...) would never
run, and is an error in normal runs and with --check-headers.  A `*.golden` file that no golden test
uses is an orphan and an error (in normal runs and with --check-headers).

With no path arguments the harness runs DEFAULT_PATHS: tests/ and the
documentation examples in doc/modules/ROOT/examples/ (every code example in
doc/ is a test; see tests/README.md).  Everything above, skip markers, orphan
goldens, unrecognized suffixes and header errors, applies to the doc examples
exactly as to tests/ (SKIP_ALLOWED has no doc path).  A default path that does
not exist is an error (exit 2) that names it, so a vanished examples directory
cannot silently drop the doc tests.  Paths named on the command line replace
the defaults.

The exit status is 1 if any test fails and 2 on usage or environment errors
(including when no test is selected).
"""

from __future__ import annotations

import argparse
import atexit
import concurrent.futures
import dataclasses
import datetime
import difflib
import functools
import hashlib
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
import threading
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TEST_SUFFIXES = {".vpr", ".c", ".cpp"}
GOLDEN_SUFFIX = ".golden"
# Suffixes (lower case) that mark a file as C/C++/Viper source.  A file under a
# scanned directory with one of these that is not in TEST_SUFFIXES (exactly,
# so `.CPP` and `.C` count) would silently never run: an error instead.
SOURCE_LIKE_SUFFIXES = {".c", ".cc", ".cp", ".cpp", ".cxx", ".c++", ".h", ".hh", ".hpp", ".inc",
                        ".def", ".tpp", ".txx", ".ixx", ".cppm", ".tbl",
                        ".hxx", ".h++", ".ipp", ".inl", ".tcc", ".i", ".ii", ".sil", ".vpr"}
SKIP_MARKER = ".harness-skip"
# The only directories where a skip marker is permitted, relative to ROOT.  A
# marker anywhere else is an error, so that nobody can turn a directory of
# real tests green by dropping a file into it.  Adding a directory here is a
# reviewed change (and needs a reason in the commit).
SKIP_ALLOWED = [Path("tests/differential/verifast")]
# What a run with no path arguments covers, relative to ROOT.  Documentation
# examples are tests: dropping the second entry would silently stop running
# them, so the harness self-test pins this constant and a run without paths.
DEFAULT_PATHS = (Path("tests"), Path("doc/modules/ROOT/examples"))
SILICON_TIMEOUT_S = 600
# The largest front-end test (the libc++ headers in the cerastes configuration) takes about 70 s of CPU, which a
# loaded machine stretches past 120 s; a hang still fails, after ten minutes.
FRONTEND_TIMEOUT_S = 600
VERIFY_TIMEOUT_S = 600
NATIVE_TIMEOUT_S = 300

# The command that runs the cerastes front end; the mode option, the walker
# option and the test path are appended.  The self-test replaces it with a
# fake front end.
FE_COMMAND = [str(ROOT / "scripts" / "cerastes-fe.sh")]
WALK_OPTION = "--cerastes_walk"
# The command that runs `cerastes verify` (decision 0011); `verify`, the test
# path, `--std=<mode>` and `--keep-vpr=<path>` are appended.  None: the program
# named by CERASTES_VERIFY in build/env.sh (written by scripts/bootstrap.sh).
# The self-test replaces it with a fake verifier.
VERIFY_COMMAND: list[str] | None = None
# Where `--keep-vpr` output goes: <VPR_DIR>/<test path>.<mode>.vpr, and the
# `.map` next to it.  The directory is cleared by a run that selects a test
# that must produce Viper, and MANIFEST lists the files that run expects
# (one path relative to VPR_DIR per line), for scripts/lint_viper.py.
VPR_DIR = ROOT / "build" / "vpr"
VPR_MANIFEST = "MANIFEST"
# The first line of the manifest: `# run-id: <UTC time>-<hash of the file list>`, so
# that scripts/lint_viper.py can tell the files of this run from stale ones.
RUN_ID_PREFIX = "# run-id: "
STDERR_MARKER = "--- stderr ---"
# The walker's last stdout line.  Its presence proves the walk ran to the end,
# so a run that printed only diagnostics cannot match a golden by accident.
END_WALK_RE = re.compile(r"end walk: \d+ routines reported, \d+ routines with bodies "
                         r"in non-system files outside the primary file")
NO_NEWLINE_MARKER = "\\ No newline at end of stdout"

# Failing tests print at most this many problem lines unless -v is given.
MAX_PROBLEM_LINES = 60

# Language modes and the EDG options that select them.  The first mode listed
# for a suffix is its default.
MODE_OPTIONS = {"c++20": "--c++20", "c23": "--c23"}
# The same modes as `cerastes verify` spells them (decision 0011 item 2).
VERIFY_STD_OPTIONS = {"c++20": "--std=c++20", "c23": "--std=c23"}
DEFAULT_MODES = {".cpp": ["c++20"], ".c": ["c23"]}


class HeaderError(Exception):
    """A test file's expectation header is malformed."""


class UsageError(Exception):
    """Bad command line, test selection or environment (exit status 2)."""


# ---------------------------------------------------------------------------
# Expectation headers
# ---------------------------------------------------------------------------

@dataclasses.dataclass(frozen=True)
class ExpectedError:
    category: str
    reason: str | None  # None: any reason
    line: int
    file: str | None = None  # a repository-relative header; None: the test file

    def __str__(self) -> str:
        reason = f":{self.reason}" if self.reason else ""
        where = f"{self.file}:" if self.file else ""
        return f"{self.category}{reason} @ {where}L{self.line}"


@dataclasses.dataclass(frozen=True)
class ExpectedDiag:
    """One `frontend-error`/`frontend-warning` line."""
    severity: str  # "error" or "warning"
    tag: str  # an EDG diagnostic tag, or `cerastes-<tag>`
    line: int
    file: str | None = None  # a repository-relative header (rejected tests); None: the test file

    def __str__(self) -> str:
        where = f"{self.file}:" if self.file else ""
        return f"{self.tag} @ {where}L{self.line}"


@dataclasses.dataclass
class Expectation:
    # "verified", "error", "rejected", "frontend-ok", "frontend-diag" (any
    # frontend-error/-warning lines), "frontend-refused" or "golden".  The
    # diagnostics of a "rejected" test are in `diags` (all errors), as for a
    # frontend-error test.
    kind: str
    errors: list[ExpectedError]
    modes: list[str]
    diags: list[ExpectedDiag] = dataclasses.field(default_factory=list)
    native_ok: bool = False
    twin: str | None = None
    twin_exempt: str | None = None  # the reason, for a positive test with no must-fail twin
    refused: str | None = None  # frontend-refused: a substring of the refusal message
    fe_options: list[str] = dataclasses.field(default_factory=list)  # cerastes-fe-options
    c_only: str | None = None  # `// c-only: <reason>`: a .c test that does not run in c++20


# Kinds that stand alone: each must be the only outcome line of its test
# (`native-ok`, which accompanies an outcome, may be added to it).
_SOLE_KINDS = ("verified", "frontend-ok", "golden")

_EXPECT_RE = re.compile(r"^//\s*expect:\s*(.*?)\s*$")
_MODE_RE = re.compile(r"^//\s*mode:\s*(.*?)\s*$")
_TWIN_RE = re.compile(r"^//\s*twin:\s*(.*?)\s*$")
_TWIN_EXEMPT_RE = re.compile(r"^//\s*twin-exempt:\s*(.*?)\s*$")
_FE_OPTIONS_RE = re.compile(r"^//\s*cerastes-fe-options:\s*(.*?)\s*$")
_C_ONLY_RE = re.compile(r"^//\s*c-only:\s*(.*?)\s*$")
_REFUSED_FE_RE = re.compile(r"^frontend-refused\s+(?P<substring>\S.*)$")
# `@ L<n>` is a line of the test file; `@ <repository-relative file>:L<n>` a line of a
# user header the test includes (decision 0011 item 4: headers are verified too).
_AT = (r"\s+@\s+(?:(?P<file>[A-Za-z0-9_][A-Za-z0-9_./+-]*):)?L(?P<line>[1-9][0-9]*)$")
_ERROR_RE = re.compile(
    r"^error\s+(?P<category>[a-z][a-z0-9-]*)(?::(?P<reason>[a-z][a-z0-9.]*))?" + _AT)
# An EDG tag is [A-Za-z0-9_]+ (some start with a digit, some have capitals);
# a cerastes back-end diagnostic is `cerastes-<tag>`.  Whether the tag exists is
# checked against the tables by validate_diag_tags (it needs the build).
_DIAG_TAG = r"(?:cerastes-[a-z0-9_]+(?:-[a-z0-9_]+)*|[A-Za-z0-9_]+)"
_REJECTED_RE = re.compile(r"^rejected\s+(?P<tag>" + _DIAG_TAG + r")" + _AT)
_DIAG_RE = re.compile(r"^frontend-(?P<severity>error|warning)\s+(?P<tag>" + _DIAG_TAG
                      + r")\s+@\s+L(?P<line>[1-9][0-9]*)$")
CERASTES_TAG_PREFIX = "cerastes-"

# The categories of `// expect: error <category> @ L<n>` in a C or C++ test:
# the table of decision 0011 item 10, which the driver, this harness, the
# error-messages page and the tests share.  Adding one changes all four (a
# self-test compares this set with the decision record and tests/README.md).
DRIVER_CATEGORIES = frozenset({
    "precondition", "postcondition", "check", "invariant-entry", "invariant-preserved",
    "overflow", "division-by-zero", "shift", "conversion", "missing-return", "termination",
    "noreturn"})
# The outcomes whose test runs `cerastes verify` (when it is a C or C++ file).
VERIFY_KINDS = ("verified", "error", "rejected")

# Outcomes that count as "positive" for the native-ok policy and for twins.
POSITIVE_KINDS = ("golden", "verified")
# Positive tests under these directories (relative to ROOT), and any positive
# test that includes <cerastes.h>, must carry `// expect: native-ok`.
NATIVE_POLICY_DIRS = (Path("tests/annotations"),
                      Path("tests/scalar"),
                      Path("doc/modules/ROOT/examples/annotations"),
                      Path("doc/modules/ROOT/examples/ordinary-compilers"),
                      # the examples of the M3 pages (verifying, error messages, integer
                      # semantics): positives build with ordinary compilers too
                      Path("doc/modules/ROOT/examples/verifying"),
                      Path("doc/modules/ROOT/examples/error-messages"),
                      Path("doc/modules/ROOT/examples/integer-semantics"))
NEGATIVE_DIR = Path("tests/annotations/negative")
SCALAR_NEGATIVE_DIR = Path("tests/scalar/negative")
# Every directory of must-fail twins; a test under one needs `twin:` and must
# expect an error.
NEGATIVE_DIRS = (NEGATIVE_DIR, SCALAR_NEGATIVE_DIR)
# The twin rule of invariant 2 in the other direction, per area: every
# positive test under the first directory (outside the second) is the `twin:`
# of a test under the second, or says why not with `twin-exempt:`.
TWIN_AREAS = ((Path("tests/annotations"), NEGATIVE_DIR),
              (Path("tests/scalar"), SCALAR_NEGATIVE_DIR))
# In these areas a positive test needs a twin that expects an `error` (a runtime
# verification failure of the encoding rule it exercises): a `rejected` twin shows
# that a construct is refused, not that the rule is checked, so it does not count.
ERROR_TWIN_AREAS = (Path("tests/scalar"),)
_CERASTES_H_RE = re.compile(r'^[ \t]*#[ \t]*include[ \t]*[<"]cerastes\.h[>"]', re.MULTILINE)


def header_file(file: str | None, lineno: int, suffix: str) -> str | None:
    """The file of `@ <file>:L<n>` after its checks: C and C++ tests only, a
    repository-relative path without `..` or `.` components, normalized."""
    if file is None:
        return None
    if suffix == ".vpr":
        raise HeaderError(f"L{lineno}: `@ <file>:L<n>` does not apply to .vpr files")
    parts = file.split("/")
    if "" in parts or "." in parts or ".." in parts or file != os.path.normpath(file):
        raise HeaderError(f"L{lineno}: `{file}` must be a normalized repository-relative path "
                          "(no `..`, `.` or empty components)")
    return file


def parse_header(text: str, suffix: str) -> Expectation:
    """Parse the expectation header of a test file with the given suffix.

    This checks the syntax and the combination rules; the rules that need the
    file's place in the tree (the native-ok policy, `twin:`) are in
    read_header, and the diagnostic tags are validated by validate_diag_tags.
    """
    expects: list[tuple[int, str]] = []
    twins: list[tuple[int, str]] = []
    exempts: list[tuple[int, str]] = []
    fe_options_lines: list[tuple[int, str]] = []
    c_only_lines: list[tuple[int, str]] = []
    modes: list[str] = []
    in_header = True
    for lineno, raw in enumerate(text.splitlines(), start=1):
        line = raw.strip()
        if in_header and (not line or line.startswith("//")):
            if m := _EXPECT_RE.match(line):
                expects.append((lineno, m.group(1)))
            elif m := _MODE_RE.match(line):
                modes.extend(s.strip() for s in m.group(1).split(","))
            elif m := _TWIN_RE.match(line):
                twins.append((lineno, m.group(1)))
            elif m := _TWIN_EXEMPT_RE.match(line):
                exempts.append((lineno, m.group(1)))
            elif m := _FE_OPTIONS_RE.match(line):
                fe_options_lines.append((lineno, m.group(1)))
            elif m := _C_ONLY_RE.match(line):
                c_only_lines.append((lineno, m.group(1)))
            continue
        in_header = False
        if (_EXPECT_RE.match(line) or _MODE_RE.match(line) or _TWIN_RE.match(line)
                or _TWIN_EXEMPT_RE.match(line) or _FE_OPTIONS_RE.match(line)
                or _C_ONLY_RE.match(line)):
            raise HeaderError(f"L{lineno}: expect:/mode:/twin:/twin-exempt:/c-only:/"
                              "cerastes-fe-options: lines must be in the leading comment block")

    if not expects:
        raise HeaderError("no `// expect:` line in the leading comment block")

    errors: list[ExpectedError] = []
    diags: list[ExpectedDiag] = []
    kinds: set[str] = set()
    refused = None
    native_ok = False
    for lineno, body in expects:
        if body == "native-ok":
            if suffix == ".vpr":
                raise HeaderError(f"L{lineno}: `expect: native-ok` does not apply to "
                                  ".vpr files")
            if native_ok:
                raise HeaderError(f"L{lineno}: more than one `expect: native-ok` line")
            native_ok = True
        elif body in _SOLE_KINDS:
            kinds.add(body)
        elif m := _ERROR_RE.match(body):
            kinds.add("error")
            category, reason = m.group("category"), m.group("reason")
            if suffix == ".vpr":
                if category not in VIPER_CATEGORIES:
                    raise HeaderError(f"L{lineno}: unknown Viper error category "
                                      f"`{category}`")
                if reason is not None and reason not in VIPER_REASONS:
                    raise HeaderError(f"L{lineno}: unknown Viper error reason "
                                      f"`{reason}`")
            else:
                if category not in DRIVER_CATEGORIES:
                    raise HeaderError(
                        f"L{lineno}: unknown error category `{category}` for a C or C++ "
                        "test (known: " + ", ".join(sorted(DRIVER_CATEGORIES)) + ")")
                if reason is not None:
                    raise HeaderError(f"L{lineno}: `:{reason}` applies to .vpr tests only; "
                                      "the driver's errors have a category, not a Viper reason")
            errors.append(ExpectedError(category, reason, int(m.group("line")),
                                        header_file(m.group("file"), lineno, suffix)))
        elif m := _DIAG_RE.match(body):
            kinds.add("frontend-diag")
            diags.append(ExpectedDiag(m.group("severity"), m.group("tag"),
                                      int(m.group("line"))))
        elif m := _REFUSED_FE_RE.match(body):
            kinds.add("frontend-refused")
            if refused is not None:
                raise HeaderError(f"L{lineno}: more than one `frontend-refused` line")
            refused = m.group("substring")
        elif m := _REJECTED_RE.match(body):
            kinds.add("rejected")
            diags.append(ExpectedDiag("error", m.group("tag"), int(m.group("line")),
                                      header_file(m.group("file"), lineno, suffix)))
        else:
            raise HeaderError(f"L{lineno}: cannot parse `expect: {body}`")
    if not kinds:
        raise HeaderError("`expect: native-ok` accompanies an outcome (golden, "
                          "frontend-error, ...); it is not a test by itself")
    if len(kinds) != 1:
        raise HeaderError("expectations mix " + ", ".join(sorted(kinds)) +
                          "; a test has exactly one kind of outcome")
    (kind,) = kinds
    if kind in _SOLE_KINDS and len(expects) - native_ok != 1:
        raise HeaderError(f"`expect: {kind}` must be the only outcome line")

    twin = None
    if twins:
        if len(twins) != 1:
            raise HeaderError(f"L{twins[1][0]}: more than one `twin:` line")
        lineno, twin = twins[0]
        if suffix == ".vpr":
            raise HeaderError(f"L{lineno}: `twin:` does not apply to .vpr files")
        parts = Path(twin).parts
        if (not twin or " " in twin or twin.startswith("/") or ".." in parts
                or Path(twin).suffix not in TEST_SUFFIXES):
            raise HeaderError(f"L{lineno}: `twin: {twin}` must be a repository-relative "
                              "path of a test file (" + ", ".join(sorted(TEST_SUFFIXES))
                              + ") without `..`")

    fe_options: list[str] = []
    if fe_options_lines:
        if kind != "frontend-refused":
            raise HeaderError(f"L{fe_options_lines[0][0]}: `cerastes-fe-options:` is only valid "
                              "with `expect: frontend-refused`")
        if len(fe_options_lines) != 1:
            raise HeaderError(f"L{fe_options_lines[1][0]}: more than one `cerastes-fe-options:` "
                              "line")
        lineno, text = fe_options_lines[0]
        try:
            fe_options = shlex.split(text)
        except ValueError as err:
            raise HeaderError(f"L{lineno}: cannot read the options: {err}") from None
        if not fe_options or any(not o.startswith("-") for o in fe_options):
            raise HeaderError(f"L{lineno}: `cerastes-fe-options:` takes one or more options "
                              "(each starting with `-`; the test file is added by the harness)")
    if kind == "frontend-refused":
        if not fe_options_lines:
            raise HeaderError("`expect: frontend-refused` needs a `// cerastes-fe-options:` line")
        if native_ok:
            raise HeaderError("`expect: frontend-refused` cannot be combined with `native-ok`")
        if modes:
            raise HeaderError("`mode:` does not apply to `expect: frontend-refused` (the "
                              "options replace the mode option)")
        if suffix == ".vpr":
            raise HeaderError("`expect: frontend-refused` does not apply to .vpr files")

    twin_exempt = None
    if exempts:
        if len(exempts) != 1:
            raise HeaderError(f"L{exempts[1][0]}: more than one `twin-exempt:` line")
        lineno, twin_exempt = exempts[0]
        if suffix == ".vpr":
            raise HeaderError(f"L{lineno}: `twin-exempt:` does not apply to .vpr files")
        if not twin_exempt:
            raise HeaderError(f"L{lineno}: `twin-exempt:` needs a reason (why no test of "
                              "tests/annotations/negative/ derives from this one)")
        if twin is not None:
            raise HeaderError(f"L{lineno}: `twin-exempt:` and `twin:` contradict each other")
        if kind not in POSITIVE_KINDS:
            raise HeaderError(f"L{lineno}: `twin-exempt:` applies to positive tests only "
                              "(" + " or ".join(POSITIVE_KINDS) + ")")

    c_only = None
    if c_only_lines:
        lineno, c_only = c_only_lines[0]
        if len(c_only_lines) != 1:
            raise HeaderError(f"L{c_only_lines[1][0]}: more than one `c-only:` line")
        if suffix != ".c":
            raise HeaderError(f"L{lineno}: `c-only:` applies to .c files only")
        if not c_only:
            raise HeaderError(f"L{lineno}: `c-only:` needs a reason (why the test does not run "
                              "in c++20)")
        if "c++20" in modes:
            raise HeaderError(f"L{lineno}: `c-only:` contradicts `mode:` listing c++20")

    if suffix == ".vpr":
        if modes:
            raise HeaderError("`mode:` does not apply to .vpr files")
        if kind not in ("verified", "error"):
            raise HeaderError(f"`expect: {kind}` does not apply to .vpr files")
    else:
        for mode in modes:
            if mode not in MODE_OPTIONS:
                raise HeaderError(f"unknown mode `{mode}` (known: "
                                  + ", ".join(MODE_OPTIONS) + ")")
        if len(set(modes)) != len(modes):
            raise HeaderError("a mode is listed twice")
        modes = [] if kind == "frontend-refused" else (modes or DEFAULT_MODES[suffix])
    return Expectation(kind, errors, modes, diags, native_ok, twin, twin_exempt,
                       refused, fe_options, c_only)


def _under(path: Path, directory: Path) -> bool:
    """Whether `path` is inside `directory` (relative to ROOT)."""
    try:
        path.resolve().relative_to((ROOT / directory).resolve())
    except ValueError:
        return False
    return True


# `.c` tests under these directories run in both languages (c23 as C, c++20 as C++)
# unless they carry `// c-only: <reason>`.
DUAL_MODE_DIRS = (Path("tests/scalar"),
                  Path("doc/modules/ROOT/examples/verifying"),
                  Path("doc/modules/ROOT/examples/error-messages"),
                  Path("doc/modules/ROOT/examples/integer-semantics"))


def policy_problem(path: Path, text: str, exp: Expectation) -> str | None:
    """The native-ok policy: positive tests of the annotation layer, and every
    positive test that includes <cerastes.h>, must compile with ordinary
    compilers (`// expect: native-ok`); `.c` tests of DUAL_MODE_DIRS run in both
    modes or say why not.  None when `path` complies."""
    if path.suffix == ".c" and exp.c_only is None and exp.kind != "frontend-refused":
        for directory in DUAL_MODE_DIRS:
            if _under(path, directory) and set(exp.modes) != {"c23", "c++20"}:
                return (f"a .c test under {directory}/ must run in both modes "
                        "(`// mode: c23, c++20`: the code is valid C and C++) or carry "
                        "`// c-only: <reason>`")
    if exp.kind == "frontend-ok" and path.suffix != ".vpr":
        # stock cpfe does not define __CERASTES__, so the annotations in such a
        # test are not even parsed: it would pass for nothing
        for directory in NATIVE_POLICY_DIRS:
            if _under(path, directory):
                return (f"`expect: frontend-ok` is not allowed under {directory}/: stock cpfe "
                        "does not define __CERASTES__, so the annotations are not parsed "
                        "(use golden, frontend-error or frontend-warning)")
    if exp.kind not in POSITIVE_KINDS or exp.native_ok or path.suffix == ".vpr":
        return None
    for directory in NATIVE_POLICY_DIRS:
        if _under(path, directory):
            return (f"a positive test under {directory}/ must carry "
                    "`// expect: native-ok` (it has to build with ordinary compilers)")
    if _CERASTES_H_RE.search(text):
        return ("a positive test that includes <cerastes.h> must carry "
                "`// expect: native-ok` (it has to build with ordinary compilers)")
    return None


def expects_failure(exp: Expectation) -> bool:
    """Whether the test expects at least one error (not only warnings)."""
    return (exp.kind in ("error", "rejected", "frontend-refused")
            or (exp.kind == "frontend-diag" and any(d.severity == "error" for d in exp.diags)))


def twin_scan(area: Path, negative_dir: Path):
    """(positives, named) for an area: the positive tests that need a twin, with
    their modes, and {positive path: [(kind, modes) of the tests under negative_dir
    that name it]}."""
    base = ROOT / area
    positives: list[tuple[Path, list[str]]] = []
    named: dict[Path, list[tuple[str, list[str]]]] = {}
    for f in sorted(base.rglob("*")):
        if f.suffix not in TEST_SUFFIXES or not f.is_file():
            continue
        try:
            exp = parse_header(f.read_text(), f.suffix)
        except HeaderError:
            continue  # reported by its own header check
        if _under(f, negative_dir):
            if exp.twin:
                named.setdefault((ROOT / exp.twin).resolve(), []).append((exp.kind, exp.modes))
        elif exp.kind in POSITIVE_KINDS and exp.twin_exempt is None:
            positives.append((f, exp.modes))
    return positives, named


def uncovered_modes(modes: list[str], twins: list[tuple[str, list[str]]]) -> list[str]:
    """The modes of a positive test in which no `error` twin runs: an error twin
    that runs in c23 only says nothing about the c++20 run of its positive."""
    return [m for m in modes if not any(kind == "error" and m in tmodes for kind, tmodes in twins)]


def cross_file_problems(areas: list[tuple[Path, Path]] | None = None
                        ) -> list[tuple[Path, str]]:
    """The twin rule in the other direction (invariant 2): every positive test
    under tests/annotations/ (and under tests/scalar/) is named by the `twin:`
    of a test under that area's negative/ directory, or says why not with
    `// twin-exempt: <reason>`; in ERROR_TWIN_AREAS one of those twins expects an
    `error` and, for each mode the positive runs in, one of them runs in that mode.
    Looks at the whole tree of each area, whatever was selected; `areas`
    (default: TWIN_AREAS) picks the areas to look at."""
    problems: list[tuple[Path, str]] = []
    for area, negative_dir in (TWIN_AREAS if areas is None else areas):
        if not (ROOT / area).is_dir():
            continue
        positives, named = twin_scan(area, negative_dir)
        need_error = area in ERROR_TWIN_AREAS
        for f, modes in positives:
            twins = named.get(f.resolve(), [])
            if need_error:
                missing = uncovered_modes(modes, twins)
                if not missing:
                    continue
                if not twins:
                    why = ""
                elif not any(kind == "error" for kind, _ in twins):
                    why = "; it has only a `rejected` twin"
                else:
                    why = ("; no `error` twin runs in " + ", ".join(missing)
                           + " (a twin that runs in one mode says nothing about the other)")
                problems.append((f, f"a positive test under {area}/ must be the `twin:` of a test "
                                    f"under {negative_dir}/ that expects an `error` (a `rejected` "
                                    "twin does not count) (invariant 2), or carry "
                                    "`// twin-exempt: <reason>`" + why))
            elif not twins:
                problems.append((f, f"a positive test under {area}/ must be the `twin:` of a test "
                                    f"under {negative_dir}/ (invariant 2), or carry "
                                    "`// twin-exempt: <reason>`"))
    return problems


def count_error_twins(area: Path = Path("tests/scalar"),
                      negative_dir: Path = SCALAR_NEGATIVE_DIR) -> int:
    """The number of positive tests of `area` that have an `error` twin in each of
    their modes: the pairs of the acceptance count (a `rejected` twin is not one)."""
    positives, named = twin_scan(area, negative_dir)
    return sum(1 for f, modes in positives
               if not uncovered_modes(modes, named.get(f.resolve(), [])))


# What would neutralise the ordinary-compiler check from inside the file: a
# pragma that ignores or downgrades diagnostics (or marks the file a system
# header), and defining __CERASTES__ (`#undef` stays fine: it is what the
# header test of the off switch does).
_NEUTRALISE_RE = re.compile(
    r"^[ \t]*#[ \t]*(?:pragma[ \t]+(?:(?:clang|GCC)[ \t]+diagnostic[ \t]+(?:ignored|warning)\b"
    r"|(?:clang|GCC)[ \t]+system_header\b)|define[ \t]+__CERASTES__\b)", re.MULTILINE)


def native_neutralised(text: str) -> str | None:
    """The offending line of a native-ok test that switches its own check off."""
    m = _NEUTRALISE_RE.search(text)
    if not m:
        return None
    end = text.find("\n", m.end())
    return text[m.start():None if end < 0 else end].strip()


def twin_problem(path: Path, exp: Expectation) -> str | None:
    """The `twin:` rules: required under tests/annotations/negative/, and the
    twin must be an existing test outside it with a positive outcome."""
    negative = next((d for d in NEGATIVE_DIRS if _under(path, d)), None)
    if negative is not None:
        if not expects_failure(exp):
            return (f"a test under {negative}/ must expect at least one error "
                    "(`frontend-error`, `error` or `rejected`); `golden`, `verified`, "
                    "`frontend-ok` and warning-only tests are not must-fail tests")
    if exp.twin is None:
        if negative is not None:
            return (f"a test under {negative}/ needs `// twin: <path>` naming the "
                    "positive test it was derived from")
        return None
    target = ROOT / exp.twin
    if not target.is_file():
        return f"`twin: {exp.twin}` does not exist (a dangling twin)"
    if target.resolve() == path.resolve():
        return "a test cannot be its own twin"
    if (inside := next((d for d in NEGATIVE_DIRS if _under(target, d)), None)) is not None:
        return (f"`twin: {exp.twin}` is itself under {inside}/; a twin is the "
                "positive test, outside it")
    twin_exp = read_header(target)
    if isinstance(twin_exp, HeaderError):
        return f"`twin: {exp.twin}` has a bad header: {twin_exp}"
    if twin_exp.kind not in POSITIVE_KINDS:
        return (f"`twin: {exp.twin}` is not a positive test (its outcome is "
                f"`{twin_exp.kind}`; need " + " or ".join(POSITIVE_KINDS) + ")")
    return None


def read_header(path: Path) -> Expectation | HeaderError:
    """The expectation of a test file, or the error that makes its header bad."""
    try:
        text = path.read_text()
        exp = parse_header(text, path.suffix)
        neutralised = native_neutralised(text) if exp.native_ok else None
        if neutralised:
            raise HeaderError(f"a native-ok test cannot switch its own check off: "
                              f"`{neutralised}` (a diagnostic pragma or a definition of "
                              "__CERASTES__)")
        for problem in (twin_problem(path, exp), policy_problem(path, text, exp)):
            if problem:
                raise HeaderError(problem)
        return exp
    except HeaderError as err:
        return err


# ---------------------------------------------------------------------------
# Silicon output
# ---------------------------------------------------------------------------

# Viper error kinds, from silver's viper/silver/verifier/VerificationError.scala
# at the pinned commit: (message prefix, Viper error id, harness category).
# Silicon's command line prints only the readable message, so the kind is
# recovered from the fixed text each error class starts with.
_VIPER_ERRORS = [
    (r"Assignment might fail\.", "assignment.failed", "assignment"),
    (r"Method call might fail\.", "call.failed", "call"),
    (r"Contract might not be well-formed\.", "not.wellformed", "wellformedness"),
    (r"The precondition of method \S+ might not hold\.", "call.precondition",
     "precondition"),
    (r"Precondition of function \S+ might not hold\.", "application.precondition",
     "precondition"),
    (r"Exhale might fail\.", "exhale.failed", "exhale"),
    (r"Inhale might fail\.", "inhale.failed", "inhale"),
    (r"Conditional statement might fail\.", "if.failed", "if"),
    (r"While statement might fail\.", "while.failed", "while"),
    (r"Assert might fail\.", "assert.failed", "assert"),
    # silver's termination plugin (TerminationErrors.scala), pinned by tests/viper-smoke/termination_kinds.vpr:
    # the function form has no name in the text; the method and loop forms are the ones decision 0013 maps
    (r"Function (?:\S+ )?might not terminate\.", "termination.failed", "termination"),
    (r"Method might not terminate\.", "termination.failed", "termination"),
    (r"Loop might not terminate\.", "termination.failed", "termination"),
    (r"Postcondition of \S+ might not hold\.", "postcondition.violated",
     "postcondition"),
    (r"Folding .+? might fail\.", "fold.failed", "fold"),
    (r"Unfolding .+? might fail\.", "unfold.failed", "unfold"),
    (r"Packaging wand might fail\.", "package.failed", "package"),
    (r"Applying wand might fail\.", "apply.failed", "apply"),
    (r"Loop invariant .+? might not be preserved\.", "invariant.not.preserved",
     "invariant-preserved"),
    (r"Loop invariant .+? might not hold on entry\.", "invariant.not.established",
     "invariant-entry"),
    (r"Function might not be well-formed\.", "function.not.wellformed",
     "wellformedness"),
    (r"Predicate might not be well-formed\.", "predicate.not.wellformed",
     "wellformedness"),
    (r"Magic wand might not be well-formed\.", "wand.not.wellformed",
     "wellformedness"),
    (r"Referencing a wand might fail\.", "letwand.failed", "letwand"),
    (r"Quasihavoc might fail\.", "quasihavoc.failed", "quasihavoc"),
    (r"Quasihavocall might fail\.", "quasihavocall.failed", "quasihavoc"),
    (r"Applying heuristics failed\.", "heuristics.failed", "heuristics"),
    (r"An internal error occurred\.", "internal", "internal"),
]

# Viper error reasons: (Viper reason id, message pattern).
_VIPER_REASONS = [
    ("assertion.false", r"Assertion .+ might not hold\."),
    ("receiver.null", r"Receiver of .+ might be null\."),
    ("division.by.zero", r"Divisor .+ might be zero\."),
    ("negative.permission", r"Fraction .+ might be negative\."),
    ("permission.not.positive", r"Fraction .+ might not be positive\."),
    ("insufficient.permission", r"There might be insufficient permission to access .+"),
    ("invalid.perm.multiplication", r"Permission multiplication might not be possible.*"),
    ("wand.not.found", r"Magic wand instance not found\."),
    ("qp.not.injective", r"Quantified resource .+ might not be injective\."),
    ("quasihavocall.not.injective",
     r"Quasihavocall statement .+ might not be injective\."),
    ("labelled.state.not.reached", r"Did not reach labelled state .+"),
    ("seq.index.negative", r"Index .+ into .+ might be negative\."),
    ("seq.index.length", r"Index .+ into .+ might exceed sequence length\."),
    ("map.key.contains", r"Map .+ might not contain an entry at key .+"),
    ("epsilon.as.param", r"The parameter .+ might be an epsilon permission.*"),
    ("feature.unsupported", r".+ is not supported\..*"),
    ("unexpected.node", r".+ occurred unexpectedly\..*"),
    # termination (TerminationErrors.scala): the three messages of TupleSimpleFalse, TupleDecreasesFalse and
    # TupleBoundedFalse (sic: "might not bounded"), TupleConditionFalse, TerminationConditionFalse
    ("tuple.false", r"Termination measure might not (?:decrease or might not be bounded|decrease|bounded)\."),
    ("tuple.condition.false", r"Required tuple condition might not hold\."),
    ("termination.condition.false", r"Required termination condition might not hold\."),
]

VIPER_CATEGORIES = {category for _, _, category in _VIPER_ERRORS}
VIPER_REASONS = {reason for reason, _ in _VIPER_REASONS}

# Never an acceptable outcome, whatever the expectation says.
UNKNOWN = "unknown"
TIMEOUT = "timeout"

_ERROR_KINDS = [(re.compile(text + r"\s+(?P<rest>.*)", re.DOTALL), viper_id, category)
                for text, viper_id, category in _VIPER_ERRORS]
_REASON_KINDS = [(reason, re.compile(pattern, re.DOTALL))
                 for reason, pattern in _VIPER_REASONS]
_TIMED_OUT_RE = re.compile(r"\s*An SMT query timed out after \d+ms\.$")
_POSITION_RE = re.compile(
    r"\s*\((?P<file>[^()@]+)@(?P<line>\d+)\.(?P<col>\d+)"
    r"(?:--(?P<eline>\d+)\.(?P<ecol>\d+))?\)\s*$")
_ENTRY_RE = re.compile(r"^\s*\[\s*(\d+)\] (.*)$")


@dataclasses.dataclass(frozen=True)
class ReportedError:
    category: str
    reason: str
    line: int
    file: str
    message: str

    def __str__(self) -> str:
        reason = f":{self.reason}" if self.reason else ""
        return f"{self.category}{reason} @ L{self.line}: {self.message}"


@dataclasses.dataclass
class SiliconResult:
    verified: bool
    errors: list[ReportedError]
    problem: str | None  # set when the output is not a clean verdict


def classify(message: str) -> tuple[str, str, str | None, int | None]:
    """Split one Silicon error message into (category, reason, file, line).

    Messages that are not Viper verification errors (parse and type errors,
    crashes) get category `unknown`; solver timeouts get reason `timeout`.
    """
    file = line = None
    if m := _POSITION_RE.search(message):
        file, line = m.group("file"), int(m.group("line"))
        message = message[:m.start()]
    message = message.strip()
    for regex, _viper_id, category in _ERROR_KINDS:
        if m := regex.fullmatch(message):
            rest = m.group("rest")
            if _TIMED_OUT_RE.search(rest):
                return category, TIMEOUT, file, line
            for reason, reason_re in _REASON_KINDS:
                if reason_re.fullmatch(rest):
                    return category, reason, file, line
            return category, UNKNOWN, file, line
    return UNKNOWN, UNKNOWN, file, line


_BANNER_RE = re.compile(r"^Silicon \S+( \(.*\))?\s*$")
_SUCCESS_LINE_RE = re.compile(r"^Silicon finished verification successfully\b.*$")
_FOUND_LINE_RE = re.compile(r"^Silicon found (\d+) errors? in .*:\s*$")


def parse_silicon_output(output: str, returncode: int) -> SiliconResult:
    """Silicon's output as a verdict.  The output is accepted only if it is the banner, exactly one verdict
    line and its error list, nothing else: a warning, a stack trace or a second verdict means that something
    else happened and is "no verdict" (decision 0011 item 11; src/driver/silicon_output.cpp reads it the
    same way, and the self-test requires the two to agree)."""
    entries: list[str] = []
    stray = ""  # the first line that is neither part of a verdict nor of an error message
    banners = successes = founds = 0
    found_count = ""
    for raw in output.splitlines():
        if m := _ENTRY_RE.match(raw):
            entries.append(m.group(2))
        elif not raw.strip():
            continue
        elif _SUCCESS_LINE_RE.match(raw):
            successes += 1
        elif m := _FOUND_LINE_RE.match(raw):
            founds += 1
            found_count = m.group(1)
        elif _BANNER_RE.match(raw):
            banners += 1
        elif entries:
            entries[-1] += "\n" + raw.strip()  # the rest of a multi-line message
        elif not stray:
            stray = raw

    errors = []
    for entry in entries:
        category, reason, file, line = classify(entry)
        errors.append(ReportedError(category, reason, line or 0, file or "?",
                                    " ".join(entry.split())))

    no_verdict = f"Silicon did not produce a verdict (exit status {returncode})"
    if successes + founds == 0:
        return SiliconResult(False, errors, no_verdict)
    if stray or banners != 1 or successes + founds != 1:
        unexpected = f" (unexpected line: {stray[:120]})" if stray else ""
        return SiliconResult(False, errors, "Silicon's output is not a banner and one verdict"
                             f"{unexpected} (exit status {returncode})")
    if successes == 1 and not entries and returncode == 0:
        return SiliconResult(True, [], None)
    problem = None
    if founds != 1 or returncode != 1:
        problem = no_verdict
    elif int(found_count) != len(errors):
        problem = f"Silicon reported {found_count} errors but {len(errors)} were parsed"
    return SiliconResult(False, errors, problem)


def same_file(a: str | None, b: str | None) -> bool:
    return a is not None and b is not None and os.path.normpath(a) == os.path.normpath(b)


def compare_errors(expected: list[ExpectedError], reported: list[ReportedError],
                   test_file: str) -> list[str]:
    """Return mismatch descriptions; empty when the outcome is exactly right.

    An expected error is in the test file unless it names a header
    (`@ <file>:L<n>`); an error in any other file is a failure."""
    problems = []
    remaining = list(reported)
    allowed = {test_file} | {e.file for e in expected if e.file}
    for r in reported:
        if not any(same_file(r.file, f) for f in allowed):
            problems.append(f"error reported in another file ({r.file}): {r}")
            remaining.remove(r)
        elif r.category == UNKNOWN or r.reason in (UNKNOWN, TIMEOUT):
            what = "solver timeout" if r.reason == TIMEOUT else "unrecognized error"
            problems.append(f"{what}: {r.message}")
            remaining.remove(r)

    def where(e: ExpectedError) -> str:
        return e.file or test_file

    def matches(e: ExpectedError, r: ReportedError) -> bool:
        return (e.category == r.category and e.line == r.line and same_file(r.file, where(e))
                and (e.reason is None or e.reason == r.reason))

    unmatched = []
    for e in expected:
        hit = next((r for r in remaining if matches(e, r)), None)
        if hit is None:
            unmatched.append(e)
        else:
            remaining.remove(hit)

    # Pair leftovers to say *how* an expected error was missed.
    for e in unmatched:
        in_file = [r for r in remaining if same_file(r.file, where(e))]
        same_kind = next((r for r in in_file if r.category == e.category
                          and (e.reason is None or e.reason == r.reason)), None)
        same_line = next((r for r in in_file if r.line == e.line), None)
        if same_kind is not None:
            problems.append(f"wrong line: expected {e}, got {same_kind}")
            remaining.remove(same_kind)
        elif same_line is not None:
            problems.append(f"wrong reason: expected {e}, got {same_line}")
            remaining.remove(same_line)
        else:
            problems.append(f"missing error: expected {e}")
    for r in remaining:
        problems.append(f"unexpected error: {r}")
    return problems


# ---------------------------------------------------------------------------
# Golden files
# ---------------------------------------------------------------------------

def golden_path(test: Path, mode: str, modes: list[str]) -> Path:
    """The golden file of `test` in `mode`, given all the test's modes."""
    suffix = GOLDEN_SUFFIX if len(modes) == 1 else f".{mode}{GOLDEN_SUFFIX}"
    return test.with_name(test.name + suffix)


def golden_text(stdout: str, stderr: str) -> str:
    """The text a golden file holds for one front-end run.

    stdout as is; if stderr is not empty, a `--- stderr ---` line and stderr.
    A missing final newline on stdout is made explicit rather than added, so
    that it still shows up as a difference.
    """
    text = stdout
    if stderr:
        if text and not text.endswith("\n"):
            text += "\n" + NO_NEWLINE_MARKER + "\n"
        text += STDERR_MARKER + "\n" + stderr
    return text


def _split_lines(text: str) -> list[str]:
    """Split on "\\n" only, keeping the line ends (so "\\r" stays visible)."""
    parts = text.split("\n")
    lines = [p + "\n" for p in parts[:-1]]
    if parts[-1]:
        lines.append(parts[-1])
    return lines


_TRAILING_WS_RE = re.compile(r"[ \t\r\f\v]+$")
_WS_NAMES = {" ": "<SP>", "\t": "<TAB>", "\r": "<CR>", "\f": "<FF>", "\v": "<VT>"}


def _show(line: str) -> str:
    """A diff line, printable, with whitespace at its end made visible."""
    if m := _TRAILING_WS_RE.search(line):
        line = line[:m.start()] + "".join(_WS_NAMES[c] for c in m.group())
    return printable(line)


def printable(text: str) -> str:
    """`text` with undecodable bytes (surrogate escapes) shown as escapes."""
    return text.encode("utf-8", "backslashreplace").decode("utf-8")


def compare_golden(expected: str, actual: str, golden_name: str = "golden") -> list[str]:
    """Return mismatch descriptions; empty when `actual` is exactly `expected`.

    The first entry says what differs; the others are a unified diff from the
    golden file (-) to the actual output (+).
    """
    if expected == actual:
        return []

    def normalized(text: str) -> list[str]:
        return [line.rstrip() for line in text.rstrip().split("\n")]

    summary = f"output differs from {golden_name} (- golden, + actual)"
    if normalized(expected) == normalized(actual):
        summary += "; only in whitespace at line ends or at the end of the file"
    problems = [summary + ":"]
    for line in difflib.unified_diff(_split_lines(expected), _split_lines(actual),
                                     f"{golden_name} (golden)", "actual output"):
        if line.endswith("\n"):
            problems.append(_show(line[:-1]))
        else:
            problems.append(_show(line))
            problems.append("\\ No newline at end of file")
    return problems


@dataclasses.dataclass
class ToolRun:
    """The result of running a tool once."""
    returncode: int | None  # None: it did not run to completion (see error)
    stdout: str = ""
    stderr: str = ""
    error: str | None = None  # why it did not run to completion


def run_problems(run: ToolRun) -> list[str]:
    """Problems with a front-end run itself: it did not finish, it failed, or
    it printed nothing (an empty golden test would pass vacuously)."""
    if run.returncode is None:
        return [f"cerastes-fe {run.error}"]
    problems = []
    if run.returncode < 0:
        problems.append(f"cerastes-fe crashed (signal {-run.returncode})")
    elif run.returncode != 0:
        problems.append(f"cerastes-fe exited with status {run.returncode}")
    if not golden_text(run.stdout, run.stderr):
        problems.append("cerastes-fe printed nothing; an empty golden test would "
                        "pass vacuously")
    if not walk_finished(run.stdout):
        problems.append("walker did not finish: stdout does not end with an "
                        "`end walk: <R> routines reported, <U> routines with bodies "
                        "in non-system files outside the primary file` line")
    return problems


def walk_finished(stdout: str) -> bool:
    """Whether the last line of stdout is the walker's `end walk:` line."""
    lines = stdout.split("\n")
    return (len(lines) >= 2 and lines[-1] == ""
            and END_WALK_RE.fullmatch(lines[-2]) is not None)


def evaluate_golden(run: ToolRun, expected: str | None, golden_name: str) -> list[str]:
    """Problems with one golden-test run; empty when it passes.

    `expected` is the golden file's text, or None if the file is missing.  A
    failed run fails even if its output matches the golden file.
    """
    problems = run_problems(run)
    if run.returncode is None:
        return problems
    if expected is None:
        problems.append(f"missing golden file {golden_name}; review the output "
                        "(-v) and create it with --update-golden")
    else:
        problems += compare_golden(expected, golden_text(run.stdout, run.stderr),
                                   golden_name)
    return problems


def read_exact(path: Path) -> str:
    """A file's text, byte for byte (no newline translation, any bytes)."""
    return path.read_bytes().decode("utf-8", "surrogateescape")


def write_exact(path: Path, text: str) -> None:
    path.write_bytes(text.encode("utf-8", "surrogateescape"))


def find_orphan_goldens(goldens: list[Path],
                        headers: dict[Path, Expectation | HeaderError]) -> list[Path]:
    """The golden files that no golden test among `headers` uses.

    Goldens next to a test whose header is bad are not called orphans: the
    header error is reported instead.
    """
    claimed: set[Path] = set()
    for test, exp in headers.items():
        if isinstance(exp, Expectation):
            if exp.kind == "golden":
                claimed |= {golden_path(test, m, exp.modes).resolve() for m in exp.modes}
        else:
            parent = test.parent.resolve()
            claimed |= {g.resolve() for g in goldens
                        if g.parent.resolve() == parent
                        and g.name.startswith(test.name + ".")}
    return [g for g in goldens if g.resolve() not in claimed]


ORPHAN_MESSAGE = ("orphan golden file: no golden test uses it (a renamed or "
                  "removed test, a changed mode list or expectation?); delete it "
                  "or restore its test")


# ---------------------------------------------------------------------------
# Front-end diagnostics (`frontend-error` / `frontend-warning`)
# ---------------------------------------------------------------------------

# The options of a diagnostics test, after the mode option.  Never
# --no_code_gen: the cerastes back end has to run, or its diagnostics (future
# `#cerastes-<tag>` ones) could never appear.
DIAG_OPTIONS = ["--brief_diagnostics", "--display_error_number", "--no_colors"]
# Where the back end's diagnostics are declared once there are any (ROOT
# relative).  Until the file exists, a `cerastes-` tag is a header error.
CERASTES_DIAGNOSTICS_DEF = Path("src/backend/diagnostics.def")
# EDG's exit status for a compilation with errors; with only warnings it is 0.
EDG_ERRORS_STATUS = 2
# A refusal of the cerastes driver (src/driver/main.cpp), before EDG runs: this
# prefix, one line on stderr, exit status 2.  EDG's own command-line error is the
# other accepted shape: three lines and exit status 4 (REFUSED_EDG_LINES).
REFUSAL_PREFIX = "cerastes-fe: error: "
REFUSAL_STATUS = 2
# `Command-line error #N: message` with --display_error_number, else `Command-line error: message`
EDG_CMDLINE_RE = re.compile(r"^Command-line error(?: #\d+)?: \S")
EDG_CMDLINE_STATUS = 4
EDG_CMDLINE_TRAILER = ("1 catastrophic error detected in this compilation.",
                       "Compilation terminated.")

_CODES_RE = re.compile(r"^\s*(ec_\w+) /\* = (\d+) \*/", re.MULTILINE)
_TAGS_START_RE = re.compile(r"error_tags\[NUMBER_OF_ERROR_TAGS\]")
_TAG_ENTRY_RE = re.compile(r'^\s*\{ "([^"]+)", (ec_\w+) \}', re.MULTILINE)
_TAG_COUNT_RE = re.compile(r"^#define NUMBER_OF_ERROR_TAGS (\d+)", re.MULTILINE)


@dataclasses.dataclass
class TagRegistry:
    """EDG's diagnostic tags and numbers, read from the generated headers.

    err_data.h has the tag table `{ "tag", ec_name }` (NUMBER_OF_ERROR_TAGS
    entries), err_codes.h the number of each `ec_name /* = N */`.  The headers
    are generated from the EDG tree's own error_msg.txt, so they must come from
    the stock build of the *active* tree ($CERASTES_EDG_GEN, decision 0005).
    """
    numbers: dict[str, int]  # tag -> number
    tags: dict[int, str]  # number -> tag

    @classmethod
    def parse(cls, codes_text: str, data_text: str) -> "TagRegistry":
        codes = {name: int(n) for name, n in _CODES_RE.findall(codes_text)}
        if not codes:
            raise UsageError("err_codes.h has no `ec_name /* = N */` entries")
        start = _TAGS_START_RE.search(data_text)
        count = _TAG_COUNT_RE.search(data_text)
        if not start or not count:
            raise UsageError("err_data.h has no error_tags table (the generated "
                             "header's format changed?)")
        entries = _TAG_ENTRY_RE.findall(data_text, start.end())
        if len(entries) != int(count.group(1)):
            raise UsageError(f"err_data.h declares {count.group(1)} error tags but "
                             f"{len(entries)} were parsed (the format changed?)")
        numbers: dict[str, int] = {}
        for tag, name in entries:
            if name not in codes:
                raise UsageError(f"err_data.h tag `{tag}` names {name}, which "
                                 "err_codes.h does not define")
            if tag in numbers:
                raise UsageError(f"err_data.h lists the tag `{tag}` twice")
            numbers[tag] = codes[name]
        return cls(numbers, {n: t for t, n in numbers.items()})

    @classmethod
    def load(cls, gen_dir: str | Path) -> "TagRegistry":
        gen = Path(gen_dir)
        try:
            return cls.parse(read_exact(gen / "err_codes.h"), read_exact(gen / "err_data.h"))
        except OSError as err:
            raise UsageError(f"cannot read EDG's generated headers in {gen} ({err}); "
                             "run scripts/bootstrap.sh") from err


_registry_cache: dict[tuple[str, int, int], TagRegistry] = {}


def load_registry(env: dict[str, str]) -> TagRegistry:
    """The tag registry of the active EDG build (`CERASTES_EDG_GEN`)."""
    gen = env.get("CERASTES_EDG_GEN")
    if not gen:
        raise UsageError("CERASTES_EDG_GEN not set; run scripts/bootstrap.sh first")
    try:
        key = (gen, (Path(gen) / "err_data.h").stat().st_mtime_ns,
               (Path(gen) / "err_codes.h").stat().st_mtime_ns)
    except OSError:
        return TagRegistry.load(gen)  # raises the usage error
    if key not in _registry_cache:
        _registry_cache[key] = TagRegistry.load(gen)
    return _registry_cache[key]


def load_cerastes_tags(path: Path | None = None) -> set[str] | None:
    """The back end's diagnostic tags, from src/backend/diagnostics.def (lines
    `MACRO(tag, ...)`, the tag a bare or quoted lower-case word); None when the
    file does not exist yet."""
    path = ROOT / CERASTES_DIAGNOSTICS_DEF if path is None else path
    if not path.is_file():
        return None
    return set(re.findall(r'^[ \t]*[A-Z][A-Z0-9_]*\([ \t]*"?([a-z][a-z0-9_-]*)"?[ \t]*[,)]',
                          read_exact(path), re.MULTILINE))


def validate_diag_tags(exp: Expectation, registry: TagRegistry,
                       cerastes_tags: set[str] | None) -> HeaderError | None:
    """The header error for the first unknown diagnostic tag of `exp`, or None."""
    for d in exp.diags:
        if d.tag.startswith(CERASTES_TAG_PREFIX):
            if cerastes_tags is None:
                return HeaderError(
                    f"unknown diagnostic `{d.tag}`: no cerastes diagnostics are defined "
                    f"yet ({CERASTES_DIAGNOSTICS_DEF} does not exist)")
            if d.tag[len(CERASTES_TAG_PREFIX):] not in cerastes_tags:
                return HeaderError(f"unknown cerastes diagnostic `{d.tag}` (not in "
                                   f"{CERASTES_DIAGNOSTICS_DEF})")
        elif d.tag not in registry.numbers:
            close = difflib.get_close_matches(d.tag, registry.numbers, n=3)
            hint = f"; did you mean {', '.join(close)}?" if close else ""
            return HeaderError(f"unknown EDG diagnostic tag `{d.tag}` (not in "
                               f"err_data.h){hint}")
    return None


def check_diag_headers(headers: dict[Path, "Expectation | HeaderError"],
                       env: dict[str, str]) -> None:
    """Replace, in `headers`, the expectations with an unknown diagnostic tag by
    the header error.  Needs the generated headers only if a test has diags."""
    needing = [f for f, e in headers.items() if isinstance(e, Expectation) and e.diags]
    if not needing:
        return
    registry = load_registry(env)
    cerastes_tags = load_cerastes_tags()
    for f in needing:
        err = validate_diag_tags(headers[f], registry, cerastes_tags)  # type: ignore[arg-type]
        if err is not None:
            headers[f] = err


# A primary diagnostic line starts in column 0:
#   "file", line N (col. C): error #NNNN[-D]: message
#   Command-line error #NNN: message       (no position)
# Back-end diagnostics (future) print `#cerastes-<tag>` in the id position.
_FE_DIAG_RE = re.compile(
    r'^(?:"(?P<file>.+?)", line (?P<line>\d+)(?: \(col\. (?P<col>\d+)\))?: )?'
    r'(?P<severity>(?i:catastrophic error|command-line error|internal error|error|'
    r'warning|remark|note)) '
    r'#(?P<id>\d+|cerastes-[a-z0-9_]+(?:-[a-z0-9_]+)*)(?P<disc>-D)?: (?P<message>.*)$')
_FE_SUMMARY_RE = re.compile(
    r'^(?:(?P<errors>\d+) errors?(?: and (?P<cat_too>\d+) catastrophic errors?)?'
    r'|(?P<cats>\d+) catastrophic errors?) detected in '
    r'(?:the compilation of ".*"|this compilation)\.$')
_FE_TERMINATORS = {
    "Compilation terminated.": "the front end terminated (catastrophic or command-line error)",
    "Compilation aborted.": "the front end aborted (internal error)",
    "Error limit reached.": "the front end stopped at its error limit; the list is incomplete",
}
# Severities (lower case) that fail a test whatever it expects.
_FE_FATAL = ("catastrophic error", "command-line error", "internal error")


@dataclasses.dataclass
class FeDiag:
    severity: str  # error, warning, remark, note, catastrophic error, ...
    id: str  # "NNNN" or "cerastes-<tag>"
    file: str | None
    line: int | None
    col: int | None
    message: str
    discretionary: bool = False

    def tag(self, registry: TagRegistry) -> str:
        """The tag this diagnostic is expected under (`#NNNN` if it has none)."""
        if self.id.startswith(CERASTES_TAG_PREFIX):
            return self.id
        return registry.tags.get(int(self.id), f"#{self.id}")

    def where(self) -> str:
        if self.line is None:
            return "(no position)"
        col = f":{self.col}" if self.col is not None else ""
        return f"L{self.line}{col}"


@dataclasses.dataclass
class FeParse:
    diags: list[FeDiag]  # primary diagnostics, in order (notes and remarks included)
    problems: list[str]  # output that cannot be classified, terminations, ...
    summary_errors: int | None  # the `N errors detected` count, if printed


def parse_fe_diagnostics(stderr: str) -> FeParse:
    """Parse the diagnostics of a front-end run made with DIAG_OPTIONS.

    Primary lines start in column 0; indented lines and blank lines continue the
    previous diagnostic (argument lists, instantiation contexts) and are
    dropped.  Any other column-0 line that is neither the sign-off summary nor
    a termination notice is a problem (`unrecognized front-end output`), so a
    format this parser does not know can never pass silently.
    """
    diags: list[FeDiag] = []
    problems: list[str] = []
    summary: int | None = None
    for raw in stderr.split("\n"):
        if not raw.strip() or raw[0] in " \t":
            continue
        if m := _FE_DIAG_RE.match(raw):
            diags.append(FeDiag(m.group("severity").lower(), m.group("id"), m.group("file"),
                                int(m.group("line")) if m.group("line") else None,
                                int(m.group("col")) if m.group("col") else None,
                                m.group("message"), bool(m.group("disc"))))
        elif m := _FE_SUMMARY_RE.match(raw):
            if summary is not None:
                problems.append(f"unrecognized front-end output: a second summary line: {raw}")
            summary = int(m.group("errors") or 0)
        elif raw in _FE_TERMINATORS:
            problems.append(_FE_TERMINATORS[raw])
        else:
            problems.append(f"unrecognized front-end output: {raw}")
    return FeParse(diags, problems, summary)


def _describe(d: FeDiag, registry: TagRegistry, filename: str | None = None) -> str:
    """`tag @ L<line>:<col>: message`, with the file if it is not `filename`."""
    if d.file is None:
        return f"{d.tag(registry)}: {d.message}"
    where = d.where() if d.file == filename else f"{d.where()} in {d.file}"
    return f"{d.tag(registry)} @ {where}: {d.message}"


def compare_diags(expected: list[ExpectedDiag], diags: list[FeDiag], registry: TagRegistry,
                  filename: str) -> list[str]:
    """Mismatch descriptions between the expected diagnostics and the ones the
    front end printed, errors and warnings separately; empty when exact.
    Notes and remarks never count.  A diagnostic is in the test file unless the
    expectation names a header (`rejected` tests); one in any other file is
    unexpected, whatever it is."""
    problems = []
    allowed = {filename} | {e.file for e in expected if e.file}
    for severity in ("error", "warning"):
        remaining = [d for d in diags if d.severity == severity]
        for d in list(remaining):
            if not any(same_file(d.file, f) for f in allowed):
                problems.append(f"unexpected {severity} (in {d.file or 'no file'}): "
                                f"{_describe(d, registry, d.file)}")
                remaining.remove(d)
        unmatched = []
        for e in (x for x in expected if x.severity == severity):
            hit = next((d for d in remaining
                        if d.tag(registry) == e.tag and d.line == e.line
                        and same_file(d.file, e.file or filename)), None)
            if hit is None:
                unmatched.append(e)
            else:
                remaining.remove(hit)
        for e in unmatched:
            in_file = [d for d in remaining if same_file(d.file, e.file or filename)]
            same_tag = next((d for d in in_file if d.tag(registry) == e.tag), None)
            same_line = next((d for d in in_file if d.line == e.line), None)
            if same_tag is not None:
                problems.append(f"wrong line: expected {severity} {e}, got "
                                f"{_describe(same_tag, registry, e.file or filename)}")
                remaining.remove(same_tag)
            elif same_line is not None:
                problems.append(f"wrong diagnostic at this line: expected {severity} {e}, "
                                f"got {_describe(same_line, registry, e.file or filename)}")
                remaining.remove(same_line)
            else:
                problems.append(f"missing {severity}: expected {e}")
        for d in remaining:
            problems.append(f"unexpected {severity}: {_describe(d, registry, filename)}")
    return problems


def evaluate_refused(run: ToolRun, exp: Expectation) -> list[str]:
    """Problems with one `frontend-refused` run; empty when the driver refused
    the options as the test says.

    Two shapes count as a refusal, nothing else:
    - the driver's: exit status 2, nothing on stdout, exactly one non-empty line
      on stderr, starting in column 0 with `cerastes-fe: error: `;
    - EDG's command-line error: exit status 4, nothing on stdout, and the three
      lines `Command-line error: <message>` (or `Command-line error #N: ...`),
      `1 catastrophic error detected in this compilation.` and `Compilation
      terminated.`.
    The expected substring must appear in the message line.  A run that is
    accepted (status 0 or 2 with diagnostics), crashes or times out, or prints
    anything else fails.
    """
    if run.returncode is None:
        return [f"cerastes-fe {run.error}"]
    if run.returncode < 0:
        return [f"cerastes-fe crashed (signal {-run.returncode})"]
    problems = []
    if run.stdout:
        problems.append("cerastes-fe printed on stdout (a refusal prints nothing there): "
                        f"{run.stdout.strip().splitlines()[0]}")
    lines = [l for l in run.stderr.split("\n") if l.strip()]
    if run.returncode == REFUSAL_STATUS:
        if len(lines) != 1 or not lines[0].startswith(REFUSAL_PREFIX):
            problems.append(f"exit status {REFUSAL_STATUS} but stderr is not exactly one line "
                            f"starting with `{REFUSAL_PREFIX}` (it may be an ordinary "
                            "compilation with errors): "
                            + (" | ".join(lines)[:300] or "(empty)"))
            return problems
        message = lines[0]
    elif run.returncode == EDG_CMDLINE_STATUS:
        if (len(lines) != 3 or not EDG_CMDLINE_RE.match(lines[0])
                or tuple(lines[1:]) != EDG_CMDLINE_TRAILER):
            problems.append(f"exit status {EDG_CMDLINE_STATUS} but stderr is not EDG's "
                            "command-line error (`Command-line error: ...` and its two "
                            "closing lines): " + (" | ".join(lines)[:300] or "(empty)"))
            return problems
        message = lines[0]
    else:
        return problems + [
            f"cerastes-fe exited with status {run.returncode}; a refusal is status "
            f"{REFUSAL_STATUS} (the driver) or {EDG_CMDLINE_STATUS} (EDG's command-line error)"
            + (": " + " | ".join(lines)[:300] if lines else "")]
    assert exp.refused is not None
    if exp.refused not in message:
        problems.append(f"the refusal does not contain `{exp.refused}`: {message}")
    return problems


def fe_output_problems(stderr: str, expected: list[ExpectedDiag], registry: TagRegistry,
                       filename: str) -> list[str]:
    """Problems with the EDG-format diagnostics of one run (the stderr of a
    `frontend-error` run or of a rejected `cerastes verify` run): output the
    parser does not know, fatal diagnostics, a summary line that disagrees with
    the errors printed, and every difference from the expected diagnostics."""
    parsed = parse_fe_diagnostics(stderr)
    problems = list(parsed.problems)
    for d in parsed.diags:
        if d.severity in _FE_FATAL:
            problems.append(f"{d.severity} always fails a test: "
                            f"{_describe(d, registry, filename)}")
    errors = sum(1 for d in parsed.diags if d.severity == "error")
    if parsed.summary_errors is None and errors:
        problems.append(f"{errors} errors were printed but there is no `N errors "
                        "detected` summary line (the front end did not finish?)")
    elif parsed.summary_errors is not None and parsed.summary_errors != errors:
        problems.append(f"the summary says {parsed.summary_errors} errors but {errors} "
                        "were parsed")
    return problems + compare_diags(expected, parsed.diags, registry, filename)


def evaluate_diags(run: ToolRun, exp: Expectation, registry: TagRegistry,
                   filename: str) -> list[str]:
    """Problems with one `frontend-error`/`-warning` run; empty when it passes."""
    if run.returncode is None:
        return [f"cerastes-fe {run.error}"]
    problems = []
    if run.returncode < 0:
        problems.append(f"cerastes-fe crashed (signal {-run.returncode})")
    if run.stdout:
        problems.append("cerastes-fe printed on stdout (a diagnostics test expects "
                        f"nothing there): {run.stdout.strip().splitlines()[0]}")
    problems += fe_output_problems(run.stderr, exp.diags, registry, filename)
    want = EDG_ERRORS_STATUS if any(d.severity == "error" for d in exp.diags) else 0
    if run.returncode >= 0 and run.returncode != want:
        problems.append(f"cerastes-fe exited with status {run.returncode}, expected {want} "
                        + ("(errors expected)" if want else "(no errors expected)"))
    return problems


# ---------------------------------------------------------------------------
# `cerastes verify` (`verified`, `error`, `rejected` in a C or C++ test)
# ---------------------------------------------------------------------------

# Exit statuses of `cerastes verify` (decision 0011 item 8).
VERIFY_OK = 0
VERIFY_ERRORS = 1
VERIFY_REJECTED = 2
VERIFY_TOOL_FAILURE = 3

# `<file>: verified (<n> functions; one translation unit: ...)`, and
# `<file>:<line>: error|note: <rest>`, in column 0.  The error's rest is
# `<category>: <message>`.  An EDG diagnostic (`"file", line N: error #id: ...`)
# matches neither.
_VERIFIED_LINE_RE = re.compile(
    r"^(?P<file>.+?): verified \((?P<n>[0-9]+) functions?; one translation unit: .+\)$")
_VERIFY_DIAG_RE = re.compile(
    r"^(?P<file>.+?):(?P<line>[1-9][0-9]*): (?P<kind>error|note): (?P<rest>.*)$")
_VERIFY_ERROR_REST_RE = re.compile(r"^(?P<category>[a-z][a-z0-9-]*): (?P<message>.*)$")


@dataclasses.dataclass
class VerifyParse:
    verified: list[str]  # the file named by each `verified` line
    functions: list[int]  # the number of functions of each `verified` line
    errors: list[ReportedError]
    problems: list[str]  # output that cannot be classified


def parse_verify_output(stdout: str, stderr: str) -> VerifyParse:
    """Parse the output of a `cerastes verify` run that ended with status 0 or 1.

    Decision 0011 item 9: the verdict line, the errors and their notes are on
    stdout, and nothing else is printed.  On stdout a `verified` line, an `error`
    line (a category outside DRIVER_CATEGORIES is category `unknown`, which no
    test accepts) and the `note` lines that follow an error are understood; blank
    lines and indented lines continue the previous line and are dropped.  A note
    with no error before it, an EDG diagnostic (they go to stderr), and any
    other line in column 0 are problems.  Anything on stderr is a problem: a
    verdict or an error there is on the wrong stream, and a front-end diagnostic
    (a warning in a test) is unexpected output; tests are warning-free.
    """
    verified: list[str] = []
    functions: list[int] = []
    errors: list[ReportedError] = []
    problems: list[str] = []
    after_error = False
    for raw in stdout.split("\n"):
        if not raw.strip() or raw[0] in " \t":
            continue
        if m := _VERIFY_DIAG_RE.match(raw):
            if m.group("kind") == "note":
                if not after_error:
                    problems.append(f"a `note:` line with no error before it: {raw}")
                continue
            after_error = True
            rest = _VERIFY_ERROR_REST_RE.match(m.group("rest"))
            if rest is None:
                problems.append(f"an `error:` line without `<category>: <message>`: {raw}")
                continue
            category = rest.group("category")
            errors.append(ReportedError(
                category if category in DRIVER_CATEGORIES else UNKNOWN, "",
                int(m.group("line")), m.group("file"),
                raw if category in DRIVER_CATEGORIES else f"unknown category `{category}`: {raw}"))
        elif m := _VERIFIED_LINE_RE.match(raw):
            after_error = False
            verified.append(m.group("file"))
            functions.append(int(m.group("n")))
        elif _FE_DIAG_RE.match(raw):
            after_error = False
            problems.append("a front-end diagnostic on stdout (diagnostics go to stderr, "
                            f"decision 0011 item 9): {raw}")
        else:
            after_error = False
            problems.append(f"unrecognized driver output: {raw}")
    for raw in stderr.split("\n"):
        if not raw.strip() or raw[0] in " \t":
            continue
        if _VERIFY_DIAG_RE.match(raw) or _VERIFIED_LINE_RE.match(raw):
            problems.append(f"a verdict or error line on stderr (they go to stdout, decision "
                            f"0011 item 9): {raw}")
        elif _FE_DIAG_RE.match(raw):
            problems.append("a front-end diagnostic in a run that reached Silicon (a warning "
                            f"is unexpected output; tests are warning-free): {raw}")
        else:
            problems.append(f"unexpected output on stderr: {raw}")
    return VerifyParse(verified, functions, errors, problems)


def _verify_tail(run: ToolRun) -> str:
    """The first lines of what a failed run printed (stderr, else stdout)."""
    lines = [l for l in (run.stderr or run.stdout).split("\n") if l.strip()]
    return " | ".join(lines)[:300] if lines else "(no output)"


def evaluate_verify(run: ToolRun, exp: Expectation, registry: TagRegistry | None,
                    filename: str) -> list[str]:
    """Problems with one `cerastes verify` run of a `verified`, `error` or
    `rejected` test; empty when it passes.

    `filename` is the test path as given on the command line (the file of its
    own messages).  Exit status 3, any status other than 0 to 2, a crash and a
    timeout fail whatever the test expects.  `registry` names the EDG tags of a
    rejection (required for a `rejected` test).
    """
    if run.returncode is None:
        return [f"cerastes verify {run.error}"]
    if run.returncode < 0:
        return [f"cerastes verify crashed (signal {-run.returncode})"]
    code = run.returncode
    if code == VERIFY_TOOL_FAILURE:
        return [f"cerastes verify failed (exit status {code}, a tool failure; it fails "
                f"whatever the test expects): {_verify_tail(run)}"]
    if code not in (VERIFY_OK, VERIFY_ERRORS, VERIFY_REJECTED):
        return [f"cerastes verify exited with status {code}, which is not one of 0 "
                f"(verified), 1 (errors), 2 (rejected): {_verify_tail(run)}"]

    if code == VERIFY_REJECTED:
        if exp.kind != "rejected":
            parsed = parse_fe_diagnostics(run.stderr)
            shown = [f"  {d.where()}: error #{d.id}: {d.message}"
                     for d in parsed.diags if d.severity == "error"]
            return ["expected " + ("verified" if exp.kind == "verified" else "verification "
                    "errors") + ", but the program was rejected (exit status 2):"] \
                + (shown or [f"  {_verify_tail(run)}"])
        assert registry is not None
        problems = []
        if run.stdout:
            problems.append("cerastes verify printed on stdout (a rejection prints "
                            f"nothing there): {run.stdout.strip().splitlines()[0]}")
        return problems + fe_output_problems(run.stderr, exp.diags, registry, filename)

    parsed = parse_verify_output(run.stdout, run.stderr)
    problems = list(parsed.problems)
    if code == VERIFY_OK:
        if parsed.errors:
            problems.append("exit status 0 but the output has `error:` lines")
        if len(parsed.verified) != 1:
            problems.append(f"exit status 0 needs exactly one `{filename}: verified (...)` "
                            f"line, found {len(parsed.verified)}")
        else:
            if parsed.verified[0] != filename:
                problems.append(f"the `verified` line names {parsed.verified[0]}, not {filename}")
            if parsed.functions[0] < 1:
                problems.append("the `verified` line counts no function: nothing of the test "
                                "was verified")
        if exp.kind != "verified":
            problems.append("expected " + ("a rejection (exit status 2)" if exp.kind ==
                            "rejected" else "errors") + ", but the program verified")
        return problems

    # exit status 1
    if parsed.verified:
        problems.append("exit status 1 but the output has a `verified` line")
    if not parsed.errors:
        problems.append("exit status 1 but the output has no `error:` line")
    keys = [(e.file, e.line, e.category) for e in parsed.errors]
    if keys != sorted(keys):
        problems.append("the errors are not sorted by printed path (byte order), line and "
                        "category (byte order) (decision 0011 item 9): " + "; ".join(
                            f"{f}:{n}: {c}" for f, n, c in keys[:6]))
    lines = [e.message for e in parsed.errors]
    for twice in sorted({l for l in lines if lines.count(l) > 1}):
        problems.append(f"an error is printed twice (decision 0011 item 9): {twice}")
    if exp.kind == "verified":
        problems.append("expected verified, but verification errors were reported:")
        problems += [f"  {e}" for e in parsed.errors]
    elif exp.kind == "rejected":
        problems.append("expected a rejection (exit status 2), but verification errors were "
                        "reported (exit status 1):")
        problems += [f"  {e}" for e in parsed.errors]
    else:
        problems += compare_errors(exp.errors, parsed.errors, filename)
    return problems


def verify_command(env: dict[str, str], path: Path, mode: str, vpr: Path) -> list[str]:
    """The `cerastes verify` command for one mode of a test (run from the root):
    VERIFY_COMMAND, else the program of CERASTES_VERIFY."""
    base = VERIFY_COMMAND if VERIFY_COMMAND is not None else [env["CERASTES_VERIFY"]]
    return [*base, "verify", rel(path), VERIFY_STD_OPTIONS[mode], f"--keep-vpr={rel(vpr)}"]


def vpr_path(path: Path, mode: str) -> Path:
    """Where `--keep-vpr` writes for `path` in `mode`: VPR_DIR/<repository-
    relative path>.<mode>.vpr (a test outside the repository goes under
    VPR_DIR/external/ with its absolute path)."""
    shown = rel(path)
    inside = not Path(shown).is_absolute()
    parts = Path(shown).parts if inside else ("external", *Path(shown).parts[1:])
    return VPR_DIR.joinpath(*parts[:-1], f"{parts[-1]}.{mode}.vpr")


def is_verify_test(path: Path, exp: Expectation | HeaderError) -> bool:
    """Whether the test runs `cerastes verify` (a C or C++ file with a verdict)."""
    return (isinstance(exp, Expectation) and path.suffix != ".vpr"
            and exp.kind in VERIFY_KINDS)


def vpr_expected(exp: Expectation) -> bool:
    """Whether a verify run of this test must leave Viper behind: it does when
    Silicon runs (exit status 0 or 1), not for a rejection (exit status 2)."""
    return exp.kind in ("verified", "error")


def vpr_problems(vpr: Path, filename: str | None = None,
                 headers: list[str] | tuple[str, ...] = ()) -> list[str]:
    """Problems with the `--keep-vpr` output of a run that ended with status 0 or
    1: the `.vpr` and its `.map` must exist and not be empty, and the map (tab-
    separated, the file the 4th field: encoding.md section 5) must have at least
    one entry in `filename`, the test file as given to the driver, so that a test
    none of whose bodies was verified (all skipped, or taken for system code)
    cannot pass.  `headers` are the helper headers with function bodies that the test
    includes: each needs an entry too, so a driver that skipped their bodies fails."""
    problems = []
    mapf = vpr.with_name(vpr.name + ".map")
    for f in (vpr, mapf):
        try:
            size = f.stat().st_size
        except OSError:
            problems.append(f"the driver did not write {rel(f)} (`--keep-vpr`)")
            continue
        if size == 0:
            problems.append(f"{rel(f)} is empty")
    if filename is not None and not problems:
        files = {line.split("\t")[3] for line in read_exact(mapf).split("\n")
                 if line.count("\t") >= 3}
        if not any(same_file(f, filename) for f in files):
            problems.append(f"{rel(mapf)} has no entry in {filename}: nothing of the test "
                            "was verified")
        for h in headers:
            if not any(same_file(f, h) for f in files):
                problems.append(f"{rel(mapf)} has no entry in {h}, a helper header with a "
                                f"function body that {filename} includes: its bodies were not "
                                "verified")
    return problems


def vpr_manifest(tests: list[tuple[Path, Expectation]]) -> list[str]:
    """The files (relative to VPR_DIR) that a run of `tests` must leave behind."""
    return sorted(vpr_path(f, m).relative_to(VPR_DIR).as_posix()
                  for f, e in tests if vpr_expected(e) for m in e.modes)


def make_run_id(manifest: list[str]) -> str:
    """`<UTC time>-<8 hex digits of the hash of the manifest's file list>`."""
    digest = hashlib.sha256("\n".join(manifest).encode()).hexdigest()[:8]
    now = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    return f"{now}-{digest}"


_VPR_LOCKS: dict[Path, object] = {}


def release_vpr_locks() -> None:
    """Drop every lock this process holds (at exit; a self-test that holds several in temporary trees)."""
    for handle in _VPR_LOCKS.values():
        handle.close()
    _VPR_LOCKS.clear()


atexit.register(release_vpr_locks)


def lock_vpr_dir() -> None:
    """Take the exclusive lock on build/vpr for the rest of this process.  A run that will empty and fill build/vpr
    (every run that selects a test expecting Viper) must be the only one: a second run would empty it under the first,
    whose tests would then fail with "the driver did not write build/vpr/...".  The lock is a `flock` on
    `<VPR_DIR>.lock` (released when the process ends, however it ends); it is never waited for: a second run fails
    at once, naming the first.  A process that already holds it (a self-test calling main() twice) keeps it."""
    import fcntl
    path = VPR_DIR.with_name(VPR_DIR.name + ".lock")
    if path in _VPR_LOCKS:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    handle = open(path, "a+")
    try:
        fcntl.flock(handle, fcntl.LOCK_EX | fcntl.LOCK_NB)
    except OSError:
        handle.seek(0)
        pid = handle.read().strip() or "unknown"
        handle.close()
        raise UsageError(f"another run_tests.py is using build/vpr (pid {pid}); wait for it or stop it "
                         f"(the lock is {rel(path)})") from None
    handle.seek(0)
    handle.truncate()
    handle.write(f"{os.getpid()}\n")
    handle.flush()
    _VPR_LOCKS[path] = handle


def reset_vpr_dir(manifest: list[str], run_id: str) -> None:
    """Empty VPR_DIR (stale files cannot pass for this run's output) and write
    its MANIFEST: the run id line, then one file per line.  Refuses a directory
    that is not named `vpr`, so a bad patch of VPR_DIR cannot delete anything
    else."""
    if VPR_DIR.name != "vpr":
        raise UsageError(f"refusing to clear {VPR_DIR}: not a directory named `vpr`")
    if VPR_DIR.is_symlink() or VPR_DIR.is_file():
        VPR_DIR.unlink()
    elif VPR_DIR.exists():
        shutil.rmtree(VPR_DIR)
    VPR_DIR.mkdir(parents=True)
    (VPR_DIR / VPR_MANIFEST).write_text(f"{RUN_ID_PREFIX}{run_id}\n"
                                        + "".join(f"{line}\n" for line in manifest))


# ---------------------------------------------------------------------------
# Ordinary compilers (`expect: native-ok`)
# ---------------------------------------------------------------------------

NATIVE_STD = {"c++20": "c++20", "c23": "c23"}
NATIVE_LANG = {"c++20": "c++", "c23": "c"}
# What a compiler with the flags above must reject and accept (anti-vacuity
# canaries).  Both snippets are valid C and C++.
CANARY_FAIL = "int canary(void) { int unused = 0; return 1; }\n"
CANARY_PASS = "int canary(void) { return 1; }\n"
# An unknown scoped attribute is a warning for both compilers, so -Werror must
# reject it: that is what keeps a `[[cerastes::...]]` left in the source from
# passing as ordinary code.  Valid C23 and C++.
CANARY_ATTRIBUTE = "[[unreg_ns::x]] int canary_v;\n"
GCC_MISSING_NOTE = "native-ok: GCC not installed; Apple clang only"


@dataclasses.dataclass
class NativeCompiler:
    family: str  # "Apple clang" or "GCC"
    cc: str
    cxx: str

    def exe(self, lang: str) -> str:
        return self.cxx if lang == "c++" else self.cc


@dataclasses.dataclass
class NativeSetup:
    include_dir: str
    compilers: list[NativeCompiler]
    # canary problems, by (family, language); a compiler whose canary failed
    # fails every native-ok run instead of vouching for the files
    canary: dict[tuple[str, str], list[str]] = dataclasses.field(default_factory=dict)
    runs: dict[str, int] = dataclasses.field(default_factory=dict)  # family -> count
    lock: threading.Lock = dataclasses.field(default_factory=threading.Lock)
    note: str | None = None


def native_command(exe: str, lang: str, mode: str, include_dir: str, path: str) -> list[str]:
    """The ordinary-compiler check of one file in one mode.  `__CERASTES__` is
    undefined so that <cerastes.h> expands the annotations to nothing."""
    return [exe, "-x", lang, f"-std={NATIVE_STD[mode]}", "-Wall", "-Wextra", "-Werror",
            "-U__CERASTES__", "-I", include_dir, "-c", "-o", "/dev/null", path]


@functools.lru_cache(maxsize=1)
def sdk_root() -> str | None:
    """The macOS SDK (`xcrun --show-sdk-path`); Apple clang run by its real
    path, not through the /usr/bin shim, needs SDKROOT to find the C++ library."""
    if sys.platform != "darwin":
        return None
    try:
        proc = subprocess.run(["xcrun", "--show-sdk-path"], capture_output=True, text=True,
                              timeout=60)
    except (OSError, subprocess.TimeoutExpired):
        return None
    return proc.stdout.strip() or None if proc.returncode == 0 else None


def native_env(env: dict[str, str], family: str) -> dict[str, str]:
    out = dict(env)
    if family == "Apple clang" and not out.get("SDKROOT") and (sdk := sdk_root()):
        out["SDKROOT"] = sdk
    return out


def banner_problem(family: str, version_output: str) -> str | None:
    """Why `--version` output does not look like `family`, or None."""
    first = version_output.strip().splitlines()[0] if version_output.strip() else ""
    if family == "Apple clang":
        ok = "Apple clang" in first
    else:
        ok = re.search(r"\bGCC\b", first) is not None and "clang" not in first.lower()
    return None if ok else f"`--version` does not say {family}: {first!r}"


def native_canary(comp: NativeCompiler, lang: str, mode: str, setup_env: dict[str, str],
                  include_dir: str) -> list[str]:
    """Anti-vacuity canary for one compiler and language: its banner is the
    expected family, a snippet that is fine compiles silently, and a snippet
    with an unused variable and one with an unknown scoped attribute are
    rejected by the same flags.  A compiler that
    accepts everything (or nothing) cannot vouch for a test."""
    exe = comp.exe(lang)
    cenv = native_env(setup_env, comp.family)
    run = run_tool([exe, "--version"], cenv, 60)
    if run.returncode is None:
        return [f"{comp.family} ({lang}): `{exe} --version` {run.error}"]
    if run.returncode != 0:
        return [f"{comp.family} ({lang}): `{exe} --version` exited with status "
                f"{run.returncode}"]
    if (why := banner_problem(comp.family, run.stdout)) is not None:
        return [f"{comp.family} ({lang}): {why}"]
    suffix = ".cpp" if lang == "c++" else ".c"
    problems = []
    with tempfile.TemporaryDirectory() as tmp:
        for name, snippet in (("canary_pass", CANARY_PASS), ("canary_fail", CANARY_FAIL),
                              ("canary_attribute", CANARY_ATTRIBUTE)):
            f = Path(tmp) / (name + suffix)
            f.write_text(snippet)
            run = run_tool(native_command(exe, lang, mode, include_dir, str(f)), cenv,
                           NATIVE_TIMEOUT_S)
            if run.returncode is None:
                return [f"{comp.family} ({lang}): canary {run.error}"]
            if name == "canary_pass" and (run.returncode != 0 or run.stdout or run.stderr):
                shown = (run.stderr or run.stdout).strip().splitlines()[:1]
                problems.append(f"{comp.family} ({lang}): a clean snippet did not compile "
                                f"silently (status {run.returncode}): {shown}")
            if name == "canary_fail" and (run.returncode == 0 or not (run.stderr or run.stdout)):
                problems.append(f"{comp.family} ({lang}): an unused variable was not "
                                "rejected by -Wall -Wextra -Werror (a vacuous check)")
            if name == "canary_attribute" and (run.returncode == 0
                                               or not (run.stderr or run.stdout)):
                problems.append(f"{comp.family} ({lang}): an unknown scoped attribute "
                                "([[unreg_ns::x]]) was not rejected by -Wall -Wextra -Werror")
    return problems


def prepare_native(env: dict[str, str],
                   tests: list[tuple[Path, Expectation]]) -> tuple[NativeSetup, list[Outcome]]:
    """The compilers named in `env` for the selected native-ok tests, with a
    canary Outcome per compiler and language that those tests use."""
    for name in ("CERASTES_CLANG_CC", "CERASTES_CLANG_CXX", "CERASTES_INCLUDE_DIR"):
        require(env, name)
    missing = [n for n in ("CERASTES_GCC_CC", "CERASTES_GCC_CXX") if n not in env]
    if missing:
        raise UsageError(f"{', '.join(missing)} not set (an empty value means GCC is not "
                         "installed); run scripts/bootstrap.sh first")
    compilers = [NativeCompiler("Apple clang", env["CERASTES_CLANG_CC"],
                                env["CERASTES_CLANG_CXX"])]
    gcc_cc, gcc_cxx = env["CERASTES_GCC_CC"], env["CERASTES_GCC_CXX"]
    setup = NativeSetup(env["CERASTES_INCLUDE_DIR"], compilers)
    if gcc_cc and gcc_cxx:
        compilers.append(NativeCompiler("GCC", gcc_cc, gcc_cxx))
    elif gcc_cc or gcc_cxx:
        raise UsageError("CERASTES_GCC_CC and CERASTES_GCC_CXX must both be set or both "
                         "empty; run scripts/bootstrap.sh")
    else:
        setup.note = GCC_MISSING_NOTE
    used = {(NATIVE_LANG[m], m) for _, exp in tests for m in exp.modes}
    outcomes = []
    for comp in compilers:
        for lang, mode in sorted(used):
            problems = native_canary(comp, lang, mode, env, setup.include_dir)
            setup.canary[(comp.family, lang)] = problems
            outcomes.append(Outcome(f"canary native-ok [{comp.family}, {lang}]", problems))
    return setup, outcomes


def run_native(path: Path, exp: Expectation, env: dict[str, str],
               setup: NativeSetup) -> list[Outcome]:
    outcomes = []
    for mode in exp.modes:
        lang = NATIVE_LANG[mode]
        for comp in setup.compilers:
            name = f"{rel(path)} [{mode}] native-ok: {comp.family}"
            if canary := setup.canary.get((comp.family, lang)):
                outcomes.append(Outcome(name, [f"the {comp.family} canary failed, so its "
                                               "result means nothing:", *canary]))
                continue
            with setup.lock:
                setup.runs[comp.family] = setup.runs.get(comp.family, 0) + 1
            cmd = native_command(comp.exe(lang), lang, mode, setup.include_dir, rel(path))
            run = run_tool(cmd, native_env(env, comp.family), NATIVE_TIMEOUT_S)
            output = run.stdout + run.stderr
            problems = []
            if run.returncode is None:
                problems.append(f"{comp.family} {run.error}")
            else:
                if run.returncode != 0:
                    problems.append(f"{comp.family} rejected the file (exit status "
                                    f"{run.returncode}); an annotated test must build with "
                                    "ordinary compilers")
                if output.strip():
                    problems.append(f"{comp.family} printed diagnostics (-Wall -Wextra "
                                    "-Werror, no __CERASTES__):")
                    problems += [f"  {l}" for l in output.rstrip().splitlines()]
            outcomes.append(Outcome(name, problems, output))
    return outcomes


# ---------------------------------------------------------------------------
# The EDG tree (decision 0005)
# ---------------------------------------------------------------------------

def edg_stamp_problem(root: Path | None = None,
                      caller: dict[str, str] | None = None) -> str | None:
    """Why the EDG tools were not built from the active tree as it is now, or
    None: the same test as `edg_tree.py describe --verify-build` (the stamp of
    the active kind equals the stamp of the tree and of the cerastes
    configuration now), so a stale binary cannot run the tests."""
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    try:
        import edg_tree
    finally:
        sys.path.pop(0)
    status, _out, err = edg_tree.describe(
        str(ROOT if root is None else root), verify_build=True,
        env=dict(os.environ) if caller is None else caller)
    if status == 0:
        return None
    problems = [l[len("error: "):] if l.startswith("error: ") else l
                for l in err if not l.startswith("warning: ")]
    return "; ".join(problems) or "the EDG tree check failed"


def edg_tree_problem(env_sh: Path | None = None,
                     caller: dict[str, str] | None = None) -> str | None:
    """Why the tools of build/env.sh are not for the tree `CERASTES_EDG_SRC`
    selects (a relative `CERASTES_EDG_SRC` is refused too), or None.  `caller` is the environment of the caller (default:
    os.environ); the selection is made from it, before env.sh is read, so an
    inherited variable cannot hide a mismatch.  Unset or empty selects
    third_party/edg.  A missing env.sh is not a mismatch (require() reports it).
    """
    env_sh = env_sh_path() if env_sh is None else env_sh
    caller = dict(os.environ) if caller is None else caller
    value = caller.get("CERASTES_EDG_SRC")
    if value and not value.startswith("/"):
        return (f"CERASTES_EDG_SRC={value} is a relative path; use an absolute path "
                "(decision 0005)")
    if not env_sh.is_file():
        return None
    selected = value or str(ROOT / "third_party" / "edg")
    requested = os.path.realpath(selected)
    active = None
    for line in env_sh.read_text().splitlines():
        if m := re.match(r'^export CERASTES_EDG_SRC_ACTIVE="(.*)"$', line):
            active = m.group(1)
    if not active:
        return ("build/env.sh does not name its EDG tree (CERASTES_EDG_SRC_ACTIVE is "
                "missing); run scripts/bootstrap.sh")
    if os.path.realpath(active) != requested:
        return (f"build/env.sh is for {active} but CERASTES_EDG_SRC selects "
                f"{requested}; run scripts/bootstrap.sh")
    return None


# ---------------------------------------------------------------------------
# Runners
# ---------------------------------------------------------------------------

@dataclasses.dataclass
class Outcome:
    name: str
    problems: list[str]
    output: str = ""
    label: str | None = None  # status shown instead of PASS (UPDATED, ...)
    notes: list[str] = dataclasses.field(default_factory=list)  # not failures


@dataclasses.dataclass
class RunOptions:
    verbose: bool = False
    update_golden: bool = False
    native: NativeSetup | None = None  # the compilers of the native-ok checks
    registry: TagRegistry | None = None  # the tags of the frontend-error checks


ENV_SH: Path | None = None  # the env file; None: ROOT/build/env.sh (self-tests point it elsewhere)
_ignored_inherited: set[str] = set()


def env_sh_path() -> Path:
    return ENV_SH if ENV_SH is not None else ROOT / "build" / "env.sh"


def load_env() -> dict[str, str]:
    """The environment with build/env.sh applied (without a shell).

    build/env.sh is authoritative, as it is for the scripts that source it:
    every variable it defines takes its value from the file, and an inherited
    value that differs (an empty CERASTES_GCC_CC, a foreign CERASTES_EDG_GEN or
    CERASTES_INCLUDE_DIR) is ignored with one note on stderr, so that the
    environment of the shell cannot silently change which compilers, tag
    tables or include directory a test run uses.  Variables the file does not
    define (PATH, SDKROOT, ...) are inherited.
    """
    env = dict(os.environ)
    env_sh = env_sh_path()
    if env_sh.exists():
        for line in env_sh.read_text().splitlines():
            if m := re.match(r'^export (\w+)="(.*)"$', line):
                name, value = m.group(1), m.group(2)
                if name in env and env[name] != value and name not in _ignored_inherited:
                    _ignored_inherited.add(name)
                    print(f"run_tests.py: note: ignoring the inherited {name}={env[name]!r}; "
                          f"{env_sh} defines {name}={value!r} and is authoritative",
                          file=sys.stderr)
                env[name] = value
    return env


def require(env: dict[str, str], *names: str) -> None:
    missing = [n for n in names if not env.get(n)]
    if missing:
        raise UsageError(f"{', '.join(missing)} not set; run scripts/bootstrap.sh first")


def run_tool(cmd: list[str], env: dict[str, str], timeout: int) -> ToolRun:
    """Run `cmd` from the repository root, keeping stdout and stderr apart."""
    try:
        proc = subprocess.run(cmd, capture_output=True, env=env, cwd=ROOT,
                              timeout=timeout)
    except subprocess.TimeoutExpired:
        return ToolRun(None, error=f"timed out after {timeout}s")
    except OSError as err:
        return ToolRun(None, error=f"could not be run ({cmd[0]}): {err}")
    return ToolRun(proc.returncode,
                   proc.stdout.decode("utf-8", "surrogateescape"),
                   proc.stderr.decode("utf-8", "surrogateescape"))


def run_silicon(path: Path, exp: Expectation, env: dict[str, str]) -> list[Outcome]:
    cmd = [env["CERASTES_JAVA"], "-Xss128m", "-jar", env["SILICON_JAR"],
           "--z3Exe", env["Z3_EXE"], "--numberOfErrorsToReport", "0", str(path)]
    try:
        proc = subprocess.run(cmd, capture_output=True, text=True, env=env,
                              timeout=SILICON_TIMEOUT_S)
    except subprocess.TimeoutExpired:
        return [Outcome(rel(path), [f"Silicon timed out after {SILICON_TIMEOUT_S}s"])]
    output = proc.stdout + proc.stderr
    result = parse_silicon_output(output, proc.returncode)
    problems = [result.problem] if result.problem else []
    if exp.kind == "verified":
        if not result.verified:
            problems.append("expected verified, but Silicon reported errors")
            problems += [f"  {e}" for e in result.errors]
    elif result.verified:
        problems.append("expected errors, but the program verified")
    else:
        problems += compare_errors(exp.errors, result.errors, path.name)
    return [Outcome(rel(path), problems, output)]


def run_frontend(path: Path, exp: Expectation, env: dict[str, str]) -> list[Outcome]:
    outcomes = []
    for mode in exp.modes:
        name = f"{rel(path)} [{mode}]"
        if exp.kind != "frontend-ok":
            outcomes.append(Outcome(name, [
                f"`expect: {exp.kind}` is not a front-end test (harness error)"]))
            continue
        cmd = [str(ROOT / "scripts" / "cpfe.sh"), MODE_OPTIONS[mode],
               "--no_code_gen", str(path)]
        try:
            proc = subprocess.run(cmd, capture_output=True, text=True, env=env,
                                  timeout=FRONTEND_TIMEOUT_S)
        except subprocess.TimeoutExpired:
            outcomes.append(Outcome(name, [f"cpfe timed out after {FRONTEND_TIMEOUT_S}s"]))
            continue
        output = proc.stdout + proc.stderr
        problems = []
        if proc.returncode != 0:
            problems.append(f"cpfe exited with status {proc.returncode}")
        if output.strip():
            problems.append("cpfe produced diagnostics")
        outcomes.append(Outcome(name, problems, output))
    return outcomes


def diag_command(path: Path, mode: str) -> list[str]:
    """The cerastes-fe command for a frontend-error/-warning test (from the root)."""
    return [*FE_COMMAND, MODE_OPTIONS[mode], *DIAG_OPTIONS, rel(path)]


def run_diags(path: Path, exp: Expectation, env: dict[str, str],
              opts: RunOptions) -> list[Outcome]:
    assert opts.registry is not None
    outcomes = []
    for mode in exp.modes:
        name = f"{rel(path)} [{mode}]"
        if opts.update_golden:
            continue  # not a golden test: nothing to update
        run = run_tool(diag_command(path, mode), env, FRONTEND_TIMEOUT_S)
        outcomes.append(Outcome(name, evaluate_diags(run, exp, opts.registry, rel(path)),
                                run.stdout + run.stderr))
    return outcomes


def refused_command(path: Path, exp: Expectation) -> list[str]:
    """The cerastes-fe command for a frontend-refused test: its own options in
    place of a mode option (from the repository root)."""
    return [*FE_COMMAND, *exp.fe_options, rel(path)]


def run_refused(path: Path, exp: Expectation, env: dict[str, str],
                opts: RunOptions) -> list[Outcome]:
    if opts.update_golden:
        return []  # not a golden test: nothing to update
    run = run_tool(refused_command(path, exp), env, FRONTEND_TIMEOUT_S)
    return [Outcome(rel(path), evaluate_refused(run, exp), run.stdout + run.stderr)]


def walk_command(path: Path, mode: str) -> list[str]:
    """The cerastes-fe command for a golden test (run from the repo root)."""
    return [*FE_COMMAND, MODE_OPTIONS[mode], WALK_OPTION, rel(path)]


def run_golden(path: Path, exp: Expectation, env: dict[str, str],
               opts: RunOptions) -> list[Outcome]:
    outcomes = []
    for mode in exp.modes:
        name = f"{rel(path)} [{mode}]"
        golden = golden_path(path, mode, exp.modes)
        run = run_tool(walk_command(path, mode), env, FRONTEND_TIMEOUT_S)
        actual = golden_text(run.stdout, run.stderr)
        if not opts.update_golden:
            expected = read_exact(golden) if golden.is_file() else None
            outcomes.append(Outcome(name, evaluate_golden(run, expected, rel(golden)),
                                    actual))
            continue
        # --update-golden: write what the front end printed, but never the
        # output of a failed or empty run.
        if problems := run_problems(run):
            outcomes.append(Outcome(name, ["not updated: " + p for p in problems], actual))
            continue
        old = read_exact(golden) if golden.is_file() else None
        if old == actual:
            outcomes.append(Outcome(rel(golden), [], actual, label="UNCHANGED"))
            continue
        write_exact(golden, actual)
        notes = ([f"new golden file ({len(_split_lines(actual))} lines)"]
                 if old is None else compare_golden(old, actual, rel(golden)))
        outcomes.append(Outcome(rel(golden), [], actual, label="UPDATED", notes=notes))
    return outcomes


def run_verify(path: Path, exp: Expectation, env: dict[str, str],
               opts: RunOptions) -> list[Outcome]:
    """One `cerastes verify` run per mode of a `verified`, `error` or `rejected`
    test (see the module docstring and evaluate_verify)."""
    if opts.update_golden:
        return []  # not a golden test: nothing to update
    outcomes = []
    for mode in exp.modes:
        vpr = vpr_path(path, mode)
        # A file left by an earlier run cannot stand in for the one the driver
        # has to write now.
        for f in (vpr, vpr.with_name(vpr.name + ".map")):
            f.unlink(missing_ok=True)
        vpr.parent.mkdir(parents=True, exist_ok=True)
        run = run_tool(verify_command(env, path, mode, vpr), env, VERIFY_TIMEOUT_S)
        problems = evaluate_verify(run, exp, opts.registry, rel(path))
        if run.returncode in (VERIFY_OK, VERIFY_ERRORS) and vpr_expected(exp):
            problems += vpr_problems(vpr, rel(path),
                                     [rel(h) for h in helper_bodies_included(path)])
        outcomes.append(Outcome(f"{rel(path)} [{mode}]", problems, run.stdout + run.stderr))
    return outcomes


def run_one(path: Path, exp: Expectation | HeaderError, env: dict[str, str],
            opts: RunOptions) -> list[Outcome]:
    if isinstance(exp, HeaderError):
        return [Outcome(rel(path), [f"bad header: {exp}"])]
    if path.suffix == ".vpr":
        return run_silicon(path, exp, env)
    if exp.kind == "golden":
        outcomes = run_golden(path, exp, env, opts)
    elif exp.kind == "frontend-diag":
        outcomes = run_diags(path, exp, env, opts)
    elif exp.kind == "frontend-refused":
        outcomes = run_refused(path, exp, env, opts)
    elif exp.kind in VERIFY_KINDS:
        outcomes = run_verify(path, exp, env, opts)
    else:
        outcomes = run_frontend(path, exp, env)
    if exp.native_ok and opts.native is not None and not opts.update_golden:
        outcomes += run_native(path, exp, env, opts.native)
    return outcomes


def rel(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(ROOT))
    except ValueError:
        return str(path)


# ---------------------------------------------------------------------------
# Test selection
# ---------------------------------------------------------------------------

@dataclasses.dataclass
class Selection:
    tests: list[Path] = dataclasses.field(default_factory=list)
    goldens: list[Path] = dataclasses.field(default_factory=list)  # under selected dirs
    skipped: dict[Path, str] = dataclasses.field(default_factory=dict)  # dir -> reason
    unrecognized: list[Path] = dataclasses.field(default_factory=list)  # source-like, not tests
    hidden: list[Path] = dataclasses.field(default_factory=list)  # tests inside skipped dirs
    helpers: list[Path] = dataclasses.field(default_factory=list)  # helper headers (headers/)


UNRECOGNIZED_MESSAGE = ("looks like a C/C++/Viper source file, but its suffix is not a "
                        "test suffix (" + ", ".join(sorted(TEST_SUFFIXES)) + "), so it "
                        "would never run; rename it or remove it (a header that tests include "
                        "belongs in a `headers/` directory)")
# Helper headers (and `.inc`, `.def`, ... files that tests include) live only in a
# directory of this name, below the test area that uses them; they are never run
# as tests, hold no `expect:` line and must be included by some test.
HELPER_DIR_NAME = "headers"
_INCLUDE_RE = re.compile(r'^[ \t]*#[ \t]*include[ \t]*"([^"\n]+)"', re.MULTILINE)
HIDDEN_MESSAGE = ("has an `// expect:` header but is in a skipped directory, so it "
                  "would never run; move it out of the skipped directory or remove "
                  "the header")


def looks_like_source(name: str) -> bool:
    """A source-like file name that is not a test file name."""
    suffix = Path(name).suffix
    return suffix not in TEST_SUFFIXES and suffix.lower() in SOURCE_LIKE_SUFFIXES


def is_helper(path: Path, scan_root: Path) -> bool:
    """Whether a source-like file is in a `headers/` directory: a directory
    component of that name below the repository root (below the scanned
    directory, for a file outside the repository), so that where the repository
    is checked out cannot decide it."""
    try:
        parts = path.resolve().relative_to(ROOT.resolve()).parts
    except ValueError:
        try:
            parts = path.resolve().relative_to(scan_root.resolve()).parts
        except ValueError:
            return False
    return HELPER_DIR_NAME in parts[:-1]


def helper_area(path: Path) -> Path:
    """The directory that holds the `headers/` directory of a helper: the tests
    that may include it are below it."""
    parts = path.parts
    return Path(*parts[:max(i for i, p in enumerate(parts[:-1]) if p == HELPER_DIR_NAME)])


def active_source_text(text: str) -> str:
    """`text` without comments and without the lines of `#if 0` blocks (up to their
    `#else`, `#elif` or `#endif`), so that an include there is not an include.  Other
    conditionals count as active."""
    text = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group().count("\n"), text, flags=re.S)
    out: list[str] = []
    skipping = 0  # nesting depth inside an `#if 0` block, 0 when not skipping
    for raw in text.split("\n"):
        line = re.sub(r"//.*$", "", raw)
        directive = re.match(r"^\s*#\s*(\w+)\s*(\S*)", line)
        word = directive.group(1) if directive else ""
        if skipping:
            if word in ("if", "ifdef", "ifndef"):
                skipping += 1
            elif word == "endif":
                skipping -= 1
            elif word in ("else", "elif") and skipping == 1:
                skipping = 0
            out.append("")
            continue
        if word == "if" and directive and directive.group(2) == "0":
            skipping = 1
            out.append("")
            continue
        out.append(line)
    return "\n".join(out)


def includes_of(path: Path) -> set[Path]:
    """The files that the `#include "..."` lines of `path` name, resolved against its
    directory (the harness passes no -I), which exist.  Includes in comments and in
    `#if 0` blocks do not count."""
    try:
        text = path.read_bytes().decode("utf-8", "replace")
    except OSError:
        return set()
    out = set()
    for name in _INCLUDE_RE.findall(active_source_text(text)):
        target = (path.parent / name).resolve()
        if target.is_file():
            out.add(target)
    return out


_TRANSPARENT_BLOCK_RE = re.compile(
    r'\s*(?:(?:\[\[.*?\]\]\s*)*(?:inline\s+)?namespace\b.*|extern\s*""\s*)\Z', re.DOTALL)
_NOT_A_BODY_RE = re.compile(r"\s*(?:struct|union|enum|class|namespace|typedef)\b")


def has_function_body(text: str) -> bool:
    """Whether `text` (a header) defines a function: a `{` at file scope, or in
    namespaces and `extern "C"` / `extern "C++"` blocks only (nested to any depth),
    whose statement so far has a `)` and is not a struct, union, enum, class,
    namespace or typedef, nor an initializer (`=` outside parentheses).  A
    function inside a class or any other braces is not a function of the file
    (the encoder rejects classes), and a block whose head is neither a namespace
    nor a linkage block hides what is inside it."""
    code = active_source_text(text)
    code = re.sub(r'"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'', '""', code)
    stack: list[bool] = []  # one entry per open brace: is it a namespace or linkage block
    start = 0  # where the statement at the current (transparent) level began
    for i, ch in enumerate(code):
        at_top = all(stack)
        if ch == "{":
            if at_top:
                stmt = code[start:i]
                if _TRANSPARENT_BLOCK_RE.match(stmt):
                    stack.append(True)
                    start = i + 1
                    continue
                paren, flat = 0, []
                for c in stmt:
                    paren += (c == "(") - (c == ")")
                    flat.append(c if paren == 0 else " ")
                outer = "".join(flat)
                if (")" in stmt and "=" not in outer.replace("==", "")
                        and not _NOT_A_BODY_RE.match(stmt)):
                    return True
            stack.append(False)
        elif ch == "}":
            if stack:
                stack.pop()
            if all(stack):
                start = i + 1
        elif ch == ";" and at_top:
            start = i + 1
    return False


def in_headers_dir(path: Path, base: Path) -> bool:
    """Whether `path` is in a `headers/` directory: below the repository root, else
    below `base` (a test outside the repository, in the self-test)."""
    for root in (ROOT, base):
        try:
            return HELPER_DIR_NAME in path.resolve().relative_to(root.resolve()).parts[:-1]
        except ValueError:
            continue
    return False


def helper_bodies_included(test: Path) -> list[Path]:
    """The helper headers with a function body that `test` includes, directly or
    through other helpers (comments and `#if 0` aside): the files whose bodies the
    driver must verify with the test."""
    out: list[Path] = []
    seen: set[Path] = set()
    todo = [test.resolve()]
    while todo:
        f = todo.pop()
        for g in sorted(includes_of(f)):
            if g in seen or not in_headers_dir(g, test.parent):
                continue
            seen.add(g)
            todo.append(g)
            try:
                if has_function_body(g.read_bytes().decode("utf-8", "replace")):
                    out.append(g)
            except OSError:
                pass
    return out


def helper_problems(helpers: list[Path]) -> list[tuple[Path, str]]:
    """Problems with the helper headers: one that has an `// expect:` line is a
    test in disguise, and one that no test includes (directly, or through other
    helpers) is an orphan that nothing verifies.  Looks at the whole tree of the
    helper's area, whatever was selected."""
    problems: list[tuple[Path, str]] = []
    reach: dict[Path, set[Path]] = {}
    for h in helpers:
        if has_expect_header(h):
            problems.append((h, "a helper header is not a test: it must not have an "
                                "`// expect:` line"))
        area = helper_area(h.resolve())
        if area not in reach:
            tests, files = [], []
            for f in sorted(area.rglob("*")):
                if f.is_file() and f.suffix in (".c", ".cpp"):
                    tests.append(f)
                elif f.is_file() and looks_like_source(f.name):
                    files.append(f)
            graph = {f.resolve(): includes_of(f) for f in tests + files}
            seen: set[Path] = set()
            todo = [t.resolve() for t in tests]
            while todo:
                f = todo.pop()
                for g in graph.get(f, includes_of(f)):
                    if g not in seen:
                        seen.add(g)
                        todo.append(g)
            reach[area] = seen
        if h.resolve() not in reach[area]:
            problems.append((h, "orphan helper header: no test includes it (directly or through "
                                "another helper), so nothing verifies it; include it from a "
                                "test or remove it"))
    return problems


def has_expect_header(path: Path) -> bool:
    """Whether the leading comment block of `path` has an `// expect:` line."""
    try:
        text = path.read_bytes().decode("utf-8", "replace")
    except OSError:
        return False
    for raw in text.splitlines():
        line = raw.strip()
        if line and not line.startswith("//"):
            return False
        if _EXPECT_RE.match(line):
            return True
    return False


def skip_reason(directory: Path) -> str | None:
    """The reason in `directory`'s skip marker, or None if it has none."""
    marker = directory / SKIP_MARKER
    if not marker.is_file():
        return None
    allowed = {(ROOT / a).resolve() for a in SKIP_ALLOWED}
    if directory.resolve() not in allowed:
        raise UsageError(
            f"{rel(marker)}: a skip marker is not permitted here; only "
            + ", ".join(sorted(str(a) for a in SKIP_ALLOWED))
            + " may have one (SKIP_ALLOWED in scripts/run_tests.py)")
    reason = marker.read_text().strip()
    if not reason:
        raise UsageError(f"{rel(marker)} is empty; write in it why the directory "
                         "is skipped")
    if "\n" in reason:
        raise UsageError(f"{rel(marker)}: the reason must be a single line")
    return reason


def skipping_directory(path: Path) -> Path | None:
    """The nearest directory at or above `path` that has a skip marker.

    The search stops at the repository root for paths inside it.
    """
    start = path.resolve()
    if not start.is_dir():
        start = start.parent
    for directory in (start, *start.parents):
        if skip_reason(directory) is not None:
            return directory
        if directory == ROOT:
            break
    return None


def default_paths() -> list[str]:
    """DEFAULT_PATHS under the repository root; one that is missing is an error.

    A missing default (a deleted examples directory, say) must not shrink the
    run quietly, so it is a usage error naming the path.
    """
    paths = []
    for p in DEFAULT_PATHS:
        full = ROOT / p
        if not full.is_dir():
            raise UsageError(f"default test directory {p} is missing (looked for "
                             f"{full}); restore it, or name the tests to run")
        paths.append(str(full))
    return paths


def discover(paths: list[str]) -> Selection:
    """The tests and golden files under `paths`, minus skipped directories."""
    sel = Selection()
    seen: set[Path] = set()
    seen_goldens: set[Path] = set()

    def skip(directory: Path) -> None:
        reason = skip_reason(directory)
        assert reason is not None
        if directory.resolve() in sel.skipped:
            return
        sel.skipped[directory.resolve()] = reason
        # Real tests hidden in a skipped directory would never run.
        for dirpath, _dirs, filenames in os.walk(directory):
            for name in sorted(filenames):
                f = Path(dirpath) / name
                if (f.suffix in TEST_SUFFIXES or looks_like_source(name)) \
                        and has_expect_header(f):
                    sel.hidden.append(f)

    def add_test(f: Path) -> None:
        if f.resolve() not in seen:
            seen.add(f.resolve())
            sel.tests.append(f)

    for p in map(Path, paths):
        if not p.exists():
            raise UsageError(f"not a test file or directory: {p}")
        if (blocker := skipping_directory(p)) is not None:
            skip(blocker)
            continue
        if p.is_file():
            if p.suffix not in TEST_SUFFIXES:
                raise UsageError(f"not a test file or directory: {p}")
            add_test(p)
            continue
        tests: list[Path] = []
        goldens: list[Path] = []
        unrecognized: list[Path] = []
        helpers: list[Path] = []
        for dirpath, dirnames, filenames in os.walk(p):
            directory = Path(dirpath)
            if skip_reason(directory) is not None:
                skip(directory)
                dirnames[:] = []
                continue
            for name in filenames:
                f = directory / name
                if f.suffix in TEST_SUFFIXES:
                    tests.append(f)
                elif name.endswith(GOLDEN_SUFFIX):
                    goldens.append(f)
                elif looks_like_source(name):
                    (helpers if is_helper(f, p) else unrecognized).append(f)
        for f in sorted(tests):
            add_test(f)
        sel.helpers += [f for f in sorted(helpers) if f not in sel.helpers]
        sel.unrecognized += [f for f in sorted(unrecognized) if f not in sel.unrecognized]
        for g in sorted(goldens):
            if g.resolve() not in seen_goldens:
                seen_goldens.add(g.resolve())
                sel.goldens.append(g)
    return sel


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def required_env(headers: dict[Path, Expectation | HeaderError]) -> list[str]:
    """The environment variables the selected tests need."""
    names: list[str] = []
    for path, exp in headers.items():
        if not isinstance(exp, Expectation):
            continue
        if path.suffix == ".vpr":
            names += ["CERASTES_JAVA", "SILICON_JAR", "Z3_EXE"]
        elif exp.kind in VERIFY_KINDS:
            names += ["CERASTES_VERIFY"]
            if exp.kind == "rejected":
                names += ["CERASTES_EDG_GEN"]  # the tags of the expected diagnostics
        elif exp.kind == "frontend-ok":
            names += ["CPFE", "EDG_CLANG_VER_SCRAPE"]
        elif exp.kind == "golden":
            names += ["CERASTES_FE", "EDG_CLANG_VER_SCRAPE"]
        elif exp.kind == "frontend-diag":
            names += ["CERASTES_FE", "EDG_CLANG_VER_SCRAPE", "CERASTES_EDG_GEN"]
        elif exp.kind == "frontend-refused":
            names += ["CERASTES_FE", "EDG_CLANG_VER_SCRAPE"]
    return list(dict.fromkeys(names))


def needs_edg_tools(headers: dict[Path, Expectation | HeaderError]) -> bool:
    """Whether a selected test runs a tool built from the EDG tree."""
    return any(isinstance(e, Expectation)
               and (e.kind in ("frontend-ok", "golden", "frontend-diag", "frontend-refused")
                    or is_verify_test(f, e))  # `cerastes verify` is built from the EDG tree
               for f, e in headers.items())


def print_outcome(o: Outcome, verbose: bool) -> None:
    label = o.label or ("FAIL" if o.problems else "PASS")
    # PASS/FAIL/SKIP lines line up; UPDATED/UNCHANGED take one space.
    print(f"{label}{'  ' if len(label) == 4 else ' '}{o.name}")
    lines = o.problems + o.notes
    shown = lines if verbose else lines[:MAX_PROBLEM_LINES]
    for p in shown:
        print(f"      {printable(p)}")
    if len(shown) < len(lines):
        print(f"      ... {len(lines) - len(shown)} more lines (-v shows them all)")
    if o.problems and verbose and o.output:
        print("      --- output ---")
        print("\n".join("      " + printable(l) for l in o.output.splitlines()))


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("paths", nargs="*",
                        help="test files or directories (default: "
                             + " and ".join(f"{p}/" for p in DEFAULT_PATHS)
                             + "; a missing default directory is an error)")
    parser.add_argument("-j", "--jobs", type=int,
                        default=max(1, (os.cpu_count() or 2) // 2))
    parser.add_argument("-v", "--verbose", action="store_true",
                        help="print full diffs and tool output for failing tests")
    parser.add_argument("--check-headers", action="store_true",
                        help="only validate expectation headers and look for "
                             "orphan golden files")
    parser.add_argument("--count-error-twins", action="store_true",
                        help="print the number of positive tests of tests/scalar that have a "
                             "twin expecting an `error` (the acceptance count) and exit")
    parser.add_argument("--expect-passed", type=int, metavar="N",
                        help="fail unless exactly N tests passed and none failed: for a "
                             "directory whose size a generator states (check.sh pins the "
                             "VIR self-test's case count with it)")
    parser.add_argument("--update-golden", action="store_true",
                        help="write the golden files of the selected golden tests "
                             "instead of comparing (review the diff; never in "
                             "check.sh)")
    args = parser.parse_args(argv)
    if args.count_error_twins:
        print(count_error_twins())
        return 0

    try:
        sel = discover(args.paths or default_paths())
    except UsageError as err:
        print(f"run_tests.py: {err}", file=sys.stderr)
        return 2
    for directory, reason in sel.skipped.items():
        print(f"SKIP  {rel(directory)}: {reason}")
    if not (sel.tests or sel.goldens or sel.unrecognized or sel.hidden or sel.helpers):
        skipped = f" ({len(sel.skipped)} directories skipped)" if sel.skipped else ""
        print(f"run_tests.py: no tests found{skipped}", file=sys.stderr)
        return 2

    headers = {f: read_header(f) for f in sel.tests}
    orphans = find_orphan_goldens(sel.goldens, headers)
    env = load_env()
    try:
        check_diag_headers(headers, env)
    except UsageError as err:
        print(f"run_tests.py: {err}", file=sys.stderr)
        return 2

    # the twin rule looks at the whole tree of each area that a selected test is in
    cross = cross_file_problems([(area, neg) for area, neg in TWIN_AREAS
                                 if any(_under(f, area) for f in headers)])
    helper_probs = helper_problems(sel.helpers)

    if args.check_headers:
        bad = 0
        for f, exp in headers.items():
            if isinstance(exp, HeaderError):
                bad += 1
                print(f"{rel(f)}: bad header: {exp}")
        for f, message in cross:
            print(f"{rel(f)}: bad header: {message}")
        for f, message in helper_probs:
            print(f"{rel(f)}: {message}")
        for g in orphans:
            print(f"{rel(g)}: {ORPHAN_MESSAGE}")
        for f in sel.unrecognized:
            print(f"{rel(f)}: {UNRECOGNIZED_MESSAGE}")
        for f in sel.hidden:
            print(f"{rel(f)}: {HIDDEN_MESSAGE}")
        summary = (f"{len(headers) - bad} of {len(headers)} headers ok, "
                   f"{len(orphans)} orphan golden files, {len(sel.skipped)} skipped")
        if cross:
            summary += f", {len(cross)} positive tests without a must-fail twin"
        if helper_probs:
            summary += f", {len(helper_probs)} problems with helper headers"
        if sel.unrecognized:
            summary += f", {len(sel.unrecognized)} files with an unrecognized suffix"
        if sel.hidden:
            summary += f", {len(sel.hidden)} tests hidden in skipped directories"
        print(summary)
        return 1 if bad or cross or helper_probs or orphans or sel.unrecognized \
            or sel.hidden else 0

    try:
        require(env, *required_env(headers))
        if needs_edg_tools(headers) and (
                (problem := edg_tree_problem()) is not None
                or (problem := edg_stamp_problem()) is not None):
            raise UsageError(problem)
    except UsageError as err:
        print(f"run_tests.py: {err}", file=sys.stderr)
        return 2

    opts = RunOptions(verbose=args.verbose, update_golden=args.update_golden)
    outcomes: list[Outcome] = []
    verify_tests = [(f, e) for f, e in headers.items()
                    if isinstance(e, Expectation) and is_verify_test(f, e)]
    # A run that must produce Viper starts from an empty build/vpr/ with its
    # MANIFEST, so scripts/lint_viper.py can require every file of this run.
    if not args.update_golden and any(vpr_expected(e) for _, e in verify_tests):
        try:
            manifest = vpr_manifest(verify_tests)
            run_id = make_run_id(manifest)
            lock_vpr_dir()
            reset_vpr_dir(manifest, run_id)
            print(f"build/vpr: run {run_id}, {len(manifest)} files expected")
        except UsageError as err:
            print(f"run_tests.py: {err}", file=sys.stderr)
            return 2
    native_tests = [(f, e) for f, e in headers.items()
                    if isinstance(e, Expectation) and e.native_ok]
    try:
        if any(isinstance(e, Expectation) and e.diags for e in headers.values()):
            opts.registry = load_registry(env)
        if native_tests and not args.update_golden:
            opts.native, canaries = prepare_native(env, native_tests)
            if opts.native.note:
                print(opts.native.note)
            for o in canaries:
                outcomes.append(o)
                print_outcome(o, args.verbose)
    except UsageError as err:
        print(f"run_tests.py: {err}", file=sys.stderr)
        return 2
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for result in pool.map(lambda f: run_one(f, headers[f], env, opts), sel.tests):
            for o in result:
                outcomes.append(o)
                print_outcome(o, args.verbose)
    for g in orphans:
        o = Outcome(rel(g), [ORPHAN_MESSAGE])
        outcomes.append(o)
        print_outcome(o, args.verbose)
    for f, message in cross + helper_probs:
        o = Outcome(rel(f), [message])
        outcomes.append(o)
        print_outcome(o, args.verbose)
    for files, message in ((sel.unrecognized, UNRECOGNIZED_MESSAGE), (sel.hidden, HIDDEN_MESSAGE)):
        for f in files:
            o = Outcome(rel(f), [message])
            outcomes.append(o)
            print_outcome(o, args.verbose)

    failed = sum(1 for o in outcomes if o.problems)
    updated = sum(1 for o in outcomes if not o.problems and o.label == "UPDATED")
    unchanged = sum(1 for o in outcomes if not o.problems and o.label == "UNCHANGED")
    passed = len(outcomes) - failed - updated - unchanged
    summary = f"{passed} passed, {failed} failed, {len(sel.skipped)} skipped"
    if opts.native is not None:
        summary += ", native-ok runs: " + ", ".join(
            f"{c.family} {opts.native.runs.get(c.family, 0)}" for c in opts.native.compilers)
    if args.update_golden:
        summary += f", {updated} goldens updated, {unchanged} unchanged"
    print(f"\n{summary}")
    if args.expect_passed is not None and not failed and passed != args.expect_passed:
        print(f"run_tests.py: expected exactly {args.expect_passed} passing tests, "
              f"got {passed}", file=sys.stderr)
        return 1
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
