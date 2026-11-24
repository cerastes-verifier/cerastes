#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Record what every rejection test of the tree reports, and diff two records.

    python3 scripts/rejections.py <out-file> [--base <file>] [--bin-dir <dir>] [-j N] [--timeout S]

It runs every `rejected` test under tests/scalar/negative, tests/memory/negative,
tests/vlibc/negative and tests/annotations/negative (`cerastes verify`, exit status 2) and
every `frontend-error` test under tests/annotations (the front end, exit status 2), in each
mode the test declares, and writes what the BINARIES printed, never what the headers say
(a record of the headers could not differ from itself).  One line per error diagnostic of a
run, tab-separated and sorted:

    <test file>  <mode>  <tag>  <line>

`<tag>` is the diagnostic's EDG tag or `cerastes-<tag>`; `<line>` is the line in the test
file, or `<repository-relative header>:<line>` for an error in an included file.  A run that
does not end in a rejection is recorded as what it did, so that a lifted rejection shows up
in the diff: tag `<verified>` (line 0) for exit status 0, `<error:<category>>` for each
verification error of exit status 1, `<frontend-ok>` (line 0) for a front end that accepts.
Every such run is also listed as a MISMATCH with the header (the exit status of the tool is
then 1, with the record complete); so is a rejection that differs from the test's expected
diagnostics.

The tool FAILS (exit status 1, every failing run listed, the out-file not written) on a
timeout, a crash, exit status 3, any other exit status, output that does not parse (stdout
of a rejection, an unknown format, a fatal diagnostic, a summary that disagrees with the
errors printed, a rejection with no error), or a test whose header is bad.  Exit status 2:
usage errors (a missing binary, a missing directory, no run at all).

`--bin-dir` (default `build/`) names the directory of `cerastes` and, for the front-end tests,
`cerastes-fe.sh` (the default directory without one uses scripts/cerastes-fe.sh, which runs
build/cerastes-fe; any other directory without a wrapper is a usage error, so that a record
never silently comes from a different front end).  The tool never writes to or locks build/vpr
(no `--keep-vpr`), and never builds.  With `--base <file>` it prints what differs from an
earlier record (added, removed, changed, per test and mode) and exits 1 on any difference.
It prints the counts per directory.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import dataclasses
import os
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_tests as rt  # noqa: E402

# the directories of `rejected` tests, and the directory of the `frontend-error` tests
REJECTED_DIRS = (Path("tests/scalar/negative"), Path("tests/memory/negative"),
                 Path("tests/vlibc/negative"), Path("tests/annotations/negative"))
FRONTEND_DIR = Path("tests/annotations")
VERIFIED = "<verified>"
FRONTEND_OK = "<frontend-ok>"
FE_WRAPPER = "cerastes-fe.sh"
FIELDS = 4


class ToolUsageError(Exception):
    """A usage or environment error (exit status 2)."""


@dataclasses.dataclass(frozen=True)
class Run:
    path: Path
    mode: str
    kind: str  # "rejected" (cerastes verify) or "frontend" (the front end)
    group: str  # the directory line of the counts
    exp: rt.Expectation


@dataclasses.dataclass
class Result:
    run: Run
    failure: str | None = None  # the tool fails, nothing is recorded
    lines: list[tuple[str, str, str, int, str]] = dataclasses.field(default_factory=list)
    mismatch: list[str] = dataclasses.field(default_factory=list)


def group_name(kind: str, directory: Path) -> str:
    return f"{directory.as_posix()} ({'rejected' if kind == 'rejected' else 'frontend-error'})"


def collect(root: Path | None = None) -> tuple[list[Run], list[str], list[str]]:
    """The runs, the notes on what was left out (skipped directories, tests of another
    platform, tests with no error to expect) and the failures (bad headers, missing
    directories, a directory with a skip marker)."""
    root = rt.ROOT if root is None else root
    runs: list[Run] = []
    notes: list[str] = []
    failures: list[str] = []
    seen: set[Path] = set()
    for kind, directory, wanted in [("rejected", d, "rejected") for d in REJECTED_DIRS] + \
            [("frontend", FRONTEND_DIR, "frontend-diag")]:
        if not (root / directory).is_dir():
            failures.append(f"{directory}: the directory is missing")
            continue
        sel = rt.discover([str(root / directory)])
        for skipped, reason in sel.skipped.items():
            notes.append(f"{rt.rel(skipped)}: skipped by its marker ({reason})")
        for f in sel.tests:
            exp = rt.read_header(f)
            if isinstance(exp, rt.HeaderError):
                failures.append(f"{rt.rel(f)}: bad header: {exp}")
                continue
            if exp.kind != wanted:
                continue
            if kind == "frontend" and not any(d.severity == "error" for d in exp.diags):
                notes.append(f"{rt.rel(f)}: a front-end test with no expected error: not run")
                continue
            if exp.platform not in (None, rt.HOST_PLATFORM):
                notes.append(f"{rt.rel(f)}: runs on {exp.platform} only: not run here")
                continue
            if f.resolve() in seen:
                continue
            seen.add(f.resolve())
            for mode in exp.modes:
                runs.append(Run(f, mode, kind, group_name(kind, directory), exp))
    return runs, notes, failures


def front_end_command(bin_dir: Path, root: Path) -> list[str]:
    wrapper = bin_dir / FE_WRAPPER
    if wrapper.is_file():
        return [str(wrapper)]
    if bin_dir.resolve() == (root / "build").resolve():
        return [str(root / "scripts" / FE_WRAPPER)]
    raise ToolUsageError(f"{bin_dir} has no {FE_WRAPPER}; the front-end tests need the wrapper of "
                         "the front end under test (scripts/cerastes-fe.sh would run build/cerastes-fe)")


def command(run: Run, bin_dir: Path, root: Path) -> list[str]:
    shown = rt.rel(run.path)
    if run.kind == "rejected":
        return [str(bin_dir / "cerastes"), "verify", shown, rt.VERIFY_STD_OPTIONS[run.mode],
                *run.exp.verify_options]
    return [*front_end_command(bin_dir, root), rt.MODE_OPTIONS[run.mode], *rt.DIAG_OPTIONS, shown]


def tag_lines(diags: list[rt.FeDiag], run: Run, registry: rt.TagRegistry):
    shown = rt.rel(run.path)
    out = []
    for d in diags:
        if d.severity != "error":
            continue
        line = d.line if d.line is not None else 0
        where = "" if d.file is None or rt.same_file(d.file, shown) else f"{d.file}:"
        out.append((shown, run.mode, d.tag(registry), line, where))
    return out


def evaluate(run: Run, proc: "rt.ToolRun", registry: rt.TagRegistry) -> Result:
    """The record of one run, or its failure."""
    shown = rt.rel(run.path)
    res = Result(run)
    if proc.returncode is None:
        res.failure = f"{run.mode}: {proc.error}"
        return res
    if proc.returncode < 0:
        res.failure = f"{run.mode}: crashed (signal {-proc.returncode})"
        return res
    code = proc.returncode
    if code == rt.VERIFY_TOOL_FAILURE and run.kind == "rejected":
        res.failure = f"{run.mode}: exit status 3 (a tool failure): {rt._verify_tail(proc)}"
        return res
    if run.kind == "rejected":
        return evaluate_verify_run(run, proc, registry, res, shown)
    return evaluate_front_end_run(run, proc, registry, res, shown)


def parsed_diags(run: Run, proc: "rt.ToolRun", res: Result) -> list[rt.FeDiag] | None:
    """The diagnostics of stderr, or None (and `res.failure`) if it does not parse."""
    parsed = rt.parse_fe_diagnostics(proc.stderr)
    problems = list(parsed.problems)
    problems += [f"{d.severity} always fails: {d.message}" for d in parsed.diags
                 if d.severity in rt._FE_FATAL]
    errors = sum(1 for d in parsed.diags if d.severity == "error")
    if parsed.summary_errors is None and errors:
        problems.append(f"{errors} errors printed but no `N errors detected` summary")
    elif parsed.summary_errors is not None and parsed.summary_errors != errors:
        problems.append(f"the summary says {parsed.summary_errors} errors but {errors} were parsed")
    if problems:
        res.failure = f"{run.mode}: output does not parse: " + " | ".join(problems)[:400]
        return None
    return parsed.diags


def evaluate_verify_run(run: Run, proc: "rt.ToolRun", registry: rt.TagRegistry, res: Result,
                        shown: str) -> Result:
    code = proc.returncode
    if code == rt.VERIFY_REJECTED:
        if proc.stdout:
            res.failure = (f"{run.mode}: a rejection printed on stdout: "
                           f"{proc.stdout.strip().splitlines()[0][:200]}")
            return res
        diags = parsed_diags(run, proc, res)
        if diags is None:
            return res
        res.lines = tag_lines(diags, run, registry)
        if not res.lines:
            res.failure = f"{run.mode}: exit status 2 but no error parsed: {rt._verify_tail(proc)}"
            res.lines = []
            return res
        res.mismatch = rt.compare_diags(run.exp.diags, diags, registry, shown)
        return res
    if code in (rt.VERIFY_OK, rt.VERIFY_ERRORS):
        parsed = rt.parse_verify_output(proc.stdout, proc.stderr)
        if parsed.problems:
            res.failure = f"{run.mode}: output does not parse: " + " | ".join(parsed.problems)[:400]
            return res
        if code == rt.VERIFY_OK:
            if len(parsed.verified) != 1 or parsed.errors:
                res.failure = f"{run.mode}: exit status 0 without exactly one `verified` line"
                return res
            res.lines = [(shown, run.mode, VERIFIED, 0, "")]
            res.mismatch = [f"expected a rejection, but the program verified (exit status 0)"]
        else:
            if not parsed.errors or parsed.verified:
                res.failure = f"{run.mode}: exit status 1 without errors (or with a `verified` line)"
                return res
            res.lines = [(shown, run.mode, f"<error:{e.category}>", e.line or 0,
                          "" if rt.same_file(e.file, shown) else f"{e.file}:")
                         for e in parsed.errors]
            res.mismatch = ["expected a rejection, but verification errors were reported "
                            "(exit status 1)"]
        return res
    res.failure = f"{run.mode}: exit status {code}, not one of 0, 1, 2: {rt._verify_tail(proc)}"
    return res


def evaluate_front_end_run(run: Run, proc: "rt.ToolRun", registry: rt.TagRegistry, res: Result,
                           shown: str) -> Result:
    code = proc.returncode
    if proc.stdout:
        res.failure = (f"{run.mode}: the front end printed on stdout: "
                       f"{proc.stdout.strip().splitlines()[0][:200]}")
        return res
    if code not in (0, rt.EDG_ERRORS_STATUS):
        res.failure = f"{run.mode}: front end exit status {code}: {rt._verify_tail(proc)}"
        return res
    diags = parsed_diags(run, proc, res)
    if diags is None:
        return res
    errors = tag_lines(diags, run, registry)
    if code == rt.EDG_ERRORS_STATUS and not errors:
        res.failure = f"{run.mode}: exit status 2 but no error parsed: {rt._verify_tail(proc)}"
        return res
    if code == 0 and errors:
        res.failure = f"{run.mode}: exit status 0 but errors were printed"
        return res
    res.lines = errors if errors else [(shown, run.mode, FRONTEND_OK, 0, "")]
    res.mismatch = rt.compare_diags(run.exp.diags, diags, registry, shown)
    return res


def execute(run: Run, bin_dir: Path, env: dict[str, str], registry: rt.TagRegistry, timeout: int,
            root: Path | None = None) -> Result:
    root = rt.ROOT if root is None else root
    proc = rt.run_tool(command(run, bin_dir, root), env, timeout)
    return evaluate(run, proc, registry)


def record_line(entry: tuple[str, str, str, int, str]) -> str:
    shown, mode, tag, line, where = entry
    return f"{shown}\t{mode}\t{tag}\t{where}{line}"


def sort_key(entry: tuple[str, str, str, int, str]):
    shown, mode, tag, line, where = entry
    return (shown, mode, where, line, tag)


def render(results: list[Result]) -> list[str]:
    entries = [e for r in results for e in r.lines]
    return [record_line(e) for e in sorted(entries, key=sort_key)]


def read_record(path: Path) -> list[str]:
    try:
        text = path.read_text()
    except OSError as err:
        raise ToolUsageError(f"cannot read {path}: {err}") from err
    lines = text.split("\n")
    if lines and lines[-1] == "":
        lines.pop()
    for n, line in enumerate(lines, 1):
        if line.count("\t") != FIELDS - 1:
            raise ToolUsageError(f"{path}:{n}: not a record line (four tab-separated fields)")
    return lines


def by_run(lines: list[str]) -> dict[tuple[str, str], list[str]]:
    groups: dict[tuple[str, str], list[str]] = {}
    for line in lines:
        shown, mode, _tag, _line = line.split("\t")
        groups.setdefault((shown, mode), []).append(line)
    return groups


def differences(base: list[str], new: list[str]) -> list[str]:
    """What differs between two records, per test and mode: empty when they are equal."""
    old, cur = by_run(base), by_run(new)
    out: list[str] = []
    for key in sorted(set(old) | set(cur)):
        if old.get(key) == cur.get(key):
            continue
        shown, mode = key
        if key not in old:
            kind = "added"
        elif key not in cur:
            kind = "removed"
        else:
            kind = "changed"
        out.append(f"{kind}: {shown} [{mode}]")
        for line in old.get(key, []):
            if line not in cur.get(key, []):
                out.append(f"  - {line.split(chr(9), 2)[2]}")
        for line in cur.get(key, []):
            if line not in old.get(key, []):
                out.append(f"  + {line.split(chr(9), 2)[2]}")
    return out


def write_record(path: Path, lines: list[str]) -> None:
    """Write whole or not at all: the record is trusted by what reruns it."""
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp = tempfile.mkstemp(dir=path.parent, prefix=path.name + ".", suffix=".tmp")
    try:
        with os.fdopen(fd, "w") as f:
            f.write("".join(l + "\n" for l in lines))
        os.replace(tmp, path)
    except BaseException:
        Path(tmp).unlink(missing_ok=True)
        raise


def counts(results: list[Result]) -> list[str]:
    groups: dict[str, list[int]] = {}
    for d in [group_name("rejected", d) for d in REJECTED_DIRS] + \
            [group_name("frontend", FRONTEND_DIR)]:
        groups[d] = [0, 0]
    for r in results:
        g = groups.setdefault(r.run.group, [0, 0])
        g[0] += 1
        g[1] += len(r.lines)
    out = [f"{name}: {n} runs, {lines} lines" for name, (n, lines) in groups.items()]
    out.append(f"total: {sum(g[0] for g in groups.values())} runs, "
               f"{sum(g[1] for g in groups.values())} lines")
    return out


def main(argv: list[str] | None = None, env: dict[str, str] | None = None,
         registry: rt.TagRegistry | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("out_file", help="the record to write")
    parser.add_argument("--base", metavar="FILE", help="an earlier record: print what differs, "
                        "exit 1 on any difference")
    parser.add_argument("--bin-dir", metavar="DIR", default=None,
                        help="the directory of cerastes (and cerastes-fe.sh); default build/")
    parser.add_argument("-j", "--jobs", type=int, default=None)
    parser.add_argument("--timeout", type=int, default=rt.VERIFY_TIMEOUT_S,
                        help="seconds per run (default %(default)s); a timeout fails the tool")
    args = parser.parse_args(argv)
    try:
        root = rt.ROOT
        # resolved once, against the caller's directory: the runs themselves start in the repository root
        bin_dir = Path(args.bin_dir).resolve() if args.bin_dir else root / "build"
        if not (bin_dir / "cerastes").is_file():
            raise ToolUsageError(f"{bin_dir / 'cerastes'} does not exist")
        base = read_record(Path(args.base)) if args.base else None
        env = rt.load_env() if env is None else env
        if registry is None:
            registry = rt.load_registry(env)
        jobs = args.jobs if args.jobs is not None else rt.default_jobs()
        runs, notes, failures = collect(root)
        if failures:
            raise ToolUsageError("\n".join(failures))
        if not runs:
            raise ToolUsageError("no rejection test found: the record would be empty")
        if any(r.kind == "frontend" for r in runs):
            front_end_command(bin_dir, root)
    except (ToolUsageError, rt.UsageError) as err:
        print(f"rejections.py: {err}", file=sys.stderr)
        return 2
    for note in notes:
        print(f"note: {note}")
    with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as pool:
        results = list(pool.map(lambda r: execute(r, bin_dir, env, registry, args.timeout, root), runs))
    for line in counts(results):
        print(line)
    failed = [r for r in results if r.failure is not None]
    if failed:
        print(f"rejections.py: FAILED, {len(failed)} of {len(results)} runs; no record written:",
              file=sys.stderr)
        for r in failed:
            print(f"  {rt.rel(r.run.path)}: {r.failure}", file=sys.stderr)
        return 1
    lines = render(results)
    write_record(Path(args.out_file), lines)
    print(f"wrote {args.out_file}: {len(lines)} lines")
    status = 0
    mismatched = [r for r in results if r.mismatch]
    if mismatched:
        status = 1
        print(f"MISMATCH with the header in {len(mismatched)} runs (the record is complete):",
              file=sys.stderr)
        for r in mismatched:
            print(f"  {rt.rel(r.run.path)} [{r.run.mode}]:", file=sys.stderr)
            for m in r.mismatch:
                print(f"      {m}", file=sys.stderr)
    if base is not None:
        diff = differences(base, lines)
        if diff:
            status = 1
            print(f"DIFFERS from {args.base}:")
            for d in diff:
                print(d)
        else:
            print(f"identical to {args.base}")
    return status


if __name__ == "__main__":
    sys.exit(main())
