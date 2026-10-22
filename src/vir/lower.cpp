// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Lowering of VIR to Viper-level form.  The meaning of every operation is decision 0010; the shape of the
// statements is decision 0012.  Section numbers refer to internal/encoding.md.
//
// How to read this file:
//  * `lower()` turns an expression into a pure Viper value (`LV`) and appends to a `Full` the obligations
//    (checks that C/C++ makes undefined or that the verifier must prove) and the calls it found.
//  * `flush()` writes a `Full` as statements, in the order of 0012 item 6: calls are hoisted into temporaries,
//    and every obligation is placed after the calls whose results it depends on (its guard counts) and
//    before every other call.
//  * Statements are lowered by `Fn::stmt()`.
#include "lower.h"

#include <algorithm>
#include <map>
#include <set>

#include "dec.h"

namespace cerastes::vir {
namespace {

using low::E;

// ---------------------------------------------------------------------------------------------------------------
// Names
// ---------------------------------------------------------------------------------------------------------------

std::string sanitize(const std::string& s) {
  std::string out;
  for (char c : s) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') out += c;
    else if (c == '$') out += "_D_";
    else {
      static const char* hex = "0123456789abcdef";
      out += "_x";
      out += hex[(static_cast<unsigned char>(c) >> 4) & 15];
      out += hex[static_cast<unsigned char>(c) & 15];
    }
  }
  return out.empty() ? "anon" : out;
}

std::string method_name(const Function& f) { return "f_" + sanitize(f.c_name) + "__" + std::to_string(f.id); }
std::string formal_name(const Function& f, int k) {
  const std::string& n = f.params[static_cast<size_t>(k)].name;
  return "p_" + sanitize(n.empty() ? "arg" : n) + "__" + std::to_string(k);
}
std::string local_name(const Var& v) { return "v_" + sanitize(v.name) + "__" + std::to_string(v.id); }
const char* kEndLabel = "l_end";
const char* kResult = "res";

// ---------------------------------------------------------------------------------------------------------------
// Integer type facts
// ---------------------------------------------------------------------------------------------------------------

std::string spell(const Type& t, const std::string& c_name) {
  if (!c_name.empty()) return c_name;
  return t.str();
}
std::string spell(const Expr& e) { return spell(e.type, e.c_type_name); }

E range_check(const E& v, const IntType& t) {
  const dec::Range r = dec::range_of(t);
  return low::in_range(v, r.lo, r.hi);
}
// The same, as one atomic assertion (for obligations).
E range_obligation(const E& v, const IntType& t) {
  const dec::Range r = dec::range_of(t);
  return low::in_range_atomic(v, r.lo, r.hi);
}

// ---------------------------------------------------------------------------------------------------------------
// The lowering context
// ---------------------------------------------------------------------------------------------------------------

struct LV {
  E e;          // the Viper value
  int dep = -1; // the highest index of a call of the current full expression whose result it mentions
};

struct Obl {
  E guard;   // may be null
  E cond;
  low::Meta meta;
  int dep = -1;
};

struct CallItem {
  std::string method;
  std::string target;  // empty for a void call
  std::vector<E> args;
  low::Check check;
  bool has_termination = false;
  low::Check termination;
};

// What one full expression leaves behind.
struct Full {
  std::vector<Obl> obls;
  std::vector<CallItem> calls;
};

// How Local/Formal/Result are read.
struct Env {
  std::vector<E> formals;      // Formal(k): a name (contract of the function itself) or an argument value
  std::vector<int> formal_dep; // the call dependency of each (empty: none)
  E result;                    // Result
};

class Lowerer {
 public:
  Lowerer(const Program& p, const LowerOptions& o) : prog_(p), opt_(o) {}

  LowerResult run() {
    for (const Function& f : prog_.functions()) function(f);
    for (const Function& f : prog_.functions())
      if (!f.requires_.empty() || !f.ensures_.empty() || !f.decreases_.empty()) wf_method(f);
    LowerResult r;
    r.program = std::move(out_);
    r.errors = std::move(errors_);
    return r;
  }

 private:
  // ---- per-function state ----
  struct Fn {
    const Function* f = nullptr;
    std::string method;
    std::vector<std::string> var_names;  // by Var::id
    std::vector<low::LVar> temps;
    int ntemp = 0;
  };

  void internal_error(const SrcLoc& loc, std::string what) { errors_.push_back(Diag{loc, std::move(what)}); }

  const std::string& var_name(const Var& v) {
    return fn_.var_names[static_cast<size_t>(v.id)];
  }

  low::Meta meta(const char* category, const SrcLoc& loc, std::string message) {
    low::Meta m;
    m.category = category;
    m.loc = loc;
    m.message = std::move(message);
    if (report_at_ != nullptr) {
      m.has_note = true;
      m.note_loc = m.loc;
      m.note_text = report_note_ != nullptr && !report_note_->empty() ? *report_note_ : "in the default argument";
      m.loc = *report_at_;
    }
    return m;
  }

  void use(const char* prelude_fn) { out_.prelude_used.insert(prelude_fn); }

  // ---- obligations ----

  void obl(Full& full, const E& guard, int gdep, E cond, int cdep, low::Meta m) {
    Obl o;
    o.guard = guard;
    o.cond = std::move(cond);
    o.meta = std::move(m);
    o.dep = std::max(gdep, cdep);
    full.obls.push_back(std::move(o));
  }

  // ---- expressions ----

  LV lower(const Expr& e, const Env& env, const E& guard, int gdep, Full& full) {
    switch (e.kind) {
      case Expr::Kind::Const:
        if (e.type.is_bool()) return {low::bool_lit(e.value == "true"), -1};
        return {low::int_lit(e.value), -1};
      case Expr::Kind::Local:
        return {low::var(var_name(*e.var)), -1};
      case Expr::Kind::Formal: {
        const size_t k = static_cast<size_t>(e.index);
        return {env.formals[k], k < env.formal_dep.size() ? env.formal_dep[k] : -1};
      }
      case Expr::Kind::Result:
        return {env.result, -1};
      case Expr::Kind::Old:
        return lower(*e.a, env, guard, gdep, full);
      case Expr::Kind::Unary: return unary(e, env, guard, gdep, full);
      case Expr::Kind::Binary: return binary(e, env, guard, gdep, full);
      case Expr::Kind::Cond: {
        LV c = lower(*e.a, env, guard, gdep, full);
        E gt = low::and_(guard, c.e);
        E gf = low::and_(guard, low::not_(c.e));
        const int gd = std::max(gdep, c.dep);
        LV t = lower(*e.b, env, gt, gd, full);
        LV f = lower(*e.c, env, gf, gd, full);
        return {low::ite(c.e, t.e, f.e), std::max({c.dep, t.dep, f.dep})};
      }
      case Expr::Kind::Convert: return convert(e, env, guard, gdep, full);
      case Expr::Kind::ToBool: {
        LV a = lower(*e.a, env, guard, gdep, full);
        return {low::bin(low::BinOp::Ne, a.e, low::int_lit("0")), a.dep};
      }
      case Expr::Kind::BoolToInt: {
        LV a = lower(*e.a, env, guard, gdep, full);
        return {low::ite(a.e, low::int_lit("1"), low::int_lit("0")), a.dep};
      }
      case Expr::Kind::ReportAt: {
        const SrcLoc* saved = report_at_;
        const std::string* saved_note = report_note_;
        const SrcLoc where = e.loc;
        // The innermost ReportAt wins only if no outer one is active: an outer one already names the call.
        if (report_at_ == nullptr) {
          report_at_ = &where;
          report_note_ = &e.value;
        }
        LV a = lower(*e.a, env, guard, gdep, full);
        report_at_ = saved;
        report_note_ = saved_note;
        return a;
      }
      case Expr::Kind::Call: return call(e, env, guard, gdep, full);
    }
    internal_error(e.loc, "unknown expression kind");
    return {low::int_lit("0"), -1};
  }

  LV unary(const Expr& e, const Env& env, const E& guard, int gdep, Full& full) {
    LV a = lower(*e.a, env, guard, gdep, full);
    switch (e.uop) {
      case UnaryOp::Not: return {low::not_(a.e), a.dep};
      case UnaryOp::Plus: return a;
      case UnaryOp::Neg: {
        const IntType& t = *e.type.int_type;
        if (t.is_signed) {
          // -a fits iff a != T_MIN (T_MIN = -2^(bits-1), and the range is asymmetric).
          const dec::Range r = dec::range_of(t);
          obl(full, guard, gdep, low::bin(low::BinOp::Ne, a.e, low::int_lit(r.lo)), a.dep,
              meta("overflow", e.loc, "negating the smallest value of `" + spell(e) + "` overflows"));
          return {low::neg(a.e), a.dep};
        }
        use("c_wrap");
        return {low::fn("c_wrap", {low::neg(a.e), low::int_lit(dec::pow2(t.bits))}), a.dep};
      }
    }
    return a;
  }

  E wrap(const E& v, const IntType& t) {
    use("c_wrap");
    return low::fn("c_wrap", {v, low::int_lit(dec::pow2(t.bits))});
  }

  static const char* op_noun(BinaryOp op) {
    switch (op) {
      case BinaryOp::Add: return "addition";
      case BinaryOp::Sub: return "subtraction";
      case BinaryOp::Mul: return "multiplication";
      case BinaryOp::Div: return "division";
      case BinaryOp::Rem: return "remainder";
      default: return "operation";
    }
  }

  LV binary(const Expr& e, const Env& env, const E& guard, int gdep, Full& full) {
    using low::BinOp;
    const bool lazy_and = e.bop == BinaryOp::LAnd;
    const bool lazy_or = e.bop == BinaryOp::LOr;
    LV a = lower(*e.a, env, guard, gdep, full);
    E gb = guard;
    int gbd = gdep;
    if (lazy_and) { gb = low::and_(guard, a.e); gbd = std::max(gdep, a.dep); }
    if (lazy_or) { gb = low::and_(guard, low::not_(a.e)); gbd = std::max(gdep, a.dep); }
    LV b = lower(*e.b, env, gb, gbd, full);
    const int dep = std::max(a.dep, b.dep);
    switch (e.bop) {
      case BinaryOp::LAnd: return {low::bin(BinOp::And, a.e, b.e), dep};
      case BinaryOp::LOr: return {low::bin(BinOp::Or, a.e, b.e), dep};
      case BinaryOp::Lt: return {low::bin(BinOp::Lt, a.e, b.e), dep};
      case BinaryOp::Le: return {low::bin(BinOp::Le, a.e, b.e), dep};
      case BinaryOp::Gt: return {low::bin(BinOp::Gt, a.e, b.e), dep};
      case BinaryOp::Ge: return {low::bin(BinOp::Ge, a.e, b.e), dep};
      case BinaryOp::Eq: return {low::bin(BinOp::Eq, a.e, b.e), dep};
      case BinaryOp::Ne: return {low::bin(BinOp::Ne, a.e, b.e), dep};
      case BinaryOp::Add: case BinaryOp::Sub: case BinaryOp::Mul: {
        const IntType& t = *e.type.int_type;
        const BinOp op = e.bop == BinaryOp::Add ? BinOp::Add : (e.bop == BinaryOp::Sub ? BinOp::Sub : BinOp::Mul);
        E v = low::bin(op, a.e, b.e);
        if (t.is_signed) {
          obl(full, guard, gdep, range_obligation(v, t), dep,
              meta("overflow", e.loc, std::string("signed `") + spell(e) + "` " + op_noun(e.bop) + " may overflow"));
          return {v, dep};
        }
        return {wrap(v, t), dep};
      }
      case BinaryOp::Div: case BinaryOp::Rem: {
        const IntType& t = *e.type.int_type;
        const bool div = e.bop == BinaryOp::Div;
        obl(full, guard, gdep, low::bin(BinOp::Ne, b.e, low::int_lit("0")), b.dep,
            meta("division-by-zero", e.loc, div ? "the divisor may be zero" : "the divisor of the remainder may be zero"));
        if (t.is_signed) {
          const dec::Range r = dec::range_of(t);
          E bad = low::and_(low::bin(BinOp::Eq, a.e, low::int_lit(r.lo)), low::bin(BinOp::Eq, b.e, low::int_lit("-1")));
          obl(full, guard, gdep, low::not_(bad), dep,
              meta("overflow", e.loc, std::string(div ? "dividing" : "taking the remainder of") + " the smallest value of `" +
                                          spell(e) + "` by -1 overflows"));
        }
        // Unsigned operands are non-negative, and the plain functions are easier for the solver (prelude.h).
        const char* name = t.is_signed ? (div ? "c_div" : "c_rem") : (div ? "c_udiv" : "c_urem");
        use(name);
        return {low::fn(name, {a.e, b.e}), dep};
      }
      case BinaryOp::Shl: case BinaryOp::Shr: {
        const IntType& t = *e.type.int_type;  // the (promoted) left operand's type
        const bool left = e.bop == BinaryOp::Shl;
        const std::string width = std::to_string(t.bits);
        obl(full, guard, gdep,
            low::in_range_atomic(b.e, "0", dec::dec1(width)), b.dep,
            meta("shift", e.loc, "the shift count may be negative or at least the width (" + width + " bits) of `" + spell(e) + "`"));
        use("c_pow2");
        if (!left) {
          use("c_shr");
          return {low::fn("c_shr", {a.e, b.e}), dep};
        }
        E v = low::bin(BinOp::Mul, a.e, low::fn("c_pow2", {b.e}));
        if (t.is_signed) {
          // C23 6.5.8: a negative left operand, or a result that does not fit, is undefined; cerastes uses
          // that rule in C++20 too (0010 item 8).
          obl(full, guard, gdep, low::bin(BinOp::Ge, a.e, low::int_lit("0")), a.dep,
              meta("shift", e.loc, "the left operand of `<<` may be negative"));
          const dec::Range r = dec::range_of(t);
          obl(full, guard, gdep, low::bin(BinOp::Le, v, low::int_lit(r.hi)), dep,
              meta("shift", e.loc, "the result of `<<` may not fit in `" + spell(e) + "`"));
          return {v, dep};
        }
        return {wrap(v, t), dep};
      }
    }
    internal_error(e.loc, "unknown binary operator");
    return a;
  }

  LV convert(const Expr& e, const Env& env, const E& guard, int gdep, Full& full) {
    LV a = lower(*e.a, env, guard, gdep, full);
    const IntType& s = *e.a->type.int_type;
    const IntType& t = *e.type.int_type;
    if (dec::contains(dec::range_of(t), dec::range_of(s))) return a;  // every source value is a target value
    if (t.is_signed) {
      obl(full, guard, gdep, range_obligation(a.e, t), a.dep,
          meta("conversion", e.loc,
               std::string("the value may not fit in `") + spell(e) + "` (a conversion to a signed type must be exact)"));
      return a;
    }
    return {wrap(a.e, t), a.dep};  // to an unsigned type: modulo 2^bits (both standards)
  }

  LV call(const Expr& e, const Env& env, const E& guard, int gdep, Full& full) {
    const Function& callee = *e.callee;
    if (guard != nullptr) internal_error(e.loc, "a call in a conditionally evaluated operand reached lowering");
    std::vector<LV> args;
    for (const Expr* a : e.args) args.push_back(lower(*a, env, nullptr, -1, full));
    (void)gdep;
    // The callee's `requires`, over the argument values, are obligations at the call (0012 item 6).
    Env cenv;
    for (const LV& a : args) {
      cenv.formals.push_back(a.e);
      cenv.formal_dep.push_back(a.dep);
    }
    for (const Clause& c : callee.requires_) {
      Full scratch;  // the clause's own obligations belong to the callee's well-formedness method
      LV v = lower(*c.expr, cenv, nullptr, -1, scratch);
      low::Meta m = meta("precondition", e.loc, "the precondition of `" + callee.c_name + "` may not hold");
      m.has_note = true;
      m.note_loc = c.loc.known() ? c.loc : callee.loc;
      m.note_text = "this is the requirement of `" + callee.c_name + "`";
      obl(full, nullptr, -1, v.e, v.dep, std::move(m));
    }
    CallItem item;
    item.method = method_name(callee);
    for (const LV& a : args) item.args.push_back(a.e);
    low::Meta backstop = meta("internal", e.loc, "internal check: Silicon's own check of the precondition of `" + callee.c_name + "`");
    item.check = low::Check{"call.precondition", std::move(backstop)};
    if (callee.body != nullptr && !callee.decreases_.empty()) {
      // Silicon checks, at a call from a function with a measure, that the callee's measure over the arguments
      // decreases when the callee is in the same recursion cycle (decision 0013 items 2 and 8).
      low::Meta tm = meta("termination", e.loc,
                          "the recursion through this call to `" + callee.c_name +
                              "` may not terminate: its measure may not decrease, or may not be bounded below by zero");
      tm.has_note = true;
      tm.note_loc = callee.decreases_.front().loc.known() ? callee.decreases_.front().loc : callee.loc;
      tm.note_text = "this is the measure of `" + callee.c_name + "`";
      item.has_termination = true;
      item.termination = low::Check{"termination.failed", std::move(tm)};
    }
    const int idx = static_cast<int>(full.calls.size());
    E value;
    if (!callee.ret.is_void()) {
      item.target = "t__" + std::to_string(fn_.ntemp++);
      fn_.temps.push_back(low::LVar{item.target, callee.ret.is_bool()});
      value = low::var(item.target);
    }
    full.calls.push_back(std::move(item));
    return {value, idx};
  }

  // ---- emitting a full expression (0012 item 6) ----

  low::LStmt assert_stmt(E cond, low::Meta m) {
    low::LStmt s;
    s.kind = low::LStmt::Kind::Assert;
    s.expr = std::move(cond);
    s.check = low::Check{"assert.failed", std::move(m)};
    return s;
  }

  void emit_assert(std::vector<low::LStmt>& out, E cond, low::Meta m) {
    if (assert_count_++ == opt_.drop_assert) return;  // self-test hook (0011 item 13)
    out.push_back(assert_stmt(std::move(cond), std::move(m)));
  }

  // Calls are hoisted in evaluation order (a nested chain, so innermost first).  An obligation goes right
  // after the call with the highest index it depends on, its guard included, or before the first call when it
  // depends on none: a callee that does not return cannot hide a check that does not depend on it.
  void flush(Full& full, std::vector<low::LStmt>& out) {
    const int n = static_cast<int>(full.calls.size());
    for (int slot = -1; slot < n; ++slot) {
      for (Obl& o : full.obls) {
        const int d = std::min(o.dep, n - 1);
        if (d == slot) emit_assert(out, low::implies(o.guard, o.cond), o.meta);
      }
      if (slot + 1 < n) {
        CallItem& c = full.calls[static_cast<size_t>(slot + 1)];
        low::LStmt s;
        s.kind = low::LStmt::Kind::Call;
        s.name = c.method;
        s.target = c.target;
        s.args = c.args;
        s.check = c.check;
        s.has_termination = c.has_termination;
        s.termination = c.termination;
        out.push_back(std::move(s));
      }
    }
  }

  Env body_env() const { return Env{}; }

  Env contract_env() const {
    Env env;
    for (size_t k = 0; k < fn_.f->params.size(); ++k) env.formals.push_back(low::var(formal_name(*fn_.f, static_cast<int>(k))));
    if (!fn_.f->ret.is_void()) env.result = low::var(kResult);
    return env;
  }

  LV full_expr(const Expr& e, std::vector<low::LStmt>& out) {
    Full full;
    LV v = lower(e, body_env(), nullptr, -1, full);
    flush(full, out);
    return v;
  }

  // ---- statements ----

  void assign(std::vector<low::LStmt>& out, const std::string& name, E value) {
    low::LStmt s;
    s.kind = low::LStmt::Kind::Assign;
    s.name = name;
    s.expr = std::move(value);
    out.push_back(std::move(s));
  }

  void stmt(const Stmt& s, std::vector<low::LStmt>& out) {
    switch (s.kind) {
      case Stmt::Kind::Decl:
      case Stmt::Kind::Assign: {
        LV v = full_expr(*s.expr, out);
        assign(out, var_name(*s.var), v.e);
        break;
      }
      case Stmt::Kind::ExprStmt:
        full_expr(*s.expr, out);  // only its obligations and calls matter
        break;
      case Stmt::Kind::If: {
        LV c = full_expr(*s.expr, out);
        low::LStmt i;
        i.kind = low::LStmt::Kind::If;
        i.expr = c.e;
        if (s.then_body != nullptr) stmt(*s.then_body, i.then_body);
        if (s.else_body != nullptr) stmt(*s.else_body, i.else_body);
        out.push_back(std::move(i));
        break;
      }
      case Stmt::Kind::While: loop(s, out); break;
      case Stmt::Kind::Return: ret(s, out); break;
      case Stmt::Kind::Check: {
        LV v = full_expr(*s.expr, out);
        emit_assert(out, v.e, meta("check", s.loc, s.from_assert ? "this `assert` may not hold" : "this `check` may not hold"));
        break;
      }
      case Stmt::Kind::Block:
        for (const Stmt* c : s.stmts) stmt(*c, out);
        break;
      case Stmt::Kind::Switch: switch_stmt(s, out); break;
      case Stmt::Kind::Case:
        internal_error(s.loc, "a case label outside the body of its switch reached lowering");
        break;
      case Stmt::Kind::Break: jump(out, "l_brk__" + std::to_string(s.jump_id)); break;
      case Stmt::Kind::Continue: jump(out, "l_cont__" + std::to_string(s.jump_id)); break;
      case Stmt::Kind::Goto: jump(out, user_label(s)); break;
      case Stmt::Kind::Label:
        if (user_targets_.count(s.jump_id) != 0) label(out, user_label(s));
        break;
    }
  }

  // ---- jumps (decision 0014) ----

  void jump(std::vector<low::LStmt>& out, std::string label) {
    low::LStmt g;
    g.kind = low::LStmt::Kind::Goto;
    g.name = std::move(label);
    out.push_back(std::move(g));
  }

  void label(std::vector<low::LStmt>& out, std::string name) {
    low::LStmt l;
    l.kind = low::LStmt::Kind::Label;
    l.name = std::move(name);
    out.push_back(std::move(l));
  }

  // The `break`s and `continue`s inside `s` (a step is not searched: it holds none), by target id.  A label is emitted only
  // for a construct that something jumps to, so programs without these statements lower exactly as before.
  static void collect_jumps(const Stmt& s, std::set<int>& breaks, std::set<int>& continues) {
    switch (s.kind) {
      case Stmt::Kind::Break: breaks.insert(s.jump_id); break;
      case Stmt::Kind::Continue: continues.insert(s.jump_id); break;
      case Stmt::Kind::If:
        if (s.then_body != nullptr) collect_jumps(*s.then_body, breaks, continues);
        if (s.else_body != nullptr) collect_jumps(*s.else_body, breaks, continues);
        break;
      case Stmt::Kind::While:
      case Stmt::Kind::Switch:
        if (s.body != nullptr) collect_jumps(*s.body, breaks, continues);
        break;
      case Stmt::Kind::Block:
        for (const Stmt* c : s.stmts) collect_jumps(*c, breaks, continues);
        break;
      default: break;
    }
  }

  // The user labels some `goto` of the function targets (a label is emitted only then, decision 0015 item 3).
  void collect_gotos(const Stmt& s, std::set<int>& targets) {
    switch (s.kind) {
      case Stmt::Kind::Goto: targets.insert(s.jump_id); break;
      case Stmt::Kind::Label: user_names_[s.jump_id] = s.label_name; break;
      case Stmt::Kind::If:
        if (s.then_body != nullptr) collect_gotos(*s.then_body, targets);
        if (s.else_body != nullptr) collect_gotos(*s.else_body, targets);
        break;
      case Stmt::Kind::While:
        if (s.body != nullptr) collect_gotos(*s.body, targets);
        if (s.step != nullptr) collect_gotos(*s.step, targets);
        break;
      case Stmt::Kind::Switch:
        if (s.body != nullptr) collect_gotos(*s.body, targets);
        break;
      case Stmt::Kind::Block:
        for (const Stmt* c : s.stmts) collect_gotos(*c, targets);
        break;
      default: break;
    }
  }

  // A Goto carries only the label's id: its name is the Label's (collect_gotos found them all before lowering the body).
  std::string user_label(const Stmt& s) const {
    const auto it = user_names_.find(s.jump_id);
    return "l_user_" + sanitize(it != user_names_.end() ? it->second : s.label_name) + "__" + std::to_string(s.jump_id);
  }

  // `switch (e) { ... }`: the controlling expression is evaluated once into a temporary (its obligations and calls first),
  // then a chain of tests jumps to the label of the matching `case` (in source order; case values are distinct), else to the
  // `default` label, else past the switch.  The body is lowered in order with a label at each case marker, so falling through
  // is falling into the next label and `break` jumps to the label after the body.
  void switch_stmt(const Stmt& s, std::vector<low::LStmt>& out) {
    const std::string n = std::to_string(s.jump_id);
    LV v = full_expr(*s.expr, out);
    const std::string t = "t__" + std::to_string(fn_.ntemp++);
    fn_.temps.push_back(low::LVar{t, false});
    assign(out, t, v.e);
    std::set<int> breaks, continues;
    collect_jumps(*s.body, breaks, continues);
    // The case values' obligations and the guards that each equals the front end's value are in the prologue of the method
    // (decision 0017 item 5), so the dispatch compares with the front end's value and asserts nothing.
    bool has_default = false;
    size_t index = 0;
    for (const Stmt* c : s.body->stmts) {
      if (c->kind == Stmt::Kind::Case) {
        if (c->expr == nullptr) {
          has_default = true;
        } else {
          low::LStmt i;
          i.kind = low::LStmt::Kind::If;
          i.expr = low::bin(low::BinOp::Eq, low::var(t), low::int_lit(c->folded));
          jump(i.then_body, "l_case__" + n + "_" + std::to_string(index));
          out.push_back(std::move(i));
        }
      }
      ++index;
    }
    const bool need_break_label = !has_default || breaks.count(s.jump_id) != 0;
    jump(out, has_default ? "l_default__" + n : "l_brk__" + n);
    index = 0;
    for (const Stmt* c : s.body->stmts) {
      if (c->kind == Stmt::Kind::Case) label(out, c->expr == nullptr ? "l_default__" + n : "l_case__" + n + "_" + std::to_string(index));
      else stmt(*c, out);
      ++index;
    }
    if (need_break_label) label(out, "l_brk__" + n);
  }

  void ret(const Stmt& s, std::vector<low::LStmt>& out) {
    if (s.expr != nullptr) {
      LV v = full_expr(*s.expr, out);
      assign(out, kResult, v.e);
    }
    if (fn_.f->noreturn) {
      // decision 0015 item 4: a function declared not to return must not return; a reachable `return` is the error.
      emit_assert(out, low::bool_lit(false),
                  meta("noreturn", s.loc, "`" + fn_.f->c_name + "` is declared not to return, but it may return here"));
    } else {
      postconditions(s.loc, out);
    }
    low::LStmt g;
    g.kind = low::LStmt::Kind::Goto;
    g.name = kEndLabel;
    out.push_back(std::move(g));
  }

  // Assert each user `ensures` at the point where the function returns (0012 item 9).
  void postconditions(const SrcLoc& at, std::vector<low::LStmt>& out) {
    const Env env = contract_env();
    for (const Clause& c : fn_.f->ensures_) {
      Full scratch;
      LV v = lower(*c.expr, env, nullptr, -1, scratch);
      low::Meta m = meta("postcondition", at, "the postcondition of `" + fn_.f->c_name + "` may not hold");
      m.has_note = true;
      m.note_loc = c.loc;
      m.note_text = "this is the postcondition";
      emit_assert(out, v.e, std::move(m));
    }
  }

  // Locals that a loop assigns but does not declare: the ones whose values cross iterations.
  static void scan(const Stmt& s, std::set<int>& assigned, std::set<int>& declared) {
    switch (s.kind) {
      case Stmt::Kind::Decl: declared.insert(s.var->id); break;
      case Stmt::Kind::Assign: assigned.insert(s.var->id); break;
      case Stmt::Kind::If:
        if (s.then_body != nullptr) scan(*s.then_body, assigned, declared);
        if (s.else_body != nullptr) scan(*s.else_body, assigned, declared);
        break;
      case Stmt::Kind::While:
        if (s.body != nullptr) scan(*s.body, assigned, declared);
        if (s.step != nullptr) scan(*s.step, assigned, declared);
        break;
      case Stmt::Kind::Switch:
        if (s.body != nullptr) scan(*s.body, assigned, declared);
        break;
      case Stmt::Kind::Block:
        for (const Stmt* c : s.stmts) scan(*c, assigned, declared);
        break;
      default: break;
    }
  }

  // A loop.  Its invariants are, in order: the automatic range invariant of each outer local the loop assigns;
  // then, for each user invariant, the obligations of its own expression (guarded by the earlier user
  // invariants) and the invariant itself; then the obligations of the loop condition (guarded by all user
  // invariants).  An obligation as an invariant clause is checked exactly where 0012 item 8 asks: before the
  // first evaluation (on entry) and before each next one (preservation), and it is then known inside the body.
  //
  // Decision 0014: a `do` loop is `while (true)` with the condition's obligations and the exit test at the end of the body
  // (item 5); a `for` loop's step follows the `continue` label; `do ... while (0)` is no loop at all (do_once).
  void loop(const Stmt& s, std::vector<low::LStmt>& out) {
    if (s.form == LoopForm::DoOnce) {
      do_once(s, out);
      return;
    }
    const bool is_do = s.form == LoopForm::DoWhile;
    low::LStmt w;
    w.kind = low::LStmt::Kind::While;

    std::set<int> assigned, declared;
    scan(*s.body, assigned, declared);
    if (s.step != nullptr) scan(*s.step, assigned, declared);
    for (int id : assigned) {
      if (declared.count(id) != 0) continue;
      const Var& v = *fn_.f->locals()[static_cast<size_t>(id)];
      if (!v.type.is_int()) continue;
      const dec::Range r = dec::range_of(*v.type.int_type);
      low::LInvariant inv;
      inv.expr = low::in_range(low::var(var_name(v)), r.lo, opt_.weaken_auto_invariant ? r.lo : r.hi);
      for (const char* id2 : {"invariant.not.established", "invariant.not.preserved"}) {
        low::Meta m;
        m.category = "internal";
        m.loc = s.loc;
        m.message = "internal check: automatic range invariant of `" + v.name + "`";
        inv.checks.push_back(low::Check{id2, std::move(m)});
      }
      w.invariants.push_back(std::move(inv));
    }

    E prior;  // the conjunction of the user invariants so far
    for (const Clause& c : s.invariants) {
      Full full;
      LV v = lower(*c.expr, body_env(), nullptr, -1, full);
      for (Obl& o : full.obls) w.invariants.push_back(obligation_clause(low::and_(prior, o.guard), o));
      low::LInvariant inv;
      inv.expr = v.e;
      low::Meta entry;
      entry.category = "invariant-entry";
      entry.loc = c.loc;
      entry.message = "this loop invariant may not hold on entry";
      low::Meta kept;
      kept.category = "invariant-preserved";
      kept.loc = c.loc;
      kept.message = "this loop invariant may not be preserved by an iteration";
      inv.checks.push_back(low::Check{"invariant.not.established", std::move(entry)});
      inv.checks.push_back(low::Check{"invariant.not.preserved", std::move(kept)});
      w.invariants.push_back(std::move(inv));
      prior = low::and_(prior, v.e);
    }

    // The measure (decision 0013): its own obligations are invariant clauses at the loop head, like the condition's
    // (the measure is evaluated where the condition is, and at the end of each iteration); the measure itself is the
    // loop's `decreases`, one line, which Silicon reports a failure at.
    for (const Clause& c : s.measure) {
      Full mfull;
      LV mv = lower(*c.expr, body_env(), nullptr, -1, mfull);
      for (Obl& o : mfull.obls) w.invariants.push_back(obligation_clause(low::and_(prior, o.guard), o));
      w.decreases.push_back(mv.e);
    }
    if (!s.measure.empty()) {
      low::Meta m;
      m.category = "termination";
      m.loc = s.measure.front().loc.known() ? s.measure.front().loc : s.loc;
      m.message = "the measure of this loop may not decrease, or may not be bounded below by zero";
      w.decreases_check = low::Check{"termination.failed", std::move(m)};
    }

    std::set<int> breaks, continues;
    collect_jumps(*s.body, breaks, continues);
    const std::string n = std::to_string(s.jump_id);
    if (!is_do) {
      Full cfull;
      LV cv = lower(*s.expr, body_env(), nullptr, -1, cfull);
      for (Obl& o : cfull.obls) w.invariants.push_back(obligation_clause(low::and_(prior, o.guard), o));
      w.expr = cv.e;
    } else {
      w.expr = low::bool_lit(true);
    }
    stmt(*s.body, w.then_body);
    if (continues.count(s.jump_id) != 0) label(w.then_body, "l_cont__" + n);
    if (s.step != nullptr) stmt(*s.step, w.then_body);
    if (is_do) {
      // C evaluates the condition here, after the body and after each `continue`; its obligations are asserts.
      LV cv = full_expr(*s.expr, w.then_body);
      low::LStmt exit;
      exit.kind = low::LStmt::Kind::If;
      exit.expr = low::not_(cv.e);
      jump(exit.then_body, "l_brk__" + n);
      w.then_body.push_back(std::move(exit));
    }
    out.push_back(std::move(w));
    if (is_do || breaks.count(s.jump_id) != 0) label(out, "l_brk__" + n);
  }

  // `do { B } while (c)` where `c` is a constant zero (decision 0014 item 5): the body runs once and no iteration follows
  // another, so there is no loop and no measure.  A written invariant is asserted once, at entry (the start of the only
  // iteration), a written measure only has its obligations checked there; `continue` goes to the condition, which is evaluated
  // (for its obligations) after the body, and `break` to the end.
  void do_once(const Stmt& s, std::vector<low::LStmt>& out) {
    E prior;
    for (const Clause& c : s.invariants) {
      Full full;
      LV v = lower(*c.expr, body_env(), nullptr, -1, full);
      for (Obl& o : full.obls) emit_assert(out, low::implies(low::and_(prior, o.guard), o.cond), o.meta);
      low::Meta entry;
      entry.category = "invariant-entry";
      entry.loc = c.loc;
      entry.message = "this loop invariant may not hold on entry";
      emit_assert(out, v.e, std::move(entry));
      prior = low::and_(prior, v.e);
    }
    for (const Clause& c : s.measure) {
      Full mfull;
      lower(*c.expr, body_env(), nullptr, -1, mfull);
      for (Obl& o : mfull.obls) emit_assert(out, low::implies(low::and_(prior, o.guard), o.cond), o.meta);
    }
    std::set<int> breaks, continues;
    collect_jumps(*s.body, breaks, continues);
    const std::string n = std::to_string(s.jump_id);
    stmt(*s.body, out);
    if (continues.count(s.jump_id) != 0) label(out, "l_cont__" + n);
    LV cv = full_expr(*s.expr, out);  // the condition's obligations, where C evaluates it
    // Defense in depth: the body ran once because the condition is false.  If EDG's folded zero ever disagrees with the value
    // cerastes encoded, the run fails (an internal check, exit 3) instead of silently following the wrong path.
    low::Meta internal;
    internal.category = "internal";
    internal.loc = s.loc;
    internal.message = "internal check: the condition of a do-once loop is not false";
    emit_assert(out, low::not_(cv.e), std::move(internal));
    if (breaks.count(s.jump_id) != 0) label(out, "l_brk__" + n);
  }

  low::LInvariant obligation_clause(const E& guard, const Obl& o) {
    low::LInvariant inv;
    inv.expr = low::implies(guard, o.cond);
    for (const char* id : {"invariant.not.established", "invariant.not.preserved"}) inv.checks.push_back(low::Check{id, o.meta});
    return inv;
  }

  // ---- the prologue (decision 0017 items 4 and 5) ----

  static bool bare_const_equals(const Expr& x, const std::string& value) {
    if (x.kind != Expr::Kind::Const) return false;
    if (x.type.is_bool()) return (x.value == "true") == (value == "1" || value == "true");
    return x.value == value;
  }

  static E literal_of(const Type& t, const std::string& value) {
    if (t.is_bool()) return low::bool_lit(value == "1" || value == "true");
    return low::int_lit(value);
  }

  // `assert <e> == <value>` as an internal check at `loc` (a failure is a bug of the encoder or of the front end's folding).
  void internal_guard(std::vector<low::LStmt>& out, const E& e, const Type& t, const std::string& value, const SrcLoc& loc,
                      std::string message) {
    low::Meta m;
    m.category = "internal";
    m.loc = loc;
    m.message = std::move(message);
    emit_assert(out, low::bin(low::BinOp::Eq, e, literal_of(t, value)), std::move(m));
  }

  // One named constant: its initializer as written (obligations at their own positions), then the guard that it equals
  // the front end's value.  Nothing for a definition without an initializer.
  // A prologue expression is closed: no call, variable, parameter, result or `old` (check_program guarantees it).  Anything
  // else is an impossible state, reported as an internal error instead of a dropped call or an out-of-range index.
  bool closed_expr(const Expr& e, const char* what) {
    switch (e.kind) {
      case Expr::Kind::Call: case Expr::Kind::Formal: case Expr::Kind::Old: case Expr::Kind::Result: case Expr::Kind::Local:
        internal_error(e.loc, std::string("a call, variable, parameter, result or `old` in ") + what + " reached the prologue");
        return false;
      default: break;
    }
    for (const Expr* x : {e.a, e.b, e.c})
      if (x != nullptr && !closed_expr(*x, what)) return false;
    return true;
  }

  bool closed_result(const Full& full, const Expr& e, const char* what) {
    if (full.calls.empty()) return true;
    internal_error(e.loc, std::string("a call in ") + what + " reached the prologue");
    return false;
  }

  void named_const_check(const NamedConst& c, std::vector<low::LStmt>& out) {
    if (c.init == nullptr) return;
    if (!closed_expr(*c.init, "the initializer of a named constant")) return;
    Full full;
    LV v = lower(*c.init, body_env(), nullptr, -1, full);
    if (!closed_result(full, *c.init, "the initializer of a named constant")) return;
    for (Obl& o : full.obls) emit_assert(out, low::implies(o.guard, o.cond), o.meta);
    if (bare_const_equals(*c.init, c.value)) return;
    internal_guard(out, v.e, c.type, c.value, c.loc,
                   "internal check: the value of `" + c.c_name + "` as written differs from the front end's");
  }

  // One `case value:` label: the value as written, its obligations at the label (with a note when an obligation's own position
  // is elsewhere, as in decision 0014 item 6), then the guard that it equals the front end's value.
  void case_check(const Stmt& c, std::vector<low::LStmt>& out) {
    if (!closed_expr(*c.expr, "a case value")) return;
    Full full;
    LV v = lower(*c.expr, body_env(), nullptr, -1, full);
    if (!closed_result(full, *c.expr, "a case value")) return;
    for (Obl& o : full.obls) {
      low::Meta m = o.meta;
      if (!m.has_note && m.loc.known() && (m.loc.line != c.loc.line || m.loc.file != c.loc.file)) {
        m.has_note = true;
        m.note_loc = m.loc;
        m.note_text = "in the value of this case label";
      }
      m.loc = c.loc;
      emit_assert(out, low::implies(o.guard, o.cond), std::move(m));
    }
    if (bare_const_equals(*c.expr, c.folded)) return;
    internal_guard(out, v.e, c.expr->type, c.folded, c.loc, "internal check: case value as written differs from the front end's");
  }

  // The case labels of a body in source order (a nested switch's labels follow the outer label before them).
  void case_checks(const Stmt* s, std::vector<low::LStmt>& out) {
    if (s == nullptr) return;
    switch (s->kind) {
      case Stmt::Kind::Case:
        if (s->expr != nullptr) case_check(*s, out);
        break;
      case Stmt::Kind::Block:
        for (const Stmt* c : s->stmts) case_checks(c, out);
        break;
      case Stmt::Kind::If:
        case_checks(s->then_body, out);
        case_checks(s->else_body, out);
        break;
      case Stmt::Kind::While:
        case_checks(s->body, out);
        case_checks(s->step, out);
        break;
      case Stmt::Kind::Switch: case_checks(s->body, out); break;
      default: break;
    }
  }

  // The prologue of a method with a body: the unit's named constants, then (with `own`: a function's method and its
  // `__wf` method) its local constants and its case values.
  void prologue(std::vector<low::LStmt>& out, const Function* own) {
    for (const NamedConst& c : prog_.named_consts()) named_const_check(c, out);
    if (own == nullptr) return;
    for (const NamedConst& c : own->local_consts()) named_const_check(c, out);
    case_checks(own->body, out);
  }

  // ---- functions ----

  low::LClause clause_checks(E expr, const SrcLoc& loc, bool ensures, const std::string& what) {
    low::LClause c;
    c.expr = std::move(expr);
    auto mk = [&](const char* id, const char* text) {
      low::Meta m;
      m.category = "internal";
      m.loc = loc;
      m.message = std::string("internal check: ") + text + " of `" + what + "`";
      return low::Check{id, std::move(m)};
    };
    c.checks.push_back(mk("not.wellformed", "well-formedness of the contract"));
    if (ensures) c.checks.push_back(mk("postcondition.violated", "Silicon's own check of the postcondition"));
    return c;
  }

  void function(const Function& f) {
    fn_ = Fn{};
    fn_.f = &f;
    fn_.method = method_name(f);
    for (const auto& v : f.locals()) fn_.var_names.push_back(local_name(*v));

    low::LMethod m;
    m.name = fn_.method;
    m.comment = "C/C++ function `" + f.c_name + "`";
    for (size_t k = 0; k < f.params.size(); ++k)
      m.formals.push_back(low::LVar{formal_name(f, static_cast<int>(k)), f.params[k].type.is_bool()});
    if (!f.ret.is_void()) m.results.push_back(low::LVar{kResult, f.ret.is_bool()});

    // requires: the parameters' type ranges, then the user's (0012 item 1).
    for (size_t k = 0; k < f.params.size(); ++k) {
      if (!f.params[k].type.is_int()) continue;
      m.requires_.push_back(clause_checks(range_check(low::var(formal_name(f, static_cast<int>(k))), *f.params[k].type.int_type),
                                          f.loc, false, f.c_name));
    }
    const Env cenv = contract_env();
    for (const Clause& c : f.requires_) {
      Full scratch;
      LV v = lower(*c.expr, cenv, nullptr, -1, scratch);
      m.requires_.push_back(clause_checks(v.e, c.loc.known() ? c.loc : f.loc, false, f.c_name));
    }
    if (f.ret.is_int())
      m.ensures_.push_back(clause_checks(range_check(low::var(kResult), *f.ret.int_type), f.loc, true, f.c_name));
    for (const Clause& c : f.ensures_) {
      Full scratch;
      LV v = lower(*c.expr, cenv, nullptr, -1, scratch);
      m.ensures_.push_back(clause_checks(v.e, c.loc.known() ? c.loc : f.loc, true, f.c_name));
    }
    if (f.noreturn) m.ensures_.push_back(clause_checks(low::bool_lit(false), f.loc, true, f.c_name));  // decision 0015 item 4

    if (f.body == nullptr) {
      m.trusted_reason = f.trusted_reason;
      m.decreases_assumed = true;  // 0013 item 5
      out_.methods.push_back(std::move(m));
      return;
    }
    for (const Clause& c : f.decreases_) {
      Full scratch;  // its own obligations belong to the well-formedness method
      LV v = lower(*c.expr, cenv, nullptr, -1, scratch);
      m.decreases_.push_back(v.e);
    }
    if (!f.decreases_.empty()) {
      low::Meta dm;
      dm.category = "termination";
      dm.loc = f.decreases_.front().loc.known() ? f.decreases_.front().loc : f.loc;
      dm.message = "the measure of `" + f.c_name + "` may not decrease at a recursive call, or may not be bounded below by zero";
      m.decreases_check = low::Check{"termination.failed", std::move(dm)};
    }
    m.has_body = true;
    // The parameters' local copies (0012 item 2).
    for (size_t k = 0; k < f.params.size(); ++k)
      assign(m.body, var_name(*f.param_var(static_cast<int>(k))), low::var(formal_name(f, static_cast<int>(k))));
    prologue(m.body, &f);
    user_targets_.clear();
    user_names_.clear();
    collect_gotos(*f.body, user_targets_);
    stmt(*f.body, m.body);
    // The end of the body: a non-void function must not reach it; a void one returns.
    if (f.noreturn) {
      emit_assert(m.body, low::bool_lit(false),
                  meta("noreturn", f.end_loc, "`" + f.c_name + "` is declared not to return, but control may reach its end"));
    } else if (f.ret.is_void()) {
      postconditions(f.end_loc, m.body);
    } else {
      emit_assert(m.body, low::bool_lit(false),
                  meta("missing-return", f.end_loc, "control may reach the end of `" + f.c_name + "` without returning a value"));
    }
    low::LStmt end;
    end.kind = low::LStmt::Kind::Label;
    end.name = kEndLabel;
    m.body.push_back(std::move(end));
    for (const auto& v : f.locals()) m.locals.push_back(low::LVar{fn_.var_names[static_cast<size_t>(v->id)], v->type.is_bool()});
    for (const low::LVar& t : fn_.temps) m.locals.push_back(t);
    out_.methods.push_back(std::move(m));
  }

  // The contract's well-formedness method (0010 item 11, 0012 item 10): the obligations of each clause, asserted
  // under the clauses before it, in a method whose only assumptions are the type ranges.
  void wf_method(const Function& f) {
    fn_ = Fn{};
    fn_.f = &f;
    fn_.method = method_name(f);
    low::LMethod m;
    m.name = fn_.method + "__wf";
    m.comment = "well-formedness of the contract of C/C++ function `" + f.c_name + "`";
    for (size_t k = 0; k < f.params.size(); ++k)
      m.formals.push_back(low::LVar{formal_name(f, static_cast<int>(k)), f.params[k].type.is_bool()});
    if (!f.ret.is_void()) m.formals.push_back(low::LVar{kResult, f.ret.is_bool()});
    for (size_t k = 0; k < f.params.size(); ++k) {
      if (!f.params[k].type.is_int()) continue;
      m.requires_.push_back(clause_checks(range_check(low::var(formal_name(f, static_cast<int>(k))), *f.params[k].type.int_type),
                                          f.loc, false, f.c_name + " (well-formedness)"));
    }
    if (f.ret.is_int())
      m.requires_.push_back(clause_checks(range_check(low::var(kResult), *f.ret.int_type), f.loc, false, f.c_name + " (well-formedness)"));
    m.has_body = true;
    prologue(m.body, &f);  // the same checks as the function's own method (the driver removes identical reports); a function whose
                           // `requires` is unsatisfiable never reaches its own prologue
    const Env env = contract_env();
    E prior;
    auto go = [&](const Clause& c) {
      Full full;
      LV v = lower(*c.expr, env, nullptr, -1, full);
      for (Obl& o : full.obls) emit_assert(m.body, low::implies(low::and_(prior, o.guard), o.cond), o.meta);
      prior = low::and_(prior, v.e);
    };
    for (const Clause& c : f.requires_) go(c);
    E requires_only = prior;
    for (const Clause& c : f.ensures_) go(c);
    // The measure is evaluated over the entry values: its obligations are checked under the requires (0013 item 7).
    for (const Clause& c : f.decreases_) {
      Full full;
      LV v = lower(*c.expr, env, nullptr, -1, full);
      for (Obl& o : full.obls) emit_assert(m.body, low::implies(low::and_(requires_only, o.guard), o.cond), o.meta);
    }
    if (m.body.empty()) {
      low::LStmt nothing;
      nothing.kind = low::LStmt::Kind::Comment;
      nothing.name = "the contract has nothing that can be undefined";
      m.body.push_back(std::move(nothing));
    }
    out_.methods.push_back(std::move(m));
  }

  const Program& prog_;
  LowerOptions opt_;
  low::LProgram out_;
  std::vector<Diag> errors_;
  Fn fn_;
  const SrcLoc* report_at_ = nullptr;
  const std::string* report_note_ = nullptr;
  int assert_count_ = 0;
  std::map<int, std::string> user_names_;  // the user's name of each label of the current function, by id
  std::set<int> user_targets_;  // the ids of the user labels some goto of the current function targets
};

}  // namespace

LowerResult lower_program(const Program& program, const LowerOptions& options) {
  // Fail closed on its own: a program that does not pass the checker (a call where none is allowed, an operand of
  // the wrong type) may have parts that the lowering's scratch structures would silently drop.
  std::vector<Diag> problems = check_program(program);
  if (!problems.empty()) {
    LowerResult refused;
    refused.errors = std::move(problems);
    return refused;
  }
  return Lowerer(program, options).run();
}

}  // namespace cerastes::vir
