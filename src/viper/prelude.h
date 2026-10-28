// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The Viper prelude (decision 0012 item 11, amended by 0023): functions with bodies, nothing else -- no domain, no axiom, no
// bodyless function; the only postconditions are the bitwise functions' ranges, which Silicon proves.  Every function is total (defined for every
// argument), so a contract that mentions one is always well-formed to Viper.  Only the functions a program uses
// (and what they need) are emitted.
#pragma once

#include <map>
#include <set>
#include <string>

namespace cerastes::viper {

// The prelude text for the used functions, in dependency order.
//   c_div(a, b)     C's division of SIGNED operands: truncates toward zero; 0 when b == 0 (the divisor
//                   check is a separate obligation)
//   c_rem(a, b)     C's remainder of signed operands: a - b * c_div(a, b), the sign of a; 0 when b == 0
//   c_udiv, c_urem  the same for operands known to be non-negative (unsigned types): 0 unless a >= 0 && b > 0.
//                   Separate functions because Z3 proves the range of these quickly and not of the general ones
//   c_pow2(n)       2^n for 1 <= n <= 127, 1 for n <= 0, and 2^128 above (always >= 1)
//   c_shr(a, n)     a >> n: floor(a / 2^n) (arithmetic for negative a)
//   c_and_N, c_or_N, c_xor_N     (N = 32, 64, 128; decision 0023 item 2, spike S21) C's `& | ^` on operands of a signed
//   c_uand_N, c_uor_N, c_uxor_N  resp. unsigned computed type of N bits, two's complement on the mathematical values.
//                   Total: they read the low N bits of their arguments (the signed ones as a signed value).  N = 32 and 64 are
//                   flat: a floor-division chain per operand, a per-bit test, a weighted sum, the top weight -2^(N-1) for the
//                   signed ones.  N = 128 is c_u<op>_64(a, b) + 2^64 * c_[u]<op>_64(a \ 2^64, b \ 2^64) (prelude_closure adds
//                   the 64-bit functions).  Each has one `ensures`, the range of its type: the only postconditions in the
//                   prelude, proved by Silicon against the body (never assumed); no `requires`, no `decreases`, no axiom.
//   c_[u]xor_mask_N_k, c_[u]xor_cmask_N_k   (decision 0027 item 3) x ^ (2^k - 1) and x ^ ~(2^k - 1) for x of the signed/unsigned computed type
//                   of N bits: one parameter, the body is the exact form over `a` (a floor quotient and a remainder by the power-of-two literal
//                   2^k), and one `ensures`, the range of the type guarded by the range of the argument (total, no `requires`).
std::string prelude_text(const std::set<std::string>& used);

// The memory model's prelude (decision 0032 item 5, decision 0028 item 2), for a program that has a cell or a pointer: a pair of fields
// `v_<t>`, `d_<t>` for each cell type `<t>` (the key; the value is the Viper type of `v_<t>`: "Int", "Bool" or "Ref"), `field lv: Bool`
// (the liveness cell), and `domain Block` with its eight functions and its four axioms (records 0033 to 0036), each tagged for the lint
// (`// cerastes:domain-ok 0028`, `// cerastes:axiom-ok <record>`).  The fields come in the sorted order of the keys.
std::string memory_prelude_text(const std::map<std::string, std::string>& cell_types);

// The prelude functions that `name` calls.
std::set<std::string> prelude_closure(const std::set<std::string>& used);

}  // namespace cerastes::viper
