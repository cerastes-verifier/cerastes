# Status

Session handoff log, newest first. Each entry says what is done, what is not,
any surprises, the decision records added and the exact first step for the
next session.

## M0 — Bootstrap and toolchain (2026-10-13): done

**Done**
- Repository skeleton: `LICENSE.txt` (Apache-2.0 WITH LLVM-exception),
  `.gitignore`, `README.md`, `CLAUDE.md`, `.claude/agents/`
  (`soundness-auditor`, `test-mutator`), `docs/status.md`,
  `docs/decisions/0001-toolchain-pins.md`, `tests/README.md`.
- Placeholders renamed in `plan.md`: `vcpp` → `cerastes`,
  `[[verifier::…]]` → `[[cerastes::…]]`, `<verifier.h>` → `<cerastes.h>`,
  `namespace verifier` → `namespace cerastes`, `__VCPP__` → `__CERASTES__`.
  "verifier" as an ordinary noun and the `VF_*` macro names were left as they
  are.
- Submodules: `third_party/edg` = upstream `edgcpp/compiler` at its `main`
  head (`9847356d`); `third_party/silicon` = `v.26.08-release` (`6b540aa4`),
  with nested `silver` at `f02e7010`. Invariant 5 in `plan.md` and
  `CLAUDE.md` now says the EDG pin is always an upstream commit.
- `scripts/bootstrap.sh`: idempotent. Installs the Homebrew prerequisites
  (cmake, ninja, python, shellcheck, sbt, openjdk@17); initializes missing
  submodules without moving checked-out ones; builds EDG out of tree
  (`build/edg`, stock `macos-arm-clang-debug` preset); runs `sbt assembly`
  for Silicon, skipped when a build stamp shows the jar is current; downloads
  Z3 4.16.0 and checks its SHA-256; writes `build/env.sh` (`Z3_EXE`,
  `SILICON_JAR`, `CERASTES_JAVA`, `CPFE`, `EDG_BASE`, scraped clang include
  paths). `--build-only` is the build step that `check.sh` uses.
- `scripts/cpfe.sh`: runs `cpfe` with clang emulation and the system include
  paths (C or C++ chosen by the mode option).
- `scripts/run_tests.py`: parses `expect:`/`mode:` headers; runs `.vpr`
  through Silicon (all errors reported) and `frontend-ok` C/C++ tests through
  `cpfe`; matches errors exactly by category, optional Viper reason and
  line; reports wrong line, wrong reason, missing and unexpected errors;
  always fails on timeouts, crashes and unrecognized messages.
  `scripts/test_run_tests.py` (23 tests) checks all of this, including
  end-to-end runs on mutated copies of `fail.vpr`.
- Tests: `tests/viper-smoke/{ok,fail,fail_kinds}.vpr`,
  `tests/edg-smoke/hello.{cpp,c}`.
- `scripts/check.sh`: build → lint (`bash -n`, shellcheck, `py_compile`,
  header check) → harness self-test → tests. Green.

**Acceptance (all met)**
- Clean clone: `git clone` of the M0 commit, then
  `scripts/bootstrap.sh && scripts/check.sh`, was green in 1 min 45 s. That
  run fetched the submodules from GitHub, configured and built EDG, built
  Silicon and downloaded Z3. Homebrew packages and the sbt/coursier caches
  were already warm, so a first run on a new machine takes longer. A second
  `bootstrap.sh` in the same tree is a 1.5 s no-op.
- `fail.vpr` is reported as `postcondition:assertion.false @ L12`, and
  mutating its expected line or category makes the harness fail.
- `cpfe` parses a C++20 and a C23 hello world with no diagnostics.

**Not done / deferred**
- `scripts/lint_viper.py` and `scripts/mutate.py` belong to M3; `check.sh`
  has a lint step ready for them.
- `.c`/`.cpp` tests with `verified`/`error`/`rejected` expectations fail with
  "needs the cerastes driver (M3)" until the driver exists.
- VeriFast is not built. The local `generic2` binary is a Linux x86-64 ELF;
  M4 decides how to run it (see 0001).

**Surprises**
- EDG at the pinned commit cannot parse `<iostream>`, `<string>`, `<vector>`
  or `<algorithm>` from the macOS SDK 27.0 libc++: errors in
  `__algorithm/find.h` and `simd_utils.h`. `<concepts>`, `<cstdint>`,
  `<type_traits>`, `<limits>`, `<utility>` and `<cstdio>` are fine, so the
  smoke test uses `<cstdio>`. Worth an upstream EDG issue; it will matter once
  tests include container headers.
- Silicon's pinned sbt 1.6.2 cannot run on JDK 27 (Homebrew's `openjdk`), so
  the toolchain uses `openjdk@17`.
- Homebrew's Z3 is 5.1.0, but Silicon is tested against 4.16.0, so Z3 is a
  pinned download.
- Silicon pads error indices (`[ 0]`) once there are ten or more errors.
  The harness handles this, and `fail_kinds.vpr` (11 errors) covers it.
- Silicon has no machine-readable error ids on the command line. The harness
  classifies errors by silver's fixed message text, and `fail_kinds.vpr`
  pins that table to the Silicon version.

**Decision records added:** 0001-toolchain-pins.

**First step for M1:** read `CLAUDE.md`, the M1 section of `plan.md`, this
file and `docs/decisions/`. Then decide where the cerastes EDG configuration
lives. Under the upstream-only pin, prefer a configuration that needs no EDG
source change (for example a macro-conf or defines block kept in this repo
and passed to EDG's CMake); otherwise upstream it first. Then copy the six
VeriFast tests from `~/Documents/verifast/tests/cxx/templates/` (branch
`generic2`, `0fb63506`) into `tests/differential/verifast/`. Those files have
no `expect:` headers, so `run_tests.py` must be taught to skip that directory
or they will fail its header check.
