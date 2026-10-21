// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// cerastes-vir-selftest <dir>: the self-test of VIR, its lowering and the Viper emitter, before any C/C++ front
// end is involved (internal/encoding.md).
//
//  * Every LOWERING case builds a VIR program by hand, checks and lowers it, emits `<dir>/<name>.vpr` and
//    `<dir>/<name>.vpr.map`, and writes at the top of the .vpr the `// expect:` header the harness understands
//    (`verified`, or one `error <category> @ L<n>` per check that must fail).  `python3 scripts/run_tests.py
//    <dir>` then runs Silicon on all of them.  The expected failures are named in terms of the VIR program (a
//    category and the source line of the node) and translated to Viper lines through the position map, so a
//    case also tests the map.
//  * Every REJECTION case builds an ill-typed program and requires check_program to fail with a given reason.
//    These are checked in-process (exit status 1 on a mismatch).
//
// Each lowering rule has at least one case that must verify and one that must fail (invariant 2).
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "check.h"
#include "emit.h"
#include "lower.h"
#include "vir.h"

namespace {

using namespace cerastes::vir;

struct Expect {
  std::string category;   // the position-map category (a driver category, or "internal")
  int line = 0;           // the source line of the node (SrcLoc::line)
  size_t count = 1;       // how many emitted checks must match (and must fail)
  std::string viper_id;   // restrict to one Viper error id ("" = any)
  std::string msg;        // restrict to checks whose message contains this ("" = any)
  int note_line = -1;     // when >= 0, the map entry must carry a note at this source line
};

// A kit for building programs: every node gets a fresh source line, so an expectation can name a node.
class Kit {
 public:
  Program prog;
  Builder b{prog};
  Type bl = Type::bool_type();
  Type i8 = Type::integer(prog.int_type("signed char", 8, true));
  Type u8 = Type::integer(prog.int_type("unsigned char", 8, false));
  Type i16 = Type::integer(prog.int_type("short", 16, true));
  Type u16 = Type::integer(prog.int_type("unsigned short", 16, false));
  Type i32 = Type::integer(prog.int_type("int", 32, true));
  Type u32 = Type::integer(prog.int_type("unsigned int", 32, false));
  Type i64 = Type::integer(prog.int_type("long", 64, true));
  Type u64 = Type::integer(prog.int_type("unsigned long", 64, false));
  Type i128 = Type::integer(prog.int_type("__int128", 128, true));
  Type u128 = Type::integer(prog.int_type("unsigned __int128", 128, false));
  Type vd = Type::void_type();
  std::vector<Expect> expects;
  // Position-map rows that must exist without failing (category, message substring): for the checks that cannot be made
  // to fail from VIR (an internal backstop).
  std::vector<std::pair<std::string, std::string>> map_requires;
  LowerOptions lower_options;

  SrcLoc at() { return SrcLoc{"t.c", ++line_, 3}; }

  void fail(const std::string& category, const SrcLoc& l, size_t count = 1, const std::string& id = "",
            const std::string& msg = "") {
    expects.push_back(Expect{category, l.line, count, id, msg});
  }

  // The same, and the entry's note (the position of the obligation inside a report_at node) must be at `note_line`.
  void fail_with_note(const std::string& category, const SrcLoc& l, int note_line) {
    expects.push_back(Expect{category, l.line, 1, "", "", note_line});
  }

  struct P {
    std::string name;
    Type type;
  };
  Function* fn(const std::string& name, Type ret, const std::vector<P>& params = {}) {
    Function* f = prog.add_function(name, ret, at(), SrcLoc{});
    for (const P& p : params) f->add_param(p.name, p.type, at());
    return f;
  }
  // Fill the closing-brace position; call after the body is built.
  void done(Function* f) { f->end_loc = at(); }

  // ---- expressions (each with a fresh line) ----
  const Expr* num(Type t, const std::string& v) { return b.int_const(t, v, at()); }
  const Expr* yes() { return b.bool_const(true, at()); }
  const Expr* no() { return b.bool_const(false, at()); }
  const Expr* v(const Var* x) { return b.local(x, at()); }
  const Expr* pv(Function* f, int k) { return v(f->param_var(k)); }  // a parameter's local copy
  const Expr* F(Function* f, int k) { return b.formal(*f, k, at()); }
  const Expr* R(Function* f) { return b.result(*f, at()); }
  const Expr* old(const Expr* e) { return b.old(e, at()); }
  const Expr* neg(const Expr* a) { return b.unary(UnaryOp::Neg, a, at()); }
  const Expr* lnot(const Expr* a) { return b.unary(UnaryOp::Not, a, at()); }
  const Expr* bin(BinaryOp op, const Expr* a, const Expr* c) { return b.binary(op, a, c, at()); }
  const Expr* add(const Expr* a, const Expr* c) { return bin(BinaryOp::Add, a, c); }
  const Expr* sub(const Expr* a, const Expr* c) { return bin(BinaryOp::Sub, a, c); }
  const Expr* mul(const Expr* a, const Expr* c) { return bin(BinaryOp::Mul, a, c); }
  const Expr* div(const Expr* a, const Expr* c) { return bin(BinaryOp::Div, a, c); }
  const Expr* rem(const Expr* a, const Expr* c) { return bin(BinaryOp::Rem, a, c); }
  const Expr* shl(const Expr* a, const Expr* c) { return bin(BinaryOp::Shl, a, c); }
  const Expr* shr(const Expr* a, const Expr* c) { return bin(BinaryOp::Shr, a, c); }
  const Expr* lt(const Expr* a, const Expr* c) { return bin(BinaryOp::Lt, a, c); }
  const Expr* le(const Expr* a, const Expr* c) { return bin(BinaryOp::Le, a, c); }
  const Expr* gt(const Expr* a, const Expr* c) { return bin(BinaryOp::Gt, a, c); }
  const Expr* ge(const Expr* a, const Expr* c) { return bin(BinaryOp::Ge, a, c); }
  const Expr* eq(const Expr* a, const Expr* c) { return bin(BinaryOp::Eq, a, c); }
  const Expr* ne(const Expr* a, const Expr* c) { return bin(BinaryOp::Ne, a, c); }
  const Expr* land(const Expr* a, const Expr* c) { return bin(BinaryOp::LAnd, a, c); }
  const Expr* lor(const Expr* a, const Expr* c) { return bin(BinaryOp::LOr, a, c); }
  const Expr* cond(const Expr* c, const Expr* a, const Expr* e) { return b.cond(c, a, e, at()); }
  const Expr* cv(Type t, const Expr* a) { return b.convert(t, a, false, at()); }
  const Expr* tb(const Expr* a) { return b.to_bool(a, at()); }
  const Expr* bi(Type t, const Expr* a) { return b.bool_to_int(t, a, at()); }
  const Expr* call(const Function* f, std::vector<const Expr*> args) { return b.call(f, std::move(args), at()); }
  const Expr* implies(const Expr* a, const Expr* c) { return lor(lnot(a), c); }
  // A comparison with a constant.
  const Expr* ge_c(const Expr* a, Type t, const std::string& k) { return ge(a, num(t, k)); }
  const Expr* le_c(const Expr* a, Type t, const std::string& k) { return le(a, num(t, k)); }
  const Expr* eq_c(const Expr* a, Type t, const std::string& k) { return eq(a, num(t, k)); }
  const Expr* range(const Expr* a, Type t, const std::string& lo, const std::string& hi) {
    return land(ge_c(a, t, lo), le_c(a, t, hi));
  }

  // ---- statements ----
  const Stmt* decl(const Var* x, const Expr* e) { return b.decl(x, e, at()); }
  const Stmt* asg(const Var* x, const Expr* e) { return b.assign(x, e, at()); }
  const Stmt* es(const Expr* e) { return b.expr_stmt(e, at()); }
  const Stmt* ret(const Expr* e) { return b.return_stmt(e, at()); }
  const Stmt* chk(const Expr* e) { return b.check(e, at()); }
  const Stmt* blk(std::vector<const Stmt*> s) { return b.block(std::move(s), at()); }
  const Stmt* iff(const Expr* c, const Stmt* t, const Stmt* e = nullptr) { return b.if_stmt(c, t, e, at()); }
  // A loop with its invariants and its termination measure (decision 0013): the measure is required by check_program.
  const Stmt* whl(const Expr* c, std::vector<Clause> inv, std::vector<Clause> measure, const Stmt* body) {
    return b.while_stmt(c, std::move(inv), std::move(measure), body, at());
  }
  void require_map(const std::string& category, const std::string& msg) { map_requires.emplace_back(category, msg); }

  // Control flow (decision 0014).  A loop in any form; `id` is from jid().
  int jid() { return b.jump_id(); }
  const Stmt* loop(const Expr* c, std::vector<Clause> inv, std::vector<Clause> measure, const Stmt* body, const Stmt* step,
                   LoopForm form, int id) {
    return b.loop(c, std::move(inv), std::move(measure), body, step, form, id, at());
  }
  const Stmt* sw(const Expr* scrutinee, const Stmt* body, int id) { return b.switch_stmt(scrutinee, body, id, at()); }
  const Stmt* cs(const Expr* value) { return b.case_label(value, at()); }
  const Stmt* dflt() { return b.case_label(nullptr, at()); }
  const Stmt* brk(int id) { return b.break_stmt(id, at()); }
  const Stmt* cnt(int id) { return b.continue_stmt(id, at()); }
  // `(long)a - (long)b`: a measure that cannot overflow.
  const Expr* gap(const Expr* a, const Expr* c) { return sub(cv(i64, a), cv(i64, c)); }
  Clause cl(const Expr* e) { return Clause{e, at()}; }
  Var* local(Function* f, const std::string& n, Type t) { return f->new_local(n, t, at()); }

  void req(Function* f, const Expr* e) { f->requires_.push_back(cl(e)); }
  void ens(Function* f, const Expr* e) { f->ensures_.push_back(cl(e)); }
  void dec(Function* f, const Expr* e) { f->decreases_.push_back(cl(e)); }
  void trusted(Function* f, const std::string& why = "declared here, defined in another unit") { f->trusted_reason = why; }

 private:
  int line_ = 0;
};

struct Case {
  std::string name;
  std::string doc;
  std::function<void(Kit&)> build;
};

std::vector<Case>& lowering_cases() {
  static std::vector<Case> c;
  return c;
}
struct Reject {
  std::string name;
  std::string must_contain;
  std::function<void(Kit&)> build;
};
std::vector<Reject>& reject_cases() {
  static std::vector<Reject> c;
  return c;
}

void lc(std::string name, std::string doc, std::function<void(Kit&)> build) {
  lowering_cases().push_back(Case{std::move(name), std::move(doc), std::move(build)});
}
[[maybe_unused]] void rc(std::string name, std::string must_contain, std::function<void(Kit&)> build) {
  reject_cases().push_back(Reject{std::move(name), std::move(must_contain), std::move(build)});
}

// ---- the cases ------------------------------------------------------------------------------------------------

// `name(a: T, b: T) returns T { return a OP b; }` with an optional `requires`.
void binop_fn(Kit& k, Type t, BinaryOp op, const std::string& lo, const std::string& hi, bool bounded, bool want_fail,
              const char* category) {
  Function* f = k.fn("f", t, {{"a", t}, {"b", t}});
  if (bounded) {
    k.req(f, k.range(k.F(f, 0), t, lo, hi));
    k.req(f, k.range(k.F(f, 1), t, lo, hi));
  }
  const Expr* e = k.bin(op, k.pv(f, 0), k.pv(f, 1));
  f->body = k.blk({k.ret(e)});
  k.done(f);
  if (want_fail) k.fail(category, e->loc);
}

void define_arithmetic() {
  lc("add_i32_unbounded", "a + b on int: may overflow", [](Kit& k) { binop_fn(k, k.i32, BinaryOp::Add, "", "", false, true, "overflow"); });
  lc("add_i32_bounded", "a + b on int with both in [0, 1000]: cannot overflow",
     [](Kit& k) { binop_fn(k, k.i32, BinaryOp::Add, "0", "1000", true, false, ""); });
  lc("sub_i32_unbounded", "a - b on int: may overflow", [](Kit& k) { binop_fn(k, k.i32, BinaryOp::Sub, "", "", false, true, "overflow"); });
  lc("sub_i32_bounded", "a - b with both in [-1000, 1000]",
     [](Kit& k) { binop_fn(k, k.i32, BinaryOp::Sub, "-1000", "1000", true, false, ""); });
  lc("mul_i32_unbounded", "a * b on int: may overflow", [](Kit& k) { binop_fn(k, k.i32, BinaryOp::Mul, "", "", false, true, "overflow"); });
  lc("mul_i32_bounded", "a * b with both in [-40000, 40000]: fits (1.6e9 < 2^31)",
     [](Kit& k) { binop_fn(k, k.i32, BinaryOp::Mul, "-40000", "40000", true, false, ""); });
  lc("mul_i32_just_too_big", "a * b with both in [-46341, 46341]: 46341^2 > INT_MAX",
     [](Kit& k) { binop_fn(k, k.i32, BinaryOp::Mul, "-46341", "46341", true, true, "overflow"); });
  lc("add_i64_unbounded", "a + b on long", [](Kit& k) { binop_fn(k, k.i64, BinaryOp::Add, "", "", false, true, "overflow"); });
  lc("add_i128_unbounded", "a + b on __int128", [](Kit& k) { binop_fn(k, k.i128, BinaryOp::Add, "", "", false, true, "overflow"); });
  lc("add_i128_bounded", "a + b on __int128 with both below 2^100",
     [](Kit& k) { binop_fn(k, k.i128, BinaryOp::Add, "0", "1267650600228229401496703205376", true, false, ""); });
  lc("neg_i32_unbounded", "-a on int: overflows for INT_MIN", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    const Expr* e = k.neg(k.pv(f, 0));
    f->body = k.blk({k.ret(e)});
    k.done(f);
    k.fail("overflow", e->loc);
  });
  lc("neg_i32_bounded", "-a on int with a > INT_MIN", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.req(f, k.ge_c(k.F(f, 0), k.i32, "-2147483647"));
    f->body = k.blk({k.ret(k.neg(k.pv(f, 0)))});
    k.done(f);
  });
  lc("u32_wraps", "unsigned arithmetic is modular: a + 1 == 0 at UINT_MAX, 0 - 1 == UINT_MAX, -1u == UINT_MAX", [](Kit& k) {
    Function* f = k.fn("f", k.u32, {{"a", k.u32}});
    k.ens(f, k.implies(k.eq_c(k.F(f, 0), k.u32, "4294967295"), k.eq_c(k.R(f), k.u32, "0")));
    k.ens(f, k.implies(k.lt(k.F(f, 0), k.num(k.u32, "4294967295")), k.eq(k.cv(k.u64, k.R(f)), k.add(k.cv(k.u64, k.F(f, 0)), k.num(k.u64, "1")))));
    f->body = k.blk({k.chk(k.eq_c(k.sub(k.num(k.u32, "0"), k.num(k.u32, "1")), k.u32, "4294967295")),
                     k.chk(k.eq_c(k.neg(k.num(k.u32, "1")), k.u32, "4294967295")),
                     k.chk(k.eq_c(k.mul(k.num(k.u32, "65536"), k.num(k.u32, "65536")), k.u32, "0")),
                     k.ret(k.add(k.pv(f, 0), k.num(k.u32, "1")))});
    k.done(f);
  });
  lc("u32_wrap_wrong", "0 - 1 is not 0 for unsigned int", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Stmt* c = k.chk(k.eq_c(k.sub(k.num(k.u32, "0"), k.num(k.u32, "1")), k.u32, "0"));
    f->body = k.blk({c});
    k.done(f);
    k.fail("check", c->loc);
  });
  lc("u128_wraps", "unsigned __int128 wraps at 2^128", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const std::string max = "340282366920938463463374607431768211455";
    f->body = k.blk({k.chk(k.eq_c(k.add(k.num(k.u128, max), k.num(k.u128, "1")), k.u128, "0")),
                     k.chk(k.eq_c(k.sub(k.num(k.u128, "0"), k.num(k.u128, "1")), k.u128, max))});
    k.done(f);
  });
  lc("signed_does_not_wrap", "int + 1 at INT_MAX is an overflow, not INT_MIN", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Expr* e = k.add(k.num(k.i32, "2147483647"), k.num(k.i32, "1"));
    const Stmt* s = k.es(e);
    f->body = k.blk({s});
    k.done(f);
    k.fail("overflow", e->loc);
  });

  lc("div_unbounded", "a / b on int: b may be zero, and INT_MIN / -1 overflows", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}, {"b", k.i32}});
    const Expr* e = k.div(k.pv(f, 0), k.pv(f, 1));
    f->body = k.blk({k.ret(e)});
    k.done(f);
    k.fail("division-by-zero", e->loc);
    k.fail("overflow", e->loc);
  });
  lc("div_bounded", "a / b with b > 0 cannot fail", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}, {"b", k.i32}});
    k.req(f, k.gt(k.F(f, 1), k.num(k.i32, "0")));
    f->body = k.blk({k.ret(k.div(k.pv(f, 0), k.pv(f, 1)))});
    k.done(f);
  });
  lc("div_min_by_minus_one", "a / b with b != 0 still overflows for INT_MIN / -1", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}, {"b", k.i32}});
    k.req(f, k.ne(k.F(f, 1), k.num(k.i32, "0")));
    const Expr* e = k.div(k.pv(f, 0), k.pv(f, 1));
    f->body = k.blk({k.ret(e)});
    k.done(f);
    k.fail("overflow", e->loc);
  });
  lc("rem_unbounded", "a % b on int: zero divisor and INT_MIN % -1", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}, {"b", k.i32}});
    const Expr* e = k.rem(k.pv(f, 0), k.pv(f, 1));
    f->body = k.blk({k.ret(e)});
    k.done(f);
    k.fail("division-by-zero", e->loc);
    k.fail("overflow", e->loc);
  });
  lc("rem_bounded", "a % b with b > 0", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}, {"b", k.i32}});
    k.req(f, k.gt(k.F(f, 1), k.num(k.i32, "0")));
    k.ens(f, k.land(k.gt(k.R(f), k.neg(k.F(f, 1))), k.lt(k.R(f), k.F(f, 1))));
    f->body = k.blk({k.ret(k.rem(k.pv(f, 0), k.pv(f, 1)))});
    k.done(f);
  });
  lc("u32_div_zero", "a / b on unsigned: only the zero divisor", [](Kit& k) {
    Function* f = k.fn("f", k.u32, {{"a", k.u32}, {"b", k.u32}});
    const Expr* e = k.div(k.pv(f, 0), k.pv(f, 1));
    f->body = k.blk({k.ret(e)});
    k.done(f);
    k.fail("division-by-zero", e->loc);
  });
  lc("u32_div_rem_values", "unsigned / and % values: 7u / 2u == 3u, 7u % 2u == 1u, 4000000000u / 3u, 4000000000u % 3u", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    auto u = [&](const char* v) { return k.num(k.u32, v); };
    f->body = k.blk({k.chk(k.eq_c(k.div(u("7"), u("2")), k.u32, "3")), k.chk(k.eq_c(k.rem(u("7"), u("2")), k.u32, "1")),
                     k.chk(k.eq_c(k.div(u("4000000000"), u("3")), k.u32, "1333333333")),
                     k.chk(k.eq_c(k.rem(u("4000000000"), u("3")), k.u32, "1")), k.chk(k.eq_c(k.div(u("0"), u("5")), k.u32, "0")),
                     k.chk(k.eq_c(k.rem(u("5"), u("5")), k.u32, "0")), k.chk(k.eq_c(k.div(u("4294967295"), u("1")), k.u32, "4294967295"))});
    k.done(f);
  });
  lc("u32_div_value_wrong", "7u / 2u is 3u, not 4u", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Stmt* s = k.chk(k.eq_c(k.div(k.num(k.u32, "7"), k.num(k.u32, "2")), k.u32, "4"));
    f->body = k.blk({s});
    k.done(f);
    k.fail("check", s->loc);
  });
  lc("u32_rem_value_wrong", "7u % 2u is 1u, not 0u", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Stmt* s = k.chk(k.eq_c(k.rem(k.num(k.u32, "7"), k.num(k.u32, "2")), k.u32, "0"));
    f->body = k.blk({s});
    k.done(f);
    k.fail("check", s->loc);
  });
  lc("u32_neg_wrong", "-1u is 4294967295u, not 1u (unary minus on an unsigned type wraps)", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Stmt* s = k.chk(k.eq_c(k.neg(k.num(k.u32, "1")), k.u32, "1"));
    f->body = k.blk({s});
    k.done(f);
    k.fail("check", s->loc);
  });
  lc("div_rem_semantics", "C division truncates toward zero and % takes the sign of the dividend", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    auto c = [&](const char* a, const char* b) { return std::make_pair(k.num(k.i32, a), k.num(k.i32, b)); };
    auto p1 = c("-7", "2"), p2 = c("7", "-2"), p3 = c("-7", "2"), p4 = c("7", "-2"), p5 = c("-7", "-2"), p6 = c("7", "2"), p7 = c("-1", "2");
    f->body = k.blk({k.chk(k.eq_c(k.div(p1.first, p1.second), k.i32, "-3")), k.chk(k.eq_c(k.div(p2.first, p2.second), k.i32, "-3")),
                     k.chk(k.eq_c(k.rem(p3.first, p3.second), k.i32, "-1")), k.chk(k.eq_c(k.rem(p4.first, p4.second), k.i32, "1")),
                     k.chk(k.eq_c(k.div(p5.first, p5.second), k.i32, "3")), k.chk(k.eq_c(k.rem(p5.first, p5.second), k.i32, "-1")),
                     k.chk(k.eq_c(k.div(p6.first, p6.second), k.i32, "3")), k.chk(k.eq_c(k.rem(p6.first, p6.second), k.i32, "1")),
                     k.chk(k.eq_c(k.div(p7.first, p7.second), k.i32, "0")), k.chk(k.eq_c(k.rem(p7.first, p7.second), k.i32, "-1"))});
    k.done(f);
  });
  lc("div_semantics_wrong", "-7 / 2 is -3, not -4 (that would be floor division)", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Stmt* s = k.chk(k.eq_c(k.div(k.num(k.i32, "-7"), k.num(k.i32, "2")), k.i32, "-4"));
    f->body = k.blk({s});
    k.done(f);
    k.fail("check", s->loc);
  });
  lc("rem_semantics_wrong", "-7 % 2 is -1, not 1", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Stmt* s = k.chk(k.eq_c(k.rem(k.num(k.i32, "-7"), k.num(k.i32, "2")), k.i32, "1"));
    f->body = k.blk({s});
    k.done(f);
    k.fail("check", s->loc);
  });
}

void define_shifts() {
  lc("shl_unbounded", "a << n on int: bad count, negative operand, and overflow of the result", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}, {"n", k.i32}});
    const Expr* e = k.shl(k.pv(f, 0), k.pv(f, 1));
    f->body = k.blk({k.ret(e)});
    k.done(f);
    k.fail("shift", e->loc, 3);
  });
  lc("shl_bounded", "a << n with 0 <= a <= 1000 and 0 <= n <= 10", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}, {"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "0", "1000"));
    k.req(f, k.range(k.F(f, 1), k.i32, "0", "10"));
    f->body = k.blk({k.ret(k.shl(k.pv(f, 0), k.pv(f, 1)))});
    k.done(f);
  });
  lc("shl_negative_operand", "a << n with a possibly negative", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}, {"n", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "-5", "5"));
    k.req(f, k.range(k.F(f, 1), k.i32, "0", "3"));
    const Expr* e = k.shl(k.pv(f, 0), k.pv(f, 1));
    f->body = k.blk({k.ret(e)});
    k.done(f);
    k.fail("shift", e->loc, 1, "", "left operand");
  });
  lc("shl_result_does_not_fit", "a << n with a >= 0 but a * 2^n too big", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}, {"n", k.i32}});
    k.req(f, k.ge_c(k.F(f, 0), k.i32, "0"));
    k.req(f, k.range(k.F(f, 1), k.i32, "0", "31"));
    const Expr* e = k.shl(k.pv(f, 0), k.pv(f, 1));
    f->body = k.blk({k.ret(e)});
    k.done(f);
    k.fail("shift", e->loc, 1, "", "may not fit");
  });
  lc("shl_into_sign_bit", "1 << 31 does not fit int (C23 6.5.8, used for C++ too)", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Expr* e = k.shl(k.num(k.i32, "1"), k.num(k.i32, "31"));
    f->body = k.blk({k.es(e)});
    k.done(f);
    k.fail("shift", e->loc, 1, "", "may not fit");
  });
  lc("shl_30_fits", "1 << 30 fits int, and means 2^30", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.chk(k.eq_c(k.shl(k.num(k.i32, "1"), k.num(k.i32, "30")), k.i32, "1073741824"))});
    k.done(f);
  });
  lc("shl_count_too_big", "1 << 32 on int: the count is out of range", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Expr* e = k.shl(k.num(k.i32, "1"), k.num(k.i32, "32"));
    f->body = k.blk({k.es(e)});
    k.done(f);
    k.fail("shift", e->loc, 1, "", "shift count");
  });
  lc("shr_semantics", "a >> n is floor(a / 2^n): -7 >> 1 == -4, 7 >> 1 == 3, -1 >> 31 == -1", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.chk(k.eq_c(k.shr(k.num(k.i32, "-7"), k.num(k.i32, "1")), k.i32, "-4")),
                     k.chk(k.eq_c(k.shr(k.num(k.i32, "7"), k.num(k.i32, "1")), k.i32, "3")),
                     k.chk(k.eq_c(k.shr(k.num(k.i32, "-1"), k.num(k.i32, "31")), k.i32, "-1")),
                     k.chk(k.eq_c(k.shr(k.num(k.u32, "4294967295"), k.num(k.i32, "31")), k.u32, "1"))});
    k.done(f);
  });
  lc("shr_wrong", "-7 >> 1 is -4, not -3", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Stmt* s = k.chk(k.eq_c(k.shr(k.num(k.i32, "-7"), k.num(k.i32, "1")), k.i32, "-3"));
    f->body = k.blk({s});
    k.done(f);
    k.fail("check", s->loc);
  });
  lc("shr_count_unbounded", "a >> n with an unconstrained count", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}, {"n", k.i32}});
    const Expr* e = k.shr(k.pv(f, 0), k.pv(f, 1));
    f->body = k.blk({k.ret(e)});
    k.done(f);
    k.fail("shift", e->loc, 1);
  });
  lc("shr_in_range", "a >> n stays within int for any a and 0 <= n < 32", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}, {"n", k.i32}});
    k.req(f, k.range(k.F(f, 1), k.i32, "0", "31"));
    k.ens(f, k.le(k.R(f), k.cond(k.ge_c(k.F(f, 0), k.i32, "0"), k.F(f, 0), k.num(k.i32, "0"))));
    f->body = k.blk({k.ret(k.shr(k.pv(f, 0), k.pv(f, 1)))});
    k.done(f);
  });
  lc("shl_u32_wraps", "unsigned << wraps: 3u << 31 == 2^31, and the count is still checked", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.chk(k.eq_c(k.shl(k.num(k.u32, "3"), k.num(k.i32, "31")), k.u32, "2147483648")),
                     k.chk(k.eq_c(k.shl(k.num(k.u32, "1"), k.num(k.i32, "31")), k.u32, "2147483648"))});
    k.done(f);
  });
  lc("shl_u32_bad_count", "1u << 32: the count is out of range", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Expr* e = k.shl(k.num(k.u32, "1"), k.num(k.i32, "32"));
    f->body = k.blk({k.es(e)});
    k.done(f);
    k.fail("shift", e->loc, 1);
  });
  lc("shl_u64_count_type_differs", "the count has its own type: 1ul << (unsigned char)40", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.chk(k.eq_c(k.shl(k.num(k.u64, "1"), k.num(k.u8, "40")), k.u64, "1099511627776"))});
    k.done(f);
  });
}

void define_conversions() {
  lc("conv_i32_to_i8_unbounded", "(signed char)a: may not fit", [](Kit& k) {
    Function* f = k.fn("f", k.i8, {{"a", k.i32}});
    const Expr* e = k.cv(k.i8, k.pv(f, 0));
    f->body = k.blk({k.ret(e)});
    k.done(f);
    k.fail("conversion", e->loc);
  });
  lc("conv_i32_to_i8_bounded", "(signed char)a with a in [-128, 127]", [](Kit& k) {
    Function* f = k.fn("f", k.i8, {{"a", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "-128", "127"));
    k.ens(f, k.eq(k.cv(k.i32, k.R(f)), k.F(f, 0)));
    f->body = k.blk({k.ret(k.cv(k.i8, k.pv(f, 0)))});
    k.done(f);
  });
  lc("conv_i32_to_i8_off_by_one", "(signed char)a with a in [-128, 128]", [](Kit& k) {
    Function* f = k.fn("f", k.i8, {{"a", k.i32}});
    k.req(f, k.range(k.F(f, 0), k.i32, "-128", "128"));
    const Expr* e = k.cv(k.i8, k.pv(f, 0));
    f->body = k.blk({k.ret(e)});
    k.done(f);
    k.fail("conversion", e->loc);
  });
  lc("conv_to_unsigned_wraps", "(unsigned)a is a modulo 2^32: -1 becomes UINT_MAX", [](Kit& k) {
    Function* f = k.fn("f", k.u32, {{"a", k.i32}});
    k.ens(f, k.implies(k.ge_c(k.F(f, 0), k.i32, "0"), k.eq(k.cv(k.i64, k.R(f)), k.cv(k.i64, k.F(f, 0)))));
    k.ens(f, k.implies(k.lt(k.F(f, 0), k.num(k.i32, "0")), k.eq(k.cv(k.i64, k.R(f)), k.add(k.cv(k.i64, k.F(f, 0)), k.num(k.i64, "4294967296")))));
    f->body = k.blk({k.chk(k.eq_c(k.cv(k.u32, k.num(k.i32, "-1")), k.u32, "4294967295")), k.ret(k.cv(k.u32, k.pv(f, 0)))});
    k.done(f);
  });
  lc("conv_to_unsigned_wrong", "(unsigned char)300 is 44, not 300", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Stmt* s = k.chk(k.eq_c(k.cv(k.u8, k.num(k.i32, "300")), k.u8, "255"));
    f->body = k.blk({k.chk(k.eq_c(k.cv(k.u8, k.num(k.i32, "300")), k.u8, "44")), s});
    k.done(f);
    k.fail("check", s->loc);
  });
  lc("conv_u32_to_i32_unbounded", "(int)u: may exceed INT_MAX", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"u", k.u32}});
    const Expr* e = k.cv(k.i32, k.pv(f, 0));
    f->body = k.blk({k.ret(e)});
    k.done(f);
    k.fail("conversion", e->loc);
  });
  lc("conv_u32_to_i32_bounded", "(int)u with u <= INT_MAX", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"u", k.u32}});
    k.req(f, k.le_c(k.F(f, 0), k.u32, "2147483647"));
    f->body = k.blk({k.ret(k.cv(k.i32, k.pv(f, 0)))});
    k.done(f);
  });
  lc("conv_widening_is_free", "int -> long, signed char -> int, unsigned -> long long: no obligation", [](Kit& k) {
    Function* f = k.fn("f", k.i64, {{"a", k.i32}, {"c", k.i8}, {"u", k.u32}});
    k.ens(f, k.eq(k.R(f), k.add(k.add(k.cv(k.i64, k.F(f, 0)), k.cv(k.i64, k.cv(k.i32, k.F(f, 1)))), k.cv(k.i64, k.F(f, 2)))));
    f->body = k.blk({k.ret(k.add(k.add(k.cv(k.i64, k.pv(f, 0)), k.cv(k.i64, k.cv(k.i32, k.pv(f, 1)))), k.cv(k.i64, k.pv(f, 2))))});
    k.done(f);
  });
  lc("conv_u64_to_i64_unbounded", "(long)u on 64 bits", [](Kit& k) {
    Function* f = k.fn("f", k.i64, {{"u", k.u64}});
    const Expr* e = k.cv(k.i64, k.pv(f, 0));
    f->body = k.blk({k.ret(e)});
    k.done(f);
    k.fail("conversion", e->loc);
  });
  lc("conv_i128_to_u128_wraps", "(unsigned __int128)(__int128)-1 == 2^128 - 1", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.chk(k.eq_c(k.cv(k.u128, k.num(k.i128, "-1")), k.u128, "340282366920938463463374607431768211455"))});
    k.done(f);
  });
  lc("conv_same_shape_is_identity", "unsigned -> unsigned of the same width needs nothing", [](Kit& k) {
    Function* f = k.fn("f", k.u32, {{"u", k.u32}});
    k.ens(f, k.eq(k.R(f), k.F(f, 0)));
    f->body = k.blk({k.ret(k.cv(k.u32, k.pv(f, 0)))});
    k.done(f);
  });
}

void define_bool_and_guards() {
  lc("bool_conversions", "to_bool(a) is a != 0; bool_to_int(c) is 0 or 1", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}, {"b", k.i32}});
    k.ens(f, k.lor(k.eq_c(k.R(f), k.i32, "0"), k.eq_c(k.R(f), k.i32, "1")));
    k.ens(f, k.eq(k.ne(k.R(f), k.num(k.i32, "0")), k.lt(k.F(f, 0), k.F(f, 1))));
    f->body = k.blk({k.chk(k.tb(k.num(k.i32, "5"))), k.chk(k.lnot(k.tb(k.num(k.i32, "0")))), k.ret(k.bi(k.i32, k.lt(k.pv(f, 0), k.pv(f, 1))))});
    k.done(f);
  });
  lc("bool_to_int_wrong", "bool_to_int(true) is 1, not 2", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Stmt* s = k.chk(k.eq_c(k.bi(k.i32, k.yes()), k.i32, "2"));
    f->body = k.blk({s});
    k.done(f);
    k.fail("check", s->loc);
  });
  lc("bool_params_and_results", "bool formals, locals and results", [](Kit& k) {
    Function* f = k.fn("f", k.bl, {{"a", k.bl}, {"b", k.bl}});
    k.ens(f, k.eq(k.R(f), k.lor(k.F(f, 0), k.F(f, 1))));
    Var* t = k.local(f, "t", k.bl);
    f->body = k.blk({k.decl(t, k.lor(k.pv(f, 0), k.pv(f, 1))), k.ret(k.v(t))});
    k.done(f);
  });
  lc("and_guards_division", "a != 0 && 100 / a > 1: the division is evaluated only when a != 0", [](Kit& k) {
    Function* f = k.fn("f", k.bl, {{"a", k.i32}});
    f->body = k.blk({k.ret(k.land(k.ne(k.pv(f, 0), k.num(k.i32, "0")), k.gt(k.div(k.num(k.i32, "100"), k.pv(f, 0)), k.num(k.i32, "1"))))});
    k.done(f);
  });
  lc("or_guards_division", "a == 0 || 100 / a > 1", [](Kit& k) {
    Function* f = k.fn("f", k.bl, {{"a", k.i32}});
    f->body = k.blk({k.ret(k.lor(k.eq_c(k.pv(f, 0), k.i32, "0"), k.gt(k.div(k.num(k.i32, "100"), k.pv(f, 0)), k.num(k.i32, "1"))))});
    k.done(f);
  });
  lc("and_wrong_order", "100 / a > 1 && a != 0: the division comes first and is unguarded", [](Kit& k) {
    Function* f = k.fn("f", k.bl, {{"a", k.i32}});
    const Expr* d = k.div(k.num(k.i32, "100"), k.pv(f, 0));
    f->body = k.blk({k.ret(k.land(k.gt(d, k.num(k.i32, "1")), k.ne(k.pv(f, 0), k.num(k.i32, "0"))))});
    k.done(f);
    k.fail("division-by-zero", d->loc);
  });
  lc("and_chain_guards", "a > 0 && a < 10 && 100 / a > 1", [](Kit& k) {
    Function* f = k.fn("f", k.bl, {{"a", k.i32}});
    const Expr* g = k.land(k.land(k.gt(k.pv(f, 0), k.num(k.i32, "0")), k.lt(k.pv(f, 0), k.num(k.i32, "10"))),
                           k.gt(k.div(k.num(k.i32, "100"), k.pv(f, 0)), k.num(k.i32, "1")));
    f->body = k.blk({k.ret(g)});
    k.done(f);
  });
  lc("cond_guards_branches", "a != 0 ? 100 / a : 0", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    f->body = k.blk({k.ret(k.cond(k.ne(k.pv(f, 0), k.num(k.i32, "0")), k.div(k.num(k.i32, "100"), k.pv(f, 0)), k.num(k.i32, "0")))});
    k.done(f);
  });
  lc("cond_wrong_branch", "a == 0 ? 100 / a : 0 divides in the wrong branch", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    const Expr* d = k.div(k.num(k.i32, "100"), k.pv(f, 0));
    f->body = k.blk({k.ret(k.cond(k.eq_c(k.pv(f, 0), k.i32, "0"), d, k.num(k.i32, "0")))});
    k.done(f);
    k.fail("division-by-zero", d->loc);
  });
  lc("cond_else_branch_guard", "a == 0 ? 0 : 100 / a: the else branch is guarded by !c", [](Kit& k) {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    f->body = k.blk({k.ret(k.cond(k.eq_c(k.pv(f, 0), k.i32, "0"), k.num(k.i32, "0"), k.div(k.num(k.i32, "100"), k.pv(f, 0))))});
    k.done(f);
  });
  lc("unguarded_division_in_comparison", "100 / a > 1 alone", [](Kit& k) {
    Function* f = k.fn("f", k.bl, {{"a", k.i32}});
    const Expr* d = k.div(k.num(k.i32, "100"), k.pv(f, 0));
    f->body = k.blk({k.ret(k.gt(d, k.num(k.i32, "1")))});
    k.done(f);
    k.fail("division-by-zero", d->loc);
  });
  lc("or_guard_does_not_cover_left", "100 / a > 1 || a == 0: the left operand is unguarded", [](Kit& k) {
    Function* f = k.fn("f", k.bl, {{"a", k.i32}});
    const Expr* d = k.div(k.num(k.i32, "100"), k.pv(f, 0));
    f->body = k.blk({k.ret(k.lor(k.gt(d, k.num(k.i32, "1")), k.eq_c(k.pv(f, 0), k.i32, "0")))});
    k.done(f);
    k.fail("division-by-zero", d->loc);
  });
  lc("comparison_semantics", "signed and unsigned comparisons and equality", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    f->body = k.blk({k.chk(k.lt(k.num(k.i32, "-1"), k.num(k.i32, "0"))), k.chk(k.gt(k.num(k.u32, "4294967295"), k.num(k.u32, "0"))),
                     k.chk(k.le(k.num(k.i8, "-128"), k.num(k.i8, "-128"))), k.chk(k.ge(k.num(k.i8, "127"), k.num(k.i8, "126"))),
                     k.chk(k.ne(k.num(k.i32, "1"), k.num(k.i32, "2"))), k.chk(k.eq(k.yes(), k.yes())), k.chk(k.ne(k.yes(), k.no()))});
    k.done(f);
  });
  lc("comparison_wrong", "-1 < 0 is true, so check(-1 >= 0) fails", [](Kit& k) {
    Function* f = k.fn("f", k.vd);
    const Stmt* s = k.chk(k.ge(k.num(k.i32, "-1"), k.num(k.i32, "0")));
    f->body = k.blk({s});
    k.done(f);
    k.fail("check", s->loc);
  });
}

}  // namespace

// The rest of the cases are in selftest_cases2.inc, included here so that this file stays one translation unit.
namespace {
void define_calls_loops_and_more();
void define_rejections();
}  // namespace

#include "selftest_cases2.inc"

namespace {

// ---- running ------------------------------------------------------------------------------------------------------

struct MapRow {
  int vpr_line = 0;
  std::string id, category, file, msg;
  int line = 0;
  int note_line = -1;  // -1: no note
};

std::vector<MapRow> parse_map(const std::string& text) {
  std::vector<MapRow> rows;
  std::istringstream in(text);
  std::string l;
  while (std::getline(in, l)) {
    std::vector<std::string> f;
    size_t p = 0;
    while (true) {
      size_t q = l.find('\t', p);
      f.push_back(l.substr(p, q == std::string::npos ? std::string::npos : q - p));
      if (q == std::string::npos) break;
      p = q + 1;
    }
    if (f.size() < 7) continue;
    MapRow r;
    r.vpr_line = std::stoi(f[0]);
    r.id = f[1];
    r.category = f[2];
    r.file = f[3];
    r.line = std::stoi(f[4]);
    r.msg = f[6];
    if (f.size() >= 8) {
      const size_t a = f[7].find(':'), b = a == std::string::npos ? a : f[7].find(':', a + 1);
      if (b != std::string::npos) r.note_line = std::stoi(f[7].substr(a + 1, b - a - 1));
    }
    rows.push_back(r);
  }
  return rows;
}

const char* harness_category(const std::string& viper_id) {
  static const std::map<std::string, const char*> t = {
      {"assert.failed", "assert"}, {"call.precondition", "precondition"}, {"postcondition.violated", "postcondition"},
      {"invariant.not.established", "invariant-entry"}, {"invariant.not.preserved", "invariant-preserved"},
      {"not.wellformed", "wellformedness"}, {"termination.failed", "termination"}};
  auto it = t.find(viper_id);
  return it == t.end() ? nullptr : it->second;
}

bool run_lowering_case(const Case& c, const std::string& dir, std::string& problem) {
  Kit k;
  c.build(k);
  std::vector<Diag> diags = check_program(k.prog);
  if (!diags.empty()) {
    problem = "check_program rejected the program: " + diags[0].reason + " (line " + std::to_string(diags[0].loc.line) + ")";
    return false;
  }
  LowerResult lr = lower_program(k.prog, k.lower_options);
  if (!lr.errors.empty()) {
    problem = "lowering reported: " + lr.errors[0].reason;
    return false;
  }
  cerastes::viper::Emitted first = cerastes::viper::emit(lr.program);
  const std::vector<MapRow> rows = parse_map(first.map);
  // Which Viper lines must fail.
  std::vector<std::pair<int, std::string>> fails;  // (relative vpr line, harness category)
  for (const Expect& e : k.expects) {
    std::vector<const MapRow*> hits;
    for (const MapRow& r : rows)
      if (r.category == e.category && r.line == e.line && (e.viper_id.empty() || r.id == e.viper_id) &&
          (e.msg.empty() || r.msg.find(e.msg) != std::string::npos) && (e.note_line < 0 || r.note_line == e.note_line))
        hits.push_back(&r);
    if (hits.size() != e.count) {
      problem = "expected " + std::to_string(e.count) + " check(s) of category " + e.category + " at line " + std::to_string(e.line) +
                " in the position map, found " + std::to_string(hits.size());
      return false;
    }
    for (const MapRow* r : hits) {
      const char* hc = harness_category(r->id);
      if (hc == nullptr) {
        problem = "no harness category for Viper error id " + r->id;
        return false;
      }
      fails.emplace_back(r->vpr_line, hc);
    }
  }
  for (const auto& [category, msg] : k.map_requires) {
    bool found = false;
    for (const MapRow& r : rows) found = found || (r.category == category && r.msg.find(msg) != std::string::npos);
    if (!found) {
      problem = "the position map has no row of category " + category + " with a message containing \"" + msg + "\"";
      return false;
    }
  }
  std::sort(fails.begin(), fails.end());
  // The header: one doc line, then the expectations; its length shifts every line.
  const size_t header_lines = 1 + std::max<size_t>(fails.size(), 1);
  std::string header = "// " + c.doc + "\n";
  if (fails.empty()) header += "// expect: verified\n";
  for (const auto& f : fails) header += "// expect: error " + f.second + " @ L" + std::to_string(f.first + static_cast<int>(header_lines)) + "\n";
  // `// expect:` lines must come first in the leading comment block? The harness reads the whole block.
  cerastes::viper::EmitOptions eo;
  eo.header = header;
  cerastes::viper::Emitted out = cerastes::viper::emit(lr.program, eo);
  for (const auto& [suffix, text] : {std::pair<const char*, const std::string&>{".vpr", out.vpr},
                                      std::pair<const char*, const std::string&>{".vpr.map", out.map}}) {
    const std::string path = dir + "/" + c.name + suffix;
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    f << text;
    f.flush();
    if (!f) {
      problem = "cannot write " + path;
      return false;
    }
  }
  return true;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::fprintf(stderr, "usage: cerastes-vir-selftest <output-dir>\n");
    return 2;
  }
  const std::string dir = argv[1];
  {
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec || !std::filesystem::is_directory(dir)) {
      std::fprintf(stderr, "cerastes-vir-selftest: cannot create the output directory %s\n", dir.c_str());
      return 2;
    }
  }
  define_arithmetic();
  define_shifts();
  define_conversions();
  define_bool_and_guards();
  define_calls_loops_and_more();
  define_rejections();
  int bad = 0;
  for (const Case& c : lowering_cases()) {
    std::string problem;
    if (!run_lowering_case(c, dir, problem)) {
      std::fprintf(stderr, "FAIL (lowering) %s: %s\n", c.name.c_str(), problem.c_str());
      ++bad;
    }
  }
  for (const Reject& r : reject_cases()) {
    Kit k;
    r.build(k);
    const std::vector<Diag> d = check_program(k.prog);
    bool ok = false;
    for (const Diag& x : d)
      if (x.reason.find(r.must_contain) != std::string::npos) ok = true;
    if (!ok) {
      std::fprintf(stderr, "FAIL (rejection) %s: expected a diagnostic containing \"%s\", got %zu: %s\n", r.name.c_str(),
                   r.must_contain.c_str(), d.size(), d.empty() ? "(none)" : d[0].reason.c_str());
      ++bad;
    }
  }
  std::printf("cerastes-vir-selftest: %zu lowering cases written to %s, %zu rejection cases checked, %d problems\n",
              lowering_cases().size(), dir.c_str(), reject_cases().size(), bad);
  return bad == 0 ? 0 : 1;
}
