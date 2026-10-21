# 0014 — Control flow: `break`, `continue`, `do`, `switch` (M4.1)

- **Status:** accepted for M4.1. Owner decisions (2026-10-20): `do … while (0)` needs no termination measure;
  `[[fallthrough]]` joins the attribute allow-list; the M3 rejection tests of these constructs are retired. Facts
  marked (P*n*) are IL probes and (S*n*) Silicon spikes, recorded with evidence in `internal/il-inventory.md`,
  "M4.1: control flow"; code relies on them only once confirmed.
- **Milestone:** M4.1 (2026-10-20), the first session of M4, "the rest of C" (plan.md).

## Context

M3 rejected `break`, `continue`, `do … while`, `switch`, `goto` and labels (0012 item 12) and noted that `break`,
`continue` and `do` are the cheapest to add next. In the IL, `break` and `continue` are compiler-generated `goto`s to
compiler-generated labels (0012, encoding.md §7). VIR has no jumps; the lowered form and the emitter already have
`Goto` and `Label` (a `return` is `goto l_end`), and Silicon accepts a `goto` from inside a loop to a label after it
(0012, spikes). Two rules of M3 constrain the design: invariants and the condition's obligations are checked
wherever C evaluates the loop condition (0012 item 8), and every loop carries a termination measure that must
decrease from each iteration to the next (0013).

## Decision

1. **Accepted:** `break` (in `while`, `for`, `do` and `switch`), `continue` (in `while`, `for` and `do`; inside a
   `switch` it continues the enclosing loop), `do … while` with prefix `invariant` and `decreases`, `switch` with
   `case`, `default` (anywhere among the labels) and fallthrough, `[[fallthrough]]`, and loops whose only exits are
   `break` or `return` (`for (;;)`, `while (1)`). **Still rejected** (invariant 3), each with a diagnostic at its
   line and a test: user `goto` and labels (M4.2 takes forward `goto`), GNU case ranges `case a ... b:` (EDG
   accepts them silently in both modes, P17), a `switch` with an init-statement or a condition declaration (P16), a
   `case`/`default` label that is not directly in its switch's own braces (Duff's device; EDG accepts it, P17, and
   Silicon crashes on a jump into a loop, S9c), a `switch` whose body has a statement before its first label
   (unreachable; EDG only warns, #111-D), a declaration in the switch's body that a later
   `case`/`default` label jumps over (the object is indeterminate there; EDG only warns, #546-D, in both modes
   unless the type has a constructor, P17), and a call in a `do` condition (like any loop condition, 0012 item 6).
   A body without braces (`switch (x) case 1: return 1;`) is accepted: EDG wraps it in a compiler-generated block,
   which is the switch's body (audit). Attributes on `case`/`default` labels (on the label statement) are held to the
   allow-list like any statement's (0011 item 7).
2. **VIR keeps the structure.** `While` gains a `step` (a `for` loop's increment, run after the body and after each
   `continue`), a form (`while`, `do`, or `do once` for `do … while (0)`, item 5) and a jump id; new statements `Switch(scrutinee, body, jump id)`,
   `Case(value | default)` (a marker inside a switch body), `Break(id)` and `Continue(id)`. `check_program` proves
   the jump rules from the nesting alone: a `Break` names the innermost enclosing loop or switch, a `Continue` the
   innermost enclosing loop (a switch between them is skipped), as C always does; a jump naming any other target is
   an internal inconsistency, so a mis-mapped label cannot leave the wrong construct (audit); ids are unique; a `Case` stands directly in its switch's body, its value is a call-free expression of the
   scrutinee's type (constant values are distinct), there is at most one default; a non-empty switch body starts with a `Case`; no declaration comes
   before a later `Case` of the same body. Every jump is therefore forward and none enters a loop: no
   control-flow-graph analysis is needed, and lowering owns every label.
3. **Lowering** uses the lowered `Goto` and `Label` only:
   `while (c) invariants decreases { body; label l_cont__N; step } label l_brk__N`. A label is emitted only when
   a jump targets it, so programs without these constructs lower exactly as in M3 (checked by comparing every
   emitted `.vpr` of the M3 tests with the baseline). The locals a loop assigns (for its automatic range
   invariants, 0012 item 8) include those assigned in `step` and in switch bodies.
4. **Invariants and measures with `break` and `continue`** (refines 0012 item 8). On a `continue` path C evaluates
   the condition next (after a `for` loop's step), so the invariants, the condition's obligations and the measure
   are checked there, exactly as at the end of the body: a failure is `invariant-preserved` at the clause, or
   `termination` at the measure. On a `break` path C does not evaluate the condition, so nothing is checked. After
   the loop, what is known is the disjunction of the exits: the normal exit (the invariants and the negated
   condition) or the state at one of the `break`s. 0013 is unchanged: an iteration left by `break` is not followed
   by another, so its measure is not compared.
5. **`do { B } while (c);`**
   - **Meaning:** the invariant holds at the start of every iteration: on entry, and after each evaluation of `c`
     that is true. (For `while` and `for` this is the same as "where the condition is evaluated"; for `do` the first
     iteration starts without one, so the Annotations page says "at the start of every iteration".)
   - **Encoding:** `while (true) invariants decreases { B; label l_cont__N; assert <obligations of c>;
     if (!c) { goto l_brk__N } } label l_brk__N`. The condition's obligations are asserts in the body, not invariant
     clauses: C evaluates `c` after `B`, never on entry. No call may appear in `c`.
   - **`do … while (0)` needs no measure** (owner, 2026-10-20; amends 0013 item 3). When the condition is a
     constant whose value is zero after EDG's folding (`0`, `false`, `1 - 1`: an `enk_constant` with value 0, P14),
     the body runs exactly once, so there is no loop to terminate. It is lowered as straight-line code, not as a
     Viper loop: `B; label l_cont__N; assert <obligations of c>; label l_brk__N`. The condition is still encoded
     (by its original expression when EDG recorded one, 0010 item 12), so its obligations are checked where C
     evaluates it, after the body. A Viper loop with `decreases 0` was rejected: it verifies termination but forgets
     what the body assigned (S7f: `res == 1` not provable; S7f2, the straight-line form, verifies). A written
     `invariant` is asserted once, at entry (`invariant-entry`), the start of the only iteration; a written
     `decreases` has its obligations checked there and is otherwise unused, since no iteration follows another.
     Only `do` has this exemption (`while (0)` and `for (;0;)` are unchanged). The criterion is sound because the
     encoded condition is the original expression (EDG folds `1 << 40`, `(signed char)256` and, in C23,
     `2147483647 + 1` to constants and keeps their `expr`, audit), whose obligations are asserted, and because a
     global, an enumerator or a call in it is rejected (0010 item 12). As a guard, lowering also asserts `!c` there
     as an `internal` check: if EDG's folded zero ever disagreed with cerastes's value, the run ends with exit 3.
     `check_program` re-evaluates the condition of a `DoOnce`: it rejects one that reads a local, calls a function,
     or folds cleanly to a value other than zero, and accepts one it cannot fold (a shift, an inexact conversion to
     a signed type, an overflow), whose failing obligation is then reported (`do … while ((signed char)256)` is a
     `conversion` error, `do … while (1 << 40)` a `shift` error) and whose value the internal assert pins.
6. **`switch (e) { … }`** (C11/C23 6.8.4.2, [stmt.switch]). The controlling expression is promoted and evaluated
   once, into a temporary; a call in it is allowed (it is evaluated exactly once), and its obligations are placed by
   0012 item 6 (in `switch (100 / g(x))` the divisor's check follows the call). Dispatch is a chain `if (t == k) { goto l_case_k }` in source order of the labels,
   then `goto` the default label, or the switch's end when there is none. The body follows in source order with a
   label at each `Case` marker, so fallthrough is falling into the next label and `break` is `goto l_brk__N`. EDG
   promotes the scrutinee with an explicit conversion where C requires one and gives each case value already
   converted to the promoted type (`case -1` on an `unsigned int` scrutinee is `4294967295`, P15), but it folds
   case values it should not: `case 1 << 40:` gets only warning #63 and becomes 0, `case 2147483647 + 1:` (C23)
   only #61, `case 1 << 31:` (C23) nothing, and verify mode suppresses those warnings (audit, blocking). So each case
   value is encoded **by its original expression**, as every folded constant is (0010 item 12): its obligations
   (`shift`, `overflow`, `conversion`) are asserted unconditionally before the dispatch, at the case label, and a
   global, an enumerator or a call in it is rejected; so is a case value that names a variable (`case k:` with a
   C++ `const int k`; M4.2 may lift it), at the label. A case value whose type differs from the scrutinee's is
   rejected (fail closed). A `for` loop with an init-statement is its own block, so a `for (int k = …)` inside a
   case is not a declaration a later label jumps over (code audit). Duplicate values and a second `default` are EDG errors. Silicon explores each case path
   separately.
7. **IL facts relied on** (`internal/il-inventory.md`, "M4.1: control flow"):
   - (P13) `break` and `continue` are compiler-generated `stmk_goto`s; the target label carries `break_label`,
     `continue_label` and, for a switch, `switch_break_label` as well as `break_label`. No field links a label to
     its construct: the encoder takes the innermost enclosing loop (or loop or switch, for `break`) and checks the
     shape — the break label is the next sibling of the loop or switch statement, the continue label is the last
     statement of the loop body (or of the generated block that wraps it); anything else is rejected (fail closed).
     Labels exist only where a jump uses them. C23 wraps more statements in compiler-generated blocks than C++20,
     so the encoder matches shapes, not depths. A `for` loop's increment is `a_for_loop.increment`.
   - (P14) `do` is `stmk_end_test_while` with `expr` (the condition) and `loop_statement`; it has no keyword
     positions (the keyword is found in the source text, as for loops in M3); prefix `invariant`/`decreases` attach
     to the statement.
   - (P15) the scrutinee carries EDG's promotion; case labels are flat siblings in the body block, each with
     `case_value` and `range_end`.
   - (P16) every C++ form with an init-statement or a declaration has `expr.kind == enk_condition`.
   - (P18) `[[fallthrough]]` (and its `clang::`, `gnu::` and `__attribute__` spellings) is a `stmk_empty` with an
     attribute named `fallthrough`, in both modes. The allow-list admits it without a namespace or in `gnu`
     (`[[gnu::fallthrough]]`, `__attribute__((fallthrough))`); `[[clang::fallthrough]]` is rejected
     (`unsupported_attribute`), like every `clang::` attribute.
8. **Spike results** (Silicon `v.26.08-release`, `build/m4.1/spikes/`): a `break` leaves a loop with the facts of
   its own path only (S1: an assert true only on the normal exit fails); a `continue` label before the `for` step
   verifies and one after it fails termination at the measure (S2); a `continue` path that breaks an invariant is
   `invariant.not.preserved` at the clause (S3); nested loops, a switch's break label inside a loop body and a
   `continue` from inside a switch behave as C does (S4–S6); the `do` shape verifies, with entry, preservation and
   termination twins failing as expected, and a negative measure in the last iteration accepted (S7); `while (true)`
   left only by `break` verifies (S8); dispatch with fallthrough and a default in the middle verifies, a label
   inside an `if` branch can be a target (S9); a jump into a loop body makes Silicon throw (S9c: no verdict, exit 3
   from the driver), which item 2's nesting rule excludes by construction; a `goto` out of loops with measures and
   labels inside loop bodies are accepted (S10).
9. **Positions and categories.** No new error category. `break`, `continue`, labels and the dispatch carry no
   checks and no position-map entries. Code after a `break` is unreachable in C and in Viper, so leaving it
   unchecked is sound. A loop with no exit other than `break` or `return`: the code after it is reached only through
   a `break`, and `missing-return` at the closing brace can fail only if such a path falls off the end.
10. **Tooling.** `scripts/lint_viper.py` rejects a backward `goto`, a label carrying invariants and a duplicate
    label in a method: each would make a loop that Silicon never checks for termination or invariants.
    `scripts/mutate.py`'s `delete-stmt` deletes `break;`, `continue;` and the statement after a `case`/`default`
    label (never the label).
11. **`[[fallthrough]]`** joins the attribute allow-list (amends 0011 item 7): it has no run-time meaning, and GCC's
    `-Wextra` (`-Wimplicit-fallthrough`) needs it for warning-free code with intentional fallthrough. Verify mode
    also suppresses EDG's warning #546 (a jump past an initialization, P17; amends 0011 item 7 and 0010 item 12's
    list): cerastes rejects every such jump itself (item 1; a user `goto` is always rejected in M4.1), at the label
    that jumps, naming the variable, so the problem is reported once. Warning #111 (an unreachable statement) is not
    suppressed. Rejection positions: a nested label, a case range and a jump over a declaration at the label; a
    statement before the first label at that statement; an init-statement, a condition declaration or a body that is
    not a compound statement at the `switch`.
12. **Amends** 0012 items 3 (statements), 8 (loops) and 12 (rejections), 0011 item 7, 0013 item 3, and
    `internal/encoding.md` §2, §3, §7 and §8.

## Consequences

- C's loop exits and `switch` verify with the same semantics as the rest of M3; the user-visible rules are on the
  Verifying a program and Annotations pages.
- Every jump is forward and structured, so the soundness of the jump encoding reduces to Silicon's handling of
  forward `goto`s (the spikes) and to `check_program`'s nesting rules.
- `goto` and labels written by the user remain rejected until M4.2.

*2026-10-21, amended by 0015 (M4.2a): items 1 and 2 — a user `goto` is accepted when its label is a later statement of a block enclosing it with no declaration between (outward and forward); `check_program` proves this from the nesting too, so every jump is still forward and none enters a loop; item 3 — lowering also emits `l_user_<name>__N` labels, only where a `goto` targets them; item 9 — gotos and labels carry no checks; item 11 — #546 stays suppressed: cerastes rejects every jump past a declaration, a user `goto` included.*
