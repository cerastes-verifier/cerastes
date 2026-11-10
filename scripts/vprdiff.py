#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Compare two trees of kept Viper byte for byte.

    python3 scripts/vprdiff.py <base_dir> <new_dir> [--allow <file>] [--show <test>]

Each directory holds `<test path>.<mode>.vpr` and `<test path>.<mode>.vpr.map` files, as
scripts/regen_vpr.py writes them (or build/vpr after a run of scripts/run_tests.py).  The tool
lists the files only in the base, the files only in the new tree and the files that differ,
by test path (the `.vpr` and its `.map` are one entry, `<test path>.<mode>`).

`--allow <file>` names the tests whose differences are expected: one pattern per line, `#` starts a
comment, blank lines are skipped.  A pattern is a repository-relative test path (`tests/memory/x.c`:
both modes; `tests/memory/x.c.c23`: one mode) or a glob (`tests/memory/owning_*`, `**` and `*`
cross directories), matched against the test path with and without the mode.  The allowed and the
unexpected differences are printed separately, with counts.  The exit status is 0 only when every
difference is allowed (and there is none unexpected), 1 when one is not, 2 on usage errors.  A
pattern that matches no difference is reported as a note (an allow-list that outlives its cause).

`--show <test>` prints the unified diff of that test's `.vpr` (and `.map`) and exits 0; the test is
given as an entry (`tests/memory/x.c.c23`) or a test path with one mode in the trees.

Entries only in the new tree are new tests: they are counted, not compared, and do not fail the run
unless `--strict-new` is given.  An entry only in the base (a test whose Viper is gone) and an entry
that differs always count.

MANIFEST and the other files of the directories are not compared.
"""

from __future__ import annotations

import argparse
import difflib
import fnmatch
import re
import sys
from pathlib import Path

VPR_SUFFIX = ".vpr"
MAP_SUFFIX = ".vpr.map"


def entries(root: Path) -> dict[str, dict[str, Path]]:
    """Every entry (`<test path>.<mode>`) under `root` with its files by suffix (`.vpr`, `.vpr.map`)."""
    found: dict[str, dict[str, Path]] = {}
    for path in sorted(root.rglob("*")):
        if not path.is_file():
            continue
        rel = path.relative_to(root).as_posix()
        for suffix in (MAP_SUFFIX, VPR_SUFFIX):
            if rel.endswith(suffix):
                found.setdefault(rel[: -len(suffix)], {})[suffix] = path
                break
    return found


def glob_re(pattern: str) -> re.Pattern[str]:
    """`*` and `?` match within a name and `**` across directories, as in a shell with globstar."""
    out = []
    i = 0
    while i < len(pattern):
        c = pattern[i]
        if pattern.startswith("**", i):
            out.append(".*")
            i += 2
            continue
        out.append("[^/]*" if c == "*" else "[^/]" if c == "?" else re.escape(c))
        i += 1
    return re.compile("".join(out) + r"\Z")


def read_allow(path: Path) -> list[str]:
    patterns = []
    for line in path.read_text().splitlines():
        line = line.split("#", 1)[0].strip()
        if line:
            patterns.append(line)
    return patterns


def entry_test(entry: str) -> str:
    """The test path of an entry: the entry without its `.<mode>`."""
    return entry.rsplit(".", 1)[0]


def matches(pattern: re.Pattern[str], entry: str) -> bool:
    return pattern.match(entry) is not None or pattern.match(entry_test(entry)) is not None


def differing(base: dict[str, dict[str, Path]], new: dict[str, dict[str, Path]],
              entry: str) -> list[str]:
    """The suffixes of `entry` that differ (one side lacks it, or the bytes differ)."""
    out = []
    for suffix in (VPR_SUFFIX, MAP_SUFFIX):
        a = base[entry].get(suffix)
        b = new[entry].get(suffix)
        if (a is None) != (b is None) or (a is not None and a.read_bytes() != b.read_bytes()):
            out.append(suffix)
    return out


def show(base: dict[str, dict[str, Path]], new: dict[str, dict[str, Path]], test: str) -> int:
    chosen = [e for e in sorted(set(base) | set(new)) if e == test or entry_test(e) == test]
    if not chosen:
        print(f"vprdiff.py: no entry {test} in either tree", file=sys.stderr)
        return 2
    for entry in chosen:
        for suffix in (VPR_SUFFIX, MAP_SUFFIX):
            a = base.get(entry, {}).get(suffix)
            b = new.get(entry, {}).get(suffix)
            left = a.read_text(errors="replace").splitlines(keepends=True) if a else []
            right = b.read_text(errors="replace").splitlines(keepends=True) if b else []
            diff = list(difflib.unified_diff(left, right, f"base/{entry}{suffix}" if a else "/dev/null",
                                             f"new/{entry}{suffix}" if b else "/dev/null"))
            if diff:
                sys.stdout.writelines(line if line.endswith("\n") else line + "\n" for line in diff)
            else:
                print(f"{entry}{suffix}: identical")
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("base_dir")
    parser.add_argument("new_dir")
    parser.add_argument("--allow", metavar="FILE", help="tests whose differences are expected (patterns, one per line)")
    parser.add_argument("--show", metavar="TEST", help="print the unified diff of one test and exit")
    parser.add_argument("--strict-new", action="store_true",
                        help="count entries only in the new tree as differences (by default they are new tests, "
                             "listed by count only)")
    args = parser.parse_args(argv)
    roots = [Path(args.base_dir), Path(args.new_dir)]
    for r in roots:
        if not r.is_dir():
            print(f"vprdiff.py: {r} is not a directory", file=sys.stderr)
            return 2
    base = entries(roots[0])
    new = entries(roots[1])
    if not base or not new:
        print(f"vprdiff.py: no .vpr files in {roots[0] if not base else roots[1]}", file=sys.stderr)
        return 2
    if args.show:
        return show(base, new, args.show)
    patterns: list[tuple[str, re.Pattern[str]]] = []
    if args.allow:
        try:
            patterns = [(p, glob_re(p)) for p in read_allow(Path(args.allow))]
        except OSError as err:
            print(f"vprdiff.py: {err}", file=sys.stderr)
            return 2

    only_base = sorted(set(base) - set(new))
    only_new = sorted(set(new) - set(base))
    changed = [(e, differing(base, new, e)) for e in sorted(set(base) & set(new))]
    changed = [(e, s) for e, s in changed if s]
    differences = ([(e, "only in base") for e in only_base]
                   + [(e, "differs: " + " and ".join(s.lstrip(".") for s in suffixes)) for e, suffixes in changed])
    if args.strict_new:
        differences += [(e, "only in new") for e in only_new]
    elif only_new:
        print(f"{len(only_new)} entries only in new (new tests; not compared, see --strict-new)")
    differences.sort()
    used: set[str] = set()
    allowed: list[tuple[str, str]] = []
    unexpected: list[tuple[str, str]] = []
    for entry, what in differences:
        hit = [p for p, rx in patterns if matches(rx, entry)]
        used.update(hit)
        (allowed if hit else unexpected).append((entry, what))

    print(f"compared {len(set(base) | set(new))} entries ({len(set(base) & set(new))} in both): "
          f"{len(base) - len(only_base) - len(changed)} identical, {len(differences)} differences")
    if allowed:
        print(f"\nallowed ({len(allowed)}):")
        for entry, what in allowed:
            print(f"  {entry}: {what}")
    if unexpected:
        print(f"\nUNEXPECTED ({len(unexpected)}):")
        for entry, what in unexpected:
            print(f"  {entry}: {what}")
    for p, _ in patterns:
        if p not in used:
            print(f"note: the allow-list pattern {p!r} matches no difference")
    print(f"\n{len(allowed)} allowed, {len(unexpected)} unexpected")
    return 1 if unexpected else 0


if __name__ == "__main__":
    sys.exit(main())
