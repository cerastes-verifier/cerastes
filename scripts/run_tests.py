#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""cerastes test harness.

Every test file starts with a header of `//` comment lines carrying its
expectations (grammar in tests/README.md):

    // expect: verified
    // expect: error <category>[:<reason>] @ L<line>     (one line per error)
    // expect: rejected <tag>
    // expect: frontend-ok
    // expect: golden
    // mode: c++20 | c23                                 (C and C++ only)

`.vpr` files run directly through Silicon.  `.c`/`.cpp` files with
`frontend-ok` run through the stock EDG front end (scripts/cpfe.sh).
`.c`/`.cpp` files with `golden` run through the cerastes front end with its IL
walker (scripts/cerastes-fe.sh <mode option> --cerastes_walk <file>, from the
repository root), and its output must equal the golden file byte for byte:
stdout, then, if stderr is not empty, a line `--- stderr ---` and stderr.
The golden file is `<test>.golden` for a test with one mode and
`<test>.<mode>.golden` for each mode of a test with several.  The other
expectations need the cerastes driver, which lands in M3.

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

The exit status is 1 if any test fails and 2 on usage or environment errors
(including when no test is selected).
"""

from __future__ import annotations

import argparse
import concurrent.futures
import dataclasses
import difflib
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TEST_SUFFIXES = {".vpr", ".c", ".cpp"}
GOLDEN_SUFFIX = ".golden"
# Suffixes (lower case) that mark a file as C/C++/Viper source.  A file under a
# scanned directory with one of these that is not in TEST_SUFFIXES (exactly,
# so `.CPP` and `.C` count) would silently never run: an error instead.
SOURCE_LIKE_SUFFIXES = {".c", ".cc", ".cp", ".cpp", ".cxx", ".c++", ".h", ".hh", ".hpp",
                        ".hxx", ".h++", ".ipp", ".inl", ".tcc", ".i", ".ii", ".sil", ".vpr"}
SKIP_MARKER = ".harness-skip"
# The only directories where a skip marker is permitted, relative to ROOT.  A
# marker anywhere else is an error, so that nobody can turn a directory of
# real tests green by dropping a file into it.  Adding a directory here is a
# reviewed change (and needs a reason in the commit).
SKIP_ALLOWED = [Path("tests/differential/verifast")]
SILICON_TIMEOUT_S = 600
FRONTEND_TIMEOUT_S = 120

# The command that runs the cerastes front end; the mode option, the walker
# option and the test path are appended.  The self-test replaces it with a
# fake front end.
FE_COMMAND = [str(ROOT / "scripts" / "cerastes-fe.sh")]
WALK_OPTION = "--cerastes_walk"
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

    def __str__(self) -> str:
        reason = f":{self.reason}" if self.reason else ""
        return f"{self.category}{reason} @ L{self.line}"


@dataclasses.dataclass
class Expectation:
    kind: str  # "verified", "error", "rejected", "frontend-ok" or "golden"
    errors: list[ExpectedError]
    rejected: str | None
    modes: list[str]


# Kinds that stand alone: each must be the only `expect:` line of its test.
_SOLE_KINDS = ("verified", "frontend-ok", "golden")

_EXPECT_RE = re.compile(r"^//\s*expect:\s*(.*?)\s*$")
_MODE_RE = re.compile(r"^//\s*mode:\s*(.*?)\s*$")
_ERROR_RE = re.compile(
    r"^error\s+(?P<category>[a-z][a-z0-9-]*)(?::(?P<reason>[a-z][a-z0-9.]*))?"
    r"\s+@\s+L(?P<line>[1-9][0-9]*)$")
_REJECTED_RE = re.compile(r"^rejected\s+(?P<tag>[a-z][a-z0-9-]*)$")


def parse_header(text: str, suffix: str) -> Expectation:
    """Parse the expectation header of a test file with the given suffix."""
    expects: list[tuple[int, str]] = []
    modes: list[str] = []
    in_header = True
    for lineno, raw in enumerate(text.splitlines(), start=1):
        line = raw.strip()
        if in_header and (not line or line.startswith("//")):
            if m := _EXPECT_RE.match(line):
                expects.append((lineno, m.group(1)))
            elif m := _MODE_RE.match(line):
                modes.extend(s.strip() for s in m.group(1).split(","))
            continue
        in_header = False
        if _EXPECT_RE.match(line) or _MODE_RE.match(line):
            raise HeaderError(f"L{lineno}: expect:/mode: lines must be in the "
                              "leading comment block")

    if not expects:
        raise HeaderError("no `// expect:` line in the leading comment block")

    errors: list[ExpectedError] = []
    kinds: set[str] = set()
    rejected = None
    for lineno, body in expects:
        if body in _SOLE_KINDS:
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
            errors.append(ExpectedError(category, reason, int(m.group("line"))))
        elif m := _REJECTED_RE.match(body):
            kinds.add("rejected")
            if rejected is not None:
                raise HeaderError(f"L{lineno}: more than one `rejected` line")
            rejected = m.group("tag")
        else:
            raise HeaderError(f"L{lineno}: cannot parse `expect: {body}`")
    if len(kinds) != 1:
        raise HeaderError("expectations mix " + ", ".join(sorted(kinds)) +
                          "; a test has exactly one kind of outcome")
    (kind,) = kinds
    if kind in _SOLE_KINDS and len(expects) != 1:
        raise HeaderError(f"`expect: {kind}` must be the only expect line")

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
        modes = modes or DEFAULT_MODES[suffix]
    return Expectation(kind, errors, rejected, modes)


def read_header(path: Path) -> Expectation | HeaderError:
    """The expectation of a test file, or the error that makes its header bad."""
    try:
        return parse_header(path.read_text(), path.suffix)
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
    (r"Function \S+ might not terminate\.", "termination.failed", "termination"),
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
_SUCCESS_RE = re.compile(r"^Silicon finished verification successfully", re.MULTILINE)
_FOUND_RE = re.compile(r"^Silicon found (\d+) errors? in", re.MULTILINE)


@dataclasses.dataclass(frozen=True)
class ReportedError:
    category: str
    reason: str
    line: int
    file: str
    message: str

    def __str__(self) -> str:
        return f"{self.category}:{self.reason} @ L{self.line}: {self.message}"


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


def parse_silicon_output(output: str, returncode: int) -> SiliconResult:
    entries: list[str] = []
    for raw in output.splitlines():
        if m := _ENTRY_RE.match(raw):
            entries.append(m.group(2))
        elif entries and raw.strip():
            entries[-1] += "\n" + raw.strip()

    errors = []
    for entry in entries:
        category, reason, file, line = classify(entry)
        errors.append(ReportedError(category, reason, line or 0, file or "?",
                                    " ".join(entry.split())))

    success = _SUCCESS_RE.search(output)
    found = _FOUND_RE.search(output)
    problem = None
    if success and not found and not entries and returncode == 0:
        return SiliconResult(True, [], None)
    if not found or returncode != 1:
        problem = f"Silicon did not produce a verdict (exit status {returncode})"
    elif int(found.group(1)) != len(errors):
        problem = (f"Silicon reported {found.group(1)} errors but "
                   f"{len(errors)} were parsed")
    return SiliconResult(False, errors, problem)


def compare_errors(expected: list[ExpectedError], reported: list[ReportedError],
                   test_file: str) -> list[str]:
    """Return mismatch descriptions; empty when the outcome is exactly right."""
    problems = []
    remaining = list(reported)
    for r in reported:
        if r.file != test_file:
            problems.append(f"error reported in another file ({r.file}): {r}")
            remaining.remove(r)
        elif r.category == UNKNOWN or r.reason in (UNKNOWN, TIMEOUT):
            what = "solver timeout" if r.reason == TIMEOUT else "unrecognized error"
            problems.append(f"{what}: {r.message}")
            remaining.remove(r)

    def matches(e: ExpectedError, r: ReportedError) -> bool:
        return (e.category == r.category and e.line == r.line
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
        same_kind = next((r for r in remaining if r.category == e.category
                          and (e.reason is None or e.reason == r.reason)), None)
        same_line = next((r for r in remaining if r.line == e.line), None)
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


def load_env() -> dict[str, str]:
    """The environment with build/env.sh applied (without a shell)."""
    env = dict(os.environ)
    env_sh = ROOT / "build" / "env.sh"
    if env_sh.exists():
        for line in env_sh.read_text().splitlines():
            if m := re.match(r'^export (\w+)="(.*)"$', line):
                env.setdefault(m.group(1), m.group(2))
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
                f"`expect: {exp.kind}` needs the cerastes driver (M3)"]))
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


def run_one(path: Path, exp: Expectation | HeaderError, env: dict[str, str],
            opts: RunOptions) -> list[Outcome]:
    if isinstance(exp, HeaderError):
        return [Outcome(rel(path), [f"bad header: {exp}"])]
    if path.suffix == ".vpr":
        return run_silicon(path, exp, env)
    if exp.kind == "golden":
        return run_golden(path, exp, env, opts)
    return run_frontend(path, exp, env)


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


UNRECOGNIZED_MESSAGE = ("looks like a C/C++/Viper source file, but its suffix is not a "
                        "test suffix (" + ", ".join(sorted(TEST_SUFFIXES)) + "), so it "
                        "would never run; rename it or remove it")
HIDDEN_MESSAGE = ("has an `// expect:` header but is in a skipped directory, so it "
                  "would never run; move it out of the skipped directory or remove "
                  "the header")


def looks_like_source(name: str) -> bool:
    """A source-like file name that is not a test file name."""
    suffix = Path(name).suffix
    return suffix not in TEST_SUFFIXES and suffix.lower() in SOURCE_LIKE_SUFFIXES


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
                    unrecognized.append(f)
        for f in sorted(tests):
            add_test(f)
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
        elif exp.kind == "frontend-ok":
            names += ["CPFE", "EDG_CLANG_VER_SCRAPE"]
        elif exp.kind == "golden":
            names += ["CERASTES_FE", "EDG_CLANG_VER_SCRAPE"]
    return list(dict.fromkeys(names))


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
    parser.add_argument("paths", nargs="*", default=[str(ROOT / "tests")],
                        help="test files or directories (default: tests/)")
    parser.add_argument("-j", "--jobs", type=int,
                        default=max(1, (os.cpu_count() or 2) // 2))
    parser.add_argument("-v", "--verbose", action="store_true",
                        help="print full diffs and tool output for failing tests")
    parser.add_argument("--check-headers", action="store_true",
                        help="only validate expectation headers and look for "
                             "orphan golden files")
    parser.add_argument("--update-golden", action="store_true",
                        help="write the golden files of the selected golden tests "
                             "instead of comparing (review the diff; never in "
                             "check.sh)")
    args = parser.parse_args(argv)

    try:
        sel = discover(args.paths)
    except UsageError as err:
        print(f"run_tests.py: {err}", file=sys.stderr)
        return 2
    for directory, reason in sel.skipped.items():
        print(f"SKIP  {rel(directory)}: {reason}")
    if not sel.tests and not sel.goldens and not sel.unrecognized and not sel.hidden:
        skipped = f" ({len(sel.skipped)} directories skipped)" if sel.skipped else ""
        print(f"run_tests.py: no tests found{skipped}", file=sys.stderr)
        return 2

    headers = {f: read_header(f) for f in sel.tests}
    orphans = find_orphan_goldens(sel.goldens, headers)

    if args.check_headers:
        bad = 0
        for f, exp in headers.items():
            if isinstance(exp, HeaderError):
                bad += 1
                print(f"{rel(f)}: bad header: {exp}")
        for g in orphans:
            print(f"{rel(g)}: {ORPHAN_MESSAGE}")
        for f in sel.unrecognized:
            print(f"{rel(f)}: {UNRECOGNIZED_MESSAGE}")
        for f in sel.hidden:
            print(f"{rel(f)}: {HIDDEN_MESSAGE}")
        summary = (f"{len(headers) - bad} of {len(headers)} headers ok, "
                   f"{len(orphans)} orphan golden files, {len(sel.skipped)} skipped")
        if sel.unrecognized:
            summary += f", {len(sel.unrecognized)} files with an unrecognized suffix"
        if sel.hidden:
            summary += f", {len(sel.hidden)} tests hidden in skipped directories"
        print(summary)
        return 1 if bad or orphans or sel.unrecognized or sel.hidden else 0

    env = load_env()
    try:
        require(env, *required_env(headers))
    except UsageError as err:
        print(f"run_tests.py: {err}", file=sys.stderr)
        return 2

    opts = RunOptions(verbose=args.verbose, update_golden=args.update_golden)
    outcomes: list[Outcome] = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for result in pool.map(lambda f: run_one(f, headers[f], env, opts), sel.tests):
            for o in result:
                outcomes.append(o)
                print_outcome(o, args.verbose)
    for g in orphans:
        o = Outcome(rel(g), [ORPHAN_MESSAGE])
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
    if args.update_golden:
        summary += f", {updated} goldens updated, {unchanged} unchanged"
    print(f"\n{summary}")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
