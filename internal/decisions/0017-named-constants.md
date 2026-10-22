# 0017 — Named constants and case values (M4.2b1)

- **Status:** accepted for M4.2b1. Owner decisions (2026-10-21, M4.2b planning): M4.2b is split into M4.2b1 (this
  record) and M4.2b2 (enumerations, the next free record); the initializers of named constants and the case values
  are checked in a **prologue at the entry of every method**, not in a separate generated method; the M3 rejection
  test `reject_case_variable.cpp` is retired (pre-approved 2026-10-21); `reject_global.c` (a mutable global) stays.
  Facts marked (P*n*) are IL probes and (S*n*) Silicon spikes, recorded with evidence in `internal/il-inventory.md`,
  "M4.2b1"; code relies on them only once confirmed.
- **Milestone:** M4.2b1 (2026-10-21).

## Context

M3 rejected every global, `const` and `constexpr` ones included, because a use is a folded constant whose `expr`
names the global and the global's initializer, which can narrow or shift, was never encoded (0012 item 12, 0010
item 12). It rejected a case value that names a variable (0014 item 6), and it checks a case value's obligations
before the dispatch, on the paths that reach the `switch` only: `if (0) switch (x) { case 1 << 40: … }` is never
checked, although an overflowing or out-of-range constant expression in a case label violates a constraint
(C23 6.6p4, 6.8.4.2p3; C++ [stmt.switch]/2 requires a converted constant expression). The M4.2 planning review found
a cross-mode trap: a `const` variable in a `do … while` condition is folded to a constant by C++ (so `do … while (k)`
with `k == 0` would be a `DoOnce`, 0014 item 5) but is a variable in C (a measured loop), so a `.c` test would have
two meanings.

## Decision

1. **Named constants.** A *named constant* is a namespace-scope (file-scope) variable, defined in a verified file
   (0011 item 4, decided on its first declaration's position), that is `const` (or, in C++, `constexpr`), has a type
   the encoder accepts as a value (a builtin integer type or `bool`; not `volatile` or `_Atomic`, 0010 item 2), is
   not `thread_local`, and is constant-initialized: EDG records `initk_static` with its value in the variable's
   `initializer.constant` (P23: in both modes for `short`, `bool`, `unsigned`, `signed char`, `static const`,
   `extern const int d = 5;` (a definition), C++ `inline constexpr`, and `{}` initializers). In C, a file-scope
   `const` definition without an initializer and without `extern` (`const int z;`, a tentative definition: EDG
   records `initk_none` with storage class unspecified, exactly like `int w;`, P23) is zero-initialized (C23 6.7.11,
   6.9.2) and is a named constant with value 0. C23 `constexpr` objects do not parse in this EDG configuration
   (errors #65/#20, P23), so in C every named constant is `const`.
2. **Still rejected**, each with a diagnostic and a test (invariant 3), at the declaration
   (`unsupported_declaration`) unless noted:
   - a global that is not `const` (`reject_global.c`, unchanged): its value can change, so it needs the frames of
     M4.5b;
   - an `extern const` declared here and not defined in this unit (`initk_none`, `sc_extern`): its value is not known
     here;
   - dynamic initialization (`initk_dynamic`): in C++ a `const int` whose initializer is not a constant expression is
     dynamically initialized, which covers `= f()`, `= 2147483647 + 1` (C23 folds it instead, item 8) and a constant
     read in its own initializer (`const int g = g + 1;`, which reads `g` before it is initialized) (P23);
   - `thread_local`/`_Thread_local` constants (EDG's `is_thread_local`; `initk_static` otherwise, P23);
   - a `volatile` constant (`unsupported_type`, as for locals; EDG folds its uses in C but not in C++, P23);
   - an initializer whose original expression is outside the encoder's allow-list (C++ `const int zv = int();`, an
     `enk_temp_init`, P23) is rejected like any such expression (`unsupported_expression`);
   - a global declared in a system file, at its **use** in verified code (`unsupported_expression`): system files are
     used only through contracts (0011 item 4), and a global's value is not a contract;
   - `static const` locals stay rejected with the other static locals (M4.5b);
   - in C, a case value that names a variable: a `const` object is not an integer constant expression in C (C23
     6.6p8), and C23's `constexpr` is not available (item 1). EDG accepts it with warning #1901 (P23), so cerastes
     rejects it at the case label (`unsupported_expression`), and verify mode suppresses #1901 (amends 0011 item 7).
3. **A use of a named constant is EDG's value.** Wherever a named constant is read — a function body, `requires`,
   `ensures`, `check`, a loop invariant, `decreases`, a case value, another constant's initializer — the encoder
   gives a literal of the constant's type holding EDG's value of its initializer (`initializer.constant`). In both
   modes a use is a folded constant whose `expr` chain ends at the variable, with any promotion inside the chain
   (P10, P23: 21 uses probed, each with the initializer's value after the chain's casts); the C tentative definition
   (item 1) is not folded, and its uses are variable reads. Both shapes reach the same lookup, so the VIR is the same
   in both modes. Contracts may now name constants (0010 item 12 rejected every variable there). A reference to a
   local `const` variable in a body is still that local (0010 item 12).
4. **Each initializer is checked as written, in a prologue.** Every Viper method that has a body (each function's
   method and its contract method `__wf`, 0010 item 11) begins, after the copies of its parameters, with a
   *prologue* that, for each named constant of the unit in definition order:
   - encodes the initializer **by its original expression** (0010 item 12, recursively; calls rejected), and asserts
     its obligations (`overflow`, `shift`, `conversion`, `division-by-zero`) at their own source positions, in the
     constant's declaration;
   - then asserts, as an **`internal`** check at the declaration, that the value as written equals EDG's value. If
     EDG's folding ever disagreed with cerastes's semantics, the run ends with exit 3 (0011 item 8), never with a
     result. The guard is omitted when the initializer is a plain literal equal to the value; a definition without
     an initializer (C, `initk_zero`) has neither.

   Every constant is checked whether or not anything uses it, and in every method: a file whose only content is
   constants has no method and is not accepted (0011: "nothing to verify", exit 2). Obligations come before their
   guard, so a constant whose initializer fails (`const int A = 1 << 40;`, which EDG folds to 0) reports the
   obligation, not a cerastes bug. Silicon ends a path at its first failed assert (S14), so no method goes on to use
   EDG's value of a constant whose initializer failed, and the same failure in several methods is one error after the
   driver removes identical errors (`report.cpp`). **Cost** (owner, accepted): a failing initializer hides every
   other error of the unit, since every method stops at it.
5. **Case values are checked in the same prologue, path-independently.** After the constants, the prologue of a
   function's method checks each case value of its body (in source order, nested switches included): the value
   encoded by its original expression, its obligations asserted at the case label (with the note "in the value of
   this case label" when an obligation's position is on another line, as in 0014 item 6), then an `internal` guard
   that the value as written equals EDG's `case_value` (P24). The dispatch compares the scrutinee with EDG's value,
   which the guard proves equal to the value as written, and asserts nothing. Case values are local-free, so they
   can be evaluated at entry. Consequences: `if (0) switch (x) { case 1 << 40: … }` is now a `shift` error; the
   three twins `switch_case_expr_shift.c`, `switch_case_expr_conversion.c` and `switch_case_expr_over.c` keep their
   single errors (S15); like a constant, a failing case value hides the function's other errors.
6. **`case k` and `case N` (C++).** In C++ a case value may name a named constant (`case N:`, `case N + 1:`) and a
   `const` or `constexpr` local whose initializer EDG folded (`const int k = 5; … case k:`, `constexpr int n = 3; …
   case n:`): its `case_value` is a constant of the promoted scrutinee type whose `expr` chain reaches the variable
   (P24). A local `const int k = 5;` is dynamically initialized in the IL (`initk_dynamic`), with its value in the
   declaration's `dik_constant` initializer (P23). Such a local is, inside case values only, a named constant of its
   function: its initializer is checked in that function's prologue (obligations, then the guard), before the case
   values, and the case value uses EDG's value. In the body the local stays an ordinary local (item 3), assigned at
   its declaration, so both readings agree. `vir::Var` needs no `const` flag: no local reaches a case value. In C the
   same case values are rejected (item 2).
7. **`do … while (c)` is a `DoOnce` only when `c` names no variable** (amends 0014 item 5). The exemption from the
   measure applies when EDG folds `c` to zero *and* `c`, following every constant's `expr`, names no variable or
   parameter (`enk_variable`, `enk_param_ref`). In this configuration EDG folds a `const` variable in a `do`
   condition in both modes (C as an extension; the folded constant keeps the variable in its `expr` chain, P24), so
   without this rule `do … while (k)` would escape the measure through EDG's extension; with it, it is a measured
   loop in both modes, and without a `decreases` it is rejected (`missing_decreases`) in both. The rule does not
   depend on whether EDG folds. `do … while (0)`, `while (false)`, `while (1 - 1)` keep the exemption.
   (Enumerators are M4.2b2's.)
8. **Mode independence.** A named constant has one value in both modes: EDG's value of the same initializer, checked
   as written with the C/C++ meaning of 0010, which is the same in both modes. Where the front ends differ, the
   program is not accepted in one mode and the test runs only in the other, with the reason in its header: C++
   makes an initializer that is not a constant expression a dynamic initialization (rejected), which C23 folds (so
   `const int A = 2147483647 + 1;` is an `overflow` error in C and rejected in C++); `const int z;` is C only; `case k`,
   `case N` and `constexpr` are C++ only (item 2). `const int A = 1 << 40;` is static in both modes (EDG folds it to
   0 with warning #63) and a `shift` error in both.
9. **Self-test hook** (0011 item 13): `--cerastes_verify_selftest=perturb_folded` moves every named constant's value
   and every case value's EDG value to a neighbouring value of its type (`bool` flipped), so the `internal` guards
   fail and the run ends with exit 3. Its harness self-test shows the guards fire.
10. **Facts relied on** (`internal/il-inventory.md`, "M4.2b1"): (P23) `init_kind` and `initializer.constant` per
    kind of definition and per mode, the shapes of uses in bodies, contracts and annotations, EDG's diagnostics;
    (P24) the `case_value` chain of a case naming a constant, and the condition of `do … while` naming one;
    (S14) a failed assert ends its path (two failing asserts and a failing postcondition in one method give one
    error), and the same failing assert in two methods is reported once per method; (S15) the prologue form keeps the
    three case-value twins' single errors and verifies `switch_case_expr.c`, and a false guard placed after its own
    case's obligations is never reached when an obligation fails.
11. **Amends** 0010 items 12 and 13; 0011 items 7 (#1901 suppressed in verify mode) and 13; 0012 items 4 and 12;
    0014 items 5 and 6; and `internal/encoding.md` §2, §3, §4, §7 and §8.

## Consequences

- `const`/`constexpr` globals and `case k`/`case N` verify; the rules are on the Integer semantics (constants),
  Verifying a program (switch, what is not supported) and Annotations (constants in contracts; `do … while (0)`)
  pages.
- Every constant expression the encoder sees is checked as written, wherever it is, and the value EDG folded is
  trusted only after an `internal` guard has compared the two.
- Constants are immutable, so 0012 item 4's argument (a callee cannot change the caller's state) still holds; mutable
  globals wait for M4.5b's frames.
- Emitted Viper changes only for functions with a switch whose case value is not a plain literal (the prologue);
  every other test's `.vpr` is unchanged.
