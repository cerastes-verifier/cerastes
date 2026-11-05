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
`git show 9d98fe8:internal/status.md`, M4.5d2a's `git show bb41d55:internal/status.md`, and S6's `git show 9255b93:internal/status.md`.

## M4.5d2v — vlibc's allocator, with the M4.5d2a re-audit's B1–S3 (2026-11-04/05): done on branch `m4.5d2v-allocator`

Plan: `.claude/plans/m4-5d2v-allocator.md`. Record 0050; 0047 amended (items 6, 10). Read `internal/handoff.md` next: §2 (owner
steps: vlibc is pushed; vlibm, then this branch's PR), §3, §8 (question 146, the cerastes code-style session).

**Owner decisions:** at planning (2026-11-04, each the recommendation): 129, 130, 134–142 (handoff §8). During the session:
143 (two new Part 1 twins redesigned to one cause after their first run), 144 (the free-and-reallocate exit 3, older than
d2v, fixed here, not pinned), 145 (its correct program pinned as a false alarm with the encoder's errors), and the code style
of vlibc and vlibm: the owner's Boost libraries' (Allman braces, braces on every control statement, 4 spaces, `T* p`;
cerastes's own refactor is a session of its own, question 146).

**Done**
- **Part 1, the M4.5d2a re-audit** (0047 item 10 amended): B1's catch-all splits, `block_of` splits between token places and
  splits against outside holders (needed, S37 (c)); S1's skip of unassigned parameter copies with an atom (t28: timeout →
  31 s); S3's automatic `internal` `flag ==> WF(v)` invariant and the free row's `WF` `internal` again
  (`heap_loop_swap_false_alarm.c` lost its two `permission` lines, pre-approved); S2 pinned. Suite after Part 1: 10291
  passed, 0 failed; only the 18 heap-loop programs' Viper changed.
- **vlibc's half** (`~/Documents/cerastes/vlibc`, local `main`, `bba61ae..6e41fbe`, pushed by the owner 2026-11-05): `vlibc_malloc`,
  `vlibc_calloc`, `vlibc_realloc`, `vlibc_free`, `vlibc_aligned_alloc`, `vlibc_free_sized`, `vlibc_free_aligned_sized` over a
  portable C23 core (size classes in 1 MiB regions, large blocks in their own mappings) and trusted POSIX glue (one mutex,
  `mmap`, errno); `libvlibc` static; run, stress and threaded tests (80 per configuration) green under ASan/UBSan and TSan
  on macOS (gcc, clang, gcc-asan, clang-asan, clang-tsan) and on the x86-64 box (all six); CI matrix configurations;
  `CERASTES_COMMIT` → `9255b93`; restyled to the owner's Boost style; `<stdlib.h>` declares the seven beside the standard
  names (three with contracts token for token, four contract-less), pointers on the type. vlibm: its `CLAUDE.md` gains the
  code style (`1e6982e`, NOT pushed).
- **cerastes's half** (record 0050): the pairing check (`vlibc_contracts.py`: token for token, inside `extern "C"`, after the
  standard name; conditionals other than the guard and `#ifdef __cplusplus` refused; a third field in `vlibc_names.inc`);
  a block's allocator as its token's value (`free(vlibc_malloc(n))` and `vlibc_free(malloc(n))` are `invalid-free`;
  `owns_block` keeps meaning the platform's), gated on any `vlibc_` view; split token rows that each read the token;
  loop value snapshots; vlibc allocations rejected in C++ (M7); contract-less library routines skipped unless called;
  `vlibc_` reserved names for C linkage only; the pin `35e9b6f → 6e41fbe`; `trusted_contracts.txt`
  (three rows added, the six respelled by whitespace).
- **Two older exit-3 holes fixed** (questions 144, audit round 2 S4): heap places' back-edge rows end their path
  (`if (!c) { assert false }`) and their value invariants have `permission` rows; a call's pointer result is split against
  earlier pointer locals in heap programs. Their correct programs are pinned (`heap_loop_replace_false_alarm.c` and the vlibc
  twin as false alarms until d2b; `heap_pick_live.c` verifies).
- **Tests and fixtures:** about 60 new tests (Part 1, the vlibc allocator, the audits' gaps), S37 (a)–(f) (70 fixtures);
  VIR self-test 1561 (`VIR_CASES_MIN`); the lint pins `vlibc_` views from emitted fixtures, reads ending rows, ties `ktv_` to
  its guard, and reports unknown typed views (it skipped them silently).
- **Docs:** "vlibc's allocator" in Memory and ownership; Verifying's vlibc section (stale since 0048) rewritten; the
  `invalid-free` mismatch with its console line; Annotations, Building with ordinary compilers (`libvlibc`), the developer
  guide's architecture and soundness pages, the tutorial's heap chapter (a pointer); `internal/encoding.md`.

**Checks run**
- `check.sh` (full, `CERASTES_CHECK_BASE` unset), twice. Run 1 stopped in the harness self-test: one test's pattern still
  expected `void *malloc(` after vlibc's restyle (fixed). Run 2: harness self-test 1108 OK; suite 10424 passed, 0 failed,
  10 skipped (native-ok Apple clang 1941, GCC 1941); lint 4029 files clean; VIR self-test 1561 cases, 1561 passed;
  walker oracle and docs OK; its last step, the lint of the VIR self-test's Viper, found 10 problems (the `vl_` cases'
  views carried `free`'s tag text under `vlibc_free`; fixed after).
- After run 2 (the re-audit's argument-only result split, the self-test tag fix; delta checks, memory
  `verify-delta-not-full-check`): `run_tests.py tests/memory tests/vlibc tests/viper-smoke/memory doc/modules/ROOT/examples`
  5078 passed, 0 failed, 9 skipped (Linux-only); VIR self-test 1561 passed and its lint 1561 files clean; `lint_viper.py`
  over `build/vpr` 2112 clean; their Viper against the previous full run: 0 changed. The re-audit's probes: the stress
  programs verify in 2–5 s (one had run over 270 s), and every shape of the old S4 family verifies, none exits 3.
- Linux (the x86-64 box, container from `cerastes-m4.5d1:runner`, at `b0a86d0`): the same four directories 5030 passed,
  0 failed; `vlibc_contracts.py check`, its self-test, `test_lint_viper.py`, `lint_viper.py` (2091 clean) OK. vlibc
  `check.sh all` there (at `cbd7515`): gcc, clang, gcc-asan, clang-asan, gcc-tsan, clang-tsan 76/76 each.
- vlibc on macOS at `6e41fbe`: `check.sh all` 80/80 in gcc, clang, gcc-asan, clang-asan, clang-tsan.
- Not run on Linux at the final tip: cerastes's changes after `b0a86d0` and vlibc's `6e41fbe` (CI's).
- Audits: round 1 on the records (B1, S1–S5: `build/m4.5d2v/audit1.md`), round 2 on the code, the pin and the allocator
  (B1 an unframed token read, S1–S5: `audit2.md`), the re-audit of round 2's fixes (B1 the quadratic result split, S1–S4:
  `audit3.md`); every blocking and should-fix item taken. The re-audit's blocking fix (the split against the call's own
  arguments, the auditor's recommendation, checked on its probes) was not audited again (the cap).

**Surprises**
- Silicon has no freshness: a new allocation may be a freed block (a real allocator reuses addresses), so permissions a
  free consumed come back with unknown contents; an assertion on `perm(tok)` alone does not resolve which held token a
  pointer designates, and the next read of it is Silicon's own error (exit 3). Rows now read what they test.
- Two exit-3 holes older than d2v (free-and-reallocate in a loop; freeing a call's result that may be one of several
  blocks) were found by this session's tests and audits, not by d2a's. The first fix of the second (splitting a result
  against every earlier pointer local) timed out correct programs that no suite test exercised: the re-audit's stress
  probes found it; `heap_call_results_many.c` now pins the cost.
- The baseline at `9255b93` ran under load 150–240: 80 Silicon timeouts (no wrong outcome); the Part 1 run on a quiet
  machine had none. Time-sensitive runs need a quiet machine.
- `void* malloc` and `void *malloc` put the declarator at the same column, so the header restyle left every `.map` row as
  it was.

**Decision records added:** 0050-vlibc-allocator (amends 0047, 0048, 0046); 0047 amended (items 6, 10).

**First step for the next session.** The owner pushes vlibm's `main` and this branch (vlibc's `main` is pushed; handoff §2); CI decides.
Then the cerastes code-style session (question 146, if the owner agrees) before M4.5d2b's planning (handoff §4.2).

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
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
