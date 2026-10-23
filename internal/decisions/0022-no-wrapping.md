# 0022 — No wrapping: unsigned arithmetic and conversions are checked (M4.4a)

- **Status:** accepted for M4.4a. Owner decisions (2026-10-23, M4.4 planning and during the session; handoff §11
  questions 13 and 29, §8.1):
  - **"Wrapping should be prohibited."** Integer semantics model real integers, not how C and C++ allow wrapping
    (question 13). The existing categories are reused (`overflow`, `conversion`, `shift`).
  - **No modular opt-in.** A modular result is written as a reduction in a wider type (item 7). A truncating marker is
    an M8 entry (`plan.md` M8, "Known entries").
  - **Enumerations compared with a signed integer** (during the session). In C both compilers warn `-Wsign-compare` on
    `c == x` for an enumeration whose C type is `unsigned int`, so it is **rejected in both modes**, like the compilers
    (item 5). The planning's alternative, a mathematical comparison, was dropped.
  - **Expectation changes, pre-approved by rule.** The rule covers every test, and every VIR self-test case, that flips
    only because of a new `overflow`, `conversion` or `shift` error at a wrapping site:
    - where the test's whole point was the wrap, it is retired and replaced: a positive in range, and an error twin at
      the old line;
    - where it is an error twin, or shares a wrapping fixture, the wrap is removed in its body in place (never an
      `expect:` line), and a new twin pins the wrap.

    Every file is listed in the status entry. More than ten flips outside the known list (handoff §8.1) is a sizing stop.
    A flip of any other kind stops the work and goes to the owner, for example:
    - a new `unsequenced_calls` (item 10), or another category;
    - a second error next to the new one;
    - exit 3;
    - an error twin whose expected error is now hidden behind an earlier failing obligation.

    The owner's item 5 rule retires `tests/scalar/enum_cast_eq.c` (`x == (enum color)2`); it is listed with the others.
  - **Question 29:** lowering's internal errors are tool failures, exit 3 (item 12).

  Facts marked (P*n*) are probes, recorded with their evidence in `internal/il-inventory.md`, "M4.4a" (P38, P39).
  Audit round 1 (Opus) found no blocking item. Its should-fix items are folded into items 1, 3, 4, 5, 9, 10 and 12,
  and into the amendments.
- **Milestone:** M4.4a (2026-10-23).

## Context

0010 makes unsigned arithmetic and conversions to unsigned types modular (items 4, 5, 8, 10), as both standards
define them. Signed overflow, narrowing to a signed type and conversions into an enumeration (0018 item 5) are already
errors. The owner's rule (handoff §11 question 13, and the memory "integer semantics: real integers") is that a value
that does not fit is an error everywhere, since a silent wrap is almost always a bug in the programs cerastes is meant
for, and VeriFast checks unsigned arithmetic too (its truncating operations need an explicit marker).

Lowering emits a wrap in four places today (`src/vir/lower.cpp`), and the prelude's `c_wrap` is their only function:
unary minus, line 489; `+ - *`, line 542; `<<`, line 584; and conversions to an unsigned type, line 611.
Compound assignment, `++` and `--` reach them through `Builder::compound_value` (`vir.cpp`), which computes in the
type 0010 item 6 gives and converts the result back.

## Decision

1. **The rule.** Every integer operation the encoder encodes has the mathematical result of 0010 item 1. If that
   result does not fit the type the operation is computed in, it is a verification error, for unsigned types exactly as
   for signed types. That type is the one after promotion and the usual arithmetic conversions (0010 item 6):
   - **Unsigned `+`, `-` and `*`:** `overflow`, "unsigned `T` addition may overflow (cerastes does not wrap)", and the
     same for subtraction and multiplication.
   - **Unary minus on an unsigned value:** `overflow` unless the value is 0, "negating a nonzero `T` value overflows
     (cerastes does not wrap)".
   - **`u << n`** whose result `u · 2ⁿ` does not fit: `shift`, "the result of `<<` may not fit in `T`". A shift count
     outside `[0, w)` is `shift`, as before.
   - **A conversion into an unsigned type** whose value does not fit: `conversion`, "the value may not fit in `T` (a
     conversion must be exact)". It applies to every conversion, implicit or explicit:
     - `(unsigned)x`, initialization and assignment;
     - arguments and return values;
     - the usual arithmetic conversions (`-1 < 0u`, `i + u` with `i < 0`, the `-1` of `u += -1`);
     - case values converted to the promoted type of the controlling expression (`case -1:` over `unsigned` in C;
       C++ refuses it, EDG #2362);
     - named-constant and enumerator initializers as written.
   - **Compound assignment, `++` and `--`.** They compute in the type 0010 item 6 gives and convert the result back,
     so the error is the one that type gives:
     - `u += 1`, `u++` and `u--` on `unsigned int` or wider: `overflow` at the operator;
     - `u += 1L` on a 32-bit `unsigned` computes in `long` and converts back: `conversion`;
     - `uc++` and `uc--` on `unsigned char` and `unsigned short` compute in `int`, where they fit, and convert back:
       `conversion` at 255, as `sc++` at 127 already is;
     - `us *= us` can overflow `int` (0010 item 6), and `uc -= 1u` computes in `unsigned int`: `overflow`.

   `/`, `%` and `>>` on unsigned values, and conversions to and from `bool`, cannot leave the range and are unchanged.
   The signed rules (0010 items 3, 5, 7, 8) are unchanged.
2. **Where.** Everywhere the encoder encodes integer operations:
   - function bodies;
   - contracts and annotations: the obligations of a clause are checked in its `__wf` method under the earlier clauses
     (0010 item 11), also for a function without a body, so a trusted `ensures(r == a + b)` gets its check;
   - named-constant initializers and case values (0017's prologue);
   - enumerator initializers (0018);
   - default arguments, which are checked at each call.

   Some values the front end computes alone, and it still wraps there:
   - `#if`, `_Static_assert`, array bounds and other expressions inside types, and template arguments
     (`_Static_assert(65536u * 65536u == 0u)` is accepted);
   - character literals (`'\xff'` is -1, `L'\xffffffff'` an `int` -1).

   These values are C's own (0010 item 13), so they are not unsound, but they are outside the real-integer model.
   Encoding closed `_Static_assert` conditions in the prologue is a handoff note.
3. **Folded constants** (0010 item 12, P38). EDG folds wrapping unsigned constants and gives no diagnostic for most
   of them; only #68, #69 and #63 fire, on a few shapes. The encoder is therefore the only check, and it encodes the
   original expression EDG keeps in `a_constant.expr`, so item 1 applies to what was written.
   - **Where `expr` is kept.** P38 found the whole expression tree, in both modes, everywhere the encoder reads a
     constant: function bodies, named constants, enumerator initializers, case values, default arguments and locals.
   - **Annotations.** EDG leaves constant expressions in annotations largely unfolded. Where it folds one (`check`,
     `invariant`, `requires`, `ensures`), the encoder's `constant_without_expr` hook rejects exactly the lines that
     hold folded constants, so every such constant has an `expr` (audit round 1).
   - **Where the encoder takes EDG's value.** Each place is covered:
     - a literal without an `expr` (`encode.cpp` `constant_value`) is the number written;
     - a constant with an `orig_type` and no `expr` is rejected (0010 item 12);
     - a use of a named constant or an enumerator is EDG's value of an initializer that is checked as written in
       every method's closed prologue (0017, 0018);
     - `sizeof`/`alignof` give their value (0010 item 12);
     - a `Case::folded` value is guarded in the prologue (0017);
     - an enumerator copy is checked against its enumeration's range (0018).

     A wrapping constant never reaches the two places where the encoder trusts EDG's value without an expression
     (`constant_value`'s literal arm, and an enumerator without an initializer).
   - **Where it is lost.** Two classes, neither reachable today:
     - non-type template arguments (`BACKING_EXPR_FOR_NONTYPE_TEMPL_ARG` is false). Templates are rejected, and there
       is a note for M5 (handoff §8.2);
     - elements of braced initializers longer than 1000 entries (`REDUCE_BACKING_EXPRESSION_USE`), which carry an
       `orig_type` and no `expr`. Arrays are rejected, and there is a note for M4.5e (handoff §8.1): read the elements
       through `constant_value`, which rejects that shape.
   - **Defence in depth.** The arm of `enum_case_value` for a constant without `expr` now rejects one with an
     `orig_type` before delegating to `constant_value`, which already rejects it, so the check is redundant.
4. **A constant that fits needs no check.** A conversion into an unsigned integer type that is not an enumeration,
   whose operand is an integer constant (a VIR `Const`) whose value lies in the target's range, emits no obligation.
   This avoids a trivial assert for the `1` of `u++` and `u += 1`. It is sound whatever the constant's provenance: it
   only drops `lo <= v <= hi` for a `v` already in range. A wrapped value of EDG's inside such a `Const` (a
   named-constant or enumerator use) comes from an initializer the prologue checks, and a failed closed obligation there
   ends the path before any use (S14, S15). The refinement applies only to conversions into unsigned types, which had no
   obligation before, so the emitted Viper of signed code is unchanged.
5. **An enumeration compared with a signed integer is rejected** (owner, 2026-10-23, P39; amends 0018 item 6).
   - **Why.** In C, an enumeration without negative values has the compatible type `unsigned int` (P26), so `c == x`
     converts a signed `x` to `unsigned int`. Under item 1 a negative `x` would be a `conversion` error in C only, since
     C++ promotes `c` to `int`.
   - **What the compilers do (P39).** clang and GCC, in C with `-Wall -Wextra`, warn `-Wsign-compare` on every shape
     where the signed operand may be negative. That includes a variable of type `int`, `short`, `signed char` or
     `char`, a negative constant or named constant, `(int)u`, `x + 1`, `-x`, and either operand order. An
     enumeration with a fixed `: unsigned` base is warned in all four configurations. No compiler warns on:
     - a literal ≥ 0 (`c == 1`); GCC in C does warn on a named constant ≥ 0 (`const int K = 1`), which cerastes accepts;
     - a `bool`, `unsigned char` or `long` operand;
     - an enumeration with negative values;
     - two values of one enumeration, `(int)c == x`, or an enumerator of an enumeration without a fixed unsigned base
       (`GREEN == x`);
     - `x == (enum color)2`, or a `?:` of constants ≥ 0.
   - **The rule.** An `==` or `!=` is rejected (`unsupported_expression`, at the operator) when all of these hold:
     - one operand is a value of an enumeration E, through the front end's implicit conversions, and not an
       enumerator; the other operand is not an enumeration value;
     - E's values are compared in an unsigned type in C: E has no negative value, and C's promoted type of E is an
       unsigned type of rank `int` or above. That is an unfixed enumeration without negative enumerators, or a fixed
       base `unsigned`, `unsigned long`, `unsigned long long` or `unsigned __int128`; a fixed `unsigned char`,
       `unsigned short` or `bool` base promotes to `int`;
     - the other operand's own type is a signed integer type, not an enumeration and not `bool`. The encoder looks
       through the compiler-generated casts EDG puts on it in either mode, and through a folded constant's `expr`;
     - the other operand is not an integer constant ≥ 0.

     The message is "an equality between a value of `enum color` and a signed integer that may be negative: C compares
     it as `unsigned int` (the compilers warn, -Wsign-compare); convert one side explicitly, for example `(int)c == x`".
     It applies in contracts and annotations too, where the native compilers see nothing but C's comparison would still
     be unsigned.
   - **A truth value counts as non-negative.** A relational, equality, `!`, `&&` or `||` operand has the value 0 or 1,
     so it is not "a signed operand that may be negative", although C types it `int` (audit round 2): `c == (x > 0)` is
     accepted in both modes, and the compilers are silent on it. An explicit `(int)b` of a `bool` is not a truth value: clang warns on
     `c == (int)b`, so it is rejected (the strictest reading).
   - **Two different enumerations.** `==`/`!=` between values, or enumerators, of two different enumeration types is
     rejected in both modes (audit round 2, blocking).
     - The compilers warn on it: GCC in C `-Wenum-compare` (and both `-Wsign-compare` when one is compared in
       `unsigned int`), `-Wdeprecated-enum-compare` in clang++, and `-Wenum-compare` in g++. Clang in C is silent for
       anonymous enumerations, so the rejection is the strictest reading.
     - Without the rejection, `c == n`, with `enum neg { NA = -1, NB }`, would be a `conversion` error in C and verify
       in C++.
     - In C an enumerator is an `int` constant. Its enumeration is still identified by the front end's enumerator mark
       (0018), so `c == NB` is rejected in both modes too.

     Two values of the same enumeration stay accepted.
   - **Stricter than the compilers.** `x == (enum color)2` and `c == (x > 0 ? 1 : 2)` are rejected too. In C, the
     first would otherwise be a `conversion` error for a negative `x` in C only, and the second keeps the rule simple.
   - **Unchanged.** A constant ≥ 0 converts exactly (item 4), so `c == 1` and `c == RED` mean the same in both modes.
     `bool`, `unsigned char` and `long` operands, two values of one enumeration, and an enumeration with negative values
     against an `int` convert no negative value to an unsigned type. Ordering against another integer stays rejected
     (0019 item 10). An enumerator of an enumeration with a fixed unsigned base against an `int` (`FA == x`) is the
     general rule of item 1: a `conversion` error for a negative `x` in both modes.
   - **Verdicts in the two modes** (re-audit 2b). No shape gives opposite meanings, such as an error in one mode and a
     verification in the other. A few shapes are rejected in one mode and accepted, with the same values, in the other;
     that is sound, and they are listed for M4.4b (handoff §11 question 36):
     - C rejects and C++ accepts: a `?:` or comma whose result is a truth value, which C types `int` and C++ `bool`
       (`c == (x ? (x > 0) : (x < 0))`, `c == (x ? true : false)`, `c == (b ? b : b)`, `c == (x++, x > 0)`);
     - C accepts and C++ rejects: a `?:` of enumerators, which C types `int` and C++ types the enumeration
       (`(x ? RED : GREEN) == y`, `c == (1 ? NB : NC)`).
6. **Unchanged suppressions** (amends 0010 item 12's reason). Verify mode still suppresses EDG's #68/#69 (sign change,
   truncation): cerastes now reports every such conversion itself, with a category and a line, so a user sees each
   problem once. Those warnings also do not fire on every shape (`0u - 1u`), and they fire in dead code.
7. **Modular arithmetic is written explicitly.** There is no wrapping cast and no opt-in. A program that wants a
   result modulo 2ᴺ writes the reduction in a type wide enough for the exact result:
   - `(unsigned char)(x % 256)` for a non-negative `x`;
   - `(unsigned)(((unsigned long long)h * k) % 4294967296ull)` for a 32-bit hash step;
   - masks from M4.4b (record 0023), such as `(unsigned char)(x & 0xFF)`.

   A VeriFast-style truncating marker is an M8 entry.
8. **The prelude** loses `c_wrap` (amends 0012 item 11 and the prelude list of 0017 item 4). Every remaining function
   has a body, no `requires` and no `ensures`.
9. **Messages.** Every new error names its type. The `overflow` messages of unsigned arithmetic say that cerastes does
   not wrap; the `shift` and `conversion` messages say what must fit. The error-messages page explains each one in
   source terms, and the Integer semantics page shows the rewrite that verifies (item 7).
10. **Consequences that are intended.**
    - **Errors now.** `unsigned u = -1;`, `-1 < 0u` and `u - v` with `u < v` are errors. So is the last decrement of
      `for (unsigned i = n; i-- > 0;)`: write `for (unsigned i = n; i > 0; i--)` and use `i - 1` instead.
    - **Modular code.** A hash or checksum written for modular arithmetic must be rewritten in a wider type (item 7).
    - **Hidden errors.** A failing `const unsigned M = -1;` ends every method's prologue and hides the unit's other
      errors, as any failing named-constant initializer does (0017).
    - **Calls.** Parameters of unsigned types now receive checked conversions. `g(f(x), h(y))`, where `g` takes
      `unsigned` and `f`, `h` return `int`, has two obligations that depend on different calls, so it is
      `unsequenced_calls` (0021 item 3), as the same shape with `short` parameters already is. New obligations in the
      arguments of a recursive call also enter 0021's family F. A test that flips either way is outside the
      pre-approval: the session stops and asks.
    - **No new `check_program` rejections.** The VIR is the same except for item 5's rejection, and `eval_const` already
      treats an inexact conversion as unknown. The only obligation-driven rejection is `unsequenced_calls`.
11. **Tests.**
    - The flips are handled by the rule in the status above.
    - New pins, in both modes and `native-ok` (positives avoid `-Wsign-compare` shapes):
      - each rule of item 1, with an error twin at its line;
      - exit 1, never 3, for `const unsigned K = -1;`, for an enumerator whose initializer has a wrapping
        subexpression (`(0u - 1u) > 5u`, whose value fits), and for C's `case -1:` over `unsigned`. A failed assert in a
        closed prologue ends the path (S14, S15), so the prologue's internal guard cannot fire;
      - item 4's constant, item 7's explicit reduction, and item 5's rejected and accepted shapes.
    - The VIR self-test gains a case per rule, each with a twin. Its retired cases follow the same rule.
    - Entries of `tests/mutants-equivalent.txt` belonging to retired files are dropped. The entries argued under modular
      semantics are re-reviewed with the session's mutation run.
12. **Exit 3 for internal inconsistencies** (question 29; amends 0011 items 8 and 13). These three are tool failures:
    lowering's internal errors (`LowerResult::errors`), the emitter's problems, and the encoder's "encoded twice"
    guard. They are reported with their own diagnostic, `internal_inconsistency` ("an internal inconsistency: … (the
    program was not checked)"), and the driver exits 3, never 2.
    - The back end sets `VerifyOutput::internal_inconsistency`, and `verify_main.cpp` tests it before its front-end
      status (`front_end_exit_status`).
    - `check_program`'s problems stay rejections (exit 2), since a program can reach some of them, and so do the
      encoder's own type-mismatch rejections. Moving 2 to 3 cannot make a test pass: the harness fails every exit 3.
    - A new self-test hook, `perturb_fixed_order`, perturbs lowering's call order so its fail-closed check fires. Its
      harness test checks lowering's own message, the driver's line and exit 3, with a twin without the hook that
      verifies. A hooked run exits 3 even when the hook changes nothing (0011 item 13), so exit 3 alone would prove
      nothing.
    - `cerastes --selftest` checks the report path.

## Amends

- **0010:**
  - Item 1: "checked or wrapped into it" becomes "checked".
  - Items 4, 5, 8, 10 (unary minus) and 11 now follow item 1. Item 5's "future opt-in annotation" is item 7 here: no
    opt-in.
  - Item 6's "faithful consequences" `-1 < 0u` and `-u` become errors; `us * us` and `uc << 24` are `int` rules and
    stand.
  - Item 12's reason for the suppressions is item 6 here.
  - The Consequences' advice "`static_cast` to unsigned, then compare" becomes item 7's reduction.
- **0011:** item 8, item 10's category table (`overflow`: arithmetic may overflow, signed or unsigned; `conversion`: a
  conversion may not fit), and item 13 (item 12 here).
- **0012:** item 11 (the prelude).
- **0014:** item 6: `case -1` over an `unsigned int` scrutinee is a `conversion` error, not `4294967295`.
- **0017:** item 4's prelude list, and its wraps: an initializer that wraps to an unsigned type is a `conversion` or
  `overflow` error in the prologue.
- **0018:** item 5: a checked conversion into an enumeration is the general rule now, not a special case. Item 6, "why
  these agree": (i) holds only without a signed operand that may be negative, which item 5 here rejects.
- **0020:** item 2: the value of `x = e` is `e` converted with a check, for unsigned types too.
- **0021:** item 3, "where it is decided": an unsigned operation creates a check unless item 4 applies; item 10 here.
- **`internal/encoding.md`:** §1 (the pipeline), §3 (the lowering table and the prelude), §4 (the Viper shape), §7 (item
  5's rule) and §8 (exit 3 and the hooks); **`tests/README.md`**: the category and exit-status tables.

## Consequences

- One meaning in both modes, now without any modular arithmetic. A `.c` test's verdict is the same in `c23` and `c++20`,
  and an unsigned value computed by an operation cerastes encodes is the mathematical value of that operation.
- Code that relies on wrapping fails verification at the wrapping operation, and the Integer semantics page shows the
  rewrite.
- The emitted Viper of programs without unsigned wrap sites is unchanged. Programs that had `c_wrap` lose the prelude
  function, and each of their former wrap sites becomes an assert, except where item 4 applies.
