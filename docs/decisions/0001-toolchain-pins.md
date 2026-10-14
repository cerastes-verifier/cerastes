# 0001 — Toolchain pins

- **Status:** accepted
- **Milestone:** M0 (2026-10-13)

## Context

M0 asks for a reproducible toolchain with every version pinned and recorded.
`scripts/bootstrap.sh` is the executable form of this record; change a pin
in both places.

## Decision

| Component | Pin | Obtained by |
|---|---|---|
| EDG front end | `edgcpp/compiler` `9847356d461d2bef846bf133e16821f0e8b69749` (`git describe`: `7.0-25-g9847356d46`), upstream `main` head on 2026-10-13: "Move the prototype instantiation IL documentation to ALL_TEMPLATE_INFO_IN_IL. [GH #81]" | submodule `third_party/edg`; stock preset `macos-arm-clang-debug`, built out of tree in `build/edg` |
| Silicon | `viperproject/silicon` tag `v.26.08-release` = `6b540aa45e5af214f4727b7be463ce84b328fabd` | submodule `third_party/silicon`; `sbt assembly` → `target/scala-2.13/silicon.jar` |
| Silver | `viperproject/silver` `f02e7010639f314447d74732146644565aa8d135` (`v.26.08-release`, the commit Silicon's tag records) | nested submodule `third_party/silicon/silver` |
| Z3 | 4.16.0, `z3-4.16.0-arm64-osx-15.7.3.zip`, SHA-256 `41828fa07d5cb77bfaee326e8e6dac074f26329c09c633f9e66012bb917cf8ae` | downloaded from the Z3 GitHub release into `build/tools/`, checksum verified |
| JDK | Homebrew `openjdk@17` (17.0.20.1) | `brew install openjdk@17` |
| sbt | 1.6.2 (Silicon's `project/build.properties`), started by the Homebrew `sbt` 2.0.10 launcher | `brew install sbt` |
| Scala | 2.13.18 (silver's `build.sbt`) | fetched by sbt |
| VeriFast (differential tests, M4) | `mborland/verifast` branch `generic2` = `0fb63506a51cae559ae92348eb54ba7f7be0bf6d`: upstream `master` plus `a18da574` (generic templates) and `0fb63506` (concepts) | not built by bootstrap; see below |

Recorded but not pinned, because any recent version works and none of them
affects verification results:

| Tool | Version used in M0 |
|---|---|
| Host compiler and SDK (builds EDG; supplies the headers EDG parses) | Apple clang 21.0.0 (clang-2100.3.34.2), Command Line Tools, macOS SDK 27.0, macOS 27.0.1 (26A434) |
| CMake / Ninja | 4.4.3 / 1.13.2 |
| Python | 3.14.7 (the scripts need 3.9 or later) |
| ShellCheck | 0.11.0 |
| Homebrew | 7.0.9 |

## Departures from plan.md

1. **EDG is pinned to upstream, not a fork.** The submodule URL is
   `https://github.com/edgcpp/compiler.git` so that anyone can build cerastes
   without access to a fork (owner's direction, 2026-10-13). Invariant 5 now
   reads: `third_party/edg` only ever pins an upstream commit; EDG changes are
   developed on a `cerastes` fork branch and land upstream before the pin
   moves. Consequence: the M1 configuration and the M2 attribute registration
   must either be upstreamed or avoid EDG source changes before cerastes can
   depend on them. M1 needed no EDG change (0002). The fork branch is
   `mborland/compiler` `cerastes`, and the rules for it are in 0003.
2. **JDK 17 instead of Homebrew `openjdk` (27).** Silicon pins sbt 1.6.2,
   whose build-definition compiler (Scala 2.12.15) cannot read JDK 27 class
   files. Under JDK 27, `sbt about` in `third_party/silicon` fails with
   `java.lang.NoClassDefFoundError: Could not initialize class
   sbt.internal.parser.SbtParser$`. Silicon's CI uses JDK 11, which Homebrew
   has deprecated; 17 is the newest LTS that sbt 1.6.2 supports. The build
   took 46 s on JDK 17.
3. **Z3 is a checksummed release download, not Homebrew `z3`.** Homebrew only
   carries the newest Z3 (5.1.0 on 2026-10-13) and changes it on every
   `brew upgrade`. Solver changes move timeouts and incompleteness, so a
   floating Z3 would make results unreproducible. 4.16.0 is the version
   Silicon v.26.08's CI tests against (`.github/workflows/ci.yml`) and the
   version of its bundled Z3 Java API jar. Silicon v.26.08 also verified the
   smoke fixtures correctly with Homebrew Z3 5.1.0, but that combination is
   untested upstream.
4. **EDG builds out of tree.** `cmake -S third_party/edg --preset
   macos-arm-clang-debug -B build/edg` keeps the submodule clean. The default
   target also builds EDG's runtime library; the whole build takes about
   10 s. Silicon builds in its own tree as the plan says; `target/` is ignored
   upstream, so the submodule stays clean.

## Running the front end

On macOS EDG has no C++ standard library of its own; it parses the SDK and
libc++ headers while emulating the installed clang. `cpfe` also needs
`EDG_BASE` to find `lib/predefined_macros.txt`. `scripts/cpfe.sh` does what
EDG's `HACKING.md` describes: it passes `--clang_version=<scraped>` and one
`--sys_include` per directory that `dev_tools/bin/edg-scrape-compiler` reports
for the installed clang. Bootstrap stores these values in `build/env.sh`.
Because of this, the headers EDG sees come from the host Command Line Tools,
which are not pinned.

**Known issue at this pin:** EDG cannot parse the macOS SDK 27.0 libc++
headers that include `<__algorithm/find.h>`, so `<iostream>`, `<string>`,
`<vector>` and `<algorithm>` fail with 10 to 17 errors in
`__algorithm/find.h` and `__algorithm/simd_utils.h`. `<cstdio>`, `<cstdint>`,
`<concepts>`, `<type_traits>`, `<utility>` and `<limits>` parse cleanly. The
hello-world smoke test therefore uses `<cstdio>`. This is an upstream EDG
issue and is out of scope for M0 (no EDG modifications).

## VeriFast

Per the pre-M0 decision, option (a) builds `mborland/verifast@generic2`
locally and option (b), recording its outcomes once, is the fallback. The
local tree `~/Documents/verifast` is on `generic2` at the pin above. Its
`bin/verifast` is a Linux x86-64 ELF, so it does not run natively on this Mac.
M4 decides between a native macOS build, running that binary in a container,
or option (b). Bootstrap does not depend on VeriFast.

**Owner decision (handoff question 8, 2026-10-13):** M4 runs the Linux
binary in a container and checks its outcomes against the recorded ones
(option (b)) as well.

## Changing a pin

1. Move the submodule (`git -C third_party/<x> checkout <rev>`, then
   `git add third_party/<x>`) or edit the constants at the top of
   `scripts/bootstrap.sh` (Z3 needs the release asset's SHA-256).
2. Run `scripts/bootstrap.sh && scripts/check.sh`. A Silicon update that
   changes error messages makes `tests/viper-smoke/fail_kinds.vpr` fail;
   update the message table in `scripts/run_tests.py` from silver's
   `VerificationError.scala`.
3. Update the tables above with the new SHAs and `--version` output.

## Raw version output (2026-10-13)

```
$ build/edg/bin/cpfe --version
Edison Design Group C/C++ Front End, version 7.0 (Oct 13 2026 13:28:50)
Copyright 1988-2026 Edison Design Group, Inc.

$ $CERASTES_JAVA -jar $SILICON_JAR <file>      # Silicon has no --version
Silicon 1.1-SNAPSHOT (6b540aa4@(detached))

$ $Z3_EXE --version
Z3 version 4.16.0 - 64 bit

$ $CERASTES_JAVA -version
openjdk version "17.0.20.1" 2026-08-18
OpenJDK Runtime Environment Homebrew (build 17.0.20.1+0)
OpenJDK 64-Bit Server VM Homebrew (build 17.0.20.1+0, mixed mode, sharing)

$ sbt --script-version            # launcher; the project runs sbt 1.6.2
2.0.10

$ clang --version
Apple clang version 21.0.0 (clang-2100.3.34.2)
Target: arm64-apple-darwin27.0.0

$ git submodule status --recursive
 9847356d461d2bef846bf133e16821f0e8b69749 third_party/edg (7.0-25-g9847356d46)
 6b540aa45e5af214f4727b7be463ce84b328fabd third_party/silicon (v.26.08-release)
 f02e7010639f314447d74732146644565aa8d135 third_party/silicon/silver (v.26.08-release)
```
