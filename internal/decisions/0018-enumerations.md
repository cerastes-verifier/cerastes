# 0018 — Enumerations (M4.2b2)

- **Status:** accepted for M4.2b2. Designed in the M4.2b planning (2026-10-21, handoff §8.1) with the owner's answers:
  an out-of-range conversion to an enumeration with a fixed unsigned underlying type is a `conversion` error like every
  conversion into an enumeration, never a wrap (owner, 2026-10-21; unsigned wrapping in general is to be relooked,
  handoff §11 question 13); the M3 rejection test `reject_enum.c` is retired (pre-approved 2026-10-21); C's strict
  rule for `const` objects in constant expressions stays (owner, 2026-10-22, handoff §11 question 14), so in C an
  enumerator initializer that names an object is refused; the front end's unflagged copies of an enumerator's value are
  read as literals of that value (owner, 2026-10-22, item 9). Facts marked (P*n*) are IL probes, recorded with evidence
  in `internal/il-inventory.md`, "M4.2b2" (P25–P27, all confirmed).
- **Milestone:** M4.2b2 (2026-10-22).

## Context

M3 rejected every enumeration type and every enumerator, in both modes (0010 item 2, 0012 item 12): in C an enumerator
has type `int` while in C++ it has the enumeration's type, so accepting enumerators in `c23` and not in `c++20` would
give a `.c` test two meanings. M4.2b1 made named constants verifiable (0017) and left enumerators to this record (0017
item 7). The two languages differ in three ways that matter here:

- **The values of an enumeration.** In C++ an enumeration whose underlying type is not fixed has the values of the
  smallest bit-field that holds all its enumerators ([dcl.enum]/8); converting a value outside that range into it is
  undefined behaviour ([expr.static.cast]/10). In C an enumeration is compatible with an implementation-defined integer
  type (C23 6.7.3.3; here `unsigned int` without a negative enumerator, `int` with one, P25), and any value of that
  type may be stored.
- **The type of an enumerator.** C: `int` (for an enumeration without a fixed type: a type entry of its own with
  `affiliated_type`, il_def.h:10161), the enumeration itself for a fixed one; C++: the enumeration (P25).
- **Promotion.** In arithmetic, C promotes an enumeration value to its compatible type (`unsigned int`, so `c - 1` with
  `c == 0` is `UINT_MAX`), C++ to `int` for an unfixed enumeration whose values fit (`c - 1` is `-1`) (P26).

## Decision

1. **Scope.** Enumeration types defined at file scope in a verified file, and their enumerators, in both modes:
   unscoped enumerations (C and C++), C++ scoped enumerations (`enum class`), fixed underlying types (both modes: the
   front end parses C23's, P25), unnamed enumerations and C++ empty enumerations. Enumeration types are used as the
   types of parameters, results, locals, named constants (0017), `old` and `decreases` expressions.
2. **Still rejected**, each with a diagnostic and a test (invariant 3), at the declaration (`unsupported_declaration`)
   unless noted:
   - an enumeration with a `bool` underlying type (its values are not integers in the sense of item 3);
   - an enumerator whose value is outside `int` (C++ gives the enumeration a wider underlying type; C23 a type of its
     own; the front end says nothing, P27): keeping every value of an enumeration without a fixed type inside `int` is
     what item 6 rests on. Reported at the enumerator, as is an attribute on an enumerator;
   - an incomplete enumeration without a fixed underlying type (no enumerator list, so no value range);
   - an enumeration declared in a block (as every type declaration in a body, 0012 item 12), in a class or in a
     template (`is_template_enum`, `is_prototype_instantiation`, `is_nonreal`);
   - an enumeration declared in a system file, at its use (`unsupported_type` for its type, `unsupported_expression`
     for one of its enumerators; an M8 entry): system files are used only through contracts (0011 item 4), and an
     enumeration's values are not a contract (every C++ unit holds the library's `std::align_val_t`, P25, which
     verified code never uses);
   - attributes outside 0011 item 7's allow-list on the enumeration or on an enumerator;
   - in C, an enumerator initializer that names an object: the front end refuses it (`--diag_error=1901`, 0017 item 2),
     and the encoder rejects it too (defence in depth, as for case values).
3. **Each enumeration is its own VIR type.** It has the size and signedness of the IL type (the underlying type in
   C++, the compatible type in C: `enum color` is `unsigned int` in C and `int` in C++, P25) and a **value range**
   `[lo, hi]`, the same in both modes:
   - fixed (an explicit underlying type, or a scoped enumeration, whose underlying type is `int` when none is written,
     [dcl.enum]/5): the underlying type's range;
   - not fixed: [dcl.enum]/8 in C++20's wording (two's complement: the values of a hypothetical integer type of
     minimal width; also what clang and GCC use; C++17's wording would give `{0}` the single value 0), computed
     exactly (decimal strings) from the enumerators' values: with `emin`, `emax` the
     smallest and largest (both 0 for an empty enumerator list), the smallest `M ≥ 1` such that `[0, 2^M − 1]` holds
     them when `emin ≥ 0`, else such that `[−2^(M−1), 2^(M−1) − 1]` does. `{0}` gives `[0, 1]`, `{0, 1, 2}` `[0, 3]`,
     `{−1, 0, 1}` `[−2, 1]`, an empty C++ enumeration `[0, 1]`.
   **C gets the same range**, which is stricter than C: a C program that stores `(enum color)100` in an `enum color`
   is valid C and is reported here (item 5). Two enumeration types are never the same VIR type, and an enumeration is
   never the same VIR type as an integer type, even of the same size and signedness: every conversion between them is
   explicit in the VIR (VIR has no implicit conversion), so a conversion the IL left implicit fails closed.
4. **Every value of an enumeration type lies in its value range.** Parameters and results carry the range as their
   type range (0010 item 1): a precondition of the function (checked at each call), a postcondition (checked at each
   return). A function declared here and defined elsewhere, or trusted, is assumed to return a value in the range, as
   for every result type; in C the range is not a fact of the type (valid C may hold any value of the compatible type),
   so for parameters and trusted results it is an assumption about code outside the verified unit (item 12). The automatic range invariant of a loop covers enumeration locals. `old` and `decreases` read
   the integer value.
5. **Every conversion into an enumeration is checked** (amends 0010 item 5): explicit (`(enum color)x`,
   `static_cast<color>(x)`) and implicit (C allows `enum color c = 2;`, assignment, arguments and returns from an
   integer). A value outside the value range is a `conversion` error at the conversion ("the value may not be one of
   the values of `enum color` (0 to 3)"), whatever the underlying type's signedness: never a wrap, also for `enum E :
   unsigned char` (owner). A value inside the range need not be an enumerator (`(enum color)3` with `{RED, GREEN,
   BLUE}` is valid in both languages). A conversion from `bool` into an enumeration is rejected
   (`unsupported_expression`).
6. **Conversions out of an enumeration.**
   - Explicit conversions to an integer type (`(int)c`, `static_cast<int>(s)`) and conversions to `bool` (`if (c)`,
     `(bool)c`: `c != 0`) are accepted, with 0010 item 5's rules for the target.
   - An **implicit** conversion of an enumeration value to an integer type is accepted only where it means the same
     in both languages:
     (i) as a direct operand of `==` or `!=`;
     (ii) as the controlling expression of a `switch`, or a case value (converted to the condition's promoted type);
     (iii) as a whole initializer, the right-hand side of an assignment, a call argument or a returned value (the
     target type is the declared one);
     (iv) when its operand is an enumerator.
     When the operand of (i)–(iii) is a folded constant (a named constant of enumeration type, `c == DEF`), the place is
     the folded constant's original expression, where the conversion sits.
     Every other implicit conversion is rejected at the conversion (`unsupported_expression`): an operand of arithmetic,
     of an ordering comparison, of a unary operator, of a shift (either operand), and an arm of `?:`.
     *(Amended by 0019 item 10, 2026-10-22: (v) as an operand of an ordering comparison whose two operands are values of
     the same unscoped enumeration or its enumerators is accepted too; ordering against any other operand stays
     rejected.)*
   - **Why these agree.** For a fixed underlying type both languages promote by that type (C23 6.3.1.1p2,
     [conv.prom]/4), so the conversions are the same. Otherwise an enumeration value lies in its range (item 4), which
     lies in `int` (item 2) and, when it is non-negative, in `unsigned int`. Converting it to C's promoted type (the
     compatible type: `unsigned int` or `int`, P26) or to C++'s (`int` for every unfixed enumeration, P26: never
     another type, so the planning's stop rule did not trigger) therefore keeps its value, so (i) compares the same values in both languages (an `int` operand that
     is negative is not congruent to any non-negative `int` modulo 2^32, so `(unsigned)c == (unsigned)-1` is false
     exactly when `c == -1` is), (ii) matches the same case, (iii) converts the same value to the same declared type,
     and (iv) an enumerator is an `int` in both languages after C++'s promotion to `int` (or the same fixed underlying
     type). Arithmetic, ordering against a negative value and shifts are computed in C's promoted `unsigned int` and
     C++'s `int`, and differ.
   - **Where C writes no conversion** (P26): the controlling expression of a `switch` over an enumeration without a
     fixed type, or with a fixed type of rank at most `int` (cerastes promotes it to its promoted type and converts
     each case constant to that type, as C23 6.8.5.3 says; the front end writes those conversions as casts into the
     enumeration, its own way of naming that type, so they are not conversions into the enumeration in the sense of
     item 5: `case 4:` over `enum color` is a case that is never taken, in both modes *(withdrawn by 0019 item 11,
     2026-10-22: a case value that is not an enumerator's value is now rejected, as `-Wswitch` does)*; a written cast,
     `case (enum color)2:`, is a checked conversion and then promoted). An enumeration with a fixed type above `int`
     (`: long`, `: unsigned long`) is not promoted, and its case constants convert into the enumeration, checked
     (`case -1:` over `enum E : unsigned long` is a `conversion` error; C++ refuses it) *(withdrawn by 0019 item 11,
     2026-10-22: a case value that is not an enumerator's value is now rejected first)*,
     conditions (`if (c)`, `!c`, `&&`: `c != 0`), the operand of unary `-` and `~` and both operands of a shift (the
     operation has the enumeration's type), and the left operand of `++`, `--` and compound assignment. The first two
     are accepted; an operation on a raw enumeration operand is rejected at the operation, the same rule as at the
     conversion.
7. **Scoped enumerations** (C++) have no implicit conversions: `switch`, `==`, `!=` and the ordering comparisons of two
   operands of the same scoped enumeration are operations on the enumeration's values; `static_cast` in and out is
   items 5 and 6. (Ordering of an unscoped enumeration's values is rejected by item 6 even between two operands of the
   same enumeration: lifting it is a later, separate rule.) *(Amended by 0019 item 10, 2026-10-22: lifted for two
   operands of the same unscoped enumeration.)*
8. **Also rejected** (`unsupported_expression`): `?:` whose result has an enumeration type (in C++ `x ? RED : GREEN`
   has the enumeration's type and is rejected; in C it has type `int`, P26, and is an ordinary `int` expression, so
   the two modes accept different programs here, never with different meanings); compound assignment and
   `++`/`--` on an enumeration variable (C allows them; C++ refuses them, P26) and a compound assignment whose
   right-hand side is an enumeration value (`x += c`): both compute in the promoted type, which differs by language
   (item 6).
9. **Enumerators.** A use of an enumerator is a literal holding the front end's value of the enumerator, of the
   enumerator's type in the IL (`int` in C, the enumeration in C++, P25): in function bodies, contracts, `check`,
   invariants, measures, case labels (`case RED:`), named constants' initializers and other enumerators' initializers.
   It is recognized by `is_named_constant_definition` (il_def.h:4931), and its enumeration (its type in C++ and for a
   fixed C type, the `affiliated_type` of its `int` type otherwise) must be one the encoder accepted (defence in depth:
   the front end also sets the flag on predeclared SVE/RISC-V enumerations and on module and member constants, audit).
   P25 found the flag on exactly the constants of
   the enumerations' lists (`constant_list`, or the `constants` of a scoped enumeration's `assoc_scope`), and every
   reference is an `enk_constant` whose constant is that very entry, also inside a folded constant's `expr` chain. It
   is recognized before the folded constant's original expression is read, so an enumerator's initializer is encoded
   once, not at each use. **Copies** (owner, 2026-10-22): in a few places the front end makes a fresh constant of the
   enumeration's type holding an enumerator's value, without the flag and without an `expr` (the C++ initializer
   `color d = GREEN;`, a scoped `case S::SA:`, the same shapes for C fixed types, P25). They are not recognized as
   enumerators: they take the existing path of a constant without an original expression, a literal of the front end's
   value, which `check_program` requires to lie in the value range. No value or position heuristic is involved, and a
   constant of an enumeration type that is not an enumerator's copy keeps its `expr` chain (`(color)7` is `color 7
   {expr=cast(...)}`, P25), so its conversion is checked.
   `names_object` (0017 item 7) does not look inside an enumerator: an enumerator names no object, so
   `do { … } while (A);` with `A == 0` runs once (a `DoOnce`) in both modes.
10. **Enumerator initializers are checked as written, in the prologue.** An enumerator with a written initializer is,
    for decision 0017 item 4, a named constant of the unit: its initializer encoded by its original expression (the
    enumerators it names are their values, item 9), its obligations asserted at their own positions in the
    enumeration's declaration, then an `internal` guard that the value as written equals the front end's value of the
    enumerator, in the prologue of every method, in declaration order with the other named constants. An earlier
    enumerator named in the initializer appears at its final type under an operation of the definition-time type (P25:
    `CB = CA + 1` with `CA = 300u` is `add[unsigned int](const C 300, 1U)` in C++): it is its value, converted to the
    operation's type (item 6 (iv)). So `enum { S = (signed char)300 };` is a `conversion` error at its line, and in C
    `X = 2147483647 + 1` an `overflow` error and `1 << 40` a `shift` error (C23 folds them with a warning; C++ refuses
    them, error #28, P27). A literal initializer has no `expr` and nothing to check, and neither has `sizeof`,
    `_Alignof` or `alignof` (their value, 0010 item 12). **In C, an enumerator named inside the list has type `int`
    when its value fits** (C23 6.7.3.3), but the front end types the reference the C++ way, by the initializer's type
    (Opus audit, round 1: `enum { U = 2u, V = U > -1 };` folds `V` to 0, C gives 1; P25), so in C an enumerator
    reference whose node type differs from the enumerator's own type is rejected (`unsupported_expression`); in C++
    that typing is right and the reference is converted. The same typing is seen by `sizeof`, `_Alignof` and `typeof`
    without a reference node being encoded (Opus audit, round 2: `enum { N = sizeof(int), M = sizeof(N) };` verified
    `M == 8`, C gives 4; round 3: an array bound or a `typeof` inside a type operand, `_BitInt(e)`), so in C an
    enumerator's initializer may use `sizeof` and `_Alignof` only of an arithmetic type that is not bit-precise, named
    directly or through plain typedefs, and no `typeof` type (a cast's target included); anything else is rejected
    (`sizeof(int)` stays accepted). An enumerator without an
    initializer has the previous enumerator's value plus one (0 for the first, C23 6.7.3.3, [dcl.enum]/2; told from a
    literal one by `enum_value_range`, il_def.h:1975, P25); the encoder checks that the front end's value is exactly
    that and fails closed otherwise (no Viper check: it is a property of the IL, like the identification of item 9). `perturb_folded` (0017
    item 9) also moves the value each enumerator guard compares with.
11. **One meaning in both modes.** *(Amended by 0019 items 10–11, 2026-10-22: ordering of one unscoped enumeration
    is accepted, and a switch on an enumeration value is held to `-Wswitch`'s two rules in both modes.)* With items
    3–10 every enumeration value is in its range, and `==`/`!=`, `switch`,
    conversions out of an enumeration and enumerator arithmetic have one meaning in C and C++. `.cpp` only: `enum
    class`, `E::A`, `static_cast`, empty enumerations, enumerations used where C++ needs a conversion that C makes
    implicitly, a `?:` of enumeration type (rejected). `c23` only: implicit integer-to-enumeration conversions (accepted
    and checked), `e++` and `e += 1` (rejected), an enumerator initializer that C++ refuses (`2147483647 + 1`, `1 <<
    40`), an enumerator list that names an earlier enumerator of another type than `int` (rejected in C, item 10; so
    an enumerator's value can differ between the modes, like `sizeof('a')`, 0017 item 8, and such a program runs in
    one mode only). The front end's warning #188 (C: an enumerated type mixed with another type, as in `enum color c = 2;`) is
    suppressed in verify mode, since every such conversion is checked (amends 0011 item 7).
12. **Several units (C).** C allows a function in another unit to pass, or a trusted function to return, any value of
    the compatible type; cerastes assumes the value range (item 4). The single-unit assumption on the Verifying a
    program page says so. In C++ a value outside the range is already undefined.
13. **Facts relied on** (`internal/il-inventory.md`, "M4.2b2"): (P25) the type entry of each kind of enumeration
    (`enum_type`, `is_scoped_enum`, `has_explicit_enum_base`, `base_type`, `constant_list`/`assoc_scope`,
    `enumerator_list_complete`, `incomplete`), one entry per enumeration (a C23 redefinition of a tag gives a second
    entry, harmless: every use refers to the first, and both are checked), C's compatible types, the enumerator's type
    per mode, the flag and pointer identity of every reference, the copies, the definition constant and its `expr`,
    `enum_value_range`; (P26) every conversion node the front end puts around an enumeration operand, per mode, C's
    unconverted operands, and C++'s promotion to `int`; (P27) the front end's enumeration diagnostics (none for an
    out-of-range cast or enumerator; #188 in C; #1901 an error in C).
14. **Amends** 0010 items 2 and 5; 0011 item 7 (#188 suppressed); 0012 item 12; 0017 item 7; and
    `internal/encoding.md` §2, §3, §7 and §8.

## Consequences

- Enumerations verify in both modes; the rules are on the Integer semantics (Enumerations, Conversions), Verifying a
  program (what is not supported; the single-unit assumption) and Error messages (`conversion`) pages.
- A program that converts an out-of-range value into an enumeration is reported, in C too.
- Emitted Viper does not change for any program without an enumeration: an enumeration is a new VIR type, and no
  existing rule changes for the other types.
- Lifting later: ordering of unscoped enumeration values (both operands of one enumeration; *lifted by 0019 item 10*),
  compound assignment on enumerations, enumerations in classes and templates (M7, M5), system enumerations (M8).
- Found while probing (not specific to enumerations; a tool failure, never a verdict): in C++ an error #28 whose note
  verify mode suppresses makes the front end stop with internal error #2656 (`enum { A = 1 << 32 };`, `constexpr int z
  = 1 << 32;`), so `cerastes verify` exits 3 (handoff §6). *(Fixed in verify mode by 0019 item 9; EDG bug 0022.)*

*2026-10-22, amended by 0019 (M4.2c; the owner's answers to handoff §11 questions 18 and 19): items 6 and 7 — an ordering comparison (`<`, `<=`, `>`, `>=`) whose two operands are values of the same unscoped enumeration or its enumerators is accepted (both modes convert both operands to one promoted type that keeps every value of the range and their order); ordering against any other operand stays rejected; item 6(ii) — a case value that is not the value of an enumerator is rejected (`case_not_in_enumeration`), no longer a case that is never taken; item 11 — a switch over an enumeration without `default` must list every enumerator's value (`enumerator_not_handled`): both rules are clang's and GCC's `-Wswitch` (0019 item 11).*

*2026-10-23, amended by 0022 (M4.4a): item 5 — a checked conversion is now the rule for every integer type, not a special case of enumerations; item 6 — (i) holds only without a signed operand that may be negative: `==`/`!=` between a value of an enumeration compared in an unsigned type in C (no negative values, and an unfixed type or a fixed unsigned base of rank `int` or above) and a signed non-enumeration operand that is not a constant ≥ 0 is rejected in both modes, as the compilers' `-Wsign-compare` warns in C, and so is `==`/`!=` between values or enumerators of two different enumerations (`-Wenum-compare`); a truth value (a comparison, `!`, `&&`, `||`) counts as non-negative (0022 item 5, P39).*
