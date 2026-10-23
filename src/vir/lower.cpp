// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Lowering of VIR to Viper-level form.  The meaning of every operation is decision 0010; the shape of the
// statements is decision 0012.  Section numbers refer to internal/encoding.md.
//
// How to read this file:
//  * `lower()` turns an expression into a pure Viper value (`LV`) and appends to a `Full` the obligations
//    (checks that C/C++ makes undefined or that the verifier must prove) and the calls it found.
//  * `flush()` writes a `Full` as statements, in the order of 0012 item 6 as decision 0021 states it: calls are
//    hoisted into temporaries, and every obligation is placed after the fixed points (calls, and branches that hold a
//    call) of its set and before every other fixed point.  The sets of a full expression must form a chain; if they do
//    not, the program is rejected (`LowerResult::rejections`).
//  * Statements are lowered by `Fn::stmt()`.
#include "lower.h"

#include <algorithm>
#include <deque>
#include <functional>
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

// The fixed points of a full expression that C must have completed before an item is evaluated (decision 0021 item 3): their ids,
// in creation order.  Always closed downward (it holds the PRE of each member).
using Deps = std::set<int>;

Deps join(const Deps& a, const Deps& b) {
  Deps r = a;
  r.insert(b.begin(), b.end());
  return r;
}

struct LV {
  E e;      // the Viper value
  Deps dep; // the fixed points whose results it mentions (D of each, so the set is closed)
};

struct Obl {
  E guard;   // may be null
  E cond;
  low::Meta meta;
  Deps dep;         // its set: emitted after the fixed points of the set, before every other
  size_t seq = 0;   // creation order within the Full (items of one slot keep it)
  int term_of = -1; // the fixed point (a call) whose explicit termination check this is (decision 0021 item 4), else -1
};

struct CallItem {
  std::string method;
  std::string target;  // empty for a void call
  std::vector<E> args;
  low::Check check;
  bool has_termination = false;
  low::Check termination;
};

// One effect of a full expression (decision 0020 items 5 and 7, decision 0021 item 3).  The FIXED POINTS are the calls and the branches
// that contain a call: they are numbered in creation order and emitted in the order pi (see flush).  Every other item, an obligation as
// well as a write, a snapshot or a call-free branch, has a set: the fixed points C must have completed before it evaluates it (through
// the temporaries it mentions, its guard, the comma floor, and the earlier accesses to the variables it touches), and is emitted right
// after the fixed points of its set and before every other, in creation order within a slot; an item with an empty set comes before
// the first fixed point.  A callee that does not return cannot hide a check C may evaluate before the call (decision 0012 item 6): a
// callee can neither read nor write a local, and the sequencing rule leaves unsequenced operands no shared local.
struct Effect {
  enum class Kind {
    Call,      // a call (CallItem): a fixed point
    Write,     // `name := value`, then `flag := true` when `flag` is set: an assignment used as a value
    Snapshot,  // `name := value` where `name` is a fresh temporary and `value` the current value of a local this expression writes
    Branch     // `stmts`: the guarded operand of `&&`, `||` or `?:` with its own effects, as an `if`, assigning a temporary
  } kind = Kind::Call;
  CallItem call;
  std::string name;
  E value;
  std::string flag;
  std::vector<low::LStmt> stmts;
  int fixed = -1;                // the id of the fixed point (a Call, or a Branch with a call); -1 for the movable items
  Deps deps;                     // movable items: their set; fixed points: PRE (what C completes before it)
  size_t seq = 0;                // creation order within the Full
  std::set<int> reads, writes;   // the locals (Var ids) a snapshot or write, or a branch's inner effects, read and write
};

// What a rejection says about a fixed point.
struct FixedInfo {
  SrcLoc loc;
  std::string callee;  // the callee's name; empty for a branch
  std::string what;    // `the call to "f"`, or `the call to "g" in the right operand of "&&"`
};

// What one full expression leaves behind.
struct Full {
  std::vector<Obl> obls;
  std::vector<Effect> effects;
  int nfixed = 0;                       // fixed points so far: the next one's id
  std::vector<Deps> pre;                // PRE of each fixed point, by id
  std::vector<size_t> fixed_effect;     // the index in `effects` of each fixed point, by id
  std::vector<FixedInfo> info;          // by id
  size_t next_seq = 0;
  std::map<int, Deps> last_write;       // Var id -> the set of the latest write (item order of the expression so far)
  std::map<int, Deps> last_access;      // Var id -> the union of the sets of the accesses (snapshots and writes) of it so far
};

// How Local/Formal/Result are read.
struct Env {
  std::vector<E> formals;          // Formal(k): a name (contract of the function itself) or an argument value
  std::vector<Deps> formal_dep;    // the set of each (empty: none)
  E result;                        // Result
};

class Lowerer {
 public:
  Lowerer(const Program& p, const LowerOptions& o) : prog_(p), opt_(o) {}

  LowerResult run() {
    compute_cycles();
    for (const Function& f : prog_.functions()) function(f);
    for (const Function& f : prog_.functions())
      if (!f.requires_.empty() || !f.ensures_.empty() || !f.decreases_.empty()) wf_method(f);
    check_cycles();
    LowerResult r;
    r.program = std::move(out_);
    r.errors = std::move(errors_);
    r.rejections = std::move(rejections_);
    return r;
  }

 private:
  // ---- recursion cycles (decision 0021 item 4) ----

  // The functions a statement or expression calls (every Call node, dead code included: Silicon's call graph has an edge for every
  // MethodCall of a body, silver's MethodCheck.scala).  `skip_nested`: the self-test hook that forgets the calls inside an `if`, a loop
  // or a block (an under-approximation, which the check against the emitted methods must catch).
  static void calls_in_expr(const Expr* e, std::vector<const Function*>& out) {
    if (e == nullptr) return;
    if (e->kind == Expr::Kind::Call && e->callee != nullptr) out.push_back(e->callee);
    for (const Expr* x : {e->a, e->b, e->c}) calls_in_expr(x, out);
    for (const Expr* x : e->args) calls_in_expr(x, out);
  }

  static void calls_in_stmt(const Stmt* s, std::vector<const Function*>& out, bool skip_nested, bool top = true) {
    if (s == nullptr) return;
    calls_in_expr(s->expr, out);
    for (const Clause& c : s->invariants) calls_in_expr(c.expr, out);
    for (const Clause& c : s->measure) calls_in_expr(c.expr, out);
    if (skip_nested && !top) return;
    calls_in_stmt(s->then_body, out, skip_nested, false);
    calls_in_stmt(s->else_body, out, skip_nested, false);
    calls_in_stmt(s->body, out, skip_nested, false);
    calls_in_stmt(s->step, out, skip_nested, false);
    for (const Stmt* c : s->stmts) calls_in_stmt(c, out, skip_nested, false);
  }

  // The strongly connected components of a graph (Tarjan): the component number of each vertex.
  static std::vector<int> components(const std::vector<std::vector<int>>& adj) {
    const size_t n = adj.size();
    std::vector<int> number(n, -1), low(n, 0), comp(n, -1);
    std::vector<char> on_stack(n, 0);
    std::vector<int> stack;
    int counter = 0, ncomp = 0;
    std::function<void(int)> visit = [&](int v) {
      number[static_cast<size_t>(v)] = low[static_cast<size_t>(v)] = counter++;
      stack.push_back(v);
      on_stack[static_cast<size_t>(v)] = 1;
      for (int w : adj[static_cast<size_t>(v)]) {
        if (number[static_cast<size_t>(w)] < 0) {
          visit(w);
          low[static_cast<size_t>(v)] = std::min(low[static_cast<size_t>(v)], low[static_cast<size_t>(w)]);
        } else if (on_stack[static_cast<size_t>(w)] != 0) {
          low[static_cast<size_t>(v)] = std::min(low[static_cast<size_t>(v)], number[static_cast<size_t>(w)]);
        }
      }
      if (low[static_cast<size_t>(v)] == number[static_cast<size_t>(v)]) {
        int w;
        do {
          w = stack.back();
          stack.pop_back();
          on_stack[static_cast<size_t>(w)] = 0;
          comp[static_cast<size_t>(w)] = ncomp;
        } while (w != v);
        ++ncomp;
      }
    };
    for (size_t v = 0; v < n; ++v)
      if (number[v] < 0) visit(static_cast<int>(v));
    return comp;
  }

  // The smallest vertex of each vertex's component: a canonical form of the partition.
  static std::vector<int> canonical(const std::vector<int>& comp) {
    std::map<int, int> first;
    std::vector<int> out(comp.size());
    for (size_t v = 0; v < comp.size(); ++v) out[v] = first.emplace(comp[v], static_cast<int>(v)).first->second;
    return out;
  }

  // The components of the call graph of the VIR functions (the `__wf` methods and the methods without a body call nothing): the same
  // sets Silicon's termination plugin computes over the emitted methods.  A call needs the explicit check exactly when the callee is in
  // the caller's component (a self-call included).  `check_cycles` compares them with the emitted program's.
  void compute_cycles() {
    const std::deque<Function>& fns = prog_.functions();
    std::map<const Function*, int> index;
    for (const Function& f : fns) index[&f] = static_cast<int>(index.size());
    std::vector<std::vector<int>> adj(fns.size());
    for (const Function& f : fns) {
      std::vector<const Function*> callees;
      calls_in_stmt(f.body, callees, opt_.cycles_ignore_nested_calls);
      for (const Function* c : callees) {
        const auto it = index.find(c);
        if (it != index.end()) adj[static_cast<size_t>(index[&f])].push_back(it->second);
      }
    }
    const std::vector<int> comp = components(adj);
    for (const Function& f : fns) component_[&f] = comp[static_cast<size_t>(index[&f])];
  }

  static void calls_in_lowered(const std::vector<low::LStmt>& body, std::vector<std::string>& out) {
    for (const low::LStmt& st : body) {
      if (st.kind == low::LStmt::Kind::Call) out.push_back(st.name);
      calls_in_lowered(st.then_body, out);
      calls_in_lowered(st.else_body, out);
      calls_in_lowered(st.stmts, out);
    }
  }

  // Decision 0021 item 4: an under-approximated cycle would be unsound (a recursive call whose explicit check is not written is hidden
  // by an earlier call that does not return), so the components used are compared with those of the emitted methods' Call statements,
  // which are what Silicon sees; a difference is an internal error.
  void check_cycles() {
    const std::deque<Function>& fns = prog_.functions();
    std::map<std::string, int> index;
    for (const Function& f : fns) index[method_name(f)] = static_cast<int>(index.size());
    std::vector<std::vector<int>> adj(fns.size());
    for (const low::LMethod& m : out_.methods) {
      std::vector<std::string> callees;
      calls_in_lowered(m.body, callees);
      const auto it = index.find(m.name);
      if (it == index.end()) {  // a `__wf` method: Silicon's graph includes it, and it must make no call (contracts hold none)
        if (!callees.empty())
          internal_error(SrcLoc{}, "the well-formedness method `" + m.name + "` contains a call: it would be an edge of the call graph that the recursion components lack");
        continue;
      }
      for (const std::string& c : callees) {
        const auto t = index.find(c);
        if (t != index.end()) adj[static_cast<size_t>(it->second)].push_back(t->second);
      }
    }
    std::vector<int> used(fns.size());
    for (const Function& f : fns) used[static_cast<size_t>(index[method_name(f)])] = component_[&f];
    if (canonical(components(adj)) != canonical(used))
      internal_error(SrcLoc{}, "the recursion components the explicit termination checks assumed differ from the call graph of the emitted methods");
  }

  bool in_cycle(const Function* caller, const Function* callee) const {
    const auto a = component_.find(caller), b = component_.find(callee);
    return a != component_.end() && b != component_.end() && a->second == b->second;
  }

  // ---- per-function state ----
  struct Fn {
    const Function* f = nullptr;
    std::string method;
    std::vector<std::string> var_names;  // by Var::id
    std::vector<low::LVar> temps;
    int ntemp = 0;
    // Decision 0019: the locals declared without an initializer (by Var::id) each get a ghost flag `d_<name>`, true exactly
    // when the object holds a value; a loop that assigns one declared outside it gets a snapshot `ds_<name>__<loop>`.
    std::set<int> flagged;
    std::vector<low::LVar> snapshots;
    int nloop = 0;
  };

  void internal_error(const SrcLoc& loc, std::string what) { errors_.push_back(Diag{loc, std::move(what)}); }

  const std::string& var_name(const Var& v) {
    return fn_.var_names[static_cast<size_t>(v.id)];
  }

  // ---- locals declared without an initializer (decision 0019) ----

  static void find_flagged(const Stmt& s, std::set<int>& out) {
    switch (s.kind) {
      case Stmt::Kind::Decl:
        if (s.expr == nullptr && s.var != nullptr) out.insert(s.var->id);
        break;
      case Stmt::Kind::If:
        if (s.then_body != nullptr) find_flagged(*s.then_body, out);
        if (s.else_body != nullptr) find_flagged(*s.else_body, out);
        break;
      case Stmt::Kind::While:
        if (s.body != nullptr) find_flagged(*s.body, out);
        if (s.step != nullptr) find_flagged(*s.step, out);
        break;
      case Stmt::Kind::Switch:
        if (s.body != nullptr) find_flagged(*s.body, out);
        break;
      case Stmt::Kind::Block:
        for (const Stmt* c : s.stmts)
          if (c != nullptr) find_flagged(*c, out);
        break;
      default: break;
    }
  }

  bool flagged(const Var& v) const { return fn_.flagged.count(v.id) != 0; }
  std::string flag_name(const Var& v) { return "d_" + var_name(v); }

  // The fixed value a variable holds while it has none: 0 (`false`), or the low end of an enumeration's value range when 0 is
  // not in it.  It only keeps the variable inside its type's range (0019 item 5).
  E dummy_value(const Var& v) {
    if (v.type.is_bool()) return low::bool_lit(false);
    const dec::Range r = dec::range_of(*v.type.int_type);
    return low::int_lit(dec::in_range("0", r) ? "0" : r.lo);
  }

  // `d := false; v := <dummy>`: the variable is indeterminate (at its declaration, at the entry of its block, before a switch's
  // dispatch).
  void reset(std::vector<low::LStmt>& out, const Var& v) {
    assign(out, flag_name(v), low::bool_lit(false));
    assign(out, var_name(v), dummy_value(v));
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

  void obl(Full& full, const E& guard, const Deps& gdep, E cond, const Deps& cdep, low::Meta m) {
    Obl o;
    o.guard = guard;
    o.cond = std::move(cond);
    o.meta = std::move(m);
    o.dep = join(join(gdep, cdep), floor_);  // floor_: after the left operand of a comma, what it depends on came first (E6)
    o.seq = full.next_seq++;
    full.obls.push_back(std::move(o));
  }

  // ---- expressions ----

  LV lower(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full) {
    switch (e.kind) {
      case Expr::Kind::Const:
        if (e.type.is_bool()) return {low::bool_lit(e.value == "true"), {}};
        return {low::int_lit(e.value), {}};
      case Expr::Kind::Local:
        if (!shared_.empty()) {
          const auto it = shared_.find(&e);  // the old value of `x++`: read once (decision 0020)
          if (it != shared_.end()) return it->second;
        }
        if (in_full_ && written_.count(e.var->id) != 0) return snapshot(e, guard, gdep, full);
        if (flagged(*e.var))  // a read of a local declared without an initializer: its flag must be set (decision 0019 item 4)
          obl(full, guard, gdep, low::var(flag_name(*e.var)), {},
              meta("uninitialized", e.loc, "`" + e.var->name + "` may be read here before it is given a value"));
        return {low::var(var_name(*e.var)), {}};
      case Expr::Kind::Formal: {
        const size_t k = static_cast<size_t>(e.index);
        return {env.formals[k], k < env.formal_dep.size() ? env.formal_dep[k] : Deps{}};
      }
      case Expr::Kind::Result:
        return {env.result, {}};
      case Expr::Kind::Old:
        return lower(*e.a, env, guard, gdep, full);
      case Expr::Kind::Unary: return unary(e, env, guard, gdep, full);
      case Expr::Kind::Binary: return binary(e, env, guard, gdep, full);
      case Expr::Kind::Cond: {
        if (has_effect(*e.b) || has_effect(*e.c)) return branch_cond(e, env, guard, gdep, full);
        LV c = lower(*e.a, env, guard, gdep, full);
        E gt = low::and_(guard, c.e);
        E gf = low::and_(guard, low::not_(c.e));
        const Deps gd = join(gdep, c.dep);
        LV t = lower(*e.b, env, gt, gd, full);
        LV f = lower(*e.c, env, gf, gd, full);
        return {low::ite(c.e, t.e, f.e), join(join(c.dep, t.dep), f.dep)};
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
      case Expr::Kind::AssignE: return assign_value(e, env, guard, gdep, full);
      case Expr::Kind::Comma: return comma(e, env, guard, gdep, full);
    }
    internal_error(e.loc, "unknown expression kind");
    return {low::int_lit("0"), {}};
  }

  LV unary(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full) {
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
        // Unsigned (decision 0022 item 1): -a fits iff a == 0; cerastes does not wrap.
        obl(full, guard, gdep, low::bin(low::BinOp::Eq, a.e, low::int_lit("0")), a.dep,
            meta("overflow", e.loc, "negating a nonzero `" + spell(e) + "` value overflows (cerastes does not wrap)"));
        return {low::neg(a.e), a.dep};
      }
    }
    return a;
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

  LV binary(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full) {
    using low::BinOp;
    const bool lazy_and = e.bop == BinaryOp::LAnd;
    const bool lazy_or = e.bop == BinaryOp::LOr;
    if ((lazy_and || lazy_or) && has_effect(*e.b)) return branch_logical(e, env, guard, gdep, full);
    LV a = lower(*e.a, env, guard, gdep, full);
    E gb = guard;
    Deps gbd = gdep;
    if (lazy_and) { gb = low::and_(guard, a.e); gbd = join(gdep, a.dep); }
    if (lazy_or) { gb = low::and_(guard, low::not_(a.e)); gbd = join(gdep, a.dep); }
    LV b = lower(*e.b, env, gb, gbd, full);
    const Deps dep = join(a.dep, b.dep);
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
        // Unsigned (decision 0022 item 1): the exact result must fit; cerastes does not wrap.
        obl(full, guard, gdep, range_obligation(v, t), dep,
            meta("overflow", e.loc, std::string("unsigned `") + spell(e) + "` " + op_noun(e.bop) + " may overflow (cerastes does not wrap)"));
        return {v, dep};
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
        // Unsigned (decision 0022 item 1): a >= 0 by type, so only the result must fit; cerastes does not wrap.
        obl(full, guard, gdep, low::bin(BinOp::Le, v, low::int_lit(dec::range_of(t).hi)), dep,
            meta("shift", e.loc, "the result of `<<` may not fit in `" + spell(e) + "`"));
        return {v, dep};
      }
    }
    internal_error(e.loc, "unknown binary operator");
    return a;
  }

  LV convert(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full) {
    LV a = lower(*e.a, env, guard, gdep, full);
    const IntType& s = *e.a->type.int_type;
    const IntType& t = *e.type.int_type;
    if (dec::contains(dec::range_of(t), dec::range_of(s))) return a;  // every source value is a target value
    if (t.is_enum) {
      // Into an enumeration: one of its values, whatever the underlying type's signedness; never a wrap (decision 0018
      // item 5).
      const dec::Range r = dec::range_of(t);
      obl(full, guard, gdep, range_obligation(a.e, t), a.dep,
          meta("conversion", e.loc,
               "the value may not be one of the values of `" + spell(e) + "` (" + r.lo + " to " + r.hi + ")"));
      return a;
    }
    if (t.is_signed) {
      obl(full, guard, gdep, range_obligation(a.e, t), a.dep,
          meta("conversion", e.loc,
               std::string("the value may not fit in `") + spell(e) + "` (a conversion to a signed type must be exact)"));
      return a;
    }
    // To an unsigned type (decision 0022 items 1 and 4): the value must fit; cerastes does not wrap.  An integer constant that
    // lies in the target's range needs no obligation.
    if (e.a->kind == Expr::Kind::Const && dec::in_range(e.a->value, dec::range_of(t))) return a;
    obl(full, guard, gdep, range_obligation(a.e, t), a.dep,
        meta("conversion", e.loc, std::string("the value may not fit in `") + spell(e) + "` (a conversion must be exact)"));
    return a;
  }

  // ---- calls (decisions 0012 item 6 and 0021) ----

  // The fixed points an item created from `effect_from` / `obl_from` on depends on, and those fixed points themselves: what a
  // later item must follow when it uses, or is sequenced after, everything created since (the arguments of a call, the left operand
  // of a comma, the condition of a branch).  The sets are closed downward, so the union is.
  static Deps since(const Full& full, size_t effect_from, size_t obl_from) {
    Deps r;
    for (size_t k = effect_from; k < full.effects.size(); ++k) {
      r = join(r, full.effects[k].deps);
      if (full.effects[k].fixed >= 0) r.insert(full.effects[k].fixed);
    }
    for (size_t k = obl_from; k < full.obls.size(); ++k) r = join(r, full.obls[k].dep);
    return r;
  }

  // Silicon's own decrease check (silver's DecreasesCheck.scala, `createLexDecreaseCheck`, both tuples extended with a top element) of a
  // call, from N (the callee's measure over the arguments) to M (the caller's measure over its parameters' entry values):
  //   OR_{i < min(|N|,|M|)} (AND_{l < i} N_l == M_l && N_i < M_i && M_i >= 0)   OR   [|N| > |M|] AND_{l < |M|} N_l == M_l
  // (decision 0021 item 4; for equal lengths it is 0013 item 2).  Written in the De Morgan form !(!alt1 && !alt2 ...), one atomic
  // assertion: Silicon reports each conjunct of an asserted `&&` as its own error (S19 reran its equivalence matrix with this form).
  static E lex_decreases(const std::vector<E>& n, const std::vector<E>& m) {
    using low::BinOp;
    const size_t k = std::min(n.size(), m.size());
    E none;  // the conjunction of the negated alternatives
    bool any = false;
    auto add = [&](E alt) {
      none = low::and_(none, low::not_(std::move(alt)));
      any = true;
    };
    for (size_t i = 0; i < k; ++i) {
      E alt;
      for (size_t l = 0; l < i; ++l) alt = low::and_(alt, low::bin(BinOp::Eq, n[l], m[l]));
      alt = low::and_(alt, low::bin(BinOp::Lt, n[i], m[i]));
      alt = low::and_(alt, low::bin(BinOp::Ge, m[i], low::int_lit("0")));
      add(std::move(alt));
    }
    if (n.size() > m.size()) {
      E alt;
      for (size_t l = 0; l < k; ++l) alt = low::and_(alt, low::bin(BinOp::Eq, n[l], m[l]));
      add(alt == nullptr ? low::bool_lit(true) : alt);
    }
    return any ? low::not_(none) : low::bool_lit(false);
  }

  LV call(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full) {
    const Function& callee = *e.callee;
    if (guard != nullptr) internal_error(e.loc, "a call in a conditionally evaluated operand reached lowering");
    (void)gdep;
    const size_t effect_from = full.effects.size(), obl_from = full.obls.size();
    std::vector<LV> args;
    for (const Expr* a : e.args) args.push_back(lower(*a, env, nullptr, {}, full));
    // PRE(c): what C completes before it makes the call: the comma floor, and everything its arguments use or contain (the items
    // created while they were lowered, the fixed points among them, and what the argument values depend on).
    Deps pre = join(floor_, since(full, effect_from, obl_from));
    for (const LV& a : args) pre = join(pre, a.dep);
    // The callee's `requires`, over the argument values, are obligations at the call (0012 item 6).
    Env cenv;
    for (const LV& a : args) {
      cenv.formals.push_back(a.e);
      cenv.formal_dep.push_back(a.dep);
    }
    for (const Clause& c : callee.requires_) {
      Full scratch;  // the clause's own obligations belong to the callee's well-formedness method
      LV v = lower(*c.expr, cenv, nullptr, {}, scratch);
      pure_full(scratch, *c.expr, "a precondition");
      low::Meta m = meta("precondition", e.loc, "the precondition of `" + callee.c_name + "` may not hold");
      m.has_note = true;
      m.note_loc = c.loc.known() ? c.loc : callee.loc;
      m.note_text = "this is the requirement of `" + callee.c_name + "`";
      obl(full, nullptr, {}, v.e, v.dep, std::move(m));
    }
    CallItem item;
    item.method = method_name(callee);
    for (const LV& a : args) item.args.push_back(a.e);
    low::Meta backstop = meta("internal", e.loc, "internal check: Silicon's own check of the precondition of `" + callee.c_name + "`");
    item.check = low::Check{"call.precondition", std::move(backstop)};
    const int id = full.nfixed++;
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
      item.termination = low::Check{"termination.failed", tm};
      if (in_cycle(fn_.f, &callee)) explicit_termination(full, id, pre, callee, cenv, std::move(tm));
    } else if (callee.body != nullptr && in_cycle(fn_.f, &callee)) {
      // Silicon would check the call against an empty tuple; check_program's `missing_decreases` rule makes this unreachable, and a
      // call that got neither its PRE in F nor an explicit check would be unsound, so fail closed.
      internal_error(e.loc, "a call in a recursion cycle to `" + callee.c_name + "`, which has no termination measure, reached lowering");
    }
    E value;
    if (!callee.ret.is_void()) {
      item.target = "t__" + std::to_string(fn_.ntemp++);
      fn_.temps.push_back(low::LVar{item.target, callee.ret.is_bool()});
      value = low::var(item.target);
    }
    Effect fx;
    fx.kind = Effect::Kind::Call;
    fx.call = std::move(item);
    fx.fixed = id;
    fx.deps = pre;
    fx.seq = full.next_seq++;
    full.pre.push_back(pre);
    full.fixed_effect.push_back(full.effects.size());
    full.info.push_back(FixedInfo{report_at_ != nullptr ? *report_at_ : e.loc, callee.c_name, "the call to \"" + callee.c_name + "\""});
    full.effects.push_back(std::move(fx));
    Deps d = pre;
    d.insert(id);
    return {value, d};
  }

  // Decision 0021 item 4: the explicit check that a recursive call's measure decreases, an obligation with the set PRE(c).  It is
  // printed by flush only when another fixed point, outside PRE(c), precedes the call; otherwise Silicon's own check at the call (which
  // stays) is the one that reports.
  void explicit_termination(Full& full, int id, const Deps& pre, const Function& callee, const Env& cenv, low::Meta tm) {
    std::vector<E> n, m;
    for (const Clause& c : callee.decreases_) {
      Full scratch;  // its own obligations belong to the callee's well-formedness method
      LV v = lower(*c.expr, cenv, nullptr, {}, scratch);
      pure_full(scratch, *c.expr, "a termination measure");
      n.push_back(v.e);
    }
    const Env own = contract_env();
    for (const Clause& c : fn_.f->decreases_) {
      Full scratch;
      LV v = lower(*c.expr, own, nullptr, {}, scratch);
      pure_full(scratch, *c.expr, "a termination measure");
      m.push_back(v.e);
    }
    Obl o;
    o.cond = lex_decreases(n, m);
    o.meta = std::move(tm);
    o.dep = pre;
    o.seq = full.next_seq++;
    o.term_of = id;
    full.obls.push_back(std::move(o));
  }

  // ---- effects used as values (decision 0020) ----

  static bool has_effect(const Expr& e) {
    if (e.kind == Expr::Kind::AssignE || e.kind == Expr::Kind::Comma || e.kind == Expr::Kind::Call) return true;
    for (const Expr* x : {e.a, e.b, e.c})
      if (x != nullptr && has_effect(*x)) return true;
    for (const Expr* x : e.args)
      if (x != nullptr && has_effect(*x)) return true;
    return false;
  }

  // The locals a full expression writes (its AssignE targets, those in guarded operands included).
  static void collect_writes(const Expr& e, std::set<int>& out) {
    if (e.kind == Expr::Kind::AssignE && e.var != nullptr) out.insert(e.var->id);
    for (const Expr* x : {e.a, e.b, e.c})
      if (x != nullptr) collect_writes(*x, out);
    for (const Expr* x : e.args)
      if (x != nullptr) collect_writes(*x, out);
  }

  std::string new_temp(bool is_bool) {
    const std::string t = "t__" + std::to_string(fn_.ntemp++);
    fn_.temps.push_back(low::LVar{t, is_bool});
    return t;
  }

  // E3: a read of a local that the full expression writes is a snapshot, taken where C reads it; its `uninitialized` obligation
  // (decision 0019) takes the snapshot's set, so the flag is read where the value is.
  LV snapshot(const Expr& e, const E& guard, const Deps& gdep, Full& full) {
    const std::string t = new_temp(e.var->type.is_bool());
    const int id = e.var->id;
    // A read after a write of the same local follows that write's set; the comma floor too.
    const auto w = full.last_write.find(id);
    const Deps slot = join(floor_, w == full.last_write.end() ? Deps{} : w->second);
    Effect fx;
    fx.kind = Effect::Kind::Snapshot;
    fx.name = t;
    fx.value = low::var(var_name(*e.var));
    fx.deps = slot;
    fx.seq = full.next_seq++;
    fx.reads.insert(id);
    full.effects.push_back(std::move(fx));
    touch(full, id, slot);
    if (flagged(*e.var))
      obl(full, guard, gdep, low::var(flag_name(*e.var)), slot,
          meta("uninitialized", e.loc, "`" + e.var->name + "` may be read here before it is given a value"));
    return {low::var(t), slot};
  }

  // `x = a` used as a value: the old value (for x++) is read once, then the stored value is lowered, then the write is an effect.
  // E4: the node's value is the stored value as lowered (or the old value's snapshot), never `v_x`.
  // Record an access (a snapshot or a write) of a local with the set `slot`.
  static void touch(Full& full, int id, const Deps& slot) {
    const auto it = full.last_access.find(id);
    if (it == full.last_access.end()) full.last_access[id] = slot;
    else it->second = join(it->second, slot);
  }

  LV assign_value(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full) {
    if (!in_full_ || e.var == nullptr || written_.count(e.var->id) == 0) {
      internal_error(e.loc, "an assignment used as a value outside a full expression reached lowering");
      return {low::int_lit("0"), {}};
    }
    if (guard != nullptr) {
      internal_error(e.loc, "an effect in a conditionally evaluated pure operand reached lowering");
      return {low::int_lit("0"), {}};
    }
    LV old_value;
    if (e.b != nullptr) {
      old_value = lower(*e.b, env, guard, gdep, full);
      shared_[e.b] = old_value;
    }
    LV stored = lower(*e.a, env, guard, gdep, full);
    if (e.b != nullptr) shared_.erase(e.b);
    // The write follows what its value depends on, the comma floor and every earlier access to the same local.
    const int id = e.var->id;
    const auto a = full.last_access.find(id);
    const Deps slot = join(join(floor_, stored.dep), a == full.last_access.end() ? Deps{} : a->second);
    Effect fx;
    fx.kind = Effect::Kind::Write;
    fx.name = var_name(*e.var);
    fx.value = stored.e;
    if (flagged(*e.var)) fx.flag = flag_name(*e.var);
    fx.deps = slot;
    fx.seq = full.next_seq++;
    fx.writes.insert(id);
    full.effects.push_back(std::move(fx));
    full.last_write[id] = slot;
    touch(full, id, slot);
    if (e.yields_old) return old_value;
    return stored;
  }

  // `a, b`: `a` for its effects and obligations, then `b`; whatever follows the left operand is placed after its last effect (E6).
  LV comma(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full) {
    if (!in_full_) {
      internal_error(e.loc, "a comma outside a full expression reached lowering");
      return {low::int_lit("0"), {}};
    }
    const size_t effect_from = full.effects.size();
    const size_t obl_from = full.obls.size();
    lower(*e.a, env, guard, gdep, full);
    const Deps saved = floor_;
    // Only what the left operand itself depends on or contains: a call elsewhere in the enclosing expression is unsequenced with the
    // right operand (decision 0021: what the left operand created, fixed points included).
    floor_ = join(floor_, since(full, effect_from, obl_from));
    LV b = lower(*e.b, env, guard, gdep, full);
    floor_ = saved;
    return b;
  }

  // The operand `e` lowered into the statements of one branch of an `if`: its own Full, flushed inside the branch, ending with
  // `t := value`.  Its obligations are asserted inside the branch, so C's guard is the branch itself.  `first_call` is set to the
  // description of its first fixed point when it has any (decision 0021: such a branch is a fixed point of the enclosing expression).
  std::vector<low::LStmt> branch_body(const Expr& e, const Env& env, const std::string& t, std::set<int>& reads, std::set<int>& writes,
                                      FixedInfo& first_call, bool& has_fixed) {
    const Deps saved = floor_;
    floor_ = {};
    Full inner;
    LV v = lower(e, env, nullptr, {}, inner);
    floor_ = saved;
    if (inner.nfixed != 0 && !has_fixed) {
      has_fixed = true;
      first_call = inner.info.front();
    }
    for (const Effect& fx : inner.effects) {
      reads.insert(fx.reads.begin(), fx.reads.end());
      writes.insert(fx.writes.begin(), fx.writes.end());
    }
    std::vector<low::LStmt> body;
    flush(inner, body);
    assign(body, t, v.e);
    return body;
  }

  // The branch as an effect: after what its condition depends on, the comma floor, every earlier write of the locals it reads and every
  // earlier access to the locals it writes (its inner effects stay in the order they were lowered in).  A branch that holds a call is a
  // fixed point (decision 0021 item 3): it also follows everything created while its condition operand was lowered (`effect_from`,
  // `obl_from`), and its value has D(B) = PRE(B) + B.  A call-free branch is a movable item with the set of 0020.
  Deps add_branch(Full& full, Effect&& fx, const Deps& cond_dep, const std::set<int>& reads, const std::set<int>& writes, bool has_fixed,
                  FixedInfo info, size_t effect_from, size_t obl_from) {
    Deps slot = join(floor_, cond_dep);
    for (int id : reads) {
      const auto w = full.last_write.find(id);
      if (w != full.last_write.end()) slot = join(slot, w->second);
    }
    for (int id : writes) {
      const auto a = full.last_access.find(id);
      if (a != full.last_access.end()) slot = join(slot, a->second);
    }
    Deps value = slot;
    fx.deps = slot;
    fx.seq = full.next_seq++;
    fx.reads = reads;
    fx.writes = writes;
    if (has_fixed) {
      slot = join(slot, since(full, effect_from, obl_from));
      fx.deps = slot;
      const int id = full.nfixed++;
      fx.fixed = id;
      full.pre.push_back(slot);
      full.fixed_effect.push_back(full.effects.size());
      full.info.push_back(std::move(info));
      value = slot;
      value.insert(id);
    }
    for (int id : writes) full.last_write[id] = value;
    for (int id : reads) touch(full, id, value);
    for (int id : writes) touch(full, id, value);
    full.effects.push_back(std::move(fx));
    return value;
  }

  // `a && b` / `a || b` where `b` has effects (E5): `t := a; if (t) { b's effects and obligations; t := b }` (`!t` for ||).
  LV branch_logical(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full) {
    if (guard != nullptr) {
      internal_error(e.loc, "an effect in a conditionally evaluated operand reached lowering");
      return {low::bool_lit(false), {}};
    }
    if (!in_full_) {
      internal_error(e.loc, "an effect in a logical operand outside a full expression reached lowering");
      return {low::bool_lit(false), {}};
    }
    const size_t effect_from = full.effects.size();
    const size_t obl_from = full.obls.size();
    LV a = lower(*e.a, env, guard, gdep, full);
    const std::string t = new_temp(true);
    Effect fx;
    fx.kind = Effect::Kind::Branch;
    assign(fx.stmts, t, a.e);
    low::LStmt i;
    i.kind = low::LStmt::Kind::If;
    i.expr = e.bop == BinaryOp::LAnd ? low::var(t) : low::not_(low::var(t));
    std::set<int> reads, writes;
    FixedInfo first;
    bool has_fixed = false;
    i.then_body = branch_body(*e.b, env, t, reads, writes, first, has_fixed);
    fx.stmts.push_back(std::move(i));
    if (has_fixed) first.what += e.bop == BinaryOp::LAnd ? " in the right operand of `&&`" : " in the right operand of `||`";
    const Deps value = add_branch(full, std::move(fx), a.dep, reads, writes, has_fixed, first, effect_from, obl_from);
    return {low::var(t), value};
  }

  // `c ? a : b` where an arm has effects (E5): `if (c) { a...; t := a } else { b...; t := b }`.
  LV branch_cond(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full) {
    if (guard != nullptr) {
      internal_error(e.loc, "an effect in a conditionally evaluated operand reached lowering");
      return {low::int_lit("0"), {}};
    }
    if (!in_full_) {
      internal_error(e.loc, "an effect in a conditional operand outside a full expression reached lowering");
      return {low::int_lit("0"), {}};
    }
    const size_t effect_from = full.effects.size();
    const size_t obl_from = full.obls.size();
    LV c = lower(*e.a, env, guard, gdep, full);
    const std::string t = new_temp(e.type.is_bool());
    Effect fx;
    fx.kind = Effect::Kind::Branch;
    low::LStmt i;
    i.kind = low::LStmt::Kind::If;
    i.expr = c.e;
    std::set<int> reads, writes;
    FixedInfo first;
    bool has_fixed = false;
    i.then_body = branch_body(*e.b, env, t, reads, writes, first, has_fixed);
    i.else_body = branch_body(*e.c, env, t, reads, writes, first, has_fixed);
    fx.stmts.push_back(std::move(i));
    if (has_fixed) first.what += " in an arm of `?:`";
    const Deps value = add_branch(full, std::move(fx), c.dep, reads, writes, has_fixed, first, effect_from, obl_from);
    return {low::var(t), value};
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

  // What the items that have exactly the set `set` are: `term_call` is the recursive call whose explicit termination check has it as PRE
  // (-1 if none), `ordinary` whether any other item (an obligation, a write, a snapshot, a branch) has it.
  struct SetOrigin {
    bool ordinary = false;
    int term_call = -1;
  };
  static SetOrigin origin_of(const Full& full, const Deps& set) {
    SetOrigin o;
    for (const Obl& x : full.obls) {
      if (x.dep != set) continue;
      if (x.term_of >= 0) { if (o.term_call < 0) o.term_call = x.term_of; }
      else o.ordinary = true;
    }
    for (const Effect& fx : full.effects) {
      if (fx.fixed < 0) {
        if (fx.deps == set) o.ordinary = true;
      } else if (fx.kind == Effect::Kind::Branch) {
        Deps d = fx.deps;
        d.insert(fx.fixed);
        if (d == set) o.ordinary = true;
      }
    }
    return o;
  }

  // The refusal of a full expression whose sets do not form a chain (decision 0021 item 3): `a` and `b` are two neighbours of the sorted
  // family, `a` not a subset of `b`.  One fixed point of each side witnesses it; the report is at the later-created one.  A side whose set
  // only a recursive call's termination check has is worded by that call (nothing visible depends on the witness alone): `the call to "h"
  // and the recursive call to "f" (its termination check needs "g" first) are not ordered`.
  void report_conflict(const Full& full, const Deps& a, const Deps& b) {
    int p = -1, q = -1;
    for (int x : a)
      if (b.count(x) == 0) { p = x; break; }
    for (int x : b)
      if (a.count(x) == 0) { q = x; break; }
    if (p < 0 || q < 0) {
      internal_error(SrcLoc{}, "the sets of a full expression are not a chain, but no witness was found");
      return;
    }
    const SetOrigin oa = origin_of(full, a), ob = origin_of(full, b);
    const bool ta = !oa.ordinary && oa.term_call >= 0, tb = !ob.ordinary && ob.term_call >= 0;
    // The fixed point each side is reported as: the recursive call itself for a termination side, else the witness.
    const int ra = ta ? oa.term_call : p, rb = tb ? ob.term_call : q;
    auto info = [&full](int id) -> const FixedInfo& { return full.info[static_cast<size_t>(id)]; };
    auto quoted = [&info](int id) { return info(id).callee.empty() ? info(id).what : "\"" + info(id).callee + "\""; };
    auto recursive = [&](int call, int witness) {
      return "the recursive call to \"" + info(call).callee + "\" (its termination check needs " + quoted(witness) + " first)";
    };
    const int lo = std::min(ra, rb), hi = std::max(ra, rb);
    const FixedInfo& later = info(hi);
    std::string why;
    if (!ta && !tb) {
      const FixedInfo& first = info(std::min(p, q));
      const FixedInfo& second = info(std::max(p, q));
      if (!first.callee.empty() && !second.callee.empty())
        why = "the calls to \"" + first.callee + "\" and \"" + second.callee + "\" are not ordered";
      else
        why = first.what + " and " + second.what + " are not ordered";
      rejections_.push_back(Diag{second.loc, std::move(why)});
      return;
    }
    auto text = [&](int side_id, bool term, int call, int witness) { return term ? recursive(call, witness) : info(side_id).what; };
    const std::string left = text(ra, ta, oa.term_call, p), right = text(rb, tb, ob.term_call, q);
    if (ta != tb) why = (ta ? right + " and " + left : left + " and " + right) + " are not ordered";  // the plain call first
    else why = (lo == ra ? left + " and " + right : right + " and " + left) + " are not ordered";
    rejections_.push_back(Diag{later.loc, std::move(why)});
  }

  // Write a full expression (decision 0021 item 3; 0012 item 6, 0020 item 7 for the shape of the items).  The family F of its sets
  // (every obligation's, every movable effect's, D(B) of every branch that holds a call; the sets of the explicit termination checks
  // are among the obligations') must form a chain under inclusion; the fixed points are then ordered pi: those of the smallest set,
  // then those each larger set adds, then the rest, each group in creation order.  Every set is a prefix of pi, and an item is emitted
  // right after the |S|-th fixed point of its set S and before every other: a callee that does not return hides only what C cannot
  // evaluate before it returns.  An item with an empty set comes first.  A fixed point is a hoisted call, or a branch (an `if`
  // for a guarded operand with effects).  A movable effect is `v := value` (and its flag) for a write, a snapshot of a local that the
  // expression writes, or an `if` for a call-free guarded operand.  Where the sets form no chain the program is refused
  // (`rejections`) and the rest is written in creation order, so that lowering finishes.
  void flush(Full& full, std::vector<low::LStmt>& out) {
    const int n = full.nfixed;
    std::vector<Deps> sets;
    for (const Obl& o : full.obls) sets.push_back(o.dep);
    for (const Effect& fx : full.effects) {
      if (fx.fixed < 0) {
        sets.push_back(fx.deps);
      } else if (fx.kind == Effect::Kind::Branch) {
        Deps d = fx.deps;
        d.insert(fx.fixed);
        sets.push_back(std::move(d));
      }
    }
    std::sort(sets.begin(), sets.end(), [](const Deps& a, const Deps& b) { return a.size() != b.size() ? a.size() < b.size() : a < b; });
    sets.erase(std::unique(sets.begin(), sets.end()), sets.end());
    bool chain = true;
    for (size_t i = 0; i + 1 < sets.size(); ++i) {
      if (std::includes(sets[i + 1].begin(), sets[i + 1].end(), sets[i].begin(), sets[i].end())) continue;
      report_conflict(full, sets[i], sets[i + 1]);
      chain = false;
      break;
    }
    // pi: the fixed points in the order they are emitted.
    std::vector<int> pi;
    std::vector<char> placed(static_cast<size_t>(n), 0);
    if (chain) {
      for (const Deps& s : sets)
        for (int id : s)
          if (placed[static_cast<size_t>(id)] == 0) { placed[static_cast<size_t>(id)] = 1; pi.push_back(id); }
    }
    for (int id = 0; id < n; ++id)
      if (placed[static_cast<size_t>(id)] == 0) pi.push_back(id);
    if (opt_.perturb_fixed_order && chain && pi.size() >= 2) std::swap(pi[0], pi[1]);  // self-test hook (0011 item 13)
    std::vector<int> pos(static_cast<size_t>(n), 0);  // the index of each fixed point in pi
    for (size_t k = 0; k < pi.size(); ++k) pos[static_cast<size_t>(pi[k])] = static_cast<int>(k);
    // The slot of an item: right after pi[|S| - 1] (-1: before every fixed point).
    auto slot_of = [n](const Deps& s) { return std::min(static_cast<int>(s.size()) - 1, n - 1); };
    if (chain) {
      check_variable_order(full, pos);
      check_fixed_order(full, pi, pos);
    }
    for (int slot = -1; slot < n; ++slot) {
      // The items of this slot, obligations and movable effects, in creation order.
      std::vector<std::pair<size_t, std::pair<bool, size_t>>> items;  // (seq, (is obligation, index))
      for (size_t k = 0; k < full.obls.size(); ++k)
        if (slot_of(full.obls[k].dep) == slot) items.push_back({full.obls[k].seq, {true, k}});
      for (size_t k = 0; k < full.effects.size(); ++k)
        if (full.effects[k].fixed < 0 && slot_of(full.effects[k].deps) == slot) items.push_back({full.effects[k].seq, {false, k}});
      std::sort(items.begin(), items.end());
      for (const auto& it : items) {
        if (it.second.first) {
          Obl& o = full.obls[it.second.second];
          // The explicit termination check of a call (decision 0021 item 4) is written only when another fixed point, outside its set,
          // comes before the call: it then asserts what Silicon's own check at the call could not reach.  Not written: nothing is
          // counted (the self-test hook counts asserts) and no temporary is made.
          if (o.term_of >= 0 && pos[static_cast<size_t>(o.term_of)] <= static_cast<int>(o.dep.size())) continue;
          emit_assert(out, low::implies(o.guard, o.cond), o.meta);
          continue;
        }
        Effect& fx = full.effects[it.second.second];
        switch (fx.kind) {
          case Effect::Kind::Call: break;
          case Effect::Kind::Write:
            assign(out, fx.name, fx.value);
            if (!fx.flag.empty()) assign(out, fx.flag, low::bool_lit(true));
            break;
          case Effect::Kind::Snapshot: assign(out, fx.name, fx.value); break;
          case Effect::Kind::Branch:
            for (low::LStmt& st : fx.stmts) out.push_back(std::move(st));
            break;
        }
      }
      if (slot + 1 < n) {
        Effect& fx = full.effects[full.fixed_effect[static_cast<size_t>(pi[static_cast<size_t>(slot + 1)])]];
        if (fx.kind == Effect::Kind::Branch) {
          for (low::LStmt& st : fx.stmts) out.push_back(std::move(st));
          continue;
        }
        CallItem& c = fx.call;
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

  // The fail-closed check of the main property of decision 0021 (like check_variable_order): the fixed points emitted before an item are
  // exactly its set; every fixed point follows its PRE; and a branch with a call, and a recursive call whose explicit termination check
  // is not written, come right after their PRE.  A violation would be a wrong Viper program (a check hidden by a call that does not
  // return), so it is an internal error.
  void check_fixed_order(const Full& full, const std::vector<int>& pi, const std::vector<int>& pos) {
    auto prefix_is = [&pi](const Deps& s) {
      if (s.size() > pi.size()) return false;
      for (size_t k = 0; k < s.size(); ++k)
        if (s.count(pi[k]) == 0) return false;
      return true;
    };
    constexpr const char* kSet = "the fixed points emitted before an item of a full expression are not exactly the set it depends on";
    for (const Obl& o : full.obls)
      if (!prefix_is(o.dep)) { internal_error(SrcLoc{}, kSet); return; }
    for (const Effect& fx : full.effects)
      if (fx.fixed < 0 && !prefix_is(fx.deps)) { internal_error(SrcLoc{}, kSet); return; }
    for (int id = 0; id < full.nfixed; ++id) {
      const Deps& pre = full.pre[static_cast<size_t>(id)];
      for (int p : pre)
        if (pos[static_cast<size_t>(p)] >= pos[static_cast<size_t>(id)]) {
          internal_error(SrcLoc{}, "a fixed point of a full expression is emitted before something it follows");
          return;
        }
      const Effect& fx = full.effects[full.fixed_effect[static_cast<size_t>(id)]];
      if (fx.kind == Effect::Kind::Branch && !(pos[static_cast<size_t>(id)] == static_cast<int>(pre.size()) && prefix_is(pre))) {
        internal_error(SrcLoc{}, "a branch with a call is not emitted right after what its condition depends on");
        return;
      }
    }
    for (const Obl& o : full.obls)  // a recursive call without a written explicit check: right after its PRE
      if (o.term_of >= 0 && pos[static_cast<size_t>(o.term_of)] == static_cast<int>(o.dep.size()) && !prefix_is(o.dep)) {
        internal_error(SrcLoc{}, "a recursive call is emitted without its explicit termination check and not right after what it depends on");
        return;
      }
  }

  // The fail-closed check of the order: for every local, its snapshots and writes (a branch counts as the accesses of its inner
  // effects) are emitted in the order they were created, except that two reads may swap.  A hoisted write that overtook an earlier
  // read or write of the same local would change a value, so it is an internal error rather than a wrong Viper program.  The key of an
  // item is its place in the emitted order: a movable item with the set S comes between pi[|S| - 1] (key 2|S| - 1) and pi[|S|]
  // (key 2|S| + 1), so its key is 2|S|; the fixed point at index i has the key 2i + 1.
  void check_variable_order(const Full& full, const std::vector<int>& pos) {
    struct Access {
      int id;
      bool write;
      int key;
      size_t seq;
    };
    std::vector<Access> all;  // in creation order
    bool perturbed = false;
    for (const Effect& fx : full.effects) {
      if (fx.reads.empty() && fx.writes.empty()) continue;
      int key = fx.fixed >= 0 ? 2 * pos[static_cast<size_t>(fx.fixed)] + 1 : 2 * static_cast<int>(fx.deps.size());
      if (opt_.perturb_access_order && !perturbed) {  // self-test hook (0011 item 13): the first access moves behind everything
        key += 1000;
        perturbed = true;
      }
      for (int id : fx.reads) all.push_back({id, false, key, fx.seq});
      for (int id : fx.writes) all.push_back({id, true, key, fx.seq});
    }
    std::sort(all.begin(), all.end(), [](const Access& x, const Access& y) { return x.seq < y.seq; });
    for (size_t i = 0; i < all.size(); ++i)
      for (size_t j = i + 1; j < all.size(); ++j) {
        if (all[i].id != all[j].id || (!all[i].write && !all[j].write)) continue;
        // emitted order is (key, seq): the earlier-created access must not come later
        if (all[i].key > all[j].key)
          internal_error(SrcLoc{}, "an access to a local was emitted before an earlier access to it in the same full expression");
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
    const std::set<int> saved_written = std::move(written_);
    const bool saved_in_full = in_full_;
    written_.clear();
    collect_writes(e, written_);
    in_full_ = true;
    LV v = lower(e, body_env(), nullptr, {}, full);
    in_full_ = saved_in_full;
    written_ = saved_written;
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
        if (s.expr == nullptr) {  // a Decl without an initializer (check_program allows no other)
          reset(out, *s.var);
          break;
        }
        LV v = full_expr(*s.expr, out);
        assign(out, var_name(*s.var), v.e);
        if (flagged(*s.var)) assign(out, flag_name(*s.var), low::bool_lit(true));
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
        // A goto past a declaration lands in this block with the variable in scope: it starts indeterminate at the block's
        // entry (decision 0019 item 4).
        for (const Stmt* c : s.stmts)
          if (c->kind == Stmt::Kind::Decl && c->expr == nullptr) reset(out, *c->var);
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
    // A case label may jump past a declaration of the body without an initializer: reset them before the dispatch (0019 item 4).
    for (const Stmt* c : s.body->stmts)
      if (c->kind == Stmt::Kind::Decl && c->expr == nullptr) reset(out, *c->var);
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
      LV v = lower(*c.expr, env, nullptr, {}, scratch);
      pure_full(scratch, *c.expr, "a postcondition");
      low::Meta m = meta("postcondition", at, "the postcondition of `" + fn_.f->c_name + "` may not hold");
      m.has_note = true;
      m.note_loc = c.loc;
      m.note_text = "this is the postcondition";
      emit_assert(out, v.e, std::move(m));
    }
  }

  // The locals an expression assigns (the targets of its AssignE nodes).
  static void scan_expr(const Expr* e, std::set<int>& assigned) {
    if (e == nullptr) return;
    if (e->kind == Expr::Kind::AssignE && e->var != nullptr) assigned.insert(e->var->id);
    for (const Expr* x : {e->a, e->b, e->c}) scan_expr(x, assigned);
    for (const Expr* x : e->args) scan_expr(x, assigned);
  }

  // Locals that a loop assigns but does not declare: the ones whose values cross iterations.  Every expression counts, a nested
  // loop's condition and a `do` condition included (decision 0020).
  static void scan(const Stmt& s, std::set<int>& assigned, std::set<int>& declared) {
    switch (s.kind) {
      case Stmt::Kind::Decl:
        declared.insert(s.var->id);
        scan_expr(s.expr, assigned);
        break;
      case Stmt::Kind::Assign:
        assigned.insert(s.var->id);
        scan_expr(s.expr, assigned);
        break;
      case Stmt::Kind::ExprStmt: case Stmt::Kind::Return: case Stmt::Kind::Check: scan_expr(s.expr, assigned); break;
      case Stmt::Kind::If:
        scan_expr(s.expr, assigned);
        if (s.then_body != nullptr) scan(*s.then_body, assigned, declared);
        if (s.else_body != nullptr) scan(*s.else_body, assigned, declared);
        break;
      case Stmt::Kind::While:
        scan_expr(s.expr, assigned);
        if (s.body != nullptr) scan(*s.body, assigned, declared);
        if (s.step != nullptr) scan(*s.step, assigned, declared);
        break;
      case Stmt::Kind::Switch:
        scan_expr(s.expr, assigned);
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
    scan_expr(s.expr, assigned);  // the loop's own condition (a side effect in it assigns, decision 0020)
    // A condition with a side effect is evaluated inside the body, at the top (the `do` shape of 0014 item 5 moved up): the invariants
    // and the measure stay where C evaluates the condition, and its obligations are asserted at every evaluation.
    const bool top_test = !is_do && has_effect(*s.expr);
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

    // Decision 0019 item 4: Viper havocs the flag of a variable the loop assigns and that is declared outside it.  A snapshot of
    // the flag before the loop and the invariant `snapshot ==> flag` keep a value given before the loop known inside it.
    const int loop_number = fn_.nloop++;
    for (int id : assigned) {
      if (declared.count(id) != 0 || fn_.flagged.count(id) == 0) continue;
      const Var& v = *fn_.f->locals()[static_cast<size_t>(id)];
      const std::string snap = "ds_" + var_name(v) + "__" + std::to_string(loop_number);
      fn_.snapshots.push_back(low::LVar{snap, true});
      assign(out, snap, low::var(flag_name(v)));
      low::LInvariant inv;
      inv.expr = low::implies(low::var(snap), low::var(flag_name(v)));
      for (const char* id2 : {"invariant.not.established", "invariant.not.preserved"}) {
        low::Meta m;
        m.category = "internal";
        m.loc = s.loc;
        m.message = "internal check: the loop keeps `" + v.name + "` given a value";
        inv.checks.push_back(low::Check{id2, std::move(m)});
      }
      if (!opt_.drop_snapshot_invariant) w.invariants.push_back(std::move(inv));
    }

    E prior;  // the conjunction of the user invariants so far
    for (const Clause& c : s.invariants) {
      Full full;
      LV v = lower(*c.expr, body_env(), nullptr, {}, full);
      pure_full(full, *c.expr, "a loop invariant");
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
      LV mv = lower(*c.expr, body_env(), nullptr, {}, mfull);
      pure_full(mfull, *c.expr, "a loop measure");
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
    if (top_test) {
      w.expr = low::bool_lit(true);
      LV cv = full_expr(*s.expr, w.then_body);
      low::LStmt exit;
      exit.kind = low::LStmt::Kind::If;
      exit.expr = low::not_(cv.e);
      jump(exit.then_body, "l_brk__" + n);
      w.then_body.push_back(std::move(exit));
    } else if (!is_do) {
      Full cfull;
      LV cv = lower(*s.expr, body_env(), nullptr, {}, cfull);
      pure_full(cfull, *s.expr, "a loop condition");
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
    if (is_do || top_test || breaks.count(s.jump_id) != 0) label(out, "l_brk__" + n);
  }

  // `do { B } while (c)` where `c` is a constant zero (decision 0014 item 5): the body runs once and no iteration follows
  // another, so there is no loop and no measure.  A written invariant is asserted once, at entry (the start of the only
  // iteration), a written measure only has its obligations checked there; `continue` goes to the condition, which is evaluated
  // (for its obligations) after the body, and `break` to the end.
  void do_once(const Stmt& s, std::vector<low::LStmt>& out) {
    E prior;
    for (const Clause& c : s.invariants) {
      Full full;
      LV v = lower(*c.expr, body_env(), nullptr, {}, full);
      pure_full(full, *c.expr, "a loop invariant");
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
      lower(*c.expr, body_env(), nullptr, {}, mfull);
      pure_full(mfull, *c.expr, "a loop measure");
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
      case Expr::Kind::AssignE: case Expr::Kind::Comma:
        internal_error(e.loc, std::string("a call, variable, parameter, result, `old` or side effect in ") + what + " reached the prologue");
        return false;
      default: break;
    }
    for (const Expr* x : {e.a, e.b, e.c})
      if (x != nullptr && !closed_expr(*x, what)) return false;
    return true;
  }

  // A Full that is lowered only for its obligations (a contract, an invariant, a measure, a pure loop condition): it must hold no effect,
  // or the effect would be dropped.
  void pure_full(const Full& full, const Expr& e, const char* what) {
    if (!full.effects.empty())
      internal_error(e.loc, std::string("a call or side effect in ") + what + " reached lowering outside a full expression");
  }

  bool closed_result(const Full& full, const Expr& e, const char* what) {
    if (full.effects.empty()) return true;
    internal_error(e.loc, std::string("a call in ") + what + " reached the prologue");
    return false;
  }

  void named_const_check(const NamedConst& c, std::vector<low::LStmt>& out) {
    if (c.init == nullptr) return;
    if (!closed_expr(*c.init, "the initializer of a named constant")) return;
    Full full;
    LV v = lower(*c.init, body_env(), nullptr, {}, full);
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
    LV v = lower(*c.expr, body_env(), nullptr, {}, full);
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
    if (f.body != nullptr) find_flagged(*f.body, fn_.flagged);

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
      LV v = lower(*c.expr, cenv, nullptr, {}, scratch);
      pure_full(scratch, *c.expr, "a precondition");
      m.requires_.push_back(clause_checks(v.e, c.loc.known() ? c.loc : f.loc, false, f.c_name));
    }
    if (f.ret.is_int())
      m.ensures_.push_back(clause_checks(range_check(low::var(kResult), *f.ret.int_type), f.loc, true, f.c_name));
    for (const Clause& c : f.ensures_) {
      Full scratch;
      LV v = lower(*c.expr, cenv, nullptr, {}, scratch);
      pure_full(scratch, *c.expr, "a postcondition");
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
      LV v = lower(*c.expr, cenv, nullptr, {}, scratch);
      pure_full(scratch, *c.expr, "a termination measure");
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
    for (int id : fn_.flagged) m.locals.push_back(low::LVar{flag_name(*f.locals()[static_cast<size_t>(id)]), true});
    for (const low::LVar& t : fn_.snapshots) m.locals.push_back(t);
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
      LV v = lower(*c.expr, env, nullptr, {}, full);
      pure_full(full, *c.expr, "a contract clause");
      for (Obl& o : full.obls) emit_assert(m.body, low::implies(low::and_(prior, o.guard), o.cond), o.meta);
      prior = low::and_(prior, v.e);
    };
    for (const Clause& c : f.requires_) go(c);
    E requires_only = prior;
    for (const Clause& c : f.ensures_) go(c);
    // The measure is evaluated over the entry values: its obligations are checked under the requires (0013 item 7).
    for (const Clause& c : f.decreases_) {
      Full full;
      LV v = lower(*c.expr, env, nullptr, {}, full);
      pure_full(full, *c.expr, "a termination measure");
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
  std::vector<Diag> rejections_;
  std::map<const Function*, int> component_;  // the recursion component of each function (compute_cycles)
  const SrcLoc* report_at_ = nullptr;
  const std::string* report_note_ = nullptr;
  int assert_count_ = 0;
  // The full expression being lowered (decision 0020): the locals it writes (every read of one is a snapshot), the comma floor (E6),
  // and the old-value nodes of `x++` whose read is shared.
  bool in_full_ = false;
  std::set<int> written_;
  Deps floor_;
  std::map<const Expr*, LV> shared_;
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
