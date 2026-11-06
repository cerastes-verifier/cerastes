# 0052 — Partnerless results, owning locals, marked locals (M4.5d2b1)

- **Status:** accepted (2026-11-05, M4.5d2b1), written before code (invariant 6); revised after spike S38, audit round 1
  (`build/m4.5d2b/audit1.md`), audit round 2 (`audit2.md`), the re-audit (`audit3.md`) and its targeted check (`audit3b.md`).
  Owner decisions (M4.5d2b1 planning and spike S38, 2026-11-05; plan `.claude/plans/m4-5d2b1-results-locals.md`; handoff questions
  152–159, each the recommendation): 152 (M4.5d2b is two sessions; this record is d2b1's), 153
  (`cerastes_owns_vlibc_block`), 154 (a `ref_to_uninit` function's returns under `--strict-init`), 155 (`must_init`
  stays parameter-only), 156 (a covered atom's output amount is `write`), 157 (a covered `live` gives pointer
  use only), 158 (partnerless atoms in contracts without a body: fresh results only), 159 (only implicit atoms
  cover). Revised after audit round 1 (`build/m4.5d2b/audit1.md`: B1–B5, S1–S11, N1, N2, N7 taken in items 2, 2a, 3, 4, 6
  and 7).
- **Milestone:** M4.5d2b1.
- **Amends:** 0029 items 1 (target `v` on automatic locals) and 2 (`ref_to_uninit` target `v`); 0037 items 6, 7 and 15
  (an `ensures` atom without a partner); 0038 item 3 (owning locals as loop places); 0042 item 8 (`--strict-init` with
  marked locals); 0045 item 7 (amounts follow objects, extended to atoms without a partner); 0047 items 4 (block sites
  without a partner, guards that read memory, `owning` beside a block site), 5, 8, 10 and 11 (written `owns_block` in loop
  invariants); 0050 item 3 (a contract spelling for vlibc's blocks).

## Context

- **Partnerless atoms.** 0037 item 6 gives each `requires` atom a ghost amount and each `ensures` atom its partner's;
  an `ensures` atom without one is rejected (0037 item 15; 0045 item 1 for `live(r.q)`, `owns(*r.q)`, `owns(**r)`; 0047
  item 4 for a block site on a formal). The caller then knows nothing about a pointer result but `WF(r)`
  (`static_local_return_address_use.c`). An output amount is the obvious form, but a callee can give only what it holds
  beyond its partnered returns (0045 item 7: "the callee cannot give what it was not given"), and a function that
  returns `&s` of its `static` local holds `s` only through its implicit `owns(s)` (question 80), which it returns.
- **Owners.** 0029 item 1 planned target `v`: a local or global holding `owns_block` of its pointee, with `leak` at
  lifetime end and on reassignment. The leak rows today are the `forperm` after the end label and the loop back-edge row
  (0047 items 7, 10); a block allocated in an iteration and kept in a variable declared outside the loop is a `leak` at the
  back edge, because the loop place is the block held at loop entry (`heap_loop_replace_false_alarm.c`). P4417R0
  [profiles.inval.owner] p5 agrees with 0029 for automatic owners (delete or transfer on every path); its static owners and
  parameter transfers are d2b2's (question 152).
- **Marked locals.** P4222R3 and P4416R0 require a local receiving a `ref_to_uninit` result to be marked; 0047 item 8
  rejects every such call under `--strict-init` until marked locals exist.
- **Facts:** P74 (`il-inventory.md`): EDG keeps `owning`, `ref_to_uninit` and `must_init` on a local's
  `source_corresp.attributes` in both positions and modes once registered with `v`; a leading attribute covers every
  declarator, a trailing one its own; `void*` lands like `int*`; no fork commit.

## Decision

1. **Scope.** This record: atoms without a partner (item 2); owning automatic locals (item 3); marked automatic locals and
   P4222R3's strict rules (item 4); the lint (item 5); the leftovers of 0047 and 0050 (item 6); vlibc blocks in contracts
   (item 7). Fail closed on the rest (item 8): d2b2 takes block sites through loads, owning globals, `static` locals,
   address-taken locals and fields, and P4418R0.

2. **Atoms without a partner: covered or given** (amends 0037 items 6 and 15, 0045 items 1 and 7, 0047 item 4; revised
   after spike S38 and audit round 1, questions 156–159). An `ensures` atom (`owns`, `reads`, `live`, a block site) with
   no `requires` partner of its kind and key is accepted, in a function with a body in the unit, on every place 0045
   accepts in an `ensures`: the result's pointee and members (`*r`, `r->x`), a struct result's members (`r.q`, `*r.q`),
   places through loads (`**r`, `**pp`), a formal's pointee, and a block site on a formal. Contracts without a body:
   item 2a.
   - **Covered** (question 159). Only **implicit** atoms cover: a `static` local's `owns(s)` (question 80) and an
     `owning` parameter's merged borrow (0040 item 3). `cov_k` is the disjunction, over the implicit `ensures` atoms `j`
     of kind `owns`, of `g_j && place_j == place_k` in the post-state: for `owns` and `reads` the value cell's place
     (`r == &s`), for `live` the liveness cell (`lvloc(block_of(…))`). Written atoms never cover: `&&` between written
     atoms stays separating, so the caller of `ensures(owns(*p) && owns(*r))` concludes `r != p`, as VeriFast's `&*&`
     lets it. The separation rows of a covered pair (0037 item 10, 0041 item 8) and `f__wf`'s distinctness facts for it
     are guarded by `!cov_k`; between two written atoms they stay.
   - **Order** (audit B5). In a method with a partnerless atom, the implicit `ensures` atoms are emitted before the
     written ones, so a cover frames the reads after it (`ensures(r, owns(*r) && *r >= 0)` on `where()`).
   - **Given.** Where `g_k && !cov_k`, `k` gives an **output amount** `o_k`, a ghost `Perm` result of the method (numbered
     after the `requires` amounts, so every partnered path is unchanged), with `ensures none < o_k && o_k <= write`
     before every user clause. Before the return rows (after 0045 item 6's pass 0, in clause order, chain-guarded: audit
     S3), `o_k := (!g_k || cov_k) ? write : (perm(L_k) − R_k) / M_k` (question 156; audit B1): `L_k` is `k`'s liveness
     cell, `R_k` the sum of the partnered return amounts naming that cell (0037 item 6's sums, with 0045 item 7's `ite`s),
     and `M_k` the number of given atoms on that cell (`Σ_j (g_j && !cov_j && L_j == L_k ? 1 : 0)`, so atoms on different
     cells each take their own cell's remainder). For `reads`, `o_k` is the least of that quotient over its value, flag
     and liveness cells (audit S2). The return rows check `g_k ==> (cov_k || none < o_k)` (`permission` at the return:
     "returns `*r` without holding it"), then the per-cell sums with the term `(g_k && !cov_k && X_k == X_i) ? o_k :
     none` (`owns`'s value and flag cells at `write`). A block site has no amount: where it is not covered, the callee
     must hold the whole block (0047 item 4's rows).
   - **The pure part** (audit S1). `g_k ==> pure_k` holds whether covered or given: `r != NULL`, `WF`, the flag and range
     of `owns`, and `offset_of(r) < len(block_of(r))`, each checked by a return row (`permission`, never `internal`: a
     partnerless place is not a parameter, a null or a cell's address).
   - **The method's postcondition:** `g_k ==> pure_k`, `g_k ==> (cov_k || …)` as the implication `g_k && !cov_k ==>
     (none < o_k && <the atom's permissions at o_k>)`, and `cov_k`'s equalities through the caller's case split; nothing
     is inhaled.
   - **The caller** receives `o_k` in a ghost local of a multi-target call (0044's form): Viper's call gives it the
     postcondition. The after-call result splits (0050's `result_splits`) cover the new places, so a later read or `free`
     of `*r` does not leave Silicon without a case; as built (audit 2 B3), `lvloc` case splits of a given place against the
     call's pointer arguments, the caller's cells and its `requires` roots follow every call that gives a `live` or
     `reads` atom. A given amount is an unknown fraction to the caller: after lending a place's liveness to a callee that
     keeps its partner and gives the place back through a partnerless atom, the caller cannot prove it holds the whole
     object again (`free` is `invalid-free`, a local's scope end `lifetime`). A callee hands back what it took by returning
     the partner (with `r == p`), never through a partnerless atom; partnerless atoms are for places the caller did not
     give.
   - **Framing and checks.** A partnerless atom frames the contract reads after it in evaluation order (0037 item 7, 0047
     item 4). 0039 item 7's partner-by-path check has no partner and does not apply; its exclusivity rows do. `owns(**r)`
     with no atom on `*r` before it is unframed (`permission` in `f__wf`). A partnerless atom on a root that also has a
     partnered `ensures` atom on the same place or one inside the other (`live(p)` beside a partnerless `owns(*p)`,
     `owns(*p)` beside a partnerless `owns(p->x)`, `owns(*pp)` beside a partnerless `owns(**pp)`) is rejected before
     lowering (`unsupported_annotation`: "a place a function was given cannot be promised again as a new one"; audit N7:
     `paths_nested`'s `internal_error`), and so is the same kind twice on one partnerless place. A formal's partnerless
     atom therefore needs a root the `ensures` returns nothing else of; `owns(**pp)` of a block stored in `*pp` waits for
     d2b2's `owns_block(*pp)`. `reject_ensures_atom_no_partner.c` (`live(p)` beside `owns(*p)`) stays rejected with this
     message.
   - **What a cover gives** (question 157). A covered `live(r)` gives the equality of liveness cells only (the model has
     no injectivity for `lvloc`), so the caller may use the pointer (copy, compare, pass it: 0028 item 3a) but not read
     `*r`, as `live` never gives access. A covered `owns(*r)` or `reads(*r)` gives the equality of the places (`r ==
     &s`), and the caller reads and writes through the atom that covers it.
   - **The `static` local.** `int *where(void) ensures(r, live(r)) { static int s; return &s; }` verifies: `live(r)` is
     covered by the implicit `owns(s)`, and its caller may use `r`; with `ensures(r, owns(*r))` the caller also reads and
     writes `*r` through its own implicit atom on `s`. Without the `ensures` (`static_local_return_address_use.c`), the
     copy stays `lifetime`, its message naming the clause. A member of a `static` struct (`return &s.x;` with
     `ensures(r, owns(*r))`) is not covered (the implicit atom covers the struct's place, not a leaf of it; covering a leaf
     broke the contract's well-formedness method): a documented false alarm ("returns `*r` without holding it"), pinned,
     for M4.5d2b2 (audit 2 N1).
   - **A written atom returned with its place.** `requires(owns(p->x)) ensures(owns(p->x)) ensures(r, owns(*r))` returning
     `&p->x` fails the separation row at the return; the callee that hands `p->x` over drops it from its `ensures` (it
     keeps it, 0037 item 13) and gives `owns(*r)`.
   - **A fresh block.** `owns(*r)` of a block the callee allocated is given (the callee holds full liveness), but the
     token stays with the callee and is a `leak` at its end: a block is handed over by `owns_block(r)`/`owning` (0047
     item 4), and `owns_block(*pp)` is d2b2's. `owns_block(r) && owns(r->balance)` stays rejected ("beside", 0047 item 4;
     audit N1): `owns_block(r) && r->balance == 0` already says it.

2a. **Contracts without a body** (question 158; closes a hole open since 0047 item 4, audit round 1 B3). A bodiless
   contract is trusted, and a partnerless atom in it gives the caller a permission its state may already hold: two write
   chunks on one cell make the caller's state impossible and everything after the call verifies
   (`void f(int *p) requires(live(p)) ensures(owns(*p));`, and since d2a `int *again(int *p) requires(live(p))
   ensures(r, owns_block(r) && r == p);` with a double free after it). In a function with no body in the unit, trusted
   library routines included, a partnerless atom is accepted only on the result or a place reached through it, and only
   where no clause of the contract equates the result, or a pointer value read through it, with anything but a null
   pointer constant: vlibc's `r == NULL || owns_block(r)`, `r == NULL || owns(*r)` and `r->x == 0` are accepted
   (Viper's call picks a place the caller does not hold); `owns_block(r) && r == p`, `r->q == p` and every partnerless
   atom on a formal are rejected (`unsupported_annotation`, naming M6's contract manifest).
   `reject_owns_block_ensures_no_partner.c` (bodiless) stays rejected with this message. As built (audit round 2 B1) the
   rule is an allow-list: in such a contract the result and every place through it appear only as an atom's place, in
   `X == NULL` or `X != NULL` with `X` directly such a path, or as a value that is not a pointer; `r != p ? r == NULL :
   owns(*r)`, `!(r != p)`, `(c ? r : r) == p` and the like are rejected. A bodiless declaration that nothing calls is not
   read (as today), so it is not checked; it cannot reach a proof (audit 2 N3).

3. **Owning automatic locals** (amends 0029 item 1's registration phasing; 0038 item 3; 0047 item 10; revised after
   audit round 1).
   - **Registration:** `owning` becomes `{"owning", "?(X)", "r|v|p", RAF_ENTITY_ATTACHED}`; `d` stays unregistered (EDG's
     #1835 on a field until d2b2).
   - **Accepted** on a pointer variable of automatic storage, not address-taken, of a pointer-to-object type that is not
     `void*` and not a pointer to a `const` or `volatile` type, in either position; each declarator carries its own
     (P74). **Rejected** (`unsupported_annotation`, one pin each): a global, a `static` local and an address-taken local
     (naming M4.5d2b2), a non-pointer, `void*`, a pointer to `const`/`volatile`, a pointer to a function (M4.7), twice,
     and a count (`owning(n)`, M4.5e1).
   - **Ghosts.** Each owning local `v` has two method-level ghosts: `held_v` (`Bool`, `false` at method entry and at `v`'s
     declaration, audit S4: a `goto` past the declaration or a loop's re-entry never reads a havocked `v`) and `tv_v` (the
     token's value: `true` for the platform's allocator, `false` for vlibc's, 0050 item 3; audit S7: an owning local may
     hold either allocator's block).
   - **The value rule.** At every initialization and assignment of `v` with `new`: `new != null ==>` the function holds
     `new`'s block whole — `offset_of(new) == 0` and the token cell at `write` — checked by a row (`permission`, at the
     initialization or assignment: "owning `v` receives a pointer that is not the start of a block it holds"); then
     `held_v := new != null` and `tv_v := tok(block_of(new))` read under the permission (0050 item 5).
   - **Dangling owners** (audit S5). After every call and every `free`/`vlibc_free`, `held_v := held_v &&
     perm(loc(block_of(v), −1).tok) > none` for each owning local in scope: once the block is consumed the owner holds
     nothing, even if a later allocation returns the same block.
   - **Owners** at a point: the owning locals in scope with `held_o`; at a `return`, the returned value if the function
     is `owning` or has an `ensures` block site on its result (audit N2); and the entry values of the formals with an
     `ensures` block site (audit S11: a borrowed block parked in an owning local).
   - **Lifetime end.** At each exit from `v`'s scope — its block's end, a `break`, `continue` or `goto` that leaves the
     block, and a `return` — the ending row (audit S6: `if (!(c)) { assert false }`, so one token is reported once) with
     `c` = `held_v ==> (held_o1 && v == o_1 || … || v == <a returned or formal owner>)`, over the owners that outlive the
     exit (`leak`, at the exit: "owning `v` still holds its block when its lifetime ends").
   - **Reassignment.** Before a write to `v`, the same ending row over its old value, with the new value counted among the
     owners (`v = v` holds) (`leak`, at the assignment: "assigning to owning `v` drops the block it holds").
   - **What is not changed.** The end-of-function `forperm` row stays and catches blocks without an owner. A non-owning
     copy (`int *q = v;`) does not take the obligation (P4417 p3), so `v`'s exit still fails if the block is held.
   - **Loops** (audit B2; S38 (f)). An owning local declared outside a loop that the loop assigns is a loop place keyed on
     its **current** value: the invariant `v != null ==> (held_v && <v's whole block: offset 0, the token at write with
     value tv_v, full liveness, every cell, 0047 item 4's facts>)`, with `held_v` and `tv_v` loop-carried and 0047 item
     10's entry-keyed place (`hb`) not built for it. Explicit entry and back-edge ending rows `v != null ==> held_v`
     precede the invariant (`permission`: "owning `v` holds no block at the loop's entry/back edge but is not null: set it
     to `NULL` after freeing it or handing it on"), so a violation is a user error at the loop, never Silicon's invariant
     check, and after the loop `v == NULL` or `held_v` with the whole block. The back-edge leak row exempts exactly `v !=
     null && t == loc(block_of(v), −1)`. A loop that allocates into a `NULL`-initialized owner every iteration fails the
     reassignment row in its second iteration; an owner freed on some iteration is set to `NULL` there (the invariant
     cannot carry "freed" otherwise). As built: the invariant states `held_v == (v != null)` (Viper havocs `held_v`, and
     the one direction let a swap of two owners fail); an owner the loop uses or assigns carries its whole block (a
     `HeapHolder`, not 0047 item 10's entry-keyed place), and one the loop neither uses nor assigns but whose body calls
     a function carries only `held_v` and its token (the call's `held_v` update reads the token's permission); beside
     the held rows, whole-block permission rows at entry and at the back edge make a lost cell a `permission` error at
     the loop, never an `internal` one; the token's value is `true` in a platform program and, in a vlibc heap program, a
     loop-entry snapshot with a row that fires if the loop swaps in the other allocator's block; where every assignment
     to the owner in the loop comes from one allocator's views (or is `NULL`), that allocator is the invariant's constant
     and a `permission` row at entry reports an owner entering with the other allocator's block (so a vlibc loop may start
     from a `NULL` owner). A non-owning alias of an owner's block keeps its 0047 item 10 place with guards that exclude
     the owner's block (both claimed the same permissions, an `internal` failure before). A user invariant or measure
     reading `*v` of a carried owner is framed by the owner's cells; a written atom on them (`owns(*v)`) is a
     `permission` error at the atom ("which the loop already owns through its owning local"). After the re-audit
     (`build/m4.5d2b/audit3.md`): an owner the loop uses but does not assign is **keyed** — its block is the variable's
     0047 item 10 heap place, which keeps its value and flag, and the loop carries only `held_v == (v != null)` and the
     held row (adding `owning` never makes a verifying read-only loop fail); two owners of one block are carried once —
     each later owner's carrier clauses, rows and exemption term are guarded by a loop-entry snapshot "not the block of
     an earlier owner" (a current-value guard loses the distinctness of two blocks after the loop); heap places are
     guarded against carried owners by entry snapshots likewise; a given atom's `lvloc` splits after a call cover struct
     result leaves and places through loads too. Because those guards are entry snapshots, a block that becomes shared
     inside the loop (an owner taking a block another carrier holds: `b = a;`, or `v = w;` with `w` a plain pointer to a
     held block) is reported by a `permission` ending row before the back-edge invariant ("owning `v` takes a block this
     loop already carries through `w`"), never Silicon's own check, and one that stops being shared (`a = NULL;` with `b`
     still holding it) is not picked up by the other carrier: both are documented false alarms, pinned, for M4.5d2b2
     (audit 3b). Programs without an owning local keep their Viper byte for byte.

4. **Marked automatic locals and P4222R3's strict rules** (amends 0029 item 2, 0042 item 8, 0047 item 8).
   - **Registration:** `ref_to_uninit` becomes `"r|v|p"`. `must_init` stays `"p"` (question 155: P4222 defines it for
     parameters; EDG refuses it on a local, #1835, and the pin says why).
   - **Accepted** on a pointer variable of automatic storage (address-taken included: the marker states what its pointee
     may be, not who owns it). **Rejected** (`unsupported_annotation`): a global or a `static` local (M4.5d2b2), a
     non-pointer, a `void*` (0042 item 4).
   - **Default mode.** The marker documents intent and changes no Viper: 0019's flags already follow the cell, and 0042
     item 7's call rows already report a pointee that may be uninitialized passed to an unmarked parameter.
   - **`--strict-init`** (P4222R3 §4.2, P4416R0 [profiles.init.ref]; audit S9):
     - a marked local may receive (initialize or be assigned) only a null pointer, the result of a `ref_to_uninit` call,
       `&x` of an `uninit` local, or another marked local or parameter; anything else is rejected
       (`uninit_ref_initialized`: "`p` is marked `ref_to_uninit` but receives `&x`, which may point to an initialized
       object"), so a marked local refers to storage that may hold no value, as the papers require;
     - a marked local or a `ref_to_uninit` call's result is accepted as the argument of a `ref_to_uninit` parameter
       (P4222R3's canonical `[[ref_to_uninit]] int *s = malloc(…); init(s);`);
     - the result of a call to a `ref_to_uninit` function initializes or is assigned to a **marked** local, is returned
       by a marked function, or is the argument of a `ref_to_uninit` parameter; anything else keeps 0047 item 8's
       rejection, now with P4222R3's message;
     - **returns** of a `ref_to_uninit` function are a marked local, a call to a `ref_to_uninit` function, or a null
       pointer constant (question 154; P4416 p6 counts null as initialized, which would reject every allocation wrapper's
       `return NULL;`); anything else is rejected;
     - a marked local passed to an unmarked parameter is checked path-precisely by 0042 item 7's row, in both modes:
       the flag follows the cell, so after `*p = 1;` it may be passed (the papers keep the marker on the pointer until
       `now_init`; cerastes accepts more, and needs no `now_init`); `free` and `vlibc_free` take any pointee (P4416
       leaves `free` unmarked);
     - a null argument to a `ref_to_uninit` parameter stays rejected (0042 item 8, P4222R3).

5. **The lint** (`lint_viper.py`): each ghost `Perm` **result** of a method has exactly `ensures none < o && o <= write`
   first (beside 0037 item 14's rule for `Perm` formals); an unassigned `o` is Silicon's own failure (exit 3), never a proof, so no rule pins it (audit 2 N4). A call's `Perm` target (`t, o := f(…)`) is allowed where the callee's `o` carries that row (S38: 0038 item 9's
   rule flags it otherwise). The owner rows read `tok` inside an `assert`, which the lint allows (0047 item 7).

6. **The leftovers.**
   - (a) **Written `owns_block` in loop invariants: cut to M4.5d2b2** (the plan's pre-agreed cut; the loop places'
     carrier for a written block site with its token value is about a session's work over 0047 item 10's machinery). It
     stays rejected, its pin `reject_owns_block_invariant.c` re-messaged; d2b2 takes audit S10's points (0040 item 5's
     NOTAUTO guards, one carrier with an owning loop variable, the written sites in the back-edge exemption). An owning
     loop variable (item 3) needs no written invariant.
   - (b) **Block-site guards that read the heap, a global or `old`** (amends 0047 item 4; audit 2 S2): framed and
     evaluated like 0045 item 11's atom guards. An `ensures` guard is read in the post-state, where the leak exemption
     (0047 item 7) repeats it (the places it reads are still held there, S38 (h)); a `requires` guard is read in the entry
     state and does not reach the exemption. A caller learns the block only where the guard holds after the call, so a
     contract states what keeps it (S38 (h): `ensures(*q == old(*q))` beside `*q > 0 ? owns_block(p) : true`).
   - (c) **`owning` beside a block site** on one parameter in both lists: merged into the block site, which gives
     everything `owns(*p)` does but its liveness fraction (`apply_owning` sees block sites); a block site in one list only
     beside `owning` stays rejected.
   - (d) **`restrict` beside a parameter whose only atom is a block site:** backed (`restrict_backed` counts block sites).
   - (e) **`f((free(p), 0), p)`:** `free`'s effect is registered on `*p`'s object, so it is `unsequenced`, not
     `permission`.

7. **vlibc's blocks in contracts** (question 153; amends 0050 item 3). `cerastes_owns_vlibc_block(p)`, declared by
   `<cerastes.h>` with its C name in both modes (no `cerastes::` spelling) and rejected in C++ like vlibc's allocations
   (0050 item 7), is `owns_block` with the token's value `false`; everything else in 0047 item 4 applies. `owns_block` and
   `owning` keep naming the platform's allocator. A contract may not name both on one root. A program that names it is a
   vlibc heap program (0050 item 4's gate; audit S8), so its token rows read the value under permission and the lint
   accepts `!tok`. vlibc's header does not change: `vlibc_free` keeps its contract, and a user function that passes a
   vlibc block to it states `cerastes_owns_vlibc_block` in its own contract.

8. **Still rejected** (fail closed; one pin each, the message naming the session): `owning` and `ref_to_uninit` on
   globals, `static` locals and (for `owning`) address-taken locals; owning fields (EDG's #1835 until d2b2 registers
   `d`); block sites through loads or on globals; written `owns_block` in loop invariants (item 6(a)); P4418R0's attributes (unregistered: EDG's warning #1097 or #1835);
   `must_init` on a local (permanent, question 155).

9. **Categories:** none new. `leak` gains the owner messages (item 3); `permission` gains the value rule (item 3) and
   the uncovered partnerless return (item 2).

10. **Facts relied on:** P74 (`internal/il-inventory.md`); S38 (`build/m4.5d2b/s38/README.md`, its fixtures kept in
    `tests/viper-smoke/memory/` where they pin a shape).

11. **Tests and documentation.** Must-verify and must-fail pairs for every accepted form in both modes where valid
    (plan, "Tests"); the flips of the plan's list; `memory.adoc`, `annotations.adoc`, `verifying.adoc`,
    `error-messages.adoc`, the tutorial's heap chapter and `internal/encoding.md`.

## Consequences

- A function can now hand its caller a place it holds (a global's cell, an interior pointer, a `static` local's
  address), and the caller learns where a pointer result points when the callee returns that place anyway.
- Owning locals report a leak where it happens (the scope exit, the assignment), and lift the loop false alarm for
  variables declared owning; unannotated programs keep their diagnostics and their Viper.
- `--strict-init` accepts P4222R3's marked locals; `malloc` into an unmarked local stays rejected in strict mode only.
- d2b2 inherits block sites through loads, owning globals and fields, and P4418R0.
