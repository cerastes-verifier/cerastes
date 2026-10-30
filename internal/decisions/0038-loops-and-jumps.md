# 0038 — Loops and jumps over memory (M4.5b1c1)

- **Status:** accepted for M4.5b1c1 (2026-10-29), written before code (invariant 6) and revised after P59, S28, audit
  round 1 (Opus, on the design: five blocking items, B1 a loop condition's checks as invariant clauses, B2 no case split
  for a pointer the loop assigns, B3 the range of an uninitialized cell, B4 a `return` inside a loop, B5 a method
  measure's reads; eight should-fix items and six notes; the blocking items and the should-fix items of the parts this
  session keeps are taken, the others go with the cut parts) and audit round 2 (on the code: one blocking item, a
  guarded parameter place not separated from a cell, and two should-fix items, all taken). Owner decisions (M4.5b1c
  planning and this session, 2026-10-29; plan `.claude/plans/m4-5b1c1-loops.md`):
  - **b1c is three sessions:** b1c1 (this record) loops and jumps; b1c2 sequencing on objects and the call placement
    of 0028 item 13, `return &local;` rejected like the compilers (handoff question 65), `c ? owns(*p) : reads(*p)`
    path-precise (question 67); b1c3 `T**`, `[[cerastes::owning]]` on parameters, `*result` as a value, `swap.c` and
    the aliasing trap.
  - **Owning results move to M4.5d2:** `[[cerastes::owning]]` on functions (target `r`), `owns_block`, `owns(*result)`
    and an `ensures` atom with no `requires` partner. `owns_block` needs the heap token and a whole-block quantified
    permission (0028 item 6), and nothing can create such a block before `malloc`.
  - **Loop frames are "every place held"** (choosing among readings of the 2026-10-28 hybrid rule, 0032's status): a
    loop that touches memory carries every cell in scope and the place of every pointer parameter the function never
    reassigns, with the permission held at entry; a place keeps its value only when no write in the loop can reach it
    (a write through a pointer of its type, or a call whose contract owns a cell of its type, havocs every place of that
    type); written `owns`/`reads`/`live` in `CERASTES_INVARIANT` are needed only for pointers the function reassigns.
  - **S28 F3 / audit B4:** a `return` (or an outward `goto` that leaves a cell's scope) inside a loop gives the loop the
    automatic invariant whenever the function holds a place; b1b reported a false `permission` there.
  - **S28 F1, F2:** the range row is guarded by the cell's flag and the flag snapshot is taken only when held (item 3);
    after a loop that first initializes an outer cell, a read needs the cell's value in the invariant (the flag is only
    known to stay true).
  - **Cuts** (the plan's stop rules, taken at the record's revision): function measures that read memory (item 8) move
    to b1c2, and written atoms in `CERASTES_INVARIANT` (item 5) to b1c3; both stay rejected.
- **Milestone:** M4.5b1c1 (2026-10-29).
- **Implements:** record 0028 items 4 (scope exits at every way out) and 8–9 (frames from permissions) for loops and
  jumps; the automatic half of 0032's hybrid rule.
- **Amends:** 0012 item 8 and 0014 item 4 (conditions that touch memory; scope exits on jump paths), 0019 item 4 (flag
  snapshots for cells), 0028 item 4, 0029 item 1 (target `r` registered in M4.5d2), 0032 items 4, 8 and 10, 0037 items
  1, 5 and 15.

## Context

Record 0032 rejected every loop that uses a pointer, `&`, `*` or a cell, every `return` inside a loop while a cell is
in scope, and every `break`, `continue` or `goto` that leaves a block declaring a cell (item 10), because a Viper loop
body sees only its invariant: a cell used there would fail its permission check, and a jump's scope exit was not
written. Record 0037 added pointer parameters with contracts, whose places a loop must also be able to reach. This
record gives loops their permissions and jumps their scope exits. Facts: P59 (the IL of memory in loop annotations) and
S28 (Silicon's loops with permissions, frames at out-edges, scope exits at jumps, measures that read the heap), in
`internal/il-inventory.md`, "M4.5b1c1".

## Decision

1. **Scope.**

   | | b1c1 (this record) | b1c2 | b1c3 | M4.5d2 |
   |---|---|---|---|---|
   | Statements | loops over memory; `break`, `continue`, `goto`, `return` out of a cell's scope | writes to cells inside expressions | — | — |
   | Annotations | automatic loop invariants; memory in invariants and loop measures | a function's `decreases(*p)` (item 8) | `owns`/`reads`/`live` in `CERASTES_INVARIANT` (item 5); `[[cerastes::owning]]` on parameters; `*result` as a value | `[[cerastes::owning]]` on functions; `owns_block`; `owns(*result)`; `ensures` atoms with no partner |
   | Checks | iteration checks of permissions; invariant and measure framing; scope exits at jumps | `unsequenced` (0028 item 12); 0028 item 13's placement; `return &local;` (question 65); question 67 | `T**` | — |

   Amends 0037 item 15's "lifted by" column: a memory call beside another access, or two in one full expression, and a
   heap read in a function's `decreases` → b1c2; `*result` as a value → b1c3; an atom on `*result` and an `ensures`
   atom with no `requires` partner → M4.5d2. Amends 0029 item 1's registration phasing: target `p` in b1c3, `r` in
   M4.5d2. Everything outside b1c1 stays rejected with a diagnostic naming the session that lifts it (item 10).

2. **Loops over memory** (lifts 0032 item 10's loop row). Every loop form (`while`, `for`, `do`) may use pointers, `&`,
   `*` and cells in its condition, body and step, and cells, `*q`, `&x` in its invariants and measure (item 6). A cell
   declared in a loop body is allocated when the body's block is entered, so each iteration has a new block (0032 item
   4, C23 6.2.4p6), and its scope exit is at the body's closing brace (0032 item 8).
   - **A condition that touches memory** (a pointer node, a cell, `&`, `*`) is lowered in 0020 item 7's top-test shape:
     `while (true)` with the condition, its checks as asserts and the exit test at the top of the body (after item 3's
     case splits), as for a condition with a side effect (audit B1: Viper refuses `perm` in an invariant, so a
     permission or liveness check cannot be an invariant clause, and a pointer `?:` there is a branch). The invariants
     and the measure stay where C evaluates the condition (0014 item 4). No invariant or measure clause ever holds `perm`.
   - **`do … while (0)`** stays straight-line code (0014 item 5): its invariants are asserted once like a `check`, in
     code mode (0032 item 4), and atoms there are rejected (item 10).

3. **The automatic invariant** (owner: "every place held").
   - **When.** A loop **touches memory** when its condition, body, step, invariants or measure hold a pointer-typed
     node, a cell, `&`, `*` or a store; and also when the function holds a place (item 3's places are not empty) and
     the loop's body holds a `return` or a `goto` to a label outside the loop (S28 F3, audit B4: their scope exits and
     the `ensures` permission checks of 0037 item 10 run inside the Viper loop body, which sees only the invariant).
     A loop that does not touch memory gets no automatic invariant: the frame keeps every place with its value (S28 P1:
     Silicon merges the frame back at every out-edge), nothing in the loop reaches one, and the Viper of every program
     accepted before b1c1 is unchanged (`cell_across_loop.c`).
   - **Places.** Every cell of every open block at the loop (`fn_.scopes`: a cell declared later in an enclosing block
     is allocated at that block's entry, 0032 item 4; `&param` copies included), and, for each pointer parameter the
     function never assigns, the places its `requires` atoms name through the formal: the value and flag cells of
     `owns(*p)` and `reads(*p)`, and one liveness cell `lvloc(block_of(p))` per parameter with an atom. A parameter the
     function assigns and a pointer local give no automatic place: what they point to is reached through the automatic
     places, or not at all in b1c1 (a written atom, item 5, is b1c3's).
   - **Entry.** Right before the Viper loop (after a `for` loop's init): the case splits `if (p_i == q) { }` for each
     automatic parameter place `p_i` against every other pointer parameter `q` with an atom of its field, an assigned
     one included (its atom's formal may alias an automatic place's), and `if (lvloc(block_of(p_i)) ==
     lvloc(block_of(q))) { }` for the liveness cells likewise, and each parameter place against each cell (`p_i == x` for
     a cell of its field, the liveness cells for a cell of any field; audit round 2, B1) (S28 P3, and found end to end:
     Silicon must branch before the snapshots, or it refuses their exhale); then the snapshots, ghost
     `Perm` locals `kv := perm(x.v_T)`, `kd := perm(x.d_T)`, `kl := perm(lvloc(block_of(x)).lv)` (`perm` of a null
     receiver is `none`: the audit's e-runs), the flag snapshot `ds := false; if (kd > none) { ds := x.d_T }` and the
     value snapshot `if (kv > none) { vs := x.v_T }` (S28 F2).
   - **Conjuncts,** before the user's invariants, each an `internal` row: `acc(x.v_T, kv) && acc(x.d_T, kd)` and the
     liveness `acc(lvloc(block_of(x)).lv, kl)`; for parameter place `i` of a field, the guard `p_i != null && AND_{j<i}
     p_i != p_j` over the earlier parameter places **of the same field (cell suffix)**, and for its liveness cell
     `p_i != null && AND_{j<i} (p_j != null ==> lvloc(block_of(p_i)) != lvloc(block_of(p_j)))` over the earlier
     parameters with an atom **of any pointee type** (one block's places share a liveness cell). Cells are fresh blocks:
     they never coincide with each other, nor with a parameter's place while the function holds part of it; on a path
     where it holds none (an atom whose guard is false, `c == 0 || owns(*p)`), Silicon cannot tell `p` from a cell, so
     the guards also hold `p_i != x` for every cell `x` of the field and `lvloc(block_of(p_i)) != lvloc(block_of(x))` for
     every cell (audit round 2, B1: without them a correct program exited 3; when `p_i` designates a cell, the cell's own
     conjunct holds it). The loop takes everything the function holds of
     each place; the frame keeps nothing of it.
   - **Values** (for a parameter place, each value, range and flag conjunct is under its value guard: the re-audit of
     round 2 found a cell written by name breaking a parameter's kept value on the path where `p` is that cell). A place
     keeps its value, `kv > none ==> x.v_T == vs`, when no write in the loop can reach it; otherwise
     only `none < kv && kv < write ==> x.v_T == vs` (a place held at less than `write` cannot be written: audit N3) and
     its range, `kv > none && kd > none && x.d_T ==> lo_T <= x.v_T <= hi_T` (no range for `bool`; guarded by the flag,
     S28 F1 / audit B3: `alloc_T` leaves an uninitialized cell's value unconstrained). A write can reach a place when
     the loop's condition, body, step or nested statements (nested loops, switch bodies, `do … while (0)` bodies) hold
     a store to the cell by name, a store through a pointer whose pointee has the place's **cell field** (VIR's suffix:
     qualifiers ignored, `long` and `long long` one field, an enumeration its own), or a call of a function whose
     `requires` has an `owns` atom on a pointer of that field (`const` pointees included: Viper havocs what a callee
     owns). The owner's rule: such a write havocs every place of the field; a sharper rule needs each pointer's
     provenance. The flag: `kd > none && ds ==> x.d_T` (0019 item 4's device, for cells: a flag only becomes true).
   - Every automatic conjunct holds by construction on entry and is preserved whenever item 4's checks pass; a failure
     of one is a cerastes bug (exit 3).

4. **Iteration checks** (0028 item 11: Silicon's own invariant checks never fail first).
   - **At the back edge** (the end of the Viper loop body: after the body, the `continue` label, a `for` loop's step and
     a `do` loop's exit test), for each automatic place, under its guard, one atomic check that the loop still holds what
     it held on entry: `perm(x.v_T) >= kv && perm(x.d_T) >= kd && perm(lvloc(block_of(x)).lv) >= kl`, category
     `permission`, "an iteration of this loop ends without the permission to `x` that it began with" (`*p` for a
     parameter's place), reported at the body's closing brace, or at the loop statement when the body has no position.
     A callee in the body that keeps an outer cell fails here, unless a later use in the same iteration fails first
     (S28 P2: the explicit check is the first and only error on its path).
   - On entry nothing can fail: the snapshots are what is held.
   - On a `break`, `return` or outward `goto` path Silicon checks no invariant (S28): a cell a callee kept there is
     reported at its own later scope exit or use, as without a loop.
   - Silicon's `invariant.not.established` and `invariant.not.preserved` stay `internal` rows on the automatic conjuncts.

5. **Written atoms in `CERASTES_INVARIANT`: cut to b1c3** (plan cut 2). `owns(*q)`, `reads(*q)`, `live(q)` in an
   invariant stay rejected (`unsupported_annotation`, naming M4.5b1c3). S28 P8 and the audit fixed what b1c3 needs:
   the guard `NOTAUTO(q)` by `Ref` for the value and flag and by liveness cell (`lvloc(block_of(q)) != L_i`) for the
   liveness, its own flag and range reads chain-guarded, case splits of `q` against every automatic place at the loop
   head, amounts and checks as a call's and a return's (0037 items 6, 8, 10) with `WF` and bounds as `permission` rows
   at the back edge, pairwise separation of two written atoms, and the place a reassigned `q` designates when the loop
   ends (P8's tie). Without them, a pointer whose place is not automatic is reported at its access, by the access's own
   check (owner's rule; 0032 item 6).

6. **Memory in invariants and measures.** An invariant or a loop measure may read a cell by name, `*q`, take `&x`,
   compare pointers with `==`/`!=` and use NULL. It is lowered in contract mode: `==` is identity in the model (0037
   item 12) and a pointer use has no liveness check. A cell or `*q` read keeps 0019's flag obligation, as an invariant
   (or measure) obligation clause, category `uninitialized`, "this loop invariant reads `x` before it is given a value":
   an invariant that states a cell's value states that it holds one, and that fact holds after the loop (found end to
   end: without it the owner's consequence below could not hold; audit N1 had proposed no flag check). `&x` in any
   annotation of a variable the code never takes the address of is rejected (`unsupported_expression`; P59: the front
   end does not flag it address-taken, so it is no cell, and b1a's lowering exited 3 on it; this also corrects 0032
   item 4's `check(&x != &y)` example, which holds only when the code takes both addresses).
   - **Framing.** Each heap read of an invariant or a measure must have, on its path, an automatic place of its field
     with a positive amount (`q == x_i && kv_i > none`; a cell read by name: `kv_x > none`). Right after the entry
     snapshots, and at the back edge after item 4's checks, lowering asserts it explicitly in evaluation order (`prior &&
     guard ==> OR_i D_i`, as 0037 item 7's `f__wf`), category `permission`, "this loop invariant reads `*q`, which this
     loop does not hold" (the function may hold it, through a parameter it reassigns, re-audit note 3; "… reads `x`,
     whose object a callee kept"; "this loop measure …"). S28 P4–P5: without it Silicon's own
     reason comes first.
   - In the invariant and the measure each heap read is guarded by its own `ite` chain over the disjuncts `D_i` (0037
     item 7's form; S28 P5: Silicon refuses a read under a disjunction and accepts the chain), so Silicon's
     well-formedness checks pass on every path; where the explicit check fails, the error is reported there.
   - A loop measure that reads memory is Silicon's loop `decreases` over the guarded reads (S28 P4).
   - **A pointer the loop assigns** is havocked at the loop head, and Silicon does not branch on a disjunction the
     invariant gives (`p == &x || p == &y`; audit B2). At the top of the body (before a top-test condition) and after the
     loop, lowering writes 0037 item 3's case splits `if (p == X) { }` for each pointer variable the loop assigns,
     against each automatic place of its field.

7. **Jumps out of a cell's scope** (lifts 0032 item 10's jump rows; amends 0014 item 4 and 0032 item 8). At a `break`,
   `continue`, outward `goto` (0015) or `return`, lowering writes 0032 item 8's scope exit for every block the jump
   leaves that holds cells, innermost first, cells in reverse declaration order, reported at the jump (`lifetime`, "the
   address of `x` may still be in use when its lifetime ends"). In the IL a `continue` leaves the loop body's block and
   every block inside it, a switch body's included (P59); a `break` out of a `switch` lands before the switch body's
   own scope exit (as since b1a), which runs once at the label, so the jump exits only the blocks nested inside the
   body. A `return` exits every open block (0032 item 8). S28 P7: the exit at the jump is the first and only failure of
   its path, innermost cell first.

8. **A function's measure that reads memory: cut to b1c2** (plan cut 1). `decreases(*p)` stays rejected
   (`unsupported_expression`, naming M4.5b1c2). S28 P6 and audit B5 fixed what b1c2 needs: Silicon reads the callee's
   measure in the current heap and the caller's in its entry heap (`old` must not be emitted); it never checks a
   method measure's well-formedness, so `f__wf`'s framing check (pre-state, 0037 item 7) is the only one and covers
   non-recursive functions; the method's `decreases` reads are chain-guarded, and 0021 item 4's explicit check follows
   the call's permission checks and case splits.

9. **The lint** (`lint_viper.py`; amends 0037 item 14): a `Perm` **local** is assigned only `perm(…)`, `none`, `write`
   or a conditional of those, never `wildcard`, and never inside a loop whose invariant uses it as an amount (audit
   S8: a havocked amount could exceed `write` and make the body vacuous); `acc` with such an amount is allowed in a
   loop invariant; `perm` stays refused in invariants (Viper refuses it); `inhale` and `assume` stay refused outside
   tagged lines.

10. **Still rejected** (fail closed; each pinned by a test):

    | Construct | Diagnostic | Lifted by |
    |---|---|---|
    | a write to a cell inside an expression; a memory call beside another cell access, or two in one full expression | `unsupported_expression` | b1c2 |
    | a heap read in a function's `decreases` | `unsupported_expression` | b1c2 |
    | `owns`/`reads`/`live` in a loop invariant, a `do … while (0)` invariant included | `unsupported_annotation` | b1c3 |
    | `T**`, the address of a pointer local; `*result` as a value | `unsupported_type`, `unsupported_expression` | b1c3 |
    | `owns(*result)`; an `ensures` atom with no `requires` partner | `unsupported_annotation` | M4.5d2 |
    | an atom in a loop measure | `unsupported_annotation` | — |
    | an atom in `CERASTES_CHECK`; in `assert` (code) | `call_in_annotation`; `spec_call_in_code` (as before) | — |
    | `&x` in an annotation of a variable the code never takes the address of | `unsupported_expression` | — |
    | a cell in a block without a closing position | `unsupported_statement` | — |

11. **Categories:** none new. `permission` gains the iteration and invariant-framing messages; `lifetime` gains the
    jumps (0011 item 10's five copies list messages only by category).

## Consequences

- Loops over memory verify without annotations when they only touch the function's own cells and its unassigned
  parameters' places; the user writes the values the loop changes (`x == i`), as for locals the loop assigns.
- A write through a pointer in a loop havocs every place of its field, so a loop that writes `*p` and reads another
  `int` cell needs that cell's value in its invariant (owner's rule; pinned by a must-fail test).
- After a loop that first initializes an outer cell, a read is reported `uninitialized` unless the invariant states the
  cell's value, which states that it holds one (item 6; S28 F2; pinned both ways).
- A parameter the function reassigns keeps its contract place outside every loop until b1c3's written atoms: an access
  to it inside a loop, and a `return` inside a loop whose `ensures` returns it, are reported (`permission`), a false
  alarm documented on the Memory and ownership page (audit N2).
- Each loop that touches memory carries one conjunct per place in scope; aliasing `reads`/`live` parameters add case
  splits like a call's (S28 P3: four parameters with a loop took about 50 s under load; record 0037, Consequences;
  M4.8 fixes their cost). Tests stay at three or fewer such parameters in a loop.
- Jumps out of a cell's scope report a callee that kept the cell at the jump.

*2026-10-29, amended by 0039 (M4.5b1c2): item 3 — writes to cells inside expressions are stores for "touches memory" and "a write can reach a place". Items 8 and 10 — a function's `decreases(*p)` is lifted (0039 item 8).*
