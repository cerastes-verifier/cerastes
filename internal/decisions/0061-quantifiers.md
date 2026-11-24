# 0061 — Quantifiers (M4.5e1c)

- **Status:** draft (2026-11-23, M4.5e1c), written before code (invariant 6) from the plan's section 6
  (`.claude/plans/m4-5e1c-quantifiers.md`, approved 2026-11-23); to be revised by P80's and S45's decision tables (the
  plan's section 5; `internal/il-inventory.md`, chapter M4.5e1c; fixtures `tests/viper-smoke/memory/s45_*.vpr`), by
  Step 0's runs, by advisor checkpoint 1 (the guards of item 7's fact (d), the one-field restriction of facts (b) and
  (c), `f`'s `bool` result, reads through marked parameters, and an S45 addendum: two read classes through `f__wf`, the
  one-term workaround, the guarded fact), by audit round 2 (Opus, on the snapshot `2dfc88d` of Parts A and B: no soundness hole; one
  blocking item, the lint refusing the type-range fact of a formal with no atom of its own in an accepted program's
  Viper, fixed by emitting facts (a)-(c) only for formals with an atom, item 7; one should-fix item, six pins of item
  4's refusals; notes 3-10 taken where they name a fix; `build/m4.5e1c/auditor-report-2.md`) and by audit round 1 (Opus,
  on Commit 1 `cbd3386`: every S45 fixture and the
  decisive P80 rows reproduced; two blocking items, both taken: B1 a trailing argument's own obligations (item 3), B2
  the lint admitting the unguarded keep fact and a state-crossing tie (item 11); eleven should-fix items, all taken:
  typed ghost functions (item 7), the guard's `i + 1` (item 6), the other forms of `f` (item 3), every
  `cerastes::forall` under cut 3 (item 2), marked sites (item 7), the lint against the record's (b) and (d), the
  guard-false fixture, the unsound-evidence fixture, the check's flag-field row (item 10), reads not indexed by the
  binder (item 7), `&` of `f`'s parameters and `f`'s signature (items 3, 4); notes 14 (item 9), 15 (item 11), 16 and 17
  (item 12) taken; `build/m4.5e1c/auditor-report-1.md`). Owner decisions:
  - At M4.5e1's planning (2026-11-11): **226** `cerastes_forall(f, free...)` names a `pure` function whose body is one
    `return E;`, inlined at the site; no Viper function is emitted for `f`, and a call of `f` anywhere else stays
    rejected until e1d. **232** `cerastes_old(p[i])` is allowed in the body of a `pure` function that only an `ensures`
    names. **239** the `must_init` loop filler stays pinned; e1c gives it a verifying sibling.
  - At M4.5e1c's planning (2026-11-23): **238** is a false alarm, not unsoundness; e1c runs its spike (S45 (r)) and
    the fix is M4.5e1f's; **242** `owns_range`'s row order D is built with 238's fix, in M4.5e1f; `cerastes_old` in a
    loop invariant is in e1c, as its last part and its first cut (item 12).
  - Answered before the session (2026-11-23, inline, handoff §8), each the recommendation: **247** an obligation inside
    `f` is reported at the `forall` site with a note at the operation in `f` (item 7); **248** a trailing argument
    reads no memory (item 3); **249** only positive positions: a nested `forall`, `!forall` and `exists` stay rejected,
    **and M4.8 must handle them** (the owner: "Keep rejected until M4.8 and document that this needs to be handled in
    M4.8"; `plan.md`'s M4.8 row, item 1's table, the Annotations page); **250** the three false-alarm pins keep their
    names and expectations (item 9); **251** under a binder `old` is written `cerastes_old` in both modes,
    `cerastes::old` in a `pure` body stays the front end's `old_outside_ensures` until M7 (item 8); **252** a contract
    on a `pure` function is rejected only where a `forall` names the function (item 3).
  - In this session (2026-11-23), each the recommendation unless said: **253** the false alarm of a multi-class
    quantifier used where only one class's term occurs (S45 (b), "recorded") is pinned and **M4.5e2 lifts it** (the
    owner chose M4.5e2 over the recommended M4.8; item 6); **254** the skipped-write twin's two errors are expected, and
    each loop also gets a one-error twin (item 9); **255** (raised by il-analyst-2 in Part B): the ghost functions of
    `f__wf` are method-qualified (item 7), and four fail-closed narrowings of the lowering are listed limits: a `forall`
    on the left of `||` (M4.8), and in a multi-class `E` a memory-reading guard, arithmetic combining two classes' reads,
    and a converted index (M4.5e2) (item 1). Cut 3 was taken by P80 (b)'s table without asking (item 2).
  - Built overnight as recommended (the owner's instruction of 2026-11-24, handoff §8, each for review): **256** split
    point 1 (Parts A and B +2,483 lines): continue to Part C with a ceiling for its own delta; **257** a read class read in
    both states gets a trigger set per state in `f__wf` (item 7); **258** the guard of a loop's obligation line and
    framing `assert` keeps the earlier quantified invariant clauses, and the lint admits them (items 9, 11); **259**
    fail-closed narrowings in loops and checks, listed for M4.5e2 (item 1); **260** split point 2: **cut 1**, item 12
    (`cerastes_old` in loop invariants) moves to M4.5e2; **261** (audit round 3, B1: an exit 3 on wrong programs) in a
    loop invariant or a `check`, a multi-class `E` with arithmetic on a read of a class other than the first is
    rejected (M4.5e2; contracts route such rows by their ghost applications and keep it); **262** (audit round 3) in a
    loop invariant or a `check` a `forall` stands only as a top-level conjunct (a `check` is split per conjunct as an
    invariant is), and under `||` or in a `?:` arm there it is rejected (M4.8, with question 249's positions).
  - Standing: 69 (no EDG work), 77 (one context), 160 (a new test's expect line corrected after its first run, listed in
    status), 176 (the owner handles CI).
- **Milestone:** M4.5e1c.
- **Amends:** 0006 item 9 and its `old` rule; 0011 items 4, 7 and 10's messages; 0028 item 2; 0029 items 3 and 4; 0037
  item 7; 0038 item 6; 0041 item 12; 0059 item 8; 0060 items 1, 11, 12 and 16.

## Context

Records 0059 and 0060 gave arrays, ranges in contracts, and loops that carry them, but nothing can be said about every
element at once: a loop that fills an array cannot say which elements it has written (three pinned false alarms,
`array_loop_fill_then_read_false_alarm.c`, `array_loop_self_write_false_alarm.c`, `range_marker_pin_loop_filler.c`), a
`max` cannot state its result, and the `carrays.c` port holds single-element facts only. Record 0029 item 4 gave C its
quantifier's spelling, a named function in the lambda's place, and left its encoding to M4.5e1; question 226 decided
that the function's body is inlined at the site, so no `pure` body becomes a Viper function before e1d. This record
gives `forall` its meaning in contracts, loop invariants and checks, the `cerastes_old` it needs under a binder and in
loop invariants, and the rows that keep each of its obligations an explicit error in source terms (0028 item 11). Facts:
P80 (the IL of the two spellings, the `pure` body, the front end's checks today) and S45 (Silicon on the shapes of this
record), in `internal/il-inventory.md`, chapter M4.5e1c; the table at the end maps each outcome to the branch of the
plan's decision table it took.

## Decision

1. **Scope.** In: `cerastes_forall(f, free...)` (a new declaration, both modes) and `cerastes::forall(f, free...)` with
   a function name, in a function's `requires` and `ensures`, a loop's `CERASTES_INVARIANT` and a `CERASTES_CHECK`; the
   reader of `f`'s body; arity and type checks; element reads under the binder with their framing, bounds and flags;
   `cerastes_old` under the binder in an `ensures`; `f__wf` for quantified contracts; calls and returns; quantified loop
   invariants (the fill loop, the `must_init` loop filler, max, `max_index`, search, sortedness of neighbours, copy);
   `cerastes_old` in loop invariants (item 12); the `carrays.c` port in full; the tutorial chapter "Arrays and
   quantifiers". Everything else stays rejected, each with a diagnostic naming where it is lifted and a pin:

   | Construct | Lifted by |
   |---|---|
   | the lambda form `cerastes::forall([](int i, …) { … }, …)`; `cerastes::forall(f, …)` with a function in verify mode (cut 3, item 2: write `cerastes_forall`) | M7 |
   | `f` a `predicate` (M4.5f1); `f` with a `requires`, `ensures` or `decreases`; a call of a `pure` function anywhere, in `E` too; recursion | e1d |
   | in a loop invariant or a `check`, a `forall` that is not a top-level conjunct (under `\|\|`, in a `?:` arm; question 262); a `forall` inside `E` (nested); under `!`; as an operand of `==` or arithmetic; in a `?:` condition; in the guard of an atom; in a function's `decreases` or a loop measure; under `old`; in a `do … while (0)` invariant; an `exists`; on the left of `\|\|` (`forall(…) \|\| g`: lowering writes `\|\|` as `!left ==> right`; `g \|\| forall(…)` is accepted; question 255) | M4.8 (question 249: M4.8 must handle them) |
   | an `E` with no element read indexed by the bound variable; an index that names the bound variable in another form than `i`, `i + c`, `i - c` (`a[2 * i]`, `a[n - i]`, `a[b[i]]`), or under a conversion; in an `E` with more than one read class, a guard that reads memory or arithmetic combining two classes' reads, and in a loop invariant or a `check` arithmetic, a unary operator or a conversion over a read of a class other than the first, the first being the first read in `E` (a `char` element compared through C's promotion included; question 261) (`a[i] + b[i] < 100`; comparisons such as `a[i] <= a[i + 1]` and `p[i] == q[i]` are accepted; question 255) | M4.5e2 |
   | a quantified read whose base argument is not a bare pointer parameter (contract) or a pointer variable or named local array (loop invariant, `check`): `&a[1]`, `p + 1`, a loaded pointer, an address-taken pointer variable; in a loop invariant or a `check`, `*p` of an array argument and a pointer argument used as a value; in a `check`, a read the binder does not index and pointer elements under the binder (question 259) | M4.5e2 |
   | under the binder: structure members (`p[i].x`), array members, global arrays | e1e |
   | a trailing argument that reads memory (the read is written inside `f`; question 248) | M4.5e2 |
   | `cerastes::old` (the C++ spelling) inside a `pure` body: the front end's `old_outside_ensures` stays (question 251) | M7 |
   | `cerastes_old` in a loop invariant (item 12, cut 1, question 260) and in a `check` | M4.5e2 |
   | `reads_range`'s minimum amount (238), `owns_range`'s row order D (242) | M4.5e1f |

   The one message in `src/` that names M4.5e1c (`"cerastes::forall" (a quantifier: M4.5e1c)`) is lifted for the
   function form and re-pointed for the lambda form (M7).

2. **Spellings.** `<cerastes.h>` declares `cerastes_forall(...)` in both modes (C23's variadic form; `bool` and
   `noexcept` inside `extern "C"` in C++), beside the other `cerastes_` names; `cerastes::forall`'s declaration does not
   change. The encoder gains `SpecFn::forall`; both names join the name lists (`encode.cpp`'s `spec_fn`;
   `il_query.cpp`'s `is_spec_function_name`; `annotation_checks.cpp`'s `is_spec_name` and `is_protected_macro_name`;
   `src/driver/edg_args.cpp`'s `is_cerastes_name`; the reserved names of the Annotations page) and pass
   `declared_in_cerastes_header`. In code they are `spec_call_in_code`. P80 (a): the callee is `enk_routine
   cerastes_forall` and `f` is `enk_routine f` or `eok_address_of(enk_routine f)`, with no conversion node, in both
   modes. **Cut 3 (P80 (b), the table's "If not", taken without asking):** in `cerastes::forall(f, …)` an rvalue
   trailing argument (`n + 1`, a literal) reaches the IL as `reference_to(enk_temp_init → dynamic_init → expression)`,
   which `annotation_checks.cpp` refuses today (`unsupported_annotation_expression`, in `cerastes-fe` too), so no single
   peel rule reads every argument. In verify mode **every** `cerastes::forall`, in each of the four positions, is
   `unsupported_annotation` (audit round 1, S6): with a lambda, the lambda form's message re-pointed to M7; with
   anything else (a function, a function pointer), "write `cerastes_forall`" (M7). Pins: each position (`requires`,
   `ensures`, an invariant, a `check`, where today the call is `call_in_annotation` and no test pins it), and an rvalue
   argument (which keeps the front end's diagnostic). **`cerastes_forall` is the spelling in both modes**. A second peel rule would
   change `annotation_checks.cpp` and front-end expectations, which the plan fences (Flips E).

3. **The quantifier** `forall(f, e_1, …, e_m)`.
   - **Positions:** a function's `requires` and `ensures`, a loop's invariant, a `check`; as a conjunct, as the right
     operand of `||` or as an arm of `?:` (a positive position; the left operand of `||`, which question 249 allowed, is
     narrowed to M4.8 by question 255). Every other position is rejected (`unsupported_annotation`, item 1's M4.8 row).
   - **`f`** is named as `f` or `&f` and nothing else (audit round 1, S5): a lambda (`enk_lambda`), `+lambda`, a written
     cast (`(fp_t)g`, which can name a function of another signature), `*f`, a function-pointer parameter or variable, a
     comma and a `?:` are each `forall_function`, each pinned, the written cast in particular. `f` is a `pure` function
     (not a `predicate`), defined in a verified file of the unit,
     whose body is exactly one `return E;` (an extra block around it is read through; P80 (c)) and whose return type
     is `bool` (`_Bool` in C); no `requires`, `ensures` or `decreases`; not a member function, a template instance or
     variadic. In C the returned expression is under a compiler-generated conversion to `bool` (P80 (c): `eok_bool_cast`
     over an `int` comparison); it keeps its meaning, `E != 0`, and is never peeled away. Its first parameter, the
     **bound variable**, has an integer type other than `bool` and the enumerations; every other parameter is a `bool`,
     an integer, an enumeration, or a pointer to one of those or to a pointer. `f`'s parameters get `check_signature`'s
     rules (spec routines skip it today; audit round 1, S13): a VLA or `[static N]` parameter and `restrict` are
     rejected on `f` with the tags they have on any function. A function that is not such a function:
     **`forall_function`** (new tag; its message says which rule). A contract on `f` (question 252) and a `predicate`:
     `unsupported_annotation` (e1d, M4.5f1).
   - **Arguments:** exactly one per remaining parameter; the written argument's type (under at most one
     compiler-generated conversion: C's promotions, C++'s reference binding; P80 (e)) is its parameter's type up
     to top-level qualifiers, `T *` for `const T *` allowed: **`forall_argument_mismatch`** (new tag) otherwise. An
     argument reads no memory (question 248); `cerastes_old(…)` as a trailing argument is rejected
   (`unsupported_annotation`, M4.5e2, with the memory-reading arguments).
   - **An argument's own obligations** (audit round 1, B1). Each trailing argument is evaluated once, at the site,
     outside the binder. Its obligations (0010 item 11's: overflow, division, shift, conversion; in a loop invariant
     or a `check`, the flag row of a local declared without an initializer) are rows of their own at the site, before
     the quantifier, in evaluation order, under the site's `prior` and path and never under `E`'s path or the binder.
     They are written in the site's mode: an `f__wf` row in a contract, an `obligation_clause` line in a loop
     invariant, an `assert` in a `check`. Inside `E` the parameter stands for the argument's value. So `n + 1` that
     may overflow is reported whether or not `f` uses its parameter, and whether or not any `i` reaches the use.
     Tests: an overflowing argument whose parameter `f` never uses, and one used only on a path no `i` reaches.
   - A pointer argument whose parameter `E` subscripts with the bound variable
     is, in a contract, a bare pointer parameter; in a loop invariant or a `check`, a pointer variable that is not
     address-taken or a named local array (its decay collapsed to the array, so `inv_elem` and the array rows still
     match). The rest is rejected (item 1).
   - **Meaning:** for every value `i` of the bound variable's type, `E` with the other parameters replaced by the
     arguments, each argument read once, at the site, in the site's state.

4. **The reader** (encoder). `E` is encoded at the site, in the site's mode, with a substitution map from `f`'s
   parameters to the bound variable and the encoded arguments (`Ctx`; `variable()` consults it first; `f`'s parameters
   are `enk_variable`s mapped by position, P80 (c)). `E` may hold what an annotation expression of the site may
   hold, element reads `p[e]` and `*p` through `f`'s pointer parameters, and `cerastes_old` by item 8. Refused in `E`, at
   the site, each with the tag the same construct has in an annotation and a pin: a call (a `pure` function: e1d), an
   atom, a `forall`, an assignment, `++`, `--`, a comma, a global (0041 item 12 holds for the inlined body), and `&` of
   a parameter of `f` (`&i`, `&n`: the substitution maps a parameter to a value, which has no address;
   `unsupported_expression`, as 0038 item 6's `&x`; audit round 1, S13). The shape
   rules that reach annotation expressions (R5, R9) reach `E`; `static_assert_pure_body.c`'s exemption is unchanged. A
   `pure` function that no `forall` names is still not read (0011 item 4).
   P80 (d): the front end raises nothing on naming `f` as an argument in an annotation, on `cerastes_old(a[i])` in a
   `pure` body, or on a side effect in the body (the reader refuses it); `cerastes::old(a[i])` there stays
   `old_outside_ensures`. No front-end check changes.
   - **A read through a marked parameter** (0042 item 6, 0060 item 12) follows the rule the same element read has at the
     site: in a contract it is rejected (`unsupported_annotation`, 0042 item 6's message naming `f`) in a `requires`,
     under `cerastes_old` and in a `ref_to_uninit` parameter's `ensures`, and accepted in a `must_init` parameter's
     `ensures` (the flags are promised there; Step 0 R1: `ensures(p[0] == 0)` of a `must_init` filler verifies); in a
     loop invariant and a `check` it is accepted and carries the flag row (items 9 and 10; R1: a marked element read in
     an invariant has its flag obligation).

5. **VIR.** `Expr::Kind::Forall` (the bound variable's id and `IntType`, the body, the site's spelling `forall(f, a, n)`
   as `c_text`, `f`'s name, and the encoded trailing arguments as its `args`, in `f`'s parameter order), `Expr::Kind::Bound`
   (the bound variable) and `Expr::Kind::ForallArg` (its `Forall`'s id and an argument index: inside the body every
   parameter of `f` other than the bound variable, pointer bases included); `Builder::forall`, `Builder::bound`,
   `Builder::forall_arg`. Lowering emits each argument's obligations once before the quantifier (item 3) and uses its
   value inside. Ids need not be unique in the program: nesting is refused, so a `Bound` or `ForallArg` is always under
   exactly the `Forall` that `check.cpp` matches it to. `check.cpp` backstops items 3,
   6 and 8: positions, no nesting, no atom, call or write inside, `Bound` only under its `Forall`, the read classes'
   shape, `Old` inside only where item 8 or 12 allows. Until Part C lifts it, `check.cpp` refuses a `Forall` in a loop
   invariant or a `check` (fail closed between parts).

6. **Binders and triggers** (lowering; amends 0028 item 2).
   - A **read class** is a pair (base, constant `c`) of the reads `base[i + c]` in `E`. `E` has at least one class, and
     every index that names the bound variable has the form `i`, `i + c` or `i - c` (rejected otherwise, in the encoder
     and in `check.cpp`).
   - The first class's absolute slot is the binder `q0`, and `i` is `q0 - offset_of(base_0) - c_0`. The antecedent
     holds the type's range of `i`. Further classes (S45 (b), branch T2): a binder `q_j` each with `q_j ==
     offset_of(base_j) + i + c_j` in the antecedent and one trigger set over every class's term. T1 (one binder, the
     other reads as computed terms) does not finish on neighbours (`a[i] <= a[i + 1]`): its computed term is a new
     instance of its own trigger. **Listed limit:** a multi-class quantifier is instantiated only where the program holds
     a term of every class, so a correct program that uses such a fact where only one class's term occurs (a `requires`
     over `p[i]` and `q[i]`, a body that reads only `p[2]`) fails (`s45_b_t2_oneterm`): a false alarm, fail closed,
     never a proof; a read of `q[2]` beside it (the other class's term at the same slot) avoids it, a read of `q[0]`
     does not (S45 addendum, `s45_b_t2_oneterm_readq*`). Pinned; **M4.5e2 lifts it** (owner, 2026-11-23, question 253).
     Two classes through `f__wf` (variant M) take the same form over two ghost applications (`{ hvq_a_pre(q0),
     hvq_a_pre(q1) }`, `{ hvq_p_pre(q0), hvq_q_pre(q1) }`), and a later obligation resting on such a clause verifies
     (S45 addendum, `s45_d_m_nb_*`, `s45_d_m_eq_*`).
   - The trigger terms are `loc(block_of(X), q_j)` with `X` a variable (S44 F-b1): the formal in a method's contract,
     the bound local at a call (`bind`, extended from a range atom's base to every pointer argument a callee's
     quantifier subscripts), the pointer variable or the array's local in a loop and a `check`. In `f__wf` (variant M)
     no heap term exists: the trigger terms are the ghost applications `hvq_<p>_<state>(q_j)`.
   - User quantifiers bind `q0`, `q1`, … (the range quantifiers keep `k`); `LExpr::Forall` takes several binders and
     trigger sets of several terms.
   - **The guard of `E` has its own obligations** (audit round 1, S4): `i + 1` in `0 <= i && i + 1 < n` is an `overflow`
     at `i == INT_MAX`, a row like every other obligation of `E`. The neighbour fixtures of S45 (b) and (d) leave it
     out: their trigger results stand, and the encoder writes the row. Tests write `0 <= i && i < n - 1` under `n >= 1`.

7. **Meaning by mode.**
   - **The method's contract:** a clause that holds a quantifier is split per conjunct (`push_clauses`); a guard stands
     outside (`g ==> forall …`); each read under the binder is its own `ite` chain over the framing disjuncts (0037 item
     7, 0060 item 5), each disjunct with its atom's guard, with `old(…)` by item 8.
   - **Marked sites frame no quantified read** (0042 item 6's alias rule; audit round 1, S7): neither in the method's
     `ite` chain nor in `f__wf`'s permission row does a marked atom's site frame a pre-state read, nor a post-state read
     where the parameter is `ref_to_uninit` without `must_init`. Twin: `requires(q == p)
     requires(cerastes_forall(nonneg, q, n))` with `p` marked.
   - **A call:** the clause's `precondition` rows, after `call_atoms`' rows, in the caller's state, the bases bound; a
     clause that holds a quantifier gives one row per conjunct, as `push_clauses` splits it in the method (as built);
     the Viper call's own precondition stays the `internal` backstop. **A return:** the `postcondition` rows, split
     likewise. Callers learn an `ensures` from the method's postcondition, as today.
   - **`f__wf`** (amends 0037 item 7; S45 (d), branch **M**). The method stays heap-free and keeps every row it writes
     today (framing, 0010 item 11's obligations, partners by path, the overlap obligations, the ties, the
     `not_invalidating` rows). A read under a binder is the ghost function of its base parameter and state at the read's
     absolute slot, `hvq_<m>__<p>_pre(s)` and `hvq_<m>__<p>_post(s)` (`m` the method, since Viper names are program-wide
     and formal names repeat across functions; question 255), declared in one domain per `__wf` method with no axiom
     (`// cerastes:domain-ok 0061`). Assumed beside today's `facts_` (`requires` of the `__wf` method), each argued as
     0037 item 7 argues today's, each with **one trigger set per ghost application it relates** (S45 (d): a one-set form
     leaves the other application uninstantiated, and a correct contract fails):
     (a) **its type's range**, per state, `forall s :: { hvq_p_st(s) } lo_T <= hvq_p_st(s) && hvq_p_st(s) <= hi_T` (a
     location in one state holds a value of its type). A ghost function's result is the element's Viper type, `Int`,
     `Bool` or `Ref` (audit round 1, S3: `f` may read `bool *` and `T **` parameters), and its range is the one today's
     scalar ghost of that type has: an integer type's bounds, none for `Bool`, a pointer's as today;
     (b) **the scalar tie**, ground: a scalar ghost `hv_k_st` of a read `p[e]` (or `*p`, `e = 0`) of the same parameter,
     state and cell field, `e` any index expression the contract may write (keyed over formals, as today's ties are), is
     `hvq_p_st(offset_of(p) + e)` (one location in one state has one value; the ghost's state suffix is the function's);
     (c) **two parameters on one block**, per state, only between parameters whose pointees have the same cell field (0028
     item 1: another field is another location): `block_of(p) == block_of(q) ==> forall s :: { hvq_p_st(s) } {
     hvq_q_st(s) } hvq_p_st(s) == hvq_q_st(s)` (both functions are indexed by the absolute slot, so on one block they
     name the same locations);
     (d) **a `reads` place keeps its value**, only where the place is taken and returned: for a `reads_range(p, n)` atom
     of guard `g_r` with an `ensures` partner of guard `g_e` (0060 item 3's partner), `(g_r && g_e) ==> forall s :: {
     hvq_p_post(s) } { hvq_p_pre(s) } offset_of(p) <= s && s < offset_of(p) + n ==> hvq_p_post(s) == hvq_p_pre(s)`, the
     guards over the pre- and post-state ghosts as their atoms read them (a `reads` place is not written between entry
     and return: 0060 item 5's argument; on a path where either atom is off, another parameter may own the same slots
     and write them), and for `reads(*p)` the ground form at its slot under the same guards. S45 addendum: the guarded
     fact verifies (`s45_d_m_guard_fact*`; today's scalar fact already conjoins both sites' guards), and the
     unguarded one lets a wrong contract's `__wf` verify (`s45_d_m_unsound_evidence_unguarded_keep.vpr`, kept as the
     evidence, against `s45_d_m_guarded_wrong_contract_fail.vpr`).
     Every obligation under a binder is one row `prior ==> (forall q0 :: { hvq_p_st(q0) } range(i) && path ==>
     obligation)`, the `prior` outside the binder. **A read in `E` that the binder does not index** (`a[r]` in `a[i] <=
     a[r]`; audit round 1, S12) is a scalar ghost of today's form (0037 item 7; tied to its parameter's function by fact
     (b)); its framing row is quantified like `E`'s other obligations, over the first class's binder and trigger with
     `E`'s path, so that a read no `i` reaches is never asked. A failed row is assumed afterwards, as every `f__wf` row is. A
     contract without a quantifier keeps its `f__wf` byte for byte. Facts (a) and (c) are emitted only for formals that
     have an atom of that cell field in the method's contract (as built after the lint's run over the emitted Viper: a
     formal with no atom, such as `q` in `requires(q == p)`, has no element field the lint can check, and a read
     through it is framed by another parameter's atom or reported unframed; losing its range fact can only add a false
     alarm). **A read class read in both states** (`a[i]` and `old(a[i])`) has one trigger set per state, `{ hvq_m__a_pre(q0) }
     { hvq_m__a_post(q0) }`, on its clause and its rows (question 257): with its first read's ghost only, a later
     obligation over `owns_range` (no keep fact bridges the states) that names only the other state's read was never
     instantiated, a false alarm (`fq_both_states_pre_only`, now a positive with its twin). More trigger sets only add
     instantiations of assumed facts and clauses, which hold; each set is one ghost application over the binder, so
     none loops. (Variant H, a heap form with a bodiless `f__wfp`,
     also passed S45 (d); the table builds M, and H's boundary of rejections is not needed.)
   - **Obligations of `E`** (0010 item 11's, framing, and in loops and checks the bound and the flag): each is one
     quantified row over its own read class's binder (an arithmetic obligation: over `q0`), under the site's `prior` and
     the path inside `E`. It is reported **at the `forall` site** (question 247), in the operation's category, its
     message naming `f` ("in `le_at`: reads `a[i]` without owning it"), with a note at the operation's line in `f`.

8. **`old` under the binder** (amends 0006 item 9's `old` rule). `cerastes_old(p[e])` and `cerastes_old(*p)` in `E`
   (the C spelling, in both modes; `cerastes::old` there stays the front end's error, question 251). A `forall` whose `f`
   holds one is accepted where `old` is accepted: in an `ensures` (and by item 12 in a loop invariant); elsewhere
   `old_outside_ensures`, at the site. `cerastes_old` of a parameter of `f` (`cerastes_old(n)`) or of the bound
   variable is rejected with the generic message of `old` on a place other than a parameter (audit round 2, N7;
   pinned). The read is the entry state's, framed by the `requires` atoms: in the method's contract the post read and `old(…)`
   around the whole guarded read, each its own framing chain (S45 (e): `set_one` and a caller that reads two untouched
   elements).

9. **Loops** (amends 0038 item 6, 0059 item 8, 0060 item 11). A quantified invariant comes after the places' own
   conjuncts. Before it, in evaluation order, each over its read class's own single binder and trigger: the framing
   against the loop's places (`permission`, "this loop invariant reads `a[i]`, which this loop does not hold"), a
   quantified `assert` at entry and at the back edge as `framing_condition` does today; then one invariant line per
   obligation (`obligation_clause`): a named array's bound (`out-of-bounds`) and **the flag** (`uninitialized`): a
   quantified invariant that reads elements states that they hold values, at the head and after the loop. Each such line and framing `assert` is
   guarded by the earlier invariant clauses in evaluation order, quantified ones included (question 258). A loop-invariant
   clause that holds a quantifier is split per top-level conjunct into invariant lines of their own, in evaluation
   order, as `push_clauses` splits a contract clause (as built: `0 <= i && i <= n && cerastes_forall(…)` is the usual
   shape); each line keeps the clause's C line and category, and the driver prints one error per line, category and
   message, so a wrong scalar conjunct is still one error. In a loop invariant a `forall` stands only as such a
   top-level conjunct (question 262). The flag line of a read is guarded by its framing, as by a named array's bound
   (audit round 3, N7): an unframed read is one `permission` error, as its single-element form is. A read not
   indexed by the binder (`a[r]` in `max_index`'s `a[j] <= a[r]`) has its own framing and flag rows. A loop whose
   invariant holds a quantified read carries its places (`expr_touches`). No quantifier in a measure. This gives the
   fill loop, the `must_init` loop filler and the self-write loop their verifying forms (S45 (c): each about 2 s); the
   three pinned programs, which write no such invariant, fail as before (question 250). **Two errors where an element's
   flag and value are both unknown** (S45 (c)): a failed invariant line is assumed afterwards, like a failed `assert`;
   where the element is known unwritten (at entry, after the allocation) the assumed flag contradicts it and the path
   ends (one error), but at the back edge a body that skips the write leaves the flag and the value unconstrained, and
   both the flag line and the user's clause fail: two errors at the invariant, `uninitialized` and
   `invariant-preserved`, both real (the element holds no value, and not the stated one), as today's single-element
   invariant read already gives where the flag is unknown (Step 0 R1). The tests expect both, and each such loop also
   has a twin that writes a wrong value (one error) (owner, 2026-11-23, question 254). **An index outside a named
   array** (audit round 1, N14): the flag line's read takes the array's bound in its antecedent, as `inv_elem` guards
   today's single read, so the flag line does not also fail at a slot outside the array; the framing and bound rows
   report it, and Part C's twins expect the count Silicon gives (question 254's rule).

10. **`check`** (S45 (f), "Expected"). Per read class, in this order: for a pointer base the null row and the liveness
    row (`kq := perm(lvloc(…).lv)` bound first, then `forall q0 :: reach ==> kq > none`); the bound (`out-of-bounds`,
    a named array); the marked row `assert forall q0 :: { loc(B, q0) } reach(q0) ==> acc(loc(B, q0).v_T, wildcard)`
    (`permission`; Silicon accepts `acc(…, wildcard)` under a quantifier in an `assert`, where `perm(…)` is refused),
    and the same row for the flag field `d_T` (audit round 1, S11: the flag row reads it, as today's `deref` checks
    `perm(v) > none && perm(d) > none`); the flag row (`uninitialized`); `E`'s own obligations (0010 item 11's), over
    the binder with `E`'s path; then the fact (`check`), in this order as built (audit round 3, N5). A `check` holding a
    quantifier is split per top-level conjunct into asserts of their own (question 262). A failed marked row ends its path. Each twin is one error where
    the flags are known; where an element's flag is unknown (after a loop with no quantified invariant), the flag row
    and the fact both fail, as in item 9.

11. **The lint** (amends 0060 item 16). A user quantifier: binders `q<j>: Int`, one trigger set of `loc(block_of(X),
    q<j>)` terms with `X` a variable, a body with no `acc`, `perm`, `forperm`, `inhale` or `forall`; admitted in a
    method's `requires` and `ensures`, an `invariant` and an `assert`. The marked `wildcard` row only as an `assert`.
    `f__wf` (variant M): one domain per function, tagged `// cerastes:domain-ok 0061`, holding ghost functions and no
    axiom; in a `__wf` method a user quantifier's trigger terms are ghost applications, and the assumed facts over the
    ghost functions have exactly the shapes (a)–(d) of item 7, each with one trigger set per application it relates, the
    keep fact (d) under its guards. **What the lint checks of each fact** (audit round 1, B2: the first rules admitted
    the unguarded keep fact S45 proved unsound and a state-crossing tie that gives a vacuous `prior`): a keep fact is
    tied to a `reads_range` (or `reads`) `requires` atom of the method on the same formal that has an `ensures` partner,
    its bounds that atom's, its guard exactly the conjunction of the two atoms' guards as the `__wf` method reads them;
    a scalar tie's ghost has the function's state suffix; a type range is the element type's, from the method's atoms
    (`Int` bounds, none for `Bool`, a pointer's for `Ref`); fact (c) only between formals of the same cell field; the
    ground forms of (b) over any index and of (d) for `reads(*p)`. A user quantifier carries the bound variable's type
    range in its antecedent and holds no computed `loc` term (T1's form; audit round 1, N15). The unguarded keep fact
    and the state-crossing tie are must-fail self-tests. The ghost functions are named `hvq_<method>__<Viper formal>_pre` and `_post` (the lint strips and checks the method prefix), the facts'
    binders `s`, `s<j>`; obligation rows put the `prior` outside the binder. Each rule with twins (as built in Commit 1:
    six rules, 22 tests; widened since for method-qualified names, cell-atom fields and per-state trigger sets in
    `__wf` methods, where a set need not name a state its row's body reads, since triggers only restrict instantiation);
    **what the lint cannot check** (audit round 2, N5): that a scalar tie's ghost `hv_eK_<st>` is the read of that
    parameter at that index (ghost names do not encode the place); only the encoder's construction guarantees it. The
    existing pins of "a `forall` here is refused" (binders `k`, `i`, `x`) are unchanged (no
    Flips D). Nothing else that holds a `forall` is admitted.

12. **`cerastes_old` in a loop invariant: cut to M4.5e2** (cut 1, question 260; the plan's Part D). It stays rejected
    (Step 0 R2: two errors at the invariant, `unsupported_expression` and `unsupported_type`, in each shape; pins
    `old_loop_invariant_reject_{cell,elem,scalar}.c`, re-pointed to M4.5e2). P80 (f) (the IL shapes are an `ensures`'s)
    and S45 (g) (Viper's unlabeled `old(…)` in an invariant is the method's entry; an `OLDFRAME` obligation line) are
    what M4.5e2 builds on, with this design:
The C spelling; a place that `old` accepts in an `ensures`, rooted in a pointer parameter the function never
    assigns and never takes the address of (an address-taken parameter lives in a cell a callee may change; audit round
    1, N17), or a by-value parameter (its entry value); an index is read in the current state and reads no memory.
    Lowered as Viper's unlabeled `old(…)` (in a method, the method's entry state, also inside a loop invariant) of the
    chain over the `requires` sites; its framing is an invariant obligation line (`permission`: "this loop invariant
    reads the entry value of `p[i]`, which the function's `requires` does not own"). A marked parameter's place is
    refused as in contracts. Not in a measure, not in a `check`. Today each of these is two errors at the invariant
    (`unsupported_expression`, `unsupported_type`; Step 0 R2); every refused shape gets one message of its own.

13. **Categories:** none new. **Tags:** `forall_function`, `forall_argument_mismatch`. **Facts:** P80, S45.

## Facts and the branches taken

Each probe and spike of the plan's section 5, its outcome (`internal/il-inventory.md`, chapter M4.5e1c), the branch of
its decision table, and the item it revised. An outcome outside its table stops the session and asks the owner.

| Row | Outcome | Branch taken | Item revised |
|---|---|---|---|
| P80 (a) the C spelling | the callee `enk_routine cerastes_forall`; `f` as `enk_routine f` or `eok_address_of(enk_routine f)`, no conversion node | Expected | 2 |
| P80 (b) the namespace spelling | `f` is `enk_constant &f`; an lvalue trailing argument is one peel rule (`eok_reference_to` over `eok_lvalue_adjust`; an array binds as the array); **an rvalue argument (`n + 1`, a literal) is `reference_to(enk_temp_init → dynamic_init → expression)`, refused by today's annotation checks** (`annotation_checks.cpp`, `unsupported_annotation_expression`: "cannot check … an expression of kind `enk_temp_init`", exit 2) | **"If not": cut 3, taken without asking** (`cerastes_forall` in both modes) | 2 |
| P80 (c) the `pure` body | in the IL in both modes, an unreferenced `inline` one too; one `stmk_return`; C23 wraps the returned expression in a generated `eok_bool_cast` | Expected | 3, 4 |
| P80 (d) the front end today | naming `f`, the C `cerastes_old(a[i])` and a side effect in a `pure` body raise nothing; `cerastes::old(a[i])` there is `old_outside_ensures` | Expected: no front-end change | 4, 8 |
| P80 (e) arity and types | no front-end diagnostic in either spelling; in C a `char`, `short` or `bool` argument under one generated cast, in C++ the deduced types | Expected: the encoder's rule | 3 |
| P80 (f) `old` in a loop invariant | the shapes of an `ensures` | Expected | 12 |
| S45 (a) one read class, contracts | the callee uses the fact, the caller's `precondition` row after the atom's rows (base in a local), the guard outside; twins one error each, vacuity reported; 2.7 s | Expected | 6, 7 |
| S45 (b) several read classes | **T2**: every positive verifies, every twin one error, ≤ 5.2 s. T1's neighbour programs do not finish in 100 s. Recorded, not gating: T2 fails a correct program where only one class's term occurs (`s45_b_t2_oneterm`: a `requires` over `p[i]` and `q[i]`, a body that reads only `p[2]`), which T1 verifies | T2 | 6 |
| S45 (c) loops | the fill loop, the `must_init` filler, the self-write loop, max, `max_index` and search verify (about 2 s each), every twin one error **except the skipped write: two errors at the back edge** (the flag line and the user clause); the stores-`i + 1` twin and the entry twin one error each. The framing asserted at entry and back edge, the bound and flag invariant lines (the invariant-line framing verifies too) | programs Expected; **the skipped-write twin outside the one-error rule: asked, question 254** (both errors expected, a one-error twin added) | 9 |
| S45 (d) `f__wf` | **M** passes (1)–(6), one error per twin, vacuity reported; H passes too. The assumed facts need a trigger set per application they relate; the `prior` stands outside the binder (inside verifies too); a failed row is assumed afterwards; (7) recorded for e1d | M | 7, 11 |
| S45 (e) `old` under a binder | `set_one` and a caller that reads two untouched elements; twins one error | Expected | 8 |
| S45 (f) `check` | item 10's rows in order; `acc(…, wildcard)` under a quantifier in an `assert` accepted; twins one error each | Expected | 10 |
| S45 (g) `old` in a loop invariant | Viper's unlabeled `old(…)` in an invariant is the method's entry; an `OLDFRAME` invariant line; twins one error | Expected | 12 |
| S45 (k) cost | ≤ 4.1 s for the combinations; the K = 16 scale program 7.3 s, 14.6 s with an array and a quantified callee (14.1 s without the quantifier) | recorded | — |
| S45 (r) for M4.5e1f | `perm` under a quantifier in a guarded `inhale` is accepted; `range_call_reads_unequal_pin.c` with the minimum amount verifies; a slot at `none` is one error; the `inhale`s are consistent; order D gives one error where the built order gives two (`range_call_fwd_one_more.c`) | recorded; nothing built | — |
| Step 0 R1 the pins today; the `must_init` sibling's fact | the five pins and `static_assert_pure_body.c` as their expect lines; the 26 positives of the plan's section 8 each rejected at the `forall` only. The invariant `i == 0 \|\| p[0] == 0` in `range_marker_pin_loop_filler.c`'s loop is accepted, its read live and its flag row present (twins: `p[0] == 1` `invariant-preserved`; unguarded `p[0] == 0` `invariant-entry` and `uninitialized`), and the program still fails only at the pin's own `postcondition @ L13` (every element's flag needs a quantified invariant); a `must_init` function whose `ensures` reads `p[0]` verifies | the read is not refused, so the table's "If refused" branch does not arise: Part C has nothing to lift for it (the plan's "both verify" could not hold for the first program, whose `must_init` row needs item 9) | 9 |
| Step 0 R2 today's diagnostics | 99 programs, both spellings, every row of item 1 rejected; a `pure` function with a contract (or recursion) that no `forall` names verifies (question 252's premise); one message names M4.5e1c (`encode.cpp:10076`); `cerastes_old` in a loop invariant is two errors at the invariant (`unsupported_expression`, `unsupported_type`), in five more shapes too (a reassigned pointer parameter, a local array, a global, an assigned by-value parameter, `*p` through a reassigned pointer): three pins added (`old_loop_invariant_reject_{cell,elem,scalar}.c`), Flips A | the baseline of the Flips; item 12's rejections each get a message of their own | 1, 12 |
| Step 0 R3 the rejections | `scripts/rejections.py` on the frozen build: 2,327 runs (scalar 1,020, memory 976, vlibc 70 `rejected`; 261 front-end errors), none failing; e1b's 2,060 `rejected` runs plus the three pins' six | the baseline of the plan's section 9 | — |
| Step 0 M the compiler matrix | clean everywhere: a `CERASTES_PURE` function only annotations name (with a prior prototype, defined after its use), `cerastes_old(a[i])` and `cerastes_old(*p)` in a `pure` body, neighbours, two arrays; GCC's `-Wextra` warns on `0 <= i` for an unsigned bound variable (`-Wtype-limits`), `int` against `size_t` warns (`-Wsign-compare`), an unused parameter of `f` warns at `-Wextra`, a prototype-only `inline` function warns in GCC's C | the plan's rule: tests write `i < n` for unsigned binders, one signedness, every parameter used; nothing new rejected | 3 |

## Consequences

- A contract, a loop invariant and a `check` can state a fact about every element of an array or a range, through a
  `pure` function whose body is inlined; a loop that fills an array, `max`, a search and a copy verify with the
  invariant that says what they did.
- No `pure` body becomes a Viper function; e1d gives `pure` functions their own meaning and their calls.
- Every obligation inside `f` is reported at the `forall` site, with a note in `f`.
- Programs accepted before e1c keep their Viper and maps byte for byte.
