# Plan: M4.5b1c3 — pointers to pointers, owning parameters, `*result`, written atoms in loop invariants (record 0040)

## Context

M4.5b1c2 (record 0039) is on `origin/main` (`a5e3e30`, PR #20, merged 2026-10-30 with both CI jobs green; the main
checkout's local `main` `7dda635` is a local merge with the same tree). At planning time PR #20 was still open. Records 0032–0039 give cells,
pointer parameters, contracts, loops, jumps and sequencing over memory. These are still rejected today, each pinned by a
test that names M4.5b1c3:
- `T**` and the address of a pointer variable (`unsupported_type` / `unsupported_expression`; `vir.h` `Type::Ptr`
  refuses a pointer pointee, `Var::address_taken` on a pointer);
- `[[cerastes::owning]]`: not registered and not in `<cerastes.h>` (0029 item 1; 0038 moved target `p` to b1c3);
- `*result` as a value in an `ensures` (0037 items 3, 5, 15);
- `owns`/`reads`/`live` written in `CERASTES_INVARIANT` (0038 item 5). Without them a parameter that the function
  reassigns keeps its place out of every loop, so three correct programs are pinned as false alarms (question 68).

b1c3 closes M4.5b1. It lands those four constructs and their ports: `address_of_local.c`'s two `int**` functions, and
`swap.c` L4-11 with its `swap(&x, &x)` aliasing trap. It also completes the b1 part of the Memory and ownership page.

**Session size** (owner, 2026-10-30, replacing the ≤ 4 h rule): a session is sized to what fits in one context, and
stays under about 12 h unless a reason is given. b1c3 is one session. Its scope is close to b1c2's, which landed in one
context with no cuts, and an estimate is 6–9 h. Written atoms are the last feature built. If context runs short before
they land, the session stops and asks (see "Stop and ask"); it never cuts them silently.

**Owner decisions (2026-10-30, this planning; handoff questions 73–77):**
- **73: atoms in a `do … while (0)` invariant stay rejected** (as in `CERASTES_CHECK`).
  `reject_atom_in_do_once_invariant.c` and `mem_rej_atom_in_do_once_invariant` keep their expectations. The handoff's
  flip list and 0038 item 10's table, which listed it for b1c3, are corrected.
- **74: `owns(**pp)` and reads of `**pp` in a contract** (places reached through a heap read) are deferred to M4.5c,
  together with struct places (`owns(p->f)`, `owns(*s)`). b1c3 allows `owns(*pp)` and `*pp` values; the rest is
  rejected naming M4.5c.
- **75: `owning` beside an explicit `owns`** on the same parameter in the same clause list is accepted and merged when
  the explicit atom is `owns(*p)` as a top-level conjunct or `p == NULL || owns(*p)`. The place keeps one atom with the
  explicit guard: `true` or `p != NULL`, either of which implies `owning`'s. `reads(*p)`, `live(p)` or an `owns` under
  another guard beside `owning` is rejected.
- **76: pointers to pointers at any depth** (`T**`, `T***`, …). Fields are named recursively (`v_ptr_ptr_int`).
- **77: session size.** Each session is sized to fit one context, under about 12 h unless a reason is given. This
  replaces the ≤ 4 h rule of 2026-10-20.
- **Still standing:** question 69 (the upstream EDG merge and the pin move wait for the box session) and question 68
  (b1c3 re-expects the three false alarms).

## Step 1 (first, before any code): reconcile and consolidate the plan documents

The session's first commit is markdown only, with one place per live fact.

1. **Branch.** The worktree `.claude/worktrees/m4.2b1` was at a detached `7174be4`. The branch
   `m4.5b1c3-pointers-to-pointers` starts from `origin/main` (`a5e3e30`; PR #20 had merged by the session's start,
   so not from the local `7dda635`, which has the same tree).
2. **The plan.** Save this plan, as approved, as `.claude/plans/m4-5b1c3-pointers-to-pointers.md`. Add an "Inputs from
   the handoff" section holding the §3 notes this session consumes (item 3 below).
3. **`internal/handoff.md`**, consolidated:
   - **Header:** one pointer, `git show c0f8823:internal/handoff.md`, to the text before this trim.
   - **§1–§2**, rewritten:
     - b1c2 is on `origin/main` (`a5e3e30`); the local `main` `7dda635` holds the same tree.
     - Owner steps: reset the local `main` to `origin/main`; at the end, b1c3's branch and its PR.
   - **§3 moved into the plan's "Inputs"** (b1c3 consumes them):
     - b1c1's "Cut part and what it needs" (written atoms, S28 P8);
     - b1c1's "Pinned false alarms";
     - b1a's "P52 is b1c3's";
     - M4.5a's "Registered attributes are skipped by `check_attributes`" (now carried out for target `p`; the note
       stays for `r`/`v`/`d` in d2 and c).
   - **§3 kept** (still open): everything else, with b1c2's notes unchanged.
   - **§4 rules:** the owner's new session-size rule replaces "≤ 4 h" (memory `session-size-few-hours` updated to match).
     Record numbers: 0040 is this session's, 0041 next.
   - **§4.2:**
     - b1c3's paragraph becomes one line pointing at the plan file. Its flip list drops
       `reject_atom_in_do_once_invariant.c` (question 73).
     - M4.5c's paragraph gains places reached through a heap read, `owns(**pp)` (question 74).
   - **§4.1 (box session):** record the fork's distance from upstream `main` (fetch, count) without merging
     (question 69 stands).
   - **§8:** one line records questions 73–77. No question is open.
4. **`plan.md`:**
   - The b1c3 row names the plan file.
   - The M4.5c row gains `owns(**pp)` (question 74).
   - The header's session rule ("≤ 4 h; owner, 2026-10-20") and the M4 "Sessions" paragraph take question 77. The
     header's "the last: M4.5b1c2" becomes M4.5b1c3.
5. **Memory** `session-size-few-hours` is rewritten for question 77.
6. **Commit 1 (planning):** items 2–4, local only, no Claude trailer (CLAUDE.md overrides the harness default).
   `status.md` is not changed until the session ends.

## Inputs from the handoff (moved here from `internal/handoff.md` §3 at step 1; `git show c0f8823:internal/handoff.md`)

- **Written atoms: the cut part and what it needs** (from M4.5b1c1, record 0038 item 5, from S28 and the audits):
  - `NOTAUTO(q)` by `Ref` for value and flag, and by liveness cell for liveness (round 1 S1);
  - the atom's own flag and range reads chain-guarded (S2);
  - case splits of `q` against every automatic place at the loop head (S28 P8);
  - amounts and checks as a call's and a return's, with `WF` and bounds as `permission` rows at the back edge (S4);
  - separation of two written atoms;
  - P8's tie (`q0 := q`, `(q0 == r) == (q == r)`), so that a reassigned `q` gives its permission back after the loop.

  S28 P8 (`il-inventory.md`): it passes only with the case splits at the top of the body, and the unguarded twin fails
  on entry. Without the tie the `ensures` fails after the loop when `q` is reassigned; a loop that never assigns `q`
  needs no tie. Two written atoms on variables that may be equal need an explicit `q != s` check (`permission`), or
  Silicon's own exhale fails first.
- **Pinned false alarms** (from M4.5b1c1, record 0038, Consequences): a parameter the function reassigns keeps its
  place out of every loop. So `loop_reassigned_param_access.c`, `loop_reassigned_param_return.c` and
  `loop_invariant_reads_unowned.c` (all `tests/memory/negative/`) expect `permission` errors on correct programs.
  b1c3's written atoms let the user say it (question 68: re-expected `verified` with the atom, each with a new twin).
- **P52 is b1c3's** (from M4.5b1a). `owning` on a parameter is kept on `param_type.attributes` (a definition reads
  `variable.assoc_param_type.attributes`, the union over all declarations), with no fork commit. Registration accepts
  a non-pointer entity and duplicates, and `RAF_MUST_MATCH` is not honoured for parameters, so the encoder checks every
  declaration. EDG refuses the type position (#1835), the decl-specifier position (#1866) and typedefs. A user
  `#define CERASTES_OWNING` is `annotation_macro_changed`.
- **Registered attributes are skipped by `check_attributes`** (from M4.5a, `encode.cpp:485`). Register target `p` in
  the same commit as the encoder's handling, never earlier. `d`, `r` and `v` wait for M4.5c and M4.5d2 (the note stays
  in the handoff for them).
- **The ports** (`build/m4.5a/ports.md` in the worktree, gitignored):
  - `swap.c`'s `swap` (L4-11) is b1's, with `swap(&x, &x)` as the aliasing trap's must-fail twin; `point_mirror` is
    M4.5c's and `main` M4.5d2's.
  - `address_of_local.c`'s `address_of_local` (L26) and `address_of_local_uintptr_t` (L45) are ported as written,
    with the ghost `int tmp` after `return` dropped.
- **The `do … while (0)` inconsistency** (found at planning): handoff §4.2 and 0038 item 10's table listed
  `reject_atom_in_do_once_invariant.c` for b1c3, while 0038 item 2 and the test's `Pins:` said "never lifted". The
  owner chose "never" (question 73).

## Setup (in parallel with step 1)

- **Worktree build:** `scripts/bootstrap.sh` in the worktree (last built for b1c1/b1c2), then
  `lockf build/m4.5b1c3/build.lock cmake --build build/cmake`.
- **Baseline that agents cannot disturb** (handoff §6), in the background in the main checkout (on `main` `7dda635`,
  not edited):
  1. `scripts/bootstrap.sh`;
  2. `python3 scripts/run_tests.py -j 8`;
  3. the VIR self-test;
  4. copy `build/vpr` and the VIR output to the worktree's `build/m4.5b1c3-baseline-vpr` and
     `build/m4.5b1c3-baseline-vir`.

## Record 0040 — pointers to pointers, owning parameters, `*result`, written atoms

The record is written before code (invariant 6), while P62 and S30 run. Audit round 1 runs on it in the background.

1. **Scope table** (b1c3 | M4.5b2 | M4.5c, which now also takes `owns(**pp)` | M4.5d2). It amends:
   - 0029 item 1: target `p` registered now; `owning(n)` with M4.5e1;
   - 0032 items 2 and 10: `T**`;
   - 0037 items 3, 5 and 15: `*result` as a value; the places an atom may name;
   - 0038 items 5 and 10: written atoms; the `do … while (0)` row corrected to "—" (question 73);
   - 0039 item 9: the still-rejected table.
2. **Pointer cells** (lifts 0032 item 10's `T**` row).
   - A pointer variable whose address is taken (a local, or a parameter through 0037 item 4's copy) is a cell. Its
     value field is named by 0028 item 1's rule (`v_ptr_int`, `v_ptr_ptr_int`, …), and its flag is `d_ptr_…`.
   - `Type::Ptr` accepts a pointer pointee at any depth (question 76). `void*` and struct pointees stay rejected at
     every level.
   - Qualifiers are ignored at every level of the field name. C++'s qualification conversion `int**` → `const int*
     const*` reads the same `v_ptr_int` cell. P62 checks its IL node, and C's implicit `int**` → `const int**` stays an
     incompatible-pointer error.
   - **`WF` instead of a range.** 0028 item 2 says `WF` is "part of the permission to every pointer-typed cell". So:
     - a store to a pointer cell asserts `WF` of the stored value (`internal`: every pointer the encoder builds is
       `WF`), and the stored value is a use (liveness, 0032 item 6);
     - `owns(*pp)`/`reads(*pp)` of a pointer cell carry `WF(pp.v_ptr_T)` where an integer cell carries its range;
     - the automatic loop invariant's range row becomes `kv > none && kd > none && x.d_ptr_T ==> WF(x.v_ptr_T)`.
   - **`**pp` in code.** A read or write through a loaded pointer composes 0032 item 6's checks:
     - first null, permission and flag on `pp`'s target, then the read gives a pointer value;
     - that value's use gets its liveness check, then null and permission on its own target.
     - Sequencing (0039) keys each place by its syntactic object; `*pp` and `**pp` are places of different fields.
   - **Havoc rule** (0038 item 3): a write through a `T**` (`*pp = q`) can reach every place of field `ptr_T`. A call
     whose `requires` owns a `ptr_T` cell havocs them too.
3. **`[[cerastes::owning]]` on parameters** (0029 item 1, target `p`; P52).
   - Registered as `{"owning", "?(X)", "p", RAF_ENTITY_ATTACHED}`, the first time the encoder handles it.
     `CERASTES_OWNING` is added to `<cerastes.h>`.
   - On a parameter, it adds `requires(p == NULL || owns(*p))` and `ensures(p == NULL || owns(*p))`, with `p` its entry
     value. They are added as the first clauses of each list, so they frame the user's reads of `*p`.
   - It counts as the `owns` conjunct of 0037 item 2's `restrict` rule.
   - **Beside an explicit atom** (question 75): `owns(*p)` as a top-level conjunct, or `p == NULL || owns(*p)`, in
     the same clause list is merged with `owning`. The place keeps one atom with the explicit guard (`true` or
     `p != NULL`), and `restrict_backed` sees that one atom.
   - Rejected by the encoder (`unsupported_annotation`, each with its own message):
     - a non-pointer, a pointer to function, `void*`;
     - `owning(n)` (M4.5e1);
     - a duplicate attribute;
     - `reads(*p)`, `live(p)` or an `owns(*p)` under any other guard beside `owning`.
   - A mismatch between declarations is `annotation_mismatch`, naming both.
   - EDG refuses other targets itself (registered `p` only). P62 records its diagnostic for a function, a local and a
     field, and a test pins one.
4. **`*result` as a value** (0037 item 3; lifts the pin `reject_result_value.c`).
   - In an `ensures`, `*result` is a post-state heap read keyed by the place `result`. It is framed per path by 0037
     item 7: the disjuncts are `D_i = g_i && q_i == result` over the earlier `ensures` atoms of its field, so `result ==
     p && owns(*p) && *result == 1` frames it.
   - In `f__wf`, `result` is a ghost formal and the read a ghost value.
   - In the method's `ensures`, the read is the `ite` chain.
   - Callers learn it through the call and 0037 item 3's case splits.
   - `old(*result)` is rejected (`result` has no pre-state).
   - An atom on `*result` stays rejected (d2).
   - Unframed, it is `permission` at the contract: `reject_result_value.c`'s `live(p) && *r == 1` becomes `error
     permission @ L10` (pre-approved re-expectation, see Flips).
5. **Written atoms in `CERASTES_INVARIANT`** (0038 item 5, S28 P8).
   - **What may be written:** `owns(*q)`, `reads(*q)` and `live(q)` (both spellings), for any pointer variable `q` in
     scope (a local or a parameter, assigned or not), in 0037 item 5's positions.
   - **Meaning:**
     - `q != null && WF(q) && offset_of(q) < len(block_of(q))` (`live`: no bound);
     - the permissions, guarded by `NOTAUTO(q)`: by `Ref` for value and flag against every automatic place of the
       field, and by liveness cell for the liveness;
     - for an automatic place, the place's own conjunct already holds it, and the atom adds `q == x_i ==> kv_i >
       none` (the atom on an automatic place is never counted twice).
   - **The atom's own flag and range reads are chain-guarded** (0037 item 7's form).
   - **Amounts:** snapshots at entry of what the loop takes; an `owns` atom takes `write` on the value and flag cells.
   - **Explicit checks, category `permission`:**
     - at entry, as a call's (0037 item 8): "enters this loop without owning `*q`, which its invariant needs";
     - at the back edge, as a return's (0037 item 10), with `WF` and bounds as rows: "an iteration of this loop ends
       without owning `*q` …";
     - pairwise separation of two written atoms (`owns`/`owns`, `owns`/`reads` of one field) under both guards.
   - **Case splits:** `q` against every automatic place of its field, at the loop head and at the top of the body (S28
     P8: without them Silicon fails on a disjunction).
   - **After the loop:** the permission comes back for the place `q` designates at the exit. When the loop assigns `q`,
     the user's invariant must say where `q` ends. P8's gap is documented with a test of each kind, the tie written by
     the user (`q0 == r` style), not emitted.
   - **Framing** (0038 item 6): a written atom frames the invariant's later reads of `*q`, as an automatic place does.
   - **Still rejected, by no session:** atoms in a loop measure, and atoms in a `do … while (0)` invariant (question
     73).
6. **Places an atom may name** (0037 item 5):
   - `*param`, including `*pp` of a `T**` parameter (a pointer cell, with `WF` in place of a range);
   - in an invariant, `*q` for a pointer variable.

   `owns(**pp)`, `live(*pp)` and reads of `**pp` in a contract stay rejected (`unsupported_annotation`, naming
   M4.5c; question 74).
7. **Categories:** none new. `permission` gains the written-atom messages and `*result` framing.
8. **Still rejected:**
   - `owning` on functions, locals and globals (d2) and on fields (c);
   - `owns(**pp)` and reads of `**pp` in contracts (c);
   - `owns_block`, `owns(*result)` and partnerless `ensures` atoms (d2);
   - `void*` and struct pointees;
   - atoms in measures, in `check` and in `do … while (0)` invariants (no session).

## Team (disjoint files; three Sonnet agents and one Opus reviewer from `.claude/agents/`)

| Agent | Owns |
|---|---|
| **il-analyst** (Sonnet) | **First, P62:** the IL in both modes of `int **pp`, `&p` of a pointer local and of a pointer parameter (`address_taken` on each), `**pp` reads and writes, `*pp = &y`, `inc(*ptrptr)`, `uintptr_t**`, `[[cerastes::owning]]` on a parameter after registering `p` only (and EDG's diagnostics on a function, a local, a field and `owning(n)`), `*r` in an `ensures` in both spellings, and written atoms in `CERASTES_INVARIANT` beside P59. **Then S30** (Silicon, hand-written targets with test-mutator): (a) a pointer cell's store, load and `WF` row in the automatic invariant, liveness of a loaded pointer, `address_of_local`; (b) `*result` in the method `ensures` chain and in `f__wf`; (c) written atoms: entry and back-edge checks, `NOTAUTO`, case splits, two written atoms that may be equal, `reads` and `live`, P8's tie; (d) time for each against S28's. **Then `src/vir/lower.cpp`** (the critical path): pointer cells (fields, `WF` rows, havoc by field); `*result` framing in `wf_method` and the method `ensures`; written atoms (amounts, checks, separation, case splits, framing). `lowered.h`/`emit.cpp` only if a node is missing. Then the baseline Viper diff. |
| **edg-backend** (Sonnet) | **First, `src/vir/vir.h`** from the record: `Type::Ptr` with a pointer pointee, `Var::address_taken` on pointers, `Atom` in an invariant context, `Deref` of the result formal. Then `vir.cpp` and `check.cpp` (lift the `T**` and address-of-pointer refusals; invariant atoms; `*result`), and `src/backend/encode.cpp`: types, `&p`, `**pp`, atoms in `CERASTES_INVARIANT`, `*r` in `ensures`, `owning` (desugar, the merge of question 75, rejections, mismatch); `register_attributes.cpp`; `include/cerastes.h` (`CERASTES_OWNING`); `diagnostics.def`; the messages of the pins that stay rejected. |
| **test-mutator** (Sonnet) | **First,** the hand-written S30 targets, then the tests and twins (below) written from the record before the code runs; the flips; the ports (`tests/memory/verifast/address_of_local.c` appended, `tests/memory/verifast/swap.c` new); the VIR self-test (`selftest_cases11.inc`, the flipped `mem_rej_*`, `VIR_CASES_MIN` in `check.sh`); `lint_viper.py` only if a new shape needs it; the doc example files. |
| **soundness-auditor** (Opus, read-only) | **Round 1:** record 0040 once drafted. **Round 2:** the code diff and tests, especially pointer-cell `WF`, the written atoms' `NOTAUTO` and checks, and `*result` framing. **One re-audit** of blocking fixes. Should-fix items not taken go to handoff §3. |
| **Orchestrator** | Step 1; record 0040 and its amendments; `doc/`; `internal/encoding.md`; `il-inventory.md` (P62, S30); integration; serialized builds; the baseline; status and handoff. Stops and asks the owner. |

Any question from an agent, or of mine, stops that work and goes to the owner.

**Code sites** (from this planning's survey):
- **`T**`:**
  - `encode.cpp` `vtype` (:433-462, the refusal at :455);
  - `address_of` (:2175);
  - the `!is_ptr()` guards on `mark_address_taken` (:2593, :4088);
  - `check.cpp` `accepted_pointer`/`pointer_reason` (:180-228);
  - `lower.cpp`:
    - `cell_suffix` (:816, which dereferences `int_type` and would crash on a `Ptr`), `use_cell_type` (:841), `fkey`
      (:1391);
    - the dummies `inv_dummy` (:946) and `contract_read`'s (:1356), which need `null`;
    - the snapshot type (:3990) and the `__wf` ghost types (:4591);
    - the range sites, which take `wf_expr` (:1193) for a pointer: `cell_range` (:1206), `cell_range_inv` (:4190),
      `atom` (:1274), `atom_returns` (:3563);
  - `prelude.cpp` (:134) for `field v_ptr_…: Ref`.
- **`owning`:**
  - `register_attributes.cpp` (:40-62);
  - `annotation_il.h`'s `Annot` (:31-48);
  - `check_attributes` (:485, parameters at :574 and :4089; P52: the attribute is on `param_type.attributes`);
  - the desugared atoms enter `requires_`/`ensures_` before `restrict_backed` runs (`function_of` :614-639,
    `vir.cpp` :613-660).
- **`*result`:**
  - `encode.cpp` `contract_read` (:1944);
  - `check.cpp` (:768);
  - `lower.cpp` `contract_read` (:1307-1385; `ghost_name`/`wf_ghosts_` are keyed by parameter index, so they need a
    result key);
  - `wf_method` (:4521-4635).
- **Written atoms:**
  - `encode.cpp` `spec_atom` (:1806);
  - `check.cpp` (:773, :515);
  - `lower.cpp` (:1803);
  - `vir.cpp` `contract_atoms` (:606) collects only `requires`/`ensures`, so invariant atoms need their own collector;
  - `lower.cpp` `loop()` (:3870-4187), `loop_places` (:3763), framing (`inv_deref` :976, `framing_condition` :1014),
    `call_atoms` (:2063) and `atom_returns` (:3499) as the models for the entry and back-edge checks.

## Tests (`tests/memory/`; `.c` in both modes; each positive has a twin failing for one reason)

| Positive | Twin (expected) |
|---|---|
| `int *p = &x; int **pp = &p; **pp = 5;` then `x == 5` (`check`) | `*pp = NULL; **pp = 5;` → `null-dereference`; `p` to a block-scope `y` read after its block → `lifetime` |
| `*pp = &y;` then `*p` reads `y` | `check(*p == old x)` → `check` |
| `&p` of a pointer parameter; a `T***` local chain (question 76) | null at the middle level → `null-dereference` |
| `void set(int **pp, int *q)`: `requires(owns(*pp) && live(q))`, `ensures(owns(*pp) && *pp == q)`, with a caller | `ensures(*pp == old(*pp))` → `postcondition`; a caller without the cell → `permission` |
| a loop over a pointer cell (the `WF` row; value kept, or havocked by `*pp = …`) | a value read after a havocking write with no invariant → the havoc's error |
| `[[cerastes::owning]] int *p`: leading and trailing, `CERASTES_OWNING`, C and C++; `if (p) *p = 1;`; callers with `&x` and `NULL`; `restrict` with `owning`; `[[cerastes::owning]] int **pp` | `*p` without a null check → `null-dereference`; the cell passed to a callee that keeps it → `permission` at the return |
| `owning` merged with `requires(owns(*p))` (question 75: `p` non-null), and with `p == NULL \|\| owns(*p)` | the merged non-null contract, called with `NULL` → `permission` at the call |
| — | rejected: non-pointer, `void*`, pointer to function, duplicate attribute, `owning(n)`, `reads(*p)`/`live(p)`/`c \|\| owns(*p)` beside it (question 75), two declarations that differ (`annotation_mismatch`), on a function or local (EDG's diagnostic, per P62); `owns(**pp)` in a contract (naming M4.5c, question 74) |
| `int *pick(int *p)`: `requires(owns(*p))`, `ensures(r, owns(*p) && r == p && *r == old(*p))`, with a caller using `*r` | `*r` wrong → `postcondition`; `*r` with only `live` → `permission` (the re-expected pin); `old(*r)` → rejected |
| written `owns(*q)` for a pointer local walking two cells, with the user's tie | no tie → the error after the loop (P8's gap, documented); a consuming call in the body → `permission` at the back edge; no permission at entry → `permission` at entry |
| written `reads(*q)`, `live(q)` | a write through `q` under `reads` → `permission` |
| — | `owns(*q) && owns(*s)` with `q == s` possible → `permission` (separation) |
| ports: `tests/memory/verifast/address_of_local.c` gains `address_of_local` and `address_of_local_uintptr_t` (appended, line map extended); new `tests/memory/verifast/swap.c` (`swap`, L4-11; `point_mirror` is M4.5c's, `main` M4.5d2's) | `swap(&x, &x)` → `permission` (the aliasing trap; `swap_owns_alias.c` already pins it in another shape); a `T**` trap: `**pp = 1; *p = 2; check(**pp == 1)` with `*pp == p` possible → `check` |

**Flips** (pre-approved: exactly b1c3's list in handoff §4.2, without `reject_atom_in_do_once_invariant.c`, per
question 73):
- **Retired, each replaced by a positive of the same shape:**
  - `reject_pointer_to_pointer.c`;
  - `reject_address_of_pointer.c`;
  - `reject_atom_in_loop_invariant.c` and `reject_atom_in_loop_invariant_cpp.cpp` (an atom on an automatic place, not
    counted twice).
- **`reject_result_value.c`:** re-expected in place as `error permission @ L10` (`*r` framed only by `live`).
- **Question 68's three:** `loop_reassigned_param_{access,return}.c` and `loop_invariant_reads_unowned.c` gain the
  written atom and are re-expected `verified`, each with a new must-fail twin.
- **VIR self-test:** `mem_rej_pointer_to_pointer`, `mem_rej_address_of_pointer_variable` and
  `mem_rej_atom_in_loop_invariant` become lowering cases. `mem_rej_atom_in_do_once_invariant` and
  `mem_rej_atom_in_loop_measure` stay.
- **`tests/memory/verifast/address_of_local.c`:**
  - The header's "Left out" text is edited without changing its line count, so the three `expect:` lines keep their
    numbers.
  - If that cannot be done, the two functions go to a new file instead, and its line map points at the original.
- **No other existing test's outcome or Viper may change.** No program accepted today has a pointer pointee, an
  address-taken pointer, `owning`, `*result` or an invariant atom. Any change stops the session.

## Docs (invariant 8; every example is a test)

- **Memory and ownership:**
  - "Pointers to pointers";
  - "Owning parameters" (in "Pointer parameters and contracts");
  - `*r` in a postcondition;
  - "Loops" gains written ownership in invariants, P8's tie, and drops the reassigned-parameter false alarm;
  - "What is not supported yet" (`memory.adoc` :300) and line 61's "pointer types are not supported yet" updated, so
    the b1 part of the page is complete.
- **Annotations:** `[[cerastes::owning]]`/`CERASTES_OWNING`, and atoms in invariants in the spec-function table.
- **Error messages:** the new `permission` messages (written atoms, `*result` framing).
- **Verifying a program:** the not-supported list.
- **`internal/encoding.md`:** §3 (memory) and §8.

## Order of work

1. Step 1, setup and the baseline. In parallel: P62; the record draft, with audit round 1 in the background; `vir.h`
   from the record.
2. S30, then `lower.cpp` in three slices: (i) pointer cells; (ii) `owning` (mostly the encoder) and `*result`; (iii)
   written atoms. In parallel: `check.cpp`/`encode.cpp`, tests and twins, and the VIR self-test.
   - **Commit 2** after slices (i)–(ii) pass their tests and the baseline diff.
   - **Commit 3** after (iii).
   - A stop between them leaves a clean branch.
3. End to end with single-file runs (`build/cerastes verify --keep-vpr`), then every kept `.vpr`/`.map` regenerated
   against the baseline (handoff §6, own output directory).
4. Audit round 2, fixes, one re-audit.
5. Docs; `check.sh` (full, at most twice); status and handoff; the last commit.

## Stop and ask

- Any agent question.
- An existing test whose outcome or Viper changes outside the flips.
- P62 contradicting P52 (`owning` not kept where recorded), or S30 contradicting items 2–5 with no fix inside the
  record, including Silicon failing first with its own reason.
- Exit 3 on a correct program.
- A new test over 60 s.
- Context running short before written atoms land. The offer then: commit slices (i)–(ii) green, and move written
  atoms with their flips to a b1c4.

## Acceptance (commands run at the end)

- `python3 scripts/run_tests.py tests/memory tests/viper-smoke/memory doc/modules/ROOT/examples/memory doc/modules/ROOT/examples/error-messages doc/modules/ROOT/examples/verifying doc/modules/ROOT/examples/annotations`:
  all pass.
- **Viper unchanged outside the flips:**
  - every kept `.vpr`/`.map` regenerated with the new `build/cerastes` is byte-identical to
    `build/m4.5b1c3-baseline-vpr`;
  - after `check.sh`, `diff -r -x MANIFEST build/m4.5b1c3-baseline-vpr build/vpr` shows only added files and the
    flipped pins;
  - the VIR self-test against `build/m4.5b1c3-baseline-vir` shows only added cases and the three flipped `mem_rej_*`.
- `python3 scripts/test_run_tests.py`, `python3 scripts/test_lint_viper.py`, `python3 scripts/run_tests.py
  --check-headers`: green.
- `scripts/build_docs.sh` and `python3 scripts/check_doc_examples.py --html build/doc`: green.
- `scripts/check.sh` (full, `CERASTES_CHECK_BASE` unset): green. There is no Linux run; CI covers it, and any x86-64
  run goes to `matt@clang4.cpp.al`.
- **The status entry** lists:
  - record 0040, the flips and the audit outcome;
  - the plan file;
  - the first step for M4.5b2.

  The b1c2 entry moves to a history row. Handoff §3 gets what was not taken, and §8 any new question with a default.
- **Commits:** planning; feature (two, per "Order of work"); status and handoff. Local only, no Claude trailer, never
  pushed.
