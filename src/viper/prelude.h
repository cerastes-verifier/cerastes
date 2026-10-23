// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The Viper prelude (decision 0012 item 11): functions with bodies, nothing else -- no domain, no axiom, no
// bodyless function, no postcondition that nothing proves.  Every function is total (defined for every
// argument), so a contract that mentions one is always well-formed to Viper.  Only the functions a program uses
// (and what they need) are emitted.
#pragma once

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
std::string prelude_text(const std::set<std::string>& used);

// The prelude functions that `name` calls.
std::set<std::string> prelude_closure(const std::set<std::string>& used);

}  // namespace cerastes::viper
