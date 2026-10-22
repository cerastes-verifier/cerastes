# 0010 — Integer semantics for verification (M3)

- **Status:** accepted (owner decisions 2026-10-16 on narrowing and on annotation arithmetic). Facts marked
  (P*n*) are confirmed by the M3 IL probes (`internal/il-inventory.md`, "M3: scalar code") before the code
  relies on them; if a probe contradicts this record, the record is amended first (invariant 6).
- **Milestone:** M3 (2026-10-16)

## Context

plan.md M3 verifies non-template functions over the builtin integer types and `bool`, in C++20 and C23. It
asks for three decisions: signed overflow is a verification error, unsigned arithmetic is modular, and the rule
for narrowing conversions to signed types. The M1/M2 handoffs add one more: how constants that EDG folds are
checked (handoff §5 item 7).

The two languages differ in places. C++20 defines conversions to a signed type modulo 2^N and defines `<<` on
negative values (P1236); C23 leaves the first implementation-defined and the second undefined. Right shift of a
negative value is implementation-defined in C23 and arithmetic in C++20. cerastes uses one rule for both modes,
always the stricter or the target's, so that a `.c` test means the same in `c23` and `c++20`.

## Decision

1. **Values.** Every integer value is a mathematical integer in Viper (`Int`); `bool` is Viper `Bool`. A value
   of a type `T` is always within `[T_MIN, T_MAX]`: parameters and results carry that range as a checked
   precondition/postcondition, and every operation that produces a `T` is checked or wrapped into it.
2. **Types and ranges.** All builtin integer types: `bool`, plain `char`, `signed`/`unsigned char`, `short`,
   `int`, `long`, `long long` and their unsigned forms, `wchar_t`, `char8_t`, `char16_t`, `char32_t`,
   `__int128` and `unsigned __int128`. Sizes and signedness come from EDG's target configuration at run time
   (P12, confirmed: each type's `size` and `int_kind`, and `plain_chars_are_signed` in the IL header), never from
   constants in cerastes: plain `char` is signed on arm64 macOS because EDG says so. Constants are read exactly
   with `decimal_str_for_integer_constant` (P9, confirmed for 128-bit values). Rejected in M3, in both modes,
   and listed for M8 (M4.2 plans enumerations): enumeration types **and enumeration constants** (in C an enumerator has type `int`, but
   accepting it in `c23` and not in `c++20` would give a `.c` test two meanings), `_BitInt(N)`, floating
   point, vector types (and the other new type kinds of EDG bug 0007's fix), and any type kind not on the
   allow-list.
3. **Signed arithmetic** (`+ - *`, unary `-`, `/`, `%`, `++`, `--`, compound assignment) whose mathematical
   result does not fit the operation's type is a verification error (`overflow`), at the operator.
4. **Unsigned arithmetic** wraps: the result is the mathematical result modulo 2^N.
5. **Conversions.**
   - To an unsigned type: modulo 2^N (both standards define it).
   - To a signed type: a value that does not fit is a verification error (`conversion`), **implicit or
     explicit** (owner, 2026-10-16). This includes `(short)x`, `static_cast<short>(x)`, initialization,
     assignment, compound assignment (`c += 1` on `signed char` converts the `int` result back), `++`/`--` on
     narrow types, arguments and return values. It is VeriFast's default too (its truncating casts need an
     explicit marker). A future opt-in annotation for wrapping casts is possible; none exists in M3.
   - To `bool`: `v != 0`. From `bool`: 0 or 1.
   - A conversion whose source range lies within the target range needs no check.
6. **Integer promotions and the usual arithmetic conversions.**
   - For binary, unary, comparison and call-argument operands, EDG makes them explicit in the IL as
     compiler-generated conversion nodes (P1, confirmed). The encoder encodes every conversion node and does not
     add its own there.
   - **Compound assignment, `++`/`--` and shift-assignment** are single IL nodes whose type is the left
     operand's type; the left operand is not converted and the right operand is raw or only promoted (P2 and the
     M3-D audit). Their computation type is not in the IL, so the encoder computes it by the standards' rules,
     from EDG's type sizes: for `x op= e` (`op` arithmetic), the usual arithmetic conversions of the promoted
     type of `x` and the type of `e`; for `x <<= n` and `x >>= n`, the promoted type of `x` alone (the operands
     promote separately); for `++x`, `x++`, `--x`, `x--`, the usual arithmetic conversions of the promoted type
     of `x` and `int`. The result is then converted back to the type of `x` (a checked conversion, item 5). This
     is the one place the encoder implements promotions; it is reviewed and tested on its own (`us *= us`
     overflowing `int`, `c <<= 1` on `signed char`, `i <<= n` with a `long` `n`, `i += u` converting back to
     `int`, `uc *= uc`).
   - **C's truth values** (P5, confirmed). In C23 the condition of `if`, `while`, `for` and `?:`, the
     operands of `!`, `&&` and `||`, and the arguments of `requires`, `ensures`, `check` and `invariant` (M3-D
     audit) are plain integers with no conversion node; C++ wraps them in a compiler-generated `bool`
     conversion. In C the encoder therefore reads a scalar in those positions as
     `e != 0`, and gives comparisons and logical operators their C type, `int`, with the value 0 or 1. This is a
     rule of C, not an inserted conversion, and VIR's typing rules state it.
   - Everywhere else, the VIR type checker rejects any operation whose operand types do not equal what the
     operation requires, so a conversion left implicit fails closed (`unsupported_expression`) instead of
     computing in the wrong type.
   - "Signed" and "unsigned" in items 3, 4, 7 and 8 mean the type the operation is computed in, after
     promotion: `uc << n` is a signed `int` shift.
   - Consequences that are faithful, not bugs, and documented on the Integer semantics page: `us * us` on
     `unsigned short` is computed in `int` and can overflow; `uc << 24` can reach the sign bit of `int`;
     `-1 < 0u` is false; `-u` on an `unsigned` wraps.
7. **Division and remainder** truncate toward zero (both standards). A zero divisor is an error
   (`division-by-zero`). `T_MIN / -1` and `T_MIN % -1` are overflow errors for signed `T` (C23 and C++
   [expr.mul]/4 both make them undefined when the quotient is not representable).
8. **Shifts** (`<<`, `>>`, `<<=`, `>>=`):
   - the shift amount must be in `[0, w)` where `w` is the width of the promoted left operand (`shift`);
   - left shift of a signed value: an error if the value is negative or the result `a * 2^n` does not fit
     (`shift`). C++20 would define both; cerastes uses C23's stricter rule in both modes;
   - left shift of an unsigned value wraps;
   - right shift of a negative value is arithmetic (floor division by 2^n): defined in C++20 and this target's
     implementation-defined behaviour in C23 (the Integer semantics page says so).
9. **Bitwise operators** `&`, `|`, `^` and `~` are rejected in M3 (`unsupported_expression`), and planned for M4.4.
10. **Comparisons and logical operators** have their mathematical meaning on the converted operands. In C, their
    result is an `int` 0 or 1; in C++, a `bool` (P5, confirmed; item 6 for C's truth values). `&&`, `||` and
    `?:` evaluate their operands conditionally, and the checks of a conditionally evaluated operand are
    conditional too (0012).
    **Unary minus** on an unsigned type wraps (`-u` is `2^N - u` for `u != 0`); unary plus is the identity.
11. **Arithmetic inside annotations has its C/C++ meaning** (owner, 2026-10-16). An annotation is a C/C++
    expression, typed by EDG; it means what that expression computes. Unsigned arithmetic wraps there too.
    Signed overflow, division by zero, a bad shift or a narrowing to a signed type inside an annotation is a
    verification error at the annotation's line, with the same category as in code. These obligations are
    checked without any `assume`: in a generated method whose preconditions are only the parameters' type
    ranges, each clause's obligations are asserted under the clauses before it (a `requires` clause may rely on
    the earlier `requires`; an `ensures` clause on all `requires` and the earlier `ensures`). A user who wants
    unbounded arithmetic in a contract widens the operands (`(long)a + b`).
12. **Folded constants are checked by encoding what was written** (P3, confirmed; M3-D audit).
    - EDG does not fold the operations it would diagnose: `INT_MAX + 1`, `1 / 0`, `1 % 0`, `1 << -1` and the
      like stay operations in the IL, which the encoder checks like any other.
    - What it does fold keeps its original expression in `a_constant.expr` in every case probed (`1 << 40`,
      `signed char c = 300`, `(short)70000`, `unsigned u = -1`, `return 300` from a `signed char` function,
      argument and default-argument constants). The encoder encodes that expression instead of the value,
      **recursively** (a nested constant has its own `expr`), so items 3–8 apply as written.
    - Kinds of `expr` and how they are used: operations and conversions are encoded; `sizeof`/`alignof` give
      their value (no run-time operation); a reference to a local `const` variable is that local; a reference
      to a global, an enumerator, or a call (a `constexpr` function evaluated at compile time) is rejected
      (`unsupported_expression`), since its initializer or body is not encoded here.
    - A constant with a recorded `orig_type` and no `expr` (il_def.h ~:4782: "for some implicitly-converted
      constants") never occurred in a function body in the probes; if one reaches the encoder in a verified
      body, it is rejected (fail closed; a self-test hook reaches this path).
    - So soundness does not depend on EDG's diagnostics. Verify mode **suppresses** the front-end warnings the
      verifier reports itself, with a category and a line (`#61` integer overflow, `#62`/`#63` shift counts,
      `#39`/`#179` division and remainder by zero, `#68`/`#69` sign change and truncation, `#940` missing
      return), so that a test or a user sees each problem once (0011 item 7 lists the other verify-mode
      suppression, `#2803`). They are not promoted to errors: `#68`/`#69`
      also fire for conversions to unsigned types, which item 5 defines, and `#61` fires falsely on some
      `__int128` arithmetic. A `#pragma diag_*` in the source therefore cannot hide a problem from the
      verifier; at most it turns a warning into an error, which only rejects the program (fails closed). No
      pragma rule is needed.
13. **Constant expressions the language requires** (array bounds, `static_assert`, case labels, template
    arguments) are checked by EDG as the standard requires (an overflowing constant expression is ill-formed
    there). Expressions embedded in types are not walked by the encoder; types with such expressions (VLAs in C)
    are rejected in M3.
14. **Shift amounts and `c_pow2`.** The prelude's `c_pow2(n)` is at least 1 for every `n` (so `c_shr` is
    well-formed for any argument); its value matters only where the `shift` obligation already holds.

## Consequences

- One meaning per expression in both modes. Code that relies on C++20's modular conversion to a signed type, or
  on its defined `<<` of negative values, fails verification; the Integer semantics page explains how to write
  it (`static_cast` to unsigned, then compare).
- Contracts with signed arithmetic need bounds or wider types, as in code.
- Every check is an `assert` in the emitted Viper with a position; nothing is inhaled or assumed (invariant 4).
- The rejections in items 2 and 9 are entries of the M8 inventory until M4 lifts them.

*2026-10-20: milestone numbers follow the renumbering of 2026-10-20 (plan.md: old M4 → M5, M5 → M6, M6 → M4.5 for C and M7 for C++, M7 → M8).*

*2026-10-20, amended by 0014 (M4.1, audit): item 12 — EDG does fold some operations it diagnoses: in C23 `2147483647 + 1` and `1 << 40` become constants with only warnings #61/#63, which verify mode suppresses; every probed case keeps the original `expr`, and soundness rests on encoding it, not on EDG's diagnostics. Verify mode also suppresses #546 (a jump past an initialization), which cerastes rejects itself. Item 13 — case labels are not left to EDG: a case value is encoded by its original expression and its obligations are checked (0014 item 6).*

*2026-10-21, amended by 0017 (M4.2b1): item 12 — a reference to a named constant (a `const`/`constexpr` global defined in a verified file and constant-initialized) is EDG's value of its initializer, and that initializer is encoded by its original expression and checked, with an `internal` guard against EDG's value, in the prologue of every method; contracts may name such constants; a local `const` named by a case value (C++) is checked the same way in its function's prologue; globals that are not named constants, and globals from system files, stay rejected. Item 12's "no pragma rule is needed" no longer holds: verify mode relies on front-end errors (#1901 in C, #161, #1675), so EDG's diagnostic pragmas are rejected in verified files (0017 item 2). Item 13 — case values are checked in that prologue, so whether or not the switch is reached.*

*2026-10-22, amended by 0018 (M4.2b2): item 2 — enumeration types and enumerators defined at file scope in a verified file are accepted in both modes; each enumeration is its own type whose values are its value range ([dcl.enum]/8 for one without a fixed type, in C too; the underlying type's range for a fixed one), and an enumerator is the front end's value; an enumeration with a `bool` underlying type, an enumerator outside `int`, and enumerations in blocks, classes, templates and system files stay rejected. Item 5 — a conversion into an enumeration, implicit or explicit, is a `conversion` error unless the value is in the value range, also for an unsigned underlying type (never a wrap); an implicit conversion out of an enumeration is accepted only where it means the same in C and C++ (`==`, `!=`, a switch, a whole initializer, assignment, argument or return, an enumerator operand), and rejected elsewhere.*
