# Status

Session log, newest first. Each entry says what is done, what is not, any surprises, the decision records added and
the exact first step for the next session. Trimmed at the end of M4.4c (owner, 2026-10-26) to the latest entry and a
history table; the full log up to M4.4c is `git show 1fcc43b:internal/status.md`, M4.4c's entry is
`git show fce46de:internal/status.md`, M4.4d's `git show 9017105:internal/status.md`, M4.5a's
`git show 72e30b3:internal/status.md`, S5's `git show 794e960:internal/status.md`, M4.5b1a's
`git show bdc6bd0:internal/status.md`, M4.5b1b's `git show 8003dca:internal/status.md`, M4.5b1c1's
`git show eab4058:internal/status.md`, M4.5b1c2's `git show a5e3e30:internal/status.md`, M4.5b1c3's
`git show 5ffb1a2:internal/status.md`, M4.5b2a's `git show 38877b4:internal/status.md`, M4.5b2b's
`git show 1bfa705:internal/status.md`, M4.5c1's `git show cbdcce5:internal/status.md`, and M4.5c2a's
`git show 7f11706:internal/status.md`, M4.5c2b's `git show d26a87c:internal/status.md`, and M4.5d1's
`git show 9d98fe8:internal/status.md`, M4.5d2a's `git show bb41d55:internal/status.md`, S6's `git show 9255b93:internal/status.md`, M4.5d2v's `git show c2bc48d:internal/status.md`, S7's `git show a660abc:internal/status.md`, and M4.5d2b1's
`git show c1b691d:internal/status.md`.

## S8 — CI toolchain profiles (2026-11-06): done on branch `s8b-ci-profiles` (S8a's two commits first)

Plan: `.claude/plans/s8-ci-toolchains.md`. Record 0053 (amends 0001, 0016 items 2, 3, 9, 10, 0031 items 1, 2). Read
`internal/handoff.md` next: §2 (owner steps), §3 "From S8", §8 (questions 169–172).

**Why:** GitHub's `xcode-27` image `20261104.0337` (a pre-release, still deploying) moved the Command Line Tools from
27.0.0.0.1788430756 to 27.2.0.0.1792721180, and runs got either image, so CI's exact pins failed both ways (runs
42158664485 and 42232697813). `c1b691d` moved `ci.sh` without the self-test's copy, so `main` also failed the harness
self-test. The owner: "These scripts should be far more robust and allow different versions of clang and gcc on linux to
be tested in the future. Like using default macos-16, 26, ubuntu-26.04, ubuntu-24.04 etc." (there is no `macos-16`).

**Owner decisions** (2026-11-06, at planning, each the recommendation but 168): 162 (assert declared majors, record the
exact builds), 163 (a different outcome fails the job; fix it or record a named exclusion; no per-toolchain `expect:`),
164 (the references in both tiers, other toolchains nightly and on dispatch), 165 (the guard as structural rules and a
profile schema), 166 (a small fix first, the profiles stacked), 167 (other ubuntu-26.04 majors, ubuntu-24.04, macos-26),
168 (GCC builds of cerastes and EDG in this work; not the recommendation, which was a later session). During the session
the owner pushed `s8b-ci-profiles` at `3fcbf92` (PR #33) instead of S8a alone.

**Done**
- `bdefc29` planning and record 0053; `d0e318a` S8a: both setup scripts assert majors (CLT 27, Apple clang 21, SDK 27,
  GCC 16; clang 21 with libc++ 21 at one package version, GCC 16, glibc 2.43 exactly) and record the builds; the guard's
  copies followed.
- `15357b6` S8b part 1: `scripts/ci_toolchains.toml` and `scripts/ci_profile.py` (schema, selection, `get`, `matrix`,
  `exclusions`); `ci.yml` with a `profiles` job and two matrix jobs, a `profiles` dispatch input; `ci.sh`/`ci-linux.sh`
  take the profile; `bootstrap.sh`'s `CERASTES_CLANG_MAJOR`, `CERASTES_GCC_MAJOR`, `CERASTES_BUILD_COMPILER`
  (`build/toolchain.env` for `--build-only`, a compilers stamp); `run_tests.py`'s exclusions (`DIVERGES`).
- `7b69beb` S8b part 2: the guard as rules (a pinned assertion order, a line-shape allow-list, ci.yml's pattern tree,
  `CI_REFERENCE_PROFILES`, 112 schema mutations; test-mutator agent, reviewed); the developer guide's CI section and
  "Adding a toolchain"; `tests/README.md`.
- `3fcbf92`, `7dd76f5`, `fc499cc` and the final commit, S8c: clang 18's bit-field narrowing (`encode.cpp`, objects
  identical); `CERASTES_WARNINGS` with GCC's `-Wdangling-pointer=1`; EDG's clang base named for GCC builds;
  `ubuntu-24.04` on clang 20, `ubuntu-24.04-clang18` at dispatch; the builder's evaluation-order bug (`vir.cpp`, see
  Surprises) with a new pair for a compound store used as a value (`tests/memory/compound_store_value.c`,
  `negative/compound_store_value_div_zero.c`; no test had one); ShellCheck 0.9.0's SC2015 in `check_edg_options.sh`;
  the exclusions; profiles promoted to `nightly` (below).
- Soundness audits (read-only Opus): the S8c build diff (no blocking item; its should-fix, `-Wdangling-pointer=1` over
  turning the warning off, taken) and the builder fix (no blocking item; clang-tidy's `bugprone-use-after-move` over all
  29 TUs finds only the two fixed lines; an empty file can change only a message; its should-fixes: the compound-store
  pair, taken, and a position map that refuses an empty file, to the handoff §3 "From S8").

**Checks**
- The harness self-test: 1159 OK (1 skip) after the guard; the CI classes after each later edit.
- `check.sh` on the Mac (the full check, run once at `3fcbf92`'s tree plus the 24.04 profile change): all green, the
  suite 11072 passed, 0 failed, 10 skipped (native-ok Apple clang 2085, GCC 2085), the VIR self-test 1642. The
  later edits are covered by targeted checks: `check_edg_options.sh` and ShellCheck; the CI classes; `vir.cpp`'s fix by
  1262 byte-identical `.vpr`/`.map` runs under Apple clang 21.
- `scripts/ci.sh macos-27` rehearsed on the Mac (CLT 27.0): passes, installs nothing.
- GitHub, PR #33 at `3fcbf92`, PR tier: `profiles` selected the two references; `check (macos-27)` ran on the new image
  (`20261104.0337.1`: CLT 27.2.0.0.1792721180, banner `clang-2100.4.1.1`, SDK 27.2 build `26B118`, all three of which the
  old pins refused) and is green (7280 passed, VIR 183, PR tier).
- The x86-64 box (`~/cerastes-s8`, one container per profile from a clean `ubuntu` image, `ci-linux.sh <profile>` then
  the full `check.sh` at `-j 16`): `ubuntu-26.04` (the reference) and `ubuntu-26.04-clang22-gcc14` all green (the suite
  11072 passed, 0 failed, 10 skipped, native-ok clang 2087, GCC 2087; VIR self-test 1642; both at `3fcbf92`);
  `ubuntu-26.04-clang20-gcc15` and `ubuntu-26.04-gcc15-build` all green at `32c4ee9` (11080 passed, 0 failed, 10
  skipped, the clang-20 one with 1 diverging as it records; native-ok clang 2089, GCC 2089; VIR 1642);
  `ubuntu-24.04` (clang 20) 11068 passed and 4 failed before its exclusions, then all green at `51b9861` (11080 passed,
  4 diverging as it records); `ubuntu-24.04-clang18` (the suite only) 11064 passed, 8 failed (record 0053 item 11).
  The four green profiles besides the reference are `nightly`; `ubuntu-24.04-clang18` and `macos-26` stay `dispatch`.
- GitHub at `fc499cc`: both jobs failed five CI profile self-tests (their fixture profile had gained an exclusion;
  fixed in `32c4ee9`, the CI classes and the full harness self-test, 1159 OK, rerun after it).

**Not done:** macos-26 never ran (it cannot be rehearsed on a macOS 27 Mac: the owner dispatches it, question 172);
`ubuntu-24.04-clang18` stays at `dispatch` (question 169); vlibc's linkage fix for libc++ 20 (question 170); the GitHub
Linux jobs of the new workflow on the final tip (CI decides when the owner pushes).

**Surprises:** the Apple clang build moved too (`2100.3.34.2` → `2100.4.1.1`) with the CLT 27.2 image, which the release
notes did not list. EDG's CMake picks `EDG_BASE` (the emulated compiler's predefined macros) from the compiler that
builds it. GCC 15 found a real bug clang's argument order hides: `Builder::compound_assign_expr` and
`compound_store_expr` moved their `SrcLoc` in the same argument list that copied it, so errors of `x /= d` used as a
value lost their file in a GCC build. EDG's clang-18 emulation is another language for eight tests (record 0053
item 11). A `find -name "._*"` cleanup on the box matched EDG test directories named `._…` (nothing tracked was lost).

**First step for the next session:** M4.5d2b2's planning (plan.md's row; handoff §3 "M4.5d2b2" and "From M4.5d2b1"),
after the owner merges PR #33 and dispatches the workflow once with `profiles=all`.

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
| S8 CI toolchain profiles | 2026-11-06 | `s8b-ci-profiles`, PR #33 | 0053 (0001, 0016, 0031 amended) | CI asserts declared majors and records exact builds; toolchain profiles (`scripts/ci_toolchains.toml`), a matrix with a `profiles` job, cadences `every`/`nightly`/`dispatch`, exclusions; the guard as rules and a schema; GCC builds of cerastes and EDG; a builder evaluation-order bug found by GCC fixed |
| M4.5d2b1 Partnerless results, owning locals | 2026-11-05 | `m4.5d2b1-results-locals` | 0052 | `ensures` atoms with no partner (covered by implicit atoms or given an output amount; bodiless contracts fresh results only), owning automatic locals (leaks at every exit and reassignment, loops), marked locals and P4222R3's strict rules, block-site guards over memory, `cerastes_owns_vlibc_block`, two d2a holes closed; written `owns_block` invariants cut to d2b2 |
| S7 Code style | 2026-11-05 | `s7-code-style` | 0051 (0001 amended) | cerastes's tool code in the owner's Boost style: the pinned clang-format 23.1.3 (`format.sh`, a `check.sh` step, fail-closed guards), then brace initialization, `U` and one declarator per declaration; proved by `object_identity.py` (objects and IR at every commit, macOS and Linux; tokens) and by `.vpr`/`.map` identical over the suite and the VIR self-test; the docs lockfile fixed |
| M4.5d2v vlibc's allocator | 2026-11-04/05 | `m4.5d2v-allocator` | 0050 (0047 amended) | the M4.5d2a re-audit's B1–S3; vlibc's allocator (`vlibc_malloc` and six more, a C23 core over trusted glue, sanitized and threaded tests); blocks know their allocator (the token's value); the pairing check; contract-less library routines; vlibc allocations rejected in C++; two older exit-3 holes fixed; vlibc and vlibm in the owner's Boost style |
| S6 Tutorial and developer guide | 2026-11-04 | `s6-tutorial` | 0049 | a VerCors-style tutorial (seven chapters, 20 verified programs and 24 failing variants, both modes) and a developer guide (eight pages) as sections of the one site; the overview rewritten; the website's landing page; a chapter per concept from now on (questions 131–133) |
| M4.5d2a The heap | 2026-11-03/04 | `m4.5d2a-heap` | 0047 | vlibc's `malloc`, `calloc`, `free` (trusted, reserved identifiers, shapes not names); typed views at `void*` → `T*` (one object, `sizeof(T)`); heap blocks with the free token; `invalid-free`; leaks by `forperm` (after `l_end`, at back edges; `main` included); `owns_block`; `owning`/`ref_to_uninit` on functions; sequencing over block sites; loops over blocks held through locals; d2 split (d2v allocator, d2b) |
| M4.5d1 vlibc in cerastes | 2026-11-03 | `m4.5d1-vlibc` | 0046 | `third_party/vlibc` pinned and guarded; vlibc's shim first in verify mode; library files by path; trust from vlibc's tags, "does not return" from vlibc, reserved names, attributes by position; three routes redirecting trusted calls closed (older than vlibc); `trusted_libc_reason` gone; d2's dependency map |
| M4.5c2b Places through a heap read | 2026-11-03 | `m4.5c2b-heap-read-places`, PR #26 | 0045 | contract and invariant places through loads (`**pp`, `*gp`, `p->q->x`, `*s.q`, `*r.q`) with the entry-state rule, the post-state reading, amounts that follow objects, bodiless ties, heap-reading guards, `eb`/`lb` loop places, written atoms through loads and on struct places, `restrict` over load roots; `live(r.q)` to M4.5d2 |
| M4.5c2a Struct values and struct storage | 2026-11-02/03 | `m4.5c2a-struct-values`, PR #25 | 0044 | by-value struct parameters and results as tuples of leaves, `f().x`, C++ temporaries, compound literals (C objects, C++ values), struct globals and `static` struct locals, member measures, `restrict` beside by-value structs; invariant reads through a loaded pointer pinned; M4.5c2 split, heap-read places and written struct atoms to M4.5c2b |
| M4.5c1 Struct objects | 2026-11-02 | `m4.5c1-struct-objects`, PR #24 `8f45843` | 0043 | C structures with member blocks, struct locals as blocks, `.`/`->`, pointers to members, initializers, copies, `struct S *` objects with `WF_S`, contract atoms on struct places with overlap per leaf cell, markers per leaf; `restrict` closed beside loaded pointers (a hole since 0037); M4.5c split, owning fields and P4418R0 to M4.5d2 |
| M4.5b2b Initialization through pointers | 2026-11-02 | `m4.5b2b-init-markers` | 0042 | P4222's markers (`uninit` on locals, `ref_to_uninit`/`must_init` on pointer parameters), every call flag row `uninitialized`, `--strict-init` with the `// verify-options:` header, the loop row on every loop place (question 89, fixing 0038's nested-loop exit 3); M4.5b2 complete |
| M4.5b2a Globals and `static` locals | 2026-10-30 | `m4.5b2a-globals`, PR #22 `38877b4` | 0041 | mutable globals and `static` locals as `unique` blocks, `program_start` for `main` with initial values by linkage, frames through `owns(g)`/`reads(g)`, implicit atoms for `static` locals; M4.5b2 split, the markers to M4.5b2b |
| M4.5b1c3 Pointers to pointers, owning, `*result`, written atoms | 2026-10-30 | `m4.5b1c3-pointers-to-pointers` | 0040 | `T**` at any depth and pointer cells, `[[cerastes::owning]]` on parameters, `*result` as a value, written `owns`/`reads`/`live` in loop invariants (question 68's false alarms gone); conversions and contract comparisons across `const` rejected (a hole since 0037 closed); M4.5b1 complete |
| M4.5b1c2 Sequencing and placement | 2026-10-29 | `m4.5b1c2-sequencing` | 0039 | writes to cells inside expressions, sequencing on objects (`unsequenced`; same-object shapes rejected like the compilers), calls beside accesses, `return &local;` rejected (R14), `c ? owns : reads` path-precise and partners by path, function measures over memory |
| M4.5b1c1 Loops and jumps | 2026-10-29 | `m4.5b1c1-loops` | 0038 | loops over memory with automatic permission invariants ("every place held"), invariants and loop measures over memory, scope exits at jumps; written atoms in invariants cut to b1c3, function measures over memory to b1c2; the `address_of_local.c` port |
| M4.5b1b Pointer parameters and contracts | 2026-10-28/29 | `m4.5b1b-pointer-params` | 0037 | pointer parameters and results, `&param`; `owns`/`reads`/`live`/`old` in both spellings with ghost amounts; framing per path; `permission` at calls and returns; `pointer-comparison`; `restrict` on owned C parameters |
| M4.5b1a The model, address-taken locals | 2026-10-28 | `m4.5b1a-pointers` | 0032–0036 | the `Block` domain and its four axioms; address-taken locals as blocks, `&`, `*`, NULL, `==`, liveness and scope exits inside one function; `null-dereference`, `permission`, `lifetime`; C's incompatible pointer types as errors |
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
