// (included by selftest.cpp) `|` and `^` with a constant mask (decision 0027): the forms `2^k * (x \ 2^k) + (2^k - 1)`, `... + ((2^k - 1) - x % 2^k)`
// and the complement forms of the table in 0027 item 1.  Uses the helpers of selftest_cases5.inc (bit_mask, bit_text, bit_type, bit_apply).
// PRE-REGISTERED, like the 0026 cases: every claim is an identity that holds for every operand in the type's range, stated with `&` by the
// constants of 0026 (decided for every operand, 0026 part B) or with C's `+ - %`, never with the bit function of `|` or `^`; each `_wrong` twin
// states a false claim that Z3 refutes (never one that needs the bits of a variable mask).  The cases do not mention how the mask is lowered: they
// are identities of the two operators and pass with the prelude function (where Z3 decides them) or with the remainder forms.
#include "dec.h"

namespace {

// The statements of `f(x)` returning `x op mask`, in one of five syntactic forms (0: `x op lit`, 1: `lit op x`, 2: `y = x; y op= lit`,
// 3: `x op ~alt`, 4: `~alt op x`); `alt` is the complementary pattern of the mask.  Returns the `return`.
const Stmt* or_xor_body(Kit& k, Function* f, Type t, BinaryOp op, const std::string& mT, const std::string& altT, int variant) {
  auto N = [&](const std::string& v) { return k.num(t, v); };
  std::vector<const Stmt*> body;
  const Stmt* r = nullptr;
  switch (variant) {
    case 0: r = k.ret(k.bin(op, k.pv(f, 0), N(mT))); break;
    case 1: r = k.ret(k.bin(op, N(mT), k.pv(f, 0))); break;
    case 2: {
      Var* y = k.local(f, "y", t);
      body.push_back(k.decl(y, k.pv(f, 0)));
      body.push_back(k.b.compound_assign(y, op, N(mT), k.at()));
      r = k.ret(k.v(y));
      break;
    }
    case 3: r = k.ret(k.bin(op, k.pv(f, 0), k.bnot(N(altT)))); break;
    default: r = k.ret(k.bin(op, k.bnot(N(altT)), k.pv(f, 0))); break;
  }
  body.push_back(r);
  f->body = k.blk(body);
  k.done(f);
  return r;
}

void define_bitwise_or_xor_masks() {
  const int widths[] = {32, 64, 128};
  const BinaryOp ops[] = {BinaryOp::BitOr, BinaryOp::BitXor};
  int variant_counter = 0;

  // ---- symbolic operand, the identities of 0027 item 2 in terms of the 0026 forms of `&`: with low = 2^k - 1 and high = ~low,
  //   x | low  == (x & high) + low                      x ^ low  == (x & high) + (low - (x & low))
  //   x | high == (x & low) + high                      x ^ high == ((x & low) + high) - (x & high)
  // k at 1, 8 and the top for the mask 2^k - 1 (N - 1 signed, N unsigned: the all-ones mask), and at 0, 8 and N - 1 for its complement
  // (k = N is the mask 0 there).  Each twin drops the term that carries the mask (x | low is not x & high; x ^ low is not x), false at x = 0.
  for (const int bits : widths) {
    for (const bool sg : {true, false}) {
      for (const BinaryOp op : ops) {
        for (const bool comp : {false, true}) {
          const std::vector<int> ks = comp ? std::vector<int>{0, 8, bits - 1} : std::vector<int>{1, 8, sg ? bits - 1 : bits};
          for (const int kk : ks) {
            const int variant = variant_counter++ % 5;
            auto build = [=](Kit& k, bool wrong) -> SrcLoc {
              const Type t = bit_type(k, bits, sg);
              const BitU128 M = bit_mask(bits);
              const BitU128 low = kk == 128 ? M : ((BitU128(1) << kk) - 1);  // 2^kk - 1
              const BitU128 high = (~low) & M;                               // ~(2^kk - 1)
              const std::string lowT = bit_text(low, bits, sg), highT = bit_text(high, bits, sg);
              const std::string mT = comp ? highT : lowT, altT = comp ? lowT : highT;
              Function* f = k.fn("f", t, {{"x", t}});
              auto N = [&](const std::string& v) { return k.num(t, v); };
              auto X = [&]() { return k.F(f, 0); };
              auto R = [&]() { return k.R(f); };
              auto A = [&](const std::string& m) { return k.band(X(), N(m)); };  // x & m: a mask of 0026
              const Expr* claim = nullptr;
              if (op == BinaryOp::BitOr) {
                if (!comp) claim = k.eq(R(), wrong ? A(highT) : k.add(A(highT), N(lowT)));
                else claim = k.eq(R(), wrong ? A(lowT) : k.add(A(lowT), N(highT)));
              } else {
                if (!comp) claim = k.eq(R(), wrong ? k.add(A(highT), A(lowT)) : k.add(A(highT), k.sub(N(lowT), A(lowT))));
                else claim = k.eq(R(), wrong ? k.add(A(lowT), A(highT)) : k.sub(k.add(A(lowT), N(highT)), A(highT)));
              }
              k.ens(f, claim);
              return or_xor_body(k, f, t, op, mT, altT, variant)->loc;
            };
            const std::string name = std::string("bit_") + bit_op_name(op) + "_mask_" + (comp ? "cmpl" : "low") + (sg ? "_s" : "_u") +
                                     std::to_string(bits) + "_k" + std::to_string(kk) + "_v" + std::to_string(variant);
            const std::string doc = std::string("x ") + bit_op_sym(op) + (comp ? " ~(2^k-1)" : " (2^k-1)") + " with k = " + std::to_string(kk) +
                                    " on a symbolic " + (sg ? "signed " : "unsigned ") + std::to_string(bits) + "-bit x (form " +
                                    std::to_string(variant) + "): the identity with & holds";
            lc(name, doc, [=](Kit& k) { build(k, false); });
            lc(name + "_wrong", doc + " (a false claim, refuted by Z3)", [=](Kit& k) {
              const SrcLoc l = build(k, true);
              k.fail("postcondition", l);
            });
          }
        }
      }
    }
  }

  // ---- the same in C's terms for the mask 2^k - 1 at k = 8 and N / 2: x | m is x - L + m and x ^ m is (x - L) + (m - L), with L the non-negative
  // remainder of x modulo 2^k (C's `%` for an unsigned x; `((x % P) + P) % P` for a signed one, whose `%` is negative for a negative x).
  // The twin of `|` is off by one in the constant (m - 1), the twin of `^` does not flip (it adds L back).
  for (const int bits : widths) {
    for (const bool sg : {true, false}) {
      for (const BinaryOp op : ops) {
        for (const int kk : {8, bits / 2}) {
          const int variant = variant_counter++ % 5;
          auto build = [=](Kit& k, bool wrong) -> SrcLoc {
            const Type t = bit_type(k, bits, sg);
            const BitU128 low = (BitU128(1) << kk) - 1;
            const std::string lowT = bit_text(low, bits, sg), lowM1 = bit_text(low - 1, bits, sg), pT = bit_text(BitU128(1) << kk, bits, sg);
            const std::string altT = bit_text((~low) & bit_mask(bits), bits, sg);
            Function* f = k.fn("f", t, {{"x", t}});
            auto N = [&](const std::string& v) { return k.num(t, v); };
            auto X = [&]() { return k.F(f, 0); };
            auto L = [&]() -> const Expr* {
              const Expr* m = k.rem(X(), N(pT));
              return sg ? k.rem(k.add(m, N(pT)), N(pT)) : m;
            };
            const Expr* claim = nullptr;
            if (op == BinaryOp::BitOr) claim = k.eq(k.R(f), k.add(k.sub(X(), L()), N(wrong ? lowM1 : lowT)));
            else claim = k.eq(k.R(f), wrong ? k.add(k.sub(X(), L()), L()) : k.add(k.sub(X(), L()), k.sub(N(lowT), L())));
            k.ens(f, claim);
            return or_xor_body(k, f, t, op, lowT, altT, variant)->loc;
          };
          const std::string name = std::string("bit_") + bit_op_name(op) + "_remainder" + (sg ? "_s" : "_u") + std::to_string(bits) + "_k" +
                                   std::to_string(kk) + "_v" + std::to_string(variant);
          const std::string doc = std::string("x ") + bit_op_sym(op) + " (2^" + std::to_string(kk) + "-1) on a symbolic " + (sg ? "signed " : "unsigned ") +
                                  std::to_string(bits) + "-bit x: " + (op == BinaryOp::BitOr ? "x - L + m" : "(x - L) + (m - L)");
          lc(name, doc, [=](Kit& k) { build(k, false); });
          lc(name + "_wrong", doc + (op == BinaryOp::BitOr ? " (the constant is m - 1)" : " (the low bits are not flipped)"), [=](Kit& k) {
            const SrcLoc l = build(k, true);
            k.fail("postcondition", l);
          });
        }
      }
    }
  }

  // ---- closed operands, five masks: 255, ~15, all ones, 0 and the maximum, on the right and on the left; every result exact ----
  // (the last claim of the twin is off by one).
  for (const int bits : widths) {
    for (const bool sg : {true, false}) {
      for (const BinaryOp op : ops) {
        auto build = [=](Kit& k, bool wrong) -> SrcLoc {
          const Type t = bit_type(k, bits, sg);
          const BitU128 M = bit_mask(bits);
          const BitU128 top = BitU128(1) << (bits - 1);
          const BitU128 patX = sg ? ((~BitU128(1000) + 1) & M) : BitU128(1000);
          auto N = [&](const std::string& v) { return k.num(t, v); };
          Function* f = k.fn("f", k.vd);
          std::vector<const Stmt*> body;
          SrcLoc last;
          auto add = [&](const Expr* mask, BitU128 maskPattern, bool left, bool is_last) {
            BitU128 want = bit_apply(op, patX, maskPattern) & M;
            if (wrong && is_last) want ^= 1;
            const Expr* x = N(sg ? "-1000" : "1000");
            const Stmt* c = k.chk(k.eq(left ? k.bin(op, mask, x) : k.bin(op, x, mask), N(bit_text(want, bits, sg))));
            last = c->loc;
            body.push_back(c);
          };
          add(N("255"), 255, false, false);
          add(k.bnot(N("15")), (~BitU128(15)) & M, true, false);
          add(N(sg ? "-1" : bit_text(M, bits, false)), M, false, false);
          add(N("0"), 0, true, false);
          add(N(bit_text(top - 1, bits, sg)), top - 1, false, true);
          f->body = k.blk(body);
          k.done(f);
          return last;
        };
        const std::string name = std::string("bit_") + bit_op_name(op) + "_mask_closed" + (sg ? "_s" : "_u") + std::to_string(bits);
        const std::string doc = std::string(sg ? "-1000 " : "1000 ") + bit_op_sym(op) + " 255, ~15, all ones, 0 and the maximum on " + (sg ? "signed " : "unsigned ") +
                                std::to_string(bits) + " bits: five exact closed results";
        lc(name, doc, [=](Kit& k) { build(k, false); });
        lc(name + "_wrong", doc + " (the last claim is off by one)", [=](Kit& k) {
          const SrcLoc l = build(k, true);
          k.fail("check", l);
        });
      }
    }
  }

  // ---- chains: each mask is applied to the result of the one before ----
  // Each `^` by a mask is one call of a prelude mask function, so the chain grows linearly (0027 item 3, revised).  Identities through a chain are
  // limited in depth (0027 item 4: Z3 decides `r == x` through four masks that cancel in pairs, not through eight): "pairs" (255, 255, 15, 15) and
  // "complements" (~15, 255, ~15, 255) give x back; the twin leaves one mask unpaired, so the result is x ^ m, which is not x (x = 0).  "Eight"
  // (alternating 255 and 15) carries a range claim only: for 0 <= x <= 1000 the result is in [0, 1023] (the masks touch the low byte), which holds at any depth.
  struct Link {
    const char* text;
    bool complement;
  };
  for (const int bits : widths) {
    for (const bool sg : {true, false}) {
      const std::vector<std::pair<std::string, std::vector<Link>>> chains = {
          {"pairs", {{"255", false}, {"255", false}, {"15", false}, {"15", false}}},
          {"complements", {{"15", true}, {"255", false}, {"15", true}, {"255", false}}},
          {"eight_range", {{"255", false}, {"15", false}, {"255", false}, {"15", false}, {"255", false}, {"15", false}, {"255", false}, {"15", false}}}};
      for (const auto& ch : chains) {
        const bool range_only = ch.first == "eight_range";
        auto build = [=](Kit& k, bool wrong) -> SrcLoc {
          const Type t = bit_type(k, bits, sg);
          auto N = [&](const std::string& v) { return k.num(t, v); };
          Function* f = k.fn("f", t, {{"x", t}});
          if (range_only) {
            // 0 <= x <= 1000 gives 0 <= r <= 1023 (the eight masks cancel in pairs, so r == x and r <= 1000 holds too); the twin claims r <= 999, false for x = 1000
            k.req(f, k.land(k.ge_c(k.F(f, 0), t, "0"), k.le_c(k.F(f, 0), t, "1000")));
            k.ens(f, k.land(k.ge_c(k.R(f), t, "0"), k.le_c(k.R(f), t, wrong ? "999" : "1023")));
          } else {
            k.ens(f, k.eq(k.R(f), k.F(f, 0)));
          }
          const Expr* acc = k.pv(f, 0);
          const size_t n = ch.second.size() - ((wrong && !range_only) ? 1 : 0);
          for (size_t i = 0; i < n; ++i) {
            const Link& l = ch.second[i];
            acc = k.bxor(acc, l.complement ? k.bnot(N(l.text)) : N(l.text));
          }
          const Stmt* r = k.ret(acc);
          f->body = k.blk({r});
          k.done(f);
          return r->loc;
        };
        const std::string name = std::string("bit_xor_chain_") + ch.first + (sg ? "_s" : "_u") + std::to_string(bits);
        const std::string doc = std::string("a chain of ") + std::to_string(ch.second.size()) + " ^ masks (" + ch.first + ") on a " + (sg ? "signed " : "unsigned ") +
                                std::to_string(bits) + "-bit x: " + (range_only ? "0 <= x <= 1000 gives 0 <= r <= 1023" : "cancels in pairs, the result is x");
        lc(name, doc, [=](Kit& k) { build(k, false); });
        lc(name + "_wrong", doc + (range_only ? " (the bound is 999: x = 1000 gives r = 1000)" : " (the last mask is left out, one is unpaired)"), [=](Kit& k) {
          const SrcLoc l = build(k, true);
          k.fail("postcondition", l);
        });
      }
    }
  }

  // ---- | and ^ on the result of the other and of &: (x | 255) ^ 255 is x & ~255, and (x ^ 255) | 255 and (x & ~255) ^ 255 are x | 255 ----
  for (const int bits : widths) {
    for (const bool sg : {true, false}) {
      auto build = [=](Kit& k, bool wrong) -> SrcLoc {
        const Type t = bit_type(k, bits, sg);
        auto N = [&](const std::string& v) { return k.num(t, v); };
        Function* f1 = k.fn("or_then_xor", t, {{"x", t}});
        k.ens(f1, k.eq(k.R(f1), wrong ? k.F(f1, 0) : k.band(k.F(f1, 0), k.bnot(N("255")))));
        const Stmt* r1 = k.ret(k.bxor(k.bor(k.pv(f1, 0), N("255")), N("255")));
        f1->body = k.blk({r1});
        k.done(f1);
        Function* f2 = k.fn("xor_then_or", t, {{"x", t}});
        k.ens(f2, k.eq(k.R(f2), k.bor(k.F(f2, 0), N("255"))));
        const Stmt* r2 = k.ret(k.bor(k.bxor(k.pv(f2, 0), N("255")), N("255")));
        f2->body = k.blk({r2});
        k.done(f2);
        Function* f3 = k.fn("clear_then_xor", t, {{"x", t}});
        k.ens(f3, k.eq(k.R(f3), k.bor(k.F(f3, 0), N("255"))));
        const Stmt* r3 = k.ret(k.bxor(k.band(k.pv(f3, 0), k.bnot(N("255"))), N("255")));
        f3->body = k.blk({r3});
        k.done(f3);
        return r1->loc;
      };
      const std::string name = std::string("bit_or_xor_mixed") + (sg ? "_s" : "_u") + std::to_string(bits);
      const std::string doc = std::string("(x | 255) ^ 255 == x & ~255, (x ^ 255) | 255 == x | 255 and (x & ~255) ^ 255 == x | 255 on a ") + (sg ? "signed " : "unsigned ") +
                              std::to_string(bits) + "-bit x";
      lc(name, doc, [=](Kit& k) { build(k, false); });
      lc(name + "_wrong", doc + " (the first claims (x | 255) ^ 255 == x)", [=](Kit& k) {
        const SrcLoc l = build(k, true);
        k.fail("postcondition", l);
      });
    }
  }

  // ---- a closed operand that is itself lowered by the rule keeps the prelude function of the outer operator and is exact (0027 item 3): in
  // x ^ (240 ^ 15) the operand 240 ^ 15 is the mask 255 by value, but lowering writes it with its own mask 15, so the outer `^` is the function ----
  for (const int bits : widths) {
    for (const bool sg : {true, false}) {
      for (const BinaryOp op : ops) {
        auto build = [=](Kit& k, bool wrong) -> SrcLoc {
          const Type t = bit_type(k, bits, sg);
          const BitU128 M = bit_mask(bits);
          const BitU128 patX = sg ? ((~BitU128(1000) + 1) & M) : BitU128(1000);
          auto N = [&](const std::string& v) { return k.num(t, v); };
          Function* f = k.fn("f", k.vd);
          BitU128 want = bit_apply(op, patX, 255) & M;
          if (wrong) want ^= 1;
          const Expr* mask = k.bin(op, N("240"), N("15"));
          const Stmt* c = k.chk(k.eq(k.bin(op, N(sg ? "-1000" : "1000"), mask), N(bit_text(want, bits, sg))));
          f->body = k.blk({c});
          k.done(f);
          return c->loc;
        };
        const std::string name = std::string("bit_") + bit_op_name(op) + "_mask_nested_closed" + (sg ? "_s" : "_u") + std::to_string(bits);
        const std::string doc = std::string(sg ? "-1000 " : "1000 ") + bit_op_sym(op) + " (240 " + bit_op_sym(op) + " 15) on " + (sg ? "signed " : "unsigned ") +
                                std::to_string(bits) + " bits: the operand is 255 by value, lowered by its own rule";
        lc(name, doc, [=](Kit& k) { build(k, false); });
        lc(name + "_wrong", doc + " (the claim is off by one)", [=](Kit& k) {
          const SrcLoc l = build(k, true);
          k.fail("check", l);
        });
      }
    }
  }
}

}  // namespace
