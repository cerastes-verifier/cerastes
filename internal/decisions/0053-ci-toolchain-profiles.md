# 0053 — CI toolchain profiles: declared majors, a matrix of toolchains, GCC builds (S8)

- **Status:** accepted for S8a (items 1–3); items 4–9 from S8b, items 10–11 from S8c, as the approved plan
  (`.claude/plans/s8-ci-toolchains.md`) gives them; each is marked accepted when its branch lands. Owner decisions
  (2026-11-06, S8 planning): "These scripts should be far more robust and allow different versions of clang and gcc on
  linux to be tested in the future. Like using default macos-16, 26, ubuntu-26.04, ubuntu-24.04 etc."; then, asked:
  a job asserts the majors its profile declares and records the exact builds (item 1); a different outcome on another
  toolchain fails the job, and is fixed or recorded as a named exclusion, never as a per-toolchain `expect:` line
  (item 7); the extra toolchains run nightly and on dispatch, the two references in both tiers (item 4); the guard
  becomes structural rules and a schema, with only the references' declared facts pinned (item 8); a small fix first,
  the profiles stacked on it; the first extra toolchains are other ubuntu-26.04 majors, ubuntu-24.04 and macos-26
  (there is no `macos-16` label: Apple's numbering went from 15 to 26); a profile can build cerastes and EDG with GCC,
  in this work (item 10).
- **Milestone:** S8 (2026-11-06), after M4.5d2b1, before M4.5d2b2.
- **Amends:** 0016 items 2, 3, 9 and 10 (exact assertions become declared majors; the guard), 0031 items 1 and 2 (the
  same for Linux), 0001 (the host-toolchain rows are recorded per profile).

## Context

0016 item 2 and 0031 item 2 assert the runner's toolchain exactly: the Command Line Tools package
`27.0.0.0.1788430756`, the banner `Apple clang version 21.0.0 (clang-2100.3.34.2)`, SDK 27.0 build `26A425`, GCC
16.2.0; on Linux the LLVM packages `1:21.1.8-6ubuntu1`, the GCC packages `16-20260322-1ubuntu1`, the banner and the
scrape `210108`. A changed image fails the job in its first minute, and 0016 item 2 says a short session then moves
the assertion and 0001 together.

That happened on 2026-11-06, in a form exact pins cannot absorb. GitHub published `xcode-27` image `20261104.0337` as a
pre-release on 2026-11-04 (a pre-release is an image still deploying; the runner-images README gives 2–3 days, and the
last four images took 2–6). Its only toolchain change is the Command Line Tools, 27.0.0.0.1788430756 →
27.2.0.0.1792721180 (the release notes list no Clang change). While it deploys, the label serves either image:
- run `42158664485` (the merge of PR #32) got `20261104.0337.1` and stopped at "Command Line Tools 27.2.0.0.1792721180,
  expected 27.0.0.0.1788430756";
- the owner then moved `CLT_VERSION` and `SDK_VERSION` to 27.2 (`c1b691d`), and run `42232697813` got
  `20261026.0314.1` and stopped at the reverse.

Any single exact value is red on part of the runs until the deployment ends, and then on every run until a session
moves it. `c1b691d` also changed `ci.sh` without the pinned copy in `scripts/test_run_tests.py` (0016 item 10's
double entry), so the harness self-test failed on both jobs. The owner's Mac still has 27.0.0.0.1788430756.

What the exact build of a tool changes: the headers EDG parses (the SDK, glibc, libc++), the clang version EDG emulates
(the scrape), and the warnings of the native-ok runs (`-Wall -Wextra -Werror`, 0008). A patch or build change within
one major can change any of these, but only by changing what a test sees, and the test then fails. The suite is the
check for that, and it already runs on every image: 0016 item 10's drift policy sends every changed result to the
owner. What a major change does (clang 21 → 22, SDK 27 → 28, GCC 16 → 17) is larger: it changes which language EDG
emulates and which warnings exist. That is where a profile should say what it runs.

## Decision

1. **Declared majors, recorded builds** (S8a). Each CI job asserts what its toolchain is at the granularity a reader
   chooses it by, and records the exact builds in the job log and `build/ci-toolchain.txt`. The macOS job (`scripts/ci.sh`):
   - the `com.apple.pkg.CLTools_Executables` version matches `27.*` (`CLT_MAJOR=27`);
   - the banner matches `^Apple clang version 21\.` (`CLANG_MAJOR=21`);
   - `xcrun --show-sdk-version` matches `27` or `27.*` (`SDK_MAJOR=27`), and the SDK's real path is
     `$CLT/SDKs/MacOSX<that version>.sdk`;
   - `g++-16 -dumpfullversion` matches `16.*` (`GCC_MAJOR=16`), and its sysroot's real path is still the SDK's;
   - after bootstrap, `EDG_CLANG_VER_SCRAPE` is in `[210000, 219999]`.
   The SDK build, the full CLT version, the banner and GCC's full version are printed in the report, not asserted.
   The Linux job (`scripts/ci-linux.sh`):
   - the banner of `clang-21`/`clang++-21` matches `clang version 21\.`;
   - `clang-21`, `libc++-21-dev` and `libc++abi-21-dev` are installed, at the same version as each other (one LLVM
     build, so the compiler and the library headers agree);
   - `gcc-16`/`g++-16` are installed, at the same version as each other, and `-dumpfullversion` matches `16.*`;
   - glibc stays exact (2.43): an Ubuntu release never changes its glibc version, only its patch packages, which the
     report records;
   - the scrape is in `[210000, 219999]`.
   Unchanged: Ubuntu 26.04 on x86-64, the paths of every compiler, `DEVELOPER_DIR`, `xcrun`'s choice, the
   environment ban, the free disk space, the EDG tree checks, libc++ (and no libstdc++) in the C++ include
   directories, and "the highest `gcc-NN` on `PATH` is the asserted one" (until item 6 makes bootstrap take the major).
   The macOS job's name becomes `check (macOS, Command Line Tools 27)`.
2. **Drift is the suite's to find.** A result on the runner that differs from the owner's (a test, a count, a golden,
   a diagnostic, a timeout) is drift exactly as in 0016 item 10, whether or not the image changed. A patch-level
   change is no longer drift by itself: the report shows it, and only a changed result stops the work.
3. **The owner's Mac and the runner** may have different builds within the declared majors: today the owner's CLT
   27.0 and the image's 27.2. Both satisfy the reference profile, and a result that differs between them is drift. The
   owner may move the Mac to the new build at any time (`softwareupdate`); nothing in the repository changes when
   either moves within a major.
4. **Toolchain profiles** (from S8b). `scripts/ci_toolchains.toml` lists every toolchain CI can run. Each entry is
   read and validated by `scripts/ci_profile.py`:
   - `name` (`[a-z0-9.-]+`, unique), `runner` (from an allowed set of GitHub labels), `cadence`, `jobs`;
   - on macOS: `macos_major`, `clt_major`, `clang_major` (Apple clang), `sdk_major`, `gcc_major`;
   - on Linux: `ubuntu_version`, `glibc`, `clang_major` (at least 18; it also chooses `libc++-N`), `gcc_major`;
   - `gcc_major` is at least 14 (GCC 13 has no `-std=c23`, so the native-ok canary would fail every C run);
   - `build`: `clang` or `gcc` (item 10), `gcc` on Linux only;
   - `exclusions` (item 7).
   `cadence` is `every` (both tiers: pull requests, pushes to `main`, the nightly run, dispatches), `nightly` (the
   nightly run and dispatches) or `dispatch` (only when a dispatch names it, or names `all`). The two references are
   `every`: `macos-27` (`xcode-27`; CLT 27, Apple clang 21, SDK 27, Homebrew GCC 16) and `ubuntu-26.04` (clang 21 and
   libc++ 21, GCC 16, glibc 2.43). A new toolchain starts as `dispatch` and becomes `nightly` once a full check on it
   is green (a rehearsal on the x86-64 box for Linux, a dispatched run for macOS).
5. **The workflow** (from S8b). A first job, `profiles`, reads the event (and a dispatch's `profiles` input) and
   prints two matrices with `ci_profile.py matrix`. `check-macos` and `check-linux` run one job per selected profile
   (`fail-fast: false`, `runs-on`, the job name and `CERASTES_JOBS` from the matrix; a job is skipped when its matrix
   is empty), each running `scripts/ci.sh <profile>` or `scripts/ci-linux.sh <profile>` and then `check.sh` unchanged.
   The artifact names and the Silicon cache key carry the profile's name. The dispatch input reaches the script
   through the environment, never through the command line. Triggers, permissions, concurrency, the
   tiers of 0016 item 14 and `timeout-minutes: 220` are unchanged. The Linux jobs keep the image's own Python (0031
   item 2: `setup-python` exports `LD_LIBRARY_PATH` on Linux); ubuntu-24.04's 3.12 satisfies the scripts' 3.9 and
   `tomllib`'s 3.11.
6. **Bootstrap inputs** (from S8b). `scripts/bootstrap.sh` takes `CERASTES_CLANG_MAJOR` (Linux: chooses `clang-N`
   and libc++ N, default 21 as today; macOS: asserts Apple clang's major), `CERASTES_GCC_MAJOR` (the native-ok GCC;
   default the highest, as today) and `CERASTES_BUILD_COMPILER` (item 10). Unset, it behaves exactly as before. A
   full bootstrap records the three in `build/toolchain.env`; `--build-only` (what `check.sh` runs) builds with the
   recorded values and stops when one of the variables names another compiler, so the check step builds with what the
   setup step chose. `build/compilers.stamp` records the compilers that built EDG and cerastes, and a build made by
   others is removed (EDG configures once, so its cache would keep the old ones); a build from before this record has
   no stamp and was made by the defaults. `build/env.sh` gains `CERASTES_BUILD_COMPILER`, `CERASTES_BUILD_CC` and
   `CERASTES_BUILD_CXX`. The CI scripts set the three inputs from the profile and refuse them from the environment
   (0016 item 7's list grows by these three). `CERASTES_CI_PROFILE` is the job's (item 7 reads it), so the scripts
   require it to equal their argument instead.
7. **Divergence** (from S8b). A test whose outcome on a toolchain differs from its `expect:` lines fails that job. It
   is fixed in cerastes, or the profile records an exclusion: the test's path, optionally the mode, and a reason. An
   excluded test still runs and must still fail (reported as `diverges (profile <name>: <reason>)`); one that passes
   fails the run with "remove the exclusion", so an exclusion cannot outlive its cause, and one that names a missing
   test fails the run. A reference profile carries no exclusions: 0031 item 6 (every test gives the same outcome on
   both platforms) still holds for the two references. There are no per-toolchain `expect:` lines.
8. **The guard** (from S8b; amends 0016 item 10). `CiGuardTests` stops holding a copy of the whole active text of
   `ci.yml`, `ci.sh` and `ci-linux.sh`. It keeps the structural rules that make a weakening fail, and adds:
   - the profile schema (`ci_profile.py check`, run by the self-test);
   - the two references' declared facts and their `every` cadence, pinned in the self-test;
   - every required assertion present in the scripts, in order, as a pattern rather than a value.
   The structural rules kept are: `set -euo pipefail`; `||` only as `|| want`; no `|| true`, `exit`, `return`, `&`,
   `trap` or `alias`; bootstrap run once; the environment ban; the workflow's keys, triggers, permissions and
   concurrency; `fail-fast: false`; the matrix expressions; no `continue-on-error`; SHA-pinned actions; the
   checkout's settings; the pinned `if:` expressions; the run lines. A new toolchain is one profile entry; changing a
   reference, or weakening a rule, is a self-test change that review sees.
9. **What a job reports** (from S8b). Each job's toolchain report names its profile and every exact build: the image,
   the CLT or LLVM and GCC packages, the banners, the SDK version and build or glibc and its packages, the scrape,
   Java, Z3, Python, CMake, Ninja, Node. Comparing two nightly reports shows what moved.
10. **GCC builds** (from S8c). With `build = "gcc"` (Linux), EDG's preset build and cerastes's CMake compile with
    `gcc-M`/`g++-M` (the profile's `gcc_major`), through the same compiler arguments bootstrap already passes on Linux.
    The scrape and the native-ok clang still use `clang-N` over libc++: EDG emulates clang whatever compiles it (0002 is
    unchanged), so the verified language, the headers and the goldens are the same as in a clang build. What GCC's
    `-Wall -Wextra -Werror` finds in `src/` is fixed in the code style (0051), proved behaviour-neutral by
    `scripts/object_identity.py` on the clang build. What it finds in cerastes's EDG flags (`EdgFrontEnd.cmake`) gets
    flags per compiler. What it finds in EDG's sources becomes small fork commits (invariant 5).
    *Found in S8 (GCC 15.2, Ubuntu 26.04):* EDG compiles cleanly in both configurations (the stock preset build and
    `edg_fe` with cerastes's flags); no fork commit. cerastes's own sources gave one family, `-Wdangling-pointer` (in
    `-Wall`), reporting pointers to the temporaries of a `?:` whose arms are copied into a stored value
    (`src/vir/lower.cpp`'s heap holders, two VIR self-test cases). Every value involved is held by copy (`Cell`,
    `HeapHolder`, the `fn` helper's parameter list), so the reports are false (the soundness audit read every type
    involved and found no reference, pointer or view into a temporary). They are all of level 2 ("may be used"). The
    six targets now share `CERASTES_WARNINGS` (the top-level `CMakeLists.txt`), `-Wall -Wextra -Werror` with
    `-Wdangling-pointer=1` for GCC only: level 1 still reports definite dangling uses, and both files compile cleanly
    with it under GCC 15.2 and 16.2 (the audit's suggestion over turning the warning off); clang's 104 compile commands
    are unchanged. The first full run then failed 165 tests: EDG's CMake takes its default `EDG_BASE` (the predefined
    macros of the emulated compiler) from the compiler that builds it, so the GCC build got `bases/docker/dev-env/gcc`
    and emulated clang without clang's macros (libc++ stops at its `__BYTE_ORDER__` check). `bootstrap.sh` names the
    clang base on Linux, the one EDG chooses for a clang build, so a clang build is unchanged.
11. **ubuntu-24.04** (from S8c). glibc 2.39, clang 18 with libc++ 18, GCC 14, CMake 3.31, ShellCheck 0.9.0. The
    encoder matches glibc 2.43's `assert` expansions exactly (0031 item 5). A 2.39 shape that differs is an encoding
    rule, with its must-verify and must-fail tests (invariant 2), an item here, and the soundness audit. Anything
    larger stays an exclusion or keeps the profile at `dispatch`, and goes to the owner.
    *Found in S8:* clang 18 refuses `const bool is_break {l->break_label};` (`src/backend/encode.cpp`, a one-bit EDG
    bit-field narrowed to `bool` in a braced initializer), which clang 21 accepts; it is now `{l->break_label != 0}`,
    like the line after it (object file and IR identical under Apple clang 21, `object_identity.py`). Nothing else in
    the build differs.
12. **Out of scope** (M8 entries):
    - arm64 Linux runners (`ubuntu-*-arm`): plain `char` and `wchar_t` are unsigned there, which the encoder's target
      facts do not model;
    - macos-15 (Apple clang 17: EDG would emulate clang 17 over SDK 15's headers);
    - toolchains from apt.llvm.org or the toolchain PPA;
    - GCC builds on macOS.

## Consequences

- An image rollout within a major no longer stops CI; a rollout that changes a result still does, through the tests.
- The reference profiles fix what "the owner's toolchain" means: majors, not a build. The owner's Mac and the runner
  can differ in a patch level, and the report shows by how much.
- Other toolchains are tested without changing what the references promise. A toolchain that cannot be made green
  stays `dispatch`, or carries exclusions that say why.
- The guard no longer makes every CI edit a two-file edit, which is what broke `main` at `c1b691d`. It still rejects
  every weakening it rejected before.
