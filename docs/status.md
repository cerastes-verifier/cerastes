# Status

Session handoff log, newest first. Each entry says what is done, what is not,
any surprises, the decision records added and the exact first step for the
next session.

## M1 — EDG IL inventory and integration spike (2026-10-13): done, Gate G1 PASS

Read `docs/handoff.md` next. It has the re-look list, the friction log and
what M1.5, M2 and M4 inherit.

**Done**
- **Integration (decision 0002, accepted).** cerastes compiles EDG's
  front-end sources itself, in the "cerastes configuration", and links an
  in-process `back_end()`. Nothing in `third_party/edg` changes; the pin is
  still `9847356d`.
  - The configuration: no lowering, `ALL_TEMPLATE_INFO_IN_IL`,
    `RECORD_UNRECOGNIZED_ATTRIBUTES`, `EXTRA_SOURCE_POSITIONS_IN_IL`, used
    templates instantiated in the TU, and no removal of unneeded entities.
  - The build:
    - top-level `CMakeLists.txt`, `cmake/EdgFrontEnd.cmake`;
    - the `edg_config` interface target, the only place the EDG macros are
      defined;
    - `edg_fe`, compiled with stock cpfe's exact flags;
    - `src/backend/`, `src/driver/`.
  - The output is `build/cerastes-fe`, built by `scripts/bootstrap.sh` and
    run with `scripts/cerastes-fe.sh`.
  - Guards: `static_assert`s in `src/backend/edg_il.h`, and
    `scripts/check_edg_config.sh` as a `check.sh` step.
- **IL walker** (`--cerastes_walk`). It prints a deterministic listing:
  - routines (plain, prototype, instance, specialization), with body or
    nobody;
  - signatures, template arguments and constraints;
  - every unrecognized or `cerastes::` attribute with its owner;
  - attributed statements;
  - the operations and template-parameter constants in prototype bodies,
    including those nested in `tpck_expression` constants;
  - a final `end walk:` line that the harness requires;
  - cross-check warnings for G1 regressions (`--cerastes_walk_selftest`
    proves two of the three can fire; the third covers pruned bodies, and
    `back_end()` refuses to run in the pruning mode).
- **Harness.** New things in `scripts/run_tests.py`:
  - the `// expect: golden` kind, which compares stdout plus a stderr
    section;
  - `--update-golden`, which `check.sh` never runs, enforced by a self-test;
  - orphan-golden detection;
  - a `.harness-skip` marker, allowed only in `tests/differential/verifast`;
  - `CERASTES_REQUIRE_E2E=1` in `check.sh`, so end-to-end self-tests fail
    instead of skipping.

  `scripts/test_run_tests.py` has grown from 23 to 140 self-tests,
  including mutation twins for every harness check.
- **Tests (`tests/il/`).**
  - G1 probes `g1/check{1..5}`.
  - Annotation-free copies of the six VeriFast template tests,
    `verifast-plain/`.
  - Inventory probes `inventory/`.
  - Walker smoke tests `walker/`.
  - Contrast probes `contrast/`, which rule out vacuous passes.

  All are golden tests. Each golden was checked against `il_display` before
  it was cut, by an agent other than the walker's author: il-analyst for g1,
  verifast-plain, inventory and walker; test-mutator for contrast. The
  auditor independently matched every prototype's `op` lines in all goldens,
  and the attribute lines of the g1, inventory and sampled contrast goldens. The VeriFast
  originals are byte-identical in `tests/differential/verifast/` (generic2
  `0fb63506`) and skipped by the harness.
- **`docs/il-inventory.md`.**
  - The five G1 checks, with `il_display` evidence.
  - Every Clang query used by VeriFast's `GenericFunctionChecker`,
    `collectIntegralTypeParams`, `usesGenericProof` and the call-site and
    specialization code (74 rows), mapped to its EDG IL field.
  - The gotchas for M2 and M4.
- **Agents.**
  - `.claude/agents/edg-backend.md` and `il-analyst.md` are new, Sonnet.
  - `soundness-auditor` is now Opus, read-only, and also reviews gate
    evidence and harness changes.
  - `test-mutator` is now Sonnet, owns the harness, and is told that golden
    files are expectations.

**Gate G1: PASS.** Proceed to M2.
- Check 1 PASS: `v + 1` is `eok_add` with `type_kind tk_template_param`, and
  `(T)` is its own `eok_cast`.
- Check 2 PASS: a template that is never instantiated has a prototype
  routine with a full body.
- Check 3 PASS: instances carry `assoc_template` and deduced or explicit
  `template_arg_list`.
- Check 4 PASS: `std::integral` is reachable from all four constraint
  locations.
- Check 5 PASS for every placement the plan lists: prefix on a function
  declaration, loop statements and null statements, each plain, in a
  function-template prototype and in its instance.
- The placement after the parameter list, where M2 puts `requires`/`ensures`,
  is not carried reliably by the unrecognized path. It lands on the function
  type and is lost on redeclarations. So M2 takes plan.md's "check 5 fails"
  branch for that placement and registers the attributes explicitly. The Opus
  auditor reproduced the evidence and agreed with this verdict in review
  passes 2, 3, 3b, 3c and 3d. Decision 0003 is not needed.

**Acceptance (all met):** see "M1 acceptance" in `docs/handoff.md` for the
commands and their results.

**Not done / deferred**
- VeriFast is still not built (0001); M4 decides.
- The walker reports only the primary source file. Code in user headers is
  counted, not shown, and M3 needs a policy for it.
- No `doc/` page. M1 adds no user-visible behaviour; `cerastes-fe` is a
  developer tool.

**Surprises**
- Turning off EDG's back ends silently turns on automatic template
  instantiation (`.ti` files, `tim_none`). The stock `cmake_defines.h` also
  pre-empts `host_envir.h`'s derived defaults. So every load-bearing macro is
  pinned, including `DEFAULT_REMOVE_UNNEEDED_ENTITIES=0`, without which
  templates that are never instantiated could be pruned.
- `il_display` does not print attributes attached to statements, or some
  template-parameter constants. A grep of it once produced a false "attribute
  lost" report.
- `(T)100` is not an `eok_cast`. It is a `tpck_expression` constant wrapping
  a `tk_unknown` cast, which M4's fail-closed rule must special-case
  narrowly.
- Class-template members do keep unrecognized attributes, despite
  `attribute.c:10906`.
- New `.claude/agents` types are only loaded at session start.

**Decision records added:** 0002-backend-integration (accepted).

**First step for the next session (M1.5, by plan order).** Read `CLAUDE.md`,
the M1.5 section of `plan.md`, this file, `docs/handoff.md` §4 and
`docs/decisions/`. Then write `docs/decisions/0004-docs-site.md` (0003 stays
unused because G1 passed). If the owner wants M2 first, do this instead:
open the EDG fork branch `cerastes` and start the upstream PR cycle for
registering the `cerastes::` attributes (handoff §5). Nothing in M2 can
depend on that registration until it is upstream.

## Plan update (2026-10-13): user documentation with Antora

Added **M1.5 — User documentation site (Antora)** to `plan.md`. User docs are
an Antora component in `doc/`, published at https://cerastes.dev/doc/. The
default hosting is an org repo named `doc`, whose GitHub Pages project site
GitHub serves under the org's `cerastes.dev` domain; it is recorded in
decision 0004 when M1.5 runs. New global invariant 9 (CLAUDE.md invariant 8):
user-visible behavior is documented in `doc/` in the milestone that adds it,
and every doc example is a test. M2–M7 each gained a docs task. The
annotation reference moves from the planned `docs/annotations.md` to `doc/`.
M1 is unaffected. M1.5 depends only on M0 and must run before M2. Creating
the `doc` repo, enabling Pages and adding the dispatch token are owner steps.

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
