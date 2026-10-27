// (included by selftest.cpp) Bitwise operators (decision 0023): `& | ^ ~` on 32-, 64- and 128-bit signed and unsigned computed
// types, exact closed results, symbolic properties the prelude decides (spike S21), compound forms with their checked conversion
// back, closed named constants, the constant evaluator of a `do ... while (0)` condition, and the typing rejections.
// Programs without these nodes are not touched by anything here.
#include "dec.h"

namespace {

using BitU128 = unsigned __int128;

BitU128 bit_mask(int bits) { return bits == 128 ? ~BitU128(0) : ((BitU128(1) << bits) - 1); }

std::string bit_dec_unsigned(BitU128 v) {
  if (v == 0) return "0";
  std::string s;
  while (v != 0) {
    s.insert(s.begin(), static_cast<char>('0' + static_cast<int>(v % 10)));
    v /= 10;
  }
  return s;
}

// The decimal text of the `bits`-bit pattern `p` read as a signed (two's complement) or unsigned value.
std::string bit_text(BitU128 p, int bits, bool is_signed) {
  p &= bit_mask(bits);
  if (is_signed && ((p >> (bits - 1)) & 1) != 0) return "-" + bit_dec_unsigned((~p + 1) & bit_mask(bits));
  return bit_dec_unsigned(p);
}

BitU128 bit_apply(BinaryOp op, BitU128 a, BitU128 b) {
  return op == BinaryOp::BitAnd ? (a & b) : (op == BinaryOp::BitOr ? (a | b) : (a ^ b));
}

const char* bit_op_name(BinaryOp op) { return op == BinaryOp::BitAnd ? "and" : (op == BinaryOp::BitOr ? "or" : "xor"); }
const char* bit_op_sym(BinaryOp op) { return op == BinaryOp::BitAnd ? "&" : (op == BinaryOp::BitOr ? "|" : "^"); }

Type bit_type(Kit& k, int bits, bool is_signed) {
  if (bits == 32) return is_signed ? k.i32 : k.u32;
  if (bits == 64) return is_signed ? k.i64 : k.u64;
  return is_signed ? k.i128 : k.u128;
}

// ---- constant masks as remainders (decision 0026, part B) ----
// Lowering writes `x & (2^k - 1)` as `x % 2^k` (Viper's, Euclidean) and `x & ~(2^k - 1)` as `2^k * (x \ 2^k)` when one operand is such a
// constant (a closed expression of literals as lowered, `~` included, on either side, and `&=`).  PRE-REGISTERED: each claim below is an identity that holds for every operand in the
// type's range, stated in C's terms (C's `%` truncates, so a claim about a negative operand is stated in closed form or by cases); each
// `_wrong` twin states a false claim that Z3 refutes (never an identity that needs the bits of a variable mask).  The cases do not mention
// how the mask is lowered: they pass with the prelude function or with the remainder.
void define_bitwise_masks() {
  const int widths[] = {32, 64, 128};
  int variant_counter = 0;

  // ---- symbolic operand, k at its extremes (0, 1, N-1, and N for an unsigned type): mask and complement, five syntactic forms ----
  for (const int bits : widths) {
    for (const bool sg : {true, false}) {
      for (const bool comp : {false, true}) {
        std::vector<int> ks = {0, 1, bits - 1};
        if (!sg) ks.push_back(bits);
        for (const int kk : ks) {
          // 0: `x & lit`, 1: `lit & x`, 2: `y = x; y &= lit`, 3: `x & ~alt`, 4: `~alt & x` (alt is the complementary pattern)
          const int variant = variant_counter++ % 5;
          auto build = [=](Kit& k, bool wrong) -> SrcLoc {
            const Type t = bit_type(k, bits, sg);
            const dec::Range rg = dec::range_of(*t.int_type);
            const BitU128 M = bit_mask(bits);
            const BitU128 low = kk == 128 ? M : ((BitU128(1) << kk) - 1);  // 2^kk - 1
            const BitU128 high = (~low) & M;                               // ~(2^kk - 1): -2^kk (signed) or 2^N - 2^kk (unsigned)
            const std::string lowT = bit_text(low, bits, sg), highT = bit_text(high, bits, sg);
            const std::string mT = comp ? highT : lowT, altT = comp ? lowT : highT;
            Function* f = k.fn("f", t, {{"x", t}});
            auto N = [&](const std::string& v) { return k.num(t, v); };
            auto X = [&]() { return k.F(f, 0); };
            auto R = [&]() { return k.R(f); };
            auto pow2 = [&](int e) { return N(bit_text(BitU128(1) << e, bits, sg)); };
            const bool z = kk == 0, full = kk == bits;
            if (!sg) {
              if (!comp) {
                if (full) k.ens(f, wrong ? k.eq_c(R(), t, "0") : k.eq(R(), X()));                       // x & MAX == x (x % 2^N)
                else k.ens(f, wrong ? k.eq(R(), X()) : k.eq(R(), k.rem(X(), pow2(kk))));                // x & (2^k-1) == x % 2^k
              } else {
                if (full) k.ens(f, wrong ? k.eq(R(), X()) : k.eq_c(R(), t, "0"));                       // x & 0 == 0
                else if (wrong) k.ens(f, z ? k.eq_c(R(), t, "0") : k.eq(R(), X()));                     // (z: x & MAX == x)
                else k.ens(f, k.eq(R(), k.sub(X(), k.rem(X(), pow2(kk)))));                             // x - x % 2^k
              }
            } else if (!comp) {
              if (z) {
                k.ens(f, k.eq_c(R(), t, wrong ? "1" : "0"));                                             // x & 0 == 0
              } else if (kk == 1) {
                if (wrong) {
                  k.ens(f, k.eq(R(), X()));
                } else {
                  k.ens(f, k.range(R(), t, "0", "1"));
                  k.ens(f, k.implies(k.ge_c(X(), t, "0"), k.eq(R(), k.rem(X(), N("2")))));
                }
              } else {  // kk == bits - 1: x & INT_MAX clears the sign bit
                if (wrong) {
                  k.ens(f, k.eq(R(), X()));
                } else {
                  k.ens(f, k.ge_c(R(), t, "0"));
                  k.ens(f, k.implies(k.ge_c(X(), t, "0"), k.eq(R(), X())));
                  k.ens(f, k.implies(k.lt(X(), N("0")), k.eq(R(), k.add(k.add(X(), N(rg.hi)), N("1")))));
                }
              }
            } else {
              if (z) {
                k.ens(f, wrong ? k.eq_c(R(), t, "0") : k.eq(R(), X()));                                  // x & -1 == x
              } else if (kk == 1) {
                if (wrong) {
                  k.ens(f, k.eq(R(), X()));
                } else {
                  k.ens(f, k.implies(k.ge_c(X(), t, "0"), k.eq(R(), k.sub(X(), k.rem(X(), N("2"))))));
                  k.ens(f, k.le(R(), X()));
                }
              } else {  // kk == bits - 1: x & INT_MIN keeps only the sign bit
                k.ens(f, wrong ? k.eq_c(R(), t, "0") : k.eq(R(), k.cond(k.lt(X(), N("0")), N(rg.lo), N("0"))));
              }
            }
            std::vector<const Stmt*> body;
            const Stmt* r = nullptr;
            switch (variant) {
              case 0: r = k.ret(k.band(k.pv(f, 0), N(mT))); break;
              case 1: r = k.ret(k.band(N(mT), k.pv(f, 0))); break;
              case 2: {
                Var* y = k.local(f, "y", t);
                body.push_back(k.decl(y, k.pv(f, 0)));
                body.push_back(k.b.compound_assign(y, BinaryOp::BitAnd, N(mT), k.at()));
                r = k.ret(k.v(y));
                break;
              }
              case 3: r = k.ret(k.band(k.pv(f, 0), k.bnot(N(altT)))); break;
              default: r = k.ret(k.band(k.bnot(N(altT)), k.pv(f, 0))); break;
            }
            body.push_back(r);
            f->body = k.blk(body);
            k.done(f);
            return r->loc;
          };
          const std::string name = std::string("bit_mask_") + (comp ? "cmpl" : "and") + (sg ? "_s" : "_u") + std::to_string(bits) + "_k" +
                                   std::to_string(kk) + "_v" + std::to_string(variant);
          const std::string doc = std::string("x ") + (comp ? "& ~(2^k-1)" : "& (2^k-1)") + " with k = " + std::to_string(kk) + " on a symbolic " +
                                  (sg ? "signed " : "unsigned ") + std::to_string(bits) + "-bit x (form " + std::to_string(variant) + "): the identity holds";
          lc(name, doc, [=](Kit& k) { build(k, false); });
          lc(name + "_wrong", doc + " (a false claim, refuted by Z3)", [=](Kit& k) {
            const SrcLoc l = build(k, true);
            k.fail("postcondition", l);
          });
        }
      }
    }
  }

  // ---- near misses stay on the prelude function and are decided on closed operands: 254 and 256 are not masks ----
  for (const int bits : widths) {
    for (const bool sg : {true, false}) {
      auto build = [=](Kit& k, bool wrong) -> SrcLoc {
        const Type t = bit_type(k, bits, sg);
        auto N = [&](const std::string& v) { return k.num(t, v); };
        const std::string X = sg ? "-1000" : "1000";  // 0x...FC18 or 0x3E8
        Function* f = k.fn("f", k.vd);
        const Stmt* c1 = k.chk(k.eq(k.band(N(X), N("254")), N(sg ? "24" : "232")));
        const Stmt* c2 = k.chk(k.eq(k.band(N("256"), N(X)), N(wrong ? "1" : (sg ? "0" : "256"))));
        f->body = k.blk({c1, c2});
        k.done(f);
        return c2->loc;
      };
      const std::string name = std::string("bit_mask_near_miss_") + (sg ? "s" : "u") + std::to_string(bits);
      const std::string doc = std::string(sg ? "-1000" : "1000") + " & 254 and 256 & " + (sg ? "-1000" : "1000") + " on " + (sg ? "signed " : "unsigned ") +
                              std::to_string(bits) + " bits: closed operands, 254 and 256 are not masks";
      lc(name, doc, [=](Kit& k) { build(k, false); });
      lc(name + "_wrong", doc + " (the last claim is false)", [=](Kit& k) {
        const SrcLoc l = build(k, true);
        k.fail("check", l);
      });
    }
  }

  // ---- closed negative (signed) and high (unsigned) operands: the remainder is Euclidean, never C's ----
  for (const int bits : widths) {
    for (const bool sg : {true, false}) {
      auto build = [=](Kit& k, bool wrong) -> SrcLoc {
        const Type t = bit_type(k, bits, sg);
        auto N = [&](const std::string& v) { return k.num(t, v); };
        const BitU128 M = bit_mask(bits);
        const BitU128 maxs = (BitU128(1) << (bits - 1)) - 1;
        const std::string X = sg ? "-5" : bit_text(M - 4, bits, false);  // -5 or 2^N - 5
        Function* f = k.fn("f", k.vd);
        const Stmt* c1 = k.chk(k.eq(k.band(N(X), N("255")), N("251")));
        const Stmt* c2 = k.chk(k.eq(k.band(k.bnot(N("15")), N(X)), N(sg ? "-16" : bit_text(M - 15, bits, false))));
        // x & MAX: signed -5 & INT_MAX == INT_MAX - 4 (the sign bit cleared); unsigned (2^N - 5) & (2^N - 1) is itself
        const Stmt* c3 = sg ? k.chk(k.eq(k.band(N(X), N(bit_text(maxs, bits, true))), N(bit_text(maxs - (wrong ? 3 : 4), bits, true))))
                            : k.chk(k.eq(k.band(N(X), N(bit_text(M, bits, false))), N(bit_text(wrong ? M - 5 : M - 4, bits, false))));
        f->body = k.blk({c1, c2, c3});
        k.done(f);
        return c3->loc;
      };
      const std::string name = std::string("bit_mask_closed_") + (sg ? "negative_s" : "high_u") + std::to_string(bits);
      const std::string doc = std::string(sg ? "-5" : "2^N - 5") + " & 255, & ~15 and & the type's maximum on " + (sg ? "signed " : "unsigned ") +
                              std::to_string(bits) + " bits: 251, the complement result and the maximum minus 4 (or itself)";
      lc(name, doc, [=](Kit& k) { build(k, false); });
      lc(name + "_wrong", doc + " (the last claim is off by one)", [=](Kit& k) {
        const SrcLoc l = build(k, true);
        k.fail("check", l);
      });
    }
  }

  // ---- the identities of decision 0026 item 9 ----
  // v & 0xFFFFFFFF == v % 4294967296 for an unsigned 64-bit v (and 2^64 for 128 bits); C's % agrees with the Euclidean one on non-negatives.
  for (const int bits : {64, 128}) {
    const int half = bits / 2;
    auto build = [=](Kit& k, bool wrong) -> SrcLoc {
      const Type t = bit_type(k, bits, false);
      const BitU128 halfmask = (BitU128(1) << half) - 1;
      Function* f = k.fn("f", t, {{"v", t}});
      // the wrong claim divides by 2^half - 1: v == 2^half - 1 gives remainder 0 on the right and 2^half - 1 on the left
      k.ens(f, k.eq(k.R(f), k.rem(k.F(f, 0), k.num(t, bit_text(wrong ? halfmask : halfmask + 1, bits, false)))));
      const Stmt* r = k.ret(k.band(k.pv(f, 0), k.num(t, bit_text(halfmask, bits, false))));
      f->body = k.blk({r});
      k.done(f);
      return r->loc;
    };
    const std::string name = "bit_mask_half_word_u" + std::to_string(bits);
    const std::string doc = "v & (2^" + std::to_string(half) + " - 1) == v % 2^" + std::to_string(half) + " for an unsigned " + std::to_string(bits) + "-bit v";
    lc(name, doc, [=](Kit& k) { build(k, false); });
    lc(name + "_wrong", doc + " (the claim divides by 2^" + std::to_string(half) + " - 1)", [=](Kit& k) {
      const SrcLoc l = build(k, true);
      k.fail("postcondition", l);
    });
  }
  // x & ~15 == x - x % 16 for x >= 0.
  for (const int bits : {32, 64}) {
    auto build = [=](Kit& k, bool wrong) -> SrcLoc {
      const Type t = bit_type(k, bits, true);
      Function* f = k.fn("f", t, {{"x", t}});
      k.req(f, k.ge_c(k.F(f, 0), t, "0"));
      k.ens(f, k.eq(k.R(f), k.sub(k.F(f, 0), k.rem(k.F(f, 0), k.num(t, wrong ? "15" : "16")))));
      const Stmt* r = k.ret(k.band(k.pv(f, 0), k.bnot(k.num(t, "15"))));
      f->body = k.blk({r});
      k.done(f);
      return r->loc;
    };
    const std::string name = "bit_mask_clear_low_s" + std::to_string(bits);
    const std::string doc = "x & ~15 == x - x % 16 for 0 <= x on signed " + std::to_string(bits) + " bits";
    lc(name, doc, [=](Kit& k) { build(k, false); });
    lc(name + "_wrong", doc + " (the claim uses x % 15, false for x = 15)", [=](Kit& k) {
      const SrcLoc l = build(k, true);
      k.fail("postcondition", l);
    });
  }
  // x & 255 for every x: ((x % 256) + 256) % 256 in C's terms; the twin drops the correction (false for x = -1).
  for (const int bits : {32, 64}) {
    auto build = [=](Kit& k, bool wrong) -> SrcLoc {
      const Type t = bit_type(k, bits, true);
      Function* f = k.fn("f", t, {{"x", t}});
      const Expr* m = k.rem(k.F(f, 0), k.num(t, "256"));
      k.ens(f, k.eq(k.R(f), wrong ? m : k.rem(k.add(m, k.num(t, "256")), k.num(t, "256"))));
      const Stmt* r = k.ret(k.band(k.pv(f, 0), k.num(t, "255")));
      f->body = k.blk({r});
      k.done(f);
      return r->loc;
    };
    const std::string name = "bit_mask_low_byte_s" + std::to_string(bits);
    const std::string doc = "x & 255 == ((x % 256) + 256) % 256 for every signed " + std::to_string(bits) + "-bit x, negative included";
    lc(name, doc, [=](Kit& k) { build(k, false); });
    lc(name + "_wrong", doc + " (the claim is x % 256, false for x = -1)", [=](Kit& k) {
      const SrcLoc l = build(k, true);
      k.fail("postcondition", l);
    });
  }
  // The cross-type pin: `(unsigned long)(~0u) & v` is the constant 4294967295 in the computed type unsigned long, so it keeps the low 32 bits of
  // an unsigned 64-bit v; the conversion is of ~0u in its own type (0023 item 1), never a 64-bit ~0.  The twin claims `r == v`, false for v = 2^32.
  for (const bool wrong : {false, true}) {
    lc(std::string("bit_mask_cross_type_u32_to_u64") + (wrong ? "_wrong" : ""),
       std::string("(unsigned long)(~0u) & v == ") + (wrong ? "v (wrong)" : "v % 4294967296") + " for an unsigned 64-bit v", [=](Kit& k) {
         Function* f = k.fn("f", k.u64, {{"v", k.u64}});
         k.ens(f, wrong ? k.eq(k.R(f), k.F(f, 0)) : k.eq(k.R(f), k.rem(k.F(f, 0), k.num(k.u64, "4294967296"))));
         const Stmt* r = k.ret(k.band(k.cv(k.u64, k.bnot(k.num(k.u32, "0"))), k.pv(f, 0)));
         f->body = k.blk({r});
         k.done(f);
         if (wrong) k.fail("postcondition", r->loc);
       });
  }
  // The range check of a masked result: (unsigned)(v & 4294967295u) fits, (unsigned)(v & 8589934591u) may not.
  for (const bool wrong : {false, true}) {
    lc(std::string("bit_mask_converts_back_u64") + (wrong ? "_wrong" : ""),
       std::string("(unsigned)(v & ") + (wrong ? "8589934591" : "4294967295") + ") for an unsigned 64-bit v: the masked result " +
           (wrong ? "may not fit unsigned int" : "always fits unsigned int"),
       [=](Kit& k) {
         Function* f = k.fn("f", k.u32, {{"v", k.u64}});
         const Expr* back = k.cv(k.u32, k.band(k.pv(f, 0), k.num(k.u64, wrong ? "8589934591" : "4294967295")));
         f->body = k.blk({k.ret(back)});
         k.done(f);
         if (wrong) k.fail("conversion", back->loc);
       });
  }
}

void define_bitwise() {
  const BinaryOp ops[] = {BinaryOp::BitAnd, BinaryOp::BitOr, BinaryOp::BitXor};
  const int widths[] = {32, 64, 128};

  // ---- exact closed results: five operand pairs per operator, type and width (negative operands, results equal to the type's
  // minimum, the alternating patterns, the all-ones pattern) ----
  for (const int bits : widths) {
    for (const bool sg : {true, false}) {
      for (const BinaryOp op : ops) {
        const std::string name = std::string("bit_") + bit_op_name(op) + (sg ? "_s" : "_u") + std::to_string(bits);
        const std::string doc = std::string("a ") + bit_op_sym(op) + " b on " + (sg ? "signed " : "unsigned ") + std::to_string(bits) +
                                " bits: five closed pairs, every result exact";
        auto build = [=](Kit& k, bool wrong) -> SrcLoc {
          const Type t = bit_type(k, bits, sg);
          const BitU128 m = bit_mask(bits);
          const BitU128 top = BitU128(1) << (bits - 1);
          BitU128 alt5 = 0, alt3 = 0;
          for (int i = 0; i < bits; i += 4) {
            alt5 |= BitU128(0x5) << i;
            alt3 |= BitU128(0x3) << i;
          }
          const std::pair<BitU128, BitU128> pairs[] = {{0xF0, 0x3C}, {m - 7, 13}, {top, m}, {m, top - 1}, {alt5, alt3}};
          Function* f = k.fn("f", k.vd);
          std::vector<const Stmt*> body;
          SrcLoc last;
          for (size_t i = 0; i < 5; ++i) {
            BitU128 want = bit_apply(op, pairs[i].first, pairs[i].second);
            if (wrong && i == 4) want ^= 1;
            const Expr* e = k.bin(op, k.num(t, bit_text(pairs[i].first, bits, sg)), k.num(t, bit_text(pairs[i].second, bits, sg)));
            const Stmt* c = k.chk(k.eq(e, k.num(t, bit_text(want, bits, sg))));
            last = c->loc;
            body.push_back(c);
          }
          f->body = k.blk(body);
          k.done(f);
          return last;
        };
        lc(name, doc, [=](Kit& k) { build(k, false); });
        lc(name + "_wrong", doc + " (the last pair claims a result that is off by one bit)", [=](Kit& k) {
          const SrcLoc l = build(k, true);
          k.fail("check", l);
        });
      }
    }
  }

  // ---- ~ : inline and linear, never through unary minus ----
  for (const int bits : widths) {
    for (const bool sg : {true, false}) {
      const std::string name = std::string("bit_not_") + (sg ? "s" : "u") + std::to_string(bits);
      const std::string doc = std::string("~a on ") + (sg ? "signed " : "unsigned ") + std::to_string(bits) + " bits: the type's minimum, maximum, 0 and 1";
      auto build = [=](Kit& k, bool wrong) -> SrcLoc {
        const Type t = bit_type(k, bits, sg);
        const BitU128 m = bit_mask(bits);
        const BitU128 top = BitU128(1) << (bits - 1);
        const BitU128 ins[] = {0, 1, m, top, top - 1};
        Function* f = k.fn("f", k.vd);
        std::vector<const Stmt*> body;
        SrcLoc last;
        for (size_t i = 0; i < 5; ++i) {
          BitU128 want = (~ins[i]) & m;
          if (wrong && i == 4) want = (want + 1) & m;
          const Stmt* c = k.chk(k.eq(k.bnot(k.num(t, bit_text(ins[i], bits, sg))), k.num(t, bit_text(want, bits, sg))));
          last = c->loc;
          body.push_back(c);
        }
        f->body = k.blk(body);
        k.done(f);
        return last;
      };
      lc(name, doc, [=](Kit& k) { build(k, false); });
      lc(name + "_wrong", doc + " (the last claim is off by one)", [=](Kit& k) {
        const SrcLoc l = build(k, true);
        k.fail("check", l);
      });
    }
  }
  lc("bit_not_has_no_obligation", "~INT_MIN on a parameter bounded to INT_MIN: no overflow, unlike -INT_MIN", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    k.req(f, k.eq_c(k.F(f, 0), k.i32, "-2147483648"));
    k.ens(f, k.eq_c(k.R(f), k.i32, "2147483647"));
    f->body = k.blk({k.ret(k.bnot(k.pv(f, 0)))});
    k.done(f);
  });
  lc("bit_not_has_no_obligation_neg_fails", "-INT_MIN on the same parameter is the overflow that ~ does not have", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    k.req(f, k.eq_c(k.F(f, 0), k.i32, "-2147483648"));
    const Expr* n = k.neg(k.pv(f, 0));
    f->body = k.blk({k.ret(n)});
    k.done(f);
    k.fail("overflow", n->loc);
  });

  // ---- symbolic operands: what the prelude's functions let Z3 decide (spike S21) ----
  for (const int bits : widths) {
    const std::string w = std::to_string(bits);
    for (const bool wrong : {false, true}) {
      const std::string sfx = wrong ? "_wrong" : "";
      // xor(x, x) == 0
      lc("bit_sym_xor_self_s" + w + sfx, "x ^ x == " + std::string(wrong ? "1 (wrong)" : "0") + " on a symbolic signed " + w + "-bit x", [=](Kit& k) {
        const Type t = bit_type(k, bits, true);
        Function* f = k.fn("f", t, {{"x", t}});
        k.ens(f, k.eq_c(k.R(f), t, wrong ? "1" : "0"));
        const Stmt* r = k.ret(k.bxor(k.pv(f, 0), k.pv(f, 0)));
        f->body = k.blk({r});
        k.done(f);
        if (wrong) k.fail("postcondition", r->loc);
      });
      // (x & y) == (y & x) on unsigned values; the twin claims (x & y) >= 0 on signed ones, false for x = y = -1
      lc(std::string(wrong ? "bit_sym_and_nonneg_s" : "bit_sym_and_comm_u") + w + sfx,
         wrong ? "(x & y) >= 0 on a symbolic signed " + w + "-bit pair: false for x = y = -1"
               : "(x & y) == (y & x) on a symbolic unsigned " + w + "-bit pair",
         [=](Kit& k) {
           const Type t = bit_type(k, bits, wrong);
           Function* f = k.fn("f", k.vd, {{"x", t}, {"y", t}});
           const Stmt* c = wrong ? k.chk(k.ge_c(k.band(k.pv(f, 0), k.pv(f, 1)), t, "0"))
                                 : k.chk(k.eq(k.band(k.pv(f, 0), k.pv(f, 1)), k.band(k.pv(f, 1), k.pv(f, 0))));
           f->body = k.blk({c});
           k.done(f);
           if (wrong) k.fail("check", c->loc);
         });
    }
    // ten operations on symbolic values stay in the type's range (no obligation was needed)
    lc("bit_sym_ten_ops_s" + w, std::string("ten symbolic bitwise operations on signed ") + w + "-bit values: the result is in range", [=](Kit& k) {
      const Type t = bit_type(k, bits, true);
      const dec::Range r = dec::range_of(*t.int_type);
      Function* f = k.fn("f", k.vd, {{"x", t}, {"y", t}, {"z", t}});
      Var* tv[10];
      for (int i = 0; i < 10; ++i) tv[i] = k.local(f, "t" + std::to_string(i + 1), t);
      std::vector<const Stmt*> body = {
          k.decl(tv[0], k.band(k.pv(f, 0), k.pv(f, 1))), k.decl(tv[1], k.bor(k.pv(f, 0), k.pv(f, 1))),
          k.decl(tv[2], k.bxor(k.pv(f, 0), k.pv(f, 1))), k.decl(tv[3], k.band(k.v(tv[0]), k.v(tv[1]))),
          k.decl(tv[4], k.bor(k.v(tv[2]), k.v(tv[3]))),  k.decl(tv[5], k.bxor(k.v(tv[4]), k.pv(f, 2))),
          k.decl(tv[6], k.band(k.v(tv[5]), k.pv(f, 0))), k.decl(tv[7], k.bor(k.v(tv[6]), k.pv(f, 1))),
          k.decl(tv[8], k.bxor(k.v(tv[7]), k.pv(f, 2))), k.decl(tv[9], k.band(k.v(tv[8]), k.v(tv[8])))};
      body.push_back(k.chk(k.range(k.v(tv[9]), t, r.lo, r.hi)));
      f->body = k.blk(body);
      k.done(f);
    });
    lc("bit_sym_ten_ops_s" + w + "_wrong", "two symbolic operations, then check(t2 != 0): false for x = y = 0", [=](Kit& k) {
      const Type t = bit_type(k, bits, true);
      Function* f = k.fn("f", k.vd, {{"x", t}, {"y", t}});
      Var* t1 = k.local(f, "t1", t);
      Var* t2 = k.local(f, "t2", t);
      const Stmt* c = k.chk(k.ne(k.v(t2), k.num(t, "0")));
      f->body = k.blk({k.decl(t1, k.bor(k.pv(f, 0), k.pv(f, 1))), k.decl(t2, k.band(k.v(t1), k.pv(f, 0))), c});
      k.done(f);
      k.fail("check", c->loc);
    });
  }
  for (const int bits : {32, 64}) {
    const std::string w = std::to_string(bits);
    for (const bool wrong : {false, true}) {
      // x >= 0 ==> (x & 1) == x % 2
      lc("bit_sym_low_bit_s" + w + (wrong ? "_wrong" : ""), "x & 1 == x % 2 for 0 <= x on signed " + w + " bits" + (wrong ? " (claims == 1)" : ""), [=](Kit& k) {
        const Type t = bit_type(k, bits, true);
        Function* f = k.fn("f", t, {{"x", t}});
        k.req(f, k.ge_c(k.F(f, 0), t, "0"));
        k.ens(f, wrong ? k.eq_c(k.R(f), t, "1") : k.eq(k.R(f), k.rem(k.F(f, 0), k.num(t, "2"))));
        const Stmt* r = k.ret(k.band(k.pv(f, 0), k.num(t, "1")));
        f->body = k.blk({r});
        k.done(f);
        if (wrong) k.fail("postcondition", r->loc);
      });
    }
  }
  for (const bool wrong : {false, true}) {
    // a mask keeps an unsigned value inside the mask
    lc(std::string("bit_sym_mask_u32") + (wrong ? "_wrong" : ""), std::string("u & 255 <= ") + (wrong ? "254 (wrong)" : "255") + " for a symbolic unsigned int", [=](Kit& k) {
      Function* f = k.fn("f", k.u32, {{"u", k.u32}});
      k.ens(f, k.le_c(k.R(f), k.u32, wrong ? "254" : "255"));
      const Stmt* r = k.ret(k.band(k.pv(f, 0), k.num(k.u32, "255")));
      f->body = k.blk({r});
      k.done(f);
      if (wrong) k.fail("postcondition", r->loc);
    });
    // testing a flag with a constant mask (`(u | 4) & 4 == 4` is a relational claim that Z3 does not decide, see S21 and its follow-up)
    lc(std::string("bit_sym_flag_u32") + (wrong ? "_wrong" : ""), std::string("u & 4 <= ") + (wrong ? "3 (wrong)" : "4") + " for a symbolic unsigned int", [=](Kit& k) {
      Function* f = k.fn("f", k.u32, {{"u", k.u32}});
      k.ens(f, k.le_c(k.R(f), k.u32, wrong ? "3" : "4"));
      const Stmt* r = k.ret(k.band(k.pv(f, 0), k.num(k.u32, "4")));
      f->body = k.blk({r});
      k.done(f);
      if (wrong) k.fail("postcondition", r->loc);
    });
  }

  // ---- compound assignment: computed in the usual arithmetic conversions, converted back with a check (0023 item 3) ----
  auto ca = [](Kit& k, Type t, const char* init, BinaryOp op, Type rt, const char* rhs, const char* want, bool wrong, bool does_not_fit) {
    Function* f = k.fn("f", k.vd);
    Var* c = k.local(f, "c", t);
    const SrcLoc l = k.at();
    std::vector<const Stmt*> body = {k.decl(c, k.num(t, init)), k.b.compound_assign(c, op, k.num(rt, rhs), l)};
    const Stmt* ck = nullptr;
    if (want != nullptr) {
      ck = k.chk(k.eq_c(k.v(c), t, wrong ? "1" : want));
      body.push_back(ck);
    }
    f->body = k.blk(body);
    k.done(f);
    if (does_not_fit) k.fail("conversion", l);
    if (wrong) k.fail("check", ck->loc);
  };
  lc("bit_ca_and_u8", "unsigned char c = 200; c &= 15;  c == 8", [=](Kit& k) { ca(k, k.u8, "200", BinaryOp::BitAnd, k.i32, "15", "8", false, false); });
  lc("bit_ca_and_u8_wrong", "the same, then check(c == 1)", [=](Kit& k) { ca(k, k.u8, "200", BinaryOp::BitAnd, k.i32, "15", "8", true, false); });
  lc("bit_ca_xor_u8", "unsigned char c = 255; c ^= 15;  c == 240", [=](Kit& k) { ca(k, k.u8, "255", BinaryOp::BitXor, k.i32, "15", "240", false, false); });
  lc("bit_ca_xor_u8_wrong", "the same, then check(c == 1)", [=](Kit& k) { ca(k, k.u8, "255", BinaryOp::BitXor, k.i32, "15", "240", true, false); });
  lc("bit_ca_or_i8_negative", "signed char c = -3; c |= 128: the int result -3 fits (bit 7 is already set)", [=](Kit& k) {
    ca(k, k.i8, "-3", BinaryOp::BitOr, k.i32, "128", "-3", false, false);
  });
  lc("bit_ca_or_i8_negative_wrong", "the same, then check(c == 1)", [=](Kit& k) { ca(k, k.i8, "-3", BinaryOp::BitOr, k.i32, "128", "-3", true, false); });
  lc("bit_ca_or_i8_does_not_fit", "signed char c = 3; c |= 128: 131 does not fit signed char (conversion, never a wrap)", [=](Kit& k) {
    ca(k, k.i8, "3", BinaryOp::BitOr, k.i32, "128", nullptr, false, true);
  });
  lc("bit_ca_or_i8_unbounded", "signed char c; c |= 128 on a parameter: the result may be 128..255", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"c", k.i8}});
    const SrcLoc l = k.at();
    f->body = k.blk({k.b.compound_assign(f->param_var(0), BinaryOp::BitOr, k.num(k.i32, "128"), l)});
    k.done(f);
    k.fail("conversion", l);
  });
  lc("bit_ca_and_u32_mixed_negative", "unsigned u = 12; u &= -8 as `u & (unsigned)-8`: the int -8 converts to unsigned int, which does not fit", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* u = k.local(f, "u", k.u32);
    const SrcLoc l = k.at();
    f->body = k.blk({k.decl(u, k.num(k.u32, "12")), k.b.compound_assign(u, BinaryOp::BitAnd, k.num(k.i32, "-8"), l)});
    k.done(f);
    k.fail("conversion", l);
  });
  lc("bit_ca_and_u32_complement_literal", "unsigned u = 12; u &= ~7u (4294967288): the unsigned complement fits, u == 8", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* u = k.local(f, "u", k.u32);
    const Stmt* c = k.chk(k.eq_c(k.v(u), k.u32, "8"));
    f->body = k.blk({k.decl(u, k.num(k.u32, "12")), k.b.compound_assign(u, BinaryOp::BitAnd, k.bnot(k.num(k.u32, "7")), k.at()), c});
    k.done(f);
  });
  lc("bit_assign_not_u8", "uc = ~uc on unsigned char: ~(int)uc is negative, so the conversion back fails", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"uc", k.u8}});
    const Expr* back = k.cv(k.u8, k.bnot(k.cv(k.i32, k.pv(f, 0))));
    f->body = k.blk({k.asg(f->param_var(0), back)});
    k.done(f);
    k.fail("conversion", back->loc);
  });
  lc("bit_assign_not_u8_masked", "uc = ~uc & 255 on unsigned char: the int result is in [0, 255], so the conversion back holds", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"uc", k.u8}});
    const Expr* back = k.cv(k.u8, k.band(k.bnot(k.cv(k.i32, k.pv(f, 0))), k.num(k.i32, "255")));
    f->body = k.blk({k.asg(f->param_var(0), back)});
    k.done(f);
  });
  lc("bit_assign_not_u8_masked_wrong", "uc = ~uc & 511: the result may be 256..511, which does not fit unsigned char", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"uc", k.u8}});
    const Expr* back = k.cv(k.u8, k.band(k.bnot(k.cv(k.i32, k.pv(f, 0))), k.num(k.i32, "511")));
    f->body = k.blk({k.asg(f->param_var(0), back)});
    k.done(f);
    k.fail("conversion", back->loc);
  });

  // ---- closed named constants and the prologue (0017 item 4: decided by evaluation) ----
  lc("bit_nc_or", "const unsigned K = 240 | 15 (255): the closed initializer is decided, the guard holds", [](Kit& k) {
    const Expr* init = k.bor(k.num(k.u32, "240"), k.num(k.u32, "15"));
    k.nc("K", k.u32, init, "255");
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.require_map("internal", "the value of `K` as written differs from the front end's");
  });
  lc("bit_nc_or_value_differs", "const unsigned K = 240 | 15 with the front end's value 254: the internal guard fails", [](Kit& k) {
    NamedConst* c = k.nc("K", k.u32, k.bor(k.num(k.u32, "240"), k.num(k.u32, "15")), "254");
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.fail("internal", c->loc, 1, "", "the value of `K` as written");
  });
  lc("bit_nc_or_negative_fails", "const unsigned K = 240 | -1 (the int -1 converts to unsigned): one conversion error at the initializer", [](Kit& k) {
    const Expr* init = k.cv(k.u32, k.bor(k.num(k.i32, "240"), k.num(k.i32, "-1")));
    k.nc("K", k.u32, init, "4294967295");
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.fail("conversion", init->loc, 1, "", "the value may not fit in `unsigned int`");
  });
  lc("bit_nc_not_zero", "const int N = ~0 (-1)", [](Kit& k) {
    k.nc("N", k.i32, k.bnot(k.num(k.i32, "0")), "-1");
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.require_map("internal", "the value of `N` as written differs from the front end's");
  });
  lc("bit_nc_not_zero_value_differs", "const int N = ~0 with the front end's value 0: the internal guard fails", [](Kit& k) {
    NamedConst* c = k.nc("N", k.i32, k.bnot(k.num(k.i32, "0")), "0");
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.fail("internal", c->loc, 1, "", "the value of `N` as written");
  });
  lc("bit_nc_nested", "const int Q = ((255 ^ 15) & 240) | (3 & (1 | 2)) (243): nested closed calls, decided in the prologue", [](Kit& k) {
    auto I = [&](const char* v) { return k.num(k.i32, v); };
    const Expr* init = k.bor(k.band(k.bxor(I("255"), I("15")), I("240")), k.band(I("3"), k.bor(I("1"), I("2"))));
    k.nc("Q", k.i32, init, "243");
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.require_map("internal", "the value of `Q` as written differs from the front end's");
  });
  lc("bit_nc_nested_wrong", "the same with the front end's value 242: the guard fails", [](Kit& k) {
    auto I = [&](const char* v) { return k.num(k.i32, v); };
    const Expr* init = k.bor(k.band(k.bxor(I("255"), I("15")), I("240")), k.band(I("3"), k.bor(I("1"), I("2"))));
    NamedConst* c = k.nc("Q", k.i32, init, "242");
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.fail("internal", c->loc, 1, "", "the value of `Q` as written");
  });
  lc("bit_nc_flags_union", "const int AB = 1 | 2 (3), the shape of an enumerator `AB = A | B`: the closed initializer is decided", [](Kit& k) {
    k.nc("AB", k.i32, k.bor(k.num(k.i32, "1"), k.num(k.i32, "2")), "3");
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.require_map("internal", "the value of `AB` as written differs from the front end's");
  });
  lc("bit_case_value", "case 0x80 | 1: the case value's closed `|` is checked in the prologue and dispatched on EDG's value 129", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    const int id = k.jid();
    const Stmt* c1 = k.cs(k.bor(k.num(k.i32, "128"), k.num(k.i32, "1")), "129");
    const Stmt* swt = k.sw(k.pv(f, 0), k.blk({c1, k.ret(k.num(k.i32, "1")), k.dflt(), k.ret(k.num(k.i32, "0"))}), id);
    f->body = k.blk({swt});
    k.ens(f, k.eq(k.R(f), k.cond(k.eq_c(k.F(f, 0), k.i32, "129"), k.num(k.i32, "1"), k.num(k.i32, "0"))));
    k.done(f);
  });
  lc("bit_case_value_wrong", "case 0x80 | 1 with the front end's value 130: the case value's guard fails", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    const int id = k.jid();
    const Stmt* c1 = k.cs(k.bor(k.num(k.i32, "128"), k.num(k.i32, "1")), "130");
    const Stmt* swt = k.sw(k.pv(f, 0), k.blk({c1, k.ret(k.num(k.i32, "1")), k.dflt(), k.ret(k.num(k.i32, "0"))}), id);
    f->body = k.blk({swt});
    k.done(f);
    k.fail("internal", c1->loc, 1, "", "case value");
  });

  // ---- do ... while (c) with a constant bitwise condition: the evaluator folds it exactly (0023 item 6) ----
  auto once = [](Kit& k, const Expr* c) {
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.loop(c, {}, {}, k.blk({}), nullptr, LoopForm::DoOnce, k.jid())});
    k.done(f);
  };
  lc("bit_do_once_and_zero", "do { } while (16 & 1): the condition folds to 0", [=](Kit& k) { once(k, k.tb(k.band(k.num(k.i32, "16"), k.num(k.i32, "1")))); });
  lc("bit_do_once_xor_self", "do { } while (5 ^ 5)", [=](Kit& k) { once(k, k.tb(k.bxor(k.num(k.i32, "5"), k.num(k.i32, "5")))); });
  lc("bit_do_once_not_minus_one", "do { } while (~-1): -(-1) - 1 == 0", [=](Kit& k) { once(k, k.tb(k.bnot(k.num(k.i32, "-1")))); });
  lc("bit_do_once_not_uint_max", "do { } while (~4294967295u): MAX - MAX == 0 (unsigned: hi - v)", [=](Kit& k) { once(k, k.tb(k.bnot(k.num(k.u32, "4294967295")))); });
  lc("bit_do_once_u64_and", "do { } while (18446744073709551615u & 0u): an unsigned 64-bit operand above the signed range", [=](Kit& k) {
    once(k, k.tb(k.band(k.num(k.u64, "18446744073709551615"), k.num(k.u64, "0"))));
  });
  lc("bit_do_once_128_unknown", "do { } while (__int128 0 & 5): 128-bit types are not evaluated, accepted and guarded by lowering", [=](Kit& k) {
    once(k, k.tb(k.band(k.num(k.i128, "0"), k.num(k.i128, "5"))));
  });
  lc("bit_do_once_128_nonzero_fails", "do { } while (__int128 1 | 0): not folded, so lowering's internal guard fails (the condition is not false)", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Stmt* w = k.loop(k.tb(k.bor(k.num(k.i128, "1"), k.num(k.i128, "0"))), {}, {}, k.blk({}), nullptr, LoopForm::DoOnce, k.jid());
    f->body = k.blk({w});
    k.done(f);
    k.fail("internal", w->loc, 1, "", "do-once");
  });

  define_bitwise_masks();
}

void define_bitwise_rejections() {
  // A do-once whose condition folds to a non-zero value is refused by check_program, like any such constant.
  auto once = [](Kit& k, const Expr* c) {
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.loop(c, {}, {}, k.blk({}), nullptr, LoopForm::DoOnce, k.jid())});
    k.done(f);
  };
  const char* kOnce = "lowered as running once whose condition is not a constant zero";
  rc("bit_do_once_or_nonzero_rejected", kOnce, [=](Kit& k) { once(k, k.tb(k.bor(k.num(k.i32, "16"), k.num(k.i32, "1")))); });
  rc("bit_do_once_not_zero_rejected", kOnce, [=](Kit& k) { once(k, k.tb(k.bnot(k.num(k.i32, "0")))); });
  rc("bit_do_once_not_unsigned_zero_rejected", kOnce, [=](Kit& k) { once(k, k.tb(k.bnot(k.num(k.u32, "0")))); });
  rc("bit_do_once_xor_nonzero_rejected", kOnce, [=](Kit& k) { once(k, k.tb(k.bxor(k.num(k.i32, "5"), k.num(k.i32, "4")))); });
  rc("bit_do_once_u64_nonzero_rejected", kOnce, [=](Kit& k) {
    once(k, k.tb(k.band(k.num(k.u64, "18446744073709551615"), k.num(k.u64, "18446744073709551615"))));
  });

  // The enumeration backstop (0018 item 8, 0023 item 4): the encoder rejects first, check_program is the net.
  auto color = [](Kit& k) { return k.en("color", 32, true, "0", "2"); };
  rc("bit_enum_and_left_rejected", "a bitwise operator on a value of an enumeration", [color](Kit& k) {
    const Type c = color(k);
    Function* f = k.fn("f", k.vd, {{"c", c}});
    f->body = k.blk({k.es(k.band(k.pv(f, 0), k.num(c, "1")))});
    k.done(f);
  });
  rc("bit_enum_or_right_rejected", "a bitwise operator on a value of an enumeration", [color](Kit& k) {
    const Type c = color(k);
    Function* f = k.fn("f", k.vd, {{"c", c}});
    f->body = k.blk({k.es(k.bor(k.num(c, "2"), k.pv(f, 0)))});
    k.done(f);
  });
  rc("bit_enum_xor_rejected", "a bitwise operator on a value of an enumeration", [color](Kit& k) {
    const Type c = color(k);
    Function* f = k.fn("f", k.vd, {{"c", c}});
    f->body = k.blk({k.es(k.bxor(k.pv(f, 0), k.pv(f, 0)))});
    k.done(f);
  });
  rc("bit_enum_not_rejected", "unary arithmetic on a value of the enumeration", [color](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"c", color(k)}});
    f->body = k.blk({k.es(k.bnot(k.pv(f, 0)))});
    k.done(f);
  });
  rc("bit_enum_compound_and_rejected", "a bitwise operator on a value of an enumeration", [color](Kit& k) {
    const Type c = color(k);
    Function* f = k.fn("f", k.vd);
    Var* e = k.local(f, "e", c);
    f->body = k.blk({k.decl(e, k.num(c, "0")), k.b.compound_assign(e, BinaryOp::BitAnd, k.num(c, "1"), k.at())});
    k.done(f);
  });

  // Typing: both operands of one integer type, the result of that type; `~` like unary minus.
  rc("bit_and_different_types_rejected", "bitwise operands have types", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}, {"b", k.i64}});
    f->body = k.blk({k.es(k.band(k.pv(f, 0), k.pv(f, 1)))});
    k.done(f);
  });
  rc("bit_or_bool_operands_rejected", "bitwise operands have types", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.bl}, {"b", k.bl}});
    f->body = k.blk({k.es(k.bor(k.pv(f, 0), k.pv(f, 1)))});
    k.done(f);
  });
  rc("bit_not_bool_rejected", "operand of unary arithmetic", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.bl}});
    f->body = k.blk({k.es(k.bnot(k.pv(f, 0)))});
    k.done(f);
  });
  rc("bit_and_wrong_result_type_rejected", "a bitwise operation gives", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}, {"b", k.i32}});
    const Expr* e = k.band(k.pv(f, 0), k.pv(f, 1));
    const_cast<Expr*>(e)->type = k.i64;
    f->body = k.blk({k.es(e)});
    k.done(f);
  });

  // Lowering's own check: the computed type of a bitwise operator is 32, 64 or 128 bits wide (the encoder converts), anything
  // else is an internal error, never a silent default.
  rh("bit_and_16_bit_operands_internal_error", "a bitwise operator on a 16-bit type", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i16}, {"b", k.i16}});
    f->body = k.blk({k.es(k.band(k.pv(f, 0), k.pv(f, 1)))});
    k.done(f);
  });
  rh("bit_or_8_bit_operands_internal_error", "a bitwise operator on a 8-bit type", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.u8}, {"b", k.u8}});
    f->body = k.blk({k.es(k.bor(k.pv(f, 0), k.pv(f, 1)))});
    k.done(f);
  });
  // The same guard for `~` (M4.4b1 audit note 1).
  rh("bit_not_16_bit_operand_internal_error", "a bitwise complement on a 16-bit type", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i16}});
    f->body = k.blk({k.es(k.bnot(k.pv(f, 0)))});
    k.done(f);
  });
  rh("bit_not_8_bit_operand_internal_error", "a bitwise complement on a 8-bit type", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.u8}});
    f->body = k.blk({k.es(k.bnot(k.pv(f, 0)))});
    k.done(f);
  });
}

// ---- static assertions (decision 0024 item 2): checked in the prologue like a named constant whose value is true ----
void define_static_assertions() {
  lc("sa_file_scope_holds", "static_assert(2 + 3 == 5) at file scope: the closed condition is decided, the internal guard holds", [](Kit& k) {
    k.sa(k.eq(k.add(k.num(k.i32, "2"), k.num(k.i32, "3")), k.num(k.i32, "5")));
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.require_map("internal", "static assertion condition as written differs from the front end's");
  });
  lc("sa_file_scope_overflow_fails", "static_assert(2147483647 + 1 > 0) at file scope: the addition is an overflow at its own line (the guard cannot fire after it)", [](Kit& k) {
    const Expr* sum = k.add(k.num(k.i32, "2147483647"), k.num(k.i32, "1"));
    k.sa(k.gt(sum, k.num(k.i32, "0")));
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.fail("overflow", sum->loc, 1, "", "addition");
  });
  lc("sa_file_scope_conversion_fails", "static_assert((signed char)200 == -56) at file scope: the conversion of 200 to signed char is an error", [](Kit& k) {
    const Expr* narrow = k.cv(k.i8, k.num(k.i32, "200"));
    k.sa(k.eq(k.cv(k.i32, narrow), k.num(k.i32, "-56")));
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.fail("conversion", narrow->loc, 1, "", "the value may not fit in `signed char`");
  });
  lc("sa_file_scope_shift_fails", "static_assert((1 << 40) == 0) at file scope: the shift is an error", [](Kit& k) {
    const Expr* sh = k.shl(k.num(k.i32, "1"), k.num(k.i32, "40"));
    k.sa(k.eq(sh, k.num(k.i32, "0")));
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.fail("shift", sh->loc, 1, "", "shift count");
  });
  lc("sa_file_scope_checked_in_the_wf_method", "a failing file-scope assertion is reported in a function's method and in its __wf method", [](Kit& k) {
    const Expr* sum = k.add(k.num(k.i32, "2147483647"), k.num(k.i32, "1"));
    k.sa(k.gt(sum, k.num(k.i32, "0")));
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    k.req(f, k.ge_c(k.F(f, 0), k.i32, "0"));
    f->body = k.blk({k.ret(k.pv(f, 0))});
    k.done(f);
    k.fail("overflow", sum->loc, 2, "", "addition");
  });
  lc("sa_literal_true_has_no_guard", "static_assert(true): a bare literal, nothing to check and no internal guard", [](Kit& k) {
    NamedConst* c = k.sa(k.yes());
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.rows("internal", c->loc, 0);
  });
  lc("sa_literal_true_value_differs", "static_assert(true) with the value 0 (the perturbation hook): a bare literal that differs still gets its guard, which fails", [](Kit& k) {
    NamedConst* c = k.sa(k.yes());
    c->value = "0";
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.fail("internal", c->loc, 1, "", "static assertion condition as written");
  });
  lc("sa_value_differs_guard_fails", "static_assert(2 + 3 == 5) with the value 0: the condition holds, the guard compares with false and fails", [](Kit& k) {
    NamedConst* c = k.sa(k.eq(k.add(k.num(k.i32, "2"), k.num(k.i32, "3")), k.num(k.i32, "5")));
    c->value = "0";
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.fail("internal", c->loc, 1, "", "static assertion condition as written");
  });
  lc("sa_after_failing_constant_not_reported", "const int A = 1 << 40; static_assert(2147483647 + 1 > 0): the constant fails first and ends the path, so the assertion's overflow is not reported", [](Kit& k) {
    const Expr* init = k.shl(k.num(k.i32, "1"), k.num(k.i32, "40"));
    k.nc("A", k.i32, init, "0");
    const Expr* sum = k.add(k.num(k.i32, "2147483647"), k.num(k.i32, "1"));
    k.sa(k.gt(sum, k.num(k.i32, "0")));
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.fail("shift", init->loc, 1, "", "shift count");
  });
  lc("sa_before_failing_constant_is_reported_first", "static_assert(1 + 1 == 2) then const int A = 1 << 40: the assertion holds, the constant fails (call order is prologue order)", [](Kit& k) {
    k.sa(k.eq(k.add(k.num(k.i32, "1"), k.num(k.i32, "1")), k.num(k.i32, "2")));
    const Expr* init = k.shl(k.num(k.i32, "1"), k.num(k.i32, "40"));
    k.nc("A", k.i32, init, "0");
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.fail("shift", init->loc, 1, "", "shift count");
  });
  lc("sa_names_a_named_constant", "const int N = 20 + 20 (value 40); static_assert(N == 40) with N as the literal 40: closed, holds", [](Kit& k) {
    k.nc("N", k.i32, k.add(k.num(k.i32, "20"), k.num(k.i32, "20")), "40");
    k.sa(k.eq(k.num(k.i32, "40"), k.num(k.i32, "40")));
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.require_map("internal", "static assertion condition as written differs from the front end's");
  });
  lc("sa_unsigned_wrap_fails", "static_assert(65536u * 65536u == 0u): the multiplication does not fit, an overflow (cerastes does not wrap)", [](Kit& k) {
    const Expr* prod = k.bin(BinaryOp::Mul, k.num(k.u32, "65536"), k.num(k.u32, "65536"));
    k.sa(k.eq(prod, k.num(k.u32, "0")));
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
    k.fail("overflow", prod->loc, 1, "", "unsigned `unsigned int` multiplication");
  });
  // Block scope: the function's own local list, in this function's method and its __wf method.
  lc("sa_block_scope_holds", "static_assert(sizeof-like 4 == 4) inside f: a local static assertion, the guard holds", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    k.lsa(f, k.eq(k.add(k.num(k.i32, "2"), k.num(k.i32, "2")), k.num(k.i32, "4")));
    f->body = k.blk({});
    k.done(f);
    k.require_map("internal", "static assertion condition as written differs from the front end's");
  });
  lc("sa_block_scope_shift_fails", "static_assert((1 << 40) == 0) inside f: the shift is reported in f's method", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Expr* sh = k.shl(k.num(k.i32, "1"), k.num(k.i32, "40"));
    k.lsa(f, k.eq(sh, k.num(k.i32, "0")));
    f->body = k.blk({});
    k.done(f);
    k.fail("shift", sh->loc, 1, "", "shift count");
  });
  lc("sa_block_scope_checked_when_requires_is_unsatisfiable", "requires x > 0 && x < 0 makes f's method vacuous; the block assertion's overflow is still reported, by the __wf method", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    k.req(f, k.land(k.gt(k.F(f, 0), k.num(k.i32, "0")), k.lt(k.F(f, 0), k.num(k.i32, "0"))));
    const Expr* sum = k.add(k.num(k.i32, "2147483647"), k.num(k.i32, "1"));
    k.lsa(f, k.gt(sum, k.num(k.i32, "0")));
    f->body = k.blk({k.ret(k.pv(f, 0))});
    k.done(f);
    k.fail_only("overflow", sum->loc, 2, 1, "addition");  // the method's own copy is vacuous; the __wf copy fails
  });
  lc("sa_block_scope_with_local_const_in_call_order", "a local constant, then a block assertion, then a case value's shift: the prologue is local constants and assertions in call order, then the case values", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"x", k.i32}});
    k.lnc(f, "k", k.i32, k.add(k.num(k.i32, "2"), k.num(k.i32, "3")), "5");
    k.lsa(f, k.eq(k.num(k.i32, "5"), k.num(k.i32, "5")));
    const int id = k.jid();
    f->body = k.blk({k.sw(k.pv(f, 0), k.blk({k.cs(k.num(k.i32, "5")), k.ret(k.num(k.i32, "1")), k.dflt(), k.ret(k.num(k.i32, "0"))}), id)});
    k.done(f);
    k.require_map("internal", "static assertion condition as written differs from the front end's");
  });
}

// What check_program refuses about a static assertion (decision 0024 item 2): a non-bool condition, a condition that names a
// variable, a parameter, the result, `old` or a call, and a block assertion in a function without a body.
void define_static_assertion_rejections() {
  rc("sa_non_bool_type_rejected", "not bool", [](Kit& k) {
    NamedConst* c = k.sa(k.num(k.i32, "1"));
    c->type = k.i32;  // C's `static_assert(1)` must arrive as a Bool condition: the encoder converts
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
  });
  rc("sa_non_bool_condition_rejected", "has type int, not bool", [](Kit& k) {
    k.sa(k.num(k.i32, "1"));
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
  });
  rc("sa_condition_names_a_parameter_rejected", "mentions a variable", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"x", k.i32}});
    f->body = k.blk({});
    k.done(f);
    k.sa(k.eq(k.pv(f, 0), k.num(k.i32, "1")));
  });
  rc("sa_local_condition_names_a_parameter_rejected", "mentions a variable", [](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"x", k.i32}});
    k.lsa(f, k.eq(k.pv(f, 0), k.num(k.i32, "1")));
    f->body = k.blk({});
    k.done(f);
  });
  rc("sa_condition_calls_a_function_rejected", "mentions a variable", [](Kit& k) {
    Function* g = k.fn("g", k.bl);
    g->body = k.blk({k.ret(k.yes())});
    k.done(g);
    k.sa(k.call(g, {}));
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({});
    k.done(f);
  });
  rc("sa_local_in_a_bodyless_function_rejected", "which has no body", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    k.trusted(f, "declared here, defined in another unit");
    k.lsa(f, k.yes());
    k.done(f);
  });
}

}  // namespace
