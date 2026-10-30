// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "lowered.h"

#include <stdexcept>

#include "dec.h"

namespace cerastes::low {

namespace {
std::shared_ptr<LExpr> make(LExpr::Kind k) {
  auto e = std::make_shared<LExpr>();
  e->kind = k;
  return e;
}
}  // namespace

E int_lit(std::string decimal) {
  auto e = make(LExpr::Kind::IntLit);
  e->text = std::move(decimal);
  return e;
}

E bool_lit(bool v) {
  auto e = make(LExpr::Kind::BoolLit);
  e->bool_value = v;
  return e;
}

E var(std::string name) {
  auto e = make(LExpr::Kind::Var);
  e->text = std::move(name);
  return e;
}

E neg(E a) {
  auto e = make(LExpr::Kind::Un);
  e->un = UnOp::Neg;
  e->a = std::move(a);
  return e;
}

E not_(E a) {
  auto e = make(LExpr::Kind::Un);
  e->un = UnOp::Not;
  e->a = std::move(a);
  return e;
}

E bin(BinOp op, E a, E b) {
  auto e = make(LExpr::Kind::Bin);
  e->bin = op;
  e->a = std::move(a);
  e->b = std::move(b);
  return e;
}

E ite(E c, E a, E b) {
  auto e = make(LExpr::Kind::Ite);
  e->a = std::move(c);
  e->b = std::move(a);
  e->c = std::move(b);
  return e;
}

E fn(std::string prelude_name, std::vector<E> args) {
  auto e = make(LExpr::Kind::Fn);
  e->text = std::move(prelude_name);
  e->args = std::move(args);
  return e;
}

E null_() { return make(LExpr::Kind::Null); }

E field(E recv, std::string field_name) {
  auto e = make(LExpr::Kind::Field);
  e->text = std::move(field_name);
  e->a = std::move(recv);
  return e;
}

E perm_of(E recv, std::string field_name) {
  auto e = make(LExpr::Kind::PermOf);
  e->text = std::move(field_name);
  e->a = std::move(recv);
  return e;
}

E write_perm() { return make(LExpr::Kind::Write); }
E no_perm() { return make(LExpr::Kind::NoPerm); }

E acc(E recv, std::string field_name) {
  auto e = make(LExpr::Kind::Acc);
  e->text = std::move(field_name);
  e->a = std::move(recv);
  return e;
}

E acc(E recv, std::string field_name, E amount) {
  auto e = make(LExpr::Kind::Acc);
  e->text = std::move(field_name);
  e->a = std::move(recv);
  e->b = std::move(amount);
  return e;
}

E old_(E a) {
  auto e = make(LExpr::Kind::Old);
  e->a = std::move(a);
  return e;
}

E perm_div(E a, int divisor) {
  if (divisor <= 0) throw std::logic_error("a permission is divided by a positive literal");
  return bin(BinOp::PermDiv, std::move(a), int_lit(std::to_string(divisor)));
}

bool is_domain_function(const std::string& name) {
  for (const char* n : {"loc", "len", "block_of", "offset_of", "member", "parent_of", "index_of", "lvloc"})
    if (name == n) return true;
  return name.compare(0, 3, "GB_") == 0;  // a global's block constant (decision 0041 item 4)
}

E dom(const std::string& domain_function, std::vector<E> args) {
  if (!is_domain_function(domain_function)) throw std::logic_error("`" + domain_function + "` is not a function of the Block domain");
  auto e = make(LExpr::Kind::Dom);
  e->text = domain_function;
  e->args = std::move(args);
  return e;
}

E mod_pow2(E a, int k) {
  auto e = make(LExpr::Kind::ModPow2);
  e->text = vir::dec::pow2(k);
  e->a = std::move(a);
  return e;
}

E div_pow2(E a, int k) {
  auto e = make(LExpr::Kind::DivPow2);
  e->text = vir::dec::pow2(k);
  e->a = std::move(a);
  return e;
}

E and_(E a, E b) {
  if (!a) return b;
  if (!b) return a;
  return bin(BinOp::And, std::move(a), std::move(b));
}

E implies(E guard, E c) {
  if (!guard) return c;
  return bin(BinOp::Implies, std::move(guard), std::move(c));
}

E in_range(E e, const std::string& lo, const std::string& hi) {
  return bin(BinOp::And, bin(BinOp::Le, int_lit(lo), e), bin(BinOp::Le, e, int_lit(hi)));
}

E in_range_atomic(E e, const std::string& lo, const std::string& hi) {
  return not_(bin(BinOp::Or, bin(BinOp::Lt, e, int_lit(lo)), bin(BinOp::Lt, int_lit(hi), e)));
}

}  // namespace cerastes::low
