# Status

Session handoff log, newest first. Each entry says what is done, what is not,
any surprises, the decision records added and the exact first step for the
next session.

## M2 — Annotation layer (2026-10-14 to 2026-10-15): done locally; pushes pending

Read `internal/handoff.md` next. Its §2 lists the owner's push steps in
order, and §10 has the questions.

**Owner decisions this milestone**
- EDG work is a local commit series on the fork's `cerastes` branch; the
  owner pushes.
- **cerastes pins the fork** (decision 0009, 2026-10-14). It replaces
  "upstream only". Each session starts by merging upstream `main` into the
  fork (never rebase).
- The macros are named `CERASTES_*`.
- Loop invariants are prefix only.
- `forall` takes its free variables as trailing arguments.
- Redeclarations must match (the VeriFast rule).
- M1's probes were renamed from `cerastes::` to `unreg_ns::`.
- The full EDG `all` suite on the x86-64 box is the owner's step.

**Done**
- **The EDG fork series** A, FILT, B, C, D0, D, F, E, G, H, I, J and K, on
  base `034385ba0a`, with the pin at K (`23f55e3628`). It adds:
  - a general attribute-registration mechanism, with no `cerastes` string
    in EDG;
  - attributes after the parameter list attached to the routine;
  - must-match across redeclarations and explicit specializations;
  - class-template substitution;
  - result names;
  - substitution that fails closed.

  Each commit has `GH-*` tests, fail-before against its parent, and a
  `changes,regressions` run against the expectations (handoff §3).
- **cerastes registers its eight attributes** from the driver
  (`src/backend/register_attributes.cpp`). The walker prints each one, on
  its entity, with a typed expression tree: dependent in prototypes,
  substituted in instances.
- **24 cerastes-side checks** (`src/backend/diagnostics.def`) reject what
  EDG cannot. They fail closed on anything the listing cannot represent.
- **`<cerastes.h>`** has the `CERASTES_*` macros and the declared-only spec
  functions. It keys on `__has_cpp_attribute(cerastes::requires)`. The
  same tests build with Apple clang and GCC 16 under
  `-Wall -Wextra -Werror` (`native-ok`).
- **Tests.** `tests/annotations/` has 242 files:
  - 188 negatives, each with a twin;
  - positives with goldens, each checked against `--il_display`.

  The harness gained `frontend-refused`, `native-ok`, canaries and
  `--check-headers`. There are 675 self-tests.
- **Docs.** The Annotations page and "Building with ordinary compilers";
  every example is a test.
- **Upstream draft:** `internal/upstream/attribute-registration.md`.
- **Audits.** CP1 to CP8b (Opus). Every blocking finding is fixed; the
  open should-fix items are in handoff §4 item 8.

**Acceptance.** See handoff §9.
- `check.sh` is green.
- The annotation tests and doc examples pass, with native-ok on Apple clang
  and GCC.
- The docs build with zero warnings.
- **Pending the owner's pushes** (fork, then cerastes, then the website):
  publication at https://cerastes.dev/cerastes/doc/.

**Not done / deferred**
- **The full EDG suite** (`imported,edg,cwg,stdlib`, both x86-64
  configurations) has not been run at K. The run at J found the crash that
  K fixes; it is the owner's step on the x86-64 box (handoff §2).
- The mutation runs for H and I were never repeated after their final
  versions.
- The open CP7/CP8 items (handoff §4 item 8).
- CI for the repository (a session before M3).

**Surprises**
- **The full suite found a front-end crash** in our own series. G stored,
  and I dereferenced, an uninitialized template symbol for explicit
  specializations declared inside a class template. It crashed the x86
  ASan build on `gnats-23971*`, `d00244a` and `ms_cpgen01`. Native arm64
  survived by luck, and `cerastes-fe` runs the same path. Fixed by K. The
  lesson: `changes,regressions` is not enough before a pin moves.
- **The audits found real bugs at every round.** Silent drops, misattached
  attributes, parameter numbering after packs, crashes, and an EDG change
  that altered ordinary overload resolution (the first J, amended before
  it was pinned).
- **Narrowing a check without an audit regressed it** (CP8). The shadow
  checks now fail closed.
- **The session's scratchpad was lost at a context reset.** The per-commit
  evidence and the auditor's oracle went with it. Evidence now lives in
  the fork's `.tmp/agent-*` (handoff §3).

**Decision records added:** 0005-edg-source-override,
0006-annotation-language-v0, 0007-edg-attribute-registration,
0008-cerastes-header, 0009-edg-fork-pin.

**First step for the next session.** Confirm that the owner has pushed the
fork's `cerastes` branch, then cerastes `main`, then the website (handoff
§2). Then merge upstream `main` into the fork's `cerastes` (0009), move the
pin and run `check.sh`. Then the CI session (a decision record; a macOS
runner first, x64 Docker if the goldens drift), before M3.

## M1.5 — User documentation site (2026-10-14): done locally; live deploy pending the owner's push

Read `internal/handoff.md` next.

**Owner decisions this session**
- **Q1–Q8.** The answers to the M1 handoff questions are folded into
  0001, 0003, CLAUDE.md and plan.md (commit `78c8010`).
- **No docs-only repo** (Q4). The docs are this repository's own Pages site.
- **The address** is **https://cerastes.dev/cerastes/doc/**. The org
  convention is `cerastes.dev/<repo>/doc/` (decision 0004).
- **C++ in AsciiDoc prose** is written `pass:[C++]`.
- **Developer docs** move from the `docs` directory to `internal` (a
  separate mechanical commit after the M1.5 commit).
- **Scope for the EDG proposal.** VeriFast parity and beyond, with specs in
  C++ grammar inside `[[cerastes::…]]` and no new EDG grammar.

**Done**
- **Antora component in `doc/`.**
  - `antora.yml` (`ROOT`, unversioned);
  - `local-playbook.yml` (builds the worktree into `build/doc/`, fails on
    warnings, lunr search);
  - `package.json` and its lockfile (Antora 3.2.1 and lunr 1.0.0-alpha.13,
    exact pins);
  - `.node-version` (26).
- **UI.** The default Antora UI, vendored as `doc/ui/antora-ui-default.zip`
  (SHA-256 checked, and forced with `--ui-bundle-url`), plus supplemental
  files in `doc/ui/supplemental/`:
  - an Ink navbar with the lockup;
  - Archivo;
  - the favicons;
  - Sand used only as a graphic on light grounds;
  - links in a docs-only `#7f5f2d`.

  The assets are byte-identical copies from the brand kit and the website.
  Provenance is in `doc/ui/README.md`.
- **Pages.** Overview (what cerastes is, the pipeline, honest status) and
  Installation (requirements, bootstrap, check, trying the front end,
  building the docs, known limitations). Every claim was checked: a claims
  ledger by il-analyst, then re-verification by the auditor.
- **Examples are tests.**
  - `doc/modules/ROOT/examples/installation/hello.cpp` (`frontend-ok`).
  - `run_tests.py` discovers `doc/modules/ROOT/examples/` by default, and a
    missing default directory is an error.
  - `scripts/check_doc_examples.py` is the source lint. Its rules (a)–(e) and
    their extensions are in `tests/README.md`.
  - `--html build/doc` is the rendered backstop.
- **Scripts.**
  - `scripts/build_docs.sh [PLAYBOOK]` is new.
  - `bootstrap.sh` installs Homebrew `node` and runs `npm ci --ignore-scripts`
    in `doc/`, behind a stamp of the lockfile and `package.json` that is
    checked against `node_modules/.package-lock.json`. With `--build-only`, a
    stale install is an error.
  - `check.sh` adds the source lint to its lint step, and a final docs step:
    the build, then `--html`.
- **CI** (`.github/workflows/docs.yml`).
  - Pull requests run the lint, the build and `--html`.
  - Pushes to `main` that touch `doc/**` or the doc scripts do the same, then
    deploy to Pages. The Antora site is staged at `doc/` in the artifact, with
    a root redirect and a root `404.html`.
  - Actions are pinned to commit SHAs, on `ubuntu-24.04`. Deploys share a
    concurrency group, so they land in order. Least privilege, and checkout
    does not keep its credentials.
- **Self-tests** went from 140 to 305. Each new check has a twin. The
  end-to-end twins build real Antora sites and show three things:
  - warnings fail `build_docs.sh`, but not bare Antora;
  - lint-evading pages (q1–q18, e1/e2/e5) fail the source lint, `--html`,
    or both;
  - the `check.sh` and CI wiring cannot be weakened (`|| true`, comments,
    `continue-on-error`).
- **Upstream draft for the owner.** `internal/upstream/attribute-registration.md`
  plus 16 probes (handoff §5).
- **Decision records.**
  - 0004-docs-site (accepted).
  - 0001 gains the docs pins and the GitHub Actions SHAs, and corrects the
    libc++ limitation: `<utility>` fails too, and there are three causes.
  - 0003 records the owner's push and pointer answers.

**Acceptance.** See "M1.5 acceptance" in `internal/handoff.md`.
- **Met locally:** zero-warning docs build in `check.sh`; every doc example
  runs as a test; branding matches cerastes.dev. The screenshots are
  `internal/images/m1.5-docs-overview.png`, served under `/cerastes/doc/`, and
  `internal/images/m1.5-website-reference.png`.
- **Pending the owner's push:** the live URL returns 200, search works live,
  and a docs-only commit appears with no manual step.

**Not done / deferred**
- CI for the rest of the repository; the owner wants it soon, on x64 Docker
  if macOS runners cannot reproduce the goldens. It needs a decision record.
- The "Documentation" link on cerastes.dev (website repo, owner).
- Fast-forwarding the fork branch (owner).

**Surprises**
- `https://cerastes.dev` had no certificate for the custom domain (a
  Cloudflare-proxied `www`); `.dev` is HSTS-preloaded. The owner fixed it.
- Antora resolves non-`./` playbook paths against the current directory.
- A bare `C++` in AsciiDoc swallows text silently.
- `<utility>` was misreported as parsing in M0.
- The default Antora UI hard-codes its colours, so `cerastes.css` overrides
  selectors.

**Decision records added:** 0004-docs-site.

**First step for the next session (M2).** Read `CLAUDE.md`, the M2 section
of `plan.md`, this file, `internal/handoff.md` §5 and the upstream draft
`internal/upstream/attribute-registration.md`. Check whether the owner has
fast-forwarded the fork branch and filed the Discussion (handoff §8). Then
write M2's decision record for the attribute design (invariant 6), sized for
the full scope in the draft.

## Decision 0003 (2026-10-13): EDG fork branch

The owner created branch `cerastes` on the fork
https://github.com/mborland/compiler. `internal/decisions/0003-edg-fork-branch.md`
records how it is used:
- every EDG source change is made there, never in `third_party/edg`;
- the submodule only ever pins upstream `main` commits, and no commit of this
  repository records a fork pointer;
- the branch is based on the pinned upstream commit, and is separate from
  the `cir` branch;
- each commit follows EDG's `CONTRIBUTING.md` checklist, and general
  mechanisms are preferred to cerastes-specific code inside EDG;
- cerastes code that needs an unmerged EDG change waits until the pin moves.

**Owner rules (2026-10-13):** sessions may commit and push to the fork's
`cerastes` branch. Sessions never open issues, PRs or Discussions against
upstream; the owner files them from text drafted in `internal/handoff.md`.
Recorded in 0003 item 6, CLAUDE.md and the `edg-backend` agent.

**Not done:** the branch tip is `90b66472`, the fork's old `main`, 22 commits
behind the pin `9847356d`. It must be fast-forwarded before its first commit.
This session had no GitHub push credentials, so that is handoff question 1.
From now on, every handoff ends with a "Questions for the owner" section
(`internal/handoff.md` §8).

**Decision records added:** 0003-edg-fork-branch (accepted). G1 passed, so
the fallback record plan.md had reserved 0003 for was never written. A
fallback needed after a later pin move takes the next free number.

## M1 — EDG IL inventory and integration spike (2026-10-13): done, Gate G1 PASS

Read `internal/handoff.md` next. It has the re-look list, the friction log and
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
- **`internal/il-inventory.md`.**
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
  passes 2, 3, 3b, 3c and 3d. The front-end fallback record is not needed.

**Acceptance (all met):** see "M1 acceptance" in `internal/handoff.md` for the
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
the M1.5 section of `plan.md`, this file, `internal/handoff.md` §4 and
`internal/decisions/`. Then write `internal/decisions/0004-docs-site.md` (0003 is
the EDG fork-branch record; see the entry above). If the owner wants M2
first, do this instead: once the fork branch `cerastes` has been
fast-forwarded to the pin, start the upstream issue and PR cycle for
registering the `cerastes::` attributes (decision 0003, handoff §5). Nothing in M2 can
depend on that registration until it is upstream.

## Plan update (2026-10-13): user documentation with Antora

Added **M1.5 — User documentation site (Antora)** to `plan.md`. User docs are
an Antora component in `doc/`, published at https://cerastes.dev/doc/. The
default hosting is an org repo named `doc`, whose GitHub Pages project site
GitHub serves under the org's `cerastes.dev` domain; it is recorded in
decision 0004 when M1.5 runs. New global invariant 9 (CLAUDE.md invariant 8):
user-visible behavior is documented in `doc/` in the milestone that adds it,
and every doc example is a test. M2–M7 each gained a docs task. The
annotation reference moves from the planned `internal/annotations.md` to `doc/`.
M1 is unaffected. M1.5 depends only on M0 and must run before M2. Creating
the `doc` repo, enabling Pages and adding the dispatch token are owner steps.

## M0 — Bootstrap and toolchain (2026-10-13): done

**Done**
- Repository skeleton: `LICENSE.txt` (Apache-2.0 WITH LLVM-exception),
  `.gitignore`, `README.md`, `CLAUDE.md`, `.claude/agents/`
  (`soundness-auditor`, `test-mutator`), `internal/status.md`,
  `internal/decisions/0001-toolchain-pins.md`, `tests/README.md`.
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
file and `internal/decisions/`. Then decide where the cerastes EDG configuration
lives. Under the upstream-only pin, prefer a configuration that needs no EDG
source change (for example a macro-conf or defines block kept in this repo
and passed to EDG's CMake); otherwise upstream it first. Then copy the six
VeriFast tests from `~/Documents/verifast/tests/cxx/templates/` (branch
`generic2`, `0fb63506`) into `tests/differential/verifast/`. Those files have
no `expect:` headers, so `run_tests.py` must be taught to skip that directory
or they will fail its header check.
