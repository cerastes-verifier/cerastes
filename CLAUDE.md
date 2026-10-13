# cerastes — C/C++ verifier (EDG → Viper/Silicon → Z3)

Plan: @plan.md   Status: @docs/status.md
User docs: doc/ (Antora, from M1.5) → https://cerastes.dev/doc/

## Commands
- Bootstrap: scripts/bootstrap.sh   (idempotent; macOS on Apple silicon)
- Everything: scripts/check.sh   (must be green before ending a session)
- Tests only: python3 scripts/run_tests.py [file|dir ...]   (-v shows tool output)
- Harness self-test: python3 scripts/test_run_tests.py
- EDG front end (stock): scripts/cpfe.sh --c++20 [cpfe options] <file>   (adds system headers)
- EDG in the cerastes configuration (decision 0002) + our back end:
  scripts/cerastes-fe.sh --c++20 [--cerastes_walk | --il_display] <file>
  (--cerastes_walk prints the IL walker listing; --il_display is IL evidence)
- Golden tests: python3 scripts/run_tests.py --update-golden <test>   (a golden is
  an expectation: invariant 1 applies; review the diff, give the reason)
- Pinned tools in your shell: . build/env.sh   (Z3_EXE, SILICON_JAR, CERASTES_JAVA, CPFE, CERASTES_FE)
- Mutants: python3 scripts/mutate.py <dir>   (from M3)
- Verify one file: build/cerastes verify <file> --keep-vpr   (from M3)
- User docs: scripts/build_docs.sh   (from M1.5; builds doc/ into build/doc, fails on warnings)

## Invariants (non-negotiable)
1. Never edit an `expect:` line to make a test pass; stop and ask instead.
2. Every encoding rule: ≥1 must-verify and ≥1 must-fail test.
3. Unsupported constructs are rejected with a diagnostic, never approximated.
4. No inhale/assume in emitted Viper without `// cerastes:inhale-ok <reason>`.
5. third_party/edg pins upstream edgcpp/compiler only; EDG edits are small
   commits with tests on a `cerastes` fork branch, upstreamed before the pin moves.
6. Non-obvious semantics → docs/decisions/NNNN-*.md before code.
7. Errors shown to users point at C++ source lines.
8. User-visible behavior lands with its page in doc/ in the same milestone;
   every code example in doc/ is a test with `expect:` lines (from M1.5).

## Layout
src/backend (EDG IL → VIR) · src/vir · src/viper (VIR → .vpr + pos map) ·
src/driver · include/cerastes.h · tests/<area>/ · scripts/ ·
doc/ (user docs, Antora, published) · docs/ (developer docs, not published) ·
third_party/edg (EDG, upstream) · third_party/silicon (Silicon + silver)

## Toolchain
- Pins and the reasons for them: docs/decisions/0001-toolchain-pins.md.
  scripts/bootstrap.sh is the executable form; change a pin in both.
- EDG twice: stock build in build/edg (cpfe, edg-smoke tests) and the cerastes
  configuration compiled by our CMake into build/cerastes-fe (build/cmake;
  decision 0002). The EDG macros live only in the `edg_config` target; every
  TU that includes EDG headers links it (IL layout depends on them). EDG IL
  facts we rely on: docs/il-inventory.md.
- EDG builds out of tree into build/edg; never build inside third_party/edg.
- Silicon needs JDK 17 (its sbt 1.6.2 cannot run on newer JDKs); use
  $CERASTES_JAVA, not the java on PATH.
- Z3 is the pinned 4.16.0 in build/tools, not Homebrew's z3.

## Conventions
- C++20 for tool code; EDG code follows EDG's own style in third_party/edg.
- Test files start with `// expect:` lines; grammar and categories in
  tests/README.md (see also plan.md §0).
- Annotation names: never `assert` (macro collision); use `check`.
