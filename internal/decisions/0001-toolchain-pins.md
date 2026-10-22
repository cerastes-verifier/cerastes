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
| EDG front end | the owner's fork `mborland/compiler`, branch `cerastes` (decision 0009, since 2026-10-14; submodule URL https://github.com/mborland/compiler.git), currently at the fork's `f0ed5525af135a139630d9b5840bd13b7d2d158c` (2026-10-22): the cerastes series merged with upstream `edgcpp/compiler` `main` `ec2f12cffa` ("Add codecov reporting []"), through the fix branch of EDG bug 0022. The base was upstream `034385ba0a` (`7.0-33`) in M2; see "Pin history" | submodule `third_party/edg`; stock preset `macos-arm-clang-debug`, built out of tree in `build/edg` |
| Silicon | `viperproject/silicon` tag `v.26.08-release` = `6b540aa45e5af214f4727b7be463ce84b328fabd` | submodule `third_party/silicon`; `sbt assembly` → `target/scala-2.13/silicon.jar` |
| Silver | `viperproject/silver` `f02e7010639f314447d74732146644565aa8d135` (`v.26.08-release`, the commit Silicon's tag records) | nested submodule `third_party/silicon/silver` |
| Z3 | 4.16.0, `z3-4.16.0-arm64-osx-15.7.3.zip`, SHA-256 `41828fa07d5cb77bfaee326e8e6dac074f26329c09c633f9e66012bb917cf8ae` | downloaded from the Z3 GitHub release into `build/tools/`, checksum verified |
| JDK | Homebrew `openjdk@17` (17.0.20.1) | `brew install openjdk@17` |
| sbt | 1.6.2 (Silicon's `project/build.properties`), started by the Homebrew `sbt` 2.0.10 launcher | `brew install sbt` |
| Scala | 2.13.18 (silver's `build.sbt`) | fetched by sbt |
| VeriFast (differential tests, M5) | `mborland/verifast` branch `generic2` = `0fb63506a51cae559ae92348eb54ba7f7be0bf6d`: upstream `master` plus `a18da574` (generic templates) and `0fb63506` (concepts) | not built by bootstrap; see below |
| Antora (user docs, M1.5) | `antora`, `@antora/cli`, `@antora/site-generator` 3.2.1; `@antora/lunr-extension` 1.0.0-alpha.13 | exact pins in `doc/package.json`, locked by `doc/package-lock.json`; `npm ci` in `doc/` by bootstrap |
| Antora default UI (user docs) | `antora-ui-default` `master` at `0e38223adfd81eb74d4b1779e158f8ad05ff8923` (GitLab job 15385706035), zip SHA-256 `e0ac81aad26961a9cd3cf9bce692a598eff4a1bd2822ce73c9cdc13a15a1e70d` | vendored as `doc/ui/antora-ui-default.zip`; `scripts/build_docs.sh` checks the SHA-256 before every build (`doc/ui/README.md`) |
| GitHub Actions (docs CI) | `actions/checkout` v7.0.1 `3d3c42e5aac5ba805825da76410c181273ba90b1`, `actions/setup-node` v7.1.0 `949feb2413d6458794dcd2491c4babbbce0c15c1`, `actions/upload-pages-artifact` v5.0.0 `fc324d3547104276b827a68afc52ff2a11cc49c9`, `actions/deploy-pages` v5.0.1 `368f82528645a54fb793d4d04e342629a3f51346`; runner `ubuntu-24.04` | full commit SHAs in `.github/workflows/docs.yml` (from `git ls-remote`; all four are lightweight tags) |
| GitHub Actions (CI, decision 0016) | `actions/checkout` v7.0.1 and `actions/setup-node` v7.1.0 (the SHAs above), `actions/setup-python` v7.0.0 `5fda3b95a4ea91299a34e894583c3862153e4b97` (2026-07-20), `actions/cache` v6.1.0 `55cc8345863c7cc4c66a329aec7e433d2d1c52a9` (2026-06-26; its `restore` and `save` sub-actions), `actions/upload-artifact` v7.0.2 `cf430e030ddbb5b0abf93d22962f4752f3646cd9` (2026-10-07); runner `xcode-27` (GitHub's image, a public preview; `20261012.0260.1` when S2 started) | full commit SHAs in `.github/workflows/ci.yml` (from `git ls-remote`; all lightweight tags; releases at least two weeks old) |
| Node.js (user docs) | major 26 (`doc/.node-version`; CI's `setup-node` reads it); 26.9.0 / npm 11.19.1 locally | Homebrew `node`; bootstrap warns when the major differs |

Recorded but not pinned, because any recent version works and none of them
affects verification results:

| Tool | Version used in M0 |
|---|---|
| Host compiler and SDK (builds EDG; supplies the headers EDG parses) | Apple clang 21.0.0 (clang-2100.3.34.2), Command Line Tools 27.0.0.0.1788430756, macOS SDK 27.0 (build `26A425`), macOS 27.0.1 (26A434). CI selects the same Command Line Tools on GitHub's `xcode-27` image and asserts the package version, the clang banner and the SDK version and build (`scripts/ci.sh`, 0016 item 2) |
| GCC (the native-ok runs, 0008) | Homebrew `gcc` 16.2.0, the macOS 27 bottle (`--build=aarch64-apple-darwin27`, sysroot `/Library/Developer/CommandLineTools/SDKs/MacOSX27.sdk`, i.e. SDK 27.0); reinstalled by the owner on 2026-10-21, before which it was the macOS 26 bottle with sysroot SDK 26.5. CI installs the same formula and asserts 16.2.0 and that its sysroot is the asserted SDK (0016 item 3) |
| CMake / Ninja | 4.4.3 / 1.13.2 (CI: the image's, 4.4.4 / 1.13.2) |
| Python | 3.14.7 (the scripts need 3.9 or later; CI uses 3.14 through `actions/setup-python`, the image's cached 3.14.8) |
| ShellCheck | 0.11.0 |
| Homebrew | 7.0.9 |

Node is pinned only by its major version: nothing Antora produces depends on
the minor version, and CI uses the same major through `doc/.node-version`.
Bootstrap installs Homebrew's unversioned `node`, which floats. Homebrew also
has versioned `node@NN` formulae; switch to one if the floating major ever
breaks the build.

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
`<concepts>`, `<type_traits>` and `<limits>` parse cleanly. The hello-world
smoke test therefore uses `<cstdio>`. This is an upstream EDG issue and is out
of scope for M0 (no EDG modifications).

**Correction (M1.5, 2026-10-14):** `<utility>` does not parse either. M0 listed
it as clean, but `scripts/cpfe.sh --c++20 --no_code_gen` on a file that only
includes it exits 2 with 8 errors in the SDK's `_stdlib.h`, at lines 298 (3),
308, 345, 352, 360 and 368. Each is a parameter in Apple's block syntax (`^`),
for example `atexit_b`'s. There are three independent causes:
1. `_stdlib.h`'s block syntax, in all five failing headers (8 errors);
2. `__algorithm/find.h`, with notes in `simd_utils.h` (7 errors);
3. `__new/interference_size.h:23-24`, where `__GCC_DESTRUCTIVE_SIZE` and
   `__GCC_CONSTRUCTIVE_SIZE` are undefined (2 errors).

`<iostream>`, `<string>`, `<vector>` and `<algorithm>` have all three (17
errors); `<utility>` has only the first. Same Command Line Tools as M0 (SDK
27.0, CLT 27.0.0.0.1788430756). The owner raises these with the EDG team
(handoff, question 6). Root causes, reproduced on upstream on 2026-10-15:
`internal/edg_bugs/` 0005 (`__BLOCKS__`), 0007 (vector types not configured
for macOS) and 0006 (the interference-size macros).

**Fixes (2026-10-15).** Each cause is fixed by a branch of the fork, merged
into its `cerastes` branch and pinned here:
- 0005, pinned at `8e1eb6c0a6`: `<utility>`, `<cstdlib>` and C's
  `<stdlib.h>` parse. `<iostream>`, `<string>`, `<vector>` and `<algorithm>`
  have 9 errors left (causes 2 and 3), `<new>` 2 (cause 3).
- 0006, pinned at `99d825cff0`: `<new>` parses. `<iostream>`, `<string>`,
  `<vector>` and `<algorithm>` have 7 errors left, all in
  `__algorithm/find.h` (cause 2).
- 0007, pinned at `c1cb2bb47b` (with its test follow-up): none of the five headers has an error left,
  nor do `<map>`, `<unordered_map>`, `<functional>`, `<memory>`, `<array>`,
  `<span>`, `<ranges>`, `<format>`, `<numeric>`, `<optional>`, `<variant>`,
  `<tuple>`, `<sstream>`, `<regex>`, `<filesystem>`, `<new>` and `<cstdlib>`,
  with `scripts/cpfe.sh` and with `scripts/cerastes-fe.sh` (`--c++20`).
  `tests/edg-smoke/libcxx_headers.cpp` keeps it so. The fix enables vector
  types natively, which changes the IL layout: `GNU_VECTOR_TYPES_ALLOWED` is
  now required by `scripts/check_edg_config.sh` and `src/backend/edg_il.h`.
  The SDK's `<simd/simd.h>` now fails instead of being silently empty
  (`internal/edg_bugs/0013`).

## VeriFast

Per the pre-M0 decision, option (a) builds `mborland/verifast@generic2`
locally and option (b), recording its outcomes once, is the fallback. The
local tree `~/Documents/verifast` is on `generic2` at the pin above. Its
`bin/verifast` is a Linux x86-64 ELF, so it does not run natively on this Mac.
M5 decides between a native macOS build, running that binary in a container,
or option (b). Bootstrap does not depend on VeriFast.

**Owner decision (handoff question 8, 2026-10-13):** M5 runs the Linux
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

## Pin history

- **EDG `9847356d` → `034385ba` (M2, 2026-10-14).** The owner's answer to
  M1.5 handoff question 2. The 8 upstream commits in between (`4e739c28`,
  `7a0533db`, `b971e940`, `0d49bfb9`, `d05f214f`, `996d67fa`, `435b7d16`,
  `034385ba`) touch CI, `dev_tools/`, `doc/source/testing.rst`,
  `util/eccp.sh` and `tests/expectations/`, nothing under `src/`
  (`git log 9847356d..034385ba -- src` is empty). `ninja` rebuilt nothing,
  `scripts/check.sh` was green and every golden was byte-identical. The
  version string below is unchanged because the binaries were not rebuilt;
  `git describe` is the record of the commit.

- **Submodule URL → the fork (M2, 2026-10-14; decision 0009).** Same pin
  `034385ba`, now fetched from `https://github.com/mborland/compiler.git`.
  Pins that are not yet pushed are fetched through the `fork-local` remote
  (`git -C third_party/edg remote add fork-local ~/Documents/compiler`).

- **Fork pins M2–M4.2a (2026-10-14 to 2026-10-21).** K `23f55e3628`, then
  `4ff94f065c` (EDG bugs 0003–0007), `0bcd9bf28a` (upstream `307002eae6`
  merged), P `b9519388ea` (0020 and series L, M, N, P); each is recorded in
  `internal/status.md`.

- **Upstream merge (S1, 2026-10-21).** P → `97c2141be9`, which merges upstream
  `main` `21065e551e` into the fork's `cerastes` (10 commits after
  `307002eae6`: #229, #307 (our 0005 re-committed), #302, #309, #218, CI,
  issue-template and test-filter commits). Under `src/` only four `.c` files
  change (`lexical.c`, `statements.c`, `decl_inits.c`, `lower_il.c`; no
  header, so the IL layout is unchanged; #309's hunk is compiled out with
  `DO_IL_LOWERING` 0). The full box suites at the merge match the
  expectations in both 64-bit configurations except the known `cwg1835` and
  `cwg2881`; every test's emitted Viper (667 `.vpr` and their maps) is
  byte-identical to P's; `check.sh` is green.

- **CI (S2, 2026-10-21; decision 0016).** No pin moves. `.github/workflows/ci.yml`
  runs `scripts/ci.sh` and the full `scripts/check.sh` on GitHub's `xcode-27`
  image (a public preview) with the owner's Command Line Tools, which
  `ci.sh` asserts together with GCC 16; the rows "GitHub Actions (CI)", "Host
  compiler and SDK", "GCC" and "Python" record it. A change of the image's
  tools fails the job until the owner moves the assertion and this record
  together.

- **EDG bug 0022 and upstream `ec2f12cffa` (M4.2c, 2026-10-22).** `97c2141be9` →
  `f0ed5525af`, the owner's merge of branch `0022` (`3b05dd5c43`, the fix of EDG bug
  0022, based on upstream `main` `ec2f12cffa`) into the fork's `cerastes`; it brings
  the 11 upstream commits after `21065e551e` too. Under `src/`: `error.c` (0022),
  `templates.c`, `types.c`, `overload.c`, `il.c`, `il_display.c` (#173, #335, #340),
  `targ_def.h` and `defines_linux.h` (#38: 128-bit integer value limits when `__int128`
  is allowed), `il.h` (a defaulted parameter) and `il_def.h` (a comment): no IL layout
  change. Every test's emitted Viper is byte-identical to the previous pin's (1107
  `.vpr`, see `internal/status.md`, M4.2c); `check.sh` is green. One expectation
  changed, with the owner's approval: #38 removes EDG's spurious #61 on a valid
  `__int128` multiply, so the M3 probe `p9_wide_constants.cpp` is `frontend-ok`, and
  `p9_wide_constants_overflow.cpp` pins #61 on a product outside `__int128`. The box
  suites at `f0ed5525af` were not run by the session (owner's merge; the next box
  session).

## Dependabot

`.github/dependabot.yml` (owner, M1.5 handoff question 8) opens weekly
grouped pull requests for the SHA-pinned GitHub Actions and for the npm
packages in `doc/`. Both are pinned in the tables above, so a Dependabot pull
request must update this record in the same change (step 3 of "Changing a
pin"), and after an npm update `scripts/bootstrap.sh` must be re-run so that
`doc/node_modules` matches the lockfile.

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
 034385ba0a6d08578aa3215fda6f674f2efb0c77 third_party/edg (7.0-33-g034385ba0a)   # since 2026-10-14; was 9847356d (7.0-25)
 6b540aa45e5af214f4727b7be463ce84b328fabd third_party/silicon (v.26.08-release)
 f02e7010639f314447d74732146644565aa8d135 third_party/silicon/silver (v.26.08-release)
```

*2026-10-20: milestone numbers follow the renumbering of 2026-10-20 (plan.md: old M4 → M5, M5 → M6, M6 → M4.5 for C and M7 for C++, M7 → M8).*
