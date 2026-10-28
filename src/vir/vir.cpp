// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "vir.h"

namespace cerastes::vir {

std::string Type::str() const {
  switch (kind) {
    case Kind::Void: return "void";
    case Kind::Bool: return "bool";
    case Kind::Int: return int_type != nullptr ? int_type->name : "<int>";
    case Kind::Ptr: return (pointee != nullptr ? pointee->str() : std::string("<?>")) + "*";
  }
  return "?";
}

int Function::add_param(std::string name, Type type, SrcLoc loc) {
  const int k = static_cast<int>(params.size());
  params.push_back(Param{name, type, loc, std::string()});
  Var* v = new_local(name.empty() ? "arg" + std::to_string(k) : name, type, std::move(loc));
  v->is_param_copy = true;
  v->param_index = k;
  param_vars_.push_back(locals_.size() - 1);
  return k;
}

Var* Function::new_local(std::string name, Type type, SrcLoc loc) {
  auto v = std::make_unique<Var>();
  v->id = static_cast<int>(locals_.size());
  v->name = std::move(name);
  v->type = type;
  v->loc = std::move(loc);
  locals_.push_back(std::move(v));
  return locals_.back().get();
}

void Function::mark_address_taken(const Var* v) {
  if (v == nullptr || v->id < 0 || static_cast<size_t>(v->id) >= locals_.size() || locals_[static_cast<size_t>(v->id)].get() != v) return;
  locals_[static_cast<size_t>(v->id)]->address_taken = true;
}

Type Program::pointer_to(const Type& pointee) {
  for (const Type& t : pointees_)
    if (t == pointee) return Type{Type::Kind::Ptr, nullptr, &t};
  pointees_.push_back(pointee);
  return Type{Type::Kind::Ptr, nullptr, &pointees_.back()};
}

const IntType* Program::int_type(const std::string& name, int bits, bool is_signed) {
  for (const auto& t : int_types_)
    if (t->bits == bits && t->is_signed == is_signed) return t.get();
  auto t = std::make_unique<IntType>();
  t->name = name;
  t->bits = bits;
  t->is_signed = is_signed;
  int_types_.push_back(std::move(t));
  return int_types_.back().get();
}

const IntType* Program::int_type_of(int bits, bool is_signed) {
  return int_type((is_signed ? "int" : "uint") + std::to_string(bits), bits, is_signed);
}

const IntType* Program::enum_type(const std::string& name, int bits, bool is_signed, bool is_scoped, const std::string& lo,
                                  const std::string& hi) {
  auto t = std::make_unique<IntType>();
  t->name = name;
  t->bits = bits;
  t->is_signed = is_signed;
  t->is_enum = true;
  t->is_scoped = is_scoped;
  t->lo = lo;
  t->hi = hi;
  enum_types_.push_back(std::move(t));
  return enum_types_.back().get();
}

Function* Program::add_function(std::string c_name, Type ret, SrcLoc loc, SrcLoc end_loc) {
  Function& f = functions_.emplace_back();
  f.id = static_cast<int>(functions_.size()) - 1;
  f.c_name = std::move(c_name);
  f.ret = ret;
  f.loc = std::move(loc);
  f.end_loc = std::move(end_loc);
  return &f;
}

NamedConst* Program::add_named_const(std::string c_name, Type type, SrcLoc loc, const Expr* init, std::string value) {
  NamedConst& c = named_consts_.emplace_back();
  c.id = static_cast<int>(named_consts_.size()) - 1;
  c.c_name = std::move(c_name);
  c.type = type;
  c.loc = std::move(loc);
  c.init = init;
  c.value = std::move(value);
  return &c;
}

NamedConst* Program::add_static_assertion(SrcLoc loc, const Expr* condition) {
  NamedConst& c = named_consts_.emplace_back();
  c.id = static_cast<int>(named_consts_.size()) - 1;
  c.c_name = "static_assert";
  c.type = Type::bool_type();
  c.loc = std::move(loc);
  c.init = condition;
  c.value = "1";
  c.kind = NamedConst::Kind::StaticAssertion;
  return &c;
}

NamedConst* Function::add_local_static_assertion(SrcLoc loc, const Expr* condition) {
  NamedConst& c = local_consts_.emplace_back();
  c.id = static_cast<int>(local_consts_.size()) - 1;
  c.c_name = "static_assert";
  c.type = Type::bool_type();
  c.loc = std::move(loc);
  c.init = condition;
  c.value = "1";
  c.kind = NamedConst::Kind::StaticAssertion;
  return &c;
}

NamedConst* Function::add_local_const(std::string c_name, Type type, SrcLoc loc, const Expr* init, std::string value) {
  NamedConst& c = local_consts_.emplace_back();
  c.id = static_cast<int>(local_consts_.size()) - 1;
  c.c_name = std::move(c_name);
  c.type = type;
  c.loc = std::move(loc);
  c.init = init;
  c.value = std::move(value);
  return &c;
}

const IntType* promoted(Program& p, const IntType* t) {
  if (t->bits < p.c_int_bits()) return p.int_type_of(p.c_int_bits(), true);
  return t;
}

const IntType* usual_arithmetic(Program& p, const IntType* a, const IntType* b) {
  if (a->is_signed == b->is_signed) return a->bits >= b->bits ? a : b;
  const IntType* s = a->is_signed ? a : b;
  const IntType* u = a->is_signed ? b : a;
  if (u->bits >= s->bits) return p.int_type_of(u->bits, false);
  return s;
}

// ---- Builder ------------------------------------------------------------------------------------------------

namespace {
Expr* fresh(Program& p, Expr::Kind k, Type t, SrcLoc loc) {
  Expr* e = p.new_expr();
  e->kind = k;
  e->type = t;
  e->loc = std::move(loc);
  return e;
}
Stmt* fresh_stmt(Program& p, Stmt::Kind k, SrcLoc loc) {
  Stmt* s = p.new_stmt();
  s->kind = k;
  s->loc = std::move(loc);
  return s;
}
}  // namespace

const Expr* Builder::int_const(Type t, std::string decimal, SrcLoc loc) {
  Expr* e = fresh(p_, Expr::Kind::Const, t, std::move(loc));
  e->value = std::move(decimal);
  return e;
}

const Expr* Builder::bool_const(bool value, SrcLoc loc) {
  Expr* e = fresh(p_, Expr::Kind::Const, Type::bool_type(), std::move(loc));
  e->value = value ? "true" : "false";
  return e;
}

const Expr* Builder::local(const Var* v, SrcLoc loc) {
  Expr* e = fresh(p_, Expr::Kind::Local, v->type, std::move(loc));
  e->var = v;
  return e;
}

const Expr* Builder::formal(const Function& f, int k, SrcLoc loc) {
  Expr* e = fresh(p_, Expr::Kind::Formal, f.params[static_cast<size_t>(k)].type, std::move(loc));
  e->index = k;
  return e;
}

const Expr* Builder::result(const Function& f, SrcLoc loc) {
  return fresh(p_, Expr::Kind::Result, f.ret, std::move(loc));
}

const Expr* Builder::old(const Expr* a, SrcLoc loc) {
  Expr* e = fresh(p_, Expr::Kind::Old, a->type, std::move(loc));
  e->a = a;
  return e;
}

const Expr* Builder::unary(UnaryOp op, const Expr* a, SrcLoc loc) {
  Expr* e = fresh(p_, Expr::Kind::Unary, op == UnaryOp::Not ? Type::bool_type() : a->type, std::move(loc));
  e->uop = op;
  e->a = a;
  return e;
}

const Expr* Builder::binary(BinaryOp op, const Expr* a, const Expr* b, SrcLoc loc) {
  Type t = a->type;
  switch (op) {
    case BinaryOp::Lt: case BinaryOp::Le: case BinaryOp::Gt: case BinaryOp::Ge:
    case BinaryOp::Eq: case BinaryOp::Ne: case BinaryOp::LAnd: case BinaryOp::LOr:
      t = Type::bool_type();
      break;
    default: break;
  }
  Expr* e = fresh(p_, Expr::Kind::Binary, t, std::move(loc));
  e->bop = op;
  e->a = a;
  e->b = b;
  return e;
}

const Expr* Builder::cond(const Expr* c, const Expr* a, const Expr* b, SrcLoc loc) {
  // A Null arm takes the other arm's pointer type (decision 0032 item 7).
  const Type t = a->kind == Expr::Kind::Null && b->type.is_ptr() ? b->type : a->type;
  Expr* e = fresh(p_, Expr::Kind::Cond, t, std::move(loc));
  e->a = c;
  e->b = a;
  e->c = b;
  return e;
}

const Expr* Builder::convert(Type to, const Expr* a, bool explicit_cast, SrcLoc loc) {
  Expr* e = fresh(p_, Expr::Kind::Convert, to, std::move(loc));
  e->a = a;
  e->explicit_cast = explicit_cast;
  return e;
}

const Expr* Builder::to_bool(const Expr* a, SrcLoc loc) {
  Expr* e = fresh(p_, Expr::Kind::ToBool, Type::bool_type(), std::move(loc));
  e->a = a;
  return e;
}

const Expr* Builder::bool_to_int(Type to, const Expr* a, SrcLoc loc) {
  Expr* e = fresh(p_, Expr::Kind::BoolToInt, to, std::move(loc));
  e->a = a;
  return e;
}

const Expr* Builder::call(const Function* callee, std::vector<const Expr*> args, SrcLoc loc) {
  Expr* e = fresh(p_, Expr::Kind::Call, callee->ret, std::move(loc));
  e->callee = callee;
  e->args = std::move(args);
  return e;
}

const Expr* Builder::report_at(const Expr* a, SrcLoc loc, std::string note) {
  Expr* e = fresh(p_, Expr::Kind::ReportAt, a->type, std::move(loc));
  e->a = a;
  e->value = std::move(note);
  return e;
}

const Expr* Builder::spelled(const Expr* e, std::string c_type_name) {
  const_cast<Expr*>(e)->c_type_name = std::move(c_type_name);
  return e;
}

const Expr* Builder::text(const Expr* e, std::string c_text) {
  const_cast<Expr*>(e)->c_text = std::move(c_text);
  return e;
}

const Expr* Builder::null_const(Type ptr_type, SrcLoc loc) {
  return fresh(p_, Expr::Kind::Null, ptr_type, std::move(loc));
}

const Expr* Builder::addr_of(const Var* v, SrcLoc loc) {
  Expr* e = fresh(p_, Expr::Kind::AddrOf, p_.pointer_to(v->type), std::move(loc));
  e->var = v;
  e->c_text = "&" + v->name;
  return e;
}

const Expr* Builder::deref(const Expr* addr, SrcLoc loc) {
  Expr* e = fresh(p_, Expr::Kind::Deref, addr->type.is_ptr() && addr->type.pointee != nullptr ? *addr->type.pointee : addr->type, std::move(loc));
  e->a = addr;
  if (!addr->c_text.empty()) e->c_text = "*" + addr->c_text;
  return e;
}

const Stmt* Builder::decl(const Var* v, const Expr* init, SrcLoc loc) {
  Stmt* s = fresh_stmt(p_, Stmt::Kind::Decl, std::move(loc));
  s->var = v;
  s->expr = init;
  return s;
}

const Stmt* Builder::assign(const Var* v, const Expr* value, SrcLoc loc) {
  Stmt* s = fresh_stmt(p_, Stmt::Kind::Assign, std::move(loc));
  s->var = v;
  s->expr = value;
  return s;
}

const Stmt* Builder::expr_stmt(const Expr* e, SrcLoc loc) {
  Stmt* s = fresh_stmt(p_, Stmt::Kind::ExprStmt, std::move(loc));
  s->expr = e;
  return s;
}

const Stmt* Builder::if_stmt(const Expr* c, const Stmt* then_body, const Stmt* else_body, SrcLoc loc) {
  Stmt* s = fresh_stmt(p_, Stmt::Kind::If, std::move(loc));
  s->expr = c;
  s->then_body = then_body;
  s->else_body = else_body;
  return s;
}

const Stmt* Builder::while_stmt(const Expr* c, std::vector<Clause> invariants, std::vector<Clause> measure, const Stmt* body, SrcLoc loc) {
  Stmt* s = fresh_stmt(p_, Stmt::Kind::While, std::move(loc));
  s->expr = c;
  s->invariants = std::move(invariants);
  s->measure = std::move(measure);
  s->body = body;
  return s;
}

const Stmt* Builder::loop(const Expr* c, std::vector<Clause> invariants, std::vector<Clause> measure, const Stmt* body, const Stmt* step,
                          LoopForm form, int jump_id, SrcLoc loc) {
  Stmt* s = fresh_stmt(p_, Stmt::Kind::While, std::move(loc));
  s->expr = c;
  s->invariants = std::move(invariants);
  s->measure = std::move(measure);
  s->body = body;
  s->step = step;
  s->form = form;
  s->jump_id = jump_id;
  return s;
}

const Stmt* Builder::switch_stmt(const Expr* scrutinee, const Stmt* body, int jump_id, SrcLoc loc) {
  Stmt* s = fresh_stmt(p_, Stmt::Kind::Switch, std::move(loc));
  s->expr = scrutinee;
  s->body = body;
  s->jump_id = jump_id;
  return s;
}

const Stmt* Builder::case_label(const Expr* value, SrcLoc loc) {
  Stmt* s = fresh_stmt(p_, Stmt::Kind::Case, std::move(loc));
  s->expr = value;
  return s;
}

const Stmt* Builder::case_label(const Expr* value, std::string folded, SrcLoc loc) {
  Stmt* s = fresh_stmt(p_, Stmt::Kind::Case, std::move(loc));
  s->expr = value;
  s->folded = std::move(folded);
  return s;
}

const Stmt* Builder::break_stmt(int target, SrcLoc loc) {
  Stmt* s = fresh_stmt(p_, Stmt::Kind::Break, std::move(loc));
  s->jump_id = target;
  return s;
}

const Stmt* Builder::continue_stmt(int target, SrcLoc loc) {
  Stmt* s = fresh_stmt(p_, Stmt::Kind::Continue, std::move(loc));
  s->jump_id = target;
  return s;
}

const Stmt* Builder::return_stmt(const Expr* value, SrcLoc loc) {
  Stmt* s = fresh_stmt(p_, Stmt::Kind::Return, std::move(loc));
  s->expr = value;
  return s;
}

const Stmt* Builder::check(const Expr* e, SrcLoc loc, bool from_assert) {
  Stmt* s = fresh_stmt(p_, Stmt::Kind::Check, std::move(loc));
  s->expr = e;
  s->from_assert = from_assert;
  return s;
}

const Stmt* Builder::goto_stmt(int label_id, SrcLoc loc) {
  Stmt* s = fresh_stmt(p_, Stmt::Kind::Goto, std::move(loc));
  s->jump_id = label_id;
  return s;
}

const Stmt* Builder::label_stmt(int label_id, std::string name, SrcLoc loc) {
  Stmt* s = fresh_stmt(p_, Stmt::Kind::Label, std::move(loc));
  s->jump_id = label_id;
  s->label_name = std::move(name);
  return s;
}

const Stmt* Builder::block(std::vector<const Stmt*> stmts, SrcLoc loc) {
  Stmt* s = fresh_stmt(p_, Stmt::Kind::Block, std::move(loc));
  s->stmts = std::move(stmts);
  return s;
}

const Stmt* Builder::block(std::vector<const Stmt*> stmts, SrcLoc loc, SrcLoc end_loc) {
  Stmt* s = const_cast<Stmt*>(block(std::move(stmts), std::move(loc)));
  s->end_loc = std::move(end_loc);
  return s;
}

const Stmt* Builder::store(const Expr* addr, const Expr* value, SrcLoc loc) {
  Stmt* s = fresh_stmt(p_, Stmt::Kind::Store, std::move(loc));
  s->addr = addr;
  s->expr = value;
  return s;
}

const Stmt* Builder::compound_store(const Expr* addr, BinaryOp op, const Expr* rhs, SrcLoc loc) {
  const Expr* cur = deref(addr, loc);  // shares `addr`
  return store(addr, compound_value(cur->type, cur, op, rhs, loc), loc);
}

const Stmt* Builder::inc_dec_store(const Expr* addr, bool increment, SrcLoc loc) {
  const IntType* c_int = p_.int_type_of(p_.c_int_bits(), true);
  const Expr* one = int_const(Type::integer(c_int), "1", loc);
  return compound_store(addr, increment ? BinaryOp::Add : BinaryOp::Sub, one, std::move(loc));
}

const Expr* Builder::compound_value(const Type& xt, const Expr* xval, BinaryOp op, const Expr* rhs, SrcLoc loc) {
  if (!xt.is_int() || !rhs->type.is_int()) return rhs;  // ill-typed on purpose
  // An enumeration operand (decision 0018 item 8): the arithmetic on it, which check_program rejects; never promoted here.
  if (xt.int_type->is_enum || rhs->type.int_type->is_enum) return binary(op, xval, rhs, loc);
  const IntType* tx = xt.int_type;
  const IntType* px = promoted(p_, tx);
  const Expr* left;
  const Expr* right;
  const IntType* c;
  if (op == BinaryOp::Shl || op == BinaryOp::Shr) {
    c = px;
    left = convert(Type::integer(c), xval, false, loc);
    right = rhs;
  } else {
    c = usual_arithmetic(p_, px, promoted(p_, rhs->type.int_type));
    left = convert(Type::integer(c), xval, false, loc);
    right = convert(Type::integer(c), rhs, false, loc);
  }
  const Expr* v = binary(op, left, right, loc);
  return convert(xt, v, false, loc);
}

const Stmt* Builder::compound_assign(const Var* x, BinaryOp op, const Expr* rhs, SrcLoc loc) {
  if (!x->type.is_int() || !rhs->type.is_int()) return assign(x, rhs, loc);  // ill-typed on purpose
  // An enumeration operand: compound_value gives the ill-typed `x op rhs`, which check_program rejects.
  const Expr* xval = local(x, loc);
  return assign(x, compound_value(x->type, xval, op, rhs, loc), loc);
}

const Stmt* Builder::inc_dec(const Var* x, bool increment, SrcLoc loc) {
  const IntType* c_int = p_.int_type_of(p_.c_int_bits(), true);
  const Expr* one = int_const(Type::integer(c_int), "1", loc);
  return compound_assign(x, increment ? BinaryOp::Add : BinaryOp::Sub, one, std::move(loc));
}

const Expr* Builder::assign_expr(const Var* v, const Expr* value, SrcLoc loc) {
  Expr* e = fresh(p_, Expr::Kind::AssignE, v->type, std::move(loc));
  e->var = v;
  e->a = value;
  return e;
}

const Expr* Builder::compound_assign_expr(const Var* x, BinaryOp op, const Expr* rhs, SrcLoc loc) {
  const Expr* xval = local(x, loc);
  return assign_expr(x, compound_value(x->type, xval, op, rhs, loc), std::move(loc));
}

const Expr* Builder::inc_dec_expr(const Var* x, bool increment, bool post, SrcLoc loc) {
  const IntType* c_int = p_.int_type_of(p_.c_int_bits(), true);
  const Expr* one = int_const(Type::integer(c_int), "1", loc);
  const Expr* old_value = local(x, loc);  // one node: the stored value and (for x++) the node's value share it
  const Expr* value = compound_value(x->type, old_value, increment ? BinaryOp::Add : BinaryOp::Sub, one, loc);
  Expr* e = const_cast<Expr*>(assign_expr(x, value, std::move(loc)));
  if (post) {
    e->b = old_value;
    e->yields_old = true;
  }
  return e;
}

const Expr* Builder::comma(const Expr* a, const Expr* b, SrcLoc loc) {
  Expr* e = fresh(p_, Expr::Kind::Comma, b->type, std::move(loc));
  e->a = a;
  e->b = b;
  return e;
}

}  // namespace cerastes::vir
