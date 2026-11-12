# 0060 — Ranges in contracts (M4.5e1b)

- **Status:** draft (2026-11-11, M4.5e1b), written before code (invariant 6) from the plan's section 6
  (`.claude/plans/m4-5e1b-ranges-in-contracts.md`, approved 2026-11-11), revised by P79's and S44's decision tables
  (the plan's section 5; `internal/il-inventory.md`, chapter M4.5e1b; 52 fixtures `tests/viper-smoke/memory/s44_*.vpr`)
  and Step 0's runs (the table at the end: every outcome inside its table, S44 (b) on its "If not" branch), and by
  audit round 1 (Opus, on this draft and the 55 fixtures, all reproduced: one blocking item, B1, the partners-by-path
  rule of 0039 item 7 missing for ranges, a false proof through a trusted contract, item 3; seven should-fix items, all
  taken: S1 the return shape row `permission` (item 9), S2 the caller's own read amounts in item 6's minimum and
  question 238 restated, S3 the count's and the base's conversions (item 3), S4 the count bound to a local (item 8),
  S5 the argument that no range sum row is needed at a call (item 8), S6 the cell's return sum (item 9), S7 the
  multi-error twins (item 8); notes N2, N4, N5, N10 and N11 taken in items 1, 5, 15 and the tests;
  `build/m4.5e1b/auditor-report-1.md`). Owner
  decisions:
  - At M4.5e1's planning and before e1a (2026-11-11): **228** `cerastes_reads_range(p, n)` beside `owns_range`;
    **230** `int a[static N]` stays rejected (M8); **231** `owns_range(p, n)` and `reads_range(p, n)` imply `p !=
    NULL`, and `(NULL, 0)` is written `n == 0 || owns_range(p, n)`.
  - At M4.5e1b's planning (2026-11-11): **236** an atom on one element (`owns(p[i])`) is M4.5e2's, with `owns(*(p +
    i))`; **237** a range atom written in a loop invariant is M4.5e2's.
  - Open, each built as its recommendation unless answered (handoff §8): **238** `reads_range`'s amount from the
    range's first cell, lowered by the caller's own read amounts (audit round 1, S2), so that only a caller whose
    holding over the range is unequal for another reason gets a false `permission`, a pinned limit (item 6); **239** a `must_init` filler that
    fills with a loop a pinned false alarm until e1c (item 12); **240** the array-parameter shapes the compilers warn
    about rejected like the compilers (item 15; the matrix's reading: in both modes); **241** (this session, Step 0's
    R2) VLA parameters on a declaration without a body rejected like those of a definition (item 15).
  - Standing: 69 (no EDG work), 77 (one context), 160 (a new test's expect line corrected after its first run, listed
    in status).
- **Milestone:** M4.5e1b.
- **Amends:** 0011 item 10's messages; 0028 items 7, 8, 11 and 13; 0029 items 1 and 3; 0037 items 2, 5–10 and 15;
  0038 items 3 and 6; 0039 items 4 and 7; 0040 item 3; 0042 items 4–8; 0059 items 1, 8 and 10.

## Context

Record 0059 (M4.5e1a) gave arrays that live inside one function: local arrays as blocks of N slots, `a[i]` and `p[i]`
with `out-of-bounds`, the marked range rows of record 0028 item 11, loops that carry arrays as range places. A function
still cannot say that it owns or reads `p[0..n)`: `owns_range` is rejected, an element cannot be read in a contract,
and `p[i]` of a pointer cannot be read in a loop invariant. This record lets ranges cross calls, as record 0037 did for
single cells: the atoms, element reads in contracts and their framing, the amounts and rows at calls and returns, loops
over a parameter's range, P4222's markers over a range, `owning(n)` on a parameter, `restrict` backed by a range, and
array parameters. Facts: P79 (the IL of the new shapes) and S44 (Silicon on the shapes of this record, the fixtures
`tests/viper-smoke/memory/s44_*.vpr`), in `internal/il-inventory.md`, chapter M4.5e1b; the table at the end maps each
outcome to the branch of the plan's decision table it took.

## Decision

1. **Scope.** In: `cerastes_owns_range(p, n)`, `cerastes_reads_range(p, n)` and the C++ `cerastes::` forms, as atoms
   in a function's `requires` and `ensures`, `p` a pointer parameter; element reads `p[e]` in `requires`, `ensures` and
   a function's `decreases`; `cerastes_old(p[e])`; calls and returns over ranges; `f__wf`; loops over the ranges of
   parameters (automatic range places); `p[e]` of any pointer and an array used as a pointer in loop invariants and
   measures; P4222's markers on a parameter whose atom is a range; `owning(n)` on a parameter with
   `CERASTES_OWNING_N(n)`; `restrict` backed by a range; array parameters (`T a[]`, `T a[N]`) tested and documented,
   `sizeof` of one rejected; the `carrays.c` port with single-element facts. Everything else stays rejected, each
   with a diagnostic naming where it is lifted and a pin:

   | Construct | Lifted by |
   |---|---|
   | an atom on one element (`owns(p[i])`, `reads(a[i])`); `&p[e]` and `p + i` in a contract | M4.5e2 (236) |
   | a range atom in a loop invariant; a loop in a function that assigns its range parameter (a false alarm) | M4.5e2 (237) |
   | a range whose base is not a pointer parameter (`result`, a global, a member, a loaded pointer); a count that reads memory; a partnerless `ensures` range; an `ensures` range with another count than its `requires` partner; `owning(n)` on results, locals, fields and globals | M4.5e2 (members: e1e) |
   | `owns_range` and `reads_range` on one parameter in one clause list; a range atom and a cell atom of the same parameter, in one clause list or one in each | M4.8 |
   | `cerastes_forall`; a `must_init` filler that fills with a loop (a false alarm); the full lists of `carrays.c` | e1c |
   | ranges over structures, array members, global arrays | e1e |
   | `int a[static N]`, VLA parameters (`int a[n]`, `int a[*]`) | M8 |
   | `owns_range` of a `const` pointee (write `reads_range`); a range over `void *` or a function pointer | — |

   **The messages that name M4.5e1b or "M4.5e1"** (R2: twenty in `src/`, thirteen and seven) are each lifted by this
   session or re-pointed to the row above that holds their construct. Three have no row: `ref_to_uninit` on an array
   local names no session (an array is not a pointer; Flip B's `array_reject_ref_to_uninit_array.c`); an array used as
   a pointer in a contract can only be a global array, so it names e1e; and the declaration message's "parameters and
   results are M4.5e1b" is dropped (array parameters are pointer parameters, and no C function returns an array).
   `owning(n)` on a function gets the pin it lacks (M4.5e2).

2. **Spellings.** `<cerastes.h>` declares, in both modes, `cerastes_owns_range(...)` and `cerastes_reads_range(...)`
   (C23's variadic form, `noexcept` inside `extern "C"` in C++), and in namespace `cerastes` a `reads_range` with
   `owns_range`'s signature, which stays as it is (two goldens print it). Each takes a pointer and a count. The names
   join the six name lists (`encode.cpp`'s `SpecFn`, `spec_fn`, `declared_in_cerastes_header` and `call`'s dispatch;
   `il_query.cpp`'s `is_spec_function_name`; `annotation_checks.cpp`'s `is_spec_name` and `is_protected_macro_name`;
   `src/driver/edg_args.cpp`'s `is_cerastes_name`; the reserved names of the Annotations page) and pass
   `declared_in_cerastes_header`. In code they are `spec_call_in_code`; in a `check`, `call_in_annotation`.

3. **The atom.** `owns_range(p, n)` and `reads_range(p, n)`, in 0037 item 5's positions, in a function's `requires`
   and `ensures`.
   - **`p`** is a pointer parameter (in an `ensures`, its entry value) to `bool`, an integer type, an enumeration or a
     pointer to an object type; for `owns_range` the pointee is not `const` (0040 item 2).
   - **`n`** is an integer expression over parameters and constants that reads no memory, with 0010 item 11's
     obligations checked in `f__wf`; its value is the written expression's, never a compiler-generated conversion's
     (P79 (a): a generated cast is flagged `compiler_generated` and has no `expr_range`, so it is told from a written
     cast, which keeps its meaning; in C a `char` or `short` count is promoted to `int` by such a cast, and in C++ both
     arguments of `cerastes::owns_range` and `reads_range` are); in an `ensures` the parameters are entry values.
     **Only the top-level generated conversion to the spec function's parameter is peeled** (audit round 1, S3):
     generated conversions inside the written count keep their meaning (`n + 1u` converts `n` to `unsigned int`, and
     `n == -2` gives C's `UINT_MAX`, not `-1`). After the peel the count's type is an integer type other than `bool`;
     a `bool`, enumeration, floating or pointer count is rejected.
   - **The base** is the parameter as written, with only the top-level generated conversion of C++'s `const T*`
     parameter peeled; a written cast on the base (`cerastes::owns_range((const char *)p, n)` over an `int *p`) is
     rejected (audit round 1, S3): the element type is always the parameter's pointee, never `T`.
   - **VIR:** `AtomKind::OwnsRange` and `ReadsRange` on `Expr::Kind::Atom`, with the count as a new operand;
     `Builder::range_atom`; a structural equality of pure expressions for the partner rule.
   - **Partners:** an `ensures` range atom pairs with the `requires` atom of the same kind, parameter and count
     (structurally equal). A `requires` range with no partner is kept by the callee (0037 item 13).
   - **Partners by path** (0039 item 7; audit round 1, B1). In `f__wf`, after each `ensures` range atom in evaluation
     order: `assert prior ==> (g_ens ==> g_partner)`, the `ensures` guard over the post-state ghost values and the
     partner's over the pre-state ones, category `permission`, at the `ensures` atom: "the contract of `f` returns
     `p[0..n)` on a path where its `requires` did not take it". Every contract is checked, trusted or not. Without it a
     trusted `requires(n == 0 || owns_range(p, n)) ensures(owns_range(p, n))` called with `(a, 0)` hands back a
     liveness fraction it never took and the caller's state becomes impossible: the audit's
     `abl/C1_trusted_guard_mismatch.vpr` verified an `assert false` after such a call.
   - **Rejected** (`unsupported_annotation`, one pin each): item 1's rows on bases, counts and partners; the same kind
     twice on one parameter; a wrong number of arguments; a structure pointee (e1e).

4. **Viper meaning** (`atom`, `CMode::Method`). Under the atom's guard `g`, each line its own conjunct, each quantifier
   over the absolute slot with the trigger `{ loc(block_of(p), k) }` and the range `offset_of(p) <= k && k <
   offset_of(p) + n`:
   - `p != null && WF(p) && 0 <= n && offset_of(p) + n <= len(block_of(p))`;
   - `acc(loc(block_of(p), k).v_T[, amt])`; `acc(loc(block_of(p), k).d_T[, amt])`: full for `owns_range`, the atom's
     ghost amount for `reads_range`;
   - the flags, `loc(block_of(p), k).d_T`; the type's range (`WF` for a pointer element, none for `bool`);
   - `acc(lvloc(block_of(p)).lv, amt)`.

   One ghost `Perm` formal per `requires` atom, numbered with the cell atoms (0037 item 6). An empty range (`n == 0`)
   holds the liveness fraction only. **The guard** stands outside each quantifier, `g ==> forall k :: …`, one conjunct
   per line, and a marked row under a guard is written inside `if (g) { }` (S44 (a), the table's "Expected" branch).

5. **Element reads in contracts, and `f__wf`** (0037 item 7).
   - `p[e]` is accepted in `requires`, `ensures` and a function's `decreases`, and inside `cerastes_old` and
     `cerastes::old` in an `ensures`, for a pointer parameter `p` and an index that reads no memory (P79 (e): the
     subscript node of code; the index may name the result). `cerastes_old(p[i])` is `address_of(subscript(p, i))` in
     C (never folded) and a reference bound to the subscript in C++ (P79 (d)); `old_c`'s rejection of an element is
     lifted. VIR:
     `Deref(ElemAddr(p, e))`; `check.cpp` accepts that read in a contract and still refuses the address as a value.
   - **Framing.** A read of the address `q` is framed by a cell atom as today (`g_i && q_i == q`) or by a range atom:
     `g_i && block_of(q) == block_of(p_i) && offset_of(p_i) <= offset_of(q) && offset_of(q) < offset_of(p_i) + n_i`.
     `f__wf` asserts the disjunction (`permission`: "the contract of `f` reads `p[i]` without owning it"); no separate
     bound row.
   - **Ghost values.** One ghost formal per written read, keyed by base, index and state; the existing fact `q_a ==
     q_b ==> hv_a == hv_b` now compares element addresses, between element reads and between a cell read and an
     element read alike (`*p` and `q[0]`, audit round 1, N4); a `reads_range` place's post value equals its pre value
     (sound: no verified callee writes a slot it holds only through read atoms, and every caller lends at most
     `N / (N + 1)` of what it holds; N4).
   - **In the method's contract** each read is the `ite` chain over its framing disjuncts (0037 item 7).
   - In a range atom's `Wf` form only the pure line of item 4 remains.

6. **Amounts at a call** (0037 item 6). `amt_k := (g_k ? m_k : write) / (N + 1)`. For `owns_range`, `m_k` is `perm`
   of the liveness cell. For `reads_range` it is `n > 0 ? min(perm(F.v_T), perm(F.d_T), perm(L)) : perm(L)`, with `F`
   the range's first slot `loc(block_of(p), offset_of(p))`: `perm` cannot stand under a quantifier, so one cell
   represents the range and the marked rows then check all of it at that amount. **The caller's own read amounts**
   (audit round 1, S2): the minimum also takes every ghost amount of the caller's own `reads` and `reads_range` atoms of
   the field (its formals `amt_j`), so `m_k = n > 0 ? min(perm(F.v_T), perm(F.d_T), perm(L), amt_1, …) : perm(L)`. It
   can only lower the amount, so it adds no error and cannot make a proof unsound; without it every function with two
   read atoms of one field that may alias (two `reads_range` parameters, or one and a `reads` cell) gets a false
   `permission` when it passes one of them on, because the range's first slot may be where the two overlap
   (`s44_c_unequal_fail.vpr`; the audit's `abl/E1_unequal_own_amounts.vpr` and `abl/E3_two_reads_pass_both_own_amounts.vpr`
   verify with it). **Listed limit:** a caller whose holding over the range is unequal for another reason (a callee
   kept a fraction of one element of a local array) gets a false `permission` (pinned; question 238, restated with this
   scope). S44 (c): this amount needs no
   case split and no sum row at the call; `(a, a)` to a `reads_range` pair, overlapping and disjoint read ranges, a
   caller holding the range whole or borrowing it, and a write after the call all verify; the unequal-fractions twin
   is one error.

7. **Entry.** The liveness case splits of 0037 item 9 include range atoms. No separation is asserted for ranges (0028
   item 11).

8. **Calls** (`call_atoms`; every row atomic, under the atom's guard).
   - **The base and the count** of each range atom are first bound to locals (S44 F-b1: Silicon refuses a trigger
     `loc(block_of(e), k)` whose `e` holds `+`, "not a valid trigger"; audit round 1, S4: a count `*q` would put a heap
     read into a marked row's bounds, the premise of 0059 item 7's single reason), and every row below reads those
     locals: no row's base, bounds, count or amount reads the heap.
   - **First, the shape row** `!(p == null || !WF(p) || n < 0 || len(block_of(p)) < offset_of(p) + n)` (`permission`:
     "calls `f`, which needs to own `a[0..n)`, without owning it"; `reads_range`: "… to read …"), before every other
     row of the atom, the liveness row included. S44 (b) took the table's "If not" branch: in the planned order (the
     liveness row and the amounts, then the marked rows, then the shape row) a null pointer is two errors, the
     liveness row and then the shape or the marked row; with the shape row first every wrong argument built from a
     local array is one error (a null pointer, a negative count and a range too long for the array end the path at
     the shape row, which then reports the too-long range). A callee that forwards `(p, n + 1)` of its own range
     gets two errors, the shape row (`len(block_of(p))` is unknown there, so its assumption is consistent) and then
     the first marked row: two real cases (the object may end at `p + n`, or go on without the callee holding it), as
     S43 N1; the tests expect both. So does an argument the caller holds nothing of (an unconstrained pointer
     parameter passed beside a local array): three errors, the shape row, the liveness row and the first marked row
     (null, dangling, or another object the caller does not hold; audit round 1, S7, `abl/D1_unconstrained_arg.vpr`).
     *(An order the table does not name, the marked rows before the shape row for
     `owns_range`, gave one error everywhere in S44 (b) (`s44_b_caller_orderD.vpr`); it is not built.)* The built
     order is pinned by `s44_b_*_orderB*`, the `s44_c_*`, `s44_d_*`, `s44_e_*`, `s44_h_*` and `s44_i_*` fixtures
     (both kinds; S44's addendum), with `reads_range`'s null and too-long twins one error each; the unsuffixed
     `s44_b_*` files keep the planned order as the table's evidence (its two-error null twins). The separation rows
     follow every atom's shape, first-slot and liveness rows and precede the amounts; a placement row (item 10)
     follows the call's shape rows.
   - **Before the amounts**, with the cell atoms' rows: for `reads_range`, `!(n > 0 && (perm(F.v_T) == none ||
     perm(F.d_T) == none))`, and for both kinds the liveness row (`permission`).
   - **Separation**, with the cell pairs: for a pair of one field with at least one `owns` or `owns_range`, a range and
     a cell `x`: `!(block_of(x) == block_of(p) && offset_of(p) <= offset_of(x) && offset_of(x) < offset_of(p) + n)`;
     two ranges: `!(block_of(p) == block_of(q) && 0 < n && 0 < m && offset_of(p) < offset_of(q) + m && offset_of(q) <
     offset_of(p) + n)` (`permission`: "passes `a[0..n)` and `b[0..m)` to `f`, which may be one object, once to own
     it").
   - **After the amounts, per range atom, in this order:** (1) the two marked rows, value then flag, at `write` or
     `amt_k`, inside `if (g) { }` (`permission`, the shape row's message); (2) the flags over the range
     (`uninitialized`: "passes `a` to `f`, whose parameter `p` needs every element of `p[0..n)` to have a value"; none
     for a marked parameter); (3) the type's range over the range, under the flag (`permission`, as the cell's range
     row). A failed marked row ends its path (S43 G2), so an unowned range is one error.
   - **Sums:** the liveness sums include range atoms; a `reads` cell's sum gains `(g_r && x in range r ? amt_r : none)`
     per read range (S44 (d): the cell inside the range verifies, and an `owns` cell inside an owned range fails the
     separation row alone); no range sum row at a call (S44 (c)).
   - The explicit rows cover every conjunct of item 4, so Silicon's own precondition check never fails first. **Why no
     range sum row is needed at a call** (audit round 1, S5: the marked rows bound each amount, not the sum of two read
     atoms at a slot they share). Every read amount of a call is at most `M / (N + 1)`, with `M` at most `perm(L)` and
     at most each of the caller's own read amounts of the field (item 6). For a slot the caller holds, `perm(slot) >=
     M`: in a block the caller holds through its liveness cell at write strength (a local array, an allocation), every
     taker removes from a slot at least what it removes from `L` (a `reads` or `reads_range` taker the same fraction
     from both, an `owns` taker the whole slot), so `perm(slot) >= perm(L)` wherever the slot is held at all; in a block
     the caller holds only through its own atoms, every held slot holds at least one of its own amounts. The `N` amounts
     of one call that meet at a slot then sum to at most `N · M / (N + 1) < perm(slot)`, and Silicon's exhale of the
     callee's quantified `requires` succeeds wherever the marked rows passed. A cell atom of an `owns` kind inside a
     range is refused by the separation row before.

9. **Returns** (`atom_returns`), for each `ensures` range atom under its guard, after the separation rows of the
   `ensures` pairs: the marked rows (`permission`: "returns without owning `p[0..n)`, which its contract promises";
   `reads_range`: "… without the permission to read `p[0..n)` it borrowed"; where two read atoms of one field may
   overlap, the sum form, a marked row whose amount depends on the slot, `acc(loc(B, k).f, amt_i + (in_j(k) ? amt_j :
   none))`: S44 (e), the table's "Expected" branch; with per-atom rows only, a callee that kept a fraction passes them
   and Silicon's own postcondition check fails, an exit 3); the shape row (`permission`, the marked rows' message:
   audit round 1, S1 — on the path where a guard that does not imply its partner's leaves the range empty, the marked
   rows pass and an `internal` shape row would be the first to fail, an exit 3 on a user's wrong contract,
   `abl/C2_body_guard_mismatch.vpr`; where the contract is right it holds by the entry facts); the flags (`permission`; for `must_init` `postcondition`, "returns without
   initializing every element of `p[0..n)`, which `cerastes::must_init` on `p` promises"; for `ref_to_uninit` the
   `internal` row `old(flag) ==> flag` under the quantifier); the type's range; the liveness sum. A `reads` cell's
   return sum gains `(g_r && x in range r ? amt_r : none)` per read range, as at a call (audit round 1, S6; the range's
   own return rows rely on it for the slot they share, `s44_g_loops.vpr`).

10. **Accesses, sequencing and placement.** `p[i]` in a body keeps 0059 item 4's rows; under a range atom the bound
    follows from the atom's pure line and `0 <= i && i < n` (an unprovable index is two errors, S43 N1). A call whose
    callee holds a range, beside an access or another memory call in one full expression, gets 0039 item 4's rows with
    the range forms of item 8's separation (`unsequenced`). A callee's `owns_range` reaches its field in `LoopWrites`
    (0038 item 3's havoc rule).

11. **Loops** (0059 item 8, for a parameter's range).
    - **Places.** Each `requires` range atom of a pointer parameter the function never assigns is a range place: base
      `p`, bounds `offset_of(p)` and `offset_of(p) + n` (entry values), the amount `write` (`owns_range`) or the
      function's own `amt_k` (`reads_range`), the guard `g`.
    - **Rows:** 0059 item 8's, with the place's bounds, amount and guard: the marked entry rows ("this loop begins
      without the permission to every element of `p[0..n)`"), the label, the conjuncts (permissions, flags kept, the
      type's range, and the values kept when no write reaches the field **or** the place is read-only), the marked
      back-edge rows, the liveness snapshot and row as a parameter's cell place has them.
    - **Beside cell places:** a parameter's cell place gains, for every range place `r` of its field, the guard "not
      inside `r`" on every conjunct and on its back-edge row, with a case split before the snapshots (a cell inside a
      range is carried by the range; what the function holds beyond it stays in the frame; S44 (g): the unguarded twin
      fails on entry).
    - **Entry case splits** (S44 F-g1): `if (lvloc(block_of(p)) == lvloc(block_of(q))) { }` for each pair of range
      places, and for a range place against each cell place, before the snapshots (item 7's splits, at the loop):
      without them a loop over two ranges fails its liveness conjunct on entry.
    - **Order of the conjuncts:** the place's permission conjuncts come before the user's invariants, so an invariant
      that reads the range (a search's progress clause) is well-formed (S44 (g)).
    - **The count's type range** (`n <= INT_MAX` for an `int` count) must be known to a loop that stores its index
      (S44 (g)); lowering already writes every integer parameter's range as a `requires` of its method (0010), so
      nothing is added.
    - **Reads in invariants and measures:** `p[e]` of any pointer and `a` used as a pointer (`q == a`) are accepted. A
      read is framed by a range place (the interval test of item 5) or a cell place; unframed, it is `permission`
      ("this loop invariant reads `p[e]`, which this loop does not hold"); a named array keeps e1a's `out-of-bounds`
      row; the flag obligation is 0038 item 6's.
    - **Not built** (question 237): a range atom in an invariant is rejected; a loop in a function that assigns its
      range parameter has no range place (a pinned false alarm).
    - **S44 (g)**, the table's "Expected" branch: a fill loop over an `owns_range` parameter, a search over
      `reads_range` (values kept by `old[lh]`) with a `return` in its body, a copy over two range parameters, a guarded
      range, two overlapping read ranges (with no exclusion: the fallback chain was not entered) and a range beside a
      `reads(*q)` cell inside it all verify (1.9 s each for the fill and the search, 3.3 s for all six loops).

12. **Markers** (0042 items 4–8). A marker's atom may be `owns_range(p, n)`. `ref_to_uninit`: no flags on entry, the
    range under the flag, `old(flag) ==> flag` on return, no flags row at the caller. `must_init`: every element's flag
    on return. `reads_range` beside a marker and a contract read `p[e]` of a marked parameter are rejected as their cell
    forms are. `--strict-init` reads an array argument as it reads `&x`. A `must_init` filler that fills with a loop
    cannot state its invariant before e1c (a pinned false alarm; question 239).

13. **`owning(n)` on a parameter** (0029 item 1, 0040 item 3). `p == NULL || owns_range(p, n)` first in both clause
    lists, `n` as P79 (c) reads it (the argument is an expression over `enk_param_ref {k, levels_up 1}` nodes, the same
    on every declaration whatever the parameters are named, so a parameter is resolved by position; a later parameter
    is the front end's #20), merged with an explicit `owns_range(p, n)` of the same count as 0040 item 3 merges
    `owns(*p)`; declarations must agree on the count (`annotation_mismatch`); the argument gets the annotation checks
    (`check_attribute`). `CERASTES_OWNING_N(n)` is `[[cerastes::owning(n)]]`, and nothing in another compiler. The
    other targets stay rejected, their messages naming M4.5e2.

14. **`restrict`** (0037 item 2). `restrict_backed` takes `owns_range(p, n)`, unguarded or after `p == NULL ||`, in
    both lists, as the backing atom; every other parameter of the pointee type needs an `owns`, `reads`, `owns_range`
    or `reads_range` of its own.

15. **Array parameters.** `T a[]` and `T a[N]` are pointer parameters; a written bound is checked as a local array's is
    (0059 item 2) and otherwise ignored; `sizeof` of one is rejected like the compilers (a new tag); the matrix's other
    warning shapes likewise (Step 0, M; question 240); `[static N]` and VLA parameters stay rejected (M8).
    - **The IL** (P79 (b)): a parameter's `declared_type` keeps its array type and bound, as a local array's does. In
      C++ every parameter bound is typed `unsigned long` under a compiler-generated cast over the written tree, a
      literal `[3]` included (F2), so the bound is read under that cast and through its constant's own `expr`. With two
      declarations of different bounds only the definition's bound is on the routine; an earlier declaration's is
      reached through its secondary declaration (F3), which `array_parameter_mismatch` reads. `sizeof(a)` is a constant
      whose `expr` is `enk_sizeof` over the parameter. A marker leading or after the declarator-id marks the
      parameter as for `int *p` (P79 (f)); after the bound it is the front end's #1835.
    - **R1** (Step 0, the base build): `int a[]` and `int a[3]` with `owns(*a)`, `const int a[]` and `const int a[3]`
      with `reads(*a)`, in both spellings and modes, verify and compile cleanly with clang and GCC (`-Wall -Wextra
      -Werror`); each twin fails. They are Part E's positives.
    - **The matrix** (Step 0, M; clang and GCC, both modes, no flag, `-Wall`, `-Wall -Wextra`):
      - `sizeof(a)` and `sizeof(a) / sizeof(a[0])` of an array parameter warn without a flag in every compiler
        (`-Wsizeof-array-argument`; clang also `-Wsizeof-pointer-div` on the quotient): rejected,
        **`sizeof_array_argument`**.
      - Two declarations of one function whose parameter bounds differ, or a bound against none (`int a[3]` and `int
        a[]`), warn at `-Wall` in clang, clang++ and GCC's C (`-Warray-parameter`, GCC's `-Warray-parameter=`; g++ is
        silent); `int a[3]` against
        `int *a` warns only in GCC's C. By 0020 item 5's union rule both are rejected in both modes (as 0039 item 3
        rejects C++-defined shapes), **`array_parameter_mismatch`**.
      - A local array shorter than the bound passed to `int a[4]` warns at no front-end tier (GCC's C only at `-O2`,
        `-Wstringop-overflow=`): accepted, no tag, listed for M8's sweep. `int a[0]` and `int a[const 3]` parameters
        are silent: accepted (the bound is ignored), listed for M8.
    - **VLA parameters on a declaration without a body** (`void f(int n, int a[n]);`, `int a[*]`) verify today (R2;
      only definitions were rejected, by C's VLA error or EDG's #411/#890): the plan's scope rejects them, so item 15's
      rule covers every declaration, at the site and with the reach of the `[static N]` rejection, with a pin. It
      rejects programs that verify today, as question 230 did for `[static N]`: question 241, built as its
      recommendation (reject; M8 lifts both).
    - **Arrays of arrays as parameters** (`int a[][3]`, adjusted to `int (*)[3]`) are rejected today as a pointer to an
      array; since their `declared_type` is an array, the rejection is kept explicit, naming e1e, with a pin (audit
      round 1, N11).

16. **The lint.** New admitted shapes, each with twins: a range atom's quantifiers in `requires` and `ensures`
    (permission with or without a `Perm` formal as amount, flags, range or `WF`, `old(flag) ==> flag`); marked rows with
    `offset_of` bounds, an amount, and inside `if (g) { }`; the pure quantified `assert` rows of items 8 and 9; the loop
    conjuncts with an amount and a guard; the amount expression of item 6. Nothing else that holds a `forall` is
    admitted.

17. **Categories:** none new. New diagnostic tags only from item 15: `sizeof_array_argument` and
    `array_parameter_mismatch`. **Facts:** P79, S44 (`internal/il-inventory.md`, chapter M4.5e1b): S44's stop rules
    (a second Silicon error after a marked row, `qp.not.injective`, an outcome no table names) never fired.

## Facts and the branches taken

Each probe and spike of the plan's section 5, its outcome (`internal/il-inventory.md`, chapter M4.5e1b), the branch of
its decision table, and the item it revised. An outcome outside its table stops the session and asks the owner.

| Row | Outcome | Branch taken | Item revised |
|---|---|---|---|
| P79 (a) the count and its conversions | a generated cast is `compiler_generated` with no `expr_range`, a written one has an `expr_range` | Expected: the encoder reads the operand under generated casts only | 3 |
| P79 (b) array parameters, bounds, `sizeof(a)` | `declared_type` keeps the array type and bound; C++ bounds are `unsigned long` under a generated cast (F2); a first declaration's bound only through its secondary declaration (F3); `sizeof(a)` an `enk_sizeof` over the parameter | Expected, with F2 and F3 | 15 |
| P79 (c) `owning(n)`'s argument | `enk_param_ref {k, levels_up 1}`, the same on every declaration whatever the names; a later parameter is EDG's #20 | Expected: resolved by position (F4: one `interpret_param_ref` rule) | 13 |
| P79 (d) `old(p[i])` | C `address_of(subscript(p, i))`, never folded; C++ a reference bound to the subscript | Expected: read as the element | 5 |
| P79 (e) `p[i]` in contracts | `eok_subscript(param p, index)` in `requires`, `ensures`, `decreases`; the index may be the result binder | Expected | 5 |
| P79 (f) a marker on `int a[]` | leading or after the declarator-id: a marked `int *` parameter; after the bound EDG's #1835 | Expected | 12 |
| S44 (a) the callee | the guard outside each quantifier verifies, marked rows in `if (g) { }`; `p[n]` two errors (N1); a write under a false guard two (`null-dereference`, then `lifetime`: 0059 item 4's rows on a pointer nothing is known of; the addendum added the null row the first fixtures lacked) | Expected | 4 |
| S44 (b) the caller on a local array | every correct program verifies; in item 8's planned order a null pointer is two errors | **"If not": the shape row first**; one two-error twin (a forwarded `(p, n + 1)`) expected; an extra order outside the table recorded, not built; F-b1 the base in a local | 8 |
| S44 (c) `reads_range` amounts | the first-slot amount suffices with no split or sum row; the unequal-fractions twin one error | Expected | 6, 8 |
| S44 (d) a range beside a cell | the separation row alone fails the `owns` twin; a `reads` cell inside a read range verifies | Expected | 8 |
| S44 (e) sum rows | the sum row at returns verifies; the kept-fraction twin fails on it alone | Expected | 9 |
| S44 (f) `f__wf` and the contract's reads | item 5 verifies as written; the twins fail at the framing assert | Expected | 5 |
| S44 (g) loops | the six loops verify, overlapping read ranges without exclusion; F-g1 the entry liveness splits and the cell's guard | Expected | 11 |
| S44 (h) markers | the filler, the `uninit` caller and their twins as item 12 says | Expected | 12 |
| S44 (i) placement | item 10's rows verify | Expected (cut 2 not needed) | 10 |
| S44 (k) cost | two ranges, two loops, three calls 4.5 s; S43 (h)'s scale program rebuilt at K = 16: cells 6.9 s, an array 10.4 s, a range call 11.8 s (load 3–4) | recorded | — |
| Step 0 R1 array parameters today | all eight forms verify, native-ok | "the survey's reading holds": Part E's positives | 15 |
| Step 0 R2 today's diagnostics | every range probe fails at the atom; VLA parameters on a bodiless declaration verify | the baseline of Flips B; VLA declarations rejected per section 1 | 15 |
| Step 0 M the compiler matrix | two shapes warn (`sizeof`, mismatched bounds); a shorter array and `a[0]`, `a[const 3]` silent | the plan's rule: two tags, the rest listed for M8 | 15 |
| Step 0 R3 the rejections | 1,974 runs pass on the base (7 Linux-only skips) | the baseline of section 9 | — |

## Consequences

- A function can own or read `p[0..n)` and say what it does to the elements; callers pass whole arrays, sub-ranges and
  borrowed ranges, with each missing permission, short range and uninitialized element one error at the call.
- Loops over a parameter's range carry it without a written invariant, as e1a's loops carry local arrays.
- `reads_range` takes its amount from the range's first cell, lowered by the caller's own read amounts: a caller whose
  holding over the range is otherwise unequal (a callee kept a fraction of one element) gets a false `permission`
  (question 238).
- A contract whose `ensures` returns a range on a path where its `requires` did not take it is reported in `f__wf`, as
  for cells (0039 item 7).
- Programs accepted before e1b keep their Viper and maps byte for byte.
