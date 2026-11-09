# 0054 — Block sites through memory, owners in memory (M4.5d2b2a)

- **Status:** draft (2026-11-09, M4.5d2b2a), written before code (invariant 6). Revised after spike S39
  (`build/m4.5d2b2a/s39/README.md`) and audit round 1 (Opus; blocking B1 bodiless sites through memory, B2 the initial
  value of an owning global, B3 the carrier guards, the last pending S39 part 2; should-fix items 4–14 and notes 16–20
  taken in items 2–8 and 10). Owner questions during the session: 180 and 181. Owner decisions are from M4.5d2b2a's planning, 2026-11-09; plan `.claude/plans/m4-5d2b2a-owners-in-memory.md`;
  handoff questions 173–179, each the recommendation:
  - **173:** M4.5d2b2 is two sessions. This record is d2b2a's; d2b2b takes owning fields and P4418R0.
  - **174:** static owners are exempt at `main`'s end, as in P4417R0.
  - **175–179:** these are d2b2b's (handoff §4.2).
- **Milestone:** M4.5d2b2a.
- **Amends:**
  - 0029 item 1: target `v` on globals, `static` locals and address-taken locals.
  - 0041 items 7, 10 and 12: `owns(g)` of an owning global; loops; the implicit atom.
  - 0045 items 2, 7 and 10: block sites as places; partners; loops.
  - 0047 items 4, 7 and 10: block sites through memory; `main`'s leak row; loops.
  - 0052 items 2, 3, 4, 6(a) and 8.

## Context

**Block sites name only a formal or the result.** Record 0047 item 4 accepts `owns_block(p)` on a pointer formal and on
the result. It rejects:
- a block site through a load (`owns_block(*pp)`, `owns_block(p->q)`);
- a block site on a global (`owns_block(g)`);
- a block site in a loop invariant (0052 item 6(a), cut from d2b1).

The lowering keys a block site by `block_key`, which is the formal's index or −1 for the result. About twelve
consumers depend on that encoding (the planning survey).

**What this costs today:**
- A callee cannot hand its caller a block through `*pp` (0052's limits: `owns(**pp)` of a block allocated into `*pp`
  is a `leak` at the callee's end).
- A function cannot free a block reachable from a global.
- A loop cannot say which of two pointers holds which block (`heap_loop_swap_false_alarm.c`).

**Owners in memory.** 0029 item 1 gave `owning` four targets. d2b1 (0052 item 3) built owning automatic locals, whose
ownership lives in method ghosts (`held_v`, `tv_v`). An owner whose value is in memory cannot carry its ownership that
way:
- a global or a `static` local outlives every call;
- an address-taken local can be written through an alias.

Its ownership must travel with the permission to its cell, through contracts.

**P4417R0** [profiles.inval.owner] p5 Note 3 and (2.3): an owner with static storage need not delete its object. Record
0029 item 1 and question 123 had an owning global freed by `main`'s end; the owner chose P4417 (question 174).

**The loop carriers of 0052 item 3** guard each carrier with loop-entry snapshots ("not the block of an earlier
carrier"). So a block that becomes shared, or stops being shared, inside a loop is a pinned false alarm (audit 3b).
Written block invariants and owners in memory use the same carriers.

## Decision

1. **Scope.** This record covers:
   - block sites through loads and on globals (item 2);
   - loop carriers keyed by the current state (item 3);
   - written `owns_block` in loop invariants (item 4);
   - owning and marked globals and `static` locals (item 5);
   - owning address-taken locals (item 6);
   - the static struct leaf cover (item 7).

   It fails closed on the rest (item 10). Owning fields and P4418R0 are M4.5d2b2b's.

2. **Block sites through memory** (amends 0047 item 4, 0045 items 2 and 7, 0052 item 2).
   - **Forms.** `cerastes_owns_block(E)`, `cerastes::owns_block(E)` and `cerastes_owns_vlibc_block(E)` take a pointer
     value `E` reached from a formal, the result or a global through loads, member steps and by-value fields: `*pp`,
     `p->q`, `**ppp`, `*s.q`'s pointer `s.q`, `g`, `*gp`, `r->q`. That is any pointer value 0045's `place_pointer`
     accepts (the encoder's `live(*pp)` reader).
   - **The place** (audit round 1, item 12). A site's key is 0045 item 2's `(root, via, path)` of the object `E`
     points to. `owns_block(*pp)` has the key `(pp, [load], [])`, the key of `owns(**pp)`, and its block is
     `block_of(E)`. Distinct `(root, via)` bases get 0045's derived keys. So:
     - `owns(*pp) && owns_block(*pp)` names a pointer cell and the block it points to. That is not "beside";
     - `owns(**pp) && owns_block(*pp)` names one object twice, and stays rejected as "beside" (0047 item 4);
     - duplicates, mixed allocators and "beside" compare whole keys, not roots (`finish_atoms`, `block_atoms`).
   - **The mode** (audit item 10). A site through a load or on a global, and the deep atom of item 5, is 0047 item 4's
     plain block: every cell's flag is set (P4222's default). The marker of the root formal does not apply to a place
     through a load (`block_marker` reads only a site whose place is the formal itself). An owning variable also marked
     `ref_to_uninit` has a deep atom whose flags are unknown (0047 item 8's marked mode). The two attributes on one
     variable are accepted together only when every declaration carries both.
   - **When it is read.** In a `requires` the site is read in the entry state; in an `ensures`, in the return state (0045
     item 3, question 104). Its loads are contract reads, framed by the atoms before it in evaluation order:
     `owns(*pp)` or `reads(*pp)` before `owns_block(*pp)`, and `owns(g)`, `reads(g)` or the implicit atom of item 5
     before `owns_block(g)`. An unframed load is `permission` in `f__wf`, as in 0045 item 4.
   - **Partners.** An `ensures` site is the partner of the `requires` site whose entry place its return place equals
     (0045 item 7's object rule, here without an amount). The three cases:
     - **consumed**, a `requires` site alone: the callee takes the block, and the caller's cell keeps a dangling value;
     - **given**, an `ensures` site alone: the callee must hold the whole block at the return. There is no amount (0052
       item 2), and the caller receives the block. This is how `owns(**pp)` of a block allocated into `*pp` is handed
       over: `owns(*pp) && owns_block(*pp)` in the `ensures`;
     - **borrowed**, both sites.
   - **Bodiless contracts** (audit round 1, B1; extends 0045 item 5 and 0052 item 2a to block sites). Nothing proves a
     bodiless `ensures`. A borrowed site whose place loads, or rests on a global, can name a different block on return
     than on entry. `ensures(owns(*pp) && owns_block(*pp) && *pp == q)` with the caller holding `q`'s block would give
     the caller a second token for that block, and everything after the call would verify. So, in a function without a
     body:
     - **The tie.** For each `ensures` site whose place loads or rests on a global, the deep atom of item 5 included,
       `f__wf` asserts that it names its partner's entry object: `prior ==> (g_ens ==> E_post == E_partner_pre)`. This
       is a `permission` error: "the contract of `f` returns the block of `*pp` without saying it is the one its
       `requires` took (write `*pp == old(*pp)`)". Each pair of `ensures` block sites, one of which loads, gets 0045
       item 5's distinctness assert, as an assert and not a fact.
     - **Partnerless sites.** A partnerless site on a place reached from a formal or a global stays rejected
       (`unsupported_annotation`, naming M6's contract manifest; 0052 item 2a, which named formals only).
     - **The result.** Sites through the result keep 2a's allow-list (fresh results only).
   - **Keys** (the survey's coupling risk; audit item 12). Every consumer of a block site's identity takes the site's
     place key and lowers its pointer through 0045's `site_root`. No consumer reads a key as a formal's index. The
     consumers are:
     - the argument at a call;
     - the marker and the mode;
     - partners and consumption;
     - returned owners, which read an `ensures` site's pointer in the post-state, framed;
     - framing order (`first_block_site`);
     - entry checks and `block_entry_checks`;
     - the leak exemption;
     - 0047 item 9's sequencing registration, which takes 0045's pre-call snapshot (question 111) for a site through a
       load;
     - the back-edge leak trigger and the havoc rule;
     - `f__wf`'s partner rows;
     - `check_program`'s `block_atoms`, `operand_key` and `init_markers`;
     - `restrict_backed` and `restrict_hazard`;
     - `apply_owning`.

     A re-keying commit that changes no output comes first (`vprdiff`: none differ).
   - **`restrict`** (audit item 13). A site through a load does not back `restrict` on its root formal, because its
     block is not the formal's pointee. Only a site whose place is the formal itself backs it (0052 item 6(d)). A site
     rooted at a global counts in `restrict_hazard` like an atom on that global.
   - **The leak exemption** at the end label (0047 item 7) reads each `ensures` site's pointer in the post-state, framed
     by the atoms before it, as the site guard of 0052 item 6(b) does.
   - **Calls and returns.** A site through a load at a call is 0047 item 4's rows on the pointer the callee's place
     designates. That pointer is read in the caller through the call's arguments, or through the global for a global
     root. The separation rows of 0047 item 4 hold between every pair of block sites and between a block site and every
     `owns`, `reads` or `live` site (`lvloc` rows).
   - **Loops.** A `requires` site through a load is an automatic loop place, through 0045 item 10's `eb`/`lb` snapshots,
     with 0047 item 10's token place. Because the place is keyed on its entry snapshot, a loop that replaces `*pp`'s
     block hits `heap_loop_replace_false_alarm.c`'s limit (audit note 18); a written invariant (item 4) is the way out.
     The back-edge leak trigger of 0047 item 7 also counts calls whose callee gives a site through a load or carries a
     deep atom of item 5 (audit note 17).
   - **The dormant holder stays dormant** (S39 (h)). The holder `loop_places` builds for a formal with a block site is
     never reached today, because `fa.sites` holds no block site. The local copy's heap place already gives the same
     case splits. `heap_loop_param_block.c`, `heap_loop_call_borrow.c` and five probes verify today. Activating the
     holder would add only redundant `if (…) { }` splits, so it is left unreachable, and no existing Viper changes.
   - **Still rejected:** a block site under `old` or in a measure. Neither holds a permission.

3. **Loop carriers keyed by the current state** (amends 0052 item 3, Loops; S39 (d)). There are three parts, and all
   three are needed. *(Pending S39 part 2, audit round 1 B3: 0052 item 3 recorded that a current-value guard loses the
   distinctness of two blocks after the loop. The auditor predicts that (a) as written regresses
   `owning_loop_swap.c` and `owning_loop_replace.c`, which verify today. The spike re-encodes every existing owner-loop
   test, and (a) is rewritten from its result. It may keep the entry snapshot as a disjunct, or add an exclusivity
   clause.)*
   - **(a) Guards on current values.** The carriers are ordered:
     - first the full carriers, which are the owners the loop assigns and the owners it does not name (c), in
       declaration order;
     - then the keyed owners and the other places (audit item 6).

     Carrier `i` is guarded by `o_i != null && !(o_j != null && block_of(o_j) == block_of(o_i))` for every earlier
     carrier `j`. These guards replace 0052's loop-entry snapshots (`ad_`, `dup_`) and its pair rows ("owning `v` takes
     a block this loop already carries through `w`"). A block that becomes shared moves to the earlier carrier; one that
     stops being shared moves to the remaining one.
   - **(b) The value set.** For each owner the loop writes, a ghost `ev_o := o` is taken at loop entry and never
     assigned, and the invariant gains `o == ev_o || o == x_1 || …`. Each `x_k` is the value of a write to `o` in the
     loop that is `NULL` or a local pointer the loop never writes and that is declared outside the loop. A write is any
     write to `o` (audit item 4):
     - an assignment, including one inside an expression;
     - a store through a pointer that may equal `&o`;
     - a call whose contract may change `o`: through `&o` for an address-taken owner, or `owns(g)` for an owning global.

     If any write's value is anything else (a call's result, an owner the loop writes, an expression, a local declared
     in the body), no value-set clause is emitted for `o`. Without the clause, an owner's value after the loop is
     unconstrained (S39 `d1`: the loop verifies, but the return row fails).
   - **(c) Owners the loop does not name.** When the loop writes an owner or contains a call, every owning local in
     scope is a **full** carrier, even one the loop never names. A keyed place cannot take a block over: its flag-kept
     clause is not preserved (S39 `d3c`). That carrier is guarded by `held_o` (audit item 5): its entry row is `held_o
     ==> whole block`, so an owner freed through an alias before the loop and left non-null does not newly fail at the
     loop's entry.
   - **Results** (S39 `d1b`, `d3a`; 5–11 s each under load):
     - `owning_loop_becomes_shared` and `owning_loop_stops_shared` verify as written;
     - a block dropped at the back edge or by a reassignment stays `leak`;
     - `owning_loop_alias_becomes_owned` verifies only with a user invariant (`i < 3 || v != NULL`), because if the loop
       ended with `v` null, `w`'s block would leak (S39 `d2a`, `d2b`). The record states that limit. Its pin is retired
       for that positive (question 180).
   - **The cost** is O(n²) guard literals in the number of owners in scope. Programs with three or more owners in one
     loop are timed in Part B.
   - **A limit** (S39 `d2d`). After `v = w;` hands a block to the owner, a read of `*w` in a later iteration is
     `uninitialized`, because the owner's carrier carries the cells' `flag ==> range` but not the flags themselves.
     Part B tries flag-kept clauses per block under each carrier's guard; if none verifies, it
     is pinned as a documented false alarm for M4.8 (question 181).

4. **Written `owns_block` in loop invariants** (0052 item 6(a); audit S10).
   - **Form.** `CERASTES_INVARIANT(p == NULL || owns_block(p))` and its forms through loads and globals (`owns_block(s->q)`,
     `owns_block(g)`).
   - **The carrier.** A whole-block place keyed on the **current** value: the token with its value, full liveness, every
     cell, 0047 item 4's block facts and `flag ==> range`. It is the same carrier shape as an owning loop variable's
     (0052 item 3).
   - **Rows.** Entry and back-edge ending rows, written `if (!c) { assert false }`, report a missing block as
     `permission` at the loop, never as Silicon's own invariant check.
   - **Guards.** NOTAUTO guards keep it apart from the heap, parameter and cell places (0040 item 5).
   - **Exemption.** Its token term joins the back-edge leak exemption (0047 item 7).
   - **Merging.** A written site on an owning loop variable's value merges with that variable's carrier: one carrier per
     block. The merged carrier keeps the written site's meaning, so it is **not** guarded against earlier carriers
     (audit item 7). The current-value guard would remove exactly the distinctness the user wrote.
   - **Pairs** (S39 `c1x`, `c1y`; audit item 7). Two written sites in one invariant get the row `p != null && q != null
     ==> block_of(p) != block_of(q)` (`permission`) **at entry and at the back edge**. The held rows are checks, not
     exhales, so without the pair rows two sites naming one block fail Silicon's own invariant check, on entry or on
     preservation (exit 3). The same pair rows stand between a written site and each owner carrier, an owning
     global's included, when the site names a different variable (a non-owning alias of an owner's block).
   - `heap_loop_swap_false_alarm.c` (no invariant) stays pinned. The same loop with the written invariant verifies.

5. **Owning and marked globals and `static` locals** (amends 0029 item 1, 0041 items 7, 10 and 12, 0047 item 7, 0052
   items 3, 4 and 8; question 174).
   - **Accepted** on a mutable global (0041 item 2) or a `static` local whose type is a pointer to an object type. Each
     declaration keeps its own attribute list, and the variable's list is their union (P75 (a)). EDG raises no mismatch,
     and the union may hold both `owning` and `ref_to_uninit`, so the encoder compares every declaration. These
     are rejected (`unsupported_annotation`, one pin each):
     - `void*`;
     - a pointer to a `const` or `volatile` type;
     - a pointer to a function;
     - a count (`owning(n)`, M4.5e1);
     - twice on one declaration;
     - a declaration of the global without the attribute beside one with it (`annotation_mismatch`, naming both).
   - **Initializer.** It must be absent or a null pointer constant. Any other is `unsupported_annotation`: "an owning
     global starts null; it owns only what it is given".
   - **The initial value** (audit round 1, B2). `program_start` is unchanged: 0041 item 3 decides by linkage what it
     states. A C tentative definition and a C++ global with external linkage have an unknown value, since another unit
     may set it. `program_start` gives no block, so `main` starts with `held_g := false` for every owning global and
     owning `static` local, whatever the value. Where the value is unknown, `main` must assign the global (`g = NULL;`
     or an allocation) before it hands it to a callee whose contract has the deep atom. Otherwise the call is
     `permission`: "calls `f`, which needs owning global `g` to hold its block or be null". `if (g) free(g);` at
     `main`'s start is `invalid-free` there, not dead code.
   - **The deep atom.** Every `owns(g)` of an owning global carries item 2's site on the global's value:
     `owns(g) && (g == NULL || owns_block(g))`, in both lists, as one atom. This covers:
     - a written atom;
     - the implicit `owns(s)` of a `static` local (question 80).

     `program_start` and `main` never carry it (above).

     A function that only reads the global writes `reads(g)`, which gives the value and no part of the block. A read of
     `*g` needs `reads(*g)` or `owns(*g)` after it (0045). `owns(*g)` beside the deep atom names a cell of the block,
     so it is rejected as "beside" (0047 item 4).
   - **In the body:**
     - **Ghosts.** The ghosts are those of 0052 item 3: `held_g` and `tv_g`. At method entry, in a function whose
       `requires` holds the deep atom, `held_g := g != null`, and `tv_g` is read from the token, both under the deep
       atom. In a function that holds `g` only through `reads(g)`, or not at all, `held_g := false`.
     - **Stores.** At every store to `g` by name, including one inside an expression, the value rule (`permission`) and
       the reassignment row (`leak`) of 0052 item 3 apply, reading the old value from the cell. After every call and
       every `free`, `held_g` is updated as for an owning local.
     - **Updates are framed** (audit item 8). The update reads `g`'s cell only where the function still holds it:
       `held_g := held_g && perm(cell_g) > none && g != null && perm(tok(block_of(g))) > none`, chain-guarded as in
       0045. After a callee consumed `owns(g)` (a `requires` atom with no partner), the cell is gone and `held_g`
       becomes false without a read.
     - **Stores through an alias** (audit item 9). A store `*q = x` through a `T**` that may equal `&g` gets no guarded
       row. In a function without `owns(g)`, such a row would be a false alarm, because nothing proves `q != &g`; in a
       function with it, separation makes the row vacuous. Such a store is still caught, at a later position:
       - the old block, still held and now unowned, is a `leak` at the end label;
       - a value that is not a held whole block fails the deep atom's return row.
     - **Owner lists.** The owning globals the function holds join the owner lists of every ending row (by their current
       value).
   - **Returns.** The deep atom's `ensures` site is checked by item 2's return rows. A function that frees the block and
     leaves `g` non-null fails: `permission`, "returns while owning global `g` does not hold its block: set it to
     `NULL` after freeing it".
   - **Calls.** A callee whose contract has `owns(g)` takes the block with the cell (item 2's rows) and returns what `g`
     holds after the call. Then `held_g := g != null`. S39 (b) verifies `init()` called twice, and its twins fail at
     the reassignment row, the return row, the value row and, for a non-owning global, `main`'s leak row.
   - **`main`** (question 174; amends 0047 item 7 and question 123's reading for owners). `main`'s leak row exempts, for
     each owning global and owning `static` local `g`, the term `held_g && t == loc(block_of(g), −1)` (audit item 8).
     The term is guarded by `held_g`, and `g`'s cell is read only under that guard. Every path that sets `held_g`
     holds the cell, and a consuming callee clears it (above), so the read is framed. A block reachable only from a
     non-owning global is still a `leak` at `main`'s end. Every other function returns the block through its `ensures`
     atom on `g`, or frees it.
   - **Loops.** An owning global the function holds is a loop place (0041 item 10). Its block is carried by the current
     value (item 3's carrier), with `held_g` loop-carried.
   - **Marked globals and `static` locals** (`ref_to_uninit`, amends 0052 items 4 and 8).
     - In the default mode the marker documents intent, and the Viper does not change.
     - Under `--strict-init`, a store to a marked global follows 0052 item 4's rule for a marked local. It may receive
       only a null pointer, the result of a `ref_to_uninit` call, `&x` of an `uninit` local, or another marked
       variable or parameter.
     - Its initializer must be absent or null.

6. **Owning address-taken locals** (amends 0052 items 3 and 8; S39 (f) verifies the shape and its three twins).
   - The ghosts and rows of 0052 item 3 apply, with the value read from the local's cell (0032).
   - **Stores.** Rows are written at stores by name, and at stores through any pointer of the owner's type `T**` that
     may equal `&v` (guarded by `q == &v`, path-precise).
   - **`&v` passed to a callee:** the callee's contract decides.
     - With `owns(*pp)` alone, the callee may change `v`. After the call, `held_v := held_v && perm(token) > none`, and
       `v`'s ending rows check its new value.
     - With `owns(*pp) && (*pp == NULL || owns_block(*pp))` in both lists, the callee borrows the block (item 2).
     - With an `ensures` site on `*pp` alone, the callee gives a block (item 2), and `v` owns it: after the call,
       `held_v := v != null && offset_of(v) == 0 && perm(tok(block_of(v))) == write`, as item 5 does for `owns(g)`
       (audit item 11). Then `tv_v` is read from the token. The `&&` form above would leave `held_v` false and report
       the received block at a later row.
   - **Positions.** A callee given `&v` with `owns(*pp)` alone that sets `*pp = NULL` while `v` holds a block is
     reported `leak` at the caller's end label, not at the call (S39 `f2x`). The message names the block, not the call.
   - **The IL** (P75 (e)). `&v` stays an `eok_address_of`, while `&g` folds to a `ck_address` constant. A store `*pp = x`
     shows only `pp`, so which owner it may write is the lowering's question (the guarded rows).
   - This item is the plan's first pre-agreed cut.

7. **The static struct leaf cover** (amends 0052 item 2; audit 2 N1; S39 (e)).
   - An implicit `owns(s)` of a `static` struct covers an `owns`, `reads` or `live` atom on a leaf place of `s`
     (`ensures(r, owns(*r))` returning `&s.x`): `cov := res == leaf_0 || res == leaf_1 || …`, over the leaves whose
     type is the atom's pointee type only (audit item 14). A leaf of another type in the disjunction would give the
     caller a case it cannot frame, a `permission` false alarm.
   - **Why the d2b1 attempt failed.** The atom's value facts (`res.d_int && range(res.v_int)`) stood outside the cover's
     guard, and a disjunctive cover cannot frame them: the contract's well-formedness failed, as Silicon's own check.
   - **The working shape.** The facts stand under `!cov`, the separation rows are guarded by `!cov`, and `o := cov ?
     write : …` as in 0052 item 2. The caller then reads and writes through `*r` by its own implicit atom on `s`.

8. **The lint** (`lint_viper.py`):
   - token reads in block sites through loads;
   - the carrier ghosts of item 3 and their rows;
   - `main`'s exemption terms, admitted only as `held_g && t == loc(block_of(<the global's cell value>), −1)`, and only
     for a `GB_` block the program declares owning. The lint reads only Viper (audit note 19), so `program_start` carries
     a tag line `// cerastes:owning-global GB_<name>` per owning global and owning `static` local. The lint pins the tags
     against the `GB_` declarations, and admits an exemption term only for a tagged block.

9. **Categories:** none new. `leak` and `permission` gain the messages of items 5 and 6.

10. **Still rejected** (fail closed; one pin each, the message naming the session):
    - owning fields: EDG's #1835 until M4.5d2b2b registers target `d`;
    - P4418R0's attributes, which are unregistered (M4.5d2b2b);
    - a non-null initializer of an owning or marked global;
    - `owning` and `ref_to_uninit` on one variable unless every declaration carries both (`annotation_mismatch`, P75
      (a): EDG unions the lists without a diagnostic);
    - a block site through a load, or one resting on a global, in a contract without a body when its `ensures` site
      has no partner (item 2);
    - a block site under `old` or in a measure;
    - an owning global or `static` local in a `pure` or `predicate` function (0041 item 12).

11. **Facts relied on:** P75 (`internal/il-inventory.md`) and S39 (`build/m4.5d2b2a/s39/README.md`, with its kept
    fixtures in `tests/viper-smoke/memory/s39_*`).

12. **Tests and documentation.** Must-verify and must-fail pairs for every accepted form, in both modes where valid
    (plan, "Tests"), and the flips of the plan's list. Documentation: `memory.adoc`, `annotations.adoc`,
    `error-messages.adoc`, `verifying.adoc`, the tutorial's heap chapter and `internal/encoding.md`.

## Consequences

*(Written when the record is accepted.)*
