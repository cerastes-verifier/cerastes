# Status

Session log, newest first. Each entry says what is done, what is not, any surprises, the decision records added and
the exact first step for the next session. Trimmed at the end of M4.4c (owner, 2026-10-26) to the latest entry and a
history table; the full log up to M4.4c is `git show 1fcc43b:internal/status.md`, M4.4c's entry is
`git show fce46de:internal/status.md`, M4.4d's `git show 9017105:internal/status.md`, and M4.5a's
`git show 72e30b3:internal/status.md`.

## S5 — Linux CI (2026-10-28): done on branch `s5-linux-ci`

Read `internal/handoff.md` next: §2 the owner steps (push, then a full run on GitHub's Linux runner), §3 "From S5", §8
questions 59–63.

**Owner decisions (2026-10-28):** handoff questions 53–56 as recommended (a port time of 10× is when liveness is
revisited); **57:** `ubuntu-26.04`, else the newest toolchains in that image; **58:** the box may be used for any
testing. During the session: **the eight tests that name what only the macOS SDK declares** get `; platform: macos`.

**Done**
- **Record 0031** (amends 0001, 0008 item 2, 0015 item 6, 0016 item 13): a second job, `check-linux`, on GitHub's
  `ubuntu-26.04` x86-64 image, in both tiers, `CERASTES_JOBS: 4`; `scripts/ci-linux.sh` asserts Ubuntu 26.04 on
  x86-64, glibc 2.43, Ubuntu's clang 21.1.8 and libc++ 21 (`1:21.1.8-6ubuntu1`), GCC 16 (`16-20260322-1ubuntu1`, the
  snapshot 26.04 packages), the image's JDK 17, EDG's emulated clang 210108 and libc++ include directories, and
  installs what is missing from Ubuntu's archive.
- **`bootstrap.sh` on Linux:** EDG's `linux-clang-debug` with clang-21, ASan and EDG's runtime library off (as macOS's
  stock build); the scrape through clang++-21 with `-stdlib=libc++`; Z3's x64 glibc asset and the sbt 2.0.10 launcher
  by SHA-256; `is_gcc` for Ubuntu's banner; `sha256_of`/`fetch_pinned`; bash 3.2-safe empty arrays.
- **Code:** glibc's two `assert` expansions (`encode.cpp`, the C23 comma's constant must be the folded `sizeof`);
  `posix_spawn_file_actions_addchdir_np` off Apple; a `$<LINK_GROUP:RESCAN,…>` for GNU ld; five Embedded C
  extensions off in `edg_config` (both front ends compile the same 386 options); in a system file, an `#undef` of a
  predefined macro and its exact redefinition in the same inclusion are no change (glibc's `__INT64_C`; by path in
  verify mode).
- **Harness:** `CLANG_FAMILY` (`clang` on Linux), GCC banners by the FSF line, `; platform: macos|linux` on a
  `mode:`/`c-only:` line; `CiGuardTests` pins both jobs and `ci-linux.sh` (`ci_sh_problems` with a spec); new
  `PlatformTagTests`, `SystemHeaderMacroTests` (11 cases) and banner cases; the test
  `tests/annotations/macros/system_header_redefinition.cpp`.
- **Docs:** Installation gains a Linux section; Overview, Integer semantics, `README.md`, `tests/README.md`.

**Acceptance**
- The rehearsal (Docker on the Mac, `ubuntu:26.04` amd64, Rosetta, 4 CPUs; the box was not answering then): `ci-linux.sh`
  green; `check.sh` in the PR tier against `72e30b3`: every step green after the fixes, the tests 3999 passed and 12
  failed before the platform tag, then the affected areas rerun with it (179 passed, 8 skipped); walker oracle 0 of
  1924 facts failed; VIR self-test 127 of 1123 cases run (the rest left to the full check); docs green.
- Against the Mac's full run of the same tree (5763 passed): every outcome the Linux run made agreed but those 12; of
  653 `.vpr` and map files, 17 differ, all benign (record 0031 item 6).
- The final `check.sh` on the Mac (the full check, `05b2b8b`): **all green** (harness self-test 1017 tests, 1 skip;
  tests 5766 passed, 0 failed, 1 skipped; VIR self-test 1123; walker oracle and docs green). The first run that
  session stopped in the self-test: `SystemHeaderMacroTests` had been inserted under `ChecksSelftestTests`'s
  `@e2e_gate`, which `E2EGateTests` caught; fixed before the second.
- **The full check on Linux**, natively on the box once it answered (`05b2b8b`, an `ubuntu:26.04` container
  `cerastes-s5`, `CERASTES_JOBS=8`): `ci-linux.sh` green with the rehearsal's toolchain report; `check.sh` **all
  green** in 79 min (harness self-test 1017 tests, 2 skips; tests 5752 passed, 0 failed, 9 skipped; native-ok runs
  clang 1078, GCC 1078; VIR self-test 1123; walker oracle 0 of 1924 facts failed; docs green). Against the Mac's
  final run: every outcome the same but the 8 tagged tests (14 runs there, 8 named skips here); of 4057 `.vpr` and
  map files, 71 differ, all benign (`MANIFEST`; 18 `.vpr` with system parameter names; 52 maps with system paths).
  The self-test's second skip is the built-site check, which skips until `check.sh`'s docs step has built
  `build/doc` (it passes on Linux once the site exists); a fresh runner skips it on both platforms.
- **Not yet seen:** the job on GitHub's real `ubuntu-26.04` runner (its image, its 4 cores and the 220-minute limit).

**Surprises**
- GNU ld's one pass over static archives (`back_end()` undefined); Ubuntu's GCC banner has no "GCC"; glibc's
  `<stdint.h>` undefines and redefines predefined macros, which 0008's rule refused; EDG's stock Linux configuration
  compiles in Embedded C (7 more options); its runtime library does not build with clang 21 (`bf16` literals); GNU tar
  fails under Rosetta.
- Two Opus audits each found blocking holes (an unencoded overflow beside glibc's `sizeof`; a macro window that let a
  user file verify, and pairs across inclusions); all fixed and re-audited.

**Decision records added:** 0031-linux-ci.

**First step for the next session (M4.5b1).** Confirm that the owner pushed this branch and that both CI jobs are green
on it (a full run by dispatch, with the Linux job's time: handoff §8 question 63), or take the Linux job's evidence
back to a short S5 follow-up; then handoff §4.4, "M4.5b1".

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
| S5 Linux CI | 2026-10-28 | `s5-linux-ci` | 0031 | `check-linux` on `ubuntu-26.04` in both tiers; Linux bootstrap; glibc `assert`; the same EDG language on both platforms; eight macOS-only tests tagged |
| M4.5a Memory-model design, `\|`/`^` masks | 2026-10-27 | `m4.5a-memory-design` | 0027–0030 | the C memory model designed and audited (E2, four axioms, member blocks, liveness), ownership annotations, vlibc; `\|`/`^` constant masks decided; S5 Linux CI next |
| M4.4d Exact sets, constant masks | 2026-10-27 | `m4.4d-exact-sets` | 0026 | preconditions and comma values get exact sets, with today's as the fallback; one `requires` per argument accepted; constant masks as remainders, the `bit_mask.c [c23]` hang gone; hooks `force_base_sets`, `drop_exact_set` |
| M4.4c Writes outside the chain, `void` `?:`, `-Wall` shapes | 2026-10-26 | `m4.4c-writes-void-warnings` | 0025 | finding A fixed (a flagged read is an access); writes and snapshots leave the chain; a `void` `?:` statement is an `if`; warnings in `verified`/`error` tests; the static-assertion cross-check in bodies; 13 `-Wall` shape rules; M4.4e planned |
| M4.4b2 Values the front end computes | 2026-10-26 | `m4.4b2-front-end-values`, `8bff6ba` | 0024 | static assertions checked as written (file scope through source sequence lists, now on); wrapped character literals are `conversion` errors; types computed from a constant rejected in `sizeof`/`typeof`/typedefs until M4.5e |
| M4.4b1 Bitwise operators | 2026-10-23/24 | `m4.4b1-bitwise`, PR #9 `effc63f` | 0023 | `& \| ^ ~` and `&= \|= ^=` exact (prelude functions per width with proved range postconditions); seven bitwise `-Wall` shapes rejected; multi-character literals refused; item 5's truth values one verdict in both modes |
| M4.4a No wrapping | 2026-10-23 | `m4.4a-no-wrapping`, PR #8 `00f60d2` | 0022 | unsigned arithmetic and conversions checked (`overflow`, `conversion`, `shift`), `c_wrap` gone; lowering's internal errors exit 3; enumeration `==`/`!=` with possibly negative operands rejected |
| M4.3b Calls in expressions | 2026-10-23 | `m4.3b-calls`, PR #7 `58d208c` | 0021 | calls in guarded operands and loop conditions; the dependency-chain rule (`unsequenced_calls`); explicit termination checks; `self_assignment` |
| M4.3a Side effects in expressions | 2026-10-22 | `m4.3a-side-effects`, PR #6 `d694606` | 0020 | assignments, `++`/`--` and commas as values; C's sequencing rule (`unsequenced_modification`); the first `-Wall` shapes |
| M4.2c Locals without an initializer | 2026-10-22 | `m4.2c-uninitialized-locals`, PR #5 `1c1cc29` | 0019 | category `uninitialized` by a ghost flag; jumps past such declarations; EDG #2656 fixed on fork branch `0022` (pin `f0ed5525af`); `-Wswitch`'s rules |
| M4.2b2 Enumerations | 2026-10-22 | `m4.2b2-enumerations`, PR #4 `4622259` | 0018 | enumeration types with value ranges, every conversion into one checked |
| M4.2b1 Named constants and case values | 2026-10-21 | `m4.2b1-named-constants`, PR #3 `ae2d2a4` | 0017 | `const`/`constexpr` globals and case values checked in each method's prologue |
| S3 Mutation triage | 2026-10-21 | `mutation-triage`, PR #2 `915e372` | — | 17 tests strengthened, 33 twins, 436 reviewed equivalents |
| S2 CI | 2026-10-21 | `ci`, PR #1 `e1f2860` | 0016 | `check.sh` on GitHub's `xcode-27` runner, required on `main` |
| S1 EDG upstream merge | 2026-10-21 | `edg-upstream-merge` | — (0001) | fork `cerastes` merged upstream `main`; 0020's Linux failure was stale box binaries |
| M4.2a Jumps and ends | 2026-10-21 | `m4.2a-goto-noreturn-assert` | 0015 | outward-and-forward `goto`, `noreturn` (category), trusted `abort`/`exit`/`_Exit`, `assert` as `check` |
| M4.1 Control flow | 2026-10-20 | `m4.1-control-flow`, `29a3dd3` | 0014 | `break`, `continue`, `do`, `switch`; the plan re-sequenced (new M4) |
| M3 close-out | 2026-10-20 | `worktree-m3-closeout`, `bfeda03` | — | M3 acceptance, mutation run |
| M2 re-look and M3 Scalar encoder | 2026-10-16/19 | — | 0010–0013 | VIR, emitter, driver, scalar encoding, termination |
| EDG bugs 0003–0007 | 2026-10-15 | fork branches | — | fixed, branches for the owner's PRs |
| M2 Annotation layer | 2026-10-14/15 | — | 0005–0009 | `[[cerastes::…]]` registration in the EDG fork, `<cerastes.h>`, the fork pin |
| M1.5 Documentation site | 2026-10-14 | — | 0004 | Antora docs at cerastes.dev/cerastes/doc/ |
| M1 IL inventory · Gate G1 | 2026-10-13 | — | 0002, 0003 | G1 PASS; in-process back end |
| M0 Bootstrap | 2026-10-13 | — | 0001 | toolchain pins, harness, `check.sh` |
