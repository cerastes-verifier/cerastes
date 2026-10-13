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
    // mode: c++20 | c23                                 (C and C++ only)

`.vpr` files run directly through Silicon.  `.c`/`.cpp` files with
`frontend-ok` run through the EDG front end (scripts/cpfe.sh); the other
expectations need the cerastes driver, which lands in M3.

A test passes only if the outcome matches exactly: every expected error is
reported with the expected category (and reason, if given) at the expected
line, and nothing else is reported.  The exit status is 1 if any test fails
and 2 on usage or environment errors.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import dataclasses
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TEST_SUFFIXES = {".vpr", ".c", ".cpp"}
SILICON_TIMEOUT_S = 600
FRONTEND_TIMEOUT_S = 120

# Language modes and the EDG options that select them.  The first mode listed
# for a suffix is its default.
MODE_OPTIONS = {"c++20": "--c++20", "c23": "--c23"}
DEFAULT_MODES = {".cpp": ["c++20"], ".c": ["c23"]}


class HeaderError(Exception):
    """A test file's expectation header is malformed."""


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
    kind: str  # "verified", "error", "rejected" or "frontend-ok"
    errors: list[ExpectedError]
    rejected: str | None
    modes: list[str]


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
        if body in ("verified", "frontend-ok"):
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
    if kind in ("verified", "frontend-ok") and len(expects) != 1:
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
# Runners
# ---------------------------------------------------------------------------

@dataclasses.dataclass
class Outcome:
    name: str
    problems: list[str]
    output: str = ""


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
        sys.exit(f"run_tests.py: {', '.join(missing)} not set; "
                 "run scripts/bootstrap.sh first")


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


def run_one(path: Path, env: dict[str, str]) -> list[Outcome]:
    try:
        exp = parse_header(path.read_text(), path.suffix)
    except HeaderError as err:
        return [Outcome(rel(path), [f"bad header: {err}"])]
    if path.suffix == ".vpr":
        return run_silicon(path, exp, env)
    return run_frontend(path, exp, env)


def rel(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(ROOT))
    except ValueError:
        return str(path)


def discover(paths: list[str]) -> list[Path]:
    files: list[Path] = []
    for p in map(Path, paths):
        if p.is_dir():
            files += sorted(f for f in p.rglob("*")
                            if f.is_file() and f.suffix in TEST_SUFFIXES)
        elif p.is_file() and p.suffix in TEST_SUFFIXES:
            files.append(p)
        else:
            sys.exit(f"run_tests.py: not a test file or directory: {p}")
    return files


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("paths", nargs="*", default=[str(ROOT / "tests")],
                        help="test files or directories (default: tests/)")
    parser.add_argument("-j", "--jobs", type=int,
                        default=max(1, (os.cpu_count() or 2) // 2))
    parser.add_argument("-v", "--verbose", action="store_true",
                        help="print tool output for failing tests")
    parser.add_argument("--check-headers", action="store_true",
                        help="only validate expectation headers")
    args = parser.parse_args(argv)

    files = discover(args.paths)
    if not files:
        print("run_tests.py: no tests found", file=sys.stderr)
        return 2

    if args.check_headers:
        bad = 0
        for f in files:
            try:
                parse_header(f.read_text(), f.suffix)
            except HeaderError as err:
                bad += 1
                print(f"{rel(f)}: bad header: {err}")
        print(f"{len(files) - bad} of {len(files)} headers ok")
        return 1 if bad else 0

    env = load_env()
    if any(f.suffix == ".vpr" for f in files):
        require(env, "CERASTES_JAVA", "SILICON_JAR", "Z3_EXE")
    if any(f.suffix != ".vpr" for f in files):
        require(env, "CPFE", "EDG_CLANG_VER_SCRAPE")

    outcomes: list[Outcome] = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for result in pool.map(lambda f: run_one(f, env), files):
            for o in result:
                outcomes.append(o)
                print(f"{'FAIL' if o.problems else 'PASS'}  {o.name}")
                for p in o.problems:
                    print(f"      {p}")
                if o.problems and args.verbose and o.output:
                    print("      --- output ---")
                    print("\n".join("      " + l for l in o.output.splitlines()))
    failed = sum(1 for o in outcomes if o.problems)
    print(f"\n{len(outcomes) - failed} passed, {failed} failed")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
