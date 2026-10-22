# 0012 — Encoding scalar C and C++ into Viper (M3)

- **Status:** accepted for M3. Facts marked (P*n*) are confirmed by the M3 IL probes before code relies on them.
- **Milestone:** M3 (2026-10-16)

## Context

M3 verifies non-template functions over integers and `bool` (plan.md): locals, arithmetic, comparisons and
logical operators, `if`/`else`, `while`/`for` with `invariant`, `return`, calls verified modularly, `check`. The
integer meaning of each operation is decision 0010; the driver is 0011. This record fixes how statements,
expressions, contracts and calls are encoded, what is rejected, and why each choice is sound. The VIR that
carries the encoding is specified in `internal/encoding.md`.

Spikes (2026-10-16, Silicon `v.26.08-release`): a `goto` from inside a `while` body to a label after the loop
verifies; Viper's `\` and `%` on `Int` are Euclidean (`(-7)\2 == -4`, `(-7)%2 == 1`, `7\(-2) == -3`).

## Decision

1. **One Viper method per function** with a body in a verified file (0011 item 4). It has a formal argument per
   parameter and a result `res` for non-`void` functions. Its `requires` are the parameters' type ranges, then
   the user's `requires` in order; its `ensures` are the result's type range, then the user's `ensures`. A
   function used only through its contract (declared here, defined elsewhere, or in a system file) becomes a
   bodyless method with the same specification, marked `// cerastes:trusted-contract <reason>` (the single-unit
   assumption of 0011 item 6; `scripts/lint_viper.py` rejects a bodyless method without the tag).
2. **Parameters denote entry values in contracts** (0006 item 5). Viper's formal arguments cannot be assigned,
   so each by-value parameter is copied to a local at entry and the body uses the local. Contracts read the
   formals, which hold the entry values; `check` and `invariant` read the locals, which hold the current values
   (P6, confirmed: contracts use `enk_param_ref`, bodies and statement annotations use `enk_variable` of the
   parameter, linked to its number by `variable.assoc_param_type`). `cerastes::old(e)` (C++) in an `ensures` is
   encoded as `e` over the formals: in M3 the parameters are the only state, so the value on entry is the
   formals' value. `old` applied to the result name is rejected (`unsupported_expression`): the result has no
   value on entry.
3. **Statements** (an allow-list; anything else is rejected with `unsupported_statement`):
   - blocks, the null statement with no attribute other than `cerastes::` ones (0011 item 7), `check`;
   - a local declaration of an allowed scalar type **with** an initializer (`const` allowed) that does not name
     the variable being declared (`int x = x;` reads an indeterminate value: rejected);
   - an expression statement whose expression is one assignment, compound assignment, `++` or `--` (pre or
     post) of a local, a call, or an expression without side effects (including `(void)e`). A discarded
     expression is still evaluated: its obligations are asserted (`x / 0;` is an error);
   - `if` / `if … else` without an init-statement or a condition declaration;
   - `while` with a side-effect-free condition; `for (init; cond; incr)`, where `init` is an allowed
     declaration or expression statement and `incr` an allowed expression statement (either may be empty),
     encoded as `init; while (cond) { body; incr }`, so a `for` loop's invariants are first checked after
     `init`;
   - `return` (with an expression in a non-`void` function; a written `return;` in a non-`void` function, which
     EDG accepts in clang emulation with only warning #117, is rejected: it violates a constraint of C and is
     ill-formed C++);
   - `static_assert` (checked by EDG at compile time; no run-time effect, so it is not encoded).
4. **Side effects only at the top** of a full expression: an assignment, compound assignment, `++` or `--` is
   allowed only as an expression statement, as a `for` increment, or as a declaration's initializer. Anywhere
   else (an assignment used as a value, a chained assignment, `x++` inside an expression, the comma operator) is
   rejected (`unsupported_expression`). With no heap and no globals in M3, a callee cannot change the caller's
   locals, so the remaining expressions are free of unsequenced modifications and their order of evaluation
   cannot change their value: the encoding evaluates operands left to right.
5. **Expressions lower to a pure value plus obligations.** Each obligation is a condition with a category, a
   C++ position and a guard. The guard is the condition under which the C/C++ program evaluates the operand:
   in `a && b` the obligations of `b` are guarded by `a`; in `a || b` by `!a`; in `c ? x : y` those of `x` by
   `c` and those of `y` by `!c`. Each obligation is emitted as `assert guard ==> cond` on its own line before
   the statement that uses the value, so checks inside an operand that C/C++ does not evaluate never fire.
6. **Calls** are hoisted into temporaries before the statement that uses them.
   - **Unsequenced calls are rejected** (M3-D audit, blocking). Two calls in one full expression, neither of
     which is inside the other's arguments (`f(x) + g(y)`, `h(f(x), g(y))`), may run in either order, and a
     callee that never returns (`ensures(false)` verifies under partial correctness) would hide every check
     placed after it, although C/C++ may run those first. So the calls of one full expression must be nested
     in one another (`f(g(h(x)))`), and anything else is rejected (`unsupported_expression`).
   - **Obligation placement.** Each obligation is asserted after the calls whose results it depends on and
     before every other call of the full expression: an obligation that depends on no call comes before the
     first call, so a callee that does not return cannot hide it. An obligation's guard counts as something it
     depends on: in `f(x) > 0 && 100 / y > 0`, the `y != 0` obligation is guarded by `f(x) > 0`, so it comes after
     the call of `f`.
   - **Calls of `predicate` and `pure` functions from code are rejected** by the encoder too
     (`spec_call_in_code`), whatever M2's checks let through: their bodies are not encoded (0011 item 4), so
     such a call would rest on a contract nothing verifies.
   - each argument is evaluated with its obligations, converted to the parameter type (a checked conversion,
     0010 item 5), and stored in a temporary;
   - each `requires` clause of the callee, with the parameters replaced by the temporaries, is asserted at the
     call (`precondition`, with a note at the clause), so a failure names its clause; Silicon's own check of the
     method precondition is then implied (it is mapped as a backstop);
   - `t := f(…)` is a Viper method call; the callee's `ensures` hold afterwards (Viper's call rule).
   - A call inside a guarded operand (`&&`, `||`, `?:`) or inside a loop condition is rejected
     (`unsupported_expression`): a hoisted call would run when C/C++ does not run it.
   - A call inside an annotation is rejected unless it is `cerastes::old` (0011 item 7).
   - **Default arguments.** An argument the call omits is the callee's default-argument expression, which the
     IL copies into the call (`generated_default_arg`, P7). It is encoded at the call like a written argument,
     and its obligations are reported at the call, with a note at the default argument.
   - Recursion is allowed, with a termination measure (decision 0013): results are total correctness. A
     trusted callee is assumed to terminate (part of its trusted contract); one that throws is outside its
     `ensures`, which are claims about normal returns; the docs say so.
7. **Assignments** evaluate the right-hand side, convert it to the left operand's type (checked), and assign.
   A compound assignment `x op= e` is `x = (T)((C)x op (C)e)`, where the encoder computes the computation type
   `C` itself, because the IL does not give it (0010 item 6; P2); shift-assignment converts the operands
   separately; `++x`/`x++` as statements are `x += 1` in the same way. `++`/`--` on `bool` are rejected.
8. **Loops.** A loop's condition is checked each time C/C++ evaluates it, and so are the obligations of its
   invariants (0010 item 11). Both are emitted as further Viper `invariant` clauses, so Silicon checks them at
   exactly the points C/C++ evaluates the condition: on entry, and after each iteration (after the `for`
   increment), before the next evaluation. The clauses of a loop, in order:
   - an automatic invariant `T_MIN <= x && x <= T_MAX` for each local `x` the loop assigns;
   - the user's `invariant` clauses, in source order, each directly preceded by its own obligations (so an
     invariant's obligations are guarded by the invariants before it, and the invariant itself is evaluated only
     where it is well-defined);
   - the obligations of the condition.

   Each clause is checked in the state where C/C++ evaluates it, and may rely on the clauses before it, which are
   checked first (an obligation asserted at the end of the body could not use the invariants that hold after
   the iteration). Inside the body, all clauses are known, as for any invariant. Nothing is inhaled; a failure of
   an automatic invariant is an internal error (exit 3), since every assignment is already checked. The map
   entries of an obligation clause carry the obligation's own category with both invariant ids (M3-4, as
   built; il-analyst).
9. **Returns.** `return e;` assigns `res` (converted to the return type, checked), asserts each user `ensures`
   clause at the `return` (`postcondition`, with a note at the clause), then jumps to a label at the end of the
   method (`goto end`, which Silicon accepts from inside loops). A `void` function asserts its `ensures` at each
   `return;` and at its closing brace. A non-`void` function whose end is reachable gets `assert false` at its
   closing brace (`missing-return`); `main` instead returns 0, as the standards say, and asserts its `ensures`
   there like any `return`. In the IL, reaching the closing brace is a compiler-generated `return` without an
   expression (for `main`, with the constant 0) (P11). The method's own `ensures` remain, and always hold after
   these asserts.
10. **Contract well-formedness** (0010 item 11). For each function with a contract that the unit uses or
    defines (trusted contracts included, 0011 item 6), a method `f__wf` with the
    same formals, a `res` of the result type, and only the type-range preconditions asserts the obligations of
    each `requires` clause guarded by the earlier `requires`, and of each `ensures` clause guarded by all
    `requires` and the earlier `ensures`. The contract clauses themselves are emitted with total prelude
    functions, so Viper's own well-definedness checks never fail.
11. **The prelude** contains only functions with bodies and no postconditions: no domains, no axioms, no
    bodyless functions. `c_div` and `c_rem` (C's truncating division and remainder for signed operands, four
    explicit sign cases over Viper's Euclidean operators, total at a zero divisor), `c_udiv` and `c_urem` (the
    unsigned forms; a single form built on an absolute value was not provable by Z3 for unsigned operands, M3-4
    spikes), `c_pow2` (a literal table up to the widest type, and at least 1 for every argument), `c_wrap(x, m)`
    (`x % m` for `m = 2^N`), and `c_shr` (floor division by `c_pow2(n)`). Every obligation is emitted as one
    atomic condition, since Silicon reports each conjunct of a failing `&&` as its own error.
12. **Rejected in M3**, each with a diagnostic at the C/C++ line and a test (invariant 3), and each an M8
    inventory entry:
    - statements: `break`, `continue` (in the IL, compiler-generated `goto`s: every `goto` is rejected),
      `switch`, `do … while`, `goto` and labels, `if`/`switch` init-statements and condition declarations,
      `try`/`throw`, `asm`, coroutines;
    - declarations: locals without an initializer or initialized from themselves, `static`/`thread_local`/
      `extern` locals, **globals, including `const` and `constexpr` ones** (their uses are folded constants whose
      `expr` names the global, and the global's initializer, which can narrow or shift, is not encoded; M3-D
      audit), arrays and VLAs, classes, unions, enumerations and enumerators, lambdas, function templates and
      class templates in a verified file (M5), `constexpr`/`consteval` functions in verified code (a call EDG
      evaluates at compile time would skip its checks), variadic functions, function pointers, `[[noreturn]]`/
      `_Noreturn` functions, `requires` on `main` (0011 item 7);
    - types: pointers, references, `volatile`, floating point, enumerations, `_BitInt`, vector types, any type
      kind not on the allow-list (0010 item 2);
    - expressions: bitwise operators, the comma operator, assignments not at the top, unsequenced calls (item 6),
      calls in guarded operands or loop conditions, `old` of the result, folded constants whose `expr` is a
      global, an enumerator or a call (0010 item 12), any operator kind not on the allow-list (checking operator
      kinds alone is not enough: vector arithmetic uses the ordinary kinds, so the operand types are checked too);
    - attributes other than `cerastes::` ones and the allow-list of 0011 item 7.
13. **Positions.** Every VIR node keeps the C/C++ position of the IL node it comes from, with the rule of 0006
    item 10 (the original position inside a `CERASTES_*` macro argument). Every emitted check is on its own line
    of the `.vpr` and has an entry in the position map.

## Consequences

- No `inhale` or `assume` is emitted in M3; the only trusted facts are the contracts of functions defined in
  other units, marked as such.
- The rejections keep M3 small and sound; each one names the construct, so users know what to rewrite.
- `break`/`continue` and `do` are the cheapest to add next (forward `goto`s, as `return` already uses).

*2026-10-20: milestone numbers follow the renumbering of 2026-10-20 (plan.md: old M4 → M5, M5 → M6, M6 → M4.5 for C and M7 for C++, M7 → M8).*

*2026-10-20, amended by 0014 (M4.1): items 3 and 12 — `break`, `continue`, `do … while` and `switch` are accepted (a `for` loop's increment is its `step`); item 8 — on a `continue` path the invariants, the condition's obligations and the measure are checked as at the end of the body, on a `break` path nothing; a `do` loop's invariant holds at the start of every iteration. User `goto` and labels stay rejected (M4.2).*

*2026-10-21, amended by 0015 (M4.2a): item 3 — a user `goto` and labels are accepted when the jump is outward and forward (0015 item 2), and `assert(e)` from `<assert.h>` as an expression statement; item 6 — calls of `abort`/`exit`/`_Exit` and of functions declared not to return are ordinary calls whose `ensures false` ends the path, and the call of `__assert_rtn` in `assert`'s expansion is never encoded (0015 item 6); item 9 — a function declared not to return asserts `false` (category `noreturn`) at each `return` and at its closing brace instead of its postconditions; item 12 — user `goto`/labels (outward and forward) and `[[noreturn]]`/`_Noreturn` functions are no longer rejected.*

*2026-10-21, amended by 0017 (M4.2b1): item 4 — constants cannot change, so with no mutable globals a callee still cannot change the caller's state; item 12 — `const`/`constexpr` globals that are constant-initialized and defined in a verified file are accepted, and so are case values naming them or a folded `const`/`constexpr` local in C++ (in C a case value naming a variable is rejected); still rejected: mutable globals, `extern const` without a definition here, dynamic initialization, a constant read in its own initializer, `thread_local` and `volatile` constants, globals from system files, and folded constants whose `expr` is an enumerator or a call.*
