# 0021 — Calls in expressions: guarded operands, loop conditions and the dependency-chain rule (M4.3b)

- **Status:** accepted for M4.3b. Owner decisions:
  - **The dependency-chain rule** (2026-10-22, M4.3 planning; handoff §8.1): two calls that are not nested and not
    ordered by `,` are accepted when the checks of their full expression form a chain (item 3), and rejected otherwise.
  - **Termination (B)** (2026-10-23, M4.3b planning): where another call may precede a recursive call, cerastes asserts
    the call's measure decrease itself, where C may make the call (item 4). **Fallback (A)**, pre-approved: a case the
    spike cannot reproduce exactly is a call whose check cannot move, and a conflict is rejected.
  - **Retired** (approved, 2026-10-23): the tests `reject_call_guarded.c`, `reject_call_cond.c`,
    `reject_call_loop_condition.c`, `reject_do_call_condition.c`, `reject_unsequenced_calls.c`,
    `reject_effect_call_guarded.c`, `reject_effect_call_loop_condition.c`; the VIR rejection cases
    `call_in_loop_condition`, `call_in_guarded_operand`, `call_in_conditional_branch`, `two_unnested_calls`,
    `sibling_calls_in_arguments`, `cf_call_in_do_condition`, `fx_rej_call_in_effectful_loop_condition`,
    `fx_rej_effect_with_call_in_guarded_operand`, `fx_rej_calls_around_a_comma_in_a_sum`,
    `fx_rej_two_unnested_calls_still`. Each shape returns as a positive with twins or as a lowering case.
  - **Handoff §11 question 24** (2026-10-23): clang's `-Wself-assign` shapes are rejected (`self_assignment`, item 8),
    with a test per form. Question 25: the union rule of 0020 item 5 stays. Question 26: sequencing with pointers is
    part of M4.5a's memory-model record.

  Facts marked (P*n*) and (S*n*) are probes and Silicon spikes, recorded with their evidence in
  `internal/il-inventory.md`, "M4.3b" (P36–P37, S19–S20). Audit round 1 (Opus) found one blocking item (the termination
  member of F in item 3, fixed) and should-fix items folded into items 1, 3, 4, 8, 9, 11 and 13.
- **Milestone:** M4.3b (2026-10-23).

## Context

0012 item 6 hoists every call of a full expression into a temporary before the statement, asserts the callee's
`requires` at the call, and places every other check after the calls it depends on and before every other call. That
placement is sound only if the calls are evaluated in the order they are emitted, so 0012 item 6 rejected:
- two calls that are not nested (`f(x) + g(y)`, `h(f(x), g(y))`): C may run them in either order, and a callee that
  does not return (`ensures false`, a `[[noreturn]]` function, or a function that calls `exit` on some inputs) would
  hide every check emitted after it, although C may evaluate those checks first;
- a call in the right operand of `&&`/`||` or an arm of `?:`: a hoisted call would run when C does not run it;
- a call in a loop condition: a hoisted call would run once, where C runs it at every evaluation.

0020 lifted the first rule for calls ordered by `,` and gave guarded operands with side effects and loop conditions
with side effects a lowering (0020 item 7: branches, the top-test loop). It kept the three rejections for calls.

C's rules for calls (C17 6.5.2.2p10, unchanged in C23; C++ [intro.execution]/11): there is a sequence point after the
evaluation of the function designator and the arguments, before the call; every evaluation in the caller that is not
otherwise sequenced before or after the callee's body is *indeterminately sequenced* with it: the bodies of two calls
never interleave, but their order is unspecified, and a program is correct only if it is correct for every order.

## Decision

1. **Accepted, in both modes.**
   - A call in the right operand of `&&` or `||`, or in an arm of `?:`: it runs only when C evaluates that operand.
   - A call in the condition of a `while`, `for` or `do` loop: it runs at every evaluation of the condition.
     (`do … while (g())` is a measured `DoWhile`: only a constant zero makes a `DoOnce`, 0014 item 5. A `DoOnce`
     condition still refuses calls; C evaluates it once, and it is folded to zero.)
   - Any number of calls in one full expression, in any arrangement (`f(x) + g(y)`, `h(f(x), g(y))`,
     `max(f(a), f(b))`, `fib(n - 1) + fib(n - 2)`, `(f(), 0) + g()`), when their checks form a chain (item 3).
   - The same in the condition of `assert`, which 0015 item 6 checks like `check` and which may contain calls (but no
     side effect): `assert(a <= 0 || pos(a) > 0)`, `assert(f(x) + g(y) > 0)` (amends 0015 item 6).
   - **Discarded guarded calls** follow 0020 item 5's union rule, with calls counted as side effects (P37, all four
     configurations): `c ? g(x) : h(y);` and `c ? g(x) : 0;` are accepted (every compiler is silent), while
     `a && g(x);`, `a || g(x);`, `g(x) && a;` and `a && (g(x), 1);` are rejected (`value_not_used`: GCC's C mode warns,
     and for the last Clang). P37 also showed that GCC warns on a discarded `&&`/`||` whose side effect is only in its
     *left* operand when its right operand is not an assignment of a constant (`x++ && a;`, `(x = 2) && a;`), which the
     M4.3a detector accepted: the detector now applies 0020 item 5's right-operand rule whenever the expression has a
     side effect anywhere (a correction of the implementation, not of the rule), and reports at the operator whose
     right operand fails. A `?:` whose arms are `void` calls (`c ? v(x) : w(y);`) stays rejected (`unsupported_type`:
     VIR has no `void` value; an M8 inventory entry).
2. **Meaning.** A call in a guarded operand runs, with its precondition checked, exactly when C evaluates the operand. A
   call in a loop condition runs, with its precondition checked, at each evaluation. Calls that C leaves unsequenced
   are verified for every order C may choose: no order may reach undefined behaviour, a failed precondition, a
   non-terminating recursion or a broken `ensures`. A callee cannot read or write the caller's locals (no globals or
   pointers before M4.5) and 0020 item 4 leaves unsequenced operands no shared local, so every order gives the caller's
   locals the same values, and every order satisfies the same contracts (a trusted callee defined in another unit may
   return different values in different orders, but contracts mention no state, so each of its results satisfies its
   `ensures` in every order); what depends on the order is *whether a check is reached*, through callees that do not
   return.
3. **The dependency-chain rule** (`src/vir/lower.cpp`; `internal/encoding.md` §3).
   - **Fixed points** are the calls of a full expression and the branches (0020 E5) that contain a call. They are
     numbered in creation order (arguments before their call, left before right).
   - **Sets.** Every item of a full expression — an obligation, a write, a snapshot, a call-free branch — has the set of
     fixed points C must have completed before it evaluates the item: the fixed points whose values it uses (a call's
     result, and the snapshots, writes and branches whose values depend on one), its guard's, the comma floor's (0020
     E6), and, for an item that touches a local, the sets of the earlier accesses to it that it must follow (0020 E2): a
     read (a snapshot, or a branch's read) follows the earlier *writes* of the local, a write (or a branch that writes)
     follows *every* earlier access. A fixed point P has PRE(P): for a call, everything its arguments use or contain,
     and the floor (C completes the arguments before the call); for a branch with a call, everything its condition
     operand uses or contains, the floor, and the sets of the earlier accesses its reads and writes must follow. A value that uses P has the set D(P) = PRE(P) ∪ {P}. Every set is
     closed downward: it contains the PRE of each of its members. A set may be smaller than this (item 10, "Smaller sets
     are safe"); it is never larger.
   - **The family F** of a full expression: the sets of all its obligations, writes, snapshots and call-free branches;
     D(B) for every branch B with a call (its checks are inside it and cannot be placed apart from it); and PRE(c) for
     **every** call c whose callee is in the caller's recursion cycle (its termination check, item 4), added before the
     order is computed, whether or not the explicit check is printed (audit round 1: adding it only for printed checks
     made F depend on the order it determines).
   - **Accepted** when F is a chain under ⊆. Then the fixed points are ordered (π): those of the smallest set, then
     those each larger set adds, then those in no set, each group in creation order. Every set of F is then a prefix of
     π, and each item is emitted right after the fixed points of its set and before every other: a callee that does not
     return hides only items that C cannot evaluate before it returns. Each fixed point follows its PRE (the sets are
     closed downward and creation order follows PRE), so π is one of C's orders. Soundness does not rest on that: each
     item is asserted right after exactly its set, which every C and C++ execution completes before evaluating it (in
     C++, π may differ from C++17's extra order for `<<`, `>>` and argument initialization, which cannot change a value
     here, item 2).
   - **Rejected** otherwise (`unsequenced_calls`, at the later-created of two fixed points that witness the conflict,
     naming both): two items each depend on a fixed point the other does not, so whichever is emitted first can hide
     the other's check while C may evaluate it first. Examples rejected: `100 / f(x) + 100 / g(y)`,
     `(f(x), 100 / g(y)) + 100 / h(z)`, `(a && g(x)) + (b && h(y))`, `a && (100 / f(x) + 100 / g(y) > 0)` (inside the
     branch), `(f(x) > 0 && 100 / y > 0) + 100 / g(z)` (the guard counts), and `g(f(x), h(y))` when the parameters of `g`
     are `short` (each argument's conversion is a check on its own call) or when `g` has one `requires` clause about each
     argument (`requires(1 <= a)`, `requires(2 <= b)`: each clause is a check on one call), while it is accepted with
     `int` parameters and one clause about both (`requires(1 <= a && 2 <= b)`).
     Accepted: `f(x) + g(y)`, `max(f(a), f(b))`, `f(g(x)) + 100 / h(y)` (emitted h, its check, g, f — when `f` has no
     `requires`, the argument `g(x)` needs no conversion check and `f` is not in the caller's recursion cycle; otherwise
     PRE(f) = {g} is a set of F that conflicts with {h}, and it is rejected), `(f(), 0) + g()`,
     `nr(k) + 100 / zero(y)` (emitted `zero`, its check, then `nr`: the check is not hidden).
   - **Writes and snapshots are in F**, although neither can fail: one rule, and the emitted order of every access to a
     local follows from its set. So `(x = f(a)) + (y = g(b))` is rejected; a later record may relax it.
   - **Branches with a call** are lowered as in 0020 E5, with their own full expression inside the `if`; the same rule
     applies inside (an inner conflict is rejected; "a fixed point outside PRE(c)" of item 4 is measured inside the
     branch), and the branch comes right after PRE(B) in the outer order.
   - **Fail closed.** After the order is fixed, lowering checks that the fixed points emitted before each item are
     exactly its set, and that each branch with a call, and each recursive call without a printed explicit check, comes
     right after its PRE; otherwise an internal error (exit 3). A self-test hook perturbs the order and shows that the
     check fires, as another does for 0020 item 7's check of each local's accesses.
   - **Where it is decided.** Only lowering knows exactly which operations create checks (a conversion from a range
     inside the target creates none, an unsigned operation wraps, a `requires` clause may not mention an argument), so
     the rule is decided there, exactly, and its refusals are reported as rejections through `LowerResult::rejections`
     (`src/vir/lower.h`), which the encoder reports before anything else from lowering. `check_program` no longer has
     a rule about the number or placement of calls (item 6).
4. **Termination of a call that another call may precede.** Silicon checks a call's termination (0013 items 2 and 8)
   only at the call statement, so an earlier callee that does not return would hide it, although C may make the call
   first. Every call c whose callee is in the caller's recursion cycle contributes PRE(c) to F (item 3); when c is then
   emitted after a fixed point outside PRE(c), cerastes asserts the decrease itself, right after the fixed points of
   PRE(c), in category `termination`, with 0013 item 8's message and note:
   - **The condition** is Silicon's own (silver's `DecreasesCheck.scala:111-165`, which extends both tuples with a top
     element) between the callee's measure N over the call's arguments and the caller's measure M over its parameters'
     entry values: ⋁ᵢ<min(|N|,|M|) (⋀ₗ<ᵢ Nₗ = Mₗ ∧ Nᵢ < Mᵢ ∧ Mᵢ ≥ 0), and, when |N| > |M|, also the disjunct
     ⋀ₗ<|M| Nₗ = Mₗ (a longer callee measure that agrees on the caller's whole measure decreases; an equal or shorter one
     that agrees does not). For equal lengths it is 0013 item 2's relation. It is emitted as one atomic `assert`,
     `!(!alt₁ && !alt₂ && …)` over the disjuncts, so that a failure is one report (with a plain `||` Silicon reported one
     S19 point twice); S19 was rerun on this form (S19:
     2950 points over one to three components, mutual recursion of equal and unequal lengths, a `c_div` measure and a
     parameter changed before the call; the explicit assert fails exactly where Silicon's check fails, 0 mismatches).
   - **The cycle.** Silicon checks a call when the callee is in the caller's strongly connected component of the method
     call graph, which has an edge for every call in any body, dead code included (silver's `MethodCheck.scala:66`); a
     call outside it gets no check (S19 case 8). Lowering computes the same components from every call node of the
     VIR bodies (the `__wf` methods and the methods without a body call nothing). An over-approximation would only add
     false alarms; an under-approximation would be unsound, so the set is exact, and lowering checks it: it recomputes
     the components from the call statements of the lowered program and fails closed (internal error) if they differ.
     A must-fail test has a cycle whose only back edge is dead code.
   - **Values.** The explicit check reads the argument values the call reads: an argument that depends on a call has that
     call in PRE(c), a local the expression writes is read through a snapshot (0020 E3), and the caller's measure is
     over its parameters, which never change.
   - **Reports.** Silicon's own check at the call stays, as a backstop. When the explicit check fails, Silicon reports it
     once (`assert.failed`, `assertion.false`), mapped to `termination` at the call with 0013 item 8's note; its own
     check at the call then passes, since a failed assert's condition is assumed afterwards: one report per failure
     (S19 case 10).
   - A call emitted right after its PRE (every call accepted before M4.3b) gets no explicit check, so its Viper is
     unchanged.
   - **Fallback (A)** (pre-approved, not used: S19 reproduced every case, unequal lengths included): a call whose check
     cannot be reproduced exactly would be one whose check cannot move (D(c) in F), and a conflict rejected.
5. **Loop conditions.** A `while` or `for` condition with a call is lowered by 0020 item 7's top-test shape (the call is
   a side effect): its precondition is asserted, and the call made, at each evaluation, after the invariants and the
   measure are checked. A `do` condition is already lowered at the end of the body. The automatic invariants are
   unchanged (a call writes no local).
6. **check_program** (`src/vir/check.cpp`): the rule "the calls of a full expression are nested or ordered by `,`",
   the rule against a call in a conditionally evaluated operand, and the refusal of calls in a loop condition
   (`Ctx::LoopCond`) are dropped; a loop whose condition has a call has a jump id, like one with a side effect. Calls
   stay refused in contracts, `check`, invariants, measures, case values, named-constant initializers and a `DoOnce`
   condition (`Ctx::Annot`, its text unchanged).
7. **Why the emitted Viper of programs accepted before M4.3b is unchanged.** Before M4.3b a call was accepted only when
   every call created before it was in its arguments or in the left operand of an enclosing `,`, that is, in its PRE;
   no call was lowered inside a branch. So every set is an interval of creation order (the one 0012 item 6 and 0020
   item 7 computed as "the highest call"), F is a chain of intervals, π is creation order, every item keeps its slot,
   and no explicit termination check is emitted.
8. **Self-assignment** (`self_assignment`, handoff §11 question 24; P36). An assignment `x = x` is rejected, at the
   `=`, exactly where clang's `-Wself-assign` warns (GCC 16 and EDG never warn): both operands, after the front end's
   compiler-generated conversions (parentheses are a flag in the IL, not a node), are the same variable, and neither
   the `=` nor either operand comes from a macro expansion (the IL's `orig_seq`/`orig_column` equal `seq`/`column` on
   all three nodes). It applies wherever the assignment is: a statement, a value (the inner assignment of
   `y = (x = x)`), a `for` step, a comma operand, a condition (`if ((x = x))`). Accepted, as clang is silent there: an
   explicit cast (`x = (int)x`), any other operator (`x = +x`, `x = x + 0`), compound assignments, the outer assignment
   of `x = y = x`, and any macro involvement (`SET(x, x)`, `X = x`). `sizeof(x = x)` stays `side_effect_not_evaluated`,
   and `x = (x = x)` is also an unsequenced modification (0020 item 4; `self_assignment` is reported, being found
   first). The rule agrees with clang on all 132 function and mode pairs of P36 (compared per function), and on the 131
   probe functions run through `cerastes verify` (compared by line).
9. **Positions and categories.** No new verification error category. Two new rejections: `unsequenced_calls` (at the
   later witness, item 3, or at the call whose default argument holds it, as 0012 item 6 reports default arguments;
   the conflicting items may be checks or assignments) and `self_assignment` (at the assignment). A termination failure is reported at the call
   with a note at the callee's measure, whether the explicit check or Silicon's finds it (0013 item 8).
10. **Why it is sound.**
    - **Order.** π is one of C's orders, and every order gives the same values (item 2). Each item is emitted right
      after the fixed points of its set, all of which C completes before evaluating it; so an item hidden by a callee
      that does not return is one C cannot evaluate before that callee returns, in any order.
    - **Calls' own checks.** A call's precondition is an obligation whose set is that of the argument values it reads
      and the floor (0012 item 6), a subset of PRE(c); its termination is checked where C may make the call (item 4).
      A branch with a call comes right after PRE(B).
    - **Smaller sets are safe.** Some sets are smaller than "everything C completes first": a precondition's (above),
      and the value of a comma, which carries the values it uses but not its left operand's floor (0020 E6). An item
      with a smaller set is asserted earlier than C may evaluate it, which can only report an error on a path where C
      would not reach it (a false alarm), never hide one: `h((validate(y), y))`, where `validate` ends the program unless
      `y > 0` and `h` requires a positive argument, reports `precondition` (audit round 2). Making these sets exact would
      change the Viper of programs accepted in M4.3a, so it is left for a later record (handoff §6).
    - **Rejection.** When F is not a chain, no order of the calls places every item right after its set, and the
      expression is rejected.
    - **No `inhale`, `assume` or axiom is emitted** (invariant 4).
11. **Tests and documentation.** Must-verify and must-fail pairs for every accepted form, in both modes; the soundness
    pins `nr(k) + 100 / zero(y)` (must fail `division-by-zero`) and `g(n - 1) + g(n + 1)` in a recursive `g` that does
    not return (must fail `termination` at `g(n + 1)`; the C test writes `g(n - 1) + g(n)`, since `g(n + 1)` would also
    break `g`'s own range precondition, and the VIR self-test keeps `g(n + 1)`), each of which verifies if the calls are
    reordered without sets, or without item 4; for item 4 also unequal lengths behind a call that does not return (a longer callee measure that
    agrees verifies, a shorter one fails), the argument values at the check (`nr(0) + g((m = n + 1, m))` fails), the
    check inside a branch and in a loop condition, mutual recursion, and a cycle whose only back edge is dead code;
    `assert` conditions with calls; the discarded guarded calls of item 1; verified positives for the shapes
    `self_assignment` leaves alone (`SET(x, x)`, `X = x`, `x = (int)x`, `x = y = x`, `x = +x`) and rejections for
    `f(x = x)`, `return x = x;` and `c ? (x = x) : 0`; a rejection test for every rejected shape; VIR self-test cases
    for every rule, including the two fail-closed checks. Pages: Verifying a
    program ("Calls in expressions", the not-supported list), Error messages (termination at an unsequenced recursive
    call), Overview. Tooling: `mutate.py` operators `swap-logical` (the operands of `&&`/`||` around a call) and
    `swap-arms` (the arms of `?:`).
12. **Facts relied on** (`internal/il-inventory.md`, "M4.3b"): (P36) the compilers' and EDG's warnings on
    self-assignment and its IL; (P37) the compilers' warnings on discarded guarded calls and on a discarded `&&`/`||`
    with a side effect in its left operand; (S19) the explicit termination check against Silicon's; (S20) the order,
    branches with calls and calls in loop conditions in Silicon.
13. **Amends:**
    - 0012 item 6 (calls) and item 12's list;
    - 0013 item 2 (the relation Silicon uses for measures of unequal lengths, now emitted by cerastes) and item 8 (a
      termination failure may be found by the explicit check);
    - 0015 item 6 (an `assert` condition may hold guarded and unsequenced calls);
    - 0020 item 5's implementation (the right-operand rule for a discarded `&&`/`||` applies whenever the expression has
      a side effect, item 1);
    - 0014 item 5 (a call in a loop condition);
    - 0020 items 1, 7, 9 and 14;
    - `internal/encoding.md` §2, §3 and §8.

## Consequences

- Calls may appear anywhere in code: in guarded operands, loop conditions and unsequenced operands, in both modes.
- An expression whose checks depend on different unsequenced calls is rejected (`unsequenced_calls`); rewriting it with
  a local per call (`int a = f(x); int b = g(y); … 100 / a + 100 / b`) fixes the order and is accepted.
- The emitted Viper of every program accepted before M4.3b is unchanged.
