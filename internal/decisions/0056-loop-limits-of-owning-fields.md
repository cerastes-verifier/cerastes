# 0056 — Loop limits of owning fields (M4.5d2b2c)

- **Status:** draft (2026-11-10, M4.5d2b2c), written before code (invariant 6); S41 (0) taken in (every run agrees
  with the survey's rows: items 2–4 and 8 stand); to be revised after S41 (a)–(c) (`internal/il-inventory.md`,
  chapter M4.5d2b2c) and audit round 1. Owner decisions from M4.5d2b2c's planning,
  2026-11-10 (plan `.claude/plans/m4-5d2b2c-loop-limits.md`), each the recommendation:
  - **195:** d2b2c takes record 0055 item 7's pinned loop limits first.
  - **198:** M4.5d2b2c is the loop limits and the long-running tests (record 0057); `not_invalidating`,
    `--strict-owner` and marked fields are M4.5d2b2d's.
  - **201:** the written-invariant rule stays (item 8). The five pins it covers keep their `expect:` lines, their
    headers name the invariant the loop needs, and each gets a verifying twin that writes it.
  - Standing: 160 (a new test's expect line corrected after its first run, listed in status), 174 (static owners exempt
    at `main`'s end).
- **Milestone:** M4.5d2b2c.
- **Amends:** 0055 item 7 (its limits); 0054 item 3 (the value set, the flag snapshots) and item 5 (globals in `main`'s
  loops).

## Context

**Eleven pinned false alarms.** M4.5d2b2b left record 0055 item 7's loop limits as pinned false alarms
(`tests/memory/negative/owning_field_loop_*_false_alarm.c`; question 195). None hides an error: a carrier or a frame can
only add checks. M4.5d2b2c's planning surveyed each pin's cause from the code at `262edb3` (it could not run the
verifier; three rows rest on inference, which S41 (0) checks):

| Limit | Pin (`owning_field_loop_…_false_alarm.c`) | Cause | Read | Item |
|---|---|---|---|---|
| 5 | `pointer_arg` | a callee's `owns(*pp)` havocs the leaf's value; `wef` starts false on the path that enters null | code | 2 |
| 5 | `owning_local_maybe_null` | a lone written owner with a closure gets no value set | code | 3 |
| 2 | `alias_call` | the ghost is refreshed only for `&s` by syntax; `callee_takes_owner` is never true for a leaf site | code; inferred | 4 |
| 4 | `main_holds` | in `main` the leaf is neither carried nor framed, and its value is lost too | code | 5 |
| 3 | `global_pointer` | a `WBlock`'s cells are never places, so `gp->data` cannot be framed | code | 6 |
| 6 | `chain3` | `closure_deep = false` for struct-local leaves; only the last cell's flag is guarded | code | 7 |
| 1 | `alias` | the loop replaces the block without the written `owns(*p)` its twin has | inferred | 8 |
| 7 | `owner_replaced`, `_alias`, `_in_expr` | they need a fact true only in the first iteration | inferred | 8 |
| 7 | `owner_renewed` | the contract of `renew` consumes the structure's cells | inferred | 8 |

**What the encoder has** (`src/vir/lower.cpp` at `262edb3`):
- **Owners:** `OwnerRef` (a root `Var*` or `Global*` and a leaf path); `held_updates` frames every owner's ghost after a
  call or a `free` and runs the plain update `held := L != NULL` only for an argument that is `&s` by syntax
  (`gives_param_leaf`).
- **Flags:** a synthetic site that does not require its cells' flags snapshots them at the entry (`wef_`, `wfc_`); the
  invariant `known && wef ==> flag` and its `uninitialized` back-edge row keep what the loop began with. `wef` starts
  `stores_type(loop, T) || callee_takes_owner(loop, site)`; the second tests only global and cell-variable sites.
- **The value set** (0054 item 3(c)): an owner the loop writes gets `v == ev_v || v == null` only when another carrier
  or a keyed place could take or give its block (`others`).
- **Sites:** `owner_loop_sites` builds the synthetic sites: named leaves reached by the loop, globals reached by type
  outside `main` where the function holds them at entry, owning locals' closures (`make_closure`), and the closure of a
  struct local's leaf to one level (`closure_deep`). `resolve_blocks` turns each site into a `WBlock` (guard, pointer,
  cells), and `wblock_invariants`/`wblock_rows` give its invariant and rows. A `WBlock`'s cells are not places of the
  loop: only struct-local leaf cells are added to `inv_all` by a special case.

## Decision

1. **Scope.** Items 2–7 lift six false alarms; item 8 restates the rule for the other five pins. A limit that is not
   built keeps its pin untouched (the plan's cut order: limit 6, then 3, then 4; limits 5 and 2 are not cut without
   asking).

2. **Flags of a site the loop cannot fill** (limit 5, `pointer_arg`).
   - `wef`'s default gains `|| !loop_may_fill(loop, site)`.
   - `loop_may_fill` is a new scan, true when the loop holds a store of a non-null value of the site's pointer type, or
     a call whose callee has an `ensures` block site through a load of that type (a callee that can put a new block
     where the site's pointer is). **As built** (Part L-1), three routes: a store through memory (`Store`, `StoreE`)
     of a value of the site's pointer type that is not the null constant, or of a whole structure; a write to an
     address-taken variable of that type (an assignment, a declaration's initializer, an assignment inside an
     expression) with such a value; a callee whose `ensures` has a block site with a load step and that pointer type.
   - **Scope** (owner, 2026-11-10, question 204, "Allow them"): every synthetic site, not only owning leaves. Eleven
     existing programs' Viper changes only by `wef_… := false` becoming `:= true` (struct-local leaf closures, the
     owning `struct S *` global's sites, owning-local closures, 0054's owning-global carriers), each outcome unchanged,
     and three VIR self-test cases likewise.
   - **Sound:** `known && wef ==> flag` stays a checked invariant, with its entry check and its `uninitialized`
     back-edge row; a larger `wef` only asks more of the loop. A loop that cannot fill the site cannot give it a block
     without a value, so the row holds for every correct loop. A user can already write the same as a plain site on
     `s.data` (S41 (0) run 5).
   - **The call route is for non-plain sites only** (audit round 2 on Part L-1, S1; owner, 2026-11-10, question 208,
     "Restrict to non-plain"). A plain `ensures` block site (0054 item 2, "The mode") hands back only a block whose
     cells have values, so a loop calling such a callee keeps the guarantee; the route counts a callee whose `ensures`
     block site through a load of that type has another mode (`Kept` on a marked formal, `Uninit` on a result, the
     owning `ref_to_uninit` deep atom). The pin `owning_field_loop_site_call_written_false_alarm.c` then verifies and
     is retired for a positive.
   - **Tests** (audit round 1, S4; corrected after audit round 2, B1): each route of the scan has its own must-fail
     twin, a loop over a site that is NULL at the entry, which the loop fills by that route with a block it never
     writes, and a read of the block after the loop (`uninitialized` at the read, `wef` false). A scan that drops the
     route makes `wef` true and moves the error to the back-edge row ("an iteration of this loop may end with … having
     no value"), so each twin pins its route (the auditor's models `model_store.c`, `model_addr.c`). A twin over a site
     that is not null at the entry takes `wef` from the entry flag and pins nothing, and a struct local written only by
     name has no synthetic site at all: the first twins of Part L-1 were of both kinds and are replaced.

3. **The value set of a lone written owner with a closure** (limit 5, `owning_local_maybe_null`).
   - `others` also counts an owner that has a synthetic closure site, so 0054 item 3(c)'s value set `v == ev_v || v ==
     null` is emitted and an owner that entered null stays null in the loop's view.
   - **Sound:** the value set appears only where `valset` already holds (an owner the loop writes only with null or its
     entry value), and it is a checked invariant with its rows.

4. **A leaf's ghost after a call through an alias** (limit 2, `alias_call`).
   - **(a)** In `held_updates`, an argument that is not `&s` by syntax but has the root's struct pointer type, where
     `gives_param_leaf` holds, runs the plain update under `arg == &s` (0054 item 6's guarded form):
     `held := (arg == &s) ? (L != NULL, read framed) : held` after the framed update. The same for the leaves of
     struct globals (`arg == &g`).
   - **As built** (Part L-1): the argument is a plain pointer variable by syntax (a local, not `&t` of another struct
     and not an expression), its parameter a pointer to the root's struct type, and `gives_param_leaf` holds; after
     the framed update, `t := (a == <root> && perm(<L's cell>) > none) ? L : null` and `held := (a == <root>) ? (t !=
     null) : held`, where `<root>` is the struct local's block or the global's (`GB_`). A wider first draft (any
     argument lowered to a variable) changed `owning_field_copy_both_call.c`, which `&t` of another local reaches; the
     syntactic condition keeps it byte-identical.
   - **Sound:** `held` rises only where the callee's `ensures` returns the leaf's block (`arg == &s` and the callee gives
     the place back), and a true `held` adds the lifetime and end rows, never removes one.
   - **(b)** `callee_takes_owner` is true for a leaf site when a callee's `requires` has a block site through a load of
     the leaf's pointer type (0054 item 5's rule, never built for leaves). It only feeds `wef`, a checked invariant.
   - **Consequence:** in `tests/memory/negative/owning_field_alias_fill_leak.c` the error moves from the closing line to
     the return (the named-leaf row now sees the block the callee gave through the alias; the plan's Flips, B), and so
     does the VIR self-test case of the same program, `owning_leaf_call_alias_leak` (owner, 2026-11-10, question 205,
     "Apply as Flip B's mirror").

5. **In `main`, a global leaf reached only by type** (limit 4, `main_holds`).
   - The leaf gets its synthetic site under a ghost `hs := held && perm(<L's cell>) > none`, taken at the loop entry
     before the entry rows and conjoined to the site's guard (`WBlock::g`, in `resolve_blocks`).
   - Invariants: `hs ==> (held == (L != NULL))` and `!hs ==> !held`, each with a mapped row at the entry and at the
     back edge (`permission`, 0055 item 7's message), so that a loop that raises `held` while `hs` is false is a
     reported false alarm, never Silicon's own failure (audit round 1, S2(ii); its example: `main` passing `pg = &g` to
     a filler in the loop, which item 4(a) makes raise `held`; pinned).
   - The pair is **in place of** the `hp` pair the encoder gives every carried leaf (`hp := perm(cell) > none`,
     `leaf_held_agrees(o, hp)` and `leaf_held_row`), not beside it: kept beside it, its entry row would read a `g.data`
     of unknown value in a `main` that never assigns it, a false alarm on a planned positive (audit round 1, S2(i)).
   - The loop gets the back-edge `forperm` leak row it lacked (no site was carried before).
   - **As built** (Part L-2): the ghost is named `hsl_<held>__<loop>` (clear of the lint's `hs_` names for owners a
     loop does not mention). An `hs` leaf gets no closure of its own (its pointee's owning fields are not carried).
   - **Sound:** where `hs` is false nothing is asked or carried and `held` is false, so `main`'s exemption (question 174)
     covers nothing for that leaf; where it is true every clause has its entry and back-edge rows, and `main`'s
     exemption covers only the carried token. `hs` is a snapshot of facts the function holds at the entry, never an
     assumption.
   - **Evidence** (S41 (c)): the pin with the site under `hs` verifies (3.5 s), and so does a loop in `main` beside a
     struct global it never names (which already verifies today); the twins fail at `main`'s exit row (another block
     leaked) and at the back edge (a block leaked in the body). In the pin `hs` is neither provable nor refutable
     (`fill()`'s `malloc` may fail): the two invariants cover both cases. A `main` that reaches the global only through
     a callee requiring `g.data == NULL` cannot be written (the call is `permission`: `program_start` fixes no value
     for `g.data`), as 0055 item 7 says.

6. **A site's block cells are places for later sites** (limit 3, `global_pointer`, and the base of limit 6).
   - In `resolve_blocks`, each resolved site's block cells become readable places of the loop, guarded by the site's
     `on`, for later sites and for the user's invariants. This replaces the special case that adds only struct-local
     leaf cells to the invariant. The permissions go to the invariant (`inv_all`), not to the loop's keyed places, and a
     site merged with an owner's carrier (an owning local's closure) adds none, so the owning-local loops keep their
     Viper.
   - **The value rule** (audit round 1, S3): `inv_all` frames reads only, so the value of such a cell is stated
     explicitly. A cell the loop does not write gets a snapshot under the site's `on` at the entry, the invariant `on
     ==> cell == snapshot`, and a mapped back-edge row; a deeper site needs it to know its pointer is the one it began
     with (S41 (b): without it the body's read of `a.b->c` is `null-dereference`). A cell is written when the loop
     stores it by name, stores through a pointer of the cell's type, or calls a function whose atoms reach cells of
     that type (the reach rules `loop_calls_reach_cells_of` and `touches_cells_of_type` already use); a written cell gets
     no value clause.
   - `owner_loop_sites` builds the closure (`make_closure`) for an owning `struct S *` global and for the leaves of
     struct globals that already have a site. **The replace test for globals**: no closure is carried (0055 item 7's
     replaced-owner rule) when the loop may store a value to the global itself, by name (inside an expression too) or
     through a pointer of type `struct S **`, or calls a function whose contract covers the global's cell by name
     (`owns(gp)`) or by type (an atom through a pointer whose pointee is `struct S *`). Each route has a must-fail test
     or a pin: the closure's `wfc_` clause has no row of its own, and a missed route would carry a closure across a
     replacement.
   - The site's guard is `gp != NULL` read under the entry snapshot of the cell's permission (`kv > none ? gp : NULL`);
     `held` need not join it, since `held == (gp != NULL)` is already the carrier's invariant. The existing `wef`
     snapshot covers the field's flag; no new flag ghost.
   - **As built** (Part L-2): the value rule is stated over the site's entry ghosts, not the live chain of guards and
     pointers: `eob ==> cell == vsb` and `eob ==> on && p == epb`, where `eob` and `epb` are the site's guard and pointer
     at the entry and `vsb` the cell's snapshot, each with its mapped row (stating it over the live chain nested the
     guards and made the Viper 2.5 times larger). A site's cells are places only where the loop cannot write the site's
     pointer; no existing program has a site outside that. A closure for a struct-global leaf is built only where the
     leaf already has a site other than item 5's.
   - **Sound:** a place only frames reads, and its value clause is a checked invariant; each new site is 0054 item 4's
     carrier with its entry and back-edge rows.
   - **Evidence** (S41 (a)): the `global_pointer` loop with the block's cells as places verifies (4.0–4.7 s, both
     modes); the twin without `free(gp->data)` fails at the back edge (`leak`).

7. **Chains at any depth in loops** (limit 6, `chain3`).
   - `closure_deep` goes: the closure of a struct local's leaf is built to every owning depth, as for owning locals.
     Each intermediate cell's flag joins the guard of the deeper site, and each intermediate cell is a place with item
     6's value rule.
   - **The pre-read of a site whose pointer is read through an earlier site's block cells is emitted after that
     site's rows** (at the entry and at the back edge), for the closures of struct leaves and of owning globals; a site
     merged with an owner's carrier (an owning local's closure) keeps its order, since the owner's rows already come
     first (owner, 2026-11-10, question 206, "Build it now"). Audit round 1 (S1) showed that S41 (a) and (b) had placed
     the pre-read so, against the first draft's reading of them, and that the order decides how a failing program
     fails: on the base build a two-level chain whose loop frees the middle block exits 3 (Silicon's own
     `assignment.failed` at site 2's hoisted pre-read, before site 1's row), and with the move only mapped rows remain.
     In a correct program the order cannot matter (the auditor's beside-order variant of S41 (b) verifies too). The
     tests include must-fail programs of that shape at depth 2, at depth 3 and for an owning `struct S *` global, each
     a mapped error and no exit 3.
   - S41 (b) decides the cost: the plan's rule was "over 60 s in either mode, the limit stays pinned". The hand-edited
     loop verifies in 12.1–16.6 s over six timed runs (both modes; Silicon only, load 5–10 with no other Silicon or
     compiler running), and its twin without `free(a.b->c->x)` fails at the back edge (`leak`, 20.1–23.7 s). The item
     stands.
   - **As built** (Part L-2): the first complete build verified the pin in 38–72 s. The third site's case splits
     (`wblock_splits`: no-op `if` statements on pointer and block equalities with the earlier sites, which guide
     Silicon's paths and carry no obligation; 52 against the hand edit's 28) multiplied the paths. The build leaves out
     only the pair splits between a closure site read through an earlier site's cells and that earlier site (the two are
     distinct by the pair rows, which are checked, and by type); every other program's splits are unchanged (`vprdiff`).
     Built and quiet, the pin verifies in 21–22 s (C23, C++20), beside the hand edit's 12–16 s.

8. **The written-invariant rule** (question 201; restates 0055 item 7's first bullet).
   - A loop that replaces the block of an owning field of a **borrowed** structure writes `owns(*p)` in its invariant
     (`tests/memory/owning_field_loop_replace.c`). The `alias` pin is this case: its loop replaces `p->data` through `v`
     with no written invariant.
   - A loop that replaces **the owner itself** and frees the field only in its first iteration writes the guarded site
     `v == NULL || i != 0 || v->data == NULL || owns_block(v->data)`: only a written invariant can state a fact of one
     iteration (the `owner_replaced` pins). **One of the three cannot write it today:** in
     `owning_field_loop_owner_replaced_alias_false_alarm.c` the owner `v` is address-taken (`pv = &v`), and a written
     `owns_block(v->data)` is rejected (`unsupported_expression`: a block site names a pointer parameter or a place
     reached through loads), while the spelling `(*pv)->data` is `permission` ("this loop invariant reads `(*pv)->data`,
     which this loop does not hold"). So that pin stays a pinned false alarm, unrenamed, as the plan provides for a
     twin that does not verify; it goes to the handoff's questions.
   - `renew` takes `owns(**pp)` and returns only `owns(*pp)`, so its contract consumes the structure's cells: the
     caller cannot free `v->data` afterwards, and a second call cannot be made. S41 (0) run 7 confirms it (two
     straight-line calls: `permission` at the second, "needs to own `**pp`"), so that pin is an ordinary must-fail,
     not a false alarm.
   - **Evidence** (S41 (0)): run 1a, the `alias` pin with `owns(*p)` written, verifies; run 1c, the same loop without
     the alias and without the invariant, gives the pin's seven errors at the same lines, so the alias is not the
     cause; run 6, the `owner_replaced` pin with the guarded site, verifies. The invariant must name the borrowed
     parameter: `owns(*v)` for the alias is `permission` ("may be `*p->data`", run 1b). The takeover of a loop's places
     needs no change.
   - Automatic closure sites for borrowed structures are not built: they would change existing loops' Viper and give a
     structure with two owning fields the 218 s shape (0055 item 7, Cost). M4.8 may revisit them with the cost work.

9. **The lint** (`lint_viper.py`): item 4(a)'s guarded update, admitted only as `held := (a == <root>) ? (t != null) :
   held` after the conditional read of the same leaf's cell under `a == <root>`, where `<root>` is the ghost's own
   owner; the ghost `hs` and its one assignment at the loop entry (item 5); the new closure sites reuse the existing
   rules. **Categories:** none new; the messages of 0054 item 4 and 0055 item 7 are reused.

10. **Facts:** S41 (`internal/il-inventory.md`, chapter M4.5d2b2c; kept fixtures `tests/viper-smoke/memory/s41_*`).
    **Still pinned after this session:** item 8's pins, four renamed (the plan's Flips, C: three `_no_invariant` with
    verifying `_written` twins, and `owner_renewed_consumed`, an ordinary must-fail) and `owner_replaced_alias` kept as
    a false alarm (item 8); any limit cut; N2 (no pin; M4.8).

11. **A struct local whose address does not escape is not reached by type** (audit round 2 on Part L-1, S2; owner,
    2026-11-10, question 209, "Build it now").
    - **The limit.** A loop reaches a struct local's owning leaf "by type" (0055 item 7) when it stores through a
      pointer of the leaf's type or calls a function whose contract names cells of that type, even when no pointer to
      the structure or into it exists. Its synthetic site then needs item 2's flag rule, and a loop that may fill such a
      site loses the flag of the block the leaf holds (`owning_field_loop_site_store_written_false_alarm.c`; the
      `pointer_arg` limit was the same reach).
    - **The rule.** A struct local escapes when its address or the address of any of its members, at any depth, is
      used as a value anywhere in the function: passed to a call, stored, assigned or used to initialize a variable,
      returned, converted, or compared with a pointer that is not the null constant. Forming such an address only as
      the base of a member access, a load or a store through it is not an escape. VIR marks every struct local
      address-taken (it is a block with member cells), so the scan is new: over the function's body (and its loops'
      written invariants, which may name `&s`), not over VIR's `address_taken`.
    - **Effect.** A leaf of a struct local that does not escape is reached by a loop only when the loop names the
      structure (`lused`); the by-type tests (`touches_cells_of_type`, `loop_calls_reach_cells_of`) skip it, so a loop
      that does not name it frames it out (its cells, block and ghost keep their state across the loop).
    - **Sound.** A framed owner's cells and block are not in the loop's invariant, so the body holds no permission to
      them: a store or a call that did reach them through a pointer the scan missed would be a `permission` error in
      the body, never a silent change (Viper frames what the invariant does not name). A missed escape costs a false
      alarm, not a wrong verdict.
    - **Tests.** The store pin verifies and is retired for a positive; must-fail twins: the same loop with the address
      escaping by each listed route (and a store through it in the loop that leaves the block without a value), each
      reported; existing programs' Viper changes only where a by-type reach of a non-escaping struct local disappears
      (an allow-list fixed before the build).

12. **A call's closure rows after the row that the caller holds the structure** (audit round 2 on Part L-1, S3; owner,
    2026-11-10, question 210, "Fix now, bounded").
    - **The exit 3.** On the base build too: a call to a function with `requires(owns(gp)) ensures(owns(gp))` for an
      owning `struct S *` global, after `free(gp)` in a failing program. The call's row for the closure of `owns(gp)`
      reads `gp->data` before any row checks that the caller still holds `gp`'s block, so Silicon fails on its own
      (`insufficient.permission`, unmapped) instead of reporting the mapped error.
    - **The rule.** At a call, the read of a closure site's pointer comes after the rows that check the caller holds
      the cell it is read from (the structure's `owns` row), as item 7 orders a loop's pre-reads. Then only mapped rows
      fail.
    - **Bound.** If the change grows beyond a small reordering of one function's rows, the exit 3 is pinned instead and
      filed for M4.8.
    - **Sound.** Only the order of checks changes; no check is added or removed.

## Consequences

- Six loop shapes over owning fields verify: a leaf whose loop cannot fill it, an owning local that may enter null, a
  call through an alias of `&s`, a global leaf in `main`'s loop, an owning `struct S *` global's field, and a chain of
  three owning levels (each where its item is built).
- The five shapes that need a fact the encoder cannot infer are documented with the invariant to write, each with a
  verifying twin that writes it.
- Existing programs' Viper changes only where the plan's allow-list (Flips, D) says, each with the same outcome.
