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
     where the site's pointer is).
   - **Sound:** `known && wef ==> flag` stays a checked invariant, with its entry check and its `uninitialized`
     back-edge row; a larger `wef` only asks more of the loop. A loop that cannot fill the site cannot give it a block
     without a value, so the row holds for every correct loop. A user can already write the same as a plain site on
     `s.data` (S41 (0) run 5).

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
   - **Sound:** `held` rises only where the callee's `ensures` returns the leaf's block (`arg == &s` and the callee gives
     the place back), and a true `held` adds the lifetime and end rows, never removes one.
   - **(b)** `callee_takes_owner` is true for a leaf site when a callee's `requires` has a block site through a load of
     the leaf's pointer type (0054 item 5's rule, never built for leaves). It only feeds `wef`, a checked invariant.
   - **Consequence:** in `tests/memory/negative/owning_field_alias_fill_leak.c` the error moves from the closing line to
     the return (the named-leaf row now sees the block the callee gave through the alias; the plan's Flips, B).

5. **In `main`, a global leaf reached only by type** (limit 4, `main_holds`).
   - The leaf gets its synthetic site under a ghost `hs := held && perm(<L's cell>) > none`, taken at the loop entry
     before the entry rows and conjoined to the site's guard (`WBlock::g`, in `resolve_blocks`).
   - Invariants: `hs ==> (held == (L != NULL))` and `!hs ==> !held`.
   - The loop gets the back-edge `forperm` leak row it lacked (no site was carried before).
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
     leaf cells to the invariant. They are places with the loop's ordinary value rule: a cell the loop does not write
     keeps its entry value (a value snapshot and `on ==> cell == snapshot`, checked like every place's), which a deeper
     site needs to know its pointer is the one it began with (S41 (b): without it the body's read of `a.b->c` is
     `null-dereference`). The permissions go to the invariant (`inv_all`), not to the loop's keyed places, and a site
     merged with an owner's carrier (an owning local's closure) adds none, so the owning-local loops keep their Viper.
   - `owner_loop_sites` builds the closure (`make_closure`) for an owning `struct S *` global and for the leaves of
     struct globals that already have a site, with a replace test for globals like `may_replace_local`'s (a loop that
     may replace the global itself carries no closure: 0055 item 7's replaced-owner rule).
   - The site's guard is `gp != NULL` read framed (`perm(gp's cell) > none ? gp : NULL`); `held` need not join it,
     since `held == (gp != NULL)` is already the carrier's invariant. The existing `wef` snapshot covers the field's
     flag; no new flag ghost.
   - **Sound:** a place only frames reads, and its value clause is a checked invariant; each new site is 0054 item 4's
     carrier with its entry and back-edge rows.
   - **Evidence** (S41 (a)): the `global_pointer` loop with the block's cells as places verifies (4.0–4.7 s, both
     modes); the twin without `free(gp->data)` fails at the back edge (`leak`).

7. **Chains at any depth in loops** (limit 6, `chain3`).
   - `closure_deep` goes: the closure of a struct local's leaf is built to every owning depth, as for owning locals.
     Each intermediate cell's flag joins the guard of the deeper site, and each intermediate cell is a place with item
     6's value rule.
   - The planning's second change, each site's pointer pre-read moved after the rows of the earlier sites, is **not
     made**: S41 (b) verifies with the deeper pre-read beside the others, and moving every pre-read would reorder the
     Viper of every loop with two load sites (the owning-local closures among them) for no verdict.
   - S41 (b) decides the cost: the plan's rule was "over 60 s in either mode, the limit stays pinned". The hand-edited
     loop verifies in 12.1–16.6 s over six timed runs (both modes; Silicon only, load 5–10 with no other Silicon or
     compiler running), and its twin without `free(a.b->c->x)` fails at the back edge (`leak`, 20.1–23.7 s). The item
     stands; the built encoder's time is measured again on its tests.

8. **The written-invariant rule** (question 201; restates 0055 item 7's first bullet).
   - A loop that replaces the block of an owning field of a **borrowed** structure writes `owns(*p)` in its invariant
     (`tests/memory/owning_field_loop_replace.c`). The `alias` pin is this case: its loop replaces `p->data` through `v`
     with no written invariant.
   - A loop that replaces **the owner itself** and frees the field only in its first iteration writes the guarded site
     `v == NULL || i != 0 || v->data == NULL || owns_block(v->data)`: only a written invariant can state a fact of one
     iteration (the three `owner_replaced` pins).
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

9. **The lint** (`lint_viper.py`): the ghost `hs` and its one assignment at the loop entry (item 5); the new closure
   sites reuse the existing rules. **Categories:** none new; the messages of 0054 item 4 and 0055 item 7 are reused.

10. **Facts:** S41 (`internal/il-inventory.md`, chapter M4.5d2b2c; kept fixtures `tests/viper-smoke/memory/s41_*`).
    **Still pinned after this session:** item 8's five pins, renamed (the plan's Flips, C); any limit cut; N2 (no pin;
    M4.8).

## Consequences

- Six loop shapes over owning fields verify: a leaf whose loop cannot fill it, an owning local that may enter null, a
  call through an alias of `&s`, a global leaf in `main`'s loop, an owning `struct S *` global's field, and a chain of
  three owning levels (each where its item is built).
- The five shapes that need a fact the encoder cannot infer are documented with the invariant to write, each with a
  verifying twin that writes it.
- Existing programs' Viper changes only where the plan's allow-list (Flips, D) says, each with the same outcome.
