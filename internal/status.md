# Status

Session log, newest first. Each entry says what is done, what is not, any surprises, the decision records added and
the exact first step for the next session. Trimmed at the end of M4.4c (owner, 2026-10-26) to the latest entry and a
history table; the full log up to M4.4c is `git show 1fcc43b:internal/status.md`, M4.4c's entry is
`git show fce46de:internal/status.md`, M4.4d's `git show 9017105:internal/status.md`, M4.5a's
`git show 72e30b3:internal/status.md`, S5's `git show 794e960:internal/status.md`, M4.5b1a's
`git show bdc6bd0:internal/status.md`, M4.5b1b's `git show 8003dca:internal/status.md`, M4.5b1c1's
`git show eab4058:internal/status.md`, M4.5b1c2's `git show a5e3e30:internal/status.md`, M4.5b1c3's
`git show 5ffb1a2:internal/status.md`, M4.5b2a's `git show 38877b4:internal/status.md`, M4.5b2b's
`git show 1bfa705:internal/status.md`, and M4.5c1's `git show cbdcce5:internal/status.md`.

## M4.5c2a — struct values and struct storage (2026-11-02/03): done on branch `m4.5c2a-struct-values`

Plan: `.claude/plans/m4-5c2a-struct-values.md`. Read `internal/handoff.md` next: §2 (owner steps: reset the local `main`
to `origin/main`, then push this branch and open its PR), §3 (notes by session; "M4.5c2b" first), §4.2 (M4.5c2b), §8 (no
question open).

**Owner decisions (2026-11-02):** at planning, M4.5c2 is two sessions, c2a struct values and struct storage now, c2b places
through a heap read next (question 95); compound literals are C objects and C++ values (96); passing and returning by
value read every member (97); c2a's flips (98); written struct atoms in loop invariants move to c2b (99); struct `?:`,
comma and assignment values stay rejected naming M8 (100). During the session: two VIR self-test rejection cases retired
for lowering cases (101); the expectations of new tests corrected after their first run (102: `struct_value_seq_calls.c`
`error unsequenced`, `struct_value_result_pointer_unrelated.c` `lifetime`, S34's two fixtures, the `&h.x`/`&(int){5}`
global-initializer rejections as `unsupported_expression`, the constexpr-element pin's two lines, four twins rewritten to
fail for one reason, the static-local twin `unsequenced`, the `return &(…)` pins' front-end warning,
`call_in_struct_init_check.c`'s line, two VIR cases); one more new VIR case's row index after the last `check.sh` (103).
PR #24 (M4.5c1) merged during the session. The session ran in the
worktree `.claude/worktrees/m4.5c2a` (branched from c1's tip `cbdcce5`) with its baseline in a detached one,
`.claude/worktrees/m4.5c2a-base`.

**Done**
- **Record 0044** (accepted; revised after P67, P68, P69, S34, audit round 1 on the record (blocking B1 the elements of
  `&(T){…}` unsequenced, B2 a `const` literal's address; S1–S8, N1–N7 taken), audit round 2 on the code (no blocking item;
  S1 `volatile`/`_Atomic` literal values, S2/S3 pins, S4 the lint's `Addr` rule reworded, S5 `u = mk().t;`; N6–N8 taken)
  and one re-audit (no blocking item; S1 the global-leaf loop rule outside `main`, N1 a C++ initializer context kept
  rejected)), amending 0012 item 2, 0037 items 2–3, 0038 item 3 (for globals), 0039 items 2–4 and 8, 0041 items 2–5, 7,
  10–12 and 0043 items 1, 5–9, 15: struct values as tuples of leaves between fixed producers and consumers; by-value
  parameters (leaf formals, the copy a struct block) and results (one result per leaf, multi-target calls, per-leaf result
  splits); object arguments and `return s;` reading every member; struct stores of calls, call members and aggregates;
  `f().x`; C++ temporaries; compound literals (values; in C `&(T){…}` a hidden local of its block, every element first);
  struct globals and `static` struct locals (per-leaf initial values by linkage, member lengths in the global facts,
  `program_start` rows per leaf; H8 needs no fact); member measures; `restrict` beside by-value structs; invariant reads
  through a loaded pointer accepted and pinned (P69).
- **Code:** `src/vir/vir.{h,cpp}` (`Field`, `Aggregate`, `GlobalLeaf`, `Var::compound_literal`, `restrict_hazard`),
  `check.cpp` (struct positions, the literal `StoreE`, the Sequencer's struct cases), `src/backend/encode.cpp`
  (`struct_value`, `aggregate_value`, `return_struct`, struct-global initializers, hidden literal locals, the
  rejections), `diagnostics.def`, `src/vir/lower.cpp` + `lowered.h` + `src/viper/emit.cpp` (leaf formals and results,
  `Env`/`LV` leaves, entry copies, multi-target calls, `struct_store`, `struct_cell_write`, the global facts and rows,
  loop places and the global-leaf write rule); `scripts/lint_viper.py` and its self-tests.
- **Tests:** `tests/memory` 40 positives (ports `verifast/call_in_struct_init.c` and `verifast/struct_assignment.c`), 71
  error twins and 66 rejection pins; S34's 31 fixtures (`tests/viper-smoke/memory/s34_*`); VIR self-test
  `selftest_cases15.inc` (`VIR_CASES_MIN` 1468); lint self-tests (374); 3 doc examples.
- **Docs:** Memory and ownership (passing and returning structures, compound literals, structure globals and `static`
  locals, the not-yet list), Annotations (member measures and places), Error messages (the copy message corrected to the
  printed one; calls and returns; a literal's lifetime), Verifying (the rejected list); `internal/encoding.md`;
  `il-inventory.md` P67, P68, P69, S34.

**Flips (pre-approved, questions 98, 99, 101):** `reject_struct_by_value_param.c`, `reject_struct_by_value_result.c`,
`reject_struct_temporary_init.cpp`, `reject_struct_init_compound_literal.cpp`, `reject_struct_static_local.c`,
`reject_struct_decreases_member.c`, `global_struct.c`, `reject_struct_compound_literal.c` and
`reject_compound_literal_address.c` retired for positives of the same shape (the last two also get C++ pins); seven c2b
pins re-messaged (`Pins:` only, `expect:` lines kept); the self-test cases `struct_param_by_value` and
`struct_result_by_value` retired. No existing `expect:` line changed.

**Acceptance**
- The baseline (`cbdcce5`, detached worktree): 8416 passed, 0 failed, 1 skipped; VIR self-test 1448.
- **Viper against the baseline** (`build/m4.5c2a/regen/regen.sh`, every kept program regenerated): all 3180 `.vpr` and
  their `.map` byte-identical, none missing, 207 programs added; the VIR self-test's 1448 old cases byte-identical, 20
  added; every regenerated `.vpr` (3387) lint clean.
- `check.sh` (the full check, `CERASTES_CHECK_BASE` unset), run twice: the first stopped at the tests (one new twin,
  `struct_global_loop_frame.c`, had EDG warnings #550/#177 for globals it no longer read; fixed by reading them after the
  loop, its expect line kept); the second passed every step through the tests and the Viper lint — harness self-test
  1044 tests, 79, lint self-test 374, mutate self-test 292; tests **8872 passed, 0 failed, 1 skipped** (native-ok runs:
  Apple clang 1661, GCC 1661); Viper lint 3377 files clean — and stopped at the VIR self-test on one new case,
  `sglob_static_unheld`, whose expected row index was wrong (question 103: 0 → 3, the same reason at the same line). With
  the owner's approval the remaining steps were then run singly instead of a third full run: the VIR self-test **1468**
  lowering and 251 rejection cases, 0 problems, all 1468 through Silicon (1468 passed) and lint clean; walker oracle 136
  goldens, 1924 facts, 0 failed; `build_docs.sh` without warnings and `check_doc_examples.py --html` 0 problems.
- No Linux run (CI covers it).

**Surprises**
- The loop-invariant rejection of `**pp` was by spelling only: `p->q->x`, `*p->q` and `*gp` had always reached invariant
  and measure read chains, unpinned. P69 (44 programs, both modes) and audit round 1 found every verdict right; record
  0044 item 12 states the argument (guarded read chains, framing asserts for every recorded read) and pins them.
- H8 resolved itself: the existing liveness facts, `member_lv`, `loc_inv` and the `unique` `GB_` functions already tell a
  struct global's member blocks from another global's block (S34 (d)).
- C++ folds a constant compound literal to a plain constant or aggregate (P67 H2), and folds a struct temporary inside a
  variable's initializer so its braces vanish: `int r = get(T{{1, 2}, 3});` cannot be told from brace elision and stays
  rejected (re-audit N1).
- The global-leaf write rule (a loop writing `g.x` keeps `g.y` and `h`) was first applied outside `main` too; the re-audit
  showed it exits 3 on correct programs whose parameter atoms hold the leaf; it is now limited to leaves the function
  holds by name. Struct locals keep the field-wide rule (a completeness gap; changing it would change existing Viper).
- Record 0043 item 7 and the Error messages page quoted a copy message the tool never printed; the page now quotes the
  printed one.
- An agent's rebuild replaced `build/cerastes` while the first suite run was starting; that run's results were treated as
  provisional and every disagreement re-checked on the final build.

**Decision records added:** 0044-struct-values-and-storage.

**First step for the next session (M4.5c2b).** Confirm the owner merged this branch's PR with both CI jobs green; then
plan M4.5c2b from handoff §4.2 and §3 "M4.5c2b" (places reached through a heap read: `owns(**pp)`, `reads(**pp)`,
`live(*pp)`, `**pp`, `*gp`, `owns(*gp)`, `p->q->x`, `*s->q`, atoms on cell pointers and through a by-value struct's pointer
member; written struct atoms in loop invariants; the `restrict` re-check), saving the plan as
`.claude/plans/m4-5c2b-<topic>.md`.

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
| M4.5c2a Struct values and struct storage | 2026-11-02/03 | `m4.5c2a-struct-values` | 0044 | by-value struct parameters and results as tuples of leaves, `f().x`, C++ temporaries, compound literals (C objects, C++ values), struct globals and `static` struct locals, member measures, `restrict` beside by-value structs; invariant reads through a loaded pointer pinned; M4.5c2 split, heap-read places and written struct atoms to M4.5c2b |
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
