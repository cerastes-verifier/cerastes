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
`git show 9d98fe8:internal/status.md`.

## M4.5d2a — the heap (2026-11-03/04): done on branch `m4.5d2a-heap`, the final check left to CI

Plan: `.claude/plans/m4-5d2a-heap.md`. Read `internal/handoff.md` next: §2 (owner steps: PR #27 first, then push vlibc's
`main`, then this branch's PR, and CI decides, below), §3 (M4.5d2v's and M4.5d2b's notes), §8 (questions 120–127).

**Owner decisions:** at planning, 120 (d2 split: d2a the heap, `realloc` to M4.5e2, loops the first cut), 121 (vlibc holds
complete implementations of what it covers, verified as cerastes becomes able to; the allocator replaces the platform's,
amending 0030 item 2), 122 (that allocator is M4.5d2v, next), 123 (blocks held when `main` returns are leaks). During the
session: 124 (`reject_owning_function.c` expects `lifetime @ L8`, not the plan's `permission`), 125–127 (new pins re-expected
to what they can reach: a template, a lambda, two member functions with two lines each, the discarded `malloc` with EDG's two
warnings). The owner ended the session at 08:00 (travel): the second `check.sh` and the Linux rerun are CI's (below).

**Done**
- **Record 0047** (written before code; audit round 1 on the draft: B1 leaks missed in loops, B2 view calls skipping
  0046 item 8's redirection check, B3 block sites invisible to sequencing, S1–S8, N1–N14, all taken; P72 PASS on macOS and
  glibc; S36 PASS, 64 fixtures; audit round 2 on the code: no blocking item, S1–S5 taken, notes to §3; a re-audit of the
  loops and round 2's fixes, `build/m4.5d2a/audit3.md`: no wrong verdict; **one blocking item not fixed (B1: exit 3 on
  correct programs whose loop heap places may alias a block held another way, e.g. `int *x = &s->b;` in a loop, then
  `free(s)`), S1 a timeout regression (parameter copies as heap places), S2 an unpinned exemption, S3 the `WF` row's
  category** — the owner ended the session first; they are the next session's first work, handoff §2 and §3).
- **vlibc** `35e9b6f` (local `main`, NOT YET PUSHED; three commits on `5962865`: the malloc family's trusted contracts with
  reserved identifiers, run tests, `CERASTES_COMMIT` → `f59e4b2`, `CLAUDE.md` with the owner's directive; the note below
  `_Exit` so earlier lines stay; `<stddef.h>` for `size_t`/`NULL`). vlibc's `check.sh` green on macOS (both compilers, 24).
- **Encoder:** registration `owning`/`ref_to_uninit` `r|p`; `cerastes_owns_block`, `cerastes_zeroed`, `cerastes::owns_block`;
  library markers; the (A)/(D) shape recognizer and the diversion; typed views at `void*` → `T*` conversions (size a constant
  equal to `sizeof(T)`, arguments kept for their checks, the call's trust and redirection checks), `free`'s `T*` argument;
  `owns_block` in contracts (consume, give, borrow; framing; rejections); target `r`; `--strict-init` rejecting
  `ref_to_uninit` results. **VIR:** `AtomKind::OwnsBlock`, `contract_block_atoms`, `result_init`, `deallocates`, backstops.
  **Lowering:** heap programs (`field tok`), views, block sites in contracts, `__wf`, calls (`invalid-free` rows, separation
  first, token before offset), returns, entry, the leak `forperm` after `l_end` and at back edges, sequencing over block
  sites and liveness, loops over blocks held through locals (record item 10, landed), the S3 framing order.
- **Categories** `invalid-free` and `leak` (twenty) in the five copies. **Lint:** `lint_heap_token`, the views' text pinned
  (`lint_views`, with programs with globals). **`vlibc_contracts.py`:** reserved identifiers, the two shapes.
- **Tests:** about 130 new files (heap positives and twins, pins, ports `account.c`, `deposit.c`, `swap_main.c`,
  `globals_heap.c`, `malloc_block_free_local.c`, vlibc tests), `selftest_cases17.inc` (47 lowering, 19 rejection cases;
  `VIR_CASES_MIN` 1540), 64 S36 fixtures.
- **Docs:** Memory and ownership ("Allocated memory", the not-yet list), Annotations (`owns_block`, target `r`), Error messages
  (`invalid-free`, `leak`), Verifying (vlibc's allocation functions); `encoding.md`; il-inventory P72, S36; record 0011's
  table and 0031 (null is all bits zero).

**Flips and expectation changes:** `reject_ref_to_uninit_function.c` → `unsupported_annotation @ L7` (plan);
`reject_owning_function.c` → `lifetime @ L8` (q124); new pins re-expected (q125–127); every heap test re-registered once
after audit round 1 before its first run (`// Pre-registered … re-registered after audit round 1`); Pins comments only in
fifteen existing pins (M4.5d2b, M4.7/M8).

**Acceptance** (at `ad691eb` and the fix after it)
- **Viper against the baseline** (`9d98fe8`; 9540 passed, 0 failed): every existing kept program's `.vpr`/`.map`
  byte-identical (the il-analyst's regeneration of all 3667 programs: 7334 files identical, 0 different, 0 missing, after
  vlibc's lines were kept); the VIR self-test's 2986 old files byte-identical, 1540 cases 0 problems.
- **Full runs:** at `250a2af` 9903 passed, 19 failed (all since fixed or re-expected); `check.sh` at `e645a20` stopped in
  the harness self-test (six failures: doc markup and the guard-skipped fixture, fixed in `ad691eb`); after that the harness
  self-test 1108 OK, walker-oracle, lint (432), mutate and vlibc-contracts self-tests OK, `check_doc_examples` (and
  `--html`) 0 problems. The last full run (started after `ad691eb`) was stopped at the session's end: its printed part (1634 passing runs, through `tests/memory/init_*`) failed only `heap_block_then_load.c`'s native compile (fixed in `889c974`).
- **Linux** (`matt@clang4.cpp.al`, container `cerastes-m4.5d1-p71`, glibc 2.43, clang 21 over libc++ 21, gcc-16; clean
  bootstrap at `e645a20`/vlibc `0f5de92`, then `ad691eb`/vlibc `35e9b6f`): `tests/vlibc` 134 passed, 0 failed (the 8
  `platform: linux` tests ran); `check_vlibc_headers.sh` clean; `vlibc_contracts.py check` clean; the doc examples 32 passed;
  vlibc's own `check.sh` 24/24 with gcc-16 and clang-21; the heap group 450 passed, 7 failed: `heap_block_then_load.c`'s
  native compile (fixed in `889c974`), `reject_malloc_discarded.c` (glibc gives no result-use warning: split into a
  `platform: macos` pin and a `platform: linux` sibling `reject_malloc_discarded_glibc.c`, the same rejection at L13,
  untested on Linux), and **`reject_owning_system_redeclaration.c [c23]`, not fixed**: on glibc C23 `strtol` carries the
  asm label `__isoc23_strtol`, so record 0046 item 8's asm-label rejection (`unsupported_declaration @ L9`) comes before
  the pinned marker rejection (`unsupported_annotation @ L9`); a pre-existing pin (M4.5b2b) that d1's Linux run of
  `tests/memory` (at `45f655b`) passed, so likely changed by d1's late asm-label rule (`26e4deb`) — CI's Linux full run
  will show it red (handoff §2).
- **Not run (owner, 08:00):** a second `check.sh` (macOS) on the final tip: CI's two jobs on the PR are the check (§2).

**Surprises**
- A double free would have been `lifetime` (the argument's liveness use check first): the `invalid-free` rows replace it.
- Viper is affine: a `return` inside a loop sees only the invariant, so the leak row went after `l_end` (audit B1).
- vlibc's header comment shifted the `abort`/`exit` lines and 66 `.map` files; the comment moved below `_Exit`.
- EDG folds `(void*)&g` into one address constant: the encoder reads its original expression.

**Decision records added:** 0047-the-heap (amends 0011, 0028, 0029, 0030, 0031, 0038, 0039, 0042, 0046).

**First step for the next session.** Check CI on this branch's PR (both jobs): red → fix first (handoff §2). Then, before
M4.5d2v, fix the re-audit's B1 and S1 in `src/vir/lower.cpp` (and S2, S3; `build/m4.5d2a/audit3.md`, handoff §3 "M4.5d2a
follow-up"), on this branch or a follow-up branch, with the five pins; then M4.5d2v (vlibc's allocator, handoff §3 and §4.2).

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
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
