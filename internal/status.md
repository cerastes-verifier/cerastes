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
`git show 7f11706:internal/status.md`.

## M4.5c2b — places reached through a heap read (2026-11-03): done on branch `m4.5c2b-heap-read-places`

Plan: `.claude/plans/m4-5c2b-heap-read-places.md`. Read `internal/handoff.md` next: §2 (owner steps: PR #25 first, then
this branch's PR), §3 (notes by session), §4.2 (M4.5d1, M4.5d2), §8 (no question open).

**Owner decisions (2026-11-03):** at planning, an `ensures` place through a load is the object on return, `old(**pp)` the
entry value of the entry object (question 104); heap-read `requires` places are automatic loop places by a snapshot
(105); `live(r.q)` moves to M4.5d2 (106); c2b's flips (107). After the design review: amounts follow objects (108, not the
recommendation); guards that read the heap accepted now (109, not the recommendation); both snapshots, method entry and
loop entry, where they can differ (110). During the session: a call with a load-rooted contract beside another access in
one expression accepted with a pre-call snapshot (111); written atoms done in this session, not cut (the pre-agreed cut
point); nine new twins gain duplicate same-line `permission` expect lines (112, as 102 in c2a). The session ran in the
worktree `.claude/worktrees/m4.5c2b` (from c2a's tip `7f11706`, stacked on PR #25) with its baseline in the detached
`.claude/worktrees/m4.5c2b-base`. The first test-mutator was stopped by the owner after it went idle on a finished
background task; a fresh one finished from a recovery brief (nothing lost but its per-file run log).

**Done**
- **Record 0045** (accepted; revised after P70, audit round 1 on the draft (blocking B1 a contract without a body
  returning a permission it never took, B2 the state of a `requires` atom's guard and place read outside the entry state;
  S3-S7, N8-N12 taken), S35, question 111, audit round 2 on the code (no blocking item; should-fix 1-3, notes 4, 5, 7
  taken) and one re-audit (nothing blocking)): places as access paths (root, steps to the base, member tail), the
  entry-state rule, the post-state reading, `f__wf` over ghost bases, call/entry/return rows with pass 0, amounts that
  follow objects, bodiless ties, measures, by-value leaves and `*r.q`, `eb`/`lb` loop places, heap-reading guards,
  written atoms through loads and on struct places, the encoder's rules over loads, `restrict` over load roots.
- **Code** (`fcb2d09`; the VIR block `f4dea4c`): `src/vir/vir.{h,cpp}` (`PlaceKey{root, via, path}`, `place_root`,
  disjoint key ranges with `is_*_site_key`, `guard_ok`, `restrict_hazard`'s walk, `user_spelling`), `check.cpp` (the place
  grammar), `src/backend/encode.cpp` (`place_pointer`, every former rejection site, `finish_atoms` by full key, markers,
  `owning` and `const` over load roots, `spell_loaded_place`), `src/vir/lower.cpp` (derived keys and everything above);
  `scripts/lint_viper.py` (`lint_ghost_refs`).
- **Tests:** S35's 61 fixtures (`tests/viper-smoke/memory/s35_*`; three contrast fixtures verified on purpose, each with
  its must-fail twin); `tests/memory` 53 positives (ports `verifast/issue310.c` and `verifast/globals_struct.c`) and 95
  twins and pins; 4 doc examples; VIR self-test `selftest_cases16.inc` (25 cases; `VIR_CASES_MIN` 1493); 389 lint
  self-tests.
- **Docs:** Memory and ownership (pointers to pointers, contracts, structures, globals, loops, the not-yet list),
  Annotations (places, `old`, guards, invariant atoms; a stale sentence corrected), Error messages (`permission`'s new
  messages), Verifying (the rejected list); `internal/encoding.md` (§3 and §8 paragraphs); `il-inventory.md` P70 and S35.

**Flips (pre-approved, question 107; 112):** ten pins retired for positives of the same shape
(`reject_owns_double_deref.c`, `reject_read_double_deref.c`, `reject_read_double_deref_invariant.c`,
`reject_atom_on_cell_pointer.c`, `global_pointer_global_deref_atom.c`, `reject_struct_member_chain_contract.c`,
`reject_struct_atom_in_invariant.c`, `reject_struct_value_deref_member.c`, `reject_struct_value_reads_member.c`,
`reject_struct_value_live_member.c`); `reject_struct_value_live_result.c` re-messaged naming M4.5d2 (expect line kept);
`Pins:` comments only in `global_pointer_global_unowned.c`, `struct_init_pointers.c`, `verifast/globals.c`. No existing
`expect:` line changed. Question 112 added duplicate expect lines to nine NEW twins only.

**Acceptance**
- The baseline (`7f11706`, detached worktree): 8872 passed, 0 failed, 1 skipped; VIR self-test 1468.
- **Viper against the baseline** (`build/m4.5c2b/regen/regen.sh`): all 3377 kept `.vpr` and `.map` byte-identical, none
  missing, after the last lowering change; the VIR self-test's 1468 old cases byte-identical, 25 added.
- The session's new and changed tests together (149 files, both modes): 545 passed, 0 failed; `tests/viper-smoke/memory`
  206 passed; `test_run_tests.py` 1044; `test_lint_viper.py` 389; `--check-headers` 3478.
- `check.sh` (the full check, `CERASTES_CHECK_BASE` unset), green on the first run: harness self-test 1044 tests, 79,
  lint self-test 389, mutate self-test 292; tests **9434 passed, 0 failed, 1 skipped** (native-ok runs: Apple clang 1779,
  GCC 1779); Viper lint 3643 files clean; VIR self-test **1493** lowering and 253 rejection cases, 0 problems, all 1493
  through Silicon and lint clean; walker oracle 136 goldens, 1924 facts, 0 failed; docs green; EDG tree clean at
  `f0ed5525`.
- `build_docs.sh` and `check_doc_examples.py --html` (212 code blocks, 0 problems). No Linux run (CI covers it).

**Surprises**
- The design review found that every place-keyed mechanism rebuilt its root from an integer key in every state; a
  derived key per (root, via) let the existing consumers treat a load-rooted base as one more root, keeping every
  load-free program's Viper byte-identical.
- Audit round 1's two blocking items were both consequences of the post-state reading: a trusted contract could return
  an object it never took, and a framing disjunct read in the wrong state verified a wrong body (both reproduced in
  Silicon, both pinned).
- Fractional liveness amounts make two places of one block indistinguishable, so some keeper twins legitimately report a
  sibling place's row too (question 112); no assumed fact can remove it (invariant 4).
- A loop moving a global pointer between two parameters' objects hits 0040 item 5's swap false alarm (amounts); the pin
  moves it between locals.

**Decision records added:** 0045-places-through-a-heap-read.

**First step for the next session.** Confirm the owner merged PR #25 (M4.5c2a) and this branch's PR with both CI jobs
green; then M4.5d1 if the vlibc repository exists, else plan M4.5d2 from handoff §4.2 and §3 "M4.5d2" (`live(r.q)` and
every partnerless `ensures` atom are there now, question 106), saving the plan as `.claude/plans/m4-5d2-<topic>.md`.

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
| M4.5c2b Places through a heap read | 2026-11-03 | `m4.5c2b-heap-read-places` | 0045 | contract and invariant places through loads (`**pp`, `*gp`, `p->q->x`, `*s.q`, `*r.q`) with the entry-state rule, the post-state reading, amounts that follow objects, bodiless ties, heap-reading guards, `eb`/`lb` loop places, written atoms through loads and on struct places, `restrict` over load roots; `live(r.q)` to M4.5d2 |
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
