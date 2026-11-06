#!/usr/bin/env python3
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""The toolchains CI runs (decision 0053): reads scripts/ci_toolchains.toml, validates it and prints
what the workflow, the setup scripts and the test runner need.

  ci_profile.py check                       validate the file (the harness self-test runs it too)
  ci_profile.py list                        every profile: name, platform, runner, cadence
  ci_profile.py get NAME FIELD              one field of a profile (scripts/ci.sh, scripts/ci-linux.sh)
  ci_profile.py matrix --event E [--select S]
                                            the two job matrices, as GitHub step outputs
                                            (`macos=[...]`, `linux=[...]`)
  ci_profile.py exclusions NAME             the profile's exclusions, one JSON object per line

The selection (item 4): a pull request or a push runs the `every` profiles; the schedule runs `every`
and `nightly`; a dispatch runs `every` and `nightly` when its `profiles` input is empty, every profile
for `all`, and exactly the named profiles otherwise (names separated by commas or blanks).

Exit status: 0, 1 when the file or a request is wrong (with a message on stderr).
"""

import json
import re
import sys
import tomllib
from pathlib import Path

TOOLCHAINS = Path(__file__).resolve().parent / "ci_toolchains.toml"

SCHEMA = 1
CADENCES = ("every", "nightly", "dispatch")
BUILDS = ("clang", "gcc")
# The GitHub labels a profile may name, with what each one is (decision 0053 item 4): the platform,
# and the OS version every profile on it must declare.
RUNNERS = {
    "xcode-27": ("macos", 27),
    "macos-26": ("macos", 26),
    "ubuntu-26.04": ("linux", "26.04"),
    "ubuntu-24.04": ("linux", "24.04"),
}
COMMON = {"name": str, "platform": str, "runner": str, "cadence": str, "jobs": int, "clang_major": int,
          "gcc_major": int, "build": str}
FIELDS = {
    "macos": dict(COMMON, macos_major=int, clt_major=int, sdk_major=int),
    "linux": dict(COMMON, ubuntu_version=str, glibc=str),
}
OPTIONAL = {"exclusions": list}
# The oldest compilers whose native-ok canary passes: GCC 13 has no -std=c23; clang 18 is the first
# with it on Linux, and Apple clang 17 on macOS (decision 0053 item 4).
MIN_GCC = 14
MIN_CLANG = {"macos": 17, "linux": 18}
NAME_RE = re.compile(r"[a-z0-9][a-z0-9.-]*\Z")
EXCLUSION_FIELDS = {"test": str, "reason": str}
EXCLUSION_OPTIONAL = {"mode": str}
MODES = ("c23", "c++20")


class ProfileError(Exception):
    pass


def _check_fields(where, table, required, optional):
    problems = []
    for key, kind in required.items():
        if key not in table:
            problems.append(f"{where}: `{key}` is missing")
        elif type(table[key]) is not kind:
            problems.append(f"{where}: `{key}` must be a {kind.__name__}, not {table[key]!r}")
    for key in table:
        if key not in required and key not in optional:
            problems.append(f"{where}: unknown field `{key}`")
        elif key in optional and type(table[key]) is not optional[key]:
            problems.append(f"{where}: `{key}` must be a {optional[key].__name__}")
    return problems


def problems_of(data, root=None):
    """Everything wrong with the parsed file `data` (an empty list when it is fine).  `root` is the
    repository root that exclusions name tests under (the file's own repository by default)."""
    root = root or TOOLCHAINS.parent.parent
    problems = []
    if set(data) - {"schema", "profile"}:
        problems.append(f"unknown top-level keys: {sorted(set(data) - {'schema', 'profile'})}")
    if data.get("schema") != SCHEMA:
        problems.append(f"`schema` must be {SCHEMA}")
    profiles = data.get("profile")
    if not isinstance(profiles, list) or not profiles:
        return problems + ["no [[profile]] tables"]
    seen = set()
    for i, p in enumerate(profiles):
        where = f"profile {i + 1}"
        if not isinstance(p, dict):
            problems.append(f"{where}: not a table")
            continue
        name = p.get("name")
        if isinstance(name, str):
            where = f"profile `{name}`"
            if not NAME_RE.match(name):
                problems.append(f"{where}: the name must match {NAME_RE.pattern[:-2]}")
            if name in seen:
                problems.append(f"{where}: the name is used twice")
            seen.add(name)
        platform = p.get("platform")
        if platform not in FIELDS:
            problems.append(f"{where}: `platform` must be one of {sorted(FIELDS)}")
            continue
        found = _check_fields(where, p, FIELDS[platform], OPTIONAL)
        problems += found
        if found:
            continue
        runner = RUNNERS.get(p["runner"])
        if runner is None:
            problems.append(f"{where}: `runner` must be one of {sorted(RUNNERS)}")
        elif runner[0] != platform:
            problems.append(f"{where}: the runner {p['runner']} is a {runner[0]} runner")
        elif runner[1] != p["macos_major" if platform == "macos" else "ubuntu_version"]:
            problems.append(f"{where}: the runner {p['runner']} has "
                            f"{'macOS' if platform == 'macos' else 'Ubuntu'} {runner[1]}")
        if p["cadence"] not in CADENCES:
            problems.append(f"{where}: `cadence` must be one of {list(CADENCES)}")
        if not 1 <= p["jobs"] <= 64:
            problems.append(f"{where}: `jobs` must be between 1 and 64")
        if p["build"] not in BUILDS:
            problems.append(f"{where}: `build` must be one of {list(BUILDS)}")
        elif p["build"] == "gcc" and platform != "linux":
            problems.append(f"{where}: a GCC build is Linux only (decision 0053 item 12)")
        if p["gcc_major"] < MIN_GCC:
            problems.append(f"{where}: GCC {p['gcc_major']} has no -std=c23 (at least {MIN_GCC})")
        if p["clang_major"] < MIN_CLANG[platform]:
            problems.append(f"{where}: clang {p['clang_major']} is older than {MIN_CLANG[platform]}")
        if platform == "macos":
            for key in ("macos_major", "clt_major", "sdk_major"):
                if not 1 <= p[key] <= 99:
                    problems.append(f"{where}: `{key}` must be a major version")
        else:
            if not re.fullmatch(r"[0-9]{2}\.(04|10)", p["ubuntu_version"]):
                problems.append(f"{where}: `ubuntu_version` must be an Ubuntu release such as 26.04")
            if not re.fullmatch(r"2\.[0-9]{2}", p["glibc"]):
                problems.append(f"{where}: `glibc` must be a version such as 2.43")
        exclusions = p.get("exclusions", [])
        if exclusions and p["cadence"] == "every":
            problems.append(f"{where}: a reference (`every`) profile carries no exclusions (decision 0053 item 7)")
        keys = set()
        for j, e in enumerate(exclusions):
            ewhere = f"{where}, exclusion {j + 1}"
            if not isinstance(e, dict):
                problems.append(f"{ewhere}: not a table")
                continue
            found = _check_fields(ewhere, e, EXCLUSION_FIELDS, EXCLUSION_OPTIONAL)
            problems += found
            if found:
                continue
            test = e["test"]
            if not test.startswith(("tests/", "doc/")) or ".." in Path(test).parts or not (root / test).is_file():
                problems.append(f"{ewhere}: `{test}` is not a test of this repository")
            if "mode" in e and e["mode"] not in MODES:
                problems.append(f"{ewhere}: `mode` must be one of {list(MODES)}")
            if len(e["reason"].split()) < 3:
                problems.append(f"{ewhere}: give the reason in words")
            key = (test, e.get("mode"))
            if key in keys or (test, None) in keys or (e.get("mode") is None and any(k[0] == test for k in keys)):
                problems.append(f"{ewhere}: `{test}` is excluded twice")
            keys.add(key)
    return problems


def load(path=TOOLCHAINS, root=None):
    try:
        data = tomllib.loads(Path(path).read_text(encoding="utf-8"))
    except (OSError, tomllib.TOMLDecodeError) as e:
        raise ProfileError(f"{path}: {e}") from e
    problems = problems_of(data, root)
    if problems:
        raise ProfileError("\n".join(f"{path}: {p}" for p in problems))
    return {p["name"]: p for p in data["profile"]}


def profile(name, path=TOOLCHAINS):
    profiles = load(path)
    if name not in profiles:
        raise ProfileError(f"no profile `{name}` in {path} (there are: {', '.join(profiles)})")
    return profiles[name]


def select(profiles, event, chosen=""):
    """The names of the profiles an event runs, in file order."""
    if event in ("pull_request", "push"):
        cadences = {"every"}
    elif event == "schedule":
        cadences = {"every", "nightly"}
    elif event == "workflow_dispatch":
        names = [n for n in re.split(r"[\s,]+", chosen.strip()) if n]
        if names == ["all"]:
            return list(profiles)
        if names:
            unknown = [n for n in names if n not in profiles]
            if unknown:
                raise ProfileError(f"no profile {', '.join(f'`{n}`' for n in unknown)} "
                                   f"(there are: {', '.join(profiles)}, or `all`)")
            return [n for n in profiles if n in names]
        cadences = {"every", "nightly"}
    else:
        raise ProfileError(f"no selection for the event `{event}`")
    if chosen.strip() and event != "workflow_dispatch":
        raise ProfileError("only a dispatch names profiles")
    return [n for n, p in profiles.items() if p["cadence"] in cadences]


def matrices(profiles, names):
    out = {"macos": [], "linux": []}
    for n in names:
        p = profiles[n]
        out[p["platform"]].append({"profile": n, "runner": p["runner"], "jobs": p["jobs"]})
    return out


def main(argv):
    try:
        if argv[:1] == ["check"] and len(argv) == 1:
            load()
            return 0
        if argv[:1] == ["list"] and len(argv) == 1:
            for n, p in load().items():
                print(f"{n}\t{p['platform']}\t{p['runner']}\t{p['cadence']}")
            return 0
        if argv[:1] == ["get"] and len(argv) == 3:
            p = profile(argv[1])
            if argv[2] not in p or argv[2] == "exclusions":
                raise ProfileError(f"the profile `{argv[1]}` has no field `{argv[2]}`")
            print(p[argv[2]])
            return 0
        if argv[:1] == ["exclusions"] and len(argv) == 2:
            for e in profile(argv[1]).get("exclusions", []):
                print(json.dumps(e, sort_keys=True))
            return 0
        if argv[:1] == ["matrix"]:
            event, chosen, rest = None, "", argv[1:]
            while rest:
                if rest[0] == "--event" and len(rest) > 1:
                    event, rest = rest[1], rest[2:]
                elif rest[0] == "--select" and len(rest) > 1:
                    chosen, rest = rest[1], rest[2:]
                else:
                    raise ProfileError(f"unexpected argument `{rest[0]}`")
            if event is None:
                raise ProfileError("matrix needs --event")
            profiles = load()
            m = matrices(profiles, select(profiles, event, chosen))
            print(f"macos={json.dumps(m['macos'], separators=(',', ':'))}")
            print(f"linux={json.dumps(m['linux'], separators=(',', ':'))}")
            return 0
        print(__doc__.strip(), file=sys.stderr)
        return 1
    except ProfileError as e:
        print(f"ci_profile.py: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
