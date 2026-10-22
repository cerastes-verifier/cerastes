# 0019 — Locals without an initializer (M4.2c)

- **Status:** accepted for M4.2c. Owner decisions (2026-10-22, M4.2c planning):
  - **The new verification error category `uninitialized`.** Reading an uninitialized object is the first rule of the
    C++ profiles papers: P4222R1, "An initialization profile", §4.2.
  - **The check is path-precise.** It is decided by Silicon on the paths the contract allows, not by a syntactic
    definite-assignment rule.
  - **Jumps past declarations.** A `goto` or a `case` label may now jump past a declaration *without* an initializer.
  - **The M3 rejection test `reject_uninit_local.c` is retired** (pre-approved 2026-10-21).
  - **Handoff §11 questions 17–19 are settled in the same session:**
    - item 9, the front end's internal error #2656;
    - item 10, ordering of one unscoped enumeration, amending 0018;
    - item 11, `-Wswitch`'s two rules, amending 0018.

  Facts marked (P*n*) and (S*n*) are probes and Silicon spikes, recorded with their evidence in
  `internal/il-inventory.md`, "M4.2c" (P28–P31, S16).
- **Milestone:** M4.2c (2026-10-22).

## Context

M3 rejected a local declared without an initializer (0012 items 3 and 12): its value is indeterminate, and the scalar
encoder had no notion of a value that may not be read.

- **C.** The object's value is indeterminate when its lifetime begins. It becomes indeterminate again each time its
  declaration is reached (C23 6.2.4p6). Reading it is undefined when the object could have been declared `register`
  (C23 6.3.2.1p2), and every local accepted today could have been.
- **C++.** Reading an indeterminate value is undefined behaviour ([basic.indet]), except for copies of `unsigned char`
  and `std::byte`. C++26 makes it erroneous behaviour (P2795).
- **The profiles.** P4222R1 makes "every object is initialized before it is read" a profile guarantee. Its rule is
  local and conservative ("consider all branches of a run-time conditional statement executed", §1.3), and it asks
  for an `[[uninit]]` marker on a definition that is deliberately left uninitialized (§4.2).

A verifier can give the same guarantee on each feasible path without the marker. That is what the owner chose.

C and C++ also allow a jump past such a declaration: a forward `goto`, or a `case` label of a switch body. C++20
[stmt.dcl]/3 allows it when the variable has scalar type and no initializer; C allows any jump that does not enter the
scope of a variably modified type. 0014 item 7 and 0015 item 2 rejected every jump past a declaration, because the
object would be indeterminate at the target. With this record that object is ordinary.

## Decision

1. **Scope.** A local of an accepted scalar type (an integer type, `bool`, an enumeration: 0012, 0018) declared
   without an initializer (`initk_none` with no `stmk_init` statement, P28), in both modes. This includes each
   declarator of a declaration with several (`int x, y = 1;`: one `stmk_decl`, whose initializers follow it, P28), a
   `for` loop's init-statement and C's `register` locals.
   - **Unchanged:** a local initialized from itself (`int x = x;`) stays rejected (0012 item 3; the front end's
     pending-initialization state, not a read). `static`, `thread_local` and `extern` locals stay rejected.
     `int x, y = x;` is an `uninitialized` read of `x`.
   - **C++ value-initialization** (`int x{};`, `int x = {};`, `bool b{};`) is already an initializer: `initk_dynamic`
     with a constant 0, accepted since M3 (P28). `int x = int();` stays rejected (`enk_temp_init`).
   - **A `const` local without an initializer stays rejected** (`unsupported_declaration`). It can never be given a
     value, C++ refuses it (error #257), and the C front end warns (#257-D, P28).
2. **Meaning.**
   - **Indeterminate.** Such a local holds an indeterminate value from the start of its block until it is assigned, and
     again each time its declaration is reached (C23 6.2.4p6; in C++ each execution of the declaration makes a new
     object).
   - **What assigns it.** An assignment `x = e` gives it a value, and so does `x op= e` when its read succeeds.
   - **What reads it.** Every evaluation of its value is a read:
     - an operand in code, a condition, the controlling expression of a `switch`;
     - a `return` value, a call argument, the right-hand side of an assignment or initializer;
     - the read inside `x op= e`, `++x`, `x++`, `--x` and `x--`;
     - any mention in an annotation (`check`, `assert`, `invariant`, `decreases`).
   - **The error.** A read that may see an indeterminate value is a verification error, category **`uninitialized`**,
     reported at the read: "`x` may be read here before it is given a value".

   This is stricter than the standards in three places, stated on the Verifying a program page:
   - no exception is made for C++'s `unsigned char` and `std::byte` copies;
   - in C++ a discarded-value expression `x;` or `(void)x;` of a non-volatile scalar is not a read ([expr.context]/2),
     but cerastes treats it as one, as C does (C23 6.3.2.1p2), so it means the same in both modes;
   - C's distinction between objects whose address is taken and others does not arise (no `&` before M4.5).

   An unevaluated operand (`sizeof`, `_Alignof`, `typeof`) is not a read: the encoder uses its value, which does not
   depend on the object's contents.
3. **Path-precise.** The error is reported only on paths that the `requires` clauses and the program's own conditions
   allow, and Silicon decides it like every other obligation. So `int x; if (c) x = 1; if (c) return x;` verifies.
   A verified function never reads an indeterminate value, which is P4222's guarantee for that function. cerastes
   asks for no `[[uninit]]` marker: it verifies C as well, and a declaration without an initializer is ordinary C. A
   stricter mode that also requires P4222's markers is left to a later record.
4. **Encoding** (`src/vir/lower.cpp`; VIR: a `Decl` with no initializer expression).
   - **The flag.** Each local that has a declaration without an initializer gets a ghost Viper `Bool` local, its
     **flag**. Its name is `d_` followed by the variable's Viper name; no other generated name starts with `d_`. It is
     true exactly when the C/C++ object holds a value.
   - **Reset** (`flag := false`, and the variable set to a fixed in-range value of its type, item 5) happens in three
     places:
     - at the declaration;
     - at the entry of the block that directly contains the declaration, before its first statement;
     - for a declaration directly in the body of a `switch`, before the switch's dispatch, whose jumps to the case
       labels would otherwise pass the block's entry.
   - **Write.** Every assignment to the variable also sets `flag := true`, including `x op= e` and `++`/`--`.
   - **Read.** Every read of the variable is an obligation `flag` (category `uninitialized`) at the read's position. It
     is built by the same machinery as every other obligation (0012 item 8), so a read in the right operand of `&&`
     or `||` or in an arm of `?:` is guarded like a division there. A read in a loop condition, invariant or measure
     is an invariant clause, checked on entry and after each iteration.
   - **Loops.** Viper havocs every local a loop assigns. For each flagged variable that a loop assigns and that is
     declared outside it:
     - a snapshot `flag_at := flag` is assigned before the loop;
     - the loop gets the invariant `flag_at ==> flag` (category `internal`, like the automatic range invariant), so a
       value given before the loop is still known inside it and after it.

     Without the snapshot invariant, the flag of a variable assigned before the loop is lost at the loop head (S16
     (b'), a false error). The loop's invariants come in this order: the automatic range invariants, these snapshot
     invariants, then the user's clauses with their obligations. The snapshot is named `ds_` followed by the
     variable's Viper name and the loop's number.

     A variable declared inside the loop needs nothing: its flag is reset in every iteration before any read.
   - **Unchanged emitted Viper.** None of this is emitted for a function without such a local, so the emitted Viper of
     every other program is unchanged.
5. **Why it is sound.**
   - **The flag is false whenever the object is indeterminate.**
     - Every path to a statement of the declaring block passes the block's entry. A `goto` is forward and outward
       (0015 item 2), so it starts inside the block. A `case` label is reached only through its switch's dispatch,
       which comes after the reset for the switch body.
     - Every execution of the declaration resets the flag.
     - Only an assignment, after its own reads, sets the flag.
     - The flag is never assigned anywhere else.
   - **So a verified function never reads an indeterminate value:** every read asserts the flag.
   - **The safety net (Opus audit).** Only a write sets the flag to true (the lowering of an assignment, after its
     reads), and Viper starts every local, and every local a loop assigns at its head, with an unknown value. A missing
     reset therefore leaves the flag unknown, which can only cause a false error, never a wrong "verified". The
     property lowering must protect is that `flag := true` follows nothing but a write of the variable.
   - **The fixed value is never observed on a verified path.** It only keeps the variable inside its type's range, so
     the automatic range invariant of 0012 item 8 holds for it on entry to a loop. Any path that reads it has already
     failed the flag assertion. After a failed assertion Silicon continues with the assertion assumed (S14), so a
     second error that depends on the fixed value can follow on the same path (for example a division by it). That
     changes only the list of errors, never a verdict.
     - **Where the flag was just reset**, the assumed assertion contradicts it and the path ends, so no second error
       follows. S16 (f) checked 24 shapes with the dummy 0, 1 or the type's minimum: one error each.
     - **A second error is possible only where the flag is unknown** (after a loop head), and there the variable may
       hold any value of its range anyway.
     - **The value** is 0 (`false` for `bool`). Every enumeration's value range contains 0 (0018 item 3), so 0 is
       always in range; lowering falls back to the low end of the range for a VIR type whose range does not contain
       0, which no program produces.
     - **The block-entry reset of the value is needed too.** Without it, a `goto` past the declaration leaves the
       variable unconstrained (S16 (d2): two spurious range errors).
   - **The loop invariant is inductive.** The flag of a variable declared outside a loop is reset only outside that
     loop, so inside the loop it can only become true. The invariant is checked like every other one, and nothing is
     assumed.
   - **No `inhale` or `assume` is emitted** (invariant 4).
6. **Jumps past a declaration without an initializer** are accepted (amends 0014 items 6–7 and 0015 item 2):
   - a `goto` that jumps forward and outward past such a declaration of the label's block;
   - a `case` or `default` label of a switch body that follows such a declaration in that body;
   - such a declaration before the first case label of a switch body (`switch (k) { int x; case 1: … }`). It is never
     executed, but `x` is in scope in the cases (P28).

   At the target the variable is in scope, and its flag is false (item 5). A jump past a declaration **with** an
   initializer stays rejected (`unsupported_statement`): C++ refuses it ([stmt.dcl]/3), and in C the skipped
   initialization would leave the object indeterminate, which a later record may accept the same way. VIR's
   `goto_problems` reports `skips_declaration` only for a declaration with an initializer, and `check_program`'s
   switch rule does the same.
7. **The front end's warnings** (P29).
   - **#549 goes by the order of the source, not by paths.** The front end prints `warning #549-D: variable "x" is used
     before its value is set` at the first use with no textual assignment before it, and never inside a loop or after
     a label.
     - It misses the one-armed `if (c) x = 1; return x;` and fires on the dead `if (0) return x;`. So it is no evidence
       either way.
     - A warning on stderr would also fail every test, since tests are warning-free.
   - **Verify mode turns the check off with `--no_use_before_set_warnings`.** Unlike `--diag_suppress=549`, a
     diagnostic pragma cannot turn it back on. The `uninitialized` check replaces it, as cerastes's own checks replace
     #546 and #1305 (amends 0011 item 7).
   - **Unchanged:** warnings #550 ("set but never used") and #177 ("declared but never referenced").
   - **#546.** The front end gives none for a jump past a declaration without an initializer. A C++ `goto` past an
     initialized declaration stays its hard error #546.
8. **Driver and harness.**
   - **The category.** `uninitialized` is a category of the driver's report (exit status 1, mapped through the position
     map like every other category, 0011 item 10), of `scripts/run_tests.py` and of the Error messages page.
   - **Silicon's errors.** Silicon reports a failed read as `assert.failed:assertion.false` at the read. A read in the
     invariant, measure or condition of a `while` or `for` loop fails as its obligation clause,
     `invariant.not.established` or `invariant.not.preserved` (S16). In a `do` loop's condition, evaluated after the
     body, and in every clause of a `do … while (0)`, it is an assertion. All of them map to `uninitialized` through
     the category of the position map.
9. **Internal error #2656 in verify mode** (handoff §11 question 17; P30).
   - **The failure.** In C++, an error #28 ("expression must have a constant value") from constant evaluation carries
     the evaluator's diagnostic as a note. When that diagnostic's number is one verify mode suppresses, the front end
     stops with internal error #2656, `determine_severity_code: bad severity`, and `cerastes verify` exits 3 instead of
     2. Every C++ constant-expression context does this: a `constexpr` variable, an enumerator, `static_assert`, an
     array bound, a template argument, `if constexpr`, `alignas`, a bit-field width. The shapes include
     `enum { A = 1 << 32 };` and `constexpr int z = 1 << 32;`. The note-capable numbers of the suppression list are #39,
     #62, #63 and #61 (the constant-expression interpreter's notes, P30). C has no such notes and never fails.
   - **The cause.** The front end applies a number's severity override to a note too, so a suppressed note has no
     severity, and the front end displays every note of a displayed error.
   - **No driver change** (owner, 2026-10-22). Making those numbers remarks (`--diag_remark`) stops the failure, but
     the front end then prints the error twice under a "1 error detected" summary: a second front-end bug in how an
     overridden note is displayed (P30). That output is inconsistent and cannot be pinned by a test. The failure is a
     tool failure (exit status 3), never a verdict, and the program is one C++ refuses anyway, so these shapes stay
     exit status 3 until the fix is in the pin, and no test uses them.
   - **The EDG side.** The bug is `internal/edg_bugs/0022-suppressed-note-internal-error.md`, reproduced on unmodified
     upstream `main` (`ec2f12cffa`). Its fix is one commit on the fork branch `0022`, based on upstream `main`: a
     suppressed note is left out of its error. A session with the x86-64 box runs the full suites, merges it into
     `cerastes` and moves the pin; then the C++ shapes give one error #28, exit status 2, with no driver change, and
     their tests land.
10. **Ordering of one unscoped enumeration** (handoff §11 question 18; amends 0018 items 6 and 7).
    - **Accepted.** `<`, `<=`, `>`, `>=` whose two operands are values of the same unscoped enumeration, or one of its
      enumerators.
    - **Why both modes agree** (P32). Both operands are converted to the same promoted type, and the promoted type
      holds every value of the enumeration:
      - **a fixed underlying type:** both modes promote by that type (C23 6.3.1.1p2, [conv.prom]/4; 0018 item 6), so
        `: unsigned long` compares in `unsigned long` and `: long` in `long` in both modes. The values are those of
        the type, so the conversion keeps them;
      - **no fixed type:** C converts to the compatible type (`unsigned int`, or `int` when an enumerator is negative,
        0018 P26) and C++ to `int`. The value range lies in `int` and, when it is non-negative, in `unsigned int`
        (0018 items 2–4), so both conversions keep each value.

      Converting two values of one range to one type that holds them keeps their order, so the comparison has one
      meaning in both modes. An enumerator operand is the same: in C it is an `int` constant whose folded `expr` names
      the enumerator, converted like the other operand, and its value lies in the range.
    - **Still rejected** (`unsupported_expression`):
      - ordering against any other integer, even a constant (`c < 2`): in C it is computed in `unsigned int` when no
        enumerator is negative, so `c < -1` is true in C and false in C++;
      - ordering two different enumerations.
11. **Switches over an enumeration: `-Wswitch`'s two rules** (handoff §11 question 19; amends 0018 item 6(ii) and 0014
    item 6). The rules apply, in both modes, to a `switch` whose controlling expression is an **enumeration value**:
    a variable, parameter, call result, cast or named constant of an enumeration type; an enumerator
    (`switch (RED)`; in C an `int` constant whose folded `expr` names it); or a `?:` whose arms are enumerators of one
    enumeration (in C++ a `?:` of enumeration type is rejected already, 0018 item 8). This is the strictest reading,
    g++'s (owner, 2026-10-22): clang never checks a constant controlling expression, and C gives an enumerator and
    such a `?:` type `int`, so the compilers disagree on those shapes (P31); applying the rules to all of them gives
    one meaning in both modes. A `switch ((int)c)` is a switch on an `int`.
    - **Rule 1.** A case whose value (the front end's) is not the value of an enumerator of that enumeration is
      rejected at the case label (`case_not_in_enumeration`). This covers `case 4:` and `case (enum color)7:` over
      `enum color { RED, GREEN, BLUE }`, and `case -1:` over an enumeration with a fixed `unsigned long` type.
    - **Rule 2.** A switch without a `default` label that has no case for the value of some enumerator is rejected at
      the `switch` (`enumerator_not_handled`), naming the first such enumerator.

    These are the two warnings of clang's and GCC's `-Wswitch`, part of `-Wall`, and both compilers refuse them under
    `-Werror` for a switch on an enumeration variable (P31, 68 shapes in both modes). Duplicate enumerator values are
    judged by value, as both compilers do. A case value is the front end's: the 0017 prologue guard already reports
    any written case value that folds differently. The owner's reason: such a program may be
    legal, but it is wrong. 0018 item 6(ii) had accepted `case 4:` as a case that is never taken. That is withdrawn,
    and so is the `#ifdef __CERASTES__` exception for native-ok tests (handoff §11 question 19; `tests/README.md`).
12. **Tests and documentation.**
    - **Tests** (invariant 2): must-verify and must-fail pairs for every rule above, in both modes where both accept the
      code. Each read form gets an `uninitialized` twin.
    - **Retired with the owner's approval** (2026-10-22): `reject_uninit_local.c`, `reject_enum_order.c`,
      `enum_case_promoted.c` and its twins `enum_case_promoted_taken.c` and `enum_case_promoted_negative.c`,
      `enum_case_cast_over.c` and `enum_fixed_long_case.c`. The eight equivalence entries of `enum_case_promoted.c` go
      with it.
    - **Pages:**
      - Verifying a program: locals without an initializer, jumps past declarations, switches over an enumeration;
      - Error messages: `uninitialized`;
      - Integer semantics: enumeration ordering and switches;
      - Overview.
13. **For later sessions.**
    - **M4.3** (assignments as values, the comma operator, `x++` inside expressions): an assignment inside an
      expression sets the flag at its point in the evaluation order.
    - **M4.5** (`&x`, structs, arrays): the flag follows the cell. A pointer to an uninitialized object is P4222's
      `[[ref_to_uninit]]`, member-wise initialization of structs is P4222 §5, and random access into an uninitialized
      array is the case P4222 bans (§5.4), which a path-precise flag per element could verify.
14. **Facts relied on** (`internal/il-inventory.md`, "M4.2c"):
    - (P28) the IL of a local without an initializer in both modes;
    - (P29) the front end's warnings on such locals and on jumps past them;
    - (P30) the note-capable numbers of #2656, the effect of `--diag_remark`, and the fork fix's evidence;
    - (P31) clang's and GCC's `-Wswitch` on each switch shape;
    - (P32) the conversions of an ordering comparison of enumeration values, per mode and underlying type;
    - (S16) the flag encoding in Silicon: resets, the loop snapshot, the error positions, the fixed value.
15. **Amends:**
    - 0011 items 7 and 10;
    - 0012 items 3 and 12;
    - 0014 items 6 and 7;
    - 0015 item 2;
    - 0018 items 6, 7 and 11;
    - `internal/encoding.md`.

## Consequences

- A local may be declared without an initializer and assigned later, in both modes. A read that may come first is an
  `uninitialized` error at the read.
- A `goto` or a `case` label may jump past such a declaration.
- `cerastes verify` still stops with an internal error (exit status 3) on an out-of-range constant expression in
  C++ until the fork fix 0022 is in the pin; then it refuses the program with the front end's error #28.
- Ordering two values of one unscoped enumeration verifies. A switch over an enumeration is held to `-Wswitch`.
- The emitted Viper of every program without a local declared without an initializer is unchanged.
