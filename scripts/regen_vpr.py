#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Regenerate the kept Viper of the test suite without running Silicon.

    python3 scripts/regen_vpr.py --out <dir> [-j N] [--cerastes <path>] [--only <path> ...]

Every `verified` or `error` test that scripts/run_tests.py would run through `cerastes verify`
(C and C++, in each of its modes, with its `// verify-options:`) is run with `--keep-vpr`, and
`<dir>/<test path>.<mode>.vpr` and its `.map` are written, the layout of build/vpr.  The test
files, their modes and the command line come from run_tests.py itself (imported), so the two
cannot drift.  The default selection is run_tests.py's: tests/ and the documentation examples;
`--only` names files or directories instead.

Silicon is not run.  The driver writes the kept files before it starts Silicon (decision 0011
item 9), and its self-test hook `silicon_timeout` runs Silicon with a timeout of zero: the
driver starts the Java process, kills it at once and exits with status 3 and the message "Silicon
did not finish within 0 seconds".  The hook is the only change to the command line, and it does
not change the encoder, so the `.vpr` and `.map` are byte for byte what `build/cerastes verify
<file> --std=<mode> --keep-vpr=<path>` keeps.

`--out` is required and must be a new or empty directory: build/vpr belongs to run_tests.py (it
empties it and holds a lock), and two runs that shared a directory would race.  A lock in
`<dir>/.regen.lock` stops a second run in the same directory.  `<dir>/MANIFEST` lists the files
expected.  Compare two such trees with scripts/vprdiff.py.

The summary counts the files expected, written and missing.  The exit status is 1 when a file
is missing (a test the front end rejects or a back-end failure leaves none) and 2 on usage or
environment errors.
"""

from __future__ import annotations

import argparse
import concurrent.futures
import datetime
import hashlib
import os
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_tests as rt  # noqa: E402

HOOK_OPTION = "--cerastes_verify_selftest=silicon_timeout"
HOOK_MESSAGE = "Silicon did not finish within 0 seconds"
HOOK_STATUS = 3
LOCK_NAME = ".regen.lock"
MANIFEST = "MANIFEST"
RUN_TIMEOUT_S = rt.VERIFY_TIMEOUT_S


def select(paths: list[str]) -> list[tuple[Path, rt.Expectation]]:
    """The tests whose `cerastes verify` run keeps Viper, sorted by path: run_tests.py's own
    selection (discover, read_header, is_verify_test, vpr_expected), without the tests of
    another platform (which run_tests.py skips)."""
    sel = rt.discover(paths or rt.default_paths())
    chosen = []
    for f in sorted(sel.tests):
        exp = rt.read_header(f)
        if not isinstance(exp, rt.Expectation):
            continue  # a bad header: run_tests.py reports it, there is no Viper to regenerate
        if exp.platform not in (None, rt.HOST_PLATFORM):
            continue
        if rt.is_verify_test(f, exp) and rt.vpr_expected(exp):
            chosen.append((f, exp))
    return chosen


def out_path(out: Path, f: Path, mode: str) -> Path:
    """Where the Viper of `f` in `mode` goes: run_tests.py's name below `out` instead of build/vpr."""
    return out / rt.vpr_path(f, mode).relative_to(rt.VPR_DIR)


def lock_out_dir(out: Path):
    """The exclusive, non-blocking lock on `out` for the rest of the process (returned: the caller keeps it open)."""
    import fcntl
    handle = open(out / LOCK_NAME, "a+")
    try:
        fcntl.flock(handle, fcntl.LOCK_EX | fcntl.LOCK_NB)
    except OSError:
        handle.close()
        raise rt.UsageError(f"another regen_vpr.py is using {out}; give each run its own --out") from None
    handle.seek(0)
    handle.truncate()
    handle.write(f"{os.getpid()}\n")
    handle.flush()
    return handle


def regen_one(f: Path, mode: str, exp: rt.Expectation, out: Path, env: dict[str, str]) -> tuple[Path, list[str]]:
    """One run; the file written and the problems with it (empty: written, and what run_tests.py
    requires of a kept file holds)."""
    vpr = out_path(out, f, mode)
    for g in (vpr, vpr.with_name(vpr.name + ".map")):
        g.unlink(missing_ok=True)
    vpr.parent.mkdir(parents=True, exist_ok=True)
    cmd = rt.verify_command(env, f, mode, vpr, [*exp.verify_options, HOOK_OPTION])
    run = rt.run_tool(cmd, env, RUN_TIMEOUT_S)
    problems = []
    if run.returncode != HOOK_STATUS or HOOK_MESSAGE not in run.stderr:
        tail = (run.error or (run.stderr.strip().splitlines() or ["no output"])[-1])
        problems.append(f"driver status {run.returncode}, not {HOOK_STATUS} with the hook's message: {tail}")
    problems += rt.vpr_problems(vpr, rt.rel(f), [rt.rel(h) for h in rt.helper_bodies_included(f)])
    return vpr, problems


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--out", required=True, metavar="DIR",
                        help="where to write <test path>.<mode>.vpr and .map (a new or empty directory)")
    parser.add_argument("-j", "--jobs", type=int, default=None,
                        help=f"parallel runs (default: ${rt.JOBS_VARIABLE} from the shell if set, else half the CPUs)")
    parser.add_argument("--cerastes", metavar="PATH", default=None,
                        help="the cerastes program (default: CERASTES_VERIFY of build/env.sh)")
    parser.add_argument("--only", nargs="+", metavar="PATH", default=None,
                        help="test files or directories (default: tests/ and the doc examples)")
    args = parser.parse_args(argv)
    try:
        out = Path(args.out).resolve()
        if out == rt.VPR_DIR.resolve() or rt.VPR_DIR.resolve() in out.parents or out in rt.VPR_DIR.resolve().parents:
            raise rt.UsageError(f"{rt.rel(out)} overlaps build/vpr, which run_tests.py owns; choose another --out")
        if out.exists() and (not out.is_dir() or any(out.iterdir())):
            raise rt.UsageError(f"{rt.rel(out)} exists and is not an empty directory; each run takes its own")
        env = rt.load_env()
        if args.cerastes is not None:
            env["CERASTES_VERIFY"] = str(Path(args.cerastes).resolve())
        rt.require(env, "CERASTES_VERIFY")
        if not os.access(env["CERASTES_VERIFY"], os.X_OK):
            raise rt.UsageError(f"{env['CERASTES_VERIFY']} is not an executable file")
        if args.jobs is None:
            args.jobs = rt.default_jobs()
        if args.jobs < 1:
            raise rt.UsageError("-j needs a positive number")
        if args.cerastes is None and ((problem := rt.edg_tree_problem()) is not None
                                      or (problem := rt.edg_stamp_problem()) is not None):
            raise rt.UsageError(problem)
        tests = select(args.only or [])
        if not tests:
            raise rt.UsageError("no test that keeps Viper is selected")
        out.mkdir(parents=True, exist_ok=True)
        lock = lock_out_dir(out)
    except rt.UsageError as err:
        print(f"regen_vpr.py: {err}", file=sys.stderr)
        return 2

    jobs = [(f, m, e) for f, e in tests for m in e.modes]
    manifest = sorted(out_path(out, f, m).relative_to(out).as_posix() for f, m, _ in jobs)
    digest = hashlib.sha256("\n".join(manifest).encode()).hexdigest()[:8]
    run_id = f"{datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%SZ')}-{digest}"
    (out / MANIFEST).write_text(f"{rt.RUN_ID_PREFIX}{run_id}\n" + "".join(f"{line}\n" for line in manifest))
    print(f"{rt.rel(out)}: run {run_id}, {len(manifest)} files expected, -j {args.jobs}")

    started = time.monotonic()
    written = 0
    missing: list[tuple[str, list[str]]] = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = {pool.submit(regen_one, f, m, e, out, env): (f, m) for f, m, e in jobs}
        for done in concurrent.futures.as_completed(futures):
            f, m = futures[done]
            _, problems = done.result()
            if problems:
                missing.append((f"{rt.rel(f)} [{m}]", problems))
            else:
                written += 1
    for name, problems in sorted(missing):
        print(f"MISSING {name}")
        for p in problems[:rt.MAX_PROBLEM_LINES]:
            print(f"      {p}")
    print(f"\nfiles expected {len(jobs)}, written {written}, missing {len(missing)} "
          f"({time.monotonic() - started:.0f} s at -j {args.jobs})")
    lock.close()
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(main())
