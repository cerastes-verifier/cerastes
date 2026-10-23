// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "prelude.h"

#include "dec.h"

namespace cerastes::viper {

namespace {

// n <= 0 ? 1 : (n == 1 ? 2 : (n == 2 ? 4 : ... (n == 127 ? 2^127 : 2^128)))
std::string pow2_table() {
  std::string out = cerastes::vir::dec::pow2(128);
  for (int n = 127; n >= 1; --n)
    out = "(n == " + std::to_string(n) + " ? " + cerastes::vir::dec::pow2(n) + " : " + out + ")";
  return "(n <= 0 ? 1 : " + out + ")";
}

}  // namespace

std::set<std::string> prelude_closure(const std::set<std::string>& used) {
  std::set<std::string> all = used;
  if (all.count("c_shr") != 0) all.insert("c_pow2");
  return all;
}

std::string prelude_text(const std::set<std::string>& used) {
  const std::set<std::string> all = prelude_closure(used);
  std::string t;
  auto fn = [&](const std::string& head, const std::string& body) { t += "function " + head + "\n{\n  " + body + "\n}\n"; };
  if (all.count("c_pow2") != 0) fn("c_pow2(n: Int): Int", pow2_table());
  if (all.count("c_shr") != 0) fn("c_shr(a: Int, n: Int): Int", "(a \\ c_pow2(n))");
  // Signed division and remainder: four explicit cases, so that Z3 can bound the result without nonlinear help.
  if (all.count("c_div") != 0)
    fn("c_div(a: Int, b: Int): Int",
       "(b == 0 ? 0 : (a >= 0 ? (b > 0 ? (a \\ b) : (-(a \\ (-b)))) : (b > 0 ? (-((-a) \\ b)) : ((-a) \\ (-b)))))");
  if (all.count("c_rem") != 0)
    fn("c_rem(a: Int, b: Int): Int",
       "(b == 0 ? 0 : (a >= 0 ? (b > 0 ? (a % b) : (a % (-b))) : (b > 0 ? (-((-a) % b)) : (-((-a) % (-b))))))");
  // Unsigned operands are never negative: plain Euclidean division is C's division.
  if (all.count("c_udiv") != 0) fn("c_udiv(a: Int, b: Int): Int", "((a >= 0 && b > 0) ? (a \\ b) : 0)");
  if (all.count("c_urem") != 0) fn("c_urem(a: Int, b: Int): Int", "((a >= 0 && b > 0) ? (a % b) : 0)");
  return t;
}

}  // namespace cerastes::viper
