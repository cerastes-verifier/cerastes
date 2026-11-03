# Status

Session log, newest first. Each entry says what is done, what is not, any surprises, the decision records added and
the exact first step for the next session. Trimmed at the end of M4.4c (owner, 2026-10-26) to the latest entry and a
history table; the full log up to M4.4c is `git show 1fcc43b:internal/status.md`, M4.4c's entry is
`git show fce46de:internal/status.md`, M4.4d's `git show 9017105:internal/status.md`, M4.5a's
`git show 72e30b3:internal/status.md`, S5's `git show 794e960:internal/status.md`, M4.5b1a's
`git show bdc6bd0:internal/status.md`, M4.5b1b's `git show 8003dca:internal/status.md`, M4.5b1c1's
`git show eab4058:internal/status.md`, M4.5b1c2's `git show a5e3e30:internal/status.md`, M4.5b1c3's
`git show 5ffb1a2:internal/status.md`, M4.5b2a's `git show 38877b4:internal/status.md`, and M4.5b2b's
`git show 1bfa705:internal/status.md`.

## M4.5c1 — struct objects (2026-11-02): done on branch `m4.5c1-struct-objects`

Plan: `.claude/plans/m4-5c1-struct-objects.md`. Read `internal/handoff.md` next: §2 (owner steps: b2b's PR first, then this
branch), §3 (notes by session), §4.2 (M4.5c2), §8 (no question open).

**Owner decisions (2026-11-02):** at planning, M4.5c is two sessions, c1 struct objects (copies inside a function the
pre-agreed cut, not needed) and c2 struct values, struct globals and places through a heap read (question 90); owning
fields move to M4.5d2, which also covers the rest of P4418R0's invalidation profile (91); a struct copy reads every
member, an uninitialized one `uninitialized` at the copy (92); c1's flips (93). During the session: a new test's second
error after a contract that fails framing is not expected (94). The session ran in the worktree `.claude/worktrees/m4.5c1`
(branched from b2b's pushed tip `1bfa705`, so it stacks on b2b's PR) with its baseline in a detached one,
`.claude/worktrees/m4.5c1-base`.

**Done**
- **Record 0043** (accepted; revised after P65, P66, S33, audit round 1 on the record (four blocking items: overlapping
  atoms on one root, `restrict` through a loaded pointer — a hole since 0037 the auditor reproduced —, struct atoms and
  copies in the sequencing rules, overriding designators; S1–S11, N1–N5 taken), audit round 2 on the code (one blocking
  item: an initializer writing or taking the address of its own object, where GCC and clang contradict the C order; S1
  `restrict` through a pointer to a structure, S2 a struct-pointer global's `program_start` row, S3 `#pragma pack`;
  taken) and one re-audit (all closed; a C++ compound-literal initializer taken)), amending 0028 items 1 and 16, 0029 item
  1, 0037 items 2, 5–8 and 10, 0038 item 3, 0039 items 2–4, 0040 items 6 and 9, and the scope rows of 0041 and 0042:
  verified structures (C structures of scalars, pointers and nested structures; C++ only trivially copyable C-like
  aggregates; everything else rejected with its session); member blocks `loc(member(r, k), 0)`; struct locals always
  blocks through a trusted `alloc_struct_<n>_<tag>()` with member lengths and slot facts (`X != Y` for every
  ancestor/descendant slot, S33 (i)/(j): the four axioms admit cyclic models); `.`, `->`, `(*p).x`, nested chains, the
  null check on the base, `&p->x` with null/in-bounds and liveness; braced initializers per leaf (each element a checked
  conversion read under EDG's fold, in C every element first, the pre-fold list when EDG folded one; overrides, brace
  elision, out-of-order C designators, writes to the object being initialized, C++ temporaries and compound literals
  rejected); copies leaf by leaf at statement level; `struct S *` objects with `WF_S`; contract atoms `owns(*p)`,
  `reads(*p)`, `live(p)`, `owns(p->x)`, `owns(p->t)` and values `p->x`, `old(p->x)`, `r->x`; overlap decided per leaf cell
  and VIR field in every rule (separation, sums, splits, framing, `f__wf` ghosts by root/path/state, havoc, loop places,
  memory calls); overlapping atoms on one root rejected; P4222's markers per leaf; `restrict` rejected beside every place
  that can hold a pointer from which a same-type pointer can be formed (closing 0037 item 2's hole for `T**` and pointer
  globals too); loops with one liveness row per root; sequencing keys with member paths.
- **Code:** `src/vir/vir.{h,cpp}` (`StructType`, `Type::Struct`, `MemberAddr`, `PlaceKey`, `struct_leaves`,
  `restrict_hazard`), `check.cpp` (backstops, the sequencer's member keys), `src/backend/encode.cpp` (`struct_info`, member
  access, initializers, copies, contract places, the rejections), `diagnostics.def`, `src/vir/lower.cpp` (layout,
  allocation, scope exits, member places, per-leaf atoms, overlap, splits, loops, sequencing, `WF_S`); `scripts/lint_viper.py`
  (`alloc_struct_*` pinned structurally, the struct-pointer global's row) and its self-test; `src/viper/selftest.cpp`.
- **Tests:** `tests/memory` 39 positives (the ports `verifast/malloc_block.c` and `swap.c`'s `point_mirror` included) and
  122 negatives (53 error twins, 69 rejections); `tests/scalar/struct_declaration.c` with its twin; S33's 31 fixtures
  (`tests/viper-smoke/memory/s33_*`); VIR self-test `selftest_cases14.inc` (34 cases; `VIR_CASES_MIN` 1448); lint
  self-tests; 4 doc examples.
- **Docs:** Memory and ownership ("Structures", `restrict`, the not-yet list), Annotations (member places), Error messages
  (member messages of `uninitialized`, `null-dereference`, `permission`, `lifetime`), Verifying (the rejected list);
  `internal/encoding.md` §3 and §8; `il-inventory.md` P65, P66, S33.

**Flips (pre-approved, questions 93 and 94):** `tests/scalar/negative/reject_struct.c` retired for
`tests/scalar/struct_declaration.c`; `tests/memory/negative/global_struct.c` lost its `@ L8` line (a comment in its place);
seven pins re-messaged (`Pins:` only); the new `struct_bodiless_frame.c` dropped its `check @ L22` (question 94). No other
`expect:` line changed.

**Acceptance**
- The baseline (`1bfa705`, detached worktree): 7923 passed, 0 failed, 1 skipped; VIR self-test 1414.
- **Viper against the baseline** (`build/m4.5c1/vpr_compare.sh`, every kept program regenerated): of 2994 programs only
  `tests/memory/verifast/swap.c`'s four files differ (its source gained `point_mirror`); none rejected, none exiting
  differently; the VIR self-test's 1414 old cases byte-identical, 34 added. No exit 3 in 170 struct runs; the slowest new
  test 3.5 s alone.
- `check.sh` (the full check, `CERASTES_CHECK_BASE` unset): **all green** on the second run (the first stopped at the
  tests: a regenerated test had lost question 94's approved edit, see Surprises). Harness self-test 1044 tests, 79, lint
  self-test 343, mutate self-test 292; tests **8416 passed, 0 failed, 1 skipped** (native-ok runs: Apple clang 1588, GCC
  1588); Viper lint 3180 files clean; VIR self-test **1448** lowering and 251 rejection cases, 0 problems, all 1448 through
  Silicon; walker oracle 136 goldens, 1924 facts, 0 failed; docs green. No Linux run (CI covers it).

**Surprises**
- `restrict` was unsound since 0037 through a pointer loaded from memory (`T**` with `*pp == p`, a pointer global `gp ==
  p`): cerastes verified a result clang and GCC at `-O2` contradict. Audit round 1 reproduced it; record 0043 item 10
  closes it fail closed (no test used the shapes).
- The four axioms admit a cyclic model (`r == loc(member(r, 0), 0)`), so a nested struct held by fractions failed at a
  loop's entry with Silicon's own reason; the slot facts in the allocation and `WF_S` fix it (S33 (i), (j)).
- In C, "every element first" is not what compilers do once an element writes the object being initialized; in C++ EDG
  folds such a list to constants, hiding the write; both are rejected (audit round 2 B1), and folded lists are encoded
  from EDG's pre-fold original.
- P51's "`x.i` marks `x`": only under `&` (P65); irrelevant to item 4, which makes every struct local a block.
- test-mutator generated its tests with a script outside the tree and reran it after the owner-approved edit of question
  94, silently restoring the dropped `expect:` line; the edit was re-applied and the handoff (§6) gains a rule for
  generators.

**Decision records added:** 0043-struct-objects.

**First step for the next session (M4.5c2).** Confirm the owner merged b2b's and this branch's PRs with both CI jobs
green; then plan M4.5c2 from handoff §4.2 and §3 "M4.5c2" (by-value struct parameters and results, struct globals and
`static` struct locals, places reached through a heap read: `owns(**pp)`, `live(*pp)`, `**pp`, `*gp`, `*s->q`), saving the
plan as `.claude/plans/m4-5c2-<topic>.md`.

## History (one row per session; the full entries are in `git show 1fcc43b:internal/status.md`)

| Session | Date | Branch, merge | Records | Outcome |
|---|---|---|---|---|
| M4.5c1 Struct objects | 2026-11-02 | `m4.5c1-struct-objects` | 0043 | C structures with member blocks, struct locals as blocks, `.`/`->`, pointers to members, initializers, copies, `struct S *` objects with `WF_S`, contract atoms on struct places with overlap per leaf cell, markers per leaf; `restrict` closed beside loaded pointers (a hole since 0037); M4.5c split, owning fields and P4418R0 to M4.5d2 |
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
