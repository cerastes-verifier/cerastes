// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "check.h"

#include <set>
#include <string>
#include <unordered_set>

#include "dec.h"

namespace cerastes::vir {
namespace {

// A tiny constant evaluator for the condition of a `do ... while (0)` (LoopForm::DoOnce): lowering runs that body exactly
// once, so the checker refuses a condition that is not false.  Values are host integers, so only types of at most 64 bits
// are evaluated; anything it cannot evaluate exactly (division by zero, a result outside its type or an overflow of the
// host integer, a shift, a call, a variable) is "unknown".  A DoOnce is rejected only if its condition reads a variable or
// calls, or folds cleanly to a non-zero value; an unknown constant is accepted, because lowering asserts the condition's
// obligations after the body and then `assert !c` as an internal check.
struct ConstVal {
  bool ok = false;
  bool is_bool = false;
  __int128 v = 0;
};

ConstVal unknown() { return ConstVal{}; }
ConstVal int_val(__int128 v) { return ConstVal{true, false, v}; }
ConstVal bool_val(bool b) { return ConstVal{true, true, b ? 1 : 0}; }

bool fits(__int128 v, const Type& t) {
  // An enumeration's values are its value range, not its bits (decision 0018): left "unknown" here, which only makes a
  // `DoOnce` condition unknown (accepted, and guarded by lowering).
  if (!t.is_int() || t.int_type->bits > 64 || t.int_type->is_enum) return false;
  const int bits = t.int_type->bits;
  const __int128 one = 1;
  if (t.int_type->is_signed) return v >= -(one << (bits - 1)) && v <= (one << (bits - 1)) - 1;
  return v >= 0 && v <= (one << bits) - 1;
}

ConstVal eval_const(const Expr& e) {
  auto sub = [&](const Expr* x) { return x != nullptr ? eval_const(*x) : unknown(); };
  switch (e.kind) {
    case Expr::Kind::Const: {
      if (e.type.is_bool()) return bool_val(e.value == "true");
      if (!e.type.is_int() || !dec::valid(e.value) || e.value.size() > 30) return unknown();
      __int128 v = 0;
      const bool neg = dec::negative(e.value);
      for (size_t i = neg ? 1 : 0; i < e.value.size(); ++i) v = v * 10 + (e.value[i] - '0');
      v = neg ? -v : v;
      return fits(v, e.type) ? int_val(v) : unknown();
    }
    case Expr::Kind::ReportAt: return sub(e.a);
    case Expr::Kind::ToBool: {
      ConstVal a = sub(e.a);
      return a.ok && !a.is_bool ? bool_val(a.v != 0) : unknown();
    }
    case Expr::Kind::BoolToInt: {
      ConstVal a = sub(e.a);
      return a.ok && a.is_bool && fits(a.v, e.type) ? int_val(a.v) : unknown();
    }
    case Expr::Kind::Convert: {
      ConstVal a = sub(e.a);
      return a.ok && !a.is_bool && fits(a.v, e.type) ? int_val(a.v) : unknown();
    }
    case Expr::Kind::Unary: {
      ConstVal a = sub(e.a);
      if (!a.ok) return unknown();
      if (e.uop == UnaryOp::Not) return a.is_bool ? bool_val(a.v == 0) : unknown();
      if (a.is_bool) return unknown();
      const __int128 r = e.uop == UnaryOp::Neg ? -a.v : a.v;
      return fits(r, e.type) ? int_val(r) : unknown();
    }
    case Expr::Kind::Cond: {
      ConstVal c = sub(e.a);
      if (!c.ok || !c.is_bool) return unknown();
      return sub(c.v != 0 ? e.b : e.c);
    }
    case Expr::Kind::Binary: {
      ConstVal a = sub(e.a);
      if (!a.ok) return unknown();
      if (e.bop == BinaryOp::LAnd || e.bop == BinaryOp::LOr) {
        if (!a.is_bool) return unknown();
        if (e.bop == BinaryOp::LAnd && a.v == 0) return bool_val(false);
        if (e.bop == BinaryOp::LOr && a.v != 0) return bool_val(true);
        ConstVal b = sub(e.b);
        return b.ok && b.is_bool ? bool_val(b.v != 0) : unknown();
      }
      ConstVal b = sub(e.b);
      if (!b.ok || a.is_bool != b.is_bool) return unknown();
      if (e.bop == BinaryOp::Eq) return bool_val(a.v == b.v);
      if (e.bop == BinaryOp::Ne) return bool_val(a.v != b.v);
      if (a.is_bool) return unknown();
      switch (e.bop) {
        case BinaryOp::Lt: return bool_val(a.v < b.v);
        case BinaryOp::Le: return bool_val(a.v <= b.v);
        case BinaryOp::Gt: return bool_val(a.v > b.v);
        case BinaryOp::Ge: return bool_val(a.v >= b.v);
        case BinaryOp::Add: return fits(a.v + b.v, e.type) ? int_val(a.v + b.v) : unknown();
        case BinaryOp::Sub: return fits(a.v - b.v, e.type) ? int_val(a.v - b.v) : unknown();
        case BinaryOp::Mul: {
          __int128 r;
          if (__builtin_mul_overflow(a.v, b.v, &r)) return unknown();  // two large unsigned 64-bit operands
          return fits(r, e.type) ? int_val(r) : unknown();
        }
        case BinaryOp::Div: return b.v != 0 && fits(a.v / b.v, e.type) ? int_val(a.v / b.v) : unknown();
        case BinaryOp::Rem: return b.v != 0 && fits(a.v % b.v, e.type) ? int_val(a.v % b.v) : unknown();
        default: return unknown();
      }
    }
    default: return unknown();
  }
}

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
    for (const NamedConst& c : prog_.named_consts()) named_const(c, "the named constant");
    for (const Function& f : prog_.functions()) function(f);
    return std::move(diags_);
  }

 private:
  struct CallState {
    int seen = 0;  // calls seen so far in the current full expression (in evaluation order)
  };

  void err(const SrcLoc& loc, std::string reason) { diags_.push_back(Diag{loc, std::move(reason)}); }

  static bool is_enum(const Type& t) { return t.is_int() && t.int_type != nullptr && t.int_type->is_enum; }

  static bool value_type(const Type& t) {
    if (t.is_bool()) return true;
    return t.is_int() && t.int_type != nullptr && t.int_type->bits >= 1 && t.int_type->bits <= 128;
  }

  void function(const Function& f) {
    fn_ = &f;
    declared_.clear();
    jump_ids_.clear();
    targets_.clear();
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
    for (const NamedConst& c : f.local_consts()) {
      if (f.body == nullptr) err(c.loc, "a local constant of '" + f.c_name + "', which has no body");
      named_const(c, "the local constant");
    }
    for (const Clause& c : f.requires_) clause(c, Ctx::Requires, "requires");
    for (const Clause& c : f.ensures_) clause(c, Ctx::Ensures, "ensures");
    for (const Clause& c : f.decreases_) measure(c, Ctx::Requires, "decreases");
    if (f.noreturn && !f.ensures_.empty())
      err(f.ensures_.front().loc, "an `ensures` on '" + f.c_name + "', which is declared not to return (decision 0015 item 4)");
    if (f.body != nullptr) {
      for (const GotoProblem& p : goto_problems(f)) {
        const char* why = "";
        switch (p.kind) {
          case GotoProblem::Kind::backward: why = "goes backward to"; break;
          case GotoProblem::Kind::enters: why = "enters a block, branch, loop or switch to reach"; break;
          case GotoProblem::Kind::skips_declaration: why = "skips a declaration to reach"; break;
          case GotoProblem::Kind::no_label: why = "names a label that is not in the function:"; break;
        }
        err(p.go->loc, std::string("a `goto` that ") + why + " label '" + p.label + "'" +
                           (p.kind == GotoProblem::Kind::skips_declaration ? " (past '" + p.variable + "')" : "") +
                           " (decision 0015 item 2)");
      }
      stmt(*f.body);
    }
  }

  // A decimal literal of an Int type's range, or "0"/"1" of bool (a front-end value, decision 0017).
  static bool literal_fits(const std::string& v, const Type& t) {
    if (t.is_bool()) return v == "0" || v == "1";
    if (!t.is_int() || t.int_type == nullptr) return false;
    return dec::valid(v) && dec::in_range(v, dec::range_of(*t.int_type));
  }
  // The front end's value of a named constant or a case label (decision 0017): a value of the type, except that for an
  // enumeration it need only be a value of the underlying type (decision 0018 item 5).  An enumeration value outside the value
  // range comes from a conversion into the enumeration in the initializer or the case value as written, whose closed
  // obligation the prologue asserts, and fails, before anything uses the value; the encoder never makes a Const of it.
  static bool front_end_value_fits(const std::string& v, const Type& t) {
    if (!is_enum(t)) return literal_fits(v, t);
    IntType underlying;
    underlying.bits = t.int_type->bits;
    underlying.is_signed = t.int_type->is_signed;
    return dec::valid(v) && dec::in_range(v, dec::range_of(underlying));
  }

  // A named constant (decision 0017 item 4): the initializer as written is call-free, mentions no variable, parameter, result
  // or `old`, and has exactly the constant's type; `value` is a literal of that type.
  void named_const(const NamedConst& c, const char* what) {
    const std::string name = std::string(what) + " '" + c.c_name + "'";
    if (!c.loc.known()) err(c.loc, name + " has no source position");
    if (!value_type(c.type)) {
      err(c.loc, name + " is not a bool or an integer");
      return;
    }
    if (!front_end_value_fits(c.value, c.type)) err(c.loc, "the front end's value of " + name + " ('" + c.value + "') is not a value of " + c.type.str());
    if (c.init == nullptr) {
      if (c.value != "0") err(c.loc, name + " has no initializer but a value other than 0");
      return;
    }
    if (mentions_state(*c.init)) {
      err(c.init->loc, "the initializer of " + name + " mentions a variable, a parameter, the result, `old` or a call");
      return;
    }
    CallState st;
    expr(*c.init, Ctx::Annot, false, st, false);
    if (c.init->type != c.type) err(c.init->loc, "the initializer of " + name + " has type " + c.init->type.str() + ", not " + c.type.str());
  }

  // A statement of a Block: a Label is allowed here and only here (decision 0015 item 3).
  void child(const Stmt& c) {
    if (c.kind != Stmt::Kind::Label) { stmt(c); return; }
    if (!c.loc.known()) err(c.loc, "statement without a source position");
    if (c.jump_id <= 0) err(c.loc, "a label without an id");
    else if (!jump_ids_.insert(c.jump_id).second) err(c.loc, "label id " + std::to_string(c.jump_id) + " is used twice (by two labels, or by a loop or switch)");
    if (c.label_name.empty()) err(c.loc, "a label without a name");
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
        if (s.jump_id < 0) err(s.loc, "a loop with a negative jump id");
        else if (s.jump_id > 0) register_id(s.jump_id, s.loc);
        if (s.form == LoopForm::DoWhile && s.jump_id <= 0) err(s.loc, "a `do` loop without a jump id (its exit test jumps to a label named by it)");
        CallState st;
        expr(*s.expr, Ctx::Annot, false, st, false);  // no calls in a loop condition, a `do` condition included (0012 item 6)
        if (!s.expr->type.is_bool()) err(s.expr->loc, "a loop condition of type " + s.expr->type.str() + " (convert it with to_bool)");
        const bool once = s.form == LoopForm::DoOnce;
        if (once) {
          const ConstVal cv = eval_const(*s.expr);
          // Rejected: a condition that reads a variable or calls, or that folds cleanly to a non-zero value.  A constant the
          // evaluator cannot fold (a shift, an inexact conversion, an overflow) is accepted: lowering asserts its obligations
          // after the body, so the user sees that error, and the internal `assert !c` stops any value that is not zero.
          if (mentions_result_or_local(*s.expr) || (cv.ok && !(cv.is_bool && cv.v == 0)))
            err(s.expr->loc, "a `do ... while` loop lowered as running once whose condition is not a constant zero (decision 0014 item 5)");
        }
        for (const Clause& c : s.invariants) {
          if (c.expr == nullptr) { err(c.loc, "an invariant without an expression"); continue; }
          if (!c.loc.known()) err(c.loc, "an invariant has no source position");
          CallState ist;
          expr(*c.expr, Ctx::Annot, false, ist, false);
          if (!c.expr->type.is_bool()) err(c.expr->loc, "an invariant is not a bool");
        }
        if (s.measure.empty() && !once) err(s.loc, "a loop without a termination measure (decreases, decision 0013)");
        for (const Clause& c : s.measure) measure(c, Ctx::Annot, "decreases");
        targets_.push_back(Target{s.jump_id, true});
        scoped(s.body);
        targets_.pop_back();
        if (s.step != nullptr) {
          if (s.form != LoopForm::While) err(s.step->loc, "a loop step on a `do` loop");
          else if (s.step->kind != Stmt::Kind::Assign && s.step->kind != Stmt::Kind::ExprStmt)
            err(s.step->loc, "a loop step that is not an assignment or an expression statement");
          else scoped(s.step);
        }
        break;
      }
      case Stmt::Kind::Switch: {
        if (s.expr == nullptr || s.body == nullptr) { err(s.loc, "a switch without a controlling expression or a body"); return; }
        if (s.jump_id <= 0) err(s.loc, "a switch without a jump id");
        else register_id(s.jump_id, s.loc);
        CallState st;
        expr(*s.expr, Ctx::Body, false, st, false);  // evaluated once; a call is hoisted like a declaration's initializer
        if (!s.expr->type.is_int()) err(s.expr->loc, "a switch on a " + s.expr->type.str() + " (the controlling expression is a promoted integer)");
        if (s.body->kind != Stmt::Kind::Block) { err(s.body->loc, "the body of a switch is not a compound statement"); return; }
        if (!s.body->loc.known()) err(s.body->loc, "statement without a source position");
        targets_.push_back(Target{s.jump_id, false});
        scopes_.emplace_back();
        std::set<std::string> values;
        int defaults = 0;
        bool declaration_seen = false;
        bool first = true;
        for (const Stmt* c : s.body->stmts) {
          if (c == nullptr) { err(s.body->loc, "a null statement in a block"); continue; }
          if (c->kind == Stmt::Kind::Case) {
            if (!c->loc.known()) err(c->loc, "statement without a source position");
            if (declaration_seen) err(c->loc, "a declaration in the body of a switch that this case label jumps over");
            if (c->expr == nullptr) {
              if (++defaults > 1) err(c->loc, "a second `default` label in one switch");
            } else {
              // The value is any call-free integer expression of the scrutinee's type (the encoder uses the case constant's
              // original expression, 0010 item 12); EDG guarantees that the folded values are distinct.
              CallState vst;
              expr(*c->expr, Ctx::Annot, false, vst, false);
              if (!c->expr->type.is_int()) err(c->loc, "a case value that is not an integer");
              else if (c->expr->type != s.expr->type)
                err(c->loc, "a case value of type " + c->expr->type.str() + " in a switch on " + s.expr->type.str());
              else if (mentions_result_or_local(*c->expr)) err(c->loc, "a case value that is not a constant expression");
              if (c->folded.empty()) err(c->loc, "internal: a case without its front-end value");
              else if (!front_end_value_fits(c->folded, s.expr->type))
                err(c->loc, "the front end's case value '" + c->folded + "' is not a value of " + s.expr->type.str());
              else if (!values.insert(c->folded).second) err(c->loc, "the case value " + c->folded + " appears twice in one switch");
            }
          } else {
            if (first) err(c->loc, "a statement before the first case label of a switch (it is unreachable)");
            if (c->kind == Stmt::Kind::Decl) declaration_seen = true;
            child(*c);
          }
          first = false;
        }
        scopes_.pop_back();
        targets_.pop_back();
        break;
      }
      case Stmt::Kind::Case:
        err(s.loc, "a case label that is not directly in the body of its switch");
        break;
      case Stmt::Kind::Break:
      case Stmt::Kind::Continue: {
        const bool is_break = s.kind == Stmt::Kind::Break;
        // The target is the INNERMOST loop or switch for a break, the innermost loop for a continue (a switch in between is
        // skipped): anything else would leave or continue the wrong construct, so it is an inconsistency of the encoder.
        const Target* t = nullptr;
        for (auto it = targets_.rbegin(); it != targets_.rend(); ++it)
          if (is_break || it->is_loop) { t = &*it; break; }
        if (t == nullptr)
          err(s.loc, std::string("a `") + (is_break ? "break" : "continue") + "` outside any " + (is_break ? "loop or switch" : "loop"));
        else if (s.jump_id <= 0 || t->id != s.jump_id)
          err(s.loc, std::string("a `") + (is_break ? "break" : "continue") + "` that does not name the innermost enclosing " +
                         (is_break ? "loop or switch" : "loop"));
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
        expr(*s.expr, s.from_assert ? Ctx::Body : Ctx::Annot, false, st, false);
        if (!s.expr->type.is_bool()) err(s.expr->loc, "a check of type " + s.expr->type.str() + " (it must be a bool)");
        break;
      }
      case Stmt::Kind::Block: {
        scopes_.emplace_back();
        for (const Stmt* c : s.stmts) {
          if (c == nullptr) { err(s.loc, "a null statement in a block"); continue; }
          child(*c);
        }
        scopes_.pop_back();
        break;
      }
      case Stmt::Kind::Goto:
        if (s.jump_id <= 0) err(s.loc, "a `goto` without a label id");
        break;
      case Stmt::Kind::Label:
        err(s.loc, "a label that is not directly in a block (decision 0015 item 3)");
        break;
    }
  }

  void register_id(int id, const SrcLoc& loc) {
    if (!jump_ids_.insert(id).second) err(loc, "jump id " + std::to_string(id) + " is used by two loops or switches, or by a loop or switch and a label");
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
          else if (is_enum(e.a->type)) err(e.loc, "unary arithmetic on a value of the enumeration " + e.a->type.str() + " (decision 0018 item 6)");
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
        else if (is_enum(e.type)) err(e.loc, "bool_to_int to the enumeration " + e.type.str() + " (decision 0018 item 5)");
        break;
      case Expr::Kind::ReportAt:
        if (sub(e.a, false) && e.type != e.a->type) err(e.loc, "report_at changes the type");
        break;
      case Expr::Kind::Call: call(e, ctx, guarded, st); break;
    }
  }

  static bool mentions_state(const Expr& e) {
    if (e.kind == Expr::Kind::Result || e.kind == Expr::Kind::Local || e.kind == Expr::Kind::Call || e.kind == Expr::Kind::Formal ||
        e.kind == Expr::Kind::Old)
      return true;
    for (const Expr* x : {e.a, e.b, e.c})
      if (x != nullptr && mentions_state(*x)) return true;
    return false;
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
      case BinaryOp::Shl: case BinaryOp::Shr:
        // Arithmetic and shifts are computed in the promoted type, which differs between C and C++ for an enumeration
        // (decision 0018 item 6): the encoder converts or rejects first; this is the backstop.
        if (is_enum(a) || is_enum(b)) err(e.loc, "arithmetic or a shift on a value of an enumeration (decision 0018 item 6)");
        break;
      default: break;
    }
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
  struct Target {
    int id;
    bool is_loop;
  };
  std::set<int> jump_ids_;
  std::vector<Target> targets_;  // the loops and switches around the statement being checked, innermost last
  std::vector<std::set<int>> scopes_;
  std::vector<Diag> diags_;
};

}  // namespace

std::vector<Diag> check_program(const Program& program) { return Checker(program).run(); }

namespace {

// One statement as the pre-order walk of goto_problems sees it.
struct GotoWalk {
  struct Pos {
    const Stmt* block;
    size_t index;
  };
  struct Info {
    const Stmt* stmt = nullptr;
    int order = 0;
    std::vector<Pos> path;           // the enclosing Blocks, outermost first, with the index of the child holding the statement
    const Stmt* parent_block = nullptr;  // Label: the Block it is a direct statement of (null: not direct)
    size_t index = 0;
  };
  std::vector<Info> gotos;
  std::vector<Info> labels;
  std::vector<Pos> path;
  int counter = 0;

  void walk(const Stmt* s, const Stmt* parent_block, size_t index) {
    if (s == nullptr) return;
    const int order = counter++;
    if (s->kind == Stmt::Kind::Goto || s->kind == Stmt::Kind::Label) {
      Info i;
      i.stmt = s;
      i.order = order;
      i.path = path;
      i.parent_block = parent_block;
      i.index = index;
      (s->kind == Stmt::Kind::Goto ? gotos : labels).push_back(std::move(i));
    }
    switch (s->kind) {
      case Stmt::Kind::Block:
        for (size_t k = 0; k < s->stmts.size(); ++k) {
          path.push_back(Pos{s, k});
          walk(s->stmts[k], s, k);
          path.pop_back();
        }
        break;
      case Stmt::Kind::If:
        walk(s->then_body, nullptr, 0);
        walk(s->else_body, nullptr, 0);
        break;
      case Stmt::Kind::While:
        walk(s->body, nullptr, 0);
        walk(s->step, nullptr, 0);
        break;
      case Stmt::Kind::Switch: walk(s->body, nullptr, 0); break;
      default: break;
    }
  }
};

}  // namespace

std::vector<GotoProblem> goto_problems(const Function& f) {
  std::vector<GotoProblem> out;
  if (f.body == nullptr) return out;
  GotoWalk w;
  w.walk(f.body, nullptr, 0);
  for (const GotoWalk::Info& g : w.gotos) {
    const GotoWalk::Info* target = nullptr;
    for (const GotoWalk::Info& l : w.labels)
      if (l.stmt->jump_id == g.stmt->jump_id) { target = &l; break; }
    GotoProblem p;
    p.go = g.stmt;
    if (target == nullptr) {
      p.kind = GotoProblem::Kind::no_label;
      out.push_back(std::move(p));
      continue;
    }
    p.label = target->stmt->label_name;
    bool accepted = false;
    if (target->parent_block != nullptr) {
      for (const GotoWalk::Pos& pos : g.path) {
        if (pos.block != target->parent_block || target->index <= pos.index) continue;
        const Stmt* skipped = nullptr;
        for (size_t k = pos.index + 1; k < target->index && skipped == nullptr; ++k) {
          const Stmt* c = pos.block->stmts[k];
          if (c != nullptr && c->kind == Stmt::Kind::Decl) skipped = c;
        }
        if (skipped != nullptr) {
          p.kind = GotoProblem::Kind::skips_declaration;
          p.variable = skipped->var != nullptr ? skipped->var->name : "";
          out.push_back(p);
        }
        accepted = true;  // handled: either fine or reported as skips_declaration
        break;
      }
    }
    if (accepted) continue;
    p.kind = target->order < g.order ? GotoProblem::Kind::backward : GotoProblem::Kind::enters;
    out.push_back(std::move(p));
  }
  return out;
}

}  // namespace cerastes::vir
