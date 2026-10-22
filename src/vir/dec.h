// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Exact decimal arithmetic for the few places VIR needs it: the ranges of integer types up to 128 bits
// (and constants of that size) are strings, never host integers, so no value is ever rounded or wrapped by
// the tool itself.
#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "vir.h"

namespace cerastes::vir::dec {

// A decimal integer: an optional '-', digits without leading zeros ("0" alone), and no "-0".
inline bool valid(const std::string& s) {
  size_t i = (!s.empty() && s[0] == '-') ? 1 : 0;
  if (i >= s.size()) return false;
  if (s[i] == '0') return s.size() == i + 1 && i == 0;
  for (; i < s.size(); ++i)
    if (s[i] < '0' || s[i] > '9') return false;
  return true;
}

inline bool negative(const std::string& s) { return !s.empty() && s[0] == '-'; }

inline std::string neg(const std::string& s) {
  if (s == "0") return s;
  return negative(s) ? s.substr(1) : "-" + s;
}

// 2^n for n >= 0.
inline std::string pow2(int n) {
  std::vector<int> d{1};  // little-endian digits
  for (int i = 0; i < n; ++i) {
    int carry = 0;
    for (int& x : d) {
      int v = x * 2 + carry;
      x = v % 10;
      carry = v / 10;
    }
    if (carry != 0) d.push_back(carry);
  }
  std::string out;
  for (auto it = d.rbegin(); it != d.rend(); ++it) out += static_cast<char>('0' + *it);
  return out;
}

// a - 1 for a positive decimal.
inline std::string dec1(const std::string& a) {
  std::string s = a;
  size_t i = s.size();
  while (i > 0) {
    --i;
    if (s[i] == '0') {
      s[i] = '9';
    } else {
      --s[i];
      break;
    }
  }
  size_t first = s.find_first_not_of('0');
  return first == std::string::npos ? "0" : s.substr(first);
}

// -1, 0 or 1.
inline int cmp(const std::string& a, const std::string& b) {
  const bool na = negative(a), nb = negative(b);
  if (na != nb) return na ? -1 : 1;
  const std::string& ma = na ? a.substr(1) : a;
  const std::string& mb = nb ? b.substr(1) : b;
  int c;
  if (ma.size() != mb.size()) c = ma.size() < mb.size() ? -1 : 1;
  else c = ma < mb ? -1 : (ma > mb ? 1 : 0);
  return na ? -c : c;
}

struct Range {
  std::string lo, hi;
};

// The values of an integer type: [-2^(bits-1), 2^(bits-1)-1] or [0, 2^bits-1]; of an enumeration, its value
// range (decision 0018 item 3).
inline Range range_of(const IntType& t) {
  if (t.is_enum) return Range{t.lo, t.hi};
  if (t.is_signed) return Range{neg(pow2(t.bits - 1)), dec1(pow2(t.bits - 1))};
  return Range{"0", dec1(pow2(t.bits))};
}

inline bool contains(const Range& outer, const Range& inner) {
  return cmp(outer.lo, inner.lo) <= 0 && cmp(inner.hi, outer.hi) <= 0;
}

inline bool in_range(const std::string& v, const Range& r) { return cmp(r.lo, v) <= 0 && cmp(v, r.hi) <= 0; }

}  // namespace cerastes::vir::dec
