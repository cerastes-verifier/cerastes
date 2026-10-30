# 0040 — Pointers to pointers, owning parameters, `*result`, written atoms in loop invariants (M4.5b1c3)

- **Status:** accepted for M4.5b1c3 (2026-10-30), written before code (invariant 6), revised after P62 (H1, H2 and A8:
  item 2) and audit round 1 (Opus, on the design: four blocking items, B1 a pointer loaded from a cell without case
  splits, B2 qualification conversions at depth writing a `const` object, B3 written-atom amounts that may be `none` and
  a de-duplication that ignores guards, B4 written atoms on places reached through a heap read; seven should-fix items
  and five notes; all taken; `build/m4.5b1c3/audit1.md`), audit round 2 (on the code: one blocking item, B1, a `const`
  object written through a pointer a trusted contract relates to it, fixed fail closed in item 2; four should-fix items,
  S1–S3 taken, S4 a documented false alarm in item 5; notes N1, N2 taken in item 5) and one re-audit (B1 through a cast,
  a `?:` or `owns` of a `const` place: item 2's contract rules widened, checked by the tests, not re-audited: the cap;
  `build/m4.5b1c3/audit2.md`). End to end, written atoms gained 0038's value rule (item 5). Owner decisions (M4.5b1c3 planning, 2026-10-30; plan `.claude/plans/m4-5b1c3-pointers-to-pointers.md`;
  handoff questions 73–77):
  - **73:** an atom in a `do … while (0)` invariant stays rejected, like an atom in a `check` (0038 item 2; item 10's
    table, which named b1c3, is corrected).
  - **74:** `owns(**pp)`, `live(*pp)` and reads of `**pp` in a contract (a place reached through a heap read) move to
    M4.5c, with struct places.
  - **75:** `[[cerastes::owning]]` beside an explicit `owns(*p)` or `p == NULL || owns(*p)` on the same parameter in the
    same clause list is merged; `reads(*p)`, `live(p)` or another guard beside it is rejected.
  - **76:** pointers to pointers at any depth.
  - **77:** sessions sized to one context (process only).
  - Earlier answers this record carries out: question 68 (the three pinned false alarms of reassigned parameters are
    re-expected `verified` with a written atom, each with a new twin).
- **Milestone:** M4.5b1c3 (2026-10-30).
- **Implements:** record 0028 item 1 (a pointer-typed object's field) and item 2 (`WF` as part of the permission to a
  pointer-typed cell) for address-taken pointer variables; record 0029 item 1 for target `p`; record 0038 item 5.
- **Amends:** 0029 item 1 (target `p` registered), 0032 items 2 and 10, 0037 items 3, 5 and 15, 0038 items 5 and 10,
  0039 items 1 and 9.

## Context

Record 0032 kept pointer variables out of memory: a pointer whose pointee is a pointer, and `&p` of a pointer variable,
are rejected (0032 item 10, "b1c"). Record 0037 rejected `*result` as a value and an atom on any place other than
`*param` (0037 items 5, 15), and record 0038 cut written atoms in `CERASTES_INVARIANT` to this session (0038 item 5),
which leaves a parameter the function reassigns without its place in every loop (three pinned false alarms). Record
0029 designed `[[cerastes::owning]]`, and 0038 moved its parameter target here. Facts: P52 (where `owning` lands), S28
P8 (written atoms on Silicon), P62 (the IL of this record's shapes) and S30 (Silicon), in `internal/il-inventory.md`.

## Decision

1. **Scope.**

   | | b1c3 (this record) | M4.5c | M4.5d2 |
   |---|---|---|---|
   | Objects | pointer variables whose address is taken (cells of pointer type), `T**` at any depth | struct members | heap blocks |
   | Contracts | `owns(*pp)`/`reads(*pp)` of a pointer cell; `*result` as a value; `[[cerastes::owning]]` on parameters | `owns(**pp)`, `live(*pp)`, reads of `**pp`; `owning` on fields | `owning` on functions, locals, globals; `owns_block`; `owns(*result)`; partnerless `ensures` atoms |
   | Loops | `owns`/`reads`/`live` written in `CERASTES_INVARIANT` | — | — |

   Amends 0032 item 10's, 0037 item 15's, 0038 item 10's and 0039 item 9's rows "`T**`, the address of a pointer local;
   `*result` as a value" and "`owns`/`reads`/`live` in a loop invariant" (lifted here, except in a `do … while (0)`
   invariant: no session, question 73).

2. **Pointer cells** (lifts 0032 item 10's `T**` row; 0028 items 1–2).
   - A **pointer type** may have a pointer pointee at any depth (question 76); at every level the innermost pointee is
     a type 0032 item 2 accepts (`bool`, an integer, an enumeration). `void*` and pointers to structs, unions and
     functions stay rejected at every level.
   - A **pointer variable whose address is taken** (a local, or a parameter through 0037 item 4's entry copy) is a cell,
     allocated and released like any other (0032 items 4, 8). Its value field is named by 0028 item 1's rule, the
     pointee's name after `ptr_`: `v_ptr_int` for an `int *` object, `v_ptr_ptr_int` for an `int **`, with the flag
     `d_ptr_…`, typed `Ref` and `Bool`. Qualifiers are ignored in field names only: `const` is ignored at every level of
     the name, and `volatile`, `_Atomic` and `restrict` stay rejected at every level of a pointer type (audit N1).
   - **Pointer conversions** (P62 A5, H1, H2; audit B2). Ignoring qualifiers in names is sound only while no conversion
     lets a `const` object be written through an unqualified pointer. `(const int **)pp; *cpp = &c; *p = 2;` writes the
     `const int c` (C 6.7.3p7) with no diagnostic from any compiler, and a check that `c` is still 1 would verify if the
     cell were reachable that way. So the encoder's rule for every pointer conversion node, written or compiler-generated,
     in both modes, stays 0028 item 1's: the target's pointee is the source's pointee with qualifiers added at the
     pointee's top level only (`int*` → `const int*`, `int**` → `int* const*`). A conversion that adds or drops
     qualifiers below that level is rejected (`unsupported_expression`):
     - `int**` → `const int**` (C++'s const-laundering form, which EDG in this configuration accepts with warnings #144
       and #167 where clang++ and g++ reject it; the driver also makes #144 and #167 errors in C++, as in C, so EDG's own
       error comes first for the implicit forms);
     - `int**` → `const int* const*`, which C forbids (GCC 16 rejects it, clang warns; EDG gives no diagnostic in C) and
       C++ allows as a qualification conversion. It is rejected in both modes: in C naming GCC's
       `-Wincompatible-pointer-types`; in C++ as not supported yet (an M8 entry: C++'s multi-level qualification
       conversions need the rule of C++ [conv.qual] by types, never the field names).
   - **`WF` takes the place of a range** (0028 item 2: `WF` is part of the permission to every pointer-typed cell):
     - a store to a pointer cell checks `v != null ==> WF(v)` of the stored value in the same row as 0032 item 6's
       liveness check of the value (a store is a use), category `lifetime` (audit round 2, S1: as a separate `internal`
       assert, a pointer local that a loop assigns with no invariant about it gave exit 3 on a user error; every
       pointer the encoder builds is `WF`, so the row fails only where the liveness check fails too);
     - `owns(*pp)` and `reads(*pp)` of a pointer cell (0037 item 6) carry `WF(pp.v_ptr_T)` where an integer cell's carry
       its range; `owns` keeps the flag;
     - the automatic loop invariant's range row (0038 item 3) becomes `kv > none && kd > none && x.d_ptr_T ==>
       WF(x.v_ptr_T)`;
     - an `f__wf` ghost value of a pointer cell (0037 item 7) has `WF(h)` as its `requires` where an integer's has its
       range.
   - **`**pp` in code** composes 0032 item 6's checks: the read `*pp` (null, permission, the flag, as for any `*p`),
     then null and permission on the loaded pointer's target. A dereference is not a use of the pointer's value (0032
     item 6), so through a dangling loaded pointer `**pp` fails `permission`, while a comparison, copy, store or argument
     of `*pp` fails `lifetime`. A store `**pp = v` loads `*pp` once. A pointer loaded from a cell is used like any other
     pointer value. A
     call argument that loads a pointer (`inc(*ptrptr)`) is copied into a `Ref` temporary, as a store's address is, so
     the call's checks read it once (audit N3).
   - **Pointer comparisons in contracts across qualifiers** (audit round 2, B1: a contract does what a conversion does).
     A `requires` or `ensures` that compares two pointers whose pointees differ in qualifiers at any level (`r == v` for
     `int *r` and `const int *v`; `*pp == v` for `int **pp`; `r == cpp` for `int **r` and `int *const *cpp`) is
     rejected (`unsupported_expression`). C's `==` converts both operands to a composite type that only adds
     qualifiers, so the comparison itself is valid, but a trusted contract stating it (`int *unconst(const int *v)
     ensures(r, r == v)`, which `return (int *)v;` meets in C) hands the caller an unqualified pointer to a `const`
     object, and since field names ignore `const` the caller's write through it, undefined in C (6.7.3p7), verified
     (the auditor's `k1`–`k3`; `k2`'s shape verified since record 0037). The re-audit found the same through a
     qualification-adding cast or a `?:` in the contract (`(const int *)r == v`, `(b ? r : v) == v`, also verified since
     0037), and without any equality: a trusted `owns(*v)` of a `const int *v` with `ensures(*v == 2)` writes a `const`
     object (`*(int *)v = 2;` meets that contract for every object that is not `const`). So, in a `requires` or
     `ensures` (fail closed; trusted contracts and contracts with bodies alike):
     - every pointer conversion node, written or compiler-generated, that adds or drops qualifiers of the pointee at any
       level is rejected (`unsupported_expression`), except a null constant and the `cerastes_old` macro's own `&(e)` to
       `const void *`; this covers the comparison above, the casts and the `?:` arms;
     - `owns(*p)` and `[[cerastes::owning]]` of a `const`-qualified place (`const int *p`, `int *const *pp`) are rejected
       (`unsupported_annotation`): a function that only reads takes `reads(*p)`.
     Comparisons and atoms in code and in loop invariants are checked, not trusted, and stay. The principled
     alternative, `const` objects held at less than `write` after their initializer, changes scope exits and lifetimes
     and is left to the session that needs `strchr`-style contracts (M4.6; handoff question 78), which then lifts these
     rules.
   - **Case splits for loaded pointers** (audit B1: Silicon does not branch on the disjunction a loop's havoc or a
     contract leaves, `*pp == &x || *pp == &y`, and fails first with its own reason). A pointer loaded from a cell gets
     0037 item 3's case splits, as a call's pointer result does: `if (t == X) { }` against each cell in scope, each
     parameter place and each enclosing loop's written-atom place of its pointee's field, right after the load. The
     splits at the top of a loop's body and after the loop (0038 item 6) also cover the pointer cells the loop may
     write: they compare the cell's loaded value, not the cell's `Ref`.
   - **Writes that reach a place** (0038 item 3, the havoc rule): a store through a pointer whose pointee has field
     `ptr_T` (`*pp = q`), and a call whose `requires` owns a `ptr_T` cell, reach every automatic place of field
     `ptr_T`.
   - **Sequencing** (0039): keys are syntactic objects with their field; `*pp` and `**pp` are places of different
     fields, so they never alias (0028 item 1). 0039 item 3's static half extends to nested dereferences of one pointer
     variable the expression does not assign (`*pp`, `**pp`, …, each its own object; the `*pp` inside `**pp` is a read
     of `*pp`). P62 A8: GCC's `-Wall` (`-Wsequence-point`) warns on `**pp + (**pp = 1)`, `*pp == (*pp = &y)`,
     `**pp = (*pp = &y, 5)` and `(*pp = &y) == *pp` in both modes, and g++ on `**pp + (*pp = &y, 0)`; clang is silent.
     By 0020 item 5's union rule they are rejected `unsequenced_modification` in both modes. A shape no compiler warns
     on keeps 0039's path-precise check (`*pp = &y, **pp` is sequenced; an access through another name is item 3's
     `unsequenced`).
   - **Contracts of `T**` parameters:** `owns(*pp)`, `reads(*pp)`, `*pp` and `old(*pp)` (a pointer value; `==` in a
     contract is identity, 0037 item 12). The pointer stored in `*pp` is not live by any atom the contract can write in
     b1c3 (`live(*pp)` is M4.5c's, question 74), so a callee may store into `*pp` and compare `*pp` with null in a
     contract, but a use of the loaded pointer in code needs its liveness and fails `lifetime` unless it is null.

3. **`[[cerastes::owning]]` on parameters** (0029 item 1, target `p`; P52).
   - **Registration:** `{"owning", "?(X)", "p", RAF_NO_FLAGS}` in the commit that gives it its meaning
     (`check_attributes` skips registered attributes). P52's `RAF_ENTITY_ATTACHED` needs a routine target: with target
     `p` alone EDG aborts at startup ("an entity-attached attribute must apply to a routine"); M4.5d2 adds `r` and the
     flag. EDG then refuses `owning` on a function, a variable or a field itself (`wrong_entity_for_attribute`,
     "attribute \"cerastes::owning\" does not apply here"; pinned on a function and a local). `<cerastes.h>` defines
     `CERASTES_OWNING`; `CERASTES_OWNING_N(n)` waits for M4.5e1.
   - **Meaning:** a parameter `p` with `owning` on any of its function's declarations (the union, P52) gets the clause
     `p == NULL || owns(*p)` first in its `requires` and first in its `ensures` (`p` its entry value there, 0006 item
     5), so the user's later clauses may read `*p` under `p != NULL` (0037 item 7's framing). The clause is an
     ordinary atom with guard `p != NULL` (0037 items 5–10 apply: amounts, calls, entry, returns), positioned at the
     attribute for diagnostics. It counts as 0037 item 2's `owns` conjunct for `restrict` (`restrict_backed` runs after
     the desugaring).
   - **Merged with an explicit atom** (question 75): when the same clause list already holds `owns(*p)` as a top-level
     conjunct (guard `true`) or `p == NULL || owns(*p)` (guard `p != NULL`), `owning` adds nothing to that list: the
     explicit atom implies `owning`'s clause, and the place keeps one atom, at the explicit atom's position (so a read
     of `*p` written before it is not framed, audit S7). Each list is merged on its own: an explicit `ensures(owns(*p))`
     beside `owning`'s `requires(p == NULL || owns(*p))` fails 0039 item 7's partners-by-path check unless the
     `requires` also says `p != NULL` (pinned).
   - **Declarations** (0029 item 1): every declaration of the function carries the same `owning` on the same
     parameters; one that differs is `annotation_mismatch`, naming both. This is stricter than 0006 item 6, which lets a
     declaration carry no contract at all: a definition must repeat its prototype's `owning` (audit N2). A parameter
     declared through a typedef'd function type is handled: the front end merges the typedef's attribute into the
     function's parameter, and a declaration that differs is `annotation_mismatch` (pinned). The encoder tells
     declarations apart by their source positions (line and column, and the macro invocation's for a macro); two
     declarations of one function it cannot tell apart (both on one line from one macro invocation) with `owning` on
     either are rejected (`unsupported_annotation`; audit round 2, S2: matching by line alone accepted a mismatch on one
     line).
   - **Rejected by the encoder** (`unsupported_annotation`, each with its own message): `owning` on a non-pointer, on a
     pointer to function, on `void *`; `owning(n)` (M4.5e1); the attribute twice on one declaration; beside `reads(*p)`,
     `live(p)` or an `owns(*p)` under any other guard in the same clause list ("`p` is an owning parameter: its contract
     already holds `p == NULL || owns(*p)`"). A parameter that carries `owning` on one declaration and not on another
     (or a different argument) is `annotation_mismatch`, naming both declarations (0029 item 1).
   - A `T**` parameter may be owning: the clause owns the pointer cell `*pp` (item 2).
   - `owning` is a contract: a function declared without a body whose only contract is an `owning` parameter is
     callable (0011's `call_without_contract` does not apply), with the desugared clauses as its contract.

4. **`*result` as a value** (lifts 0037 item 3's and item 15's row).
   - In an `ensures`, `*result` (the result binder of a function returning a pointer, `*r`) reads the cell the result
     points to on return. It is framed per path by 0037 item 7, with `result` as the place: the disjuncts are `D_i =
     g_i && q_i == result` over the `ensures` atoms of its field that precede the read in evaluation order, so
     `ensures(r, r == p && owns(*p) && *r == 1)` is framed (the `requires` atoms frame `requires` reads and `old` reads;
     `result` exists only on return).
   - **`f__wf`:** the read is a post-state ghost value of the place `result` (keyed apart from the parameters), with its
     type's range (or `WF`) as a `requires`; `prior` gains `result == q ==> h_result == h_q` for each post-state ghost
     value `h_q` of a parameter place `q` of the same field, as for two parameter places (0037 item 7), and `g_r && g_e
     && result == q ==> h_result == h_q_pre` for a `reads` place `q` returned by its `ensures` partner (its value on
     return is its value on entry, 0037 item 7; audit S4: without it `requires(reads(*p) && *p > 0) ensures(r, r == p
     && reads(*p) && 10 / *r > 0)` reports a division by zero). The framing assert is `permission`: "the contract of
     `f` reads `*r` without owning it". A function with a body whose `ensures` reads `*r` unframed also fails its
     `postcondition` at the return (the chain's dummy value; audit S5): the pin expects both.
   - **The method's `ensures`:** the read is the `ite` chain over the `D_i` (0037 item 7's form).
   - **Callers** read it through the call's `ensures` and 0037 item 3's case splits after a call whose result is a
     pointer.
   - `old(*result)` is rejected (`unsupported_expression`: `result` has no value on entry); an atom on `*result` stays
     rejected (M4.5d2).

5. **Written atoms in `CERASTES_INVARIANT`** (lifts 0038 item 5; S28 P8).
   - **Accepted:** `owns(*q)`, `reads(*q)` and `live(q)` (C and C++ spellings) in a loop invariant, for any pointer
     variable `q` in scope that is not itself a cell (a local or a parameter, whether the function or the loop assigns
     it, whose address is never taken), in 0037 item 5's positions (a top-level conjunct; the right operand of `||` whose
     left operand reads no heap; an arm of `?:` whose condition reads no heap). Its guard `g` is lowered in contract
     mode. The atom names `q`'s value at each iteration's start (the invariant's state). The same kind twice on one
     variable in one loop's invariants is rejected, as in a contract.
   - **Rejected, naming M4.5c** (audit B4: a place reached through a heap read, question 74): an atom on `*q` where `q`
     is a cell (its value is `q_cell.v_ptr_T`), `owns(**pp)`, `reads(**pp)`, `live(*pp)`, and a read of `**pp` written
     in an invariant or a loop measure. A read of `*q` where `q` is a cell is accepted there (audit round 2, N2): the
     load of `q` is framed by `q`'s automatic place and followed by the loaded pointer's case splits, and the read of
     `*q` is framed by item 6 of 0038 like any other.
   - **Meaning** (in the Viper invariant, after the automatic conjuncts of 0038 item 3, before the user's other
     clauses, each an `internal` row; `x_i` ranges over the automatic places of `q`'s field, `L_i` over the liveness cells
     of all automatic places):
     - `g ==> q != null && WF(q) && offset_of(q) < len(block_of(q))` (`live`: no bound);
     - **`NOTAUTO`:** `NV = AND_i q != x_i` over the automatic places of the field, and `NL = AND_i (gl_i ==>
       lvloc(block_of(q)) != L_i)` over the liveness cells, each under the guard `gl_i` of its automatic place (audit S1:
       an unguarded comparison against a parameter that may be null gives a false alarm; the automatic liveness row is
       under `gl_i` too). `g && NV ==> a_v > none && a_d > none && acc(q.v_T, a_v) && acc(q.d_T, a_d) && q.d_T &&
       range(q.v_T)` (the range or `WF`, for `reads` as for `owns`: 0037 item 6; audit S3); `g && NL ==> a_l > none &&
       acc(lvloc(block_of(q)).lv, a_l)` (audit B3: an amount snapshot of `none` would make the atom vacuous);
     - **on an automatic place:** `g && q == x_i ==> kv_i == write && kd_i == write` for `owns` (`kv_i > none && kd_i >
       none` for `reads`; `kl_i > none` for every kind against the liveness cells), with the flag and range read
       through the place's own chain-guarded read: an atom on an automatic place takes nothing twice;
     - **between written atoms** of one field, in clause order: a later `reads` atom whose place equals an earlier
       atom's takes nothing more (`AND_{k<j} (g_k ==> q_j != q_k)` joins its `NV`), and a liveness cell is taken once
       (`AND_{k<j} (g_k ==> lvloc(block_of(q_j)) != lvloc(block_of(q_k)))` joins its `NL`; audit B3: an earlier atom
       whose guard is false holds nothing); two atoms of which one is `owns` must name different places (the separation
       check below).
     - A `reads` atom takes everything the function holds of its place, as an automatic place does: a write under it
       verifies when the function holds the place whole (pinned), and fails `permission` otherwise.
     - **Values** (0038 item 3's rule, for the place a written atom names; found end to end: without it the re-expected
       `loop_invariant_reads_unowned.c` fails `invariant-preserved`, since the atom's permission makes Silicon forget
       the value at the loop head). When the loop does not assign `q`, the snapshot `vs := q.v_T` is taken at entry
       under `g && NV`, and the row `g && NV ==> q.v_T == vs` keeps the value when no write in the loop can reach
       field `T` (0038 item 3's havoc rule); otherwise only `g && NV && a_v < write ==> q.v_T == vs` (a place held at
       less than `write` cannot be written). When the loop assigns `q`, the atom names another place at each iteration
       and no value is kept: the invariant states it.
   - **Amounts:** before the loop, after the automatic snapshots, ghost `Perm` locals: `a_v := write`, `a_d := write`
     for `owns`; `a_v := perm(q.v_T)`, `a_d := perm(q.d_T)` for `reads`; `a_l := perm(lvloc(block_of(q)).lv)` for every
     kind (0038 item 9's lint rule for `Perm` locals applies). An atom whose `q` the loop moves keeps its entry amounts,
     so the new place must be held at least as much (fail closed): for `reads` the value and flag amounts, and for every
     kind the liveness amount. A loop that moves an owned pointer from one parameter's object to another's, whose
     liveness the function holds at different fractions (the callers' ghost amounts), is reported at the back edge
     although it is correct (audit round 2, S4: a swap loop over two reassigned parameters); pinned as a false alarm
     and documented, until the liveness amount follows the place (handoff question).
   - **Guards** may read a cell by name (`flag == 0 || owns(*q)` with `flag` address-taken): the cell is an automatic
     place, whose conjunct frames the read (audit round 2, N1); a guard that reads through a pointer is rejected, as in
     a contract.
   - **Checks** (0028 item 11: Silicon's own invariant checks never fail first), each one atomic assert under the atom's
     guard, category `permission`:
     - **at entry** (after the amounts), as a call's (0037 item 8): `q != null`, `WF(q)`, the bound, the permissions
       (`== write` for `owns`, `> none` for `reads`, the liveness `> none`: the positivity rows of the amounts), the
       flag and range, for a place that is not automatic, and the amounts of the automatic place it is: "enters this
       loop without owning `*q`, which its invariant needs" (`reads`: "… without the permission to read `*q` …"; `live`:
       "… without `q` pointing to a live object …"), reported at the atom;
     - **separation:** for two written atoms of one field, one of them `owns`, `g_j && g_k ==> q_j != q_k`, at entry
       and at the back edge: "the invariant of this loop owns `*q` and `*s`, which may be one object", reported at the
       later atom;
     - **at the back edge** (after 0038 item 4's checks), the same rows with the snapshot amounts (`perm(q.v_T) >=
       a_v`, and `a_v > none`): "an iteration of this loop ends without owning `*q`, which its invariant needs",
       reported at the body's closing brace.
   - **Case splits** (S28 P8: Silicon does not branch on the disjunction the guards leave): `if (q == x_i) { }` and
     `if (lvloc(block_of(q)) == L_i) { }` for each written atom against each automatic place, and `if (q_j == q_k) { }`
     and `if (lvloc(block_of(q_j)) == lvloc(block_of(q_k))) { }` between written atoms (the liveness cells for any
     fields), before the snapshots, at the top of the body (before a top-test condition), after the loop and after its
     `break` label (audit S2). 0037 item 8's per-call splits against the caller's `reads` places also run against the
     written `reads` places of the enclosing loops (audit N4).
   - **Framing** (0038 item 6): a written atom frames the invariant's and the measure's later reads of `*r` with the
     disjunct `g && q == r && a_v > none`, after the automatic places' disjuncts in the chain.
   - **After the loop** the permission comes back for the place `q` designates on exit. When the loop assigns `q`, only
     the user's invariant says where `q` ends (S28 P8's gap: `q0 == r` style ties are the user's, never emitted, since
     an emitted tie would be a claim the loop may break). The Memory and ownership page shows the tie; a must-fail test
     pins the gap.
   - **Nested loops:** an inner loop's automatic places are the function's (0038 item 3), not the outer loop's written
     atoms: a place the outer loop holds only through a written atom needs its own atom in the inner invariant.
   - **Still rejected, by no session:** an atom in a loop measure (0038 item 10), and in a `do … while (0)` invariant
     (question 73; asserted once like a `check`, 0038 item 2).

6. **The places an atom may name** (amends 0037 item 5): in a contract, `*p` for a pointer parameter `p`, which
   includes `*pp` of a `T**` parameter (a pointer cell, item 2); in a loop invariant, `*q` for a pointer variable `q`
   (item 5). `owns(**pp)`, `reads(**pp)`, `live(*pp)` and a read of `**pp` in a contract stay rejected
   (`unsupported_annotation`, naming M4.5c; question 74); `*local`, a global and a member as before (M4.5b2, M4.5c).

7. **The lint** (`lint_viper.py`): no new rule. The pointer fields and their allocation methods follow the existing
   tagged shapes (0032 item 11); written atoms' amounts are `Perm` locals under 0038 item 9.

8. **Categories:** none new. `permission` gains item 4's framing message and item 5's three messages.

9. **Still rejected** (fail closed; each pinned by a test):

   | Construct | Diagnostic | Lifted by |
   |---|---|---|
   | `owns(**pp)`, `reads(**pp)`, `live(*pp)`, a read of `**pp` in a contract, an invariant or a loop measure; a written atom on `*q` where `q` is a cell | `unsupported_annotation`, `unsupported_expression` | M4.5c |
   | a pointer conversion that adds or drops qualifiers below the pointee's top level (`int**` → `const int**`, → `const int* const*`), written or compiler-generated | EDG's #144/#167 as errors (implicit, C and C++), `unsupported_expression` | — (C); M8 (C++'s [conv.qual]) |
   | `owning` on a function, a local, a global | EDG's own diagnostic (registration target `p`) | M4.5d2 |
   | `owning` on a field | EDG's own diagnostic | M4.5c |
   | `owning(n)` | `unsupported_annotation` | M4.5e1 |
   | `owns(*result)`, an `ensures` atom with no `requires` partner, `owns_block` | `unsupported_annotation` | M4.5d2 |
   | `old(*result)` | `unsupported_expression` | — |
   | an atom in a loop measure, a `check`, a `do … while (0)` invariant | `unsupported_annotation`, `call_in_annotation` | — |
   | `void*`, pointers to structs, unions and functions at any level | `unsupported_type` | M4.5c, M4.7, M8 |

10. **Facts relied on** (`internal/il-inventory.md`, "M4.5b1c3"): (P52) where `owning` lands; (P62) the IL of `T**`,
    `&p` of a pointer variable, `*r` in an `ensures`, written atoms, `owning` with target `p`; (S28 P8, S30) Silicon on
    pointer cells, `*result` framing and written atoms.

11. **Tests and documentation.** Must-verify and must-fail pairs for every accepted form in both modes (`tests/memory/`):
    pointer cells (null, lifetime and `check` twins, a `T***` chain), `T**` parameters, owning parameters (with the
    merge and every rejection), `*result` (a wrong value, an unframed read), written atoms (entry, back edge,
    separation, `reads`, `live`, S28 P8's gap); the ports of `address_of_local.c`'s `address_of_local` and
    `address_of_local_uintptr_t` and of `swap.c`'s `swap` with its `swap(&x, &x)` aliasing trap; a `T**` aliasing trap
    (`**pp = 1; *p = 2; check(**pp == 1)` where `*pp == p` may hold). The audit's pins: B1 (`*pp = c ? &x : &y` in a
    loop, then `**pp`), B2 (`(const int **)pp; *cpp = &c; *p = 2; check(c == 2)` in both modes, rejected; C++'s
    implicit `const int**` and `const int* const*`), B3 (`q == NULL || (reads(*q) && *q == 0)` with `q` null at entry;
    an earlier de-duplicated atom whose guard is false), B4 (an atom on a cell pointer, `**pp` in an invariant), S1
    (`owning` beside a written atom and a call), S3 (a write under `reads` on an owned place verifies), S7 (an explicit
    `ensures(owns(*p))` beside `owning`), N2 (a typedef'd function type), N5 (a callee storing `&local` into `*pp`, then
    the caller's use: `lifetime`; a guard flipping from false to true; `owns` written on an automatic `reads` place:
    fails at entry). The flips of b1c3's pins (plan). Docs: Memory and
    ownership (pointers to pointers, owning parameters, `*r`, written ownership in loops), Annotations, Error messages,
    Verifying a program.

## Consequences

- An address-taken pointer variable is a cell like any other: VeriFast's `address_of_local` shape verifies as written.
- A function taking `T**` may store into `*pp` and state where `*pp` points, but cannot use the loaded pointer until
  M4.5c lets a contract say it is live (`live(*pp)`).
- `[[cerastes::owning]]` is a spelling of `p == NULL || owns(*p)` on both sides of the contract; an explicit `owns`
  beside it is merged, anything else beside it rejected.
- A pointer the function reassigns can carry its permission into a loop through a written atom; the three pinned false
  alarms of 0038's Consequences are gone (question 68). When the loop moves the pointer, the user's invariant must say
  where it ends.
- Programs accepted before b1c3 keep their Viper: none has a pointer pointee, an address-taken pointer variable,
  `owning`, a read of `*result` or an atom in an invariant.
