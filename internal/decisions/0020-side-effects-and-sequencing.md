# 0020 — Side effects inside expressions and C's sequencing rules (M4.3a)

- **Status:** accepted for M4.3a. Owner decisions (2026-10-22, M4.3 planning):
  - **M4.3 is split.** M4.3a (this record): assignments, compound assignments and `++`/`--` used as values, the comma
    operator, side effects in guarded operands and loop conditions, and calls sequenced by `,`. M4.3b (record 0021):
    calls in guarded operands and loop conditions, and unsequenced calls.
  - **Unsequenced modifications are rejected statically**, as the compilers' `-Wunsequenced` (clang) and
    `-Wsequence-point` (GCC) warn, by C's rules in both modes.
  - **Shapes the compilers warn about are rejected like the compilers** (the union of clang's and GCC's `-Wall`
    warnings, in both modes): an assignment used as a truth value without extra parentheses, a comma operand without an
    effect, a discarded result of an operator with a side effect inside, a side effect in an unevaluated operand. A pure
    discarded expression statement (`x + 1;`) stays accepted (0012 item 3).
  - **EDG warnings.** Verify mode may suppress an EDG warning number when cerastes rejects every shape it covers, so the
    problem is reported once. P34 found no such number (item 10). Owner, after P34: EDG's #187 and #174 keep printing
    on the shapes cerastes accepts, as #174 already does for `x + 1;` and #550/#177 do (handoff §11 question 20).
  - **`typeof` and `decltype` operands** are accepted and recorded (owner, after P33/P35): they are never evaluated, and the
    front end keeps them in no tree the encoder can reach, so cerastes cannot reject `decltype(x++)` as clang does (item 5).
  - **Retired** (pre-approved): `reject_comma.c`, `reject_assign_value.c`, `reject_chained_assign.c`,
    `reject_incr_in_expr.c`.
  - Handoff §11 question 22: a jump past a declaration *with* an initializer is refused in both modes (item 12).

  Facts marked (P*n*) and (S*n*) are probes and Silicon spikes, recorded with their evidence in
  `internal/il-inventory.md`, "M4.3a" (P33–P35, S17–S18).
- **Milestone:** M4.3a (2026-10-22).

## Context

0012 item 4 allowed a side effect only at the top of a full expression: an assignment, a compound assignment, `++` or
`--` as an expression statement, a `for` increment or a declaration's initializer. Everything else was rejected: an
assignment used as a value, a chained assignment, `x++` inside an expression, the comma operator. The reason was that,
with no side effects inside expressions and no state a callee could change, the order of evaluation of operands could
not change a value, so the encoder could evaluate left to right.

Ordinary C writes `while (i++ < n)`, `for (i = 0, j = n; i < j; i++, j--)`, `if ((x = a - 1) > 0)` and
`y = x = 0`. Allowing them brings in C's sequencing rules:

- **C** (C23 6.5p2, 5.1.2.4): if a side effect on a scalar object is unsequenced relative to another side effect on the
  same object or to a value computation using its value, the behaviour is undefined. The sequence points inside an
  expression are after the first operand of `&&`, `||`, `?:` and `,`, and before a call (after its arguments). The
  operands of the other operators, and the arguments of a call, are unsequenced. The side effect of an assignment is
  sequenced after the value computations of both operands (6.5.17.1p3), not after the side effects inside them.
- **C++17** sequences more: the right operand of an assignment before the left ([expr.ass]/1), so `x = x++` and
  `x += x++` are defined in C++ and undefined in C; `<<` and `>>` left to right; function arguments indeterminately
  sequenced rather than unsequenced ([expr.call]/8).
- **The compilers** warn about unsequenced modifications in `-Wall` (GCC's `-Wsequence-point`, clang's `-Wunsequenced`,
  which is on by default), about an assignment used as a truth value without extra parentheses (`-Wparentheses`), and
  about values computed and discarded (`-Wunused-value`, clang's `-Wunevaluated-expression`) (P35).

## Decision

1. **Accepted, in both modes.**
   - `x = e`, `x op= e` (the seven arithmetic and shift compound assignments), `++x`, `x++`, `--x`, `x--` **used as a
     value**, wherever code is evaluated: an initializer, a `return`, a call argument, an operand of any accepted
     operator, the condition of `if`, `while`, `for` or `do`, a `switch`'s controlling expression. A chained
     assignment `a = b = c` is one of them.
   - **The comma operator** `a, b`, where its value is used (`return (x = a, x + 1);`) and where it is discarded (an
     expression statement, a `for` loop's init-statement `i = 0, j = n` or its increment `i++, j--`). Its left operand
     may be a call of a `void` function (`note(i), i++`).
   - A side effect inside the right operand of `&&` or `||` or an arm of `?:` happens only when C evaluates that operand;
     one inside a loop condition happens at every evaluation of the condition.
   - **Calls sequenced by `,`** (`f(x), g(y);`, `y = (f(), g());`): the comma fixes their order, so 0012 item 6's reason
     for rejecting two calls that are not nested does not apply (item 7). Other pairs of calls stay as 0012 item 6 says
     until M4.3b.
2. **Meaning** (both standards agree for the scalar types cerastes accepts):
   - the value of `x = e` is the value stored in `x`: `e` converted to `x`'s type (a checked conversion to a signed
     type or an enumeration, a wrap to an unsigned type, 0010 item 5, 0018 item 5); the value of `x op= e` and of `++x`
     is the new value of `x`; the value of `x++` and `x--` is the old value. In C the result is a value of `x`'s
     unqualified type; in C++ it is the lvalue `x`, whose value read afterwards is the same stored value, since nothing
     else can change a non-volatile local in between (item 4);
   - the value of `a, b` is the value of `b`; `a` is evaluated first, completely (a sequence point).
   - These do not change: compound assignment and `++`/`--` compute as 0012 item 7 says; they stay rejected on `bool`
     and on enumerations (0012 item 7, 0018 item 8).
3. **C++ lvalue uses are rejected.** `(x = 1) = 2`, `++x = 3`, `++ ++x`, `(a, x) = 5`, `(c ? x : y) = 1` are C++ only (C
   refuses them); each is an assignment to something that is not a local variable and is rejected as before
   (`unsupported_expression`), at the assignment.
4. **The sequencing rule** (`vir::sequence_problems`, rejection `unsequenced_modification`, at the modification).
   - For every full expression, each operand has the set of locals it may write (W) and read (R). Two operands of one
     operator conflict when one writes a local that the other reads or writes: `W(p) ∩ (W(q) ∪ R(q)) ≠ ∅`.
   - **Unsequenced** (checked): the operands of every arithmetic, shift and comparison operator, and the arguments of a
     call among themselves. **Sequenced** (not checked): the operands of `,`, `&&` and `||`, and the condition of `?:`
     against its arms. Both arms of `?:` count for the `?:` as a whole, whichever runs (as the compilers do).
   - In `x = e` and `x op= e`, a **write** of `x` inside `e` is a conflict (the store is unsequenced with the side
     effects of `e`); a **read** of `x` in `e` is not (its value computation comes first). The same holds for a
     declaration's initializer and a statement assignment.
   - **C's rules in both modes.** C++17's extra sequencing is not used, so `x = x++` is rejected in a `.cpp` file too:
     one rule, the strictest reading.
   - Examples rejected: `i++ + i`, `x = x++`, `x += x++`, `f(x++, x)`, `(x = 1) + x`, `(x = 1) + (x = 2)`,
     `(c ? x++ : 0) + x`, `(x++, 0) + x`. Accepted: `x = x + 1`, `(x = 1, x + 1)`, `(x = 1) && x`, `x++ && x`.
   - **The stored expression.** In `x = e` (and a declaration's initializer or a statement assignment), a write of `x`
     inside `e` is a conflict unless a sequence point separates it from the value of `e`: it lies, on the path from the
     root of `e`, inside the left operand of `,`, the first operand of `&&` or `||`, the condition of `?:`, or an
     argument of a call (there is a sequence point after the arguments, before the call). So `x = (x++, 2)` and
     `x = g(x++)` are accepted (defined in C, and no compiler warns, P35 `w4`, `w14`), while `x = x++`, `x = (x = 1)`,
     `x = ++x`, `x = (c ? x++ : 0)`, `x = c && x++`, `x = (1, x++)` and `x = x++ + 1` are rejected (P35 `w1`–`w9`). In
     `x op= e` the left operand is also read, unsequenced with the side effects of `e`, so any write of `x` in `e` is a
     conflict (`x += (x = 1)`, `x += x++`); the pair rule finds it through the expansion of 0012 item 7.
   - **Exact but for `?:`.** The rule rejects exactly the expressions C leaves undefined, except that both arms of `?:`
     count (`(c ? x++ : 0) + x` is rejected even when `c` is false), as every compiler warns there (P35 `u7`). Arms are not
     checked against each other (`c ? x++ : (x = 2)` is accepted, `w23`). P35's union over clang and GCC in both modes is a
     subset of what it rejects.
   - Until M4.5 a modification is of a named local, so every conflict is visible in the source. With pointers
     (`*p = (*q)++`) the objects may alias: M4.5 needs a path-precise check then.
5. **The warning rules** (rejections, both modes, the union of clang's and GCC's `-Wall` warnings, P35):
   "Has an effect" below means: contains an assignment, a compound assignment, `++`, `--` or a call, or is a `(void)`
   cast (an explicit discard, which no compiler warns about).
   - `assignment_as_condition` (`-Wparentheses`, all four configurations): the whole condition of `if`, `while`, `do`
     or `for` is a plain assignment `x = e` without its own parentheses (the front end's `is_parenthesized`, P33).
     Accepted, as every compiler is silent: `if ((x = e))`, `if ((x = e) != 0)`, `!(x = e)`, `(x = e) && b`, a compound
     assignment as the condition (`if (x += a)`), a comma (`if (x = a, x)`), a `?:` condition. (Only clang warns about
     `if (x |= a)`; bitwise assignments are rejected anyway, M4.4.)
   - `comma_without_effect` (`-Wunused-value`, all four): a comma whose left operand has no effect, anywhere
     (`(a, b)`, `(void)(a, b)`); and a comma whose value is discarded (an expression statement, a `for` init-statement or
     increment, the left operand of another comma) whose right operand has no effect (`x++, y;`, `x = 1, y;`,
     `for (x = 0, a; …)`). A comma whose value is used may end in an operand without effect (`(x = a, x + 1)`), and so
     may one under a `(void)` cast (`(void)(x++, y)`).
   - `value_not_used` (`-Wunused-value`): a discarded expression (an expression statement, or a discarded comma
     operand) whose outermost operator is not an assignment, `++`/`--`, a call, a comma or a `(void)` cast, and that has
     a side effect inside: `x++ + 1;`, `-x++;`, `x == (y = 1);`, `x++ < 3;`, `(x = 1) + 0;` (gcc C is silent there,
     clang is not), `(int)x++;` (clang only), `g(x) + 1;`. For `?:` each arm is checked by the same rule (`a ? x++ :
     0;` is accepted, all four are silent); for `&&` and `||` the right operand must be a plain assignment of a constant
     (`a && (x = 1);`, silent in all four, though EDG prints its #187 there) or another `&&`/`||` checked the same way:
     anything else with a side effect is rejected (`a && x++;`, `a && (x = b);`, `a || (x += 1);`: gcc C warns
     "value computed is not used" for them, since C turns the operand into `(x = b) != 0`, which it folds away only
     for a constant; the union keeps that asymmetry in both modes). A discarded expression without side effects stays
     accepted (0012 item 3: `x + 1;`, for which EDG prints its own #174; `x / 0;` is a `division-by-zero` error).
   - `side_effect_not_evaluated` (clang's `-Wunevaluated-expression`): a side effect inside the operand of `sizeof`, in
     both modes, and of `noexcept` in C++. The operands of `typeof` and `decltype` are in no tree the encoder can reach
     (P33), so `decltype(x++)` (which clang warns about in C++) and `typeof(x++)` (silent in both C compilers) are
     accepted: they are never evaluated, so their meaning is right (owner, 2026-10-22; an M8 inventory entry). C's
     `typeof(x++)` is pinned by a C-only test; no test pins `decltype(x++)`, since clang warns about it and a positive
     test must compile warning-free.
6. **VIR** (`internal/encoding.md` §2). Two new expression kinds:
   - `AssignE(var, a, b, yields_old)`: `a` is the value stored, already converted to `var`'s type by the Builder (the
     same code as the statement forms, `Builder::assign`, `compound_assign`, `inc_dec`); `b` is null, or for `x++`/`x--`
     the one `Local x` node that `a` also uses, so the old value is read once (one `uninitialized` obligation, as the
     statement form has). The value is `a`, or `b` when `yields_old`.
   - `Comma(a, b)`: only where its value is used (never `void`); `a` may be a `void` call.
   - The encoder keeps a top-level assignment as an `Assign` statement, and turns a discarded comma into the statements
     of its operands in order (exact: the comma is a sequence point), so code without the new forms builds the same
     VIR as before.
7. **Lowering** (`src/vir/lower.cpp`; `internal/encoding.md` §3). A full expression lowers to obligations and an
   ordered list of **effects**: a call (as before), a write (`v := value; d := true`), a snapshot (`t := v`), a
   branch (`if (c) { … } else { … }` assigning a temporary).
   - **E1.** Calls are the only fixed points: they keep 0012 item 6's order (innermost first along the nested chain,
     source order across a `,`).
   - **E2.** Every other item — an obligation, a write, a snapshot, a branch (which holds no call in M4.3a) — is placed
     right after the last call it depends on, or before the first call when it depends on none: 0012 item 6's placement,
     applied to writes and snapshots too. An item depends on a call through the temporaries it mentions (a call's result,
     and the snapshots, writes and branches whose values it uses), its guard, the floor of E6 (inside a comma's right
     operand), and the earlier accesses of the locals it touches (a snapshot of `x` follows every earlier write of `x`, a
     write of `x` every earlier snapshot or write of it). Items placed after the same call keep evaluation order (left
     to right for unsequenced operands, C's order for sequenced ones). So no item is placed after a call that C may
     evaluate after it: a call that does not return hides only what depends on it. (Audit round 1 found the first form
     of this rule, which kept every effect in one left-to-right order, unsound: in `never_returns(0) + 100 / y++` the
     checks of `100 / y++` followed the call. Moving a write or a snapshot before a call is one of C's orders, since a
     callee cannot read or write the caller's locals and item 4 leaves unsequenced operands no shared local.) A
     fail-closed check confirms after placement that the snapshots and writes of each variable kept their evaluation
     order.
   - **E3.** Every read of a local that the same full expression writes (anywhere, branches included) is a snapshot
     effect at its point in the evaluation, and its `uninitialized` obligation follows the snapshot. A printed value
     therefore never mentions a variable that the expression writes; it mentions only temporaries assigned once before
     their use.
   - **E4.** The value of `AssignE` is the lowered `a` (or the snapshot of `b`), never the variable.
   - **E5.** A right operand of `&&`/`||` or an arm of `?:` that has a side effect becomes a branch: the operand's own
     effects and obligations are emitted inside the `if`, unguarded, and the branch assigns a temporary. An operand
     without side effects keeps the `guard ==> cond` encoding of 0012 item 5.
   - **E6.** Every item of the right operand of `,` depends at least on the left operand's last call (the floor): C
     evaluates nothing of the right operand before the left is complete. The floor ends with the comma: items of the
     comma's siblings, and items that use the comma's value, are not bound by it (they depend on its calls only through
     the values they use). That direction is sound: such an item may be placed before a call that C evaluates first,
     which can only report an error on a path where C would not reach it (audit round 2: `(never_returns(0), 1) &&
     100 / d` reports `division-by-zero`, a false alarm).
     So the calls of a comma come in source order and a check after a call that does not return is never hoisted
     before it (`(abort(), 1 / d)` verifies).
   - **The flag** (0019 item 13). Only a write effect sets a flag, right after the write (0019 item 5's property, now
     also a lint rule). A read is an `uninitialized` obligation at its own point, after every write sequenced before it.
   - **Loops.** A `while` or `for` loop whose condition has a side effect is lowered with the test at the top of a
     `while (true)`: `while (true) inv dec { <the condition's effects and obligations>; if (!c) { goto l_brk__N } body;
     label l_cont__N; step } label l_brk__N`. It is 0014 item 5's `do` shape with the test moved first: the invariants
     and the measure are checked before each evaluation of the condition, where C evaluates it (0012 item 8), the
     condition's obligations are asserts where C evaluates them, a `continue` reaches the step and the back edge, and
     the exit is the jump. A `do` condition already lowers inside the body. A loop whose condition has no side effect
     lowers as before. S17 checked the shape: entry, preservation, termination, an overflow in the condition at its last
     evaluation, `continue`, `break` and a flag written only in the condition each fail or verify as they should; the
     measure need not be bounded at the head from which the loop exits (Silicon checks the bound on the back edge,
     which is 0013 item 2's rule).
   - The locals a loop assigns (its automatic range invariants, 0012 item 8, and its `ds_` snapshots, 0019 item 4)
     include those written inside any expression of the loop: its condition, its step, and every statement's
     expressions, nested loops' conditions included.
   - **Unchanged emitted Viper.** A full expression without `AssignE` or `Comma` has no snapshot, write, branch or floor,
     and lowers exactly as before.
8. **Why it is sound.**
   - **Values.** By E3 and E4 every printed value is the value C computes: a variable read is captured when C reads it,
     and a stored value is the converted value. A snapshot of `x` follows every earlier write of `x` and precedes every
     later one (E2 and its order check), so it captures the value C reads.
   - **Order.** The sequencing rule (item 4) accepts only expressions in which no local is written by one operand and
     touched by another unsequenced operand. A callee cannot read or write the caller's locals (no globals, no
     pointers before M4.5; named constants do not change). So the emitted order is one of C's orders, and every order of
     C gives the same values and the same final state.
   - **Checks.** An obligation holds in every order C may use because its value does not depend on the order (above).
     It is placed before every call it does not depend on (E2), so a call that does not return cannot hide it, except a
     call it depends on, which C must complete first (its result, its guard, or a `,` before it, E6). Asserting it
     earlier than C evaluates it can only report an error on a path where C would not evaluate it (a false alarm),
     never miss one.
   - **Branches.** A branch runs the operand's effects only when C evaluates the operand, and its obligations are
     checked only there.
   - **The flag.** Item 7 keeps 0019 item 5's argument: a flag is set only by a write of its variable, after the write.
   - **Loops.** The top-test shape checks every invariant and the measure where 0012 item 8 and 0013 item 2 say; its
     only exit other than `break` and `return` is the jump taken when the condition is false, after its effects.
   - **No `inhale`, `assume` or axiom is emitted** (invariant 4).
9. **check_program** (`src/vir/check.cpp`; `internal/encoding.md` §2).
   - Side effects are allowed in code: statement expressions, initializers, returns, `if`, `switch`, call arguments,
     and loop conditions (a new context that allows side effects and refuses calls, as 0012 item 6 still asks).
   - They are refused, as internal inconsistencies the encoder never builds, in contracts, `check` and `assert`
     conditions, invariants, measures, case values, named-constant initializers and a `do … while (0)` condition.
   - An `AssignE` node is used once; its `b` is a `Local` of its variable used inside `a`; a loop whose condition has a
     side effect has a jump id; a `for` step may be a block of assignments and expression statements.
   - `sequence_problems` runs again as a backstop.
   - The calls of a full expression must be nested in one another, or ordered by `,` (item 1).
10. **The encoder** (`src/backend/encode.cpp`).
    - It accepts `eok_assign`, the compound kinds and the four increment and decrement kinds as operands, through the
      same checks as at statement level (a local target, the `bool` and enumeration rules), and `eok_comma`.
    - It never encodes an IL node with side effects twice.
    - `int y = (y = 1);` stays rejected (a local initialized from itself, 0012 item 3), now also through an assignment.
    - A side effect in an `assert` condition stays rejected (0015 item 6: `assert` is checked like `check`, and
      `NDEBUG` would remove its side effects).
    - A `switch` on `(c = e)` or `(d, c)` whose value is an enumeration value counts as one for 0019 item 11's rules.
    - **No EDG warning is suppressed** (P34). EDG has no sequencing warning at all. Its #174 ("expression has no
      effect") also covers `x + 1;`, which stays accepted, and its #187 ("use of `=` where `==` may have been intended")
      fires on every assignment of a constant in a truth context, also on `if ((x = 3))`, `(x = 3) && a` and
      `a && (x = 1);`, which the compilers and cerastes accept; so neither qualifies for the owner's rule, and both keep
      printing (owner, after P34). A rejected `if (x = 3)` prints #187 and then `assignment_as_condition`.
11. **Positions and categories.** No new verification error category. Each new rejection is reported at the C line of
    the construct (the modification for item 4, the assignment for `assignment_as_condition`, the comma for
    `comma_without_effect`, the operator for the discarded value, the operand for an unevaluated side effect). An
    obligation inside a nested assignment is reported at its own position, as before.
12. **A jump past a declaration with an initializer** stays refused in both modes (owner, 2026-10-22, handoff §11
    question 22; amends 0019 item 6, which left C open for a later record).
13. **Tests and documentation.**
    - Tests (invariant 2): must-verify and must-fail pairs for every accepted form, in both modes where both accept the
      code; a rejection test for every rejected shape; VIR self-test cases for every lowering rule and check.
    - Retired with the owner's approval: `reject_comma.c`, `reject_assign_value.c`, `reject_chained_assign.c`,
      `reject_incr_in_expr.c` (their code is now positives with twins).
    - Pages: Verifying a program ("Side effects in expressions", the not-supported list), Error messages, Integer
      semantics, Overview.
    - Tooling: `mutate.py` operators `drop-store`, `swap-incdec`, `drop-comma`; `lint_viper.py` checks that every flag
      set follows a write of its variable.
14. **For M4.3b** (record 0021): calls in guarded operands (inside item 7's branches), calls in loop conditions (the
    top-test shape), and unsequenced calls by the dependency-chain rule (handoff §8.1).
15. **Facts relied on** (`internal/il-inventory.md`, "M4.3a"): (P33) the IL of side effects used as values in both
    modes; (P34) EDG's warnings on them in verify mode; (P35) the compilers' warnings; (S17) the top-test loop in
    Silicon; (S18) branches, snapshots and the comma floor in Silicon.
16. **Amends:**
    - 0012 items 3, 4, 6, 8 and 12;
    - 0014 item 5 (the lowering shape, not its rule on calls);
    - 0019 items 4, 6, 8 (a read in the condition of the top-test loop fails as an assertion, `assert.failed`, like a
      `do` condition's) and 13;
    - `internal/encoding.md` §2, §3 and §8.

## Consequences

- Assignments, compound assignments, `++` and `--` may be used as values, and the comma operator is accepted, in both
  modes; loop conditions may have side effects.
- An expression that C leaves undefined by an unsequenced modification is rejected at the modification, in both modes,
  even where C++17 defines it.
- Shapes the compilers' `-Wall` warns about (an assignment as a condition without extra parentheses, a comma operand
  without effect, a discarded value with a side effect inside, a side effect in `sizeof`) are rejected.
- The emitted Viper of every program without these forms is unchanged.

*2026-10-23, amended by 0021 (M4.3b): item 1 — calls are accepted in any arrangement whose checks form a chain, not only nested or ordered by `,`; item 7 — every item's dependency is a set of fixed points (calls, and branches that contain a call), and the calls are ordered by the chain of those sets (E1 and E2 generalized; a branch may contain calls, and is then a fixed point); item 9 — `check_program` no longer refuses calls in loop conditions or guarded operands, nor two calls that are not nested (the chain is decided in lowering); item 14 — done in 0021; item 5 — its rule for a discarded `&&`/`||` (the right operand must be an assignment of a constant, or another `&&`/`||` checked the same way) applies whenever the expression has a side effect anywhere, also only in its left operand (`x++ && a;` is rejected, as GCC's C mode warns, P37): the M4.3a detector applied it only when the right operand had an effect.*

*2026-10-23, amended by 0022 (M4.4a): item 2 — the value of `x = e` is `e` converted with a check for unsigned types too (never a wrap).*
