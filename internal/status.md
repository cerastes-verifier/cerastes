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
`git show 9d98fe8:internal/status.md`, M4.5d2a's `git show bb41d55:internal/status.md`, S6's `git show 9255b93:internal/status.md`, M4.5d2v's `git show c2bc48d:internal/status.md`, S7's `git show a660abc:internal/status.md`, M4.5d2b1's
`git show c1b691d:internal/status.md`, S8's `git show 15bcdc6:internal/status.md`, and M4.5d2b2a's
`git show 19d977a:internal/status.md`.

## M4.5d2b2b — owning fields (2026-11-09/10): done on branch `m4.5d2b2b-owning-fields`

Plan: `.claude/plans/m4-5d2b2b-owning-fields.md`. Record 0055 (amends 0029 item 1, 0043 items 7 and 9, 0044 items 3, 4
and 8, 0052 items 3 and 8, 0054 items 2, 5, 6 and 10). Read `internal/handoff.md` next: §1, §2, §3 "M4.5d2b2c" (with
"From M4.5d2b2b") and §8 (questions 194 on).

**Owner decisions.** At planning: 188–193 (each the recommendation; the plan's "Owner decisions"). Two decisions that
the plan reserves for the owner were first taken in the session and then put to the owner before the end (2026-11-10),
who answered both with the recommendation: **194**, a covered output atom carries no closure (record item 3, "Covered
atoms"; it fixed a timeout over 170 s; its first form was unsound, caught by the re-audit and fixed): keep it; **195**,
the loop shapes left as pinned false alarms rather than rejections (record item 7): keep the pins, and M4.5d2b2c fixes
them first. The agents' own questions (the S6 caller side, the replaced-owner loops, `main_hold_type`, the two
timeouts) were settled in the session as the record says, each sound and conservative, and are covered by 194–196.

**Step 0 and Step 1.**
- The box's full `check.sh` on `b6b3298` (d2b2a): exit 0, all green (11,779 passed). `origin/main` held `49d1775`, so
  the branch is from `origin/main` (`19d977a`).
- CI (`6a7decf`): question 169's comment in `scripts/ci_toolchains.toml`. `macos-26` not promoted: the owner's
  `profiles=all` dispatch has not run (question 172; handoff §2).
- The tools (question 193): `scripts/regen_vpr.py` (every kept `.vpr`/`.map` without Silicon, through the driver's
  `silicon_timeout` hook: the whole suite in about a minute) and `scripts/vprdiff.py` (byte comparison under an
  allow-list; new tests counted, not compared), with a section in the developer guide's Testing page.
- The baseline (4,668 `.vpr`/`.map` and the VIR self-test's 1,669 cases) was taken in the session worktree from
  bootstrap's build before any change to `src/`, not in a detached base worktree: with the tools it took 34 s, and no
  agent had built yet. A frozen copy of that build (`build/m4.5d2b2b/cerastes-base`) served the spikes.

**Done** (three Sonnet roles from `.claude/agents/` and the Opus reviewer; the il-analyst role ran out of context
twice and was taken over by a fresh one each time, as the plan allows):
- **Record 0055 and the spikes** (`78838ca`): written before code, revised after S40 and audit round 1. P76 and S40 in
  `il-inventory.md` (chapter M4.5d2b2b), 11 kept fixtures `tests/viper-smoke/memory/s40_*`. S40 (d) chose no
  dropped-block row through a pointer (the preferred row false-alarms on four correct programs); S40 (f): a chain
  verifies in 4.5 s, so cut 3 was not taken; a one-field loop 6.3 s, two fields 218.5 s (M4.8, question 189).
- **Part A, fields in contracts** (`33ef42f`): target `d` with its reader; the rejections and acyclicity; the closure
  (every atom that gives an owning leaf's cell closes with `L == NULL || owns_block(L)` under its guards, recursively;
  merging; bodiless rules); the value rule at every store to an owning leaf; the flip.
- **Part B, the neutral re-keying** (`650fc4a`): `OwnerRef`, byte-identical.
- **Parts B and C** (`bcf2e42`): owning leaves of struct globals, `static` struct locals, struct locals and by-value
  parameter copies as owners in memory (ghosts, entry, the two updates, stores by name, lifetime-end rows, `main`'s
  exemption with `exm_` ghosts for deeper owners); copies as leaf stores; Part D's pointer side in `encode.cpp`
  (the closure of written loop invariants).
- **Part D, loops** (`5f98f14`): named leaves in loops (synthetic sites, carried ghosts, framing of unreached leaves),
  an owning local's closure in its loop carrier; audit round 2's fixes on Parts B and C.
- **The re-audit's fixes** (`f344559`): the covered site's leak exemption (unsound, now closed), global leaves reached
  by type, an assigned owner's closure (not built where the loop may replace the owner), one level of chain closure
  in loops. The auditor reviewed the fixes and the first form of the replaced-owner rule; its two later extensions
  (assignments inside expressions and through a pointer of the owner's type; any call for an address-taken owner)
  came after and were not audited: both only narrow where a closure is built (no closure: the field's token stays
  outside the loop and the end row still reports a drop), and the il-analyst's probes show no exit 3.
- **The lint for those shapes** (`776583e`): found by running `lint_viper.py` over the final build's Viper (225
  problems, all in the session's new tests); two rules added with twins, none loosened.
- **Tests.** 217 new test, fixture and example files (755 runs, all passing on the Mac): positives with twins, the
  audits' tests, 23 rejection pins and 15 pinned false alarms (record 0055 item 7's loop limits; question 195). The one flip of the plan: `reject_struct_owning_field.c` retired for `struct_owning_field.c` and its
  twin. No other existing outcome changed. VIR self-test 1707 cases (was 1669). Lint rules for every new shape
  (`test_lint_viper.py` 660, was 539), each mutation-checked.
- **Docs and record** (the last commit): Memory and ownership ("Owning fields", "P4417R0's owners in cerastes", the not-yet list), Annotations
  ("Owning members"), Error messages (`permission`, `leak`), the tutorial's heap chapter ("A structure that owns its
  buffer"), the developer guide's Testing page; `internal/encoding.md`; record 0055 accepted.
- **Audits** (read-only Opus): round 1 on the draft with S40 (no blocking item; S1–S7, N1–N10 taken into the record);
  round 2 on Part A (`4e2cdc2`, no blocking item), on Parts B and C (`d299fb9`, B1 leaf ghosts havocked by loops), the
  re-audit with Part D (`7ba961f`, B1 an unsoundness from the in-session S3 change, B2 exit 3), and a focused review
  of those fixes (one more exit 3, fixed). No wrong verdict survived.

**Checks**
- vprdiff against the baseline: every existing program's Viper byte-identical (4,668 entries, 0 differences) after
  every part and at the end. No allow-list was needed.
- The VIR self-test (1707 lowering cases, 274 rejection cases, 0 problems; the session's 38 `owning_*` cases through
  Silicon, all passing), `lint_viper.py` over the final build's 5028 `.vpr` and the VIR output (6735 files clean),
  `test_lint_viper.py` (660), `test_run_tests.py` (OK), `format.sh --check`, `build_docs.sh` with
  `check_doc_examples.py --html` (0 problems).
- `check.sh` (full) on the x86-64 box: run on the docs-and-status commit after it was made; its result is in the follow-up commit (the next line is updated then).

**Corrections of new tests after their first run** (question 160; each listed, none audited separately):
- `tests/vlibc/negative/owning_field_vlibc_view_free_with_block.c`: L24→L25 (a header line miscounted).
- `negative/owning_field_noheap_value_address.c` L16→L17 and `negative/owning_field_complit_live.c` L17→L18:
  `#include <stddef.h>` added to the pair (`NULL` was undefined).
- `negative/owning_field_global_main_other_leak.c` L20→L23 and `negative/owning_field_gives_back_global_unknown.c`
  L21→L24: a read of the global added, since EDG's #550 ("set but never used") fails a test with warnings;
  `negative/owning_field_take_pointer_leak.c`, `owning_field_owner_to_leaf_leak.c`, `owning_field_leaf_to_owner_leak.c`:
  the same for #550/#177, lines kept.
- `negative/owning_field_value_result_dropped.c` L27→L30: `make` writes the buffer before returning it (a structure
  owned in a contract needs its buffer's values).
- `negative/owning_field_cover_caller_free.c`: one expected error became three (`invalid-free` twice at L27, the
  caller's return row `permission` at L29), all from its one mistake, freeing a block the caller does not own under
  question 194's rule.
- `negative/owning_field_loop_owning_local_read_drop.c`: the drop moved before the read (the first version also read
  freed memory in the next iteration).
- The static-return pair: changed so `main` frees `r->data`, then restored to its original program when question
  194's rule was taken.
- The 15 pinned false alarms record their actual errors (by design).

**Not done** (each pinned, documented and in the handoff for M4.5d2b2c or M4.8): a loop through a non-owning heap alias
of a borrowed structure (S5's second half); a call through an alias of `&s` in a loop; an owning `struct S *` global's
field freed in a loop; in `main`, a global leaf reached only by type in a loop; the flag loss of a leaf carried by type
or of an owning local that may be null at the loop's entry (`uninitialized`); a three-level chain in a loop; the cost
of two owning fields in one loop (218.5 s) and of one failing probe (`cover_twice`, over 300 s).

**Surprises.**
- **`perm` inside `forperm`.** Silicon refuses it, so `main`'s exemption for deeper owners is framed through Bool
  ghosts (`exm_<n>`) assigned before the leak row.
- **An in-session fix that was unsound.** The cost of a covered closure (two calls of a function returning `&s` took
  Silicon past 170 s) was fixed by dropping the closure; the site kept its leak exemption, so a lost block verified.
  The re-audit caught it with a three-line program, and the fix made the exemption follow the site.
- **Havocked ghosts.** Leaf ghosts that loops did not carry were havocked at the loop head: false leaks on ordinary
  programs and several exit 3s (audit round 2 B1, then B2). Framing what a loop cannot reach, and guarding every read
  that rested on a ghost with the cell's permission, closed the class.
- **Process.** The harness refuses a subagent's second hand-back, which silently dropped two reports; the agents then
  wrote each report to a file as well. Three agents (two audits and the baseline) shared one checkout's build only
  through snapshot worktrees: `.claude/worktrees/m4.5d2b2b-aux` (the lowering, committed there and moved over by tree
  objects) and `-audit` (each audit's own build).

**First step for the next session:** M4.5d2b2c's planning (`not_invalidating`, `--strict-owner`, `ref_to_uninit` on a
field; handoff §3 "M4.5d2b2c", with "From M4.5d2b2b"'s pinned loop limits).

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
| M4.5d2b2b Owning fields | 2026-11-09/10 | `m4.5d2b2b-owning-fields` | 0055 | `owning` on members: the closure of every atom over owning fields (acyclic, any depth), the value rule at stores, named leaves (struct locals, globals, `static` locals, by-value copies) as owners in memory, copies, loops; P4417R0's `[[owner]]` mapped; the session tools `regen_vpr.py` and `vprdiff.py`; seven loop limits pinned for d2b2c |
| M4.5d2b2a Owners in memory | 2026-11-09 | `m4.5d2b2a-owners-in-memory` | 0054 | block sites through loads and on globals; owning and marked globals, `static` locals and address-taken locals (the deep atom; static owners exempt at `main`'s end); written `owns_block` invariants; loop carriers by current state lifting d2b1's four false alarms; the static struct leaf cover; d2b2 split (owning fields and P4418R0 to d2b2b) |
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
