# 0017 — Named constants and case values (M4.2b1)

- **Status:** accepted for M4.2b1. Owner decisions (2026-10-21, M4.2b planning): M4.2b is split into M4.2b1 (this
  record) and M4.2b2 (enumerations, the next free record); the initializers of named constants and the case values
  are checked in a **prologue at the entry of every method**, not in a separate generated method; the M3 rejection
  test `reject_case_variable.cpp` is retired (pre-approved 2026-10-21); `reject_global.c` (a mutable global) stays.
  Facts marked (P*n*) are IL probes and (S*n*) Silicon spikes, recorded with evidence in `internal/il-inventory.md`,
  "M4.2b1"; code relies on them only once confirmed. Revised after the Opus audit (2026-10-21): weak constants and
  external-linkage tentative definitions are rejected (blocking items), #1901 is an error in C instead of being
  suppressed (an orchestrator choice, handoff §11 question 14), and a function's case values are also checked in its
  contract method.
- **Milestone:** M4.2b1 (2026-10-21).

## Context

M3 rejected every global, `const` and `constexpr` ones included, because a use is a folded constant whose `expr`
names the global and the global's initializer, which can narrow or shift, was never encoded (0012 item 12, 0010
item 12). It rejected a case value that names a variable (0014 item 6), and it checks a case value's obligations
before the dispatch, on the paths that reach the `switch` only: `if (0) switch (x) { case 1 << 40: … }` is never
checked, although an overflowing or out-of-range constant expression in a case label violates a constraint
(C23 6.6p4, 6.8.4.2p3; C++ [stmt.switch]/2 requires a converted constant expression). The M4.2 planning review found
a cross-mode trap: a `const` variable in a `do … while` condition could be a folded constant in one mode (so
`do … while (k)` with `k == 0` would be a `DoOnce`, 0014 item 5) and a variable in the other (a measured loop).

## Decision

1. **Named constants.** A *named constant* is a namespace-scope (file-scope) variable whose defining declaration is in
   a verified file (0011 item 4; EDG's `decl_position` is the defining declaration's, also after an earlier `extern`
   declaration, audit) that:
   - is `const` (or, in C++, `constexpr`); C23 `constexpr` objects do not parse in this EDG configuration (errors
     #65/#20, P23), so in C every named constant is `const`;
   - has a type the encoder accepts as a value (a builtin integer type or `bool`; not `volatile` or `_Atomic`, 0010
     item 2);
   - is not `thread_local`, not weak (`is_weak`, `is_weakref`: `__attribute__((weak))`, `[[gnu::weak]]`, and
     `#pragma weak`, which leaves no attribute entry; audit), so no other unit can replace its value;
   - carries only attributes on 0011 item 7's allow-list, like every other entity of a verified file (code audit,
     blocking: `static const int z __attribute__((loader_uninitialized));` leaves `z` without a value at run time);
   - and is constant-initialized: EDG records `initk_static` with its value in the variable's `initializer.constant`
     (P23: in both modes for `short`, `bool`, `unsigned`, `signed char`, `static const`, `extern const int d = 5;` (a
     definition), C++ `inline constexpr`, and `{}` initializers). In C, a file-scope `static const int z;` (a
     tentative definition with internal linkage) is zero-initialized (C23 6.7.11, 6.9.2) and is a named constant with
     value 0.
2. **Still rejected**, each with a diagnostic and a test (invariant 3), at the declaration
   (`unsupported_declaration`) unless noted:
   - a global that is not `const` (`reject_global.c`, unchanged): its value can change, so it needs the frames of
     M4.5b;
   - an `extern const` declared here and not defined in this unit (`initk_none`, `sc_extern`): its value is not known
     here;
   - a C tentative definition with external linkage (`const int z;`: `initk_none`, storage class unspecified, P23).
     Clang compiles C with `-fcommon` by default, so `z` is a common symbol and another unit's `const int z = 5;`
     silently wins at link time (audit, blocking): `= 0` or `static` makes it a named constant;
   - a weak constant (item 1; audit, blocking: EDG folds the weak read to the local value, the linked program may read
     another unit's); for the same reason a weak function in a verified file (`#pragma weak f` reached the encoder
     unrejected before; the attribute form was already an `unsupported_attribute`), since another unit's definition
     would replace the verified one. In C++ a `#pragma weak` that precedes every declaration of its name is ignored
     by clang++ ("weak identifier never declared"; the symbol stays strong) and by EDG alike, so that program means
     what cerastes verifies; after a declaration both make the name weak, and cerastes rejects it;
   - dynamic initialization (`initk_dynamic`): in C++ a `const int` whose initializer is not a constant expression is
     dynamically initialized, which covers `= f()`, `= 2147483647 + 1` (C23 folds it instead, item 8) and a constant
     read in its own initializer (`const int g = g + 1;`, which reads `g` before it is initialized; C refuses it, #28)
     (P23);
   - `thread_local`/`_Thread_local` constants (EDG's `is_thread_local`; `initk_static` otherwise, P23);
   - a `volatile` constant (`unsupported_type`, as for locals; EDG folds its uses in C but not in C++, P23);
   - an initializer whose original expression is outside the encoder's allow-list (C++ `const int zv = int();`, an
     `enk_temp_init`, P23) is rejected like any such expression (`unsupported_expression`);
   - a global declared in a system file, at its **use** in verified code (`unsupported_expression`): system files are
     used only through contracts (0011 item 4), and a global's value is not a contract;
   - `static const` locals stay rejected with the other static locals (M4.5b);
   - **in C, a constant-expression context that names an object.** A `const` object is not an integer constant
     expression in C (C23 6.6p8), and C23's `constexpr` is not available (item 1). EDG accepts such a name, with its
     discretionary warning #1901, in case labels, `_Static_assert`, and the initializer of a file-scope object
     (`const int B = A + 1;`), which GCC refuses in some of these and accepts as an extension in others (audit).
     Verify mode makes #1901 an error in C (`--diag_error=1901`; amends 0011 item 7), so the front end refuses every
     such context, `case k:` and `case N:` included, and the C programs cerastes accepts are portable. (The
     alternative, keeping the warning suppressed and rejecting each context in cerastes, is §11 question 14 of the
     handoff.) The encoder also rejects, in C, a case value and a named constant's initializer that name an object
     (`unsupported_expression`), as defence in depth (code audit: a `#pragma diag_suppress 1901` turned the error
     back into a warning; such pragmas are now rejected, next item);
   - **pragmas that change what the program means without leaving a trace in the IL** (re-audit, blocking):
     `#pragma clang attribute push (__attribute__((loader_uninitialized)), apply_to = variable(is_global))` gets only
     EDG's warning #161 and records nothing, while clang applies the attribute (and `weak`, `section`, … the same way,
     to globals and functions). Verify mode makes #161 (unrecognized `#pragma`) and #1675 (unrecognized GCC pragma)
     errors (`--diag_error=161,1675`; the SDK headers parse cleanly), so a pragma EDG does not understand refuses the
     file; and since cerastes now relies on these EDG errors and on #1901, EDG's own diagnostic pragmas
     (`#pragma diag_suppress`, `diag_remark`, `diag_warning`, `diag_error`, `diag_once`, `diag_default`, the
     `diagnostic` push/pop, and their `_Pragma` forms) are rejected in a verified file (`diagnostic_pragma_in_verify`),
     so nothing can turn those errors back into warnings (amends 0010 item 12's "no pragma rule is needed" and 0011
     item 7). EDG does not keep them in the IL (they are immediate pragmas, `automatically_include_in_il` FALSE), so
     in verify mode cerastes replaces, before parsing, the two entries of EDG's handler table `function_pointers[]`
     that the diagnostic pragmas dispatch to (`fn_diag_pragma`, `fn_diagnostic_pragma`) with wrappers that record the
     pragma's position and then call EDG's handler unchanged (no EDG source change; `cerastes-fe` is unaffected); the
     encoder rejects each recorded position in a verified file. Every route to the handler is seen — `#pragma`,
     `_Pragma`, a macro that produces one, inside a body — and a pragma produced by a macro is reported at the macro's
     definition, so one defined in a system header would not be seen: neither the pinned SDK nor libc++ contains an
     EDG diagnostic pragma (checked);
3. **A use of a named constant is EDG's value.** Wherever a named constant is read — a function body, `requires`,
   `ensures`, `check`, a loop invariant, `decreases`, a case value, another constant's initializer (C++) — the encoder
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
     an initializer (`static const int z;`, item 1) has neither.

   Every constant is checked whether or not anything uses it, and in every method: a file whose only content is
   constants has no method and is not accepted (0011: "nothing to verify", exit 2); the `__wf` method assumes only
   the parameters' type ranges, so the constants are checked even when a function's `requires` cannot hold.
   Obligations come before their guard, so a constant whose initializer fails (`const int A = 1 << 40;`, which EDG
   folds to 0) reports the obligation, not a cerastes bug. Silicon reports a failed assert and continues with its
   condition assumed (S14). Every obligation and guard of the prologue is **closed**: it names no variable, parameter,
   result or `old` value and contains no call, which `check_program` enforces for initializers and case values and
   lowering re-checks (an internal error otherwise); the prelude functions it can use (`c_pow2`, `c_wrap`, `c_shr`,
   `c_div`, `c_rem`, `c_udiv`, `c_urem`) have total, non-recursive bodies, so a closed assert is decided by evaluation.
   A failed one is therefore false on every path, and assuming it makes the rest of the method unreachable: no method
   goes on to use EDG's value of a constant whose initializer failed, and the same failure in several methods is one
   error after the driver removes identical errors (`report.cpp`). **Cost** (owner, accepted): a failing initializer
   hides every other error of the unit, since every method stops at it. Soundness does not rest on closedness (any
   reported failure means "not verified"); only the single report, the hiding, and exit 1 rather than 3 do.
5. **Case values are checked in the same prologue, path-independently.** After the constants, the prologue of a
   function's method — and of its `__wf` method, when it has one, so that an unsatisfiable `requires` cannot hide them
   (audit) — checks the function's local constants (item 6) and then each case value of its body (in source order,
   nested switches included): the value encoded by its original expression, its obligations asserted at the case
   label (with the note "in the value of this case label" when an obligation's position is on another line, as in
   0014 item 6), then an `internal` guard that the value as written equals EDG's `case_value` (P24). Each guard comes
   right after its own case's obligations (S15). The dispatch compares the scrutinee with EDG's value, which the guard
   proves equal to the value as written, and asserts nothing. Case values are closed, so they can be evaluated at
   entry. Consequences: `if (0) switch (x) { case 1 << 40: … }` is now a `shift` error, and so is a bad case value in a
   function whose `requires` cannot hold; the three twins `switch_case_expr_shift.c`, `switch_case_expr_conversion.c`
   and `switch_case_expr_over.c` keep their single errors (S15); like a constant, a failing case value hides the
   function's other errors. The bodies of `predicate` and `pure` functions are not encoded (0011 item 4), so their
   case values are not checked until M4.5f.
6. **`case k` and `case N` (C++).** In C++ a case value may name a named constant (`case N:`, `case N + 1:`) and a
   `const` or `constexpr` local whose initializer EDG folded (`const int k = 5; … case k:`, `constexpr int n = 3; …
   case n:`): its `case_value` is a constant of the promoted scrutinee type whose `expr` chain reaches the variable
   (P24). A local `const int k = 5;` is dynamically initialized in the IL (`initk_dynamic`), with its value in the
   declaration's `dik_constant` initializer (P23). Such a local is, inside case values only, a named constant of its
   function — transitively: a local constant named by another's initializer is one too — and its initializer is
   checked in that function's prologue (obligations, then the guard), before the case values; the case value uses
   EDG's value. In the body the local stays an ordinary local (item 3), assigned at its declaration, so both readings
   agree. `vir::Var` needs no `const` flag: no local reaches a case value. In C the same case values are refused by the
   front end (item 2).
7. **`do … while (c)` is a `DoOnce` only when `c` names no variable** (amends 0014 item 5). The exemption from the
   measure applies when EDG folds `c` to zero *and* `c`, following every constant's `expr`, names no variable or
   parameter (`enk_variable`, `enk_param_ref`). In this configuration EDG folds a `const` variable in a `do` condition
   in both modes (C as an extension; the folded constant keeps the variable in its `expr` chain, P24), so without this
   rule `do … while (k)` would escape the measure through EDG's extension; with it, it is a measured loop in both
   modes, and without a `decreases` it is rejected (`missing_decreases`) in both. The rule is about one meaning in
   both modes, not soundness (a `DoOnce` keeps its `internal` guard). `sizeof` of a variable does not count as naming
   it (it is an integer constant expression in both languages), and `0 && x`, which EDG folds while naming `x`, is now
   a measured loop. `do … while (0)`, `while (false)`, `while (1 - 1)` keep the exemption. (Enumerators are
   M4.2b2's.)
8. **Mode independence.** A named constant's initializer is checked as written with the C/C++ meaning of 0010, and
   its value is EDG's value of that initializer in the mode at hand. Most initializers have one value in both modes;
   those whose value differs between C and C++ — `sizeof('a')` and `sizeof(1 == 1)` are 4 in C23 and 1 in C++20
   (audit) — are kept out of dual-mode tests, as for locals. Where the front ends differ in what they accept, the
   program is not accepted in one mode and the test runs only in the other, with the reason in its header: C++
   makes an initializer that is not a constant expression a dynamic initialization (rejected), which C23 folds (so
   `const int A = 2147483647 + 1;` is an `overflow` error in C and rejected in C++); `static const int z;` is C only;
   `case k`, `case N`, `constexpr` and a constant named in another's initializer are C++ only (item 2).
   `const int A = 1 << 40;` is static in both modes (EDG folds it to 0 with warning #63) and a `shift` error in both.
9. **Self-test hook** (0011 item 13): `--cerastes_verify_selftest=perturb_folded` moves the value each `internal`
   guard compares with — every named constant's value (not its uses, which keep EDG's value) and every case value's
   EDG value — to a neighbouring value of its type (`bool` flipped), so the guards fail and the run ends with exit 3.
   A constant without an initializer has no guard and is left alone. Its harness self-tests show the guards fire.
10. **Facts relied on** (`internal/il-inventory.md`, "M4.2b1"): (P23) `init_kind` and `initializer.constant` per
    kind of definition and per mode, the shapes of uses in bodies, contracts and annotations, EDG's diagnostics, weak
    constants (`is_weak`); (P24) the `case_value` chain of a case naming a constant, the condition of `do … while`
    naming one, and #1901 on C case labels; (S14) Silicon continues after a failed assert with its condition assumed,
    so a failed closed assert ends its path while a failed assert over variables does not (three errors in `s14d`),
    and the same failing assert in two methods is reported once per method; (S15) the prologue form keeps the three
    case-value twins' single errors and verifies `switch_case_expr.c`, and a false guard placed after its own case's
    obligations is never reached when an obligation fails.
11. **Amends** 0010 items 12 and 13; 0011 items 7 (#1901 an error in C) and 13; 0012 items 4 and 12; 0014 items 5
    and 6; and `internal/encoding.md` §2, §3, §4, §5, §7 and §8.

## Consequences

- `const`/`constexpr` globals and (in C++) `case k`/`case N` verify; the rules are on the Integer semantics
  (constants), Verifying a program (named constants, switch, the single-unit assumption, what is not supported) and
  Annotations (constants in contracts; `do … while (0)`) pages.
- Every initializer of a named constant and every case value is checked as written, wherever it is, and the value
  EDG folded is trusted only after an `internal` guard has compared the two. (Static assertions and expressions
  inside types are still not encoded, 0010 item 13.)
- Constants are immutable, so 0012 item 4's argument (a callee cannot change the caller's state) still holds; mutable
  globals wait for M4.5b's frames.
- Several translation units (M6's contract manifest): a C++ internal-linkage constant named in a trusted contract may
  have another value in the unit that defines the function, and duplicate `inline constexpr` definitions are an ODR
  violation; the single-unit assumption on the Verifying a program page covers both.
- Emitted Viper changes only for functions with a switch whose case value is not a plain literal of the scrutinee's
  type (a generated conversion, as for an `unsigned` or `long` scrutinee, or an operation such as a negation): their
  case values move to the prologue and the dispatch compares with EDG's value. Every other existing test's `.vpr` is
  unchanged.

*2026-10-22, amended by 0018 (M4.2b2): item 7 — `names_object` does not look inside an enumerator (an enumerator names no object), so `do … while (A)` with an enumerator `A == 0` runs once in both modes; item 4 — enumerators with a written initializer are named constants of the unit, checked in the same prologue in declaration order.*
