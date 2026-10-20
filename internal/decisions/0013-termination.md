# 0013 — Termination: `decreases` and total correctness (M3)

- **Status:** accepted (owner decision 2026-10-16: "check termination in M3")
- **Milestone:** M3 (2026-10-16)

## Context

M3 as first written verified partial correctness: a function that never returns satisfies any `ensures`. The M3
pre-commit audit pointed out that this is not merely incomplete in C++: a loop or recursion with no observable side
effects that never ends is undefined behaviour ([intro.progress]), and M3 code has no I/O, `volatile` or atomics,
so *every* non-terminating loop or recursion in it is undefined. A compiler may assume such code ends and fall
through. C (6.8.5p6) exempts loops whose controlling expression is a constant, but not other loops; cerastes keeps
one rule for both modes.

The owner chose to check termination in M3, with a measure the user writes.

Spikes (2026-10-16, Silicon `v.26.08-release`, silver's termination plugin, `TerminationErrors.scala`):
- methods and loops accept `decreases e1, …, en` (lexicographic, on `Int`: each component bounded below by 0 and
  decreasing), `decreases` with no measure (a method that terminates without recursion), and `decreases _`
  (termination assumed);
- a recursive call whose measure does not decrease, and a loop whose measure does not decrease or is unbounded, are
  errors with id `termination.failed` ("Method might not terminate." / "Loop might not terminate."; reasons
  `tuple.false` — three messages — `tuple.condition.false` and `termination.condition.false`);
- a call from a method with a measure to a method without one is an error (`tuple.condition.false`), so a missing
  measure cannot be hidden behind a call.

## Decision

1. **A new annotation, `decreases`** (`[[cerastes::decreases(e1, …, en)]]`, macro `CERASTES_DECREASES(...)`), with
   one or more integer expressions (a lexicographic measure). It is written
   - before a `while` or `for` statement, like `invariant` (prefix only; the shadowing rule of 0006 item 2
     applies), or
   - after a function's parameter list, like `requires`/`ensures` (it is part of the contract: must-match across
     declarations, 0006 item 6).

   Its arguments are side-effect-free C/C++ expressions of integer type, checked like any annotation argument (0006
   item 10, 0010 item 11). If one registered attribute cannot have both placements in EDG's registration mechanism,
   the fork gets a small series commit for it (general: one name, several targets); no second name is introduced.
2. **Meaning** (Viper's well-founded lexicographic order, M3 audit and M3-4 spikes): the measure must decrease from
   each step to the next, that is, at the first component whose value changes, the new value is smaller than the
   old one and the old one is non-negative; the components after it are unconstrained (a later component may be
   negative while an earlier one decreases). The check applies only between two consecutive steps: a loop's
   measure must be non-negative at the start of every iteration that is followed by another one, and may be
   negative at the start of the last (with `invariant 0 <= i && i <= n`, `decreases(n - i - 2)` verifies and
   `decreases(n - i - 3)` does not). That is enough for termination: an infinite run would need an infinite
   decreasing chain of non-negative values. It is compared:
   - for a loop, from the start of an iteration (where the condition is evaluated) to the end of that iteration
     (after the `for` increment);
   - for a function, from the function's entry (over the parameters' entry values) to every call of a function in
     the same recursion cycle (the callee's measure over the call's arguments).
3. **Every loop needs a `decreases`.** A `while` or `for` loop without one is rejected (`missing_decreases`).
4. **Every function in a recursion cycle needs a `decreases`.** The encoder builds the call graph of the verified
   functions; a function in a cycle (direct or mutual recursion) without a measure is rejected
   (`missing_decreases`). A function outside any cycle needs none: it is encoded with an empty measure
   (`decreases`), which Silicon checks too (it fails if the function is in fact recursive).
5. **Trusted contracts include termination.** A function used only through its contract (0011 item 6) is assumed
   to terminate (`decreases _`), as part of the trusted contract and its `// cerastes:trusted-contract` tag. This is
   part of the single-unit assumption, and it has a limit the success line and the docs state: **a recursion cycle
   that passes through a function defined in another unit is not detected** (unit A: `f` calls the trusted `g`;
   unit B: `g` calls the trusted `f`; each unit verifies, the program does not terminate). Assume-guarantee across
   units is sound for the contracts, not for termination. Checking it needs the cross-unit call graph, which comes
   with the contract manifest (deferred, handoff §6). In M3 such a cycle can only go through user code in another
   unit: there are no function pointers, and system functions cannot call user code.
   `decreases _` is an assumption (invariant 4): `scripts/lint_viper.py` allows it only on a bodyless method that
   carries the trusted-contract tag, and requires a `decreases` clause on every other method (the `__wf` methods
   included, with an empty measure) and on every loop, because Silicon checks termination only where a clause is
   present. Since the M3 close-out (2026-10-20) it also rejects `decreases *` (Viper's "no termination check") and
   a conditional `decreases <measure> if <condition>`, which the emitter never produces: either would leave a method
   or loop unchecked for termination. The lint reads one line at a time, so a clause split over lines is not
   seen; the emitter writes each clause on one line (handoff, should-fix).
6. **Results are total correctness.** A verified function terminates on every input that satisfies its
   `requires` and has no undefined behaviour on the way; partial correctness is gone from the records and the docs.
7. **Well-formedness.** A measure's own C/C++ obligations (overflow, division by zero, …) are checked where it is
   evaluated: a loop's as invariant clauses at the loop head (0012 item 8), a function's in its `__wf` method,
   under its `requires`.
8. **A new error category, `termination`**: a measure may be negative or may not decrease. It is reported at the
   loop's `decreases` clause (loops), or at the recursive call with a note at the callee's `decreases` (functions).
   Silicon's `termination.failed` errors map to it, with the reasons `tuple.false`, `tuple.condition.false` or
   `termination.condition.false` (and only for that id); anything else is exit 3. `cerastes --selftest` includes a
   must-fail termination run, so a termination check that is switched off cannot make results vacuous, and the
   tests include a loop left by `return` (encoded as a `goto` out of the Viper loop). The category is added to 0011 item 10 and to every copy of the table (harness, README, error-messages page).
9. **Rejected:** `decreases` anywhere else (placement errors, EDG/cerastes as for `invariant` and `requires`), a
   non-integer measure, and a measure on a loop or function that cerastes rejects anyway.

## Consequences

- Every loop in a verified program, and every recursive function, carries a measure. The M3 tests and doc examples
  gain them; tests that pinned partial correctness change (they were never committed).
- `cerastes verify`'s success means termination too; the Verifying page and 0012 item 6 say so.
- The annotation is user-visible: the Annotations page, `<cerastes.h>`, the ordinary-compilers page and the M2-layer
  checks (placement, side effects, shadowing) cover it, each with tests.
- Silicon reports a non-decreasing loop at the measure's position, and a recursive call at the call: both are C/C++
  lines through the position map.
