// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "prelude.h"

#include <algorithm>
#include <functional>
#include <vector>

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

// The bitwise functions (decision 0023 item 2, spike S21 and its follow-up): c_<op>_N for the signed computed types and c_u<op>_N
// for the unsigned ones, op in and/or/xor, N in 32, 64, 128.  N = 32 and 64 are flat (one chain of N bits per operand); N = 128
// is the sum of two 64-bit calls.  Each carries one `ensures`, the range of its type, which Silicon proves against the body once
// per function; without it Z3 re-proves the range of a result at every use and hangs on some shapes.
struct BitFn {
  std::string op;
  int width = 0;
  bool is_unsigned = false;
};

std::string bit_fn_name(const std::string& op, int width, bool is_unsigned) {
  return std::string("c_") + (is_unsigned ? "u" : "") + op + "_" + std::to_string(width);
}

bool parse_bit_fn(const std::string& name, BitFn& f) {
  for (const std::string op : {"and", "or", "xor"}) {
    for (const bool u : {false, true}) {
      const std::string prefix = std::string("c_") + (u ? "u" : "") + op + "_";
      if (name.compare(0, prefix.size(), prefix) != 0) continue;
      const std::string w = name.substr(prefix.size());
      for (const int width : {32, 64, 128})
        if (w == std::to_string(width)) {
          f = BitFn{op, width, u};
          return true;
        }
    }
  }
  return false;
}

// A flat function of width W (32 or 64): the bits of a and b come from the floor-division chain q0 = a, q(k+1) = qk \ 2,
// bit k = qk - 2 q(k+1) (for a negative operand the chain is the infinite two's complement extension), and the result is the
// sum of the per-bit results weighted by 2^k, the top weight being -2^(W-1) for the signed functions.
std::string bit_leaf_body(const BitFn& f) {
  const int W = f.width;
  auto q = [](char v, int i) { return i == 0 ? std::string(1, v) : std::string("q") + v + std::to_string(i); };
  std::string chain, sum;
  for (int k = 1; k <= W; ++k)
    chain += "let qa" + std::to_string(k) + " == (" + q('a', k - 1) + " \\ 2) in let qb" + std::to_string(k) + " == (" +
             q('b', k - 1) + " \\ 2) in ";
  for (int k = 0; k < W; ++k) {
    const std::string ba = "(" + q('a', k) + " - 2 * qa" + std::to_string(k + 1) + ")";
    const std::string bb = "(" + q('b', k) + " - 2 * qb" + std::to_string(k + 1) + ")";
    const std::string test = f.op == "and" ? " == 2" : (f.op == "or" ? " >= 1" : " == 1");
    const std::string t = "((" + ba + " + " + bb + test + ") ? 1 : 0)";
    const std::string w = (k == W - 1 && !f.is_unsigned) ? "(-" + cerastes::vir::dec::pow2(k) + ")" : cerastes::vir::dec::pow2(k);
    if (k != 0) sum += " + ";
    sum += (k == 0) ? t : w + " * " + t;
  }
  return chain + sum;
}

// 128 bits: c_u<op>_64(a, b) + 2^64 * c_[u]<op>_64(a \ 2^64, b \ 2^64): the low half is unsigned, the high half has the
// signedness of the function.
std::string bit_composite_body(const BitFn& f) {
  const int h = f.width / 2;
  const std::string p = cerastes::vir::dec::pow2(h);
  return bit_fn_name(f.op, h, true) + "(a, b) + " + p + " * " + bit_fn_name(f.op, h, f.is_unsigned) + "(a \\ " + p + ", b \\ " + p + ")";
}

// The `^` constant-mask functions (decision 0027 item 3): c_[u]xor_mask_N_k is `x ^ (2^k - 1)` and c_[u]xor_cmask_N_k is `x ^ ~(2^k - 1)` for
// x of the signed (c_) or unsigned (c_u) computed type of N bits (N = 32, 64, 128).  The lowering emits k in 0 <= k <= N unsigned and
// 0 <= k <= N - 1 signed for c_[u]xor_mask (the mask 2^k - 1), and in 0 <= k <= N - 1 signed (the mask -2^k, so k = 0 is `x ^ -1`) and
// 1 <= k <= N - 1 unsigned (the mask 2^N - 2^k; k = 0 and k = N are the masks 2^N - 1 and 0, which are c_uxor_mask_N_N and c_uxor_mask_N_0)
// for c_[u]xor_cmask (decision 0027 item 1: the first row that matches is used).  The bodies are decision 0027 item 1's forms over the parameter `a`, naming it twice (once through `\`
// and once through `%`, with power-of-two literal divisors, so lint_viper.py's rule holds); the one `ensures` is guarded by the range of
// the argument, so the function is total and has no `requires`.
struct XorMaskFn {
  bool complement = false;
  int width = 0;
  int k = 0;
  bool is_unsigned = false;
};

bool parse_xor_mask_fn(const std::string& name, XorMaskFn& f) {
  for (const bool u : {false, true})
    for (const bool c : {false, true}) {
      const std::string prefix = std::string("c_") + (u ? "u" : "") + "xor_" + (c ? "cmask" : "mask") + "_";
      if (name.compare(0, prefix.size(), prefix) != 0) continue;
      const size_t us = name.find('_', prefix.size());
      if (us == std::string::npos) return false;
      const std::string w = name.substr(prefix.size(), us - prefix.size()), k = name.substr(us + 1);
      for (const int width : {32, 64, 128})
        if (w == std::to_string(width) && !k.empty() && k.size() <= 3 && k.find_first_not_of("0123456789") == std::string::npos) {
          f = XorMaskFn{c, width, std::atoi(k.c_str()), u};
          return f.k <= width;
        }
    }
  return false;
}

}  // namespace

std::set<std::string> prelude_closure(const std::set<std::string>& used) {
  std::set<std::string> all = used;
  if (all.count("c_shr") != 0) all.insert("c_pow2");
  // The 128-bit function calls the unsigned 64-bit function and the one of its own signedness.
  std::function<void(const std::string&, int, bool)> add = [&](const std::string& op, int w, bool uns) {
    if (w == 128) {
      add(op, 64, true);
      add(op, 64, uns);
    }
    all.insert(bit_fn_name(op, w, uns));
  };
  for (const std::string& n : used) {
    BitFn f;
    if (parse_bit_fn(n, f)) add(f.op, f.width, f.is_unsigned);
  }
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
  // The bitwise functions, narrowest first (a function is defined before its callers).
  std::vector<BitFn> bit_fns;
  for (const std::string& n : all) {
    BitFn f;
    if (parse_bit_fn(n, f)) bit_fns.push_back(f);
  }
  auto op_rank = [](const std::string& op) { return op == "and" ? 0 : (op == "or" ? 1 : 2); };
  std::sort(bit_fns.begin(), bit_fns.end(), [&](const BitFn& x, const BitFn& y) {
    if (x.width != y.width) return x.width < y.width;
    if (op_rank(x.op) != op_rank(y.op)) return op_rank(x.op) < op_rank(y.op);
    return x.is_unsigned < y.is_unsigned;
  });
  for (const BitFn& f : bit_fns) {
    namespace dec = cerastes::vir::dec;
    const dec::Range r = f.is_unsigned ? dec::Range{"0", dec::dec1(dec::pow2(f.width))}
                                       : dec::Range{dec::neg(dec::pow2(f.width - 1)), dec::dec1(dec::pow2(f.width - 1))};
    fn(bit_fn_name(f.op, f.width, f.is_unsigned) + "(a: Int, b: Int): Int\n  ensures " + (dec::negative(r.lo) ? "(" + r.lo + ")" : r.lo) +
           " <= result && result <= " + r.hi,
       f.width != 128 ? bit_leaf_body(f) : bit_composite_body(f));
  }
  // The `^` constant-mask functions (decision 0027 item 3): independent of the others.
  for (const std::string& n : all) {
    namespace dec = cerastes::vir::dec;
    XorMaskFn f;
    if (!parse_xor_mask_fn(n, f)) continue;
    const dec::Range r = f.is_unsigned ? dec::Range{"0", dec::dec1(dec::pow2(f.width))}
                                       : dec::Range{dec::neg(dec::pow2(f.width - 1)), dec::dec1(dec::pow2(f.width - 1))};
    const std::string lo = dec::negative(r.lo) ? "(" + r.lo + ")" : r.lo;
    const std::string pk = dec::pow2(f.k);
    const std::string high = "(" + pk + " * (a \\ " + pk + "))";   // x with its low k bits cleared
    std::string body;
    if (!f.complement) {
      body = high + " + (" + dec::dec1(pk) + " - (a % " + pk + "))";   // 2^k * H + ((2^k - 1) - L)
    } else if (f.is_unsigned) {
      body = "((a % " + pk + ") + (" + dec::pow2(f.width) + " - " + pk + ")) - " + high;   // (L + (2^N - 2^k)) - 2^k * H
    } else {
      body = "((a % " + pk + ") - " + pk + ") - " + high;   // (L - 2^k) - 2^k * H
    }
    fn(n + "(a: Int): Int\n  ensures (" + lo + " <= a && a <= " + r.hi + ") ==> (" + lo + " <= result && result <= " + r.hi + ")", body);
  }
  return t;
}

}  // namespace cerastes::viper
