// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "check.h"

#include <map>
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
      if (e.uop == UnaryOp::BitNot) {
        // Decision 0023 item 6: -v - 1 (signed), hi - v (unsigned), never C++'s `~` on the __int128; 128-bit types are unknown.
        if (!e.type.is_int() || e.type.int_type->bits > 64 || e.type.int_type->is_enum) return unknown();
        const __int128 one = 1;
        const int bits = e.type.int_type->bits;
        const __int128 r = e.type.int_type->is_signed ? -a.v - 1 : ((one << bits) - 1) - a.v;
        return fits(r, e.type) ? int_val(r) : unknown();
      }
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
        // Decision 0023 item 6: two's complement on the mathematical values of operands of one type (exact for signed values on
        // the __int128, and for unsigned ones up to 64 bits, which are never negative there); then the range check.
        case BinaryOp::BitAnd: return fits(a.v & b.v, e.type) ? int_val(a.v & b.v) : unknown();
        case BinaryOp::BitOr: return fits(a.v | b.v, e.type) ? int_val(a.v | b.v) : unknown();
        case BinaryOp::BitXor: return fits(a.v ^ b.v, e.type) ? int_val(a.v ^ b.v) : unknown();
        default: return unknown();
      }
    }
    default: return unknown();
  }
}

// Where an expression sits; decides which nodes are allowed.
enum class Ctx {
  Body,      // code: Local, Call, AssignE, StoreE and Comma allowed (not Formal, Result, Old)
  Annot,     // `check` and `invariant`: like Body, but no calls (0011 item 7) and no side effects (decision 0020 item 3)
  LoopCond,  // a loop condition: Body, its effects and calls run at every evaluation (decisions 0020 and 0021)
  Assert,    // C's `assert(e)`: calls as in code, no side effects (decision 0015 item 6, 0020)
  Requires,  // a `requires`: Formal only
  Ensures    // an `ensures`: Formal, Result, Old
};

// Whether a Block declares a cell directly (an address-taken variable's Decl): lowering allocates it at the Block's entry.
bool declares_cell(const Stmt& b) {
  for (const Stmt* c : b.stmts)
    if (c != nullptr && c->kind == Stmt::Kind::Decl && c->var != nullptr && c->var->address_taken) return true;
  return false;
}

// A node that b1a keeps out of invariants, measures, named constants, case values and loops (decision 0032 items 9 and 10), and b1b out of
// all but the contract nodes of decision 0037 item 5: of pointer type, a Null/AddrOf/Deref/Atom, or a read of a cell.
bool pointer_node(const Expr& e) {
  if (e.type.is_ptr()) return true;
  switch (e.kind) {
    case Expr::Kind::Null: case Expr::Kind::AddrOf: case Expr::Kind::Deref: case Expr::Kind::Atom: case Expr::Kind::StoreE: return true;
    case Expr::Kind::Local: return e.var != nullptr && e.var->address_taken;
    default: return false;
  }
}

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
  void err(const SrcLoc& loc, std::string reason) { diags_.push_back(Diag{loc, std::move(reason)}); }

  static bool is_enum(const Type& t) { return t.is_int() && t.int_type != nullptr && t.int_type->is_enum; }

  static bool scalar_type(const Type& t) {
    if (t.is_bool()) return true;
    return t.is_int() && t.int_type != nullptr && t.int_type->bits >= 1 && t.int_type->bits <= 128;
  }

  // A pointer type decisions 0032 item 2 and 0040 item 2 accept: its innermost pointee is a bool, an integer type or an enumeration, through
  // any number of pointer levels.
  static bool accepted_pointer(const Type& t) {
    if (!t.is_ptr() || t.pointee == nullptr) return false;
    return t.pointee->is_ptr() ? accepted_pointer(*t.pointee) : scalar_type(*t.pointee);
  }

  // Why a pointer type is not accepted (empty when it is, or when `t` is no pointer).
  static std::string pointer_reason(const Type& t) {
    if (!t.is_ptr() || accepted_pointer(t)) return "";
    const Type* in = &t;
    while (in->is_ptr() && in->pointee != nullptr) in = in->pointee;
    return "a pointer to " + (in->is_ptr() ? std::string("an unknown type") : in->str()) + " (decisions 0032 item 2, 0040 item 2)";
  }

  // `allow_ptr`: a pointer type counts (locals and expressions); parameters, results, named constants and the like pass false.
  static bool value_type(const Type& t, bool allow_ptr = false) {
    if (allow_ptr && t.is_ptr()) return accepted_pointer(t);
    return scalar_type(t);
  }


  void function(const Function& f) {
    fn_ = &f;
    declared_.clear();
    seen_effects_.clear();
    jump_ids_.clear();
    targets_.clear();
    scopes_.clear();
    no_ptr_ = false;
    old_depth_ = 0;
    scopes_.emplace_back();
    for (const auto& v : f.locals())
      if (v->is_param_copy) scopes_.back().insert(v->id);
    // Decision 0037 items 2-3: a parameter or a result may be a pointer to a bool, an integer or an enumeration.
    if (f.ret.is_ptr()) {
      if (!accepted_pointer(f.ret)) err(f.loc, "the return type of '" + f.c_name + "' is " + pointer_reason(f.ret));
    } else if (!(f.ret.is_void() || value_type(f.ret))) {
      err(f.loc, "the return type of '" + f.c_name + "' is not void, bool, an integer type or a pointer to one");
    }
    for (size_t k = 0; k < f.params.size(); ++k) {
      if (f.params[k].type.is_ptr()) {
        if (!accepted_pointer(f.params[k].type))
          err(f.params[k].loc, "parameter " + std::to_string(k + 1) + " of '" + f.c_name + "' is " + pointer_reason(f.params[k].type));
      } else if (!value_type(f.params[k].type)) {
        err(f.params[k].loc, "parameter " + std::to_string(k + 1) + " of '" + f.c_name + "' is not a bool, an integer type or a pointer to one");
      }
      if (f.params[k].is_restrict && !f.params[k].type.is_ptr())
        err(f.params[k].loc, "parameter " + std::to_string(k + 1) + " of '" + f.c_name + "' is `restrict` but not a pointer");
    }
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
    for (const Clause& c : f.decreases_) measure(c, Ctx::Requires, "decreases", /*memory_ok=*/true);
    atoms(f);
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
      // The backstop of the encoder's own report (decision 0020 item 4): a modification unsequenced with another access.
      for (const SequenceProblem& p : sequence_problems(f))
        err(p.at != nullptr ? p.at->loc : f.loc, "unsequenced modification of '" + p.variable +
                                                    "' (decision 0020: another access to it in the same expression is not sequenced after it)");
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
    const bool assertion = c.kind == NamedConst::Kind::StaticAssertion;
    const std::string name = assertion ? std::string("the static assertion") : std::string(what) + " '" + c.c_name + "'";
    if (!c.loc.known()) err(c.loc, name + " has no source position");
    if (assertion) {
      // Decision 0024 item 2: a static assertion is a Bool condition, closed like an initializer, and EDG accepted it (value 1).
      if (!c.type.is_bool()) {
        err(c.loc, name + " has a condition of type " + c.type.str() + ", not bool");
        return;
      }
      if (c.init == nullptr) {
        err(c.loc, name + " has no condition");
        return;
      }
      // The value is 1 (EDG accepted the assertion); only the encoder's perturbation hook (0017 item 9) sets 0, to see the guard fail.
      if (c.value != "0" && c.value != "1") err(c.loc, "the front end's value of " + name + " ('" + c.value + "') is not a bool value");
      if (mentions_state(*c.init)) {
        err(c.init->loc, "the condition of " + name + " mentions a variable, a parameter, the result, `old` or a call");
        return;
      }
      no_ptr_ = true;
      expr(*c.init, Ctx::Annot, false);
      no_ptr_ = false;
      if (c.init->type != c.type) err(c.init->loc, "the condition of " + name + " has type " + c.init->type.str() + ", not bool");
      return;
    }
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
    no_ptr_ = true;
    expr(*c.init, Ctx::Annot, false);
    no_ptr_ = false;
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

  // Decision 0037 items 5, 6 and 2: the atoms of the contract stand in allowed positions, are numbered, and pair up; `restrict` is backed by
  // an `owns` that the `ensures` returns.
  static void collect_atoms(const Expr* e, std::set<const Expr*>& all) {
    if (e == nullptr) return;
    if (e->kind == Expr::Kind::Atom) all.insert(e);
    for (const Expr* x : {e->a, e->b, e->c}) collect_atoms(x, all);
    for (const Expr* x : e->args) collect_atoms(x, all);
  }

  void atoms(const Function& f) {
    std::set<const Expr*> all;
    for (const Clause& c : f.requires_) collect_atoms(c.expr, all);
    for (const Clause& c : f.ensures_) collect_atoms(c.expr, all);
    for (const Clause& c : f.decreases_) collect_atoms(c.expr, all);
    const std::vector<AtomSite> sites = contract_atoms(f);
    std::set<const Expr*> placed;
    for (const AtomSite& s : sites) placed.insert(s.atom);
    for (const Expr* a : all)
      if (placed.count(a) == 0)
        err(a->loc, "a permission atom in a position decision 0037 item 5 does not allow (a top-level conjunct, the right of `||` or an arm of `?:` whose condition reads no heap)");
    if (placed.size() != sites.size()) err(f.loc, "a permission atom reachable twice in the contract of '" + f.c_name + "'");
    // Requires atoms are numbered in evaluation order; no two of one kind on one parameter (the partner would be ambiguous).
    std::vector<const AtomSite*> req;
    for (const AtomSite& s : sites)
      if (!s.in_ensures) req.push_back(&s);
    for (size_t k = 0; k < req.size(); ++k) {
      const Expr& a = *req[k]->atom;
      if (a.amount != static_cast<int>(k)) err(a.loc, "the permission atoms of the `requires` are not numbered in order (decision 0037 item 6)");
      if (a.a == nullptr || a.a->kind != Expr::Kind::Formal) continue;
      for (size_t j = 0; j < k; ++j) {
        const Expr& b = *req[j]->atom;
        if (b.a != nullptr && b.a->kind == Expr::Kind::Formal && b.atom == a.atom && b.a->index == a.a->index)
          err(a.loc, "the same permission atom twice in a `requires` (decision 0037 item 6)");
      }
    }
    // `owns(*p)` and `reads(*p)` of one parameter in one clause list: lowering asserts per path that they do not both hold (`f__wf`,
    // decision 0039 item 7), so only a same-kind pair is refused here.
    std::set<int> used;
    for (const AtomSite& s : sites) {
      if (!s.in_ensures) continue;
      const Expr& a = *s.atom;
      if (a.amount < 0 || static_cast<size_t>(a.amount) >= req.size()) {
        err(a.loc, "an `ensures` atom without a `requires` atom to return (decision 0037 item 6)");
        continue;
      }
      const Expr& partner = *req[static_cast<size_t>(a.amount)]->atom;
      if (partner.atom != a.atom || partner.a == nullptr || a.a == nullptr || partner.a->kind != Expr::Kind::Formal ||
          a.a->kind != Expr::Kind::Formal || partner.a->index != a.a->index)
        err(a.loc, "an `ensures` atom whose partner in the `requires` is of another kind or on another parameter (decision 0037 item 6)");
      if (!used.insert(a.amount).second) err(a.loc, "two `ensures` atoms return the same `requires` atom (decision 0037 item 6)");
    }
    // `restrict` (item 2): backed by an `owns` that the `ensures` returns.
    for (size_t k = 0; k < f.params.size(); ++k)
      if (f.params[k].is_restrict && !restrict_backed(f, static_cast<int>(k)))
        err(f.params[k].loc, "parameter " + std::to_string(k + 1) + " of '" + f.c_name +
                                 "' is `restrict` without `owns(*p)` in its `requires` and `ensures` (decision 0037 item 2)");
  }

  // Decision 0040 item 5: the atoms of a loop's invariant stand in the allowed positions (the same rule as a contract's) and no kind is
  // written twice on one variable.
  void loop_atom_sites(const Stmt& s) {
    std::set<const Expr*> all;
    for (const Clause& c : s.invariants) collect_atoms(c.expr, all);
    const std::vector<AtomSite> sites = loop_atoms(s);
    std::set<const Expr*> placed;
    for (const AtomSite& st : sites) placed.insert(st.atom);
    if (s.form == LoopForm::DoOnce) return;  // refused one by one in expr()
    for (const Expr* a : all)
      if (placed.count(a) == 0)
        err(a->loc, "a permission atom in a position decision 0037 item 5 does not allow (a top-level conjunct, the right of `||` or an arm of `?:` whose condition reads no heap)");
    if (placed.size() != sites.size()) err(s.loc, "a permission atom of an invariant reachable twice");
    for (size_t k = 0; k < sites.size(); ++k) {
      const Expr& a = *sites[k].atom;
      if (a.a == nullptr || a.a->kind != Expr::Kind::Local) continue;
      for (size_t j = 0; j < k; ++j) {
        const Expr& b = *sites[j].atom;
        if (b.a != nullptr && b.a->kind == Expr::Kind::Local && b.atom == a.atom && b.a->var == a.a->var)
          err(a.loc, "the same permission atom twice in the invariant of a loop (decision 0040 item 5)");
      }
    }
  }

  void clause(const Clause& c, Ctx ctx, const char* what) {
    if (c.expr == nullptr) {
      err(c.loc, std::string("a ") + what + " clause without an expression");
      return;
    }
    if (!c.loc.known()) err(c.loc, std::string("a ") + what + " clause has no source position");
    // A `requires` or `ensures` may hold the pointer nodes of decision 0037 item 5 (a pointer parameter, the result, NULL, `*p` of a
    // parameter, the atoms); an invariant, a measure, a constant and a case value still hold none (decision 0032 item 10), which
    // `measure` and the loop and constant checks say by setting no_ptr_.
    const bool saved = no_ptr_;
    no_ptr_ = false;
    expr(*c.expr, ctx, false);
    no_ptr_ = saved;
    if (!c.expr->type.is_bool()) err(c.expr->loc, std::string("the ") + what + " clause is not a bool");
  }

  // One component of a termination measure: an Int expression (decision 0013 item 1), evaluated like a contract
  // clause (function) or like an invariant (loop).
  // `memory_ok`: a loop's measure and a function's `decreases` may read memory (decision 0038 item 6, 0039 item 8); an atom in either is
  // refused (the atoms check).
  void measure(const Clause& c, Ctx ctx, const char* what, bool memory_ok = false) {
    if (c.expr == nullptr) {
      err(c.loc, std::string("a ") + what + " component without an expression");
      return;
    }
    if (!c.loc.known()) err(c.loc, std::string("a ") + what + " component has no source position");
    const bool saved = no_ptr_;
    no_ptr_ = !memory_ok;
    expr(*c.expr, ctx, false);
    no_ptr_ = saved;
    if (!c.expr->type.is_int()) err(c.expr->loc, std::string("a ") + what + " component of type " + c.expr->type.str() + " (a measure is an integer)");
  }

  // ---- statements ----

  void declare(const Var& v, const SrcLoc& loc) {
    if (!owned(v)) {
      err(loc, "variable '" + v.name + "' does not belong to function '" + fn_->c_name + "'");
      return;
    }
    if (v.type.is_ptr()) {
      if (!accepted_pointer(v.type)) err(loc, "variable '" + v.name + "' is " + pointer_reason(v.type));
    } else if (!value_type(v.type)) {
      err(loc, "variable '" + v.name + "' is not a bool or an integer");
    }
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
        if (s.var == nullptr) { err(s.loc, "a declaration without a variable"); return; }
        if (s.expr != nullptr) {  // no initializer: declared only (decision 0019)
          expr(*s.expr, Ctx::Body, false);
          if (s.var->type != s.expr->type)
            err(s.expr->loc, "the initializer of '" + s.var->name + "' has type " + s.expr->type.str() + ", not " + s.var->type.str());
        }
        declare(*s.var, s.loc);  // declared after its initializer: `int x = x;` reads an undeclared x
        break;
      }
      case Stmt::Kind::Assign: {
        if (s.var == nullptr || s.expr == nullptr) { err(s.loc, "an assignment without a variable or a value"); return; }
        if (!owned(*s.var)) err(s.loc, "variable '" + s.var->name + "' does not belong to function '" + fn_->c_name + "'");
        else if (!visible(*s.var)) err(s.loc, "assignment to '" + s.var->name + "', which is not declared here");
        expr(*s.expr, Ctx::Body, false);
        if (s.var->type != s.expr->type)
          err(s.expr->loc, "assignment to '" + s.var->name + "' of a " + s.expr->type.str() + " where " + s.var->type.str() + " is required");
        break;
      }
      case Stmt::Kind::ExprStmt: {
        if (s.expr == nullptr) { err(s.loc, "an expression statement without an expression"); return; }
        expr(*s.expr, Ctx::Body, true);
        break;
      }
      case Stmt::Kind::If: {
        if (s.expr == nullptr) { err(s.loc, "an `if` without a condition"); return; }
        expr(*s.expr, Ctx::Body, false);
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

        // A side effect or a call in a loop condition runs at every evaluation (decisions 0020 and 0021), except in the condition of a
        // `do ... while (0)`, which is no loop: there a call is refused (Ctx::Annot).
        expr(*s.expr, s.form == LoopForm::DoOnce ? Ctx::Annot : Ctx::LoopCond, false);
        if (has_effect(*s.expr) && s.form == LoopForm::While && s.jump_id <= 0)
          err(s.expr->loc, "a loop whose condition has a side effect or a call and no jump id (it is lowered with an exit jump, decisions 0020 and 0021)");
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
          // Decision 0038 item 6: an invariant may hold pointer nodes, cells by name, `&x` and `*q` (read like code, with the framing
          // lowering gives them), so no_ptr_ stays false.  A permission atom is accepted in a loop's invariant (decision 0040 item 5), not in
          // that of a `do ... while (0)`: it is asserted once like a `check`.
          in_invariant_ = !once;
          expr(*c.expr, Ctx::Annot, false);
          in_invariant_ = false;
          if (!c.expr->type.is_bool()) err(c.expr->loc, "an invariant is not a bool");
        }
        if (s.measure.empty() && !once) err(s.loc, "a loop without a termination measure (decreases, decision 0013)");
        loop_atom_sites(s);
        for (const Clause& c : s.measure) measure(c, Ctx::Annot, "decreases", /*memory_ok=*/true);
        targets_.push_back(Target{s.jump_id, true});
        scoped(s.body);
        targets_.pop_back();
        if (s.step != nullptr) {
          if (s.form != LoopForm::While) err(s.step->loc, "a loop step on a `do` loop");
          else if (!step_shape(*s.step))
            err(s.step->loc, "a loop step that is not an assignment or an expression statement (or a block of them, or an if with both arms such steps)");
          else scoped(s.step);
        }
        break;
      }
      case Stmt::Kind::Switch: {
        if (s.expr == nullptr || s.body == nullptr) { err(s.loc, "a switch without a controlling expression or a body"); return; }
        if (s.jump_id <= 0) err(s.loc, "a switch without a jump id");
        else register_id(s.jump_id, s.loc);
        expr(*s.expr, Ctx::Body, false);  // evaluated once; a call is hoisted like a declaration's initializer
        if (!s.expr->type.is_int()) err(s.expr->loc, "a switch on a " + s.expr->type.str() + " (the controlling expression is a promoted integer)");
        if (s.body->kind != Stmt::Kind::Block) { err(s.body->loc, "the body of a switch is not a compound statement"); return; }
        if (!s.body->loc.known()) err(s.body->loc, "statement without a source position");
        targets_.push_back(Target{s.jump_id, false});
        scopes_.emplace_back();
        if (declares_cell(*s.body) && !s.body->end_loc.known())
          err(s.body->loc, "the body of a switch declares a cell but has no closing-brace position (decision 0032 item 8)");
        std::set<std::string> values;
        int defaults = 0;
        bool declaration_seen = false;
        bool seen_case = false;  // a case label has come
        for (const Stmt* c : s.body->stmts) {
          if (c == nullptr) { err(s.body->loc, "a null statement in a block"); continue; }
          if (c->kind == Stmt::Kind::Case) {
            seen_case = true;
            if (!c->loc.known()) err(c->loc, "statement without a source position");
            if (declaration_seen) err(c->loc, "a declaration in the body of a switch that this case label jumps over");
            if (c->expr == nullptr) {
              if (++defaults > 1) err(c->loc, "a second `default` label in one switch");
            } else {
              // The value is any call-free integer expression of the scrutinee's type (the encoder uses the case constant's
              // original expression, 0010 item 12); EDG guarantees that the folded values are distinct.
              no_ptr_ = true;
              expr(*c->expr, Ctx::Annot, false);
              no_ptr_ = false;
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
            // A declaration without an initializer is not executed, but its variable is in scope in the cases (decision 0019 item 6).
            if (!seen_case && !(c->kind == Stmt::Kind::Decl && c->expr == nullptr))
              err(c->loc, "a statement before the first case label of a switch (it is unreachable)");
            if (c->kind == Stmt::Kind::Decl && c->expr != nullptr) declaration_seen = true;
            child(*c);
          }
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
          expr(*s.expr, Ctx::Body, false);
          if (s.expr->type != fn_->ret)
            err(s.expr->loc, "return of a " + s.expr->type.str() + " from a function returning " + fn_->ret.str());
        }
        break;
      }
      case Stmt::Kind::Check: {
        if (s.expr == nullptr) { err(s.loc, "a check without an expression"); return; }
        expr(*s.expr, s.from_assert ? Ctx::Assert : Ctx::Annot, false);
        if (!s.expr->type.is_bool()) err(s.expr->loc, "a check of type " + s.expr->type.str() + " (it must be a bool)");
        break;
      }
      case Stmt::Kind::Block: {
        scopes_.emplace_back();
        if (declares_cell(s) && !s.end_loc.known())
          err(s.loc, "a block that declares a variable whose address is taken has no closing-brace position (decision 0032 item 8)");
        for (const Stmt* c : s.stmts) {
          if (c == nullptr) { err(s.loc, "a null statement in a block"); continue; }
          child(*c);
        }
        scopes_.pop_back();
        break;
      }
      case Stmt::Kind::Store: {
        if (s.addr == nullptr || s.expr == nullptr) { err(s.loc, "a store without an address or a value"); return; }
        expr(*s.addr, Ctx::Body, false);
        // A compound store's value reads a Deref that shares the address node (decision 0039 item 5): the address is checked once.
        shared_addrs_.insert(s.addr);
        expr(*s.expr, Ctx::Body, false);
        shared_addrs_.erase(s.addr);
        if (!s.addr->type.is_ptr()) err(s.addr->loc, "a store through a " + s.addr->type.str() + ", not a pointer");
        else if (accepted_pointer(s.addr->type) && s.expr->type != *s.addr->type.pointee)
          err(s.expr->loc, "a store of a " + s.expr->type.str() + " through a " + s.addr->type.str() + " (the pointee is " + s.addr->type.pointee->str() + ")");
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

  // Decision 0021 item 6: no rule about the number or the placement of calls is checked here (a call in a guarded operand, a loop
  // condition or next to another call); lowering decides the order of calls exactly and reports what it cannot place.
  // `void_ok`: a Call of a void function is allowed here (an expression statement).
  void expr(const Expr& e, Ctx ctx, bool void_ok) {
    if (!e.loc.known()) err(e.loc, "expression without a source position");
    if (e.type.is_void()) {
      if (e.kind != Expr::Kind::Call) err(e.loc, "an expression of type void that is not a call");
      else if (!void_ok) err(e.loc, "the value of a void call is used");
    } else if (e.type.is_ptr()) {
      if (!accepted_pointer(e.type)) err(e.loc, "an expression of type " + e.type.str() + ", " + pointer_reason(e.type));
    } else if (!value_type(e.type)) {
      err(e.loc, "an expression of an unsupported type");
    }
    if (pointer_node(e)) {
      const bool contract = ctx == Ctx::Requires || ctx == Ctx::Ensures;
      if (no_ptr_)
        err(e.loc, "a pointer (or a variable whose address is taken) in an invariant, a measure, a constant or a case value (decision 0032 item 10)");
      else if (contract && e.kind == Expr::Kind::AddrOf)
        err(e.loc, "an address in a contract (decision 0037 item 5: a contract names a pointer parameter, never `&x`)");
    }
    auto sub = [&](const Expr* x) {
      if (x == nullptr) { err(e.loc, "a missing operand"); return false; }
      expr(*x, ctx, false);
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
        ++old_depth_;
        if (sub(e.a) && e.a->type != e.type) err(e.loc, "`old` changes the type");
        --old_depth_;
        break;
      case Expr::Kind::Unary:
        if (!sub(e.a)) break;
        if (e.uop == UnaryOp::Not) {
          if (!e.a->type.is_bool()) err(e.loc, "operand of `!` is " + e.a->type.str() + ", not bool (convert it with to_bool)");
          if (!e.type.is_bool()) err(e.loc, "`!` has a non-bool type");
        } else {
          if (!e.a->type.is_int()) err(e.loc, "operand of unary arithmetic is " + e.a->type.str() + ", not an integer");
          else if (is_enum(e.a->type)) err(e.loc, "unary arithmetic on a value of the enumeration " + e.a->type.str() + " (decision 0018 item 6)");
          if (e.type != e.a->type) err(e.loc, "unary arithmetic changes the type");
        }
        break;
      case Expr::Kind::Binary: binary(e, ctx); break;
      case Expr::Kind::Cond:
        if (e.a == nullptr || e.b == nullptr || e.c == nullptr) { err(e.loc, "a conditional with a missing operand"); break; }
        expr(*e.a, ctx, false);
        expr(*e.b, ctx, false);
        expr(*e.c, ctx, false);
        if (!e.a->type.is_bool()) err(e.loc, "the condition of `?:` is " + e.a->type.str() + ", not bool (convert it with to_bool)");
        {
          // A Null arm takes the other arm's pointer type (decision 0032 item 7).
          const bool b_null = e.b->kind == Expr::Kind::Null && e.c->type.is_ptr();
          const bool c_null = e.c->kind == Expr::Kind::Null && e.b->type.is_ptr();
          if (e.b->type != e.c->type && !b_null && !c_null)
            err(e.loc, "the branches of `?:` have different types " + e.b->type.str() + " and " + e.c->type.str());
          if (e.type != (b_null ? e.c->type : e.b->type)) err(e.loc, "`?:` has the wrong type");
        }
        break;
      case Expr::Kind::Null:
        if (!e.type.is_ptr()) err(e.loc, "a null pointer constant of type " + e.type.str());
        break;
      case Expr::Kind::AddrOf:
        if (e.var == nullptr || !owned(*e.var) || !visible(*e.var)) { err(e.loc, "the address of a variable that is not declared here"); break; }
        if (!e.var->address_taken) err(e.loc, "internal: the address of '" + e.var->name + "', which is not marked address-taken (decision 0032 item 4)");
        if (!scalar_type(e.var->type) && !accepted_pointer(e.var->type))
          err(e.loc, "the address of '" + e.var->name + "', whose type " + e.var->type.str() + " is not a bool, an integer or a pointer to one");
        else if (!e.type.is_ptr() || e.type.pointee == nullptr || *e.type.pointee != e.var->type) err(e.loc, "the address of '" + e.var->name + "' has the wrong type");
        break;
      case Expr::Kind::Deref:
        if (e.a != nullptr && shared_addrs_.count(e.a) != 0) {  // the address of the enclosing store, already checked (decision 0039 item 5)
          if (!e.a->type.is_ptr()) err(e.loc, "a dereference of a " + e.a->type.str() + ", not a pointer");
          else if (e.a->type.pointee == nullptr || e.type != *e.a->type.pointee) err(e.loc, "a dereference whose type is not the pointee's");
          break;
        }
        if (!sub(e.a)) break;
        if (!e.a->type.is_ptr()) err(e.loc, "a dereference of a " + e.a->type.str() + ", not a pointer");
        else if (e.a->type.pointee == nullptr || e.type != *e.a->type.pointee) err(e.loc, "a dereference whose type is not the pointee's");
        if ((ctx == Ctx::Requires || ctx == Ctx::Ensures) && e.a->kind != Expr::Kind::Formal && !(ctx == Ctx::Ensures && e.a->kind == Expr::Kind::Result))
          err(e.loc, "a dereference in a contract of something other than a pointer parameter or, in an `ensures`, the result (decisions 0037 item 5, 0040 items 4 and 6)");
        break;
      case Expr::Kind::Atom:
        // Decision 0037 item 5: owns(*p), reads(*p) and live(p) of a pointer parameter p, in a contract, outside `old`.  Where it may
        // stand and how it pairs with its partner is checked by `atoms`.  Decision 0040 item 5: the same of a pointer variable (a Local) in the
        // invariant of a loop (not a `do ... while (0)`); `loop_atom_sites` checks the positions.  A measure, a `check` and code hold none
        // (decision 0038 item 10): refused here, the backstop of the encoder's rejection.
        if (ctx != Ctx::Requires && ctx != Ctx::Ensures && !(ctx == Ctx::Annot && in_invariant_)) {
          err(e.loc, "a permission atom outside a contract and a loop invariant (decision 0038 item 10: a measure, a `check` and code hold none; a `do ... while (0)` invariant, never)");
          break;
        }
        if (old_depth_ > 0) { err(e.loc, "a permission atom under `old`"); break; }
        if (!e.type.is_bool()) err(e.loc, "a permission atom is not a bool");
        if (!sub(e.a)) break;
        if (in_invariant_ ? e.a->kind != Expr::Kind::Local : e.a->kind != Expr::Kind::Formal)
          err(e.loc, in_invariant_ ? "a permission atom of an invariant of something other than a pointer variable (decision 0040 item 5)"
                                   : "a permission atom of something other than a pointer parameter (decision 0037 item 5)");
        else if (!accepted_pointer(e.a->type))
          err(e.loc, "a permission atom of something that is not a pointer to a bool, an integer or a pointer");
        break;
      case Expr::Kind::Convert:
        if (!sub(e.a)) break;
        if (e.a->type.is_ptr() || e.type.is_ptr()) { err(e.loc, "a conversion between " + e.a->type.str() + " and " + e.type.str() + " (decision 0032 item 3)"); break; }
        if (!e.a->type.is_int() || !e.type.is_int()) err(e.loc, "a conversion between " + e.a->type.str() + " and " + e.type.str() + " (use to_bool / bool_to_int for bool)");
        break;
      case Expr::Kind::ToBool:
        if (!sub(e.a)) break;
        if (!e.a->type.is_int() && !e.a->type.is_ptr()) err(e.loc, "to_bool of a " + e.a->type.str());
        if (!e.type.is_bool()) err(e.loc, "to_bool does not give a bool");
        break;
      case Expr::Kind::BoolToInt:
        if (!sub(e.a)) break;
        if (!e.a->type.is_bool()) err(e.loc, "bool_to_int of a " + e.a->type.str());
        if (!e.type.is_int()) err(e.loc, "bool_to_int does not give an integer");
        else if (is_enum(e.type)) err(e.loc, "bool_to_int to the enumeration " + e.type.str() + " (decision 0018 item 5)");
        break;
      case Expr::Kind::ReportAt:
        if (sub(e.a) && e.type != e.a->type) err(e.loc, "report_at changes the type");
        break;
      case Expr::Kind::Call: call(e, ctx); break;
      case Expr::Kind::AssignE: assign_value(e, ctx); break;
      case Expr::Kind::StoreE: store_value(e, ctx); break;
      case Expr::Kind::Comma: comma(e, ctx); break;
    }
  }

  static bool effects_allowed(Ctx ctx) { return ctx == Ctx::Body || ctx == Ctx::LoopCond; }

  static bool contains_node(const Expr& root, const Expr* needle) {
    if (&root == needle) return true;
    for (const Expr* x : {root.a, root.b, root.c})
      if (x != nullptr && contains_node(*x, needle)) return true;
    for (const Expr* x : root.args)
      if (x != nullptr && contains_node(*x, needle)) return true;
    return false;
  }

  // An effect used as a value (decision 0020): in code and in a loop condition only; once per function; of a visible local; the
  // stored value has the local's type; the old value, if any, is the one `Local` node that the stored value reads.
  void assign_value(const Expr& e, Ctx ctx) {
    if (!effects_allowed(ctx)) {
      err(e.loc, "a side effect (an assignment, increment or decrement used as a value) in a contract, an annotation or an `assert`");
      return;
    }
    if (!seen_effects_.insert(&e).second) err(e.loc, "an assignment used as a value that is reachable twice (its effect would run twice)");
    if (e.var == nullptr || e.a == nullptr) { err(e.loc, "an assignment used as a value without a variable or a value"); return; }
    if (!owned(*e.var)) err(e.loc, "variable '" + e.var->name + "' does not belong to function '" + fn_->c_name + "'");
    else if (!visible(*e.var)) err(e.loc, "assignment to '" + e.var->name + "', which is not declared here");
    if (e.type != e.var->type) err(e.loc, "an assignment used as a value has type " + e.type.str() + ", not the variable's " + e.var->type.str());
    expr(*e.a, ctx, false);
    if (e.a->type != e.var->type)
      err(e.a->loc, "assignment to '" + e.var->name + "' of a " + e.a->type.str() + " where " + e.var->type.str() + " is required");
    if (e.yields_old && e.b == nullptr) err(e.loc, "an assignment that yields the old value without one");
    if (e.b != nullptr) {
      if (e.b->kind != Expr::Kind::Local || e.b->var != e.var) err(e.b->loc, "the old value of an assignment is not a read of its own variable");
      else if (!contains_node(*e.a, e.b)) err(e.b->loc, "the old value of an assignment is not the node its stored value reads");
      if (!e.yields_old) err(e.loc, "an assignment with an old value that does not yield it");
    }
  }

  // A write through a pointer used as a value (decision 0039 item 2): like an AssignE, with the address `c` in place of the variable.  The
  // stored value has the pointee's type; the old value, if any, is the one `Deref` node of `c` that the stored value reads.
  void store_value(const Expr& e, Ctx ctx) {
    if (!effects_allowed(ctx)) {
      err(e.loc, "a side effect (a write through a pointer used as a value) in a contract, an annotation or an `assert`");
      return;
    }
    if (!seen_effects_.insert(&e).second) err(e.loc, "a write through a pointer used as a value that is reachable twice (its effect would run twice)");
    if (e.c == nullptr || e.a == nullptr) { err(e.loc, "a write through a pointer used as a value without an address or a value"); return; }
    expr(*e.c, ctx, false);
    shared_addrs_.insert(e.c);
    expr(*e.a, ctx, false);
    shared_addrs_.erase(e.c);
    if (!e.c->type.is_ptr()) {
      err(e.c->loc, "a store through a " + e.c->type.str() + ", not a pointer");
      return;
    }
    if (!accepted_pointer(e.c->type) || e.c->type.pointee == nullptr) return;
    if (e.type != *e.c->type.pointee) err(e.loc, "a write through a pointer used as a value has type " + e.type.str() + ", not the pointee's " + e.c->type.pointee->str());
    if (e.a->type != *e.c->type.pointee)
      err(e.a->loc, "a store of a " + e.a->type.str() + " through a " + e.c->type.str() + " (the pointee is " + e.c->type.pointee->str() + ")");
    if (e.yields_old && e.b == nullptr) err(e.loc, "a store that yields the old value without one");
    if (e.b != nullptr) {
      if (e.b->kind != Expr::Kind::Deref || e.b->a != e.c) err(e.b->loc, "the old value of a store is not a read of its own address");
      else if (!contains_node(*e.a, e.b)) err(e.b->loc, "the old value of a store is not the node its stored value reads");
      if (!e.yields_old) err(e.loc, "a store with an old value that does not yield it");
    }
  }

  // The comma operator (decision 0020): the left operand is evaluated first and its value dropped (it may be a void call); a call
  // in it is sequenced before every call of the right operand, but not before a call elsewhere in the enclosing expression
  // (lowering's floor, decision 0021).
  void comma(const Expr& e, Ctx ctx) {
    if (!effects_allowed(ctx)) {
      err(e.loc, "the comma operator in a contract, an annotation or an `assert`");
      return;
    }
    if (e.a == nullptr || e.b == nullptr) { err(e.loc, "a comma with a missing operand"); return; }
    if (!seen_effects_.insert(&e).second) err(e.loc, "a comma that is reachable twice (its effects would run twice)");
    expr(*e.a, ctx, true);
    expr(*e.b, ctx, false);
    if (e.type != e.b->type) err(e.loc, "a comma has the wrong type");
    if (e.type.is_void()) err(e.loc, "a comma of type void (a discarded comma is two statements)");
  }

  // Whether the expression holds an AssignE, a Comma or a Call (the same question as lower.cpp's has_effect): the loop condition then
  // needs the exit jump of the top-test shape (decisions 0020 and 0021).
  static bool has_effect(const Expr& e) {
    if (e.kind == Expr::Kind::AssignE || e.kind == Expr::Kind::StoreE || e.kind == Expr::Kind::Comma || e.kind == Expr::Kind::Call) return true;
    for (const Expr* x : {e.a, e.b, e.c})
      if (x != nullptr && has_effect(*x)) return true;
    for (const Expr* x : e.args)
      if (x != nullptr && has_effect(*x)) return true;
    return false;
  }

  // A loop step: an Assign or ExprStmt, a Block of steps (a discarded comma `i++, j--`, decision 0020), or an If whose two arms are both
  // present and each a step (a `void` `?:` in a `for` step is an `if`, decision 0025 item 3: the encoder always builds both arms, an empty
  // block for `(void)0`).  A one-armed If stays refused.
  static bool step_shape(const Stmt& s) {
    switch (s.kind) {
      case Stmt::Kind::Assign:
      case Stmt::Kind::ExprStmt: return true;
      case Stmt::Kind::Block:
        for (const Stmt* c : s.stmts)
          if (c == nullptr || !step_shape(*c)) return false;
        return true;
      case Stmt::Kind::If: return s.then_body != nullptr && s.else_body != nullptr && step_shape(*s.then_body) && step_shape(*s.else_body);
      default: return false;
    }
  }

  static bool mentions_state(const Expr& e) {
    if (e.kind == Expr::Kind::Result || e.kind == Expr::Kind::Local || e.kind == Expr::Kind::Call || e.kind == Expr::Kind::Formal ||
        e.kind == Expr::Kind::Old || e.kind == Expr::Kind::AssignE || e.kind == Expr::Kind::StoreE || e.kind == Expr::Kind::Comma)
      return true;
    for (const Expr* x : {e.a, e.b, e.c})
      if (x != nullptr && mentions_state(*x)) return true;
    return false;
  }

  static bool mentions_result_or_local(const Expr& e) {
    if (e.kind == Expr::Kind::Result || e.kind == Expr::Kind::Local || e.kind == Expr::Kind::Call || e.kind == Expr::Kind::AssignE ||
        e.kind == Expr::Kind::StoreE || e.kind == Expr::Kind::Comma)
      return true;
    for (const Expr* x : {e.a, e.b, e.c})
      if (x != nullptr && mentions_result_or_local(*x)) return true;
    return false;
  }

  void binary(const Expr& e, Ctx ctx) {
    if (e.a == nullptr || e.b == nullptr) { err(e.loc, "a binary operation with a missing operand"); return; }
    expr(*e.a, ctx, false);
    expr(*e.b, ctx, false);
    const Type& a = e.a->type;
    const Type& b = e.b->type;
    switch (e.bop) {
      case BinaryOp::Add: case BinaryOp::Sub: case BinaryOp::Mul: case BinaryOp::Div: case BinaryOp::Rem:
      case BinaryOp::Shl: case BinaryOp::Shr:
      case BinaryOp::BitAnd: case BinaryOp::BitOr: case BinaryOp::BitXor:
        // Arithmetic, shifts and bitwise operators (decision 0023 item 4) are computed in the promoted type, which differs between C and C++ for an enumeration
        // (decision 0018 item 6): the encoder converts or rejects first; this is the backstop.
        if (is_enum(a) || is_enum(b))
          err(e.loc, e.bop == BinaryOp::BitAnd || e.bop == BinaryOp::BitOr || e.bop == BinaryOp::BitXor
                         ? "a bitwise operator on a value of an enumeration (decision 0023 item 4)"
                         : "arithmetic or a shift on a value of an enumeration (decision 0018 item 6)");
        break;
      default: break;
    }
    switch (e.bop) {
      case BinaryOp::Add: case BinaryOp::Sub: case BinaryOp::Mul: case BinaryOp::Div: case BinaryOp::Rem:
        if (!a.is_int() || a != b) err(e.loc, "arithmetic operands have types " + a.str() + " and " + b.str() + ": both must be the same integer type (EDG's conversions are explicit)");
        if (e.type != a) err(e.loc, "arithmetic gives " + e.type.str() + ", not " + a.str());
        break;
      case BinaryOp::BitAnd: case BinaryOp::BitOr: case BinaryOp::BitXor:
        if (!a.is_int() || a != b) err(e.loc, "bitwise operands have types " + a.str() + " and " + b.str() + ": both must be the same integer type (the conversions are explicit)");
        if (e.type != a) err(e.loc, "a bitwise operation gives " + e.type.str() + ", not " + a.str());
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
        if (a.is_ptr() && b.is_ptr()) {
          // Two pointers of one type; a Null takes the other side's type (decision 0032 item 7).
          if (a != b && e.a->kind != Expr::Kind::Null && e.b->kind != Expr::Kind::Null)
            err(e.loc, "equality operands have types " + a.str() + " and " + b.str() + ": pointers of different types (decision 0032 item 3)");
        } else if (a != b || !(a.is_int() || a.is_bool())) {
          err(e.loc, "equality operands have types " + a.str() + " and " + b.str() + ": both must be the same integer type, both bool or both pointers of one type");
        }
        if (!e.type.is_bool()) err(e.loc, "an equality gives " + e.type.str() + ", not bool (wrap it with bool_to_int for C)");
        break;
      case BinaryOp::LAnd: case BinaryOp::LOr:
        if (!a.is_bool() || !b.is_bool()) err(e.loc, "logical operands have types " + a.str() + " and " + b.str() + ": both must be bool (convert with to_bool)");
        if (!e.type.is_bool()) err(e.loc, "a logical operation gives " + e.type.str() + ", not bool");
        break;
    }
  }

  void call(const Expr& e, Ctx ctx) {
    if (!seen_effects_.insert(&e).second) err(e.loc, "a call that is reachable twice (it would run twice)");
    if (ctx == Ctx::Annot)
      err(e.loc, "a call in a loop condition or in a `check`, `invariant` or `decreases` clause (a call is hoisted before its statement, "
                 "where C/C++ does not evaluate these)");
    else if (ctx != Ctx::Body && ctx != Ctx::LoopCond && ctx != Ctx::Assert)
      err(e.loc, "a call in a contract");
    if (e.callee == nullptr || functions_.count(e.callee) == 0) { err(e.loc, "a call of a function that is not in the program"); return; }
    const Function& f = *e.callee;
    if (e.args.size() != f.params.size()) {
      err(e.loc, "call of '" + f.c_name + "' with " + std::to_string(e.args.size()) + " arguments, " +
                     std::to_string(f.params.size()) + " expected");
    }
    for (size_t k = 0; k < e.args.size(); ++k) {
      if (e.args[k] == nullptr) { err(e.loc, "a missing argument"); continue; }
      expr(*e.args[k], ctx, false);
      if (k < f.params.size() && e.args[k]->type != f.params[k].type)
        err(e.args[k]->loc, "argument " + std::to_string(k + 1) + " of '" + f.c_name + "' is " + e.args[k]->type.str() + ", the parameter is " + f.params[k].type.str() + " (convert it)");
    }
    if (e.type != f.ret) err(e.loc, "a call of '" + f.c_name + "' with the wrong result type");
  }

  const Program& prog_;
  std::unordered_set<const Function*> functions_;
  const Function* fn_ = nullptr;
  std::set<int> declared_;
  std::set<const Expr*> seen_effects_;  // the AssignE, StoreE, Comma and Call nodes met in the current function (each may be reachable once)
  bool in_invariant_ = false;  // checking a loop invariant clause: a permission atom of a pointer variable is accepted (decision 0040 item 5)
  std::set<const Expr*> shared_addrs_;  // the address nodes of the stores being checked: a compound store's Deref shares its address (0039 item 5)
  struct Target {
    int id;
    bool is_loop;
  };
  std::set<int> jump_ids_;
  std::vector<Target> targets_;  // the loops and switches around the statement being checked, innermost last
  std::vector<std::set<int>> scopes_;
  bool no_ptr_ = false;  // checking a function's measure, a constant or a case value: no pointer node (a contract clause or a loop's invariant or measure may hold some, decisions 0037 and 0038)
  int old_depth_ = 0;    // inside an `Old`
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
          if (c != nullptr && c->kind == Stmt::Kind::Decl && c->expr != nullptr) skipped = c;
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

namespace {

// The sequencing rule (decision 0020 item 4; the comment at SequenceProblem in vir.h): the objects each operand writes and reads.  An object
// is a local (key: its var id; a cell by name counts as its variable, and `*&x` is `x`) or, decision 0039 item 3, `*v` for a pointer variable
// `v` as written, whether or not the full expression also writes `v` (key: -(id + 1), one object per `v`).
struct Wr {
  std::map<int, const Expr*> w;   // object -> an AssignE or StoreE writing it, anywhere inside
  std::map<int, const Expr*> wu;  // the same for the writes no sequence point shields from the expression's value
  std::set<int> r;                // objects read
};

class Sequencer {
 public:
  explicit Sequencer(const Function& f) : f_(f) {}

  std::vector<SequenceProblem> run() {
    if (f_.body != nullptr) stmt(f_.body);
    return std::move(out_);
  }

 private:
  // An object key: a local's id (>= 0), or, for the place `*v`, `**v`, ... of a pointer variable `v` (decision 0039 item 3, 0040 A8: each depth is its
  // own object), `-(id + 1) - stride * (depth - 1)`.
  static constexpr int kDepthStride = 1 << 20;
  static int deref_key(int var_id, int depth) { return -(var_id + 1) - kDepthStride * (depth - 1); }

  void report(const Expr* at, int var_id) {
    if (!reported_.insert({at, var_id}).second) return;
    const auto& ls = f_.locals();
    int id = var_id;
    int depth = 0;
    if (var_id < 0) {  // `*v`, `**v`, ...: the pointer variable's id and the depth
      const int m = -var_id - 1;
      id = m % kDepthStride;
      depth = m / kDepthStride + 1;
    }
    std::string name = static_cast<size_t>(id) < ls.size() ? ls[static_cast<size_t>(id)]->name : "?";
    name.insert(0, static_cast<size_t>(depth), '*');
    out_.push_back(SequenceProblem{at, name});
  }

  // The object a place `*addr` is: `x` for `&x`, `*v` for a pointer variable `v` as written (decision 0039 item 3, audit S9), and `**v`, `***v`, ... when
  // the address is itself a read through `v` (`addr` = `*v`, `**v`: decision 0040, P62 A8); none for any other address (the path-precise check of
  // lowering covers it).
  static bool object_of(const Expr* addr, int* key) {
    int depth = 0;  // the reads through the variable that the address is made of
    for (;;) {
      while (addr != nullptr && addr->kind == Expr::Kind::ReportAt) addr = addr->a;
      if (addr == nullptr) return false;
      if (addr->kind == Expr::Kind::Deref && depth < 16) {
        ++depth;
        addr = addr->a;
        continue;
      }
      break;
    }
    if (addr->var == nullptr) return false;
    if (addr->kind == Expr::Kind::AddrOf && depth == 0) {
      *key = addr->var->id;
      return true;
    }
    if (addr->kind == Expr::Kind::Local && addr->var->type.is_ptr()) {
      *key = deref_key(addr->var->id, depth + 1);
      return true;
    }
    return false;
  }

  // Two unsequenced operands: a write of one meets a write or a read of the other.
  void pair(const Wr& p, const Wr& q) {
    for (const auto& [id, node] : p.w) {
      const auto qw = q.w.find(id);
      if (qw != q.w.end()) report(qw->second, id);
      else if (q.r.count(id) != 0) report(node, id);
    }
    for (const auto& [id, node] : q.w)
      if (p.r.count(id) != 0 && p.w.count(id) == 0) report(node, id);
  }

  static void absorb(Wr& into, const Wr& from, bool shielded) {
    for (const auto& [id, node] : from.w) into.w.emplace(id, node);
    for (const auto& id : from.r) into.r.insert(id);
    if (!shielded)
      for (const auto& [id, node] : from.wu) into.wu.emplace(id, node);
  }

  Wr expr(const Expr* e) {
    Wr out;
    if (e == nullptr) return out;
    switch (e->kind) {
      case Expr::Kind::Const: case Expr::Kind::Formal: case Expr::Kind::Result: case Expr::Kind::Atom: break;  // an atom is contract-only
      case Expr::Kind::Null: case Expr::Kind::AddrOf: break;  // an address is no access to the object (decision 0032)
      case Expr::Kind::Deref: {
        // A compound store's value reads a Deref that shares the store's address node: the address counts once (decision 0039 item 5).
        if (shared_.count(e->a) == 0) absorb(out, expr(e->a), false);
        int key = 0;
        if (object_of(e->a, &key)) out.r.insert(key);
        break;
      }
      case Expr::Kind::Local:
        if (e->var != nullptr) out.r.insert(e->var->id);
        break;
      case Expr::Kind::Old: case Expr::Kind::Unary: case Expr::Kind::Convert: case Expr::Kind::ToBool: case Expr::Kind::BoolToInt:
      case Expr::Kind::ReportAt:
        absorb(out, expr(e->a), false);
        break;
      case Expr::Kind::Binary: {
        const Wr a = expr(e->a);
        const Wr b = expr(e->b);
        const bool lazy = e->bop == BinaryOp::LAnd || e->bop == BinaryOp::LOr;
        if (!lazy) pair(a, b);
        absorb(out, a, lazy);  // the first operand of && and || is sequenced before the value of the whole
        absorb(out, b, false);
        break;
      }
      case Expr::Kind::Cond: {
        const Wr c = expr(e->a);
        const Wr x = expr(e->b);
        const Wr y = expr(e->c);  // the arms are not checked against each other: at most one runs
        absorb(out, c, true);
        absorb(out, x, false);
        absorb(out, y, false);
        break;
      }
      case Expr::Kind::Comma: {
        const Wr a = expr(e->a);
        const Wr b = expr(e->b);
        absorb(out, a, true);
        absorb(out, b, false);
        break;
      }
      case Expr::Kind::Call: {
        std::vector<Wr> args;
        for (const Expr* a : e->args) args.push_back(expr(a));
        for (size_t i = 0; i < args.size(); ++i)
          for (size_t j = i + 1; j < args.size(); ++j) pair(args[i], args[j]);
        for (const Wr& a : args) absorb(out, a, true);  // a call is a sequence point after its arguments
        break;
      }
      case Expr::Kind::AssignE: {
        const Wr a = expr(e->a);
        if (e->var != nullptr) {
          const auto hit = a.wu.find(e->var->id);
          if (hit != a.wu.end()) report(hit->second, e->var->id);  // a write of the target inside the stored value
        }
        absorb(out, a, false);
        if (e->var != nullptr) {
          out.w[e->var->id] = e;
          out.wu[e->var->id] = e;
        }
        break;
      }
      case Expr::Kind::StoreE: {
        // `*c := a` as a value: the address and the stored value are unsequenced operands (as for Stmt::Store); a write of the target
        // object inside the stored value is a conflict; the node writes the object `*c` (when it is one).
        const Wr ad = expr(e->c);
        shared_.insert(e->c);
        const Wr v = expr(e->a);
        shared_.erase(e->c);
        pair(ad, v);
        int key = 0;
        const bool obj = object_of(e->c, &key);
        if (obj) {
          const auto hit = v.wu.find(key);
          if (hit != v.wu.end()) report(hit->second, key);
        }
        absorb(out, ad, false);
        absorb(out, v, false);
        if (obj) {
          out.w[key] = e;
          out.wu[key] = e;
        }
        break;
      }
    }
    return out;
  }

  // A plain store of `value` to `target` (an Assign or a Decl).
  void store(const Var* target, const Expr* value) {
    const Wr v = expr(value);
    if (target == nullptr) return;
    const auto hit = v.wu.find(target->id);
    if (hit != v.wu.end()) report(hit->second, target->id);
  }

  void stmt(const Stmt* s) {
    if (s == nullptr) return;
    switch (s->kind) {
      case Stmt::Kind::Decl:
      case Stmt::Kind::Assign:
        if (s->expr != nullptr) store(s->var, s->expr);
        break;
      case Stmt::Kind::ExprStmt: case Stmt::Kind::Return: case Stmt::Kind::Check:
        expr(s->expr);
        break;
      case Stmt::Kind::Store: {
        // `*addr = value`: the two are unsequenced operands (decision 0032 item 9), as the operands of a Binary are, so `*p = (p = q, 1)`
        // is a conflict like any other Viper-local one; and the stored value may not write a local the address reads.
        const Wr a = expr(s->addr);
        shared_.insert(s->addr);  // a compound store's value reads a Deref sharing the address node (decision 0039 item 5)
        const Wr v = expr(s->expr);
        shared_.erase(s->addr);
        pair(a, v);
        int key = 0;
        if (object_of(s->addr, &key)) {  // the target object: a write of it inside the stored value is a conflict (`*p = (*p)++`)
          const auto hit = v.wu.find(key);
          if (hit != v.wu.end()) report(hit->second, key);
        }
        break;
      }
      case Stmt::Kind::If:
        expr(s->expr);
        stmt(s->then_body);
        stmt(s->else_body);
        break;
      case Stmt::Kind::While:
        expr(s->expr);
        stmt(s->body);
        stmt(s->step);
        break;
      case Stmt::Kind::Switch:
        expr(s->expr);
        stmt(s->body);
        break;
      case Stmt::Kind::Block:
        for (const Stmt* c : s->stmts) stmt(c);
        break;
      case Stmt::Kind::Case: case Stmt::Kind::Break: case Stmt::Kind::Continue: case Stmt::Kind::Goto: case Stmt::Kind::Label: break;
    }
  }

  const Function& f_;
  std::set<const Expr*> shared_;     // the address nodes of the stores being walked: their Deref reads count the address once
  std::vector<SequenceProblem> out_;
  std::set<std::pair<const Expr*, int>> reported_;
};

}  // namespace

std::vector<SequenceProblem> sequence_problems(const Function& f) { return Sequencer(f).run(); }

}  // namespace cerastes::vir
