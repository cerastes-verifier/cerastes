# 0039 — Sequencing and placement over memory (M4.5b1c2)

- **Status:** accepted for M4.5b1c2 (2026-10-29), written before code (invariant 6), revised after P60 (C1–C3) and audit
  round 1 (Opus, on the design: three blocking items, B1 a snapshot's set without its guard, B2 one key per field
  joining the sets of unsequenced accesses, B3 missing must-fail tests; nine should-fix items, all taken, S7 by the
  owner's decision, item 7's last paragraph; notes 1–5 taken; `build/m4.5b1c2/audit1.md`) and audit round 2 (on the
  code: one blocking item, R2-B1, a write in the left operand of `&&`/`||`/`?:` not ordered before the right operand's
  reads, a wrong verdict; five should-fix items, all taken; one re-audit of the fix: no blocking item, three
  should-fix items, taken; `build/m4.5b1c2/audit2.md`). Owner
  decisions (M4.5b1c2 planning, 2026-10-29; plan `.claude/plans/m4-5b1c2-sequencing.md`; handoff questions 69–71):
  - **Unsequenced accesses that name one object are rejected like the compilers** (`unsequenced_modification`, 0020
    items 4–5's union rule); the shapes whose outcome depends on aliasing get the path-precise `unsequenced` check of
    0028 item 12 (question 70). P61 measured the split: every same-object shape warns, no aliasing shape does (item 3).
  - **No time cap and no cuts** (question 71); the upstream EDG merge waits for the box session (question 69).
  - **An `ensures` atom is returned only where its partner was taken** (question 72, after audit round 1's S7): checked
    in `f__wf` for every contract, trusted or not (item 7).
  - Earlier answers this record carries out: question 65 (`return &local;` rejected like the compilers; the escape
    through a local copy stays the caller's `lifetime` error), question 67 (the `owns`+`reads` rule path-precise).
- **Milestone:** M4.5b1c2 (2026-10-29).
- **Implements:** record 0028 items 10 (`unsequenced`), 12 and 13; record 0038 item 8 (a function's measure over
  memory); handoff questions 65 and 67.
- **Amends:** 0011 item 10 (`unsequenced`: eighteen categories), 0020 items 4, 7 (E3) and 8, 0021 item 2, 0025 items 2
  and 6 (R14), 0026 item 15, 0028 items 12 and 13, 0032 items 9 and 10, 0037 items 5, 8 and 15, 0038 items 1, 3, 8
  and 10.

## Context

Record 0032 item 9 let a cell be written only as the top-level operation of an expression statement, and record 0037
item 8 refused a call whose contract holds an atom beside any other access to a cell (fail closed: 0025 item 2's
placement of writes, and 0021 item 2's argument for calls, rest on callees not seeing the caller's locals, which cells
break). Record 0028 Part D designed what replaces both: sequencing on objects (item 12) and the placement of calls and
cell accesses (item 13). Record 0038 cut a function's measure over memory to this session (its item 8). Facts: P60 (the
IL of writes inside expressions, compound stores, returned addresses, conditional atoms, measures), P61 (the compiler
matrix of unsequenced accesses and returned addresses) and S29 (Silicon), in `internal/il-inventory.md`, "M4.5b1c2".

## Decision

1. **Scope.**

   | | b1c2 (this record) | b1c3 | M4.5d2 |
   |---|---|---|---|
   | Expressions | writes to cells inside expressions, by name and through pointers; compound stores whose address has a side effect | `T**`; `*result` as a value | — |
   | Checks | `unsequenced` (items 3–4); placement of cell accesses and calls (item 4); `return &local;` (item 6) | written atoms in invariants | — |
   | Contracts | `c ? owns(*p) : reads(*p)` (item 7); a function's `decreases(*p)` (item 8) | `[[cerastes::owning]]` on parameters | `owning` on functions, `owns_block`, `owns(*result)`, partnerless `ensures` atoms |

   Amends 0037 item 15 and 0038 item 10: their rows "a write to a cell inside an expression; a memory call beside
   another cell access, or two in one full expression" and "a heap read in a function's `decreases`" are lifted here,
   and "`owns(*p)` and `reads(*p)` in one clause list" becomes item 7's path-precise check.

2. **Writes to cells inside expressions** (lifts 0032 item 9 and its row in 0032 item 10).
   - **Accepted:** `=`, a compound assignment and `++`/`--` (prefix and postfix) whose target is a cell by name (an
     address-taken local, `&param`'s copy) or `*e` for a pointer expression `e`, wherever 0020 item 1 accepts an
     assignment used as a value: an operand, an argument, an initializer, a condition, a loop condition (0021 item 5),
     a `return` value. Never in an annotation. C++ lvalue uses (`++*p = 3`, `(*p = 1) = 2`) stay rejected (0020 item 3).
   - **VIR.** A cell by name is an `AssignE` whose `var` is address-taken; a write through a pointer used as a value is a
     new expression kind appended after `Atom` (`StoreE`: `*addr := a`, the value `a` or, when `yields_old`, the old
     value `b`; `b` is null or the one `Deref` node of `addr` that `a` also reads, so the pointer is lowered once, as for
     `Stmt::Store`). `vir.h` fixes the field names. `check_program` refuses either in an annotation (as `AssignE`).
   - **Lowering.** Each is a **write effect** of the full expression (0020 item 7): for `*e`, the null check
     (`null-dereference`); for both, the write permission `perm(r.v_T) == write && perm(r.d_T) == write` (`permission`,
     0032 item 6's and 0037 item 11's messages), obligations with the write's set; then `r.v_T := value; r.d_T := true`
     (a receiver that is not a variable is first copied into a `Ref` temporary). The node's value is the stored value
     as lowered, or the old value's snapshot (E4), never a re-read of the cell.
   - **E3 for memory** (amends 0020 E3). A read of a place of field `T` (a cell by name, `*e`) is a **snapshot effect**
     (after its null, permission and flag obligations, which keep their categories) when the full expression, branch
     bodies included, holds a write of a place of field `T` anywhere (this item; a statement's own top-level store
     does not count, audit S6), or a memory call on field `T` (item 4) of which the read is not an argument. It reads
     only on its path: `t := g ? r.v_T : 0` (`false` for `bool`), `g` its guard (audit B1: in
     `(p == NULL || *p > 0) && bump(q)` an unguarded read of a null `p` would be Silicon's own error). Otherwise the
     read stays inline, as today. Fields are VIR's cell suffixes (0028 item
     1: places of different fields never alias). A heap read today creates no effect and is evaluated where its consumer
     is emitted, which a write or a call of the same full expression could then precede.
   - **Keys and sets** (amends 0020 E2 and 0025 item 2 for cells; audit B1, B2, S3). Each cell access has a **key**:
     the variable of a cell by name, `*v` for a dereference of a pointer variable `v` (item 3's objects), and a key of
     its own for any other `*e`. A snapshot follows its guard's set (audit B1: C evaluates a guard before the operand it
     guards, and the guard may hold a memory call, `bump(&x) && x == 0`), the comma floor, its receiver's set and the
     sets of the earlier writes of its key. A write follows the comma floor, the stored value's and the receiver's sets,
     every earlier access of its key, and every fixed point created in its operands (the store follows the value
     computation of both operands, C23 6.5.17.1p3, 7.6.19p1 in C++). All of these hold in both halves of the set (0026
     item 3; audit S3: the base half of a comma's value drops the floor, so `y = (x = (bump(&x), 2))` would store
     first). Accesses with different keys are never joined (audit B2: joining by field would give a set larger than
     what C completes, `(*r = nr()) + (*q = 1)` hiding the null check of `*q` behind a call that does not return):
     where C sequences them, the comma floor or a used value orders them, as for two locals, and so does the left
     operand of `&&`/`||` or the condition of `?:`, which C completes, stores and calls included, before the right
     operand or an arm: when it writes a cell or holds a memory call, everything created while it was lowered is a
     floor, in both halves, for every item of the right operand or the arms that depends on the guard and for every
     branch there (audit round 2, R2-B1, and its re-audit: a guard's
     value set alone left `((*pick(q) = 5) == 5) && *q == 5` reading `*q` before the store, a wrong verdict); where C
     does not sequence them, item 3 checks that their places differ, and then either order gives one result. Item 3's static rule leaves no
     unsequenced conflicting pair of one key, so 0020 E2's argument holds per key. Writes and snapshots of Viper locals
     keep 0025 item 2's placement.
   - **A call whose arguments write a cell** (audit S3): its permission checks and its heap-reading `requires` checks
     (0037 item 8) take PRE(c) in both halves, so they follow the write; a call whose arguments write no cell keeps
     today's sets (and its Viper).
   - **Loops** (amends 0038 item 3). A write of this item inside a loop's condition, body or step is a store for 0038
     item 3's "touches memory" and for "a write can reach a place" (by name, or through a pointer of the place's field).

3. **Sequencing on objects** (0028 item 12 with the owner's split; amends 0020 item 4).
   - **Static (rejected `unsequenced_modification`, like the compilers).** 0020 item 4's pair rule, with its sequenced
     operands and its stored-expression exceptions unchanged, counts two more kinds of object: a cell by name (its
     variable, as for a Viper local; `*&x` is `x`), and `*v` for a pointer variable `v`, one object per `v` as written
     (`*v` is written by a `Store`/`StoreE` through `v` and read by a `Deref` of `v`), whether or not the expression
     also writes `v` (audit S9: GCC compares the lvalues as written and warns on `(p = q, (*p)++ + *p)` and on C's
     `*&x = x++`). P61: every such shape warns at the no-flag or `-Wall` tier in some compiler and mode (clang
     `-Wunsequenced` on names, GCC `-Wsequence-point` on names and on `*p`, A1–A10), including shapes C++17 defines
     (`x = x++`, `*p = (*p)++`, `*p = ++*p`), so one rule for both modes, as 0020 item 4 has.
   - **Path-precise (category `unsequenced`).** Every other pair of accesses to places of one field, at least one a
     write, that 0020 item 4 calls unsequenced (operands of an arithmetic, shift or comparison operator; arguments of a
     call among themselves; a store's target against its value, with the stored-expression exceptions; the accesses
     inside a store's address against those inside its value, 0032 item 9, audit S1: `*(*q > 0 ? a : b) = (*r)++`
     with `q == r`), gets the
     obligation `g_A && g_B ==> place_A != place_B`, where a place is the cell's `Ref` variable or the pointer's value
     and `g` the access's guard (an access inside an arm of `?:` or a right operand of `&&`/`||` counts only on its
     path). Its set is the union of the sets of the two place values and guards (an ordinary obligation, 0021 item 3).
     When a place value or a guard is computed inside an E5 branch, the assert is emitted inside that branch, on its
     path, or after it with D(B) (audit S5).
     Reported at the later-created access: "modifies `*p` and accesses `*q` with no sequence point between, and they
     may be one object" (or "modifies `x` and `*p` …"). P61: no compiler warns on any of these shapes (B1–B6).
   - Places of different fields get no check (0028 item 1). A pair of accesses with a sequence point between them is
     ordered by the sets (item 2).

4. **Calls beside cell accesses** (0028 item 13; amends 0021 item 2, 0025 item 2, 0026 item 15; lifts 0037 item 8's
   placement rule).
   - **A memory call on field `T`** is a call whose callee's `requires` holds an `owns` or `reads` atom whose pointee
     has field `T` (`live` alone reaches no value or flag cell; liveness cells are written only by `free`, M4.5d2).
   - **A call and an access C does not order.** C sequences a call's arguments before it, the left operand of `,`,
     `&&`, `||` and the condition of `?:` before what follows, an operand before its operator (an access whose place or
     guard uses a call's value follows the call: `*first(&x)`, audit S2), and a store after the value computations of
     its operands; any other access and call of one full expression are indeterminately sequenced (C23 6.5.2.2p10).
     For each memory call `c` on field `T` and each access `A` of a place of field `T` that C does not order with `c`,
     and each atom `k` of `c` on field `T` that is `owns` (or `owns` or `reads`, when `A` writes): the obligation
     `g_A && g_k ==> place_A != arg_k` (`g_k` the atom's guard over the arguments), category `unsequenced`, at the
     access: "accesses `x`, which the call to `bump` may change, in an order C leaves unspecified" ("writes `x`, which
     the call to `peek` reads, …"). C leaves the order unspecified, not undefined; cerastes reports it because the
     result depends on it (0028 item 13: one verdict for every order).
   - **Two memory calls** `c1`, `c2` of one full expression that C does not order (neither is inside the other's
     arguments, no sequence point between): for each pair of atoms `k1`, `k2` on one field where one is `owns`:
     `g_1 && g_2 ==> arg_1 != arg_2`, `unsequenced`, at the later-created call: "the calls to `f` and `g` may both access
     `*p`, in an order C leaves unspecified". So `bump(&x) + bump(&y)` verifies and `bump(&x) + bump(&x)` fails. 0028
     item 13 also counts liveness and token cells: no callee writes them before `free` (M4.5d2), which adds them.
   - The checks of this item whose operands are computed inside an E5 branch are placed as item 3's (audit S5).
   - **No reordering across a memory call.** A write or a snapshot of a cell is never emitted on the other side of a
     memory call than C performs it, for every pair C orders: a write's set holds every fixed point of its operands
     (item 2), so in `y = (x = (bump(&x), 2));` the inner store follows the call (0025 item 2 would have placed it first,
     0026 item 15's caveat); a snapshot's set holds its guard's (item 2), so in `bump(&x) && x == 0` the read follows
     the call (audit B1). A statement's own top-level store (`x = (bump(&x), 2);`, `x = bump(&x);`) is not an effect:
     it follows the whole full expression, as since b1b (P60 C1: both verify today, and keep their Viper). A snapshot's
     set holds the comma floor and what its value uses, as today. Pairs C does not order are
     those this item checks: where the checks hold, the call does not touch the place, and either order gives one
     result.
   - **Fail closed** (audit S4). After emission, lowering checks (i) per key, that the writes and snapshots keep
     creation order (0020's `check_variable_order`, per key); (ii) for each write or snapshot of a field and each memory
     call on that field that C orders with it, derived from the expression tree and not from the sets (the access
     inside the call's arguments, or before the call through a sequence point, comes before; the access whose operands,
     guard or floor hold the call comes after; an E5 branch counts as the calls and accesses inside it), that the
     emitted order agrees; otherwise an internal error (exit 3). Two hooks show that it fires: `perturb_memory_order`
     (a cell write moved across a memory call) and `drop_snapshot_guard_set` (a snapshot's set without its guard's).
     Check (ii) also covers pairs of cell accesses of one field with different keys that the tree orders (audit round
     2); a third hook, `drop_left_floor` (the floor of a left operand that touches cells left out), shows that this half
     fires. Obligations that read cells and inline reads are not effects: they rely on their sets and on item 2's inline
     classification. Like 0021's checks, it cannot see a set that is too large (0026 item 4): the pins of item 12 guard
     that direction.
   - 0021 item 2's sentence "a callee cannot read or write the caller's locals (no globals or pointers before M4.5)" is
     replaced by this item for cells; for Viper locals it still holds.

5. **Compound stores whose address has a side effect** (handoff §3, M4.5b1a re-audit). A compound store `*a op= e`,
   `++*a` and `(*a)--` evaluates `a` once (C23 6.5.17.2p3): the `Deref` its value reads shares the node `a`. The
   sequencing rule (0020 item 4) and the "reachable once" rule for calls count a shared node once, so `*(q = p) += 1`
   and `*(g() ? &x : &y) += 1` are accepted (they were refused as unsequenced and as "a call that is reachable twice").

6. **`return &local;`** (question 65; amends 0025 item 6 with **R14, `return_local_address`**). A `return` whose value,
   looked through parentheses, explicit pointer casts, either arm of `?:` and the right operand of `,`, is `&x` (or
   `&*&x`) for an automatic local or a parameter `x` of the function is rejected at the `return`: "returns the address
   of `x`, whose lifetime ends when the function returns (-Wreturn-stack-address, -Wreturn-local-addr): return a value,
   or a pointer the caller gave (decision 0039)". P61: clang warns on all these shapes at the no-flag tier in both
   modes, GCC on all but `?:` (D1–D7, D9). **The front end's tiers only**, as every earlier matrix (0020 item 5): `return
   p = &x;` and `int *r = &x; return r;` warn only through GCC's optimizer (`-O2`), so they stay accepted and the
   caller's first use fails `lifetime` (question 65's answer; `tests/memory/negative/pick_ret_local.c`). The rule reads
   the IL's shape, not the front end's warning: EDG's #1056 fires on `&x`, `(&x)`, `&param`, a cast and `(0, &x)`, and in
   C on `&*&x` (C++ keeps `address_of(indirect(address_of(x)))`), but not on a `?:` (P60). Where #1056 fires it still
   prints beside the rejection; `VERIFY_WARNING_TAGS` is unchanged.

7. **`c ? owns(*p) : reads(*p)`** (question 67; amends 0037 item 5). The rejection of `owns(*p)` and `reads(*p)` on one
   parameter in one clause list is lifted. In `f__wf`, for each such pair in one clause list (the `requires` or the
   `ensures`), lowering asserts `prior ==> !(g_o && g_r)` after the later of the two in evaluation order (`prior` as in
   0037 item 7), category `permission`, at the later atom: "the contract of `f` both owns `*p` and reads it on one
   path". Where it holds, at most one of the two atoms holds on every path: the method's contract `c ? (acc(p.v_T) &&
   …) : (acc(p.v_T, a_k) && …)` is satisfiable, an `ensures` atom keeps its partner of the same kind (0037 item 6), and
   the call separation (0037 item 8: `g_o && g_r ==> p != p` for the callee's pair), the entry facts (item 9) and the
   return sums count one atom. Two atoms of one kind on one place in one clause list stay rejected. When the exclusion
   rests on a value `requires` (`requires(c == 0 || c == 1)`), a caller that breaks it is reported at the call's
   separation check (`permission`), which 0037 item 8 places before the value `requires` (`precondition`) (audit note 4).

   **Partners by path** (owner, question 72; audit S7; amends 0037 item 6). An `ensures` atom takes the ghost amount of
   its `requires` partner of the same kind and place, but the partner's guard was never compared with its own: a
   trusted contract `requires(c ? owns(*p) : reads(*p))`, `ensures(c ? reads(*p) : owns(*p))` hands the caller back a
   whole permission on the path where it took a fraction, the caller's state becomes impossible, and everything after
   the call verifies (since b1b, with `c ? owns(*p) : live(p)` too). In `f__wf`, for each `ensures` atom, after it in
   evaluation order: `assert prior ==> (g_ens ==> g_partner)`, the `ensures` guard over the post-state ghost values and
   the partner's over the pre-state ones, category `permission`, at the `ensures` atom: "the contract of `f` returns
   `*p` on a path where its `requires` did not take it". A partner taken and not returned (a callee that keeps the
   place, 0037 item 13) stays allowed. Every contract is checked, trusted or not, since a function with a body that
   breaks it fails only at its own `return`, and its callers would still see an impossible state.

8. **A function's measure that reads memory** (record 0038 item 8; S28 P6; audit B5 of 0038). A function's `decreases`
   may read `*p` for a pointer parameter `p` (the places contracts may read, 0037 item 5), with the values and guards
   contracts allow; an atom in a measure stays rejected. (P60: a measure that is a whole `*p` is an lvalue-flagged
   `eok_indirect`, one inside a `?:` arm is not; the encoder accepts both in a `decreases`.)
   - **Framing.** `f__wf` lowers the measure in contract mode over the pre-state ghost values (0037 item 7), after the
     `requires` clauses, and asserts each heap read's framing over the `requires` atoms (`permission`, at the measure:
     "the termination measure of `f` reads `*p` without owning it"). Silicon checks no method measure's
     well-formedness (S28 P6), so this is the only check, for every function, recursive or not.
   - **The method's `decreases`** is written plainly (no `old`: Silicon evaluates the caller's measure in its entry
     heap already, and an `old` there would read the caller's entry state for the callee too, S28 P6), each heap read
     guarded by its own chain over the framing disjuncts (0037 item 7's form), so it is well-formed wherever Silicon
     evaluates it.
   - **0021 item 4's explicit check**, when it is printed, comes after the call's permission checks and case splits
     (0037 item 8), reads the callee's measure over the arguments in the current heap (chain-guarded over the callee's
     atoms at the arguments) and the caller's measure under `old(…)` (the caller's entry heap, framed by the caller's
     `requires`). Item 4 guarantees that a memory call emitted between this check and the call, which C may evaluate
     after the call, writes no cell the measure reads.

9. **Still rejected** (fail closed; each pinned by a test):

   | Construct | Diagnostic | Lifted by |
   |---|---|---|
   | `T**`, the address of a pointer local; `*result` as a value | `unsupported_type`, `unsupported_expression` | b1c3 |
   | `owns`/`reads`/`live` in a loop invariant | `unsupported_annotation` | b1c3 |
   | `owns(*result)`; an `ensures` atom with no `requires` partner | `unsupported_annotation` | M4.5d2 |
   | an atom in a measure, a `check`, or code | as before | — |
   | two atoms of one kind on one place in one clause list | `unsupported_annotation` | — |
   | C++ lvalue uses of an assignment or `++`/`--` | `unsupported_expression` | — (0020 item 3) |

10. **Categories** (amends 0011 item 10, in its five copies, and the harness's pinned copies): **`unsequenced`**, "two
    accesses to one object, at least one a write, that C does not order, or an access and a call that may change the
    object (decisions 0028, 0039)", reported at the later access or call. Eighteen categories. `permission` gains item
    7's and item 8's messages. New rejection: `return_local_address` (R14).

11. **Facts relied on** (`internal/il-inventory.md`, "M4.5b1c2"): (P60) the IL of the shapes of items 2, 4–8 in both
    modes; (P61) the compiler matrix (Apple clang 21, GCC 16; both modes; three tiers; `-fsyntax-only`, and GCC `-O2`
    on the `return` shapes); (S29) Silicon on items 2–4, 7 and 8.

12. **Tests and documentation.** Must-verify and must-fail pairs for every accepted form in both modes
    (`tests/memory/`), including the null and permission checks of a write inside an expression and the soundness pins
    of audit B1 (`return bump(&x) && x == 0;` against a wrong `ensures`; `(p == NULL || *p > 0) && bump(q)` with a null
    `p`) and B2 (`(*r = nr()) + (*q = 1)` with `q` possibly null: `null-dereference`; `(*q = nr()) + 100 / *r`:
    `division-by-zero`; a termination check that a call which does not return must not hide) (audit B3); the flips of b1c2's pins (handoff §4.2); the VIR self-test's `mem_rej_cell_assignment_as_value`
    and `mem_rej_cell_postfix_increment_as_value` become lowering cases, and the hook of item 4 gets its case. Docs: the
    Memory and ownership page ("Expressions", conditional ownership, measures over memory, returned addresses), Error
    messages (`unsequenced`), Verifying a program (R14; `unsequenced_modification` on cells), Annotations.

## Consequences

- A cell may be written anywhere an assignment may stand; unsequenced accesses that may be one object are reported per
  path, and those that name one object are rejected as the compilers warn.
- A call whose contract holds a permission may share a full expression with other accesses and calls; an order that
  could change a result is reported (`unsequenced`), so `x + bump(&x)` and `x += bump(&x)` fail while `x = bump(&x)`
  verifies.
- A cell write's checks are items of 0021's family F, unlike the write of a Viper local (0025 item 2), so
  `(*p = f(a)) + (*q = g(b))` stays `unsequenced_calls` where the form with locals is accepted (audit note 3).
- In C++ the value of `*p = v` is an lvalue; cerastes uses the stored value (E4). A later read of it in the same full
  expression is unsequenced with any other writer of the place, which items 3–4 check (audit note 5).
- Programs accepted before b1c2 keep their Viper: none has a write to a cell inside an expression, a function measure
  over memory or a `?:` pair of `owns` and `reads`, and a memory call shared its full expression only with accesses in
  its own arguments and with the statement's top-level store (0037 item 8; P60 C1: `x = bump(&x);` and
  `x = (bump(&x), 2);`), which item 4 orders as before.
- `return &x;` is rejected; returning a copy of a local's address is still found at the caller's first use.
