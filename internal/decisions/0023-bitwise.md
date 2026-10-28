# 0023 — Bitwise operators, multi-character literals, and one verdict for item 5's truth values (M4.4b1)

- **Status:** accepted for M4.4b1. Owner decisions (2026-10-23, M4.4 and M4.4b planning; handoff §11 questions 33 and
  36, §8.1):
  - **Bitwise operators** `& | ^ ~` and `&= |= ^=` are exact on mathematical integers (C23 two's complement), with no
    wrapping (0022). Masks are the explicit modular reduction (0022 item 7).
  - **64-bit is not a cut.** The closed form must work for 64-bit operands; only 128-bit may be cut (a rejection, a test
    and an M8 entry).
  - **Warning shapes** are rejected like the compilers (0020 item 5). Where the IL cannot match a warned shape exactly
    (a literal's spelling), the broader shape is rejected, stricter than the compilers, and each such case is listed
    here.
  - **Question 33** (extended in planning): EDG's warnings for multi-character and truncated character literals,
    #1422, #1654, #26 and #3255, are errors in verify mode.
  - **Question 36**, in its symmetric form: a `?:` or comma whose result is a truth value counts as one in 0022 item 5;
    for `==`/`!=` in C, a `?:` whose arms are enumerators of one enumeration is rejected with 0018 item 8's message, as
    C++ rejects it.
  - **Undecided identities** (during the session, after S21). The closed form leaves some symbolic identities
    undecided (item 2); such a run ends at the driver's timeout with exit 3, as today, and the limitation is
    documented. No Silicon assertion timeout is added. Popcount and `low_bit` tests are cut (pre-approved).
  - **Range postconditions on the bitwise prelude functions** (owner, 2026-10-24). With bodies only, Z3 found the
    range of a bitwise result, which every use of an integer value needs, by luck: the same claim verified in one
    mode and hung in the other. Each bitwise prelude function therefore carries `ensures lo <= result <= hi`, which
    Silicon proves against its body on every run (never assumed). It amends 0012 item 11, and the lint accepts exactly
    that clause in exactly those functions.
  - **Expectation changes.** The five `reject_bit_*` tests are retired (pre-approved). Any other change of a verdict
    stops the work.

  Facts marked (P*n*) and (S*n*) are probes and spikes, recorded with their evidence in `internal/il-inventory.md`,
  "M4.4b1" (P40, P41, S21). Audit round 1 (Opus) found no blocking item; its should-fix items are taken into
  items 2, 4, 5, 6, 7 and 8 (one rule more: the promoted complement comparison in item 5; one shape more: the comma
  ending in an enumerator in item 7), and its notes are in handoff §6. Audit round 2 (the code and the tests) found no
  blocking item; its should-fix items were three tests (the accepted neighbours of item 5, lone enumerators, the 128-bit
  high half), and its notes are in handoff §6.
- **Milestone:** M4.4b1 (2026-10-23).

## Context

0010 item 9 rejected `&`, `|`, `^` and `~` in M3 and planned them for M4.4. M4.4a (0022) removed wrapping, so a bitwise
result, which never leaves its type, and a mask, which is the explicit way to reduce a value modulo 2ᴺ, now fit the
real-integer model without any modular arithmetic. Viper's `Int` has no bitwise operators, and the prelude may hold only
total functions with bodies (0012 item 11), so the operators need a closed form over `Int` that Z3 decides quickly,
including in 0017's closed prologue, where every assert must be decided by evaluation.

The owner's questions 33 and 36 (handoff §11, M4.4a) are small items of the same session: EDG truncates multi-character
literals with only a warning, and 0022 item 5 still gives a few shapes different verdicts in the two modes.

## Decision

1. **Meaning.** `a & b`, `a | b` and `a ^ b` compute in the type the usual arithmetic conversions give (0010 item 6),
   and `~a` in the promoted type of `a`; the front end inserts those conversions (P40). The result is the C23 two's
   complement result of the operation on the operands' mathematical values. It always lies in the computed type's
   range, so the operators carry **no obligations**:
   - for unsigned operands in [0, 2ᴺ), every result is in [0, 2ᴺ);
   - for signed operands in [−2ᴺ⁻¹, 2ᴺ⁻¹), every result is in the same range (`(−1) ^ INT_MAX` is `INT_MIN`, `~INT_MIN`
     is `INT_MAX`).

   `~a` is `−a − 1` for a signed type and `MAX − a` for an unsigned one: linear, inline, never through unary minus
   (whose `INT_MIN` obligation does not apply to `~`).
2. **Encoding** (S21, and the session's measurements of the real encoder's output). `a & b`, `a | b` and `a ^ b` are
   calls of prelude functions, chosen by the computed type's width N (32, 64 or 128) and signedness: `c_and_N`,
   `c_or_N`, `c_xor_N` for a signed type, `c_uand_N`, `c_uor_N`, `c_uxor_N` for an unsigned one. `~a` is inline:
   `-1 - a` for a signed type, `MAX - a` for an unsigned one.
   - **The functions.** For N = 32 and 64 each is flat: a floor-division chain per operand (`q₀ = a`,
     `qₖ₊₁ = qₖ \ 2`, `bitₖ = qₖ − 2·qₖ₊₁`; Viper's `\` is floor division, two's complement's extension for a negative
     operand), a test per bit (`== 2` for `&`, `>= 1` for `|`, `== 1` for `^` on the sum of the two bits), and the sum
     of the result bits weighted by `2ᵏ`, the top bit weighing `−2ᴺ⁻¹` in the signed functions and `2ᴺ⁻¹` in the
     unsigned ones. For N = 128, `c_u<op>_64(a, b) + 2⁶⁴ · c_<op>_64(a \ 2⁶⁴, b \ 2⁶⁴)`, the high half of the
     function's own signedness. Each reads the low N bits of its operands as two's complement, which is exact for
     operands in the computed type's range (every operand is, by 0010 item 2); S21 and audit round 1 checked the
     construction against Python's operators at every width, signed and unsigned, with no mismatch.
   - **One postcondition each** (owner, 2026-10-24; amends 0012 item 11). Each function has `ensures lo <= result &&
     result <= hi`, the range of its type. Silicon proves it against the body on every run; nothing assumes it. These
     are the only prelude functions with a postcondition; none has a `requires` or `decreases`, and there is no axiom
     or domain. If a prelude postcondition ever failed, Silicon's error has no position-map entry, and the driver
     reports "an error the map does not explain" with exit 3 (`report.cpp`, `pipeline.cpp`; `--selftest` covers that
     path): a tool failure, never a verdict. The lint accepts exactly that clause, with the type's exact range, in
     exactly these 18 functions.
   - **Why bodies alone did not work.** Every use of an integer value needs its range (a parameter's, a result's, a
     conversion's), and the range of a sum of N per-bit `ite` terms has no bound until every term is decided: Z3
     found it by luck. S21's hierarchical form (8-bit leaves, halves summed) passed S21's battery, but the encoder's
     methods hung (`r == (a & b)` with the result's range check), and whether they did depended on unrelated
     functions in the file (the same `c_and_32` text verified once an unused `c_or_32` was emitted) and flipped
     between the modes (C's `? 1 : 0` wrapping of a truth value). With the postcondition, the range is a fact at each
     call.
   - **What Z3 decides** (measured with `build/cerastes verify`, both modes): every closed expression at 32 and 64 bits,
     and at 128 bits every one whose operands are literals or shifts of 1, so the prologue's asserts (0017 item 4) are
     decided by evaluation, including a failing one (one error); results named in
     contracts (`r == (a & b)`); negative operands and results equal to the minimum; the range of a result; a bound by
     a constant mask (`u & 255 <= 255`); `x & 1 == x % 2` and `x & 255 == x % 256` for `x >= 0`; `x ^ x == 0`;
     `x & y == y & x`; and refutations of false claims at constant arguments. The bitwise tests take 1.4–6.3 s per
     file at 32 and 64 bits, and about 11 s at 128 bits.
   - **What it does not decide** (owner, 2026-10-23: keep this and document it). Relations between a bitwise result
     and other arithmetic on the same operands, and identities that need the bits of a symbolic operand:
     `(u | 4) & 4 == 4`, `v & 0xFFFFFFFF == v % 2³²` for a 64-bit `v`, `x & ~15 == x - x % 16`, `(u & ~4) & 4 == 0`,
     `(a ^ b) ^ b == a`, `x | 0 == x`, `x & x == x`, `0 <= (u & m) <= m` and `u | m >= m` for a symbolic `m`, the
     bounds of `x & -x` (`low_bit`) and a popcount loop; and, at 128 bits, a closed claim whose bitwise operand is a
     shifted constant other than 1 (`(__int128)2 << 100`, which lowering writes `2 * c_pow2(100)`; the same value
     written `(__int128)1 << 101`, and the 64-bit form, are decided: the mutation review). Z3 does not answer on them;
     Silicon runs until the driver's
     `--timeout` (default 300 s) and the run exits 3 as a tool failure ("not checked", 0011 item 8), never with a
     verdict. A false symbolic claim can hang the same way, depending on its shape. Silicon's `--assertTimeout` would
     turn each into an ordinary error in seconds (sound, incomplete); the owner kept the driver as it is. Lemmas that
     decide such identities are a question for M5 (Gate G2's `low_bit`, handoff §11).
   - **Rejected forms.** A flat sum without a postcondition and S21's hierarchy (above); a flat sum with an
     independent `\ 2ᵏ` per bit (constants only); bounded recursion (Silicon unfolds recursive functions with limited
     fuel, so closed prologue asserts would stay undecided); and Silicon options such as `--proverSaturationTimeout`
     (no effect).

   Amends 0012 item 11 (the prelude list and its postconditions) and 0017 item 4 (the prelude functions a closed assert
   may use: total, non-recursive, decided by evaluation).
3. **Compound assignment `&=`, `|=` and `^=`** compute in the usual arithmetic conversions of both operands and convert
   the result back with a check, like the other compound forms (0010 item 6, 0022 item 1). Consequences, each an error
   on the paths where the value does not fit:
   - `sc |= 0x80` on a `signed char` that may be non-negative: `conversion` (the result 128–255 does not fit);
   - `uc = ~uc` and `uc ^= ~0`: `conversion` (`~uc` is a negative `int`);
   - `u & -8`, `u & ~7` and `u &= ~FLAG` with an `int` `FLAG`: `conversion` (the negative `int` operand converts to
     `unsigned int`); the rewrite is `~7u`;
   - for an `unsigned int` `x`, `x & -x` is `overflow` at the unary minus (0022 item 1); for an `unsigned char` `x` it is
     computed in `int` and fits.

   A `bool` target of a compound assignment stays rejected (0012). A `bool` right operand is promoted to `int`.
4. **Enumerations** follow 0018's rule for arithmetic (0018 item 6): a value of an enumeration as an operand of
   `& | ^ ~` (including C's bare enumeration operand of `~`, P26) is rejected (`unsupported_expression`) in both modes,
   and an enumerator is accepted in both, as in arithmetic (`GREEN | 2`). `&`, `|` and `^` whose operands are
   enumerators of two different enumerations (`RED | NA`) are rejected in both modes: clang++ and g++ warn by default
   (`-Wdeprecated-enum-enum-conversion`, C++20). The same shape with `+` (`RED + NA`) predates this record and verifies
   (audit round 1; handoff §11). A compound bitwise assignment on or with an enumeration is rejected like the other
   compound forms (0018 item 8; in C++ the front end refuses `c |= 1` itself, #511).
5. **Warning shapes** (amends 0020 item 5; P41: 916 shapes in clang 21 and GCC 16, `-Wall -Wextra`, both modes; EDG
   warns on none of them). Each rule below is a rejection in both modes (`unsupported_expression`-like rejections with
   their own tags, at the operator). The rules read the IL through compiler-generated casts (an explicit cast is a
   node), the front end's `is_parenthesized` (on the innermost written operation, also inside a folded constant's
   `expr`, P40) and folded constants' `expr`. A **truth value** is item 7's: a comparison, `!`, `&&`, `||`, a `bool`
   node (a `bool` variable or call, a conversion to `bool`, `true`/`false`), a `?:` of truth values or a comma ending in
   one. P41 found no compiler counting a `?:` as a truth value in the `?:` condition rule; that shape was not probed,
   and counting it rejects more, never less (listed). The detectors run
   where the encoder encodes the operator, so they apply in bodies, contracts and annotations (where the compilers
   see nothing, since the macros expand to nothing, but the expression means the same), the prologue's initializers
   and case values alike.
   - `bitwise_operand_parentheses` (`-Wparentheses`, `-Wbitwise-op-parentheses`): an unparenthesized operand of binary
     `&`, `|` or `^` that is a comparison (both compilers; GCC also when both operands are comparisons, `a == b & c ==
     d`), a `+` or `-` (GCC), or a different bitwise operator (`a | b & c`, `a ^ b & c`, `a | b ^ c`; both). The
     compilers' verdicts change inside macros; cerastes rejects the shape whether or not a macro wrote it (broader).
   - `logical_not_bitwise` (clang `-Wlogical-not-parentheses`, GCC `-Wparentheses`): `!e` as the left operand of `&`
     or `|` (never `^`), unless the right operand is a truth value, or, for `|` only, a constant of value 0 or 1 in any
     spelling or suffix (GCC decides by value: `!x | 1L` is silent) or an explicit cast of a comparison, `!`, `&&` or
     `||` (`!x | (int)(y > 0)`, silent in all four; clang never warns on `|`), but not of a `bool` variable (GCC warns
     on `!x | (int)q`, P41 `or_castq`). So `!x & 1`, `!x & y`, `!x & (int)(y > 0)`
     (clang), `!x | 2`, `!!x & y` and `!(x > 0) & y` are rejected; `!x & !y`, `!x & (y > 0)`, `!x | 1`,
     `!x | (int)(y > 0)`, `x & !y` and `(!x) & y` are accepted.
   - `complement_of_truth_value` (`-Wbool-operation`): `~e` where `e` is a truth value, a `&`, `|` or `^` of two truth
     values (clang), or a unary `+` of one. An explicit `(int)` cast is a barrier. `~0`, `~1`, `~x` are accepted.
   - `bitwise_condition_parentheses` (clang `-Wbitwise-conditional-parentheses`): the condition of a `?:` is an
     unparenthesized `&` or `|` whose right operand is a truth value (`a & b > 0 ? c : d`). `a & b ? c : d` with
     ordinary operands is silent everywhere and accepted.
   - `bitwise_instead_of_logical` (clang): `&` or `|` whose two operands are truth values that both have a side effect
     (a call, an assignment, `++`/`--`): `f() & g()` with `bool` results. One side effect, `^`, `int` operands and the
     compound forms are silent and accepted.
   - `tautological_bitwise_compare` (GCC `-Wtautological-compare` for every type, clang
     `-Wtautological-bitwise-compare` for `int`; the union is GCC's): `(x & C1) == C2` or `!=` with constants where
     `C2 & ~C1 != 0` (including `C1 == 0`), and `(x | C1) == C2` or `!=` where `C1 & ~C2 != 0`, either operand order;
     (clang) `x | C` or `C | x` with a nonzero constant `C` used as a truth value (an `if`/`while`/`?:` condition,
     an operand of `!`, `&&`, `||`, a conversion to `bool`, and the whole condition of `requires`, `ensures`,
     `invariant`, `check` and `assert`); and (GCC `-Wsign-compare`, "comparison of promoted bitwise complement of an
     unsigned value with constant", `-Wall` in C++, `-Wextra` in C) a comparison of `~e`, where `e` has type `unsigned
     char` or `unsigned short` (looking through an explicit cast to `int` or to such a type), with a constant ≥ 0 or a
     value promoted from an unsigned type narrower than `int`: `~uc == 0x0F` is always false, since `~uc` is a negative
     `int`. Audit round 1: this shape carries no `conversion` check, so the owner's answer to handoff §11 question 35
     (keep the path-precise check for sign-compare shapes) does not cover it, and the union rule of 0020 item 5 rejects
     it.
   - `xor_as_power` (`-Wxor-used-as-pow`, all four): `2 ^ n` with `n` in 0–64 and `10 ^ n` with `n ≥ 0`, both
     operands integer constants. The compilers decide by spelling (clang: decimal literals without a suffix, outside a
     macro; GCC also accepts suffixes and, in C, parenthesized literals), which the IL does not keep. cerastes rejects
     the shape by value whatever the spelling (owner: the broader shape), so `0x2 ^ 8`, `02 ^ 8`, a named constant
     `TWO ^ 8` and `(1 + 1) ^ 8`, silent in all four compilers, are rejected too.
   - **Extensions of 0020's rules:** `assignment_as_condition` also covers `|=` as the whole condition of `if`,
     `while`, `do` or `for` (clang; `&=`, `^=` and the arithmetic compounds are silent everywhere); `self_assignment`
     also covers `x &= x` and `x |= x` (clang; `x ^= x` is silent).
   - **Not rejected, listed** (audit round 1; handoff §11, with the other `-Wall` shapes P41 measured):
     - clang's `-Wtautological-constant-out-of-range-compare`/`-Wtautological-constant-compare` (on by default, both
       modes) on a comparison of `&`, `|` or `^` of two truth values with a constant outside 0–1 (`(p ^ q) == 2`,
       `(p & q) == -1`, `(p & q) >= 0`). The same family without a bitwise operator (`(a > b) == 2`, warned by all
       four compilers) predates this record and verifies; both go together;
     - GCC's `-Wtype-limits` (`-Wextra` only, outside 0020 item 5's `-Wall` union): `(uc & vc) == 300`,
       `(uc | vc) > 255`, `(uc ^ vc) < 0`, like `u >= 0`;
     - shapes inside `_Static_assert`/`static_assert` (P41's `~true`, `2 ^ 8`): static assertions are not encoded
       until M4.4b2 (record 0024), which runs these detectors on their conditions;
     - shapes inside the operands of `sizeof`, `_Alignof` and `typeof`, which are never evaluated or encoded
       (`sizeof(char[2 ^ 8])` verifies, with EDG's value; audit round 2). Their verdicts are right; M4.4b2, which
       inspects the types inside such operands, may scan them as `unevaluated_side_effect` already does.
   - **Silent everywhere and accepted:** `x |= y == z`, `b1 ^ b2`, `!a & !b`, `(a & b) == c`, `a | (b & c)`, `u & ~7`
     (a `conversion` error by item 3, not a warning shape), `sc |= 0x80`, `x ^= x`, `x & -1`, `~uc & 0xFF`.
6. **`eval_const`** (`check_program`'s constant evaluation) computes `~` explicitly (signed `−v − 1`, unsigned
   `hi − v`; unknown for 128-bit types) and `& | ^` exactly on operands of one type, followed by its range check;
   anything else is unknown, and so is an `unsigned __int128` value outside the evaluator's `__int128` range.
7. **Question 36: one verdict in both modes for 0022 item 5** (amends 0022 item 5).
   - **Truth values.** An operand counts as a truth value (0 or 1, so never "a signed operand that may be negative")
     when, after the compiler-generated conversions are peeled and folded constants are read through their `expr`
     (in C the arms of `x ? true : false` are `int` constants whose `expr` is the `bool` constant, P40), it is a
     comparison, `&&`, `||`, `!` or a conversion to `bool`; a node of type `bool`; a `?:` whose two arms are truth
     values; or a comma whose right operand is one. An
     explicit cast is never peeled (`c == (int)b` stays rejected), and an `int` constant is not a truth value
     (`c == (x > 0 ? 1 : 2)` stays rejected). So `c == (x ? true : false)`, `c == (x ? (x > 0) : (x < 0))`,
     `c == (b ? b : b)` and `c == (x++, x > 0)` are accepted in both modes.
   - **A `?:` of enumerators in C.** For `==` and `!=` in C, an operand that is a `?:` whose two arms are enumerators of
     one enumeration (after peeling, also inside a folded constant's `expr`) is rejected with 0018 item 8's message,
     whatever the other operand is, as C++ rejects the same `?:` (its type is the enumeration). So
     `(x ? RED : GREEN) == y`, `c == (x ? RED : GREEN)` and `c == (1 ? NB : NC)` are rejected in both modes. Other
     positions of such a `?:` are unchanged (0018 item 8: C accepts `return x ? RED : GREEN;` from an `int` function,
     and `(x ? RED : GREEN) & 1`, which C++ rejects). The arms may also be commas ending in such enumerators.
   - **A comma ending in an enumerator in C** (audit round 1). C++ types `(x++, GREEN)` as the enumeration and C as
     `int`. For `==` and `!=` in C, such a comma counts as a value of its enumerator's enumeration, as in C++: `c ==
     (x++, GREEN)` compares two values of one enumeration (accepted in both modes) and `(x++, GREEN) == y` with a
     signed `y` is rejected in both.
8. **Question 33: character literals EDG truncates are refused** (amends 0011 item 7 and 0022 item 2). Verify mode runs
   EDG with `--diag_error=1422,1654,26,3255`:
   - #1422 `multi_char_literal` (`'ab'`): an implementation-defined value; both compilers warn by default
     (`-Wmultichar`);
   - #1654 `leading_character_ignored_in_char_literal` (`'abcde'`): the leading characters are dropped;
   - #26 `too_many_characters` (`L'ab'` keeps its first character in C; already an error in C++);
   - #3255 `utf16_char_lit_too_long` (C: a UTF-16 literal whose value does not fit is truncated).

   The diagnostic pragmas that could lower them again are already rejected in verified files (0017 item 2), and so is
   `#pragma GCC system_header` in a user header (0011 item 4), the other way to silence them: EDG drops these errors in
   system headers (P41's sweep: `'ab'` in a `--sys_include` header is silent, in a `-I` header #1422), and a system
   header's macros, named constants and enumerators are reported or rejected where they are used. No header the tests
   include trips them.
9. **M5 note.** An integral `T` (M5d) still rejects bitwise operators, and the per-instance fallback encodes them with
   item 2's functions, which do not decide the bounds of `x & -x`: Gate G2's `low_bit` needs lemmas or another form
   (handoff §11).

## Amends

- **0010:** item 9 (bitwise operators are supported, with this record's meaning).
- **0011:** item 7 (the verify-mode diagnostics of item 8).
- **0012:** item 11 (the prelude gains item 2's functions, the only prelude functions with a postcondition: their
  range, proved by Silicon, never assumed) and item 12 (bitwise operators leave the rejected list).
- **0017:** item 4 (the prelude functions a closed assert may use).
- **0018:** items 6 and 8 (the rejections of item 4 here; `conversion()`'s message names the bitwise operators).
- **0020:** item 5 (the bitwise warning shapes of item 5 here; the note on `if (x |= a)`).
- **0022:** item 2 (character literals EDG truncates are refused), item 5 (item 7 here) and item 7 (masks).
- **`internal/encoding.md`:** §3 (the lowering table and the prelude), §7 (item 7 here); **`tests/README.md`** as the
  harness needs.

## Consequences

- Bitwise code verifies with its exact two's complement meaning in both modes; the emitted Viper of programs without
  bitwise operators is unchanged.
- A mask written to reduce a value (`(unsigned char)(x & 255)`) is the explicit modular form; a mask whose operand is a
  negative `int` constant converted to an unsigned type (`u & ~7`) is a `conversion` error, written `u & ~7u` instead.
- A program with a multi-character literal is refused (exit 2).
- 0018 item 8's mode difference stays outside `==`/`!=`: `(x ? RED : GREEN) & 1` is accepted in C and rejected in
  C++ (a `?:` of enumeration type).
- A claim whose proof needs a symbolic bit identity (item 2) is not decided: the run ends at `--timeout` with exit 3.
  The Integer semantics page says which claims are decided.

*2026-10-26, amended by 0025 (M4.4c): item 4 — two enumerations in `+ - * / %` and as the arms of `?:` are rejected too (0025 item 6, R8); item 5 — the truth-value comparisons it listed as not rejected (`(p ^ q) == 2`) are now rejected (0025 item 6, R6).*

*2026-10-27, amended by 0026 (M4.4d): item 2 — `a & m` with a constant mask `m = 2^k − 1` is written `a % 2^k`, and with `m = ~(2^k − 1)` `a − a % 2^k` (Viper's Euclidean `%`), for every width and both signednesses (0026 items 7–8); `v & 0xFFFFFFFF == v % 4294967296`, `x & ~15 == x − x % 16` for `x ≥ 0` and the range check of a masked result are decided now, and leave the list of what Z3 does not decide.*

*2026-10-27, amended by 0027 (M4.5a): item 2 — `x | m` and `x ^ m` with a constant mask (2^k − 1 or its complement) are written without `c_[u]or_N`/`c_[u]xor_N`: `|` inline, `^` as the prelude functions `c_[u]xor_[c]mask_N_k` (0027 items 1–3); claims about them over a variable are decided, chains as 0027 item 4 measures, and leave the list of what Z3 does not decide.*
