# S8 — CI toolchain profiles (declared majors, a matrix, GCC builds)

## Context

CI asserts exact toolchain builds (0016 item 2, 0031 item 2): CLT `27.0.0.0.1788430756`, SDK build `26A425`,
banner `clang-2100.3.34.2`, `1:21.1.8-6ubuntu1`, GCC `16-20260322-1ubuntu1`. GitHub is rolling out xcode-27 image
`20261104.0337` (pre-release; rollouts take 2–6 days). Its only toolchain change is CLT 27.2.0.0.1792721180, and runs
land on either image at random:
- run 42158664485 (image 20261104) failed with "CLT 27.2…, expected 27.0…";
- run 42232697813 (image 20261026, after c1b691d) failed the other way round.

c1b691d also left `CiGuardTests`' pinned copy at 27.0, so `main` now fails the harness self-test (confirmed locally:
`RealDocTreeTests.test_ci_workflow_and_script_pass_their_guards`), and the Linux job will go red too. The owner's Mac is
still on CLT 27.0.

The owner wants scripts that survive image updates and can test other clang and gcc versions and other runners. Owner
answers (2026-11-06):
- **Pins:** assert declared majors; record exact builds.
- **Divergence:** a different outcome on another toolchain fails the job. Fix it, or record a named exclusion in the
  profile. No per-toolchain `expect:` lines.
- **Cadence:** the references run both tiers; the extras run nightly and on dispatch.
- **Guard:** structural rules plus a profile schema; only the references' declared facts are pinned.
- **Sequencing:** a small fix first, then the profiles.
- **First toolchains:** other ubuntu-26.04 majors, ubuntu-24.04, macos-26. There is no `macos-16` label.
- **GCC builds:** a profile can build cerastes and EDG with GCC, in this work.
- The ruleset on `main` has no required status checks, so renaming jobs is safe.

Records: **0053-ci-toolchain-profiles.md**, written before any code (invariant 6). It amends 0016 items 2, 3, 9 and 10,
0031 item 2, and 0001's host-toolchain rows. The approved plan is saved as `.claude/plans/s8-ci-toolchains.md` and
committed with the planning commit on S8a's branch.

Split points (one context, question 77): **S8a** and **S8b** in this session. **S8c** is the pre-agreed stop-and-ask
point if the context runs long.

## S8a — declared majors on the two existing jobs (branch `s8a-ci-majors`, from `main`)

1. Planning commit: the plan file, plus record 0053 with every item. S8b's and S8c's items are marked "from S8b/S8c".
2. **`scripts/ci.sh`** asserts the following, and prints the exact build of each:
   - `CLT_MAJOR=27`: the version of the `pkgutil` package matches `27.*`.
   - `CLANG_MAJOR=21`: the banner matches `^Apple clang version 21\.`.
   - `SDK_MAJOR=27`: `xcrun --show-sdk-version` matches `27.*`. The real path is checked against
     `$CLT/SDKs/MacOSX$sdk_version.sdk`, computed rather than pinned. The SDK build is recorded, not asserted.
   - `GCC_MAJOR=16`: `-dumpfullversion` matches `16.*`. The sysroot must still equal the SDK.
   - `EDG_CLANG_VER_SCRAPE` lies in `[210000, 219999]`.
   - Everything else is unchanged (paths, `DEVELOPER_DIR`, the env ban, the EDG tree, the report).
3. **`scripts/ci-linux.sh`**:
   - The clang banner matches `clang version 21\.`.
   - The `clang-21`, `libc++-21-dev` and `libc++abi-21-dev` packages are installed; their versions are recorded.
   - GCC matches `16.*`, and its package version is recorded.
   - glibc stays exact (`2.43`, fixed within an Ubuntu release).
   - The scrape lies in the 21 range.
4. **`ci.yml`:** the job name becomes `check (macOS, Command Line Tools 27)`.
5. **The guard** (`scripts/test_run_tests.py`): update `CI_SH_PINNED`/`CI_SH_CANONICAL`, the Linux pins, the canonical
   `ci.yml` and the mutation tables (`sh_mutations`, the Linux tables). Add mutations that drop or widen each major
   check (for example, `21\.` → `2`, or the check removed). The whole-text guard stays until S8b replaces it.
6. **Docs:** 0001's host-toolchain rows; status lines on 0016 and 0031 ("amended by 0053"); `tests/README.md`'s CI
   section; `doc/modules/ROOT/pages/dev/testing.adoc`.
7. **Verify:**
   - `python3 scripts/test_run_tests.py` is green, and so is `shellcheck scripts/*.sh`.
   - `scripts/ci.sh` passes as a rehearsal on the owner's Mac (CLT 27.0), with nothing installed.
   - `scripts/build_docs.sh` passes.
   - Commit, then the handoff step "push `s8a-ci-majors` and merge". Both xcode-27 images now pass the preamble; the test
     suite decides any drift on 27.2.

## S8b — toolchain profiles and the matrix (branch `s8b-ci-profiles`, stacked on S8a)

**Profiles: `scripts/ci_toolchains.toml`, read by a new `scripts/ci_profile.py`** (Python 3.14 `tomllib`, run with
`-I`). Subcommands:
- `check` validates the schema.
- `shell <name>` prints validated `NAME='value'` lines for the CI scripts.
- `matrix --event E [--select names|all]` prints the JSON matrices for macOS and Linux.
- `exclusions <name>` is used by `run_tests.py`.

Schema `schema = 1`, one `[[profile]]` per toolchain:
- `name` matches `[a-z0-9.-]+` and is unique.
- `runner` is one of an allowed set.
- `cadence` is `every`, `nightly` or `dispatch`.
- macOS fields: `macos_major`, `clt_major`, `clang_major` (Apple), `sdk_major`.
- Linux fields: `ubuntu_version`, `glibc`. Here `clang_major` (at least 18) also selects `libc++-N`.
- `gcc_major` is at least 14, since GCC 13 has no `-std=c23` and the native-ok canary fails without it.
- `build` is `clang` or `gcc`; `gcc` is allowed on Linux only.
- `jobs` is a positive integer.
- `exclusions` is a list of `{test, mode?, reason}` entries, refused on `every` profiles.

First set:

| name | runner | cadence | clang | gcc | build | notes |
|---|---|---|---|---|---|---|
| `macos-27` | xcode-27 | every | Apple 21, SDK 27, CLT 27 | 16 (brew) | clang | reference |
| `ubuntu-26.04` | ubuntu-26.04 | every | 21 + libc++ 21 | 16 | clang | reference, glibc 2.43 |
| `ubuntu-26.04-clang20-gcc15` | ubuntu-26.04 | nightly* | 20 | 15 | clang | archive only |
| `ubuntu-26.04-clang22-gcc14` | ubuntu-26.04 | nightly* | 22 | 14 | clang | archive only |
| `ubuntu-26.04-gcc15-build` | ubuntu-26.04 | nightly* | 21 (scrape, native-ok) | 15 | gcc | S8c |
| `ubuntu-24.04` | ubuntu-24.04 | nightly* | 18 + libc++ 18 | 14 | clang | glibc 2.39; S8c |
| `macos-26` | macos-26 | dispatch | Apple 21, SDK 26, CLT 26 | 15 (brew) | clang | promoted after its first green run |

\* A profile becomes `nightly` only once its rehearsal on the x86-64 box is green; until then it is `dispatch`.

**`ci.yml`:**
- A small `profiles` job on ubuntu-26.04: checkout without submodules, then `ci_profile.py matrix` with outputs `macos`
  and `linux`.
- The selection: pull requests and pushes get `every`; the schedule gets `every` + `nightly`; `workflow_dispatch` gets
  a new `profiles` input (default: the nightly set; a list of names, or `all`, adds candidates).
- Jobs `check-macos` and `check-linux`:
  - `needs: profiles`, with `if:` on a non-empty matrix (an empty matrix errors).
  - `strategy: {fail-fast: false, matrix: ${{ fromJSON(...) }}}`, `runs-on: ${{ matrix.runner }}`, `name: check
    (${{ matrix.name }})`.
  - `env`: `CERASTES_CI_PROFILE` and `CERASTES_JOBS` from the matrix.
  - `run: scripts/ci.sh "$CERASTES_CI_PROFILE"`.
- Linux gets `setup-python` 3.14, because ubuntu-24.04's system Python is 3.12.
- Artifact names and the Silicon cache key carry the profile name, so matrix jobs never collide.
- Triggers, permissions, concurrency and `timeout-minutes: 220` are unchanged.

**CI scripts:** `ci.sh` and `ci-linux.sh` take the profile name and load `ci_profile.py shell` output. Every assertion
becomes a check against the profile's declared facts, as in S8a.
- Linux installs `clang-N`, `libc++-N-dev`, `libc++abi-N-dev`, `gcc-M`, `g++-M` and shellcheck from Ubuntu's archive.
  apt.llvm.org and the PPA are not used.
- The "highest gcc-NN on PATH" assertion goes. Bootstrap takes the GCC major explicitly.
- macOS installs `gcc@M` (or `gcc` when its major is M) only when `g++-M` is missing.
- `CERASTES_CI_PROFILE`, `CERASTES_CLANG_MAJOR`, `CERASTES_GCC_MAJOR` and `CERASTES_BUILD_COMPILER` join the env ban.
  The scripts set the last three from the profile.

**`scripts/bootstrap.sh`** takes three optional inputs. Unset, it behaves exactly as today:
- `CERASTES_CLANG_MAJOR`: on Linux it replaces `LINUX_CLANG_MAJOR=21` as the default; on macOS it asserts Apple
  clang's major.
- `CERASTES_GCC_MAJOR`: the native-ok GCC (today: the highest).
- `CERASTES_BUILD_COMPILER` (`clang`|`gcc`, Linux): with `gcc`, EDG's preset build and cerastes's CMake get
  `gcc-M`/`g++-M` through the existing `EDG_COMPILERS` path (bootstrap.sh:223). The scrape still uses clang-N/libc++
  (EDG emulates clang; decision 0002 is unchanged).

All three are written to `build/env.sh` and the build stamps, so a change rebuilds.

**Exclusions in `scripts/run_tests.py`.** When `CERASTES_CI_PROFILE` names a profile with exclusions, each excluded test
still runs and must fail; it is reported as `diverges (profile P: reason)`. A test that passes fails the run with
"remove the exclusion", and an exclusion naming a missing test fails it too. Unset (the owner's Mac, both references),
nothing changes.

**The guard (delegated to `test-mutator`, reviewed by me).** It replaces `CI_SH_CANONICAL`, `CI_LINUX_SH_CANONICAL`
and `CI_YML_CANONICAL` (test_run_tests.py 4089–4982, `CiGuardTests` 4985–5577) with rules.

Kept rules:
- `set -euo pipefail`; `||` only as `|| want`; no `|| true`, `exit`, `return`, `&`, `trap` or `alias`.
- Bootstrap runs once; the env-ban loop holds at least the pinned set.
- The required assertion checks are present, in order. These are pinned as patterns, not values.
- In ci.yml: the top-level keys, `on`, permissions and concurrency are exact; the job keys come from the allowed set;
  `fail-fast: false`; the matrix expression is exact; no `continue-on-error`; SHAs are 40-hex; the checkout settings and
  the `if:` expressions are pinned; run lines are exact.

New rules:
- `ci_profile.py check` runs in the self-test.
- The two references' declared facts are pinned in the self-test (`macos-27`: 27/21/27/16; `ubuntu-26.04`:
  26.04/2.43/21/16) and their cadence is `every`.
- References carry no exclusions.

Mutation tables are rebuilt with at least as many cases as today (≥150 workflow, ≥200 script, ≥35 Linux). The
`RealDocTreeTests` and `DocumentationTests` hooks and `tests/README.md` are updated.

**Linux rehearsals** run on the x86-64 box (clang4.cpp.al): `ubuntu:26.04` and `ubuntu:24.04` containers, a clean build
each time, run in the background.
- Each runs `scripts/ci-linux.sh <profile>` and then `scripts/check.sh`, for the reference first and then the clang20
  and clang22 candidates.
- Green: the profile's cadence becomes `nightly`.
- Divergence: fix it if it is small. Otherwise record an exclusion with a reason, or leave the profile at `dispatch`
  with the failures listed in the handoff.

**Docs:**
- `testing.adoc` (developer guide) gets "Adding a toolchain profile": the schema, cadence, dispatching a candidate,
  exclusions.
- `tests/README.md`.
- Record 0053 marked accepted.

## S8c — GCC builds and ubuntu-24.04 (same branch; the stop-and-ask point if the context is short)

- **GCC build:** in a `ubuntu:26.04` container on the box, build with `CERASTES_BUILD_COMPILER=gcc` and GCC 15. Fix
  what `-Wall -Wextra -Werror` finds:
  - In `src/`: behaviour-neutral fixes in the Boost style, proved by `python3 scripts/object_identity.py --before <rev>`
    (clang objects identical), with `soundness-auditor` on any `src/backend|vir|viper` diff.
  - In cerastes's EDG CMake flags (`EdgFrontEnd.cmake` `-Werror=shadow -Wconversion -pedantic`): compiler-specific
    flags where GCC's warnings differ.
  - In EDG's own sources: small, upstreamable fork commits with tests (invariant 5). **Stop and ask if EDG needs more
    than a few.**
- **ubuntu-24.04:** expected divergences are glibc 2.39's `<assert.h>` shapes (the encoder matches glibc 2.43's exactly,
  `encode.cpp:12494–12560`), `shellcheck` 0.9.0 vs 0.11.0, and CMake 3.31.
  - A new assert shape is an encoding rule: it needs must-verify and must-fail tests (invariant 2), a 0053 item, and
    the soundness audit.
  - If it is more than a shape addition, use an exclusion or `dispatch` and ask.
- `macos-26` stays `dispatch` until the owner's first dispatched run (it can't be rehearsed on the macOS 27 Mac).
- **Out of scope** (M8 entries): `-arm` Linux runners (plain `char`/`wchar_t` are unsigned there, so they need encoder
  target facts), macos-15 (Apple clang 17 emulation), apt.llvm.org/PPA toolchains, and GCC builds on macOS.

## Critical files

- `scripts/ci.sh`, `scripts/ci-linux.sh`, `.github/workflows/ci.yml`, `scripts/bootstrap.sh` (36–64, 100–110, 215–227,
  370–452, 469–479)
- `scripts/run_tests.py` (the native-ok setup 1947–2131; exclusions)
- `scripts/test_run_tests.py` (4089–5608, 15362)
- New: `scripts/ci_toolchains.toml`, `scripts/ci_profile.py`, `internal/decisions/0053-ci-toolchain-profiles.md`
- Docs: `internal/decisions/0001`, `0016`, `0031`; `tests/README.md`; `doc/modules/ROOT/pages/dev/testing.adoc`;
  `internal/status.md`; `internal/handoff.md`; plan.md's Done row (S8)
- Possibly (S8c): `src/**` warning fixes, `EdgFrontEnd.cmake`, `src/backend/encode.cpp` (a glibc 2.39 assert shape)

## Verification

- **Per branch:**
  - `python3 scripts/test_run_tests.py` and `shellcheck scripts/*.sh`.
  - `python3 -I scripts/ci_profile.py check`, and `matrix` for push/schedule/dispatch with the expected sets.
  - `scripts/format.sh --check` and `scripts/build_docs.sh`.
- **Mac:** a `scripts/ci.sh macos-27` rehearsal (CLT 27.0 passes and nothing is installed); at the end, the full
  `scripts/check.sh` is green.
- **Box:** `ci-linux.sh <profile>` + `check.sh` for each Linux profile that is promoted, plus the GCC-built profile.
- **After the owner pushes:** a `workflow_dispatch` with `profiles=all` once. macos-26's result decides its cadence, and
  a differing result is drift that goes to the owner (0016 item 10).
- **Commits:** on the session branches, no Claude trailer; the owner pushes.
