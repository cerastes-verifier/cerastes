// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "check.h"

#include <set>
#include <unordered_set>

#include "dec.h"

namespace cerastes::vir {
namespace {

// Where an expression sits; decides which nodes are allowed.
enum class Ctx {
  Body,      // code: Local, Call allowed (not Formal, Result, Old)
  Annot,     // `check` and `invariant`: like Body, but no calls (0011 item 7)
  Requires,  // a `requires`: Formal only
  Ensures    // an `ensures`: Formal, Result, Old
};

class Checker {
 public:
  explicit Checker(const Program& p) : prog_(p) {
    for (const Function& f : p.functions()) functions_.insert(&f);
  }

  std::vector<Diag> run() {
    for (const Function& f : prog_.functions()) function(f);
    return std::move(diags_);
  }

 private:
  struct CallState {
    int seen = 0;  // calls seen so far in the current full expression (in evaluation order)
  };

  void err(const SrcLoc& loc, std::string reason) { diags_.push_back(Diag{loc, std::move(reason)}); }

  static bool value_type(const Type& t) {
    if (t.is_bool()) return true;
    return t.is_int() && t.int_type != nullptr && t.int_type->bits >= 1 && t.int_type->bits <= 128;
  }

  void function(const Function& f) {
    fn_ = &f;
    declared_.clear();
    scopes_.clear();
    scopes_.emplace_back();
    for (const auto& v : f.locals())
      if (v->is_param_copy) scopes_.back().insert(v->id);
    if (!(f.ret.is_void() || value_type(f.ret))) err(f.loc, "the return type of '" + f.c_name + "' is not void, bool or an integer type");
    for (size_t k = 0; k < f.params.size(); ++k)
      if (!value_type(f.params[k].type))
        err(f.params[k].loc, "parameter " + std::to_string(k + 1) + " of '" + f.c_name + "' is not a bool or an integer type");
    if (!f.loc.known()) err(f.loc, "function '" + f.c_name + "' has no source position");
    if (!f.end_loc.known() && f.body != nullptr) err(f.loc, "function '" + f.c_name + "' has no closing-brace position");
    if (f.body == nullptr && f.trusted_reason.empty())
      err(f.loc, "function '" + f.c_name + "' has no body and no reason for trusting its contract");
    for (const Clause& c : f.requires_) clause(c, Ctx::Requires, "requires");
    for (const Clause& c : f.ensures_) clause(c, Ctx::Ensures, "ensures");
    for (const Clause& c : f.decreases_) measure(c, Ctx::Requires, "decreases");
    if (f.body != nullptr) stmt(*f.body);
  }

  void clause(const Clause& c, Ctx ctx, const char* what) {
    if (c.expr == nullptr) {
      err(c.loc, std::string("a ") + what + " clause without an expression");
      return;
    }
    if (!c.loc.known()) err(c.loc, std::string("a ") + what + " clause has no source position");
    CallState st;
    expr(*c.expr, ctx, false, st, false);
    if (!c.expr->type.is_bool()) err(c.expr->loc, std::string("the ") + what + " clause is not a bool");
  }

  // One component of a termination measure: an Int expression (decision 0013 item 1), evaluated like a contract
  // clause (function) or like an invariant (loop).
  void measure(const Clause& c, Ctx ctx, const char* what) {
    if (c.expr == nullptr) {
      err(c.loc, std::string("a ") + what + " component without an expression");
      return;
    }
    if (!c.loc.known()) err(c.loc, std::string("a ") + what + " component has no source position");
    CallState st;
    expr(*c.expr, ctx, false, st, false);
    if (!c.expr->type.is_int()) err(c.expr->loc, std::string("a ") + what + " component of type " + c.expr->type.str() + " (a measure is an integer)");
  }

  // ---- statements ----

  void declare(const Var& v, const SrcLoc& loc) {
    if (!owned(v)) {
      err(loc, "variable '" + v.name + "' does not belong to function '" + fn_->c_name + "'");
      return;
    }
    if (!value_type(v.type)) err(loc, "variable '" + v.name + "' is not a bool or an integer");
    if (!declared_.insert(v.id).second) err(loc, "variable '" + v.name + "' is declared twice");
    scopes_.back().insert(v.id);
  }

  bool owned(const Var& v) const {
    const auto& ls = fn_->locals();
    return v.id >= 0 && static_cast<size_t>(v.id) < ls.size() && ls[static_cast<size_t>(v.id)].get() == &v;
  }

  bool visible(const Var& v) const {
    for (const auto& s : scopes_)
      if (s.count(v.id) != 0) return true;
    return false;
  }

  void stmt(const Stmt& s) {
    if (!s.loc.known()) err(s.loc, "statement without a source position");
    switch (s.kind) {
      case Stmt::Kind::Decl: {
        if (s.var == nullptr || s.expr == nullptr) { err(s.loc, "a declaration without a variable or an initializer"); return; }
        CallState st;
        expr(*s.expr, Ctx::Body, false, st, false);
        if (s.var->type != s.expr->type)
          err(s.expr->loc, "the initializer of '" + s.var->name + "' has type " + s.expr->type.str() + ", not " + s.var->type.str());
        declare(*s.var, s.loc);  // declared after its initializer: `int x = x;` reads an undeclared x
        break;
      }
      case Stmt::Kind::Assign: {
        if (s.var == nullptr || s.expr == nullptr) { err(s.loc, "an assignment without a variable or a value"); return; }
        if (!owned(*s.var)) err(s.loc, "variable '" + s.var->name + "' does not belong to function '" + fn_->c_name + "'");
        else if (!visible(*s.var)) err(s.loc, "assignment to '" + s.var->name + "', which is not declared here");
        CallState st;
        expr(*s.expr, Ctx::Body, false, st, false);
        if (s.var->type != s.expr->type)
          err(s.expr->loc, "assignment to '" + s.var->name + "' of a " + s.expr->type.str() + " where " + s.var->type.str() + " is required");
        break;
      }
      case Stmt::Kind::ExprStmt: {
        if (s.expr == nullptr) { err(s.loc, "an expression statement without an expression"); return; }
        CallState st;
        expr(*s.expr, Ctx::Body, false, st, true);
        break;
      }
      case Stmt::Kind::If: {
        if (s.expr == nullptr) { err(s.loc, "an `if` without a condition"); return; }
        CallState st;
        expr(*s.expr, Ctx::Body, false, st, false);
        if (!s.expr->type.is_bool()) err(s.expr->loc, "an `if` condition of type " + s.expr->type.str() + " (convert it with to_bool)");
        scoped(s.then_body);
        scoped(s.else_body);
        break;
      }
      case Stmt::Kind::While: {
        if (s.expr == nullptr || s.body == nullptr) { err(s.loc, "a loop without a condition or a body"); return; }
        CallState st;
        expr(*s.expr, Ctx::Annot, false, st, false);  // no calls in a loop condition (0012 item 6)
        if (!s.expr->type.is_bool()) err(s.expr->loc, "a loop condition of type " + s.expr->type.str() + " (convert it with to_bool)");
        for (const Clause& c : s.invariants) {
          if (c.expr == nullptr) { err(c.loc, "an invariant without an expression"); continue; }
          if (!c.loc.known()) err(c.loc, "an invariant has no source position");
          CallState ist;
          expr(*c.expr, Ctx::Annot, false, ist, false);
          if (!c.expr->type.is_bool()) err(c.expr->loc, "an invariant is not a bool");
        }
        if (s.measure.empty()) err(s.loc, "a loop without a termination measure (decreases, decision 0013)");
        for (const Clause& c : s.measure) measure(c, Ctx::Annot, "decreases");
        scoped(s.body);
        break;
      }
      case Stmt::Kind::Return: {
        if (fn_->ret.is_void()) {
          if (s.expr != nullptr) err(s.expr->loc, "a value returned from void function '" + fn_->c_name + "'");
        } else if (s.expr == nullptr) {
          err(s.loc, "`return;` in non-void function '" + fn_->c_name + "'");
        } else {
          CallState st;
          expr(*s.expr, Ctx::Body, false, st, false);
          if (s.expr->type != fn_->ret)
            err(s.expr->loc, "return of a " + s.expr->type.str() + " from a function returning " + fn_->ret.str());
        }
        break;
      }
      case Stmt::Kind::Check: {
        if (s.expr == nullptr) { err(s.loc, "a check without an expression"); return; }
        CallState st;
        expr(*s.expr, Ctx::Annot, false, st, false);
        if (!s.expr->type.is_bool()) err(s.expr->loc, "a check of type " + s.expr->type.str() + " (it must be a bool)");
        break;
      }
      case Stmt::Kind::Block: {
        scopes_.emplace_back();
        for (const Stmt* c : s.stmts) {
          if (c == nullptr) { err(s.loc, "a null statement in a block"); continue; }
          stmt(*c);
        }
        scopes_.pop_back();
        break;
      }
    }
  }

  void scoped(const Stmt* s) {
    if (s == nullptr) return;
    scopes_.emplace_back();
    stmt(*s);
    scopes_.pop_back();
  }

  // ---- expressions ----

  // `guarded`: the node is evaluated conditionally (right of && / ||, a branch of ?:).
  // `void_ok`: a Call of a void function is allowed here (an expression statement).
  void expr(const Expr& e, Ctx ctx, bool guarded, CallState& st, bool void_ok) {
    if (!e.loc.known()) err(e.loc, "expression without a source position");
    if (e.type.is_void()) {
      if (e.kind != Expr::Kind::Call) err(e.loc, "an expression of type void that is not a call");
      else if (!void_ok) err(e.loc, "the value of a void call is used");
    } else if (!value_type(e.type)) {
      err(e.loc, "an expression of an unsupported type");
    }
    auto sub = [&](const Expr* x, bool g) {
      if (x == nullptr) { err(e.loc, "a missing operand"); return false; }
      expr(*x, ctx, guarded || g, st, false);
      return true;
    };
    switch (e.kind) {
      case Expr::Kind::Const:
        if (e.type.is_bool()) {
          if (e.value != "true" && e.value != "false") err(e.loc, "a bool constant that is not true or false");
        } else if (e.type.is_int()) {
          if (!dec::valid(e.value)) err(e.loc, "malformed integer constant '" + e.value + "'");
          else if (!dec::in_range(e.value, dec::range_of(*e.type.int_type)))
            err(e.loc, "constant " + e.value + " does not fit " + e.type.str());
        }
        break;
      case Expr::Kind::Local:
        if (ctx == Ctx::Requires || ctx == Ctx::Ensures) { err(e.loc, "a local variable in a contract (contracts read the formals)"); break; }
        if (e.var == nullptr || !owned(*e.var) || !visible(*e.var)) { err(e.loc, "use of a variable that is not declared here"); break; }
        if (e.type != e.var->type) err(e.loc, "a use of '" + e.var->name + "' with the wrong type");
        break;
      case Expr::Kind::Formal:
        if (ctx != Ctx::Requires && ctx != Ctx::Ensures) { err(e.loc, "a formal parameter outside a contract"); break; }
        if (e.index < 0 || static_cast<size_t>(e.index) >= fn_->params.size()) { err(e.loc, "a formal parameter index out of range"); break; }
        if (e.type != fn_->params[static_cast<size_t>(e.index)].type) err(e.loc, "a formal parameter with the wrong type");
        break;
      case Expr::Kind::Result:
        if (ctx != Ctx::Ensures) err(e.loc, "the result outside an `ensures`");
        else if (fn_->ret.is_void()) err(e.loc, "the result of a void function");
        else if (e.type != fn_->ret) err(e.loc, "the result with the wrong type");
        break;
      case Expr::Kind::Old:
        if (ctx != Ctx::Ensures) { err(e.loc, "`old` outside an `ensures`"); break; }
        if (e.a != nullptr && mentions_result_or_local(*e.a)) err(e.loc, "`old` of the result (it has no value on entry)");
        if (sub(e.a, false) && e.a->type != e.type) err(e.loc, "`old` changes the type");
        break;
      case Expr::Kind::Unary:
        if (!sub(e.a, false)) break;
        if (e.uop == UnaryOp::Not) {
          if (!e.a->type.is_bool()) err(e.loc, "operand of `!` is " + e.a->type.str() + ", not bool (convert it with to_bool)");
          if (!e.type.is_bool()) err(e.loc, "`!` has a non-bool type");
        } else {
          if (!e.a->type.is_int()) err(e.loc, "operand of unary arithmetic is " + e.a->type.str() + ", not an integer");
          if (e.type != e.a->type) err(e.loc, "unary arithmetic changes the type");
        }
        break;
      case Expr::Kind::Binary: binary(e, ctx, guarded, st); break;
      case Expr::Kind::Cond:
        if (e.a == nullptr || e.b == nullptr || e.c == nullptr) { err(e.loc, "a conditional with a missing operand"); break; }
        expr(*e.a, ctx, guarded, st, false);
        expr(*e.b, ctx, true, st, false);
        expr(*e.c, ctx, true, st, false);
        if (!e.a->type.is_bool()) err(e.loc, "the condition of `?:` is " + e.a->type.str() + ", not bool (convert it with to_bool)");
        if (e.b->type != e.c->type) err(e.loc, "the branches of `?:` have different types " + e.b->type.str() + " and " + e.c->type.str());
        if (e.type != e.b->type) err(e.loc, "`?:` has the wrong type");
        break;
      case Expr::Kind::Convert:
        if (!sub(e.a, false)) break;
        if (!e.a->type.is_int() || !e.type.is_int()) err(e.loc, "a conversion between " + e.a->type.str() + " and " + e.type.str() + " (use to_bool / bool_to_int for bool)");
        break;
      case Expr::Kind::ToBool:
        if (!sub(e.a, false)) break;
        if (!e.a->type.is_int()) err(e.loc, "to_bool of a " + e.a->type.str());
        if (!e.type.is_bool()) err(e.loc, "to_bool does not give a bool");
        break;
      case Expr::Kind::BoolToInt:
        if (!sub(e.a, false)) break;
        if (!e.a->type.is_bool()) err(e.loc, "bool_to_int of a " + e.a->type.str());
        if (!e.type.is_int()) err(e.loc, "bool_to_int does not give an integer");
        break;
      case Expr::Kind::ReportAt:
        if (sub(e.a, false) && e.type != e.a->type) err(e.loc, "report_at changes the type");
        break;
      case Expr::Kind::Call: call(e, ctx, guarded, st); break;
    }
  }

  static bool mentions_result_or_local(const Expr& e) {
    if (e.kind == Expr::Kind::Result || e.kind == Expr::Kind::Local || e.kind == Expr::Kind::Call) return true;
    for (const Expr* x : {e.a, e.b, e.c})
      if (x != nullptr && mentions_result_or_local(*x)) return true;
    return false;
  }

  void binary(const Expr& e, Ctx ctx, bool guarded, CallState& st) {
    if (e.a == nullptr || e.b == nullptr) { err(e.loc, "a binary operation with a missing operand"); return; }
    const bool lazy = e.bop == BinaryOp::LAnd || e.bop == BinaryOp::LOr;
    expr(*e.a, ctx, guarded, st, false);
    expr(*e.b, ctx, guarded || lazy, st, false);
    const Type& a = e.a->type;
    const Type& b = e.b->type;
    switch (e.bop) {
      case BinaryOp::Add: case BinaryOp::Sub: case BinaryOp::Mul: case BinaryOp::Div: case BinaryOp::Rem:
        if (!a.is_int() || a != b) err(e.loc, "arithmetic operands have types " + a.str() + " and " + b.str() + ": both must be the same integer type (EDG's conversions are explicit)");
        if (e.type != a) err(e.loc, "arithmetic gives " + e.type.str() + ", not " + a.str());
        break;
      case BinaryOp::Shl: case BinaryOp::Shr:
        if (!a.is_int() || !b.is_int()) err(e.loc, "shift operands have types " + a.str() + " and " + b.str() + ": both must be integers");
        if (e.type != a) err(e.loc, "a shift gives " + e.type.str() + ", not the type of its left operand " + a.str());
        break;
      case BinaryOp::Lt: case BinaryOp::Le: case BinaryOp::Gt: case BinaryOp::Ge:
        if (!a.is_int() || a != b) err(e.loc, "comparison operands have types " + a.str() + " and " + b.str() + ": both must be the same integer type");
        if (!e.type.is_bool()) err(e.loc, "a comparison gives " + e.type.str() + ", not bool (wrap it with bool_to_int for C)");
        break;
      case BinaryOp::Eq: case BinaryOp::Ne:
        if (a != b || !(a.is_int() || a.is_bool())) err(e.loc, "equality operands have types " + a.str() + " and " + b.str() + ": both must be the same integer type or both bool");
        if (!e.type.is_bool()) err(e.loc, "an equality gives " + e.type.str() + ", not bool (wrap it with bool_to_int for C)");
        break;
      case BinaryOp::LAnd: case BinaryOp::LOr:
        if (!a.is_bool() || !b.is_bool()) err(e.loc, "logical operands have types " + a.str() + " and " + b.str() + ": both must be bool (convert with to_bool)");
        if (!e.type.is_bool()) err(e.loc, "a logical operation gives " + e.type.str() + ", not bool");
        break;
    }
  }

  void call(const Expr& e, Ctx ctx, bool guarded, CallState& st) {
    if (ctx == Ctx::Annot)
      err(e.loc, "a call in a loop condition or in a `check`, `invariant` or `decreases` clause (a call is hoisted before its statement, "
                 "where C/C++ does not evaluate these)");
    else if (ctx != Ctx::Body)
      err(e.loc, "a call in a contract");
    if (guarded) err(e.loc, "a call in a conditionally evaluated operand (a hoisted call would run when C/C++ does not run it)");
    if (st.seen > 0)
      err(e.loc, "two calls in one expression that are not nested in each other (their order is unspecified)");
    if (e.callee == nullptr || functions_.count(e.callee) == 0) { err(e.loc, "a call of a function that is not in the program"); return; }
    const Function& f = *e.callee;
    if (e.args.size() != f.params.size()) {
      err(e.loc, "call of '" + f.c_name + "' with " + std::to_string(e.args.size()) + " arguments, " +
                     std::to_string(f.params.size()) + " expected");
    }
    for (size_t k = 0; k < e.args.size(); ++k) {
      if (e.args[k] == nullptr) { err(e.loc, "a missing argument"); continue; }
      expr(*e.args[k], ctx, guarded, st, false);
      if (k < f.params.size() && e.args[k]->type != f.params[k].type)
        err(e.args[k]->loc, "argument " + std::to_string(k + 1) + " of '" + f.c_name + "' is " + e.args[k]->type.str() + ", the parameter is " + f.params[k].type.str() + " (convert it)");
    }
    if (e.type != f.ret) err(e.loc, "a call of '" + f.c_name + "' with the wrong result type");
    ++st.seen;
  }

  const Program& prog_;
  std::unordered_set<const Function*> functions_;
  const Function* fn_ = nullptr;
  std::set<int> declared_;
  std::vector<std::set<int>> scopes_;
  std::vector<Diag> diags_;
};

}  // namespace

std::vector<Diag> check_program(const Program& program) { return Checker(program).run(); }

}  // namespace cerastes::vir
