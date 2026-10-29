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
//    call) of its set and before every other fixed point.  The sets of the obligations and of the branches (the family
//    F) must form a chain; if they do not, the program is rejected (`LowerResult::rejections`).  A write or a snapshot
//    is not in F (decision 0025 item 2): it goes right after the last member of its set in the order of the fixed points.
//  * Every set carries two halves (decision 0026 part A; `Deps`): the EXACT one, what C completes before the item (a
//    `requires` clause has PRE of its call, the value of a comma carries its floor), and the BASE one, the smaller sets
//    of decision 0021 item 10 as M4.4c had them.  Each full expression is flushed in two steps: if its family F over
//    the exact halves is a chain, those are its sets; otherwise, if F over the base halves is a chain, those are (and the
//    placement is what M4.4c emitted); otherwise it is rejected, with the base family's conflict.  `flush` projects the
//    full expression onto the half it chooses (`FlatFull`), so the placement and the fail-closed checks run on plain sets.
//  * Statements are lowered by `Fn::stmt()`.
#include "lower.h"

#include <algorithm>
#include <deque>
#include <functional>
#include <map>
#include <set>
#include <tuple>

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
// The Viper type of a VIR type: a pointer is a Ref (decision 0032 item 5).
low::LType ltype(const Type& t) { return t.is_bool() ? low::LType::Bool : (t.is_ptr() ? low::LType::Ref : low::LType::Int); }
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
// Contracts with pointers (decision 0037): structural helpers over lowered expressions
// ---------------------------------------------------------------------------------------------------------------

// Structural equality of two lowered expressions (a null is "true", equal only to a null).
bool same_expr(const E& a, const E& b) {
  if (a == b) return true;
  if (!a || !b) return false;
  if (a->kind != b->kind || a->text != b->text || a->bool_value != b->bool_value || a->un != b->un || a->bin != b->bin) return false;
  if (!same_expr(a->a, b->a) || !same_expr(a->b, b->b) || !same_expr(a->c, b->c)) return false;
  if (a->args.size() != b->args.size()) return false;
  for (size_t i = 0; i < a->args.size(); ++i)
    if (!same_expr(a->args[i], b->args[i])) return false;
  return true;
}

// The conjuncts of `e` (left-associated `&&` flattened); empty for a null (true).
void conjuncts(const E& e, std::vector<E>& out) {
  if (!e) return;
  if (e->kind == low::LExpr::Kind::Bin && e->bin == low::BinOp::And) {
    conjuncts(e->a, out);
    conjuncts(e->b, out);
    return;
  }
  out.push_back(e);
}

// Whether every conjunct of `d` is a conjunct of `guard`: `guard ==> d` structurally.
bool implied_by(const E& d, const E& guard) {
  std::vector<E> dc, gc;
  conjuncts(d, dc);
  conjuncts(guard, gc);
  for (const E& x : dc) {
    bool found = false;
    for (const E& y : gc)
      if (same_expr(x, y)) { found = true; break; }
    if (!found) return false;
  }
  return true;
}

// Whether a lowered Bool expression is always true by its shape (an atom of a value clause lowers to `true`): a literal, a conjunction of two,
// a disjunction with one, a conditional with both arms.
bool always_true(const E& e) {
  if (!e) return true;
  switch (e->kind) {
    case low::LExpr::Kind::BoolLit: return e->bool_value;
    case low::LExpr::Kind::Bin:
      if (e->bin == low::BinOp::And) return always_true(e->a) && always_true(e->b);
      if (e->bin == low::BinOp::Or) return always_true(e->a) || always_true(e->b);
      if (e->bin == low::BinOp::Implies) return always_true(e->b);
      return false;
    case low::LExpr::Kind::Ite: return always_true(e->b) && always_true(e->c);
    default: return false;
  }
}

bool has_atom(const Expr& e) {
  if (e.kind == Expr::Kind::Atom) return true;
  for (const Expr* x : {e.a, e.b, e.c})
    if (x != nullptr && has_atom(*x)) return true;
  for (const Expr* x : e.args)
    if (x != nullptr && has_atom(*x)) return true;
  return false;
}

E min_perm(const E& a, const E& b) { return low::ite(low::bin(low::BinOp::Lt, a, b), a, b); }

// ---------------------------------------------------------------------------------------------------------------
// Constant masks (decision 0026 part B)
// ---------------------------------------------------------------------------------------------------------------

// Exact integers for the evaluation of a lowered constant operand: a sign and a magnitude of decimal digits.  Values reach 2^128 and
// beyond (2^N - 1 for N = 128, and products of such values), so no host integer is used.
struct Z {
  bool neg = false;
  std::string mag = "0";  // digits without leading zeros
};

int mag_cmp(const std::string& a, const std::string& b) {
  if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
  return a < b ? -1 : (a > b ? 1 : 0);
}
std::string mag_add(const std::string& a, const std::string& b) {
  std::string r;
  int carry = 0;
  for (size_t i = 0; i < std::max(a.size(), b.size()) || carry != 0; ++i) {
    const int x = i < a.size() ? a[a.size() - 1 - i] - '0' : 0;
    const int y = i < b.size() ? b[b.size() - 1 - i] - '0' : 0;
    r += static_cast<char>('0' + (x + y + carry) % 10);
    carry = (x + y + carry) / 10;
  }
  std::reverse(r.begin(), r.end());
  return r.empty() ? "0" : r;
}
std::string mag_sub(const std::string& a, const std::string& b) {  // a >= b
  std::string r;
  int borrow = 0;
  for (size_t i = 0; i < a.size(); ++i) {
    int x = a[a.size() - 1 - i] - '0' - borrow;
    const int y = i < b.size() ? b[b.size() - 1 - i] - '0' : 0;
    borrow = x < y ? 1 : 0;
    if (x < y) x += 10;
    r += static_cast<char>('0' + x - y);
  }
  std::reverse(r.begin(), r.end());
  const size_t first = r.find_first_not_of('0');
  return first == std::string::npos ? "0" : r.substr(first);
}
std::string mag_mul(const std::string& a, const std::string& b) {
  if (a == "0" || b == "0") return "0";
  std::vector<int> d(a.size() + b.size(), 0);
  for (size_t i = 0; i < a.size(); ++i)
    for (size_t j = 0; j < b.size(); ++j) d[i + j] += (a[a.size() - 1 - i] - '0') * (b[b.size() - 1 - j] - '0');
  int carry = 0;
  for (int& x : d) {
    x += carry;
    carry = x / 10;
    x %= 10;
  }
  std::string r;
  for (auto it = d.rbegin(); it != d.rend(); ++it) r += static_cast<char>('0' + *it);
  return r.substr(r.find_first_not_of('0'));
}
Z z_neg(const Z& a) { return a.mag == "0" ? a : Z{!a.neg, a.mag}; }
Z z_add(const Z& a, const Z& b) {
  if (a.neg == b.neg) return Z{a.neg, mag_add(a.mag, b.mag)};
  const int c = mag_cmp(a.mag, b.mag);
  if (c == 0) return Z{};
  return c > 0 ? Z{a.neg, mag_sub(a.mag, b.mag)} : Z{b.neg, mag_sub(b.mag, a.mag)};
}
Z z_mul(const Z& a, const Z& b) {
  Z r{a.neg != b.neg, mag_mul(a.mag, b.mag)};
  if (r.mag == "0") r.neg = false;
  return r;
}
std::string z_text(const Z& a) { return a.neg ? "-" + a.mag : a.mag; }

// The value of a lowered operand that is a closed expression of integer literals: Add, Sub, Mul, Neg and `c_pow2` of a literal (what
// a constant, a `~`, a shift, a cast of constants and the parentheses of them lower to; cerastes never wraps, so a cast changes no
// value).  False for anything else: such an operand is not a constant mask and keeps the prelude function (decision 0026 item 7).
bool closed_value(const E& e, Z& out) {
  if (!e) return false;
  switch (e->kind) {
    case low::LExpr::Kind::IntLit: {
      if (!vir::dec::valid(e->text)) return false;
      out = vir::dec::negative(e->text) ? Z{true, e->text.substr(1)} : Z{false, e->text};
      return true;
    }
    case low::LExpr::Kind::Un: {
      Z a;
      if (e->un != low::UnOp::Neg || !closed_value(e->a, a)) return false;
      out = z_neg(a);
      return true;
    }
    case low::LExpr::Kind::Bin: {
      Z a, b;
      if (e->bin != low::BinOp::Add && e->bin != low::BinOp::Sub && e->bin != low::BinOp::Mul) return false;
      if (!closed_value(e->a, a) || !closed_value(e->b, b)) return false;
      out = e->bin == low::BinOp::Add ? z_add(a, b) : (e->bin == low::BinOp::Sub ? z_add(a, z_neg(b)) : z_mul(a, b));
      return true;
    }
    case low::LExpr::Kind::Fn: {
      Z n;
      if (e->text != "c_pow2" || e->args.size() != 1 || !closed_value(e->args[0], n) || n.neg || n.mag.size() > 3) return false;
      const int k = std::atoi(n.mag.c_str());
      if (k > 128) return false;  // the prelude's c_pow2 is 2^128 from n = 128 on (pow2_table), not 2^n
      out = Z{false, vir::dec::pow2(k)};
      return true;
    }
    default: return false;
  }
}

enum class MaskOp { And, Or, Xor };

// `other & mask`, `other | mask` and `other ^ mask` of a computed type `t` (32, 64 or 128 bits), written without the prelude function
// when `mask` is a constant of the forms of decision 0026 item 7 (`&`) and decision 0027 item 1 (`|`, `^`), else null.  With M the
// value of `mask` (in the type's range, or it is not rewritten), H = other \ 2^k and L = other % 2^k, and where a value is both forms
// the first row is used:
//   M = 2^k - 1 (0 <= k <= N unsigned, 0 <= k <= N - 1 signed):   `&`: L;   `|`: 2^k * H + (2^k - 1);   `^`: 2^k * H + ((2^k - 1) - L)
//   M = -2^k signed, or 2^N - 2^k unsigned (the same k):   `&`: 2^k * H (other with its low k bits cleared, naming it once);
//     `|` signed: L - 2^k;  `|` unsigned: L + (2^N - 2^k);  `^` signed: (L - 2^k) - 2^k * H;  `^` unsigned: (L + (2^N - 2^k)) - 2^k * H
// Viper's `%` by a positive divisor is the non-negative remainder, which is the two's complement value of the low k bits of every
// operand in the type's range, and `\` the floor quotient, with a = d * (a \ d) + a % d (decision 0026 item 8).  The `|` and `&` forms
// name `other` once and are inline.  The `^` forms need both H and L, so they are a call of the prelude function c_[u]xor_[c]mask_N_k
// (decision 0027 item 3, whose body is the form over its parameter: `other` is named once at the call); `used` receives its name.
E mask_rewrite(MaskOp op, const IntType& t, const E& other, const E& mask, const std::function<void(const std::string&)>& used) {
  Z m;
  if (!closed_value(mask, m)) return nullptr;
  const std::string text = z_text(m);
  if (!vir::dec::in_range(text, vir::dec::range_of(t))) return nullptr;
  using low::BinOp;
  auto xor_fn = [&](bool complement, int k) {
    const std::string name = std::string("c_") + (t.is_signed ? "" : "u") + "xor_" + (complement ? "cmask" : "mask") + "_" +
                             std::to_string(t.bits) + "_" + std::to_string(k);
    used(name);
    return low::fn(name, {other});
  };
  for (int k = 0; k <= t.bits; ++k) {
    if (m.neg || text != vir::dec::dec1(vir::dec::pow2(k))) continue;
    const std::string p = vir::dec::pow2(k);
    if (op == MaskOp::And) return low::mod_pow2(other, k);
    if (op == MaskOp::Or) return low::bin(BinOp::Add, low::bin(BinOp::Mul, low::int_lit(p), low::div_pow2(other, k)), low::int_lit(text));
    return xor_fn(false, k);
  }
  for (int k = 0; k <= t.bits; ++k) {
    const std::string p = vir::dec::pow2(k);
    const Z complement = t.is_signed ? Z{true, p} : z_add(Z{false, vir::dec::pow2(t.bits)}, Z{true, p});
    if (text != z_text(complement)) continue;
    if (op == MaskOp::And) return low::bin(BinOp::Mul, low::int_lit(p), low::div_pow2(other, k));
    if (op == MaskOp::Xor) return xor_fn(true, k);
    // `|`: the low part with the complement's high bits set: L - 2^k signed, L + (2^N - 2^k) unsigned.
    return t.is_signed ? low::bin(BinOp::Sub, low::mod_pow2(other, k), low::int_lit(p))
                       : low::bin(BinOp::Add, low::mod_pow2(other, k), low::int_lit(text));
  }
  return nullptr;
}

// ---------------------------------------------------------------------------------------------------------------
// The lowering context
// ---------------------------------------------------------------------------------------------------------------

// The fixed points of a full expression that C must have completed before an item is evaluated (decision 0021 item 3): their ids,
// in creation order.  Always closed downward (it holds the PRE of each member).
using Ids = std::set<int>;

// A set of fixed points in two versions (decision 0026 item 3), kept side by side through the whole lowering: join and `add` apply to
// both halves, and only the two places of 0026 item 1 give them different contents.  Neither half is larger than what C completes
// before the item (0021 item 3); EXACT is all of it only for the two sets of 0026 item 1 (a `requires` clause's and a comma's value),
// and every other set is smaller than that in both halves (0021 item 10).  BASE is the set that decisions 0021 item 10 and 0020 E6 used
// before M4.4d.  Both are sound (a smaller set is safe), and EXACT contains BASE (every difference only adds fixed points; `flush`
// checks it).  `flush` chooses the half per full expression and projects.
struct Deps {
  Ids ex, base;
  void add(int id) { ex.insert(id); base.insert(id); }
};

Ids join(const Ids& a, const Ids& b) {
  Ids r = a;
  r.insert(b.begin(), b.end());
  return r;
}
Deps join(const Deps& a, const Deps& b) { return Deps{join(a.ex, b.ex), join(a.base, b.base)}; }

enum class Half { Exact, Base };
const Ids& half_of(const Deps& d, Half h) { return h == Half::Exact ? d.ex : d.base; }

struct LV {
  E e;      // the Viper value
  Deps dep; // the fixed points whose results it mentions (D of each, so the set is closed)
};

// The items of a full expression are templates over the set type: lowering builds them with `Deps` (both halves), `flush` projects
// them onto one half (`Ids`).  `Obl`, `Effect` and `Full` are the lowering's; `FlatObl`, `FlatEffect` and `FlatFull` are what the
// placement, emission and fail-closed checks run on.
// Every field that is not a set is in a `...Core`, which `project` copies whole: a field added there reaches the projected item without
// being listed.  A new field that holds a set goes in the templated part, and in `project`.
struct OblCore {
  E guard;   // may be null
  E cond;
  low::Meta meta;
  size_t seq = 0;   // creation order within the Full (items of one slot keep it)
  int term_of = -1; // the fixed point (a call) whose explicit termination check this is (decision 0021 item 4), else -1
  int flag_of = -1; // the local (Var id) whose `uninitialized` flag this reads, else -1: a read of that local (decision 0025 item 1)
  // Non-empty: a statement item (a case split, decision 0037 item 8), emitted in the item's place instead of an assert; `cond` and `meta` are unused.
  std::vector<low::LStmt> stmts;
};
template <class S>
struct BasicObl : OblCore {
  S dep;            // its set: emitted after the fixed points of the set, before every other
};

struct CallItem {
  std::string method;
  std::string target;  // empty for a void call
  std::vector<E> args;
  low::Check check;
  bool has_termination = false;
  low::Check termination;
  std::vector<low::LStmt> after;  // emitted right after the call: the case splits on a pointer result (decision 0037 item 3)
};

// One effect of a full expression (decision 0020 items 5 and 7, decision 0021 item 3, decision 0025 item 2).  The FIXED POINTS are the
// calls and the branches that contain a call: they are numbered in creation order and emitted in the order pi (see flush).  Every other
// item, an obligation as well as a write, a snapshot or a call-free branch, has a set: the fixed points C must have completed before it
// evaluates it (through the temporaries it mentions, its guard, the comma floor, and the earlier accesses to the variables it touches).
// An obligation and a call-free branch are emitted right after the fixed points of its set and before every other (its set is a prefix
// of pi); a write and a snapshot cannot fail, so they are not in the family whose sets must form a chain: they are emitted right after
// the member of their set that comes last in pi.  Within one slot, items keep creation order; an item with an empty set comes before the
// first fixed point.  A callee that does not return cannot hide a check C may evaluate before the call (decision 0012 item 6): a
// callee can neither read nor write a local, and the sequencing rule leaves unsequenced operands no shared local.
enum class EffectKind {
  Call,      // a call (CallItem): a fixed point
  Write,     // `name := value`, then `flag := true` when `flag` is set: an assignment used as a value
  Snapshot,  // `name := value` where `name` is a fresh temporary and `value` the current value of a local this expression writes
  Branch     // `stmts`: the guarded operand of `&&`, `||` or `?:` with its own effects, as an `if`, assigning a temporary
};

struct EffectCore {
  using Kind = EffectKind;
  Kind kind = Kind::Call;
  CallItem call;
  std::string name;
  E value;
  std::string flag;
  std::vector<low::LStmt> stmts;
  int fixed = -1;                // the id of the fixed point (a Call, or a Branch with a call); -1 for the movable items
  size_t seq = 0;                // creation order within the Full
  std::set<int> reads, writes;   // the locals (Var ids) a snapshot or write, or a branch's inner effects, read and write
};
template <class S>
struct BasicEffect : EffectCore {
  S deps;                        // movable items: their set; fixed points: PRE (what C completes before it)
};

// What a rejection says about a fixed point.
struct FixedInfo {
  SrcLoc loc;
  std::string callee;  // the callee's name; empty for a branch
  std::string what;    // `the call to "f"`, or `the call to "g" in the right operand of "&&"`
};

// What one full expression leaves behind.
struct FullCore {
  int nfixed = 0;                       // fixed points so far: the next one's id
  std::vector<size_t> fixed_effect;     // the index in `effects` of each fixed point, by id
  std::vector<FixedInfo> info;          // by id
  size_t next_seq = 0;
};
template <class S>
struct BasicFull : FullCore {
  std::vector<BasicObl<S>> obls;
  std::vector<BasicEffect<S>> effects;
  std::vector<S> pre;                   // PRE of each fixed point, by id
  std::map<int, S> last_write;          // Var id -> the set of the latest write (item order of the expression so far)
  std::map<int, S> last_access;         // Var id -> the union of the sets of the accesses (snapshots and writes) of it so far
};

using Obl = BasicObl<Deps>;
using Effect = BasicEffect<Deps>;
using Full = BasicFull<Deps>;
using FlatObl = BasicObl<Ids>;
using FlatEffect = BasicEffect<Ids>;
using FlatFull = BasicFull<Ids>;

// The full expression on one half of its sets (the step `flush` chose); the last_write/last_access records are not needed after lowering.
FlatFull project(const Full& full, Half h) {
  FlatFull r;
  static_cast<FullCore&>(r) = full;
  for (const Obl& o : full.obls) {
    FlatObl x;
    static_cast<OblCore&>(x) = o;
    x.dep = half_of(o.dep, h);
    r.obls.push_back(std::move(x));
  }
  for (const Effect& fx : full.effects) {
    FlatEffect x;
    static_cast<EffectCore&>(x) = fx;
    x.deps = half_of(fx.deps, h);
    r.effects.push_back(std::move(x));
  }
  for (const Deps& d : full.pre) r.pre.push_back(half_of(d, h));
  return r;
}

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
    alloc_methods();
    LowerResult r;
    r.program = std::move(out_);
    r.errors = std::move(errors_);
    r.rejections = std::move(rejections_);
    return r;
  }

 private:
  // The trusted allocation of each cell type used (decision 0032 item 4; 0028 item 17), before every other method: a fresh one-slot block, its
  // cell uninitialized (full permission to the value and the flag, the flag false), and its liveness cell with `lvloc(b) == loc(b, -2)`.
  // The map rows of its clauses are at the first address-taken declaration of the type.
  void alloc_methods() {
    std::vector<low::LMethod> methods;
    for (const auto& [suffix, at] : alloc_at_) {
      const Type& t = cell_type_of_.at(suffix);
      const E r = low::var("r");
      const E b = block_of(r);
      auto clause = [&](E expr) {
        low::LClause c;
        c.expr = std::move(expr);
        for (const char* id : {"not.wellformed", "postcondition.violated"}) {
          low::Meta m;
          m.category = "internal";
          m.loc = at;
          m.message = std::string("internal check: ") + (std::string(id) == "not.wellformed" ? "well-formedness of" : "Silicon's own check of") +
                      " the trusted contract of the allocation of a variable of type `" + t.str() + "` whose address is taken";
          c.checks.push_back(low::Check{id, std::move(m)});
        }
        return c;
      };
      using low::BinOp;
      low::LMethod m;
      m.name = "alloc_" + suffix;
      m.trusted_reason = "allocation of an address-taken local of type " + t.str() +
                         ": a fresh one-slot block, the cell uninitialized and the block's liveness token, full permission (decision 0032 item 4)";
      m.results.push_back(low::LVar{"r", low::LType::Ref});
      m.requires_.clear();
      m.ensures_.push_back(clause(low::bin(BinOp::And, low::bin(BinOp::And, low::bin(BinOp::Ne, r, low::null_()),
                                                          low::bin(BinOp::Eq, r, low::dom("loc", {b, low::int_lit("0")}))),
                                           low::bin(BinOp::Eq, low::dom("len", {b}), low::int_lit("1")))));
      E rest = low::bin(BinOp::Eq, live_cell(r), low::dom("loc", {b, low::int_lit("-2")}));
      rest = low::bin(BinOp::And, rest, low::acc(r, "v_" + suffix));
      rest = low::bin(BinOp::And, rest, low::acc(r, "d_" + suffix));
      rest = low::bin(BinOp::And, rest, low::not_(cell_flag(r, t)));
      rest = low::bin(BinOp::And, rest, low::acc(live_cell(r), "lv"));
      m.ensures_.push_back(clause(rest));
      m.decreases_assumed = true;  // 0013 item 5
      methods.push_back(std::move(m));
    }
    out_.methods.insert(out_.methods.begin(), std::make_move_iterator(methods.begin()), std::make_move_iterator(methods.end()));
  }

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
    calls_in_expr(s->addr, out);  // a Store's address (decision 0032)
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
    // The blocks that declare cells and are open at the statement being lowered, outermost first (decision 0032 item 8).
    struct Scope {
      std::vector<const Var*> cells;  // in declaration order
      SrcLoc end;                     // the closing brace
    };
    std::vector<Scope> scopes;
  };

  void internal_error(const SrcLoc& loc, std::string what) { errors_.push_back(Diag{loc, std::move(what)}); }

  const std::string& var_name(const Var& v) {
    return fn_.var_names[static_cast<size_t>(v.id)];
  }

  // ---- locals declared without an initializer (decision 0019) ----

  static void find_flagged(const Stmt& s, std::set<int>& out) {
    switch (s.kind) {
      case Stmt::Kind::Decl:  // a cell's flag is a field of the cell (decision 0032 item 4, 0019 item 13), not a local
        if (s.expr == nullptr && s.var != nullptr && !s.var->address_taken) out.insert(s.var->id);
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
    if (v.type.is_ptr()) return low::null_();
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

  void use(const std::string& prelude_fn) { out_.prelude_used.insert(prelude_fn); }

  // ---- obligations ----

  // `exact_set`, when given, is joined into the EXACT half of the set (decision 0026 item 1: a `requires` clause's obligation has PRE of
  // its call); the BASE half is always the guard's, the value's and the floor's.
  void obl(Full& full, const E& guard, const Deps& gdep, E cond, const Deps& cdep, low::Meta m, const Ids* exact_set = nullptr) {
    Obl o;
    o.guard = guard;
    o.cond = std::move(cond);
    o.meta = std::move(m);
    o.dep = join(join(gdep, cdep), floor_);  // floor_: after the left operand of a comma, what it depends on came first (E6)
    if (exact_set != nullptr) o.dep.ex = join(o.dep.ex, *exact_set);
    o.seq = full.next_seq++;
    full.obls.push_back(std::move(o));
  }

  // ---- the memory model (decision 0032; decision 0028 items 1-4 and 11) ----

  // The field-name suffix `<t>` of a cell type (decision 0032 item 5): the VIR type by width and signedness (`long` and `long long` share
  // `long`), an enumeration by its own, numbered name.  It never ends in `__<digits>`, so `v_<t>` cannot be the name of a local.
  std::string cell_suffix(const Type& t) {
    if (t.is_bool()) return "bool";
    const IntType& it = *t.int_type;
    if (it.is_enum) {
      auto found = enum_suffix_.find(&it);
      if (found == enum_suffix_.end()) {
        std::string name = "enum_" + std::to_string(enum_suffix_.size() + 1) + "_" + sanitize(it.name);
        const size_t us = name.rfind("__");
        if (us != std::string::npos && name.find_first_not_of("0123456789", us + 2) == std::string::npos) name += "_e";
        found = enum_suffix_.emplace(&it, std::move(name)).first;
      }
      return found->second;
    }
    switch (it.bits) {
      case 8: return it.is_signed ? "schar" : "uchar";
      case 16: return it.is_signed ? "short" : "ushort";
      case 32: return it.is_signed ? "int" : "uint";
      case 64: return it.is_signed ? "long" : "ulong";
      case 128: return it.is_signed ? "i128" : "u128";
      default: return (it.is_signed ? "i" : "u") + std::to_string(it.bits);
    }
  }

  // Record that the program uses cells of type `t` (its fields, and the memory prelude); `at` is a position of the first use that the
  // trusted `alloc_<t>` method's map rows can name: the earliest address-taken declaration of the type (a cell's type).
  std::string use_cell_type(const Type& t) {
    const std::string suffix = cell_suffix(t);
    out_.memory = true;
    out_.cell_types[suffix] = t.is_bool() ? low::LType::Bool : low::LType::Int;
    cell_type_of_[suffix] = t;
    return suffix;
  }

  void alloc_site(const Type& t, const SrcLoc& at) {
    const std::string suffix = use_cell_type(t);
    const auto it = alloc_at_.find(suffix);
    const bool earlier = it == alloc_at_.end() || (at.file == it->second.file && (at.line < it->second.line || (at.line == it->second.line && at.column < it->second.column)));
    if (earlier) alloc_at_[suffix] = at;
  }

  // `recv.v_<t>`, `recv.d_<t>`, `perm(recv.v_<t>)`, `lvloc(block_of(recv)).lv`.
  E cell_value(const E& recv, const Type& t) { return low::field(recv, "v_" + use_cell_type(t)); }
  E cell_flag(const E& recv, const Type& t) { return low::field(recv, "d_" + use_cell_type(t)); }
  E value_perm(const E& recv, const Type& t) { return low::perm_of(recv, "v_" + use_cell_type(t)); }
  E flag_perm(const E& recv, const Type& t) { return low::perm_of(recv, "d_" + use_cell_type(t)); }
  E block_of(const E& ptr) { out_.memory = true; return low::dom("block_of", {ptr}); }
  E live_cell(const E& ptr) { out_.memory = true; return low::dom("lvloc", {block_of(ptr)}); }
  E live_perm(const E& ptr) { return low::perm_of(live_cell(ptr), "lv"); }
  // perm(recv.v_<t>) > none
  E may_read(const E& recv, const Type& t) { return low::bin(low::BinOp::Gt, value_perm(recv, t), low::no_perm()); }
  // The write permission on both fields as ONE atomic assertion: !(perm(v) != write || perm(d) != write [|| perm(lv) != write]).
  E may_write(const E& recv, const Type& t, bool with_liveness) {
    using low::BinOp;
    E bad = low::bin(BinOp::Or, low::bin(BinOp::Ne, value_perm(recv, t), low::write_perm()), low::bin(BinOp::Ne, flag_perm(recv, t), low::write_perm()));
    if (with_liveness) bad = low::bin(BinOp::Or, bad, low::bin(BinOp::Ne, live_perm(recv), low::write_perm()));
    return low::not_(bad);
  }

  static std::string quoted(const std::string& text, const char* fallback) {
    return text.empty() ? std::string(fallback) : "`" + text + "`";
  }

  // The cells declared directly in a block (a Decl of an address-taken variable), in declaration order.
  static std::vector<const Var*> cells_of(const Stmt& block) {
    std::vector<const Var*> cells;
    for (const Stmt* c : block.stmts)
      if (c != nullptr && c->kind == Stmt::Kind::Decl && c->var != nullptr && c->var->address_taken) cells.push_back(c->var);
    return cells;
  }

  // `x := alloc_<t>()` at the entry of the block that declares the cell `x` (decision 0032 item 4).
  void alloc_cell(std::vector<low::LStmt>& out, const Var& x) {
    alloc_site(x.type, x.loc);
    low::LStmt c;
    c.kind = low::LStmt::Kind::Call;
    c.name = "alloc_" + cell_suffix(x.type);
    c.target = var_name(x);
    c.check = low::Check{"call.precondition", meta("internal", x.loc, "internal check: Silicon's own check of the allocation of `" + x.name + "`")};
    out.push_back(std::move(c));
  }

  // Leaving the scope of `cells` (decision 0032 item 8): full permission to each cell, its flag and its liveness cell is back (`lifetime`),
  // then all three are exhaled; the cells in reverse declaration order.
  void scope_exit(std::vector<low::LStmt>& out, const std::vector<const Var*>& cells, const SrcLoc& at) {
    for (size_t k = cells.size(); k-- > 0;) {
      const Var& x = *cells[k];
      const E r = low::var(var_name(x));
      emit_assert(out, may_write(r, x.type, true),
                  meta("lifetime", at, "the address of `" + x.name + "` may still be in use when its lifetime ends"));
      low::LStmt ex;
      ex.kind = low::LStmt::Kind::Exhale;
      ex.expr = low::bin(low::BinOp::And, low::bin(low::BinOp::And, low::acc(r, "v_" + cell_suffix(x.type)), low::acc(r, "d_" + cell_suffix(x.type))),
                         low::acc(live_cell(r), "lv"));
      out.push_back(std::move(ex));
    }
  }

  void field_assign(std::vector<low::LStmt>& out, const std::string& recv, const std::string& field, E value) {
    low::LStmt s;
    s.kind = low::LStmt::Kind::FieldAssign;
    s.name = recv;
    s.field = field;
    s.expr = std::move(value);
    out.push_back(std::move(s));
  }

  // `x.v_<t> := value; x.d_<t> := true` for a Ref variable (the lint requires the flag write to follow the value's directly).
  void write_fields(std::vector<low::LStmt>& out, const std::string& recv, const Type& t, E value) {
    const std::string suffix = use_cell_type(t);
    field_assign(out, recv, "v_" + suffix, std::move(value));
    field_assign(out, recv, "d_" + suffix, low::bool_lit(true));
  }

  // A cell read by name: its own permission is the function's (`internal`), then the flag (`uninitialized`), value `x.v_<t>`.
  LV read_cell(const Expr& e, const E& guard, const Deps& gdep, Full& full) {
    const E r = low::var(var_name(*e.var));
    obl(full, guard, gdep, may_read(r, e.var->type), {},
        meta("permission", e.loc, "reads `" + e.var->name + "` without owning it: a callee may have kept its object"));
    obl(full, guard, gdep, cell_flag(r, e.var->type), {},
        meta("uninitialized", e.loc, "`" + e.var->name + "` may be read here before it is given a value"));
    return {cell_value(r, e.var->type), {}};
  }

  // A cell written by name (`x = e;`, the declaration's initializer, `x += e;`), after the full expression of its value: the permission is
  // the function's (`internal`), then both fields are written.
  void write_cell(std::vector<low::LStmt>& out, const Var& x, const E& value, const SrcLoc& at) {
    const E r = low::var(var_name(x));
    emit_assert(out, may_write(r, x.type, false),
                meta("permission", at, "writes `" + x.name + "` without owning it: a callee may have kept its object"));
    write_fields(out, var_name(x), x.type, value);
  }

  // `*p` as a read (decision 0032 item 6): the null check, the permission check (which is also the liveness check of this use, 0028 item
  // 3a), the flag; the value `p.v_<t>`.
  LV deref(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full) {
    if (cmode_ != CMode::None) return contract_read(e, env, guard, gdep, full);
    LV p = lower(*e.a, env, guard, gdep, full);
    const std::string ptr_text = e.a->c_text;
    const std::string place = e.c_text.empty() ? (ptr_text.empty() ? std::string() : "*" + ptr_text) : e.c_text;
    const Type& t = e.type;
    obl(full, guard, gdep, low::bin(low::BinOp::Ne, p.e, low::null_()), p.dep,
        meta("null-dereference", e.loc, "dereferences " + quoted(ptr_text, "the pointer") + ", which may be null"));
    obl(full, guard, gdep, may_read(p.e, t), p.dep,
        meta("permission", e.loc,
             "reads " + quoted(place, "the object the pointer points to") + " without owning it: the object it points to may have ended"));
    obl(full, guard, gdep, cell_flag(p.e, t), p.dep,
        meta("uninitialized", e.loc, quoted(place, "the object the pointer points to") + " may be read here before it is given a value"));
    return {cell_value(p.e, t), p.dep};
  }

  // Any other use of a pointer value (`==`, `!=`, a truth test, a copy): `p != null ==> perm(lvloc(block_of(p)).lv) > none` (`lifetime`),
  // skipped for `&x` and a null constant, which cannot have a dead object (decision 0032 item 6).
  static bool address_or_null(const Expr& e) {
    if (e.kind == Expr::Kind::AddrOf || e.kind == Expr::Kind::Null) return true;
    return e.kind == Expr::Kind::Cond && e.b != nullptr && e.c != nullptr && address_or_null(*e.b) && address_or_null(*e.c);
  }

  void ptr_use(const Expr& operand, const LV& v, const E& guard, const Deps& gdep, Full& full) {
    if (cmode_ != CMode::None) return;  // a contract has no heap and no liveness to check (decision 0037 item 7)
    if (address_or_null(operand)) return;
    const E live = low::bin(low::BinOp::Gt, live_perm(v.e), low::no_perm());
    obl(full, guard, gdep, low::implies(low::bin(low::BinOp::Ne, v.e, low::null_()), live), v.dep,
        meta("lifetime", operand.loc, "uses " + quoted(operand.c_text, "the pointer") + " after the object it points to may have ended"));
  }

  // 0028 item 3's exception for `==` and `!=` (decision 0032 item 7): two non-null pointers into different blocks, one of them one past the
  // end of its block, are not decided by the model.  In b1a every pointer is null or offset 0 of a one-slot block, so it is provable; as an
  // `internal` row, a failure is a bug (exit 3).  Never for a null operand.
  void one_past_end(const Expr& e, const LV& a, const LV& b, const E& guard, const Deps& gdep, const Deps& dep, Full& full) {
    using low::BinOp;
    if (e.a->kind == Expr::Kind::Null || e.b->kind == Expr::Kind::Null) return;
    out_.memory = true;
    auto past = [&](const E& p) { return low::bin(BinOp::Eq, low::dom("offset_of", {p}), low::dom("len", {block_of(p)})); };
    E bad = low::bin(BinOp::And, low::bin(BinOp::Ne, a.e, low::null_()), low::bin(BinOp::Ne, b.e, low::null_()));
    bad = low::bin(BinOp::And, bad, low::bin(BinOp::Ne, block_of(a.e), block_of(b.e)));
    bad = low::bin(BinOp::And, bad, low::bin(BinOp::Or, past(a.e), past(b.e)));
    obl(full, guard, gdep, low::not_(bad), dep,
        meta("pointer-comparison", e.loc,
             "compares " + quoted(e.a->c_text, "a pointer") + " and " + quoted(e.b->c_text, "a pointer") +
                 ", which may point into different objects, one of them one past its end"));
  }

  static bool shares_node(const Expr& root, const Expr* needle) {
    if (root.kind == Expr::Kind::Deref && root.a == needle) return true;
    for (const Expr* x : {root.a, root.b, root.c})
      if (x != nullptr && shares_node(*x, needle)) return true;
    for (const Expr* x : root.args)
      if (x != nullptr && shares_node(*x, needle)) return true;
    return false;
  }

  // `*addr = value;` and its compound forms (decision 0032 items 6 and 9): the address and the value in one full expression, then (a
  // non-variable address copied into a Ref temporary) the null check, the write permission on both fields, the two writes.  A compound
  // store's value reads `*addr` through the same node: the pointer is lowered once and the read reuses its value (`shared_`).
  void store(const Stmt& s, std::vector<low::LStmt>& out) {
    const bool compound = shares_node(*s.expr, s.addr);  // the value reads `*addr` through the same node: the pointer is evaluated once
    const std::vector<LV> vs = full_exprs({s.addr, s.expr}, out, nullptr, compound);
    E recv = vs[0].e;
    if (recv->kind != low::LExpr::Kind::Var) {
      const std::string t = new_temp(low::LType::Ref);
      assign(out, t, recv);
      recv = low::var(t);
    }
    const Type& t = s.expr->type;
    const std::string ptr_text = s.addr->c_text;
    const std::string place = ptr_text.empty() ? std::string() : "*" + ptr_text;
    // Always checked: the read of a compound store checked the same value, but the write's own check costs nothing and is never skipped.
    emit_assert(out, low::bin(low::BinOp::Ne, recv, low::null_()),
                meta("null-dereference", s.loc, "dereferences " + quoted(ptr_text, "the pointer") + ", which may be null"));
    emit_assert(out, may_write(recv, t, false),
                meta("permission", s.loc,
                     "writes " + quoted(place, "the object the pointer points to") + " without owning it: the object it points to may have ended"));
    write_fields(out, recv->text, t, vs[1].e);
  }

  // ---- contracts with pointers (decision 0037) ----

  // How a contract expression is being lowered.  Method: a clause of the function's own Viper method (atoms are `acc` assertions, reads are heap
  // reads, `old` is Viper's); Wf: the well-formedness method (no heap: reads are ghost value formals, atoms their pure facts); Call: a callee's
  // value `requires` over the argument values at a call (the atoms were asserted one by one, so they lower to `true`); Ret: a value `ensures`
  // asserted at a `return` (atoms were asserted, `old` is Viper's).  None: code, an invariant, a measure.
  enum class CMode { None, Method, Wf, Call, Ret };

  struct FnAtoms {
    std::vector<AtomSite> sites;               // contract_atoms: the requires atoms first
    std::map<const Expr*, size_t> index;       // the Atom node -> its position in `sites`
    int nreq = 0;                              // the number of requires atoms (the ghost amounts)
  };

  const FnAtoms& atoms_of(const Function& f) {
    const auto it = atoms_.find(&f);
    if (it != atoms_.end()) return it->second;
    FnAtoms fa;
    fa.sites = contract_atoms(f);
    for (size_t i = 0; i < fa.sites.size(); ++i) {
      fa.index[fa.sites[i].atom] = i;
      if (!fa.sites[i].in_ensures) ++fa.nreq;
    }
    return atoms_.emplace(&f, std::move(fa)).first->second;
  }

  // Restores the contract context at the end of a scope.
  struct CtxSave {
    Lowerer& l;
    CMode mode;
    const Function* f;
    bool ens, old;
    size_t seen;
    explicit CtxSave(Lowerer& lw) : l(lw), mode(lw.cmode_), f(lw.cf_), ens(lw.cens_), old(lw.cold_), seen(lw.cseen_) {}
    ~CtxSave() {
      l.cmode_ = mode;
      l.cf_ = f;
      l.cens_ = ens;
      l.cold_ = old;
      l.cseen_ = seen;
    }
  };

  // The position in `sites` of the first atom at or after clause `clause` of the requires (`ens` false) or ensures list.
  static size_t first_site(const FnAtoms& fa, bool ens, int clause) {
    for (size_t i = 0; i < fa.sites.size(); ++i) {
      const AtomSite& s = fa.sites[i];
      if (s.in_ensures == ens && s.clause >= clause) return i;
      if (s.in_ensures && !ens) return i;
    }
    return fa.sites.size();
  }

  void set_ctx(const Function& f, CMode mode, bool ens, int clause) {
    cmode_ = mode;
    cf_ = &f;
    cens_ = ens;
    cold_ = false;
    cseen_ = first_site(atoms_of(f), ens, clause);
  }

  static std::string amount_name(int k) { return "amt_" + std::to_string(k); }

  // `p == null || (p == loc(block_of(p), offset_of(p)) && 0 <= offset_of(p) && offset_of(p) <= len(block_of(p)))`: decision 0028 item 2, inline.
  E wf_expr(const E& p) {
    using low::BinOp;
    out_.memory = true;
    const E off = low::dom("offset_of", {p});
    E inner = low::bin(BinOp::Eq, p, low::dom("loc", {block_of(p), off}));
    inner = low::bin(BinOp::And, inner, low::bin(BinOp::Le, low::int_lit("0"), off));
    inner = low::bin(BinOp::And, inner, low::bin(BinOp::Le, off, low::dom("len", {block_of(p)})));
    return low::bin(BinOp::Or, low::bin(BinOp::Eq, p, low::null_()), inner);
  }
  E in_block(const E& p) {  // offset_of(p) < len(block_of(p))
    return low::bin(low::BinOp::Lt, low::dom("offset_of", {p}), low::dom("len", {block_of(p)}));
  }
  // The range of a cell's value as one atomic condition (none for bool).
  E cell_range(const E& p, const Type& t) {
    if (!t.is_bool()) return range_obligation(cell_value(p, t), *t.int_type);
    return nullptr;
  }

  static const Type& site_pointee(const AtomSite& s) { return *s.atom->a->type.pointee; }
  static int site_param(const AtomSite& s) { return s.atom->a->index; }
  static bool site_takes_cell(const AtomSite& s) { return s.atom->atom != AtomKind::Live; }

  // The guard of a site over `env`'s formals: the conjunction of its `||`/`?:` conditions (null: unconditional).
  E site_guard(const AtomSite& s, const Env& env) {
    E g;
    CtxSave save(*this);
    cmode_ = CMode::Wf;  // a guard is lowered as a pure contract expression whatever the context (a pointer `?:` is an ite, never an effect)
    for (const AtomGuard& x : s.guards) {
      Full scratch;
      LV v = lower(*x.cond, env, nullptr, {}, scratch);
      g = low::and_(g, x.holds ? v.e : low::not_(v.e));
    }
    return g;
  }

  // Two atoms that must not name one cell: two `owns`, or an `owns` and a `reads`, of one field (decision 0037 items 8 and 10).
  bool exclusive_pair(const AtomSite& a, const AtomSite& b) {
    if (!site_takes_cell(a) || !site_takes_cell(b)) return false;
    if (a.atom->atom == AtomKind::Reads && b.atom->atom == AtomKind::Reads) return false;
    return cell_suffix(site_pointee(a)) == cell_suffix(site_pointee(b));
  }

  // An atom (decision 0037 items 5 and 6) in the context being lowered.
  LV atom(const Expr& e, const Env& env) {
    using low::BinOp;
    const FnAtoms& fa = atoms_of(*cf_);
    const auto it = fa.index.find(&e);
    if (it == fa.index.end()) {
      internal_error(e.loc, "a permission atom in a position the contract rules forbid reached lowering");
      return {low::bool_lit(true), {}};
    }
    const size_t idx = it->second;
    cseen_ = std::max(cseen_, idx + 1);
    if (cmode_ == CMode::Call || cmode_ == CMode::Ret) return {low::bool_lit(true), {}};  // asserted one by one at the call or the return
    const AtomSite& site = fa.sites[idx];
    const E p = env.formals[static_cast<size_t>(site_param(site))];
    const Type& t = site_pointee(site);
    E pure = low::and_(low::bin(BinOp::Ne, p, low::null_()), wf_expr(p));
    if (site_takes_cell(site)) pure = low::and_(pure, in_block(p));
    if (cmode_ == CMode::Wf) {
      // Two full atoms of one list name two cells (the fact Silicon derives from the two `acc`s), added after the second one (decision 0037 item 7).
      if (e.atom == AtomKind::Owns)
        for (size_t i = 0; i < idx; ++i) {
          const AtomSite& o = fa.sites[i];
          if (o.in_ensures != site.in_ensures || o.atom->atom != AtomKind::Owns || site_param(o) == site_param(site)) continue;
          if (cell_suffix(site_pointee(o)) != cell_suffix(t)) continue;
          pure = low::and_(pure, low::implies(site_guard(o, env), low::bin(BinOp::Ne, env.formals[static_cast<size_t>(site_param(o))], p)));
        }
      return {pure, {}};
    }
    if (e.amount < 0) {
      internal_error(e.loc, "a permission atom without its ghost amount reached lowering");
      return {low::bool_lit(true), {}};
    }
    const E amount = low::var(amount_name(e.amount));
    E r = pure;
    if (site_takes_cell(site)) {
      const bool owns = e.atom == AtomKind::Owns;
      const std::string sfx = use_cell_type(t);
      r = low::and_(r, owns ? low::acc(p, "v_" + sfx) : low::acc(p, "v_" + sfx, amount));
      r = low::and_(r, owns ? low::acc(p, "d_" + sfx) : low::acc(p, "d_" + sfx, amount));
      r = low::and_(r, cell_flag(p, t));
      if (!t.is_bool()) r = low::and_(r, range_check(cell_value(p, t), *t.int_type));
    }
    r = low::and_(r, low::acc(live_cell(p), "lv", amount));
    return {r, {}};
  }

  static std::string ghost_name(int param, bool post) { return "hv_" + std::to_string(param) + (post ? "_post" : "_pre"); }

  // A read of `*p` in a contract (decision 0037 item 7).  Wf: a ghost value formal, and the framing obligation; otherwise the heap read, guarded
  // by itself: the chain `(D_1 ? read : (D_2 ? read : ... : dummy))` over the framing disjuncts of the earlier atoms of the same field, or the
  // plain read when the framing is trivial (an unconditional atom on the same place) or implied by the guard the read is evaluated under.
  LV contract_read(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full) {
    using low::BinOp;
    if (e.a == nullptr || e.a->kind != Expr::Kind::Formal) {
      internal_error(e.loc, "a dereference in a contract of anything but a pointer parameter reached lowering");
      return {low::int_lit("0"), {}};
    }
    const int k = e.a->index;
    const E p = env.formals[static_cast<size_t>(k)];
    const Type& t = e.type;
    const bool post = cens_ && !cold_;
    const FnAtoms& fa = atoms_of(*cf_);
    const std::string sfx = use_cell_type(t);
    std::vector<E> ds;  // the framing disjuncts D_i; a null one is `true`
    for (size_t i = 0; i < fa.sites.size(); ++i) {
      const AtomSite& s = fa.sites[i];
      if (!site_takes_cell(s)) continue;
      if (post) {
        if (!s.in_ensures || i >= cseen_) continue;
      } else {
        if (s.in_ensures) continue;
        if (!cens_ && i >= cseen_) continue;
      }
      if (cell_suffix(site_pointee(s)) != sfx) continue;
      E d = site_guard(s, env);
      if (site_param(s) != k) d = low::and_(d, low::bin(BinOp::Eq, env.formals[static_cast<size_t>(site_param(s))], p));
      ds.push_back(d);
    }
    bool trivial = false;
    for (const E& d : ds)
      if (!d || implied_by(d, guard)) trivial = true;
    if (cmode_ == CMode::Wf) {
      const std::pair<int, bool> key{k, post};
      wf_ghosts_[key] = t;
      if (!trivial) {
        E any;
        for (const E& d : ds) any = any ? low::bin(BinOp::Or, any, d) : d;
        if (!any) any = low::bool_lit(false);
        obl(full, guard, gdep, any, {},
            meta("permission", e.loc,
                 "the contract of `" + cf_->c_name + "` reads " + quoted(e.c_text.empty() ? std::string() : e.c_text, "a pointer's object") +
                     " without owning it"));
      }
      return {low::var(ghost_name(k, post)), {}};
    }
    E read = cell_value(p, t);
    if (cold_ && (cmode_ == CMode::Method || cmode_ == CMode::Ret)) read = low::old_(read);
    if (trivial) return {read, {}};
    E v = t.is_bool() ? low::bool_lit(false) : low::int_lit("0");
    for (size_t i = ds.size(); i-- > 0;) v = low::ite(ds[i], read, v);
    return {v, {}};
  }

  // ---- expressions ----

  LV lower(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full) {
    if (!shared_.empty()) {  // a node used twice (the pointer of a compound store, decision 0032 item 9): lowered once
      const auto it = shared_.find(&e);
      if (it != shared_.end() && e.kind != Expr::Kind::Local) return it->second;
    }
    switch (e.kind) {
      case Expr::Kind::Const:
        if (e.type.is_bool()) return {low::bool_lit(e.value == "true"), {}};
        return {low::int_lit(e.value), {}};
      case Expr::Kind::Local:
        if (!shared_.empty()) {
          const auto it = shared_.find(&e);  // the old value of `x++`: read once (decision 0020)
          if (it != shared_.end()) return it->second;
        }
        if (e.var->address_taken) return read_cell(e, guard, gdep, full);
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
        if (cmode_ != CMode::None && reads_heap(*e.a)) {  // old(*p): the cell's value on entry (decision 0037 item 5)
          const bool saved = cold_;
          cold_ = true;
          LV r = lower(*e.a, env, guard, gdep, full);
          cold_ = saved;
          return r;
        }
        return lower(*e.a, env, guard, gdep, full);
      case Expr::Kind::Unary: return unary(e, env, guard, gdep, full);
      case Expr::Kind::Binary: return binary(e, env, guard, gdep, full);
      case Expr::Kind::Cond: {
        if (cmode_ == CMode::None && (e.type.is_ptr() || has_effect(*e.b) || has_effect(*e.c))) return branch_cond(e, env, guard, gdep, full);
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
        if (e.a->type.is_ptr()) {  // a pointer as a truth value is `p != null` (decision 0032 item 7), a use of its value
          ptr_use(*e.a, a, guard, gdep, full);
          return {low::bin(low::BinOp::Ne, a.e, low::null_()), a.dep};
        }
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
      case Expr::Kind::Null: return {low::null_(), {}};
      case Expr::Kind::AddrOf: {  // the cell's Ref: a pointer value, no obligation of its own (decision 0032 item 6)
        if (e.var == nullptr || !e.var->address_taken) {
          internal_error(e.loc, "the address of a variable that is not a cell reached lowering");
          return {low::null_(), {}};
        }
        return {low::var(var_name(*e.var)), {}};
      }
      case Expr::Kind::Deref: return deref(e, env, guard, gdep, full);
      case Expr::Kind::Atom:
        if (cmode_ == CMode::None) {
          internal_error(e.loc, "a permission atom outside a contract reached lowering");
          return {low::bool_lit(true), {}};
        }
        return atom(e, env);
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
      case UnaryOp::BitNot: {
        // Decision 0023 item 1: -1 - a (signed) or MAX - a (unsigned): linear, in range for every a, no obligation.
        const IntType& t = *e.type.int_type;
        // The same guard as the binary bitwise operators (M4.4b1 audit note 1): check_program rejects an enumeration operand and the
        // encoder converts to a computed type of 32, 64 or 128 bits, so this is unreachable, and an internal error if it is not.
        if (t.is_enum || (t.bits != 32 && t.bits != 64 && t.bits != 128))
          internal_error(e.loc, "a bitwise complement on a " + std::to_string(t.bits) + "-bit type (the encoder converts to a computed type of 32, 64 or 128 bits)");
        const E k = low::int_lit(t.is_signed ? "-1" : dec::range_of(t).hi);
        return {low::bin(low::BinOp::Sub, k, a.e), a.dep};
      }
    }
    internal_error(e.loc, "unknown unary operator");
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
    if ((lazy_and || lazy_or) && cmode_ == CMode::None && has_effect(*e.b)) return branch_logical(e, env, guard, gdep, full);
    LV a = lower(*e.a, env, guard, gdep, full);
    E gb = guard;
    Deps gbd = gdep;
    if (lazy_and) { gb = low::and_(guard, a.e); gbd = join(gdep, a.dep); }
    if (lazy_or) { gb = low::and_(guard, low::not_(a.e)); gbd = join(gdep, a.dep); }
    LV b = lower(*e.b, env, gb, gbd, full);
    const Deps dep = join(a.dep, b.dep);
    if ((e.bop == BinaryOp::Eq || e.bop == BinaryOp::Ne) && e.a->type.is_ptr()) {
      // Decision 0032 items 6 and 7: both operands are uses of a pointer value, then the one-past-the-end exception of 0028 item 3.
      ptr_use(*e.a, a, guard, gdep, full);
      ptr_use(*e.b, b, guard, gdep, full);
      if (cmode_ == CMode::None) one_past_end(e, a, b, guard, gdep, dep, full);  // in a contract `==` is identity in the model (decision 0037 item 12)
    }
    switch (e.bop) {
      case BinaryOp::LAnd:
        if ((cmode_ == CMode::Call || cmode_ == CMode::Ret) && !atoms_of(*cf_).sites.empty()) {  // an atom lowers to `true` here (asserted one by one): keep the value part alone
          if (always_true(a.e)) return {b.e, dep};
          if (always_true(b.e)) return {a.e, dep};
        }
        return {low::bin(BinOp::And, a.e, b.e), dep};
      case BinaryOp::LOr:
        // Viper refuses `acc` under `||`: `a || atom` is `!a ==> atom` (decision 0037 item 5).
        if (cmode_ == CMode::Method && has_atom(*e.b)) return {low::bin(BinOp::Implies, low::not_(a.e), b.e), dep};
        return {low::bin(BinOp::Or, a.e, b.e), dep};
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
      case BinaryOp::BitAnd: case BinaryOp::BitOr: case BinaryOp::BitXor: {
        // Decision 0023 item 2 (S21): total prelude functions c_[u]<op>_N by the computed type's signedness and width; the
        // result is always in range, so no obligation.
        const IntType& t = *e.type.int_type;
        if (t.is_enum || (t.bits != 32 && t.bits != 64 && t.bits != 128))
          internal_error(e.loc, "a bitwise operator on a " + std::to_string(t.bits) + "-bit type (the encoder converts to a computed type of 32, 64 or 128 bits)");
        // Decision 0026 item 7 (`&`) and decision 0027 item 1 (`|`, `^`): a constant mask (either operand, as lowered; both operands
        // are lowered as before, so their obligations stay) is a remainder or a quotient; anything else keeps the prelude function,
        // which is exact for every operand.
        {
          const MaskOp mop = e.bop == BinaryOp::BitAnd ? MaskOp::And : (e.bop == BinaryOp::BitOr ? MaskOp::Or : MaskOp::Xor);
          const std::function<void(const std::string&)> used = [this](const std::string& n) { use(n); };
          E r = mask_rewrite(mop, t, a.e, b.e, used);
          if (r == nullptr) r = mask_rewrite(mop, t, b.e, a.e, used);
          if (r != nullptr) return {r, dep};
        }
        const char* op = e.bop == BinaryOp::BitAnd ? "and" : (e.bop == BinaryOp::BitOr ? "or" : "xor");
        const std::string name = std::string("c_") + (t.is_signed ? "" : "u") + op + "_" + std::to_string(t.bits);
        use(name);
        return {low::fn(name, {a.e, b.e}), dep};
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
      if (full.effects[k].fixed >= 0) r.add(full.effects[k].fixed);
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

  // The explicit checks of the permissions a callee's atoms need at a call (decision 0037 item 8): each part of each atom under its guard,
  // the separation of atoms that must not share a cell, the case splits, the amounts' bounds and the sums over the cells they are taken from.
  // They are obligations of the call with the exact set PRE(c), in the order they are created.  Returns the ghost amounts, one per `requires`
  // atom, `(g ? m : write) / (N + 1)` (decision 0037 item 6).
  std::vector<E> call_atoms(const Expr& e, const Function& callee, const std::vector<LV>& args, const Env& cenv, const Deps& pre, Full& full) {
    using low::BinOp;
    std::vector<E> amounts;
    const FnAtoms& fa = atoms_of(callee);
    const size_t n = static_cast<size_t>(fa.nreq);
    if (n == 0) return amounts;
    Deps adep;
    for (const LV& a : args) adep = join(adep, a.dep);
    struct S {
      const AtomSite* site = nullptr;
      E p, g;
      const Type* t = nullptr;
      std::string place;
    };
    std::vector<S> ss;
    for (size_t i = 0; i < n; ++i) {  // the requires atoms come first in `sites`
      S x;
      x.site = &fa.sites[i];
      const int j = site_param(*x.site);
      x.p = cenv.formals[static_cast<size_t>(j)];
      x.g = site_guard(*x.site, cenv);
      x.t = &site_pointee(*x.site);
      const std::string txt = e.args[static_cast<size_t>(j)]->c_text;
      x.place = txt.empty() ? std::string() : (txt[0] == '&' ? txt.substr(1) : "*" + txt);
      ss.push_back(std::move(x));
    }
    auto text_of = [&](const S& x) {
      const std::string pl = quoted(x.place, "the object the pointer points to");
      switch (x.site->atom->atom) {
        case AtomKind::Owns: return "calls `" + callee.c_name + "`, which needs to own " + pl + ", without owning it";
        case AtomKind::Reads: return "calls `" + callee.c_name + "`, which needs to read " + pl + ", without the permission to read it";
        case AtomKind::Live: {
          const Expr& arg = *e.args[static_cast<size_t>(site_param(*x.site))];
          const std::string who = arg.kind == Expr::Kind::Null ? std::string("the null pointer")
                                  : arg.c_text.empty()         ? std::string("the pointer")
                                                               : "`" + arg.c_text + "`";
          return "calls `" + callee.c_name + "`, which needs " + who + " to point to a live object";
        }
      }
      return std::string();
    };
    auto add = [&](const E& guard, E cond, const std::string& message) {
      obl(full, guard, adep, std::move(cond), {}, meta("permission", e.loc, message), &pre.ex);
    };
    auto split = [&](E cond) {
      obl(full, nullptr, adep, low::bool_lit(true), {}, meta("internal", e.loc, "internal check: a case split"), &pre.ex);
      low::LStmt st;
      st.kind = low::LStmt::Kind::CaseSplit;
      st.expr = std::move(cond);
      full.obls.back().stmts.push_back(std::move(st));
    };
    const E none = low::no_perm(), write = low::write_perm();
    // 1. each part of each atom, under its guard
    for (const S& x : ss) {
      const std::string m = text_of(x);
      const Type& t = *x.t;
      add(x.g, low::bin(BinOp::Ne, x.p, low::null_()), m);
      add(x.g, wf_expr(x.p), m);
      if (site_takes_cell(*x.site)) {
        add(x.g, in_block(x.p), m);
        const bool owns = x.site->atom->atom == AtomKind::Owns;
        add(x.g, low::bin(owns ? BinOp::Eq : BinOp::Gt, value_perm(x.p, t), owns ? write : none), m);
        add(x.g, low::bin(owns ? BinOp::Eq : BinOp::Gt, flag_perm(x.p, t), owns ? write : none), m);
        add(x.g, cell_flag(x.p, t), m);
        if (const E r = cell_range(x.p, t)) add(x.g, r, m);
      }
      add(x.g, low::bin(BinOp::Gt, live_perm(x.p), none), m);
    }
    // 2. the separation of atoms that must not name one cell: `swap(&x, &x)` fails here
    for (size_t i = 0; i < ss.size(); ++i)
      for (size_t j = i + 1; j < ss.size(); ++j) {
        if (!exclusive_pair(*ss[i].site, *ss[j].site)) continue;
        const std::string pl = quoted(ss[i].place, "the object the pointer points to");
        add(low::and_(ss[i].g, ss[j].g), low::bin(BinOp::Ne, ss[i].p, ss[j].p),
            "passes " + pl + " to `" + callee.c_name + "` twice, but one of the two places is to be owned and they may be the same object");
      }
    // 3. the case splits: on the liveness cells of every pair of atoms, on the cells of every pair of `reads`
    for (size_t i = 0; i < ss.size(); ++i)
      for (size_t j = i + 1; j < ss.size(); ++j)
        if (!same_expr(ss[i].p, ss[j].p)) split(low::bin(BinOp::Eq, live_cell(ss[i].p), live_cell(ss[j].p)));
    for (size_t i = 0; i < ss.size(); ++i)
      for (size_t j = i + 1; j < ss.size(); ++j) {
        const AtomSite& a = *ss[i].site;
        const AtomSite& b = *ss[j].site;
        if (a.atom->atom == AtomKind::Reads && b.atom->atom == AtomKind::Reads && cell_suffix(site_pointee(a)) == cell_suffix(site_pointee(b)) &&
            !same_expr(ss[i].p, ss[j].p))
          split(low::bin(BinOp::Eq, ss[i].p, ss[j].p));
      }
    // 3b. each pointer argument that feeds a cell atom, against each `reads` parameter of the caller of the same field: the caller may hold the
    // argument's cell as that parameter's, at a fraction, and Silicon takes a fraction computed from two chunks only when it knows which (S26).
    // Only the paths that reach a call branch (decision 0037 item 9, the re-audit's blow-up of entry splits).
    {
      const FnAtoms& caller = atoms_of(*fn_.f);
      std::vector<std::pair<E, E>> done;
      for (const S& x : ss) {
        if (!site_takes_cell(*x.site)) continue;
        for (int i = 0; i < caller.nreq; ++i) {
          const AtomSite& cs = caller.sites[static_cast<size_t>(i)];
          if (cs.atom->atom != AtomKind::Reads || cell_suffix(site_pointee(cs)) != cell_suffix(*x.t)) continue;
          const E q = low::var(formal_name(*fn_.f, site_param(cs)));
          if (same_expr(x.p, q)) continue;
          bool seen = false;
          for (const auto& d : done)
            if (same_expr(d.first, x.p) && same_expr(d.second, q)) seen = true;
          if (seen) continue;
          done.emplace_back(x.p, q);
          split(low::bin(BinOp::Eq, x.p, q));
        }
      }
    }
    // 4. the amounts
    for (const S& x : ss) {
      const Type& t = *x.t;
      E m = live_perm(x.p);
      if (x.site->atom->atom == AtomKind::Reads) m = min_perm(min_perm(value_perm(x.p, t), flag_perm(x.p, t)), m);
      amounts.push_back(low::perm_div(x.g ? low::ite(x.g, m, write) : m, static_cast<int>(n) + 1));
    }
    for (size_t i = 0; i < ss.size(); ++i) {
      const std::string m = text_of(ss[i]);
      add(nullptr, low::bin(BinOp::Lt, none, amounts[i]), m);
      add(nullptr, low::bin(BinOp::Lt, amounts[i], write), m);
    }
    // 5. the sums over each cell an atom takes a fraction of (decision 0037 item 8, audit round 1 S3)
    auto term = [&](const E& cond, const E& amount) { return cond ? low::ite(cond, amount, none) : amount; };
    for (size_t i = 0; i < ss.size(); ++i) {
      const std::string m = text_of(ss[i]);
      const Type& t = *ss[i].t;
      if (ss[i].site->atom->atom == AtomKind::Reads) {
        for (const bool flag : {false, true}) {
          E total = amounts[i];
          for (size_t j = 0; j < ss.size(); ++j) {
            if (j == i || ss[j].site->atom->atom != AtomKind::Reads || cell_suffix(site_pointee(*ss[j].site)) != cell_suffix(t)) continue;
            E same = same_expr(ss[i].p, ss[j].p) ? nullptr : low::bin(BinOp::Eq, ss[i].p, ss[j].p);
            total = low::bin(BinOp::Add, total, term(low::and_(ss[j].g, same), amounts[j]));
          }
          add(ss[i].g, low::bin(BinOp::Ge, flag ? flag_perm(ss[i].p, t) : value_perm(ss[i].p, t), total), m);
        }
      }
      E total = amounts[i];
      for (size_t j = 0; j < ss.size(); ++j) {
        if (j == i) continue;
        E same = same_expr(ss[i].p, ss[j].p) ? nullptr : low::bin(BinOp::Eq, live_cell(ss[i].p), live_cell(ss[j].p));
        total = low::bin(BinOp::Add, total, term(low::and_(ss[j].g, same), amounts[j]));
      }
      add(ss[i].g, low::bin(BinOp::Ge, live_perm(ss[i].p), total), m);
    }
    return amounts;
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
    // Decision 0026 item 1: it is the EXACT set of each `requires` clause's obligation (below).  The BASE set of a clause, which step 2
    // of `flush` falls back to, is the one of 0021 item 10: only what the clause's argument values depend on, and the floor.
    Deps pre = join(floor_, since(full, effect_from, obl_from));
    for (const LV& a : args) pre = join(pre, a.dep);
    // The callee's `requires`, over the argument values, are obligations at the call (0012 item 6).
    Env cenv;
    for (const LV& a : args) {
      cenv.formals.push_back(a.e);
      cenv.formal_dep.push_back(a.dep);
    }
    // The permissions the callee's atoms need (decision 0037 item 8): explicit, atomic, in order, before the value preconditions, all with the
    // exact set PRE(c) like them.
    std::vector<E> amounts = call_atoms(e, callee, args, cenv, pre, full);
    for (size_t k = 0; k < e.args.size(); ++k)
      if (e.args[k]->type.is_ptr()) ptr_use(*e.args[k], args[k], nullptr, {}, full);  // a pointer argument is a use (decision 0032 item 6)
    for (size_t ci = 0; ci < callee.requires_.size(); ++ci) {
      const Clause& c = callee.requires_[ci];
      Full scratch;  // the clause's own obligations belong to the callee's well-formedness method
      LV v;
      {
        CtxSave save(*this);
        set_ctx(callee, CMode::Call, false, static_cast<int>(ci));
        v = lower(*c.expr, cenv, nullptr, {}, scratch);
      }
      pure_full(scratch, *c.expr, "a precondition");
      if (has_atom(*c.expr) && always_true(v.e)) continue;  // nothing but atoms: asserted above
      low::Meta m = meta("precondition", e.loc, "the precondition of `" + callee.c_name + "` may not hold");
      m.has_note = true;
      m.note_loc = c.loc.known() ? c.loc : callee.loc;
      m.note_text = "this is the requirement of `" + callee.c_name + "`";
      // EXACT (decision 0026 item 1): C completes everything in PRE(c) before it makes the call, so the clause is asserted after all of it.
      // BASE (step 2): only the argument values the clause reads, and the floor (0021 item 10).
      obl(full, nullptr, {}, v.e, v.dep, std::move(m), &pre.ex);
      if (opt_.drop_exact_set && !exact_dropped_ && !full.obls.back().dep.base.empty()) {  // self-test hook (decision 0026)
        full.obls.back().dep.ex.clear();
        exact_dropped_ = true;
      }
    }
    CallItem item;
    item.method = method_name(callee);
    for (const LV& a : args) item.args.push_back(a.e);
    for (const E& a : amounts) item.args.push_back(a);  // the ghost amounts, after the user's arguments
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
      fn_.temps.push_back(low::LVar{item.target, ltype(callee.ret)});
      value = low::var(item.target);
      if (callee.ret.is_ptr()) result_splits(callee.ret, value, item.after);
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
    d.add(id);
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

  // A `?:` of pointer type counts: it is lowered as an `if` that assigns a temporary, so that each path holds one pointer (a Ref variable
  // with a plain value) and Silicon finds the permission of a cell through it: an `ite` term as the receiver of a field access does not
  // (S-probe of M4.5b1a: "insufficient permission to access" right after asserts that `perm` is `write`).
  static bool has_effect(const Expr& e) {
    if (e.kind == Expr::Kind::AssignE || e.kind == Expr::Kind::Comma || e.kind == Expr::Kind::Call) return true;
    if (e.kind == Expr::Kind::Cond && e.type.is_ptr()) return true;
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

  std::string new_temp(low::LType type) {
    const std::string t = "t__" + std::to_string(fn_.ntemp++);
    fn_.temps.push_back(low::LVar{t, type});
    return t;
  }

  // E3: a read of a local that the full expression writes is a snapshot, taken where C reads it; its `uninitialized` obligation
  // (decision 0019) is the read of the flag, which is part of the read of the local: it has the set of the read (the snapshot's own
  // set, its guard's and the comma floor's), and that set, not just the snapshot's, is what a later write of the local must follow
  // (decision 0025 item 1: a comma drops its left operand's calls, so a later write could otherwise be emitted before the check).
  LV snapshot(const Expr& e, const E& guard, const Deps& gdep, Full& full) {
    const std::string t = new_temp(ltype(e.var->type));
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
    if (flagged(*e.var)) {
      obl(full, guard, gdep, low::var(flag_name(*e.var)), slot,
          meta("uninitialized", e.loc, "`" + e.var->name + "` may be read here before it is given a value"));
      Obl& check = full.obls.back();
      check.flag_of = id;
      touch(full, id, opt_.drop_flag_access ? slot : check.dep);  // gdep + slot + floor_: every later access follows the check (hook: only the snapshot's)
    } else {
      touch(full, id, slot);
    }
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
    if (e.var->type.is_ptr()) ptr_use(*e.a, stored, guard, gdep, full);  // a pointer copy uses the value (decision 0032 item 6)
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
  // The set of the value (decision 0026 item 1): EXACT, its own set and the floor in force while `b` was lowered, which is what C
  // completes before the value exists; BASE (the fallback of step 2), its own set only, as before M4.4d.  Everything inside `b`
  // has the floor in both halves.
  LV comma(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full) {
    if (!in_full_) {
      internal_error(e.loc, "a comma outside a full expression reached lowering");
      return {low::int_lit("0"), {}};
    }
    const size_t effect_from = full.effects.size();
    const size_t obl_from = full.obls.size();
    {
      const LV left = lower(*e.a, env, guard, gdep, full);
      if (e.a->type.is_ptr()) ptr_use(*e.a, left, guard, gdep, full);  // a discarded pointer value is a use (decision 0032 item 6)
    }
    const Deps saved = floor_;
    // Only what the left operand itself depends on or contains: a call elsewhere in the enclosing expression is unsequenced with the
    // right operand (decision 0021: what the left operand created, fixed points included).
    floor_ = join(floor_, since(full, effect_from, obl_from));
    LV b = lower(*e.b, env, guard, gdep, full);
    // The value of the comma exists only after its left operand is complete, and everything in the enclosing floor completed before the
    // comma began: the floor in force while the right operand was lowered.  EXACT carries it (decision 0026 item 1; the guard of a lazy
    // operand, a divisor, an argument that uses the value then follow the left operand's calls).  BASE is the right operand's own set
    // (0020 E6 as M4.4c had it): the floor ends with the comma.
    b.dep.ex = join(b.dep.ex, floor_.ex);
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
      value.add(id);
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
    const std::string t = new_temp(low::LType::Bool);
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
    const std::string t = new_temp(ltype(e.type));
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

  // What the items of F that have exactly the set `set` are: `term_call` is the recursive call whose explicit termination check has it as
  // PRE (-1 if none), `ordinary` whether any other item (an obligation, a branch) has it.  A write or a snapshot is not in F.
  struct SetOrigin {
    bool ordinary = false;
    int term_call = -1;
  };
  static SetOrigin origin_of(const FlatFull& full, const Ids& set) {
    SetOrigin o;
    for (const FlatObl& x : full.obls) {
      if (x.dep != set) continue;
      if (x.term_of >= 0) { if (o.term_call < 0) o.term_call = x.term_of; }
      else o.ordinary = true;
    }
    for (const FlatEffect& fx : full.effects) {
      if (fx.kind != FlatEffect::Kind::Branch) continue;
      if (fx.fixed < 0) {
        if (fx.deps == set) o.ordinary = true;
      } else {
        Ids d = fx.deps;
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
  void report_conflict(const FlatFull& full, const Ids& a, const Ids& b) {
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

  // Where each item of a full expression is emitted: `obl[k]` for `full.obls[k]`, `eff[k]` for a movable `full.effects[k]` (a write, a
  // snapshot or a call-free branch; unused, -2, for a fixed point).  Slot s is right after pi[s], before pi[s + 1]; -1 is before every
  // fixed point.  Computed once by flush; emission and both order checks read this one vector, so a check sees the order emission uses.
  struct Placement {
    std::vector<int> obl;
    std::vector<int> eff;
  };

  // The family F of a full expression on one half of its sets (decision 0021 item 3, decision 0025 item 2): the sets of every obligation
  // (the explicit termination checks among them), of the call-free branches, and D(B) of every branch that holds a call; sorted by size,
  // without duplicates.  A write or a snapshot is not in F, and a call is a fixed point.
  static std::vector<Ids> family(const FlatFull& full) {
    std::vector<Ids> sets;
    for (const FlatObl& o : full.obls) sets.push_back(o.dep);
    for (const FlatEffect& fx : full.effects) {
      if (fx.kind != FlatEffect::Kind::Branch) continue;
      if (fx.fixed < 0) {
        sets.push_back(fx.deps);
      } else {
        Ids d = fx.deps;
        d.insert(fx.fixed);
        sets.push_back(std::move(d));
      }
    }
    std::sort(sets.begin(), sets.end(), [](const Ids& a, const Ids& b) { return a.size() != b.size() ? a.size() < b.size() : a < b; });
    sets.erase(std::unique(sets.begin(), sets.end()), sets.end());
    return sets;
  }

  // The index i of the first neighbours sets[i], sets[i + 1] with sets[i] not a subset of sets[i + 1], or kChain when F is a chain under inclusion.
  static constexpr size_t kChain = ~size_t{0};
  static size_t first_break(const std::vector<Ids>& sets) {
    for (size_t i = 0; i + 1 < sets.size(); ++i)
      if (!std::includes(sets[i + 1].begin(), sets[i + 1].end(), sets[i].begin(), sets[i].end())) return i;
    return kChain;
  }

  // The fail-closed check of decision 0026 item 3: every base half is contained in its exact half, for every item of the full expression
  // (the set of each obligation and effect, and PRE of each fixed point).  Step 2 relies on it (it is today's smaller set), and so does the
  // claim that a rejection after step 1 failed is the base family's.  A violation is an internal error (hook: `drop_exact_set`).
  void check_halves(const Full& full) {
    auto inside = [](const Deps& d) { return std::includes(d.ex.begin(), d.ex.end(), d.base.begin(), d.base.end()); };
    bool ok = true;
    for (const Obl& o : full.obls) ok = ok && inside(o.dep);
    for (const Effect& fx : full.effects) ok = ok && inside(fx.deps);
    for (const Deps& d : full.pre) ok = ok && inside(d);
    if (!ok) internal_error(SrcLoc{}, "the base half of a set of a full expression is not contained in its exact half");
  }

  // Write a full expression in two steps (decision 0026 item 2).  Step 1: project it onto the EXACT halves of its sets (what C completes
  // before each item) and test that F is a chain; if it is, those are its sets for everything that follows.  Step 2: otherwise project it
  // onto the BASE halves (the smaller sets of 0021 item 10, as M4.4c had them): if F is a chain, those are its sets (and the placement is
  // what M4.4c emitted, apart from the constant masks of 0026 part B).  Otherwise the program is refused with the base family's conflict (today's message and position; nothing is
  // reported by step 1).  Both choices are sound (a smaller set is safe, 0021 item 10); each full expression, and the inner one of each
  // branch with effects, decides on its own.  `LowerOptions::force_base_sets` skips step 1.
  void flush(const Full& full, std::vector<low::LStmt>& out) {
    check_halves(full);
    FlatFull flat = project(full, Half::Exact);
    std::vector<Ids> sets = family(flat);
    size_t broken = first_break(sets);
    const bool exact_chain = broken == kChain && !opt_.force_base_sets;  // self-test hook force_base_sets: skip step 1
    if (!exact_chain) {
      flat = project(full, Half::Base);
      sets = family(flat);
      broken = first_break(sets);
      if (broken != kChain) report_conflict(flat, sets[broken], sets[broken + 1]);
    }
    emit_full(flat, sets, broken == kChain, out);
  }

  // Write a full expression on the chosen sets (decision 0021 item 3; 0012 item 6, 0020 item 7 for the shape of the items; decision 0025
  // item 2).  `sets` is its family F, `chain` whether F is a chain under inclusion; the fixed points are then ordered pi: those of the
  // smallest set, then those each larger set adds, then the rest, each group in creation order.  Every set of F is a prefix of pi, and
  // an obligation or a call-free branch is emitted right after the |S|-th fixed point of its set S and before every other: a callee that
  // does not return hides only what C cannot evaluate before it returns.  A write or a snapshot cannot fail, so its set is not in F: it
  // is emitted right after the member of its set that comes last in pi (before every fixed point when its set is empty).  Items emitted
  // after the same fixed point keep creation order.  An item with an empty set comes first.  A fixed point is a hoisted call, or a
  // branch (an `if` for a guarded operand with effects).  A movable effect is `v := value` (and its flag) for a write, a snapshot of a
  // local that the expression writes, or an `if` for a call-free guarded operand.  Where the sets of F form no chain the program is
  // refused (`rejections`, by flush) and the rest is written in creation order, so that lowering finishes.
  void emit_full(FlatFull& full, const std::vector<Ids>& sets, bool chain, std::vector<low::LStmt>& out) {
    const int n = full.nfixed;
    // pi: the fixed points in the order they are emitted.
    std::vector<int> pi;
    std::vector<char> placed(static_cast<size_t>(n), 0);
    if (chain) {
      for (const Ids& s : sets)
        for (int id : s)
          if (placed[static_cast<size_t>(id)] == 0) { placed[static_cast<size_t>(id)] = 1; pi.push_back(id); }
    }
    for (int id = 0; id < n; ++id)
      if (placed[static_cast<size_t>(id)] == 0) pi.push_back(id);
    if (opt_.perturb_fixed_order && chain && pi.size() >= 2) std::swap(pi[0], pi[1]);  // self-test hook (0011 item 13)
    std::vector<int> pos(static_cast<size_t>(n), 0);  // the index of each fixed point in pi
    for (size_t k = 0; k < pi.size(); ++k) pos[static_cast<size_t>(pi[k])] = static_cast<int>(k);
    // The slot of an item of F: right after pi[|S| - 1] (-1: before every fixed point).  Of a write or a snapshot: right after the member
    // of its set that comes last in pi (-1 for an empty set).
    auto prefix_slot = [n](const Ids& s) { return std::min(static_cast<int>(s.size()) - 1, n - 1); };
    auto last_member = [&pos](const Ids& s) {
      int m = -1;
      for (int id : s) m = std::max(m, pos[static_cast<size_t>(id)]);
      return m;
    };
    Placement place;
    for (const FlatObl& o : full.obls) place.obl.push_back(prefix_slot(o.dep));
    bool perturbed = false;
    bool delayed = false;
    // Whether an item created after `w` has a set that contains w's set.
    auto depended_on = [&full](const FlatEffect& w) {
      auto contains = [&w](const Ids& big) { return std::includes(big.begin(), big.end(), w.deps.begin(), w.deps.end()); };
      for (const FlatObl& o : full.obls)
        if (o.seq > w.seq && contains(o.dep)) return true;
      for (const FlatEffect& x : full.effects)
        if (&x != &w && x.seq > w.seq && contains(x.deps)) return true;
      return false;
    };
    for (const FlatEffect& fx : full.effects) {
      int slot = -2;
      if (fx.fixed < 0) {
        slot = fx.kind == FlatEffect::Kind::Branch ? prefix_slot(fx.deps) : last_member(fx.deps);
        if (opt_.perturb_write_slot && !perturbed && fx.kind == FlatEffect::Kind::Write && !fx.deps.empty()) {  // self-test hook (0025)
          slot = -1;
          perturbed = true;
        }
        if (opt_.delay_write_slot && !delayed && fx.kind == FlatEffect::Kind::Write && !fx.deps.empty() && depended_on(fx)) {  // self-test hook (0025)
          slot = n - 1;
          delayed = true;
        }
      }
      place.eff.push_back(slot);
    }
    if (chain) {
      check_variable_order(full, pos, place);
      check_fixed_order(full, pi, pos, place);
    }
    for (int slot = -1; slot < n; ++slot) {
      // The items of this slot, obligations and movable effects, in creation order.
      std::vector<std::pair<size_t, std::pair<bool, size_t>>> items;  // (seq, (is obligation, index))
      for (size_t k = 0; k < full.obls.size(); ++k)
        if (place.obl[k] == slot) items.push_back({full.obls[k].seq, {true, k}});
      for (size_t k = 0; k < full.effects.size(); ++k)
        if (full.effects[k].fixed < 0 && place.eff[k] == slot) items.push_back({full.effects[k].seq, {false, k}});
      std::sort(items.begin(), items.end());
      for (const auto& it : items) {
        if (it.second.first) {
          FlatObl& o = full.obls[it.second.second];
          // The explicit termination check of a call (decision 0021 item 4) is written only when another fixed point, outside its set,
          // comes before the call: it then asserts what Silicon's own check at the call could not reach.  Not written: nothing is
          // counted (the self-test hook counts asserts) and no temporary is made.
          if (o.term_of >= 0 && pos[static_cast<size_t>(o.term_of)] <= static_cast<int>(o.dep.size())) continue;
          if (!o.stmts.empty()) {
            for (const low::LStmt& st : o.stmts) out.push_back(st);
            continue;
          }
          emit_assert(out, low::implies(o.guard, o.cond), o.meta);
          continue;
        }
        FlatEffect& fx = full.effects[it.second.second];
        switch (fx.kind) {
          case FlatEffect::Kind::Call: break;
          case FlatEffect::Kind::Write:
            assign(out, fx.name, fx.value);
            if (!fx.flag.empty()) assign(out, fx.flag, low::bool_lit(true));
            break;
          case FlatEffect::Kind::Snapshot: assign(out, fx.name, fx.value); break;
          case FlatEffect::Kind::Branch:
            for (low::LStmt& st : fx.stmts) out.push_back(std::move(st));
            break;
        }
      }
      if (slot + 1 < n) {
        FlatEffect& fx = full.effects[full.fixed_effect[static_cast<size_t>(pi[static_cast<size_t>(slot + 1)])]];
        if (fx.kind == FlatEffect::Kind::Branch) {
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
        for (const low::LStmt& st : c.after) out.push_back(st);
      }
    }
  }

  // The fail-closed check of the main property of decision 0021 (like check_variable_order): the fixed points emitted before an
  // obligation or a call-free branch are exactly its set; every fixed point follows its PRE; and a branch with a call, and a recursive
  // call whose explicit termination check is not written, come right after their PRE.  A violation would be a wrong Viper program (a
  // check hidden by a call that does not return), so it is an internal error.  For a write and a snapshot (decision 0025 item 2): every
  // member of its set is emitted before it, and every item created later whose set (for a fixed point: its PRE) contains that set is
  // emitted after it.  `place` is the placement emission uses.
  void check_fixed_order(const FlatFull& full, const std::vector<int>& pi, const std::vector<int>& pos, const Placement& place) {
    auto prefix_is = [&pi](const Ids& s) {
      if (s.size() > pi.size()) return false;
      for (size_t k = 0; k < s.size(); ++k)
        if (s.count(pi[k]) == 0) return false;
      return true;
    };
    constexpr const char* kSet = "the fixed points emitted before an item of a full expression are not exactly the set it depends on";
    for (const FlatObl& o : full.obls)
      if (!prefix_is(o.dep)) { internal_error(SrcLoc{}, kSet); return; }
    for (const FlatEffect& fx : full.effects)
      if (fx.fixed < 0 && fx.kind == FlatEffect::Kind::Branch && !prefix_is(fx.deps)) { internal_error(SrcLoc{}, kSet); return; }
    for (int id = 0; id < full.nfixed; ++id) {
      const Ids& pre = full.pre[static_cast<size_t>(id)];
      for (int p : pre)
        if (pos[static_cast<size_t>(p)] >= pos[static_cast<size_t>(id)]) {
          internal_error(SrcLoc{}, "a fixed point of a full expression is emitted before something it follows");
          return;
        }
      const FlatEffect& fx = full.effects[full.fixed_effect[static_cast<size_t>(id)]];
      if (fx.kind == FlatEffect::Kind::Branch && !(pos[static_cast<size_t>(id)] == static_cast<int>(pre.size()) && prefix_is(pre))) {
        internal_error(SrcLoc{}, "a branch with a call is not emitted right after what its condition depends on");
        return;
      }
    }
    for (const FlatObl& o : full.obls)  // a recursive call without a written explicit check: right after its PRE
      if (o.term_of >= 0 && pos[static_cast<size_t>(o.term_of)] == static_cast<int>(o.dep.size()) && !prefix_is(o.dep)) {
        internal_error(SrcLoc{}, "a recursive call is emitted without its explicit termination check and not right after what it depends on");
        return;
      }
    // Writes and snapshots.  The emission order of an item: its slot, then movable items (0) before the fixed point that follows the slot
    // (1), then creation order; the fixed point at index p of pi follows the items of slot p - 1.
    using Order = std::tuple<int, int, size_t>;
    auto movable = [](int slot, size_t seq) { return Order{slot, 0, seq}; };
    auto fixed_at = [](int p) { return Order{p - 1, 1, 0}; };
    auto contains = [](const Ids& big, const Ids& small) { return std::includes(big.begin(), big.end(), small.begin(), small.end()); };
    for (size_t i = 0; i < full.effects.size(); ++i) {
      const FlatEffect& w = full.effects[i];
      if (w.fixed >= 0 || w.kind == FlatEffect::Kind::Branch) continue;
      const int slot = place.eff[i];
      for (int id : w.deps)
        if (pos[static_cast<size_t>(id)] > slot) {
          internal_error(SrcLoc{}, "a write or a snapshot of a full expression is emitted before a fixed point of its set");
          return;
        }
      const Order mine = movable(slot, w.seq);
      constexpr const char* kLater = "a write or a snapshot of a full expression is emitted after an item that follows it";
      for (size_t k = 0; k < full.obls.size(); ++k) {
        const FlatObl& o = full.obls[k];
        if (o.seq > w.seq && contains(o.dep, w.deps) && !(movable(place.obl[k], o.seq) > mine)) { internal_error(SrcLoc{}, kLater); return; }
      }
      for (size_t k = 0; k < full.effects.size(); ++k) {
        const FlatEffect& x = full.effects[k];
        if (k == i || x.seq <= w.seq || !contains(x.deps, w.deps)) continue;
        const Order theirs = x.fixed >= 0 ? fixed_at(pos[static_cast<size_t>(x.fixed)]) : movable(place.eff[k], x.seq);
        if (!(theirs > mine)) { internal_error(SrcLoc{}, kLater); return; }
      }
    }
  }

  // The fail-closed check of the order: for every local, its snapshots and writes, its flag reads (an obligation that reads the
  // `uninitialized` flag, decision 0025 item 1) and the branches that touch it (a branch counts as the accesses of its inner effects) are
  // emitted in the order they were created, except that two reads may swap.  A hoisted write that overtook an earlier read or write of the
  // same local would change a value, so it is an internal error rather than a wrong Viper program.  The key of an item is its place in the
  // emitted order: a movable item in slot s comes after pi[s] (key 2s + 1) and before pi[s + 1] (key 2s + 3), so its key is 2s + 2, the
  // same as 2|S| for an item of F; the fixed point at index i has the key 2i + 1.
  void check_variable_order(const FlatFull& full, const std::vector<int>& pos, const Placement& place) {
    struct Access {
      int id;
      bool write;
      int key;
      size_t seq;
    };
    std::vector<Access> all;
    for (size_t i = 0; i < full.effects.size(); ++i) {
      const FlatEffect& fx = full.effects[i];
      if (fx.reads.empty() && fx.writes.empty()) continue;
      const int key = fx.fixed >= 0 ? 2 * pos[static_cast<size_t>(fx.fixed)] + 1 : 2 * (place.eff[i] + 1);
      for (int id : fx.reads) all.push_back({id, false, key, fx.seq});
      for (int id : fx.writes) all.push_back({id, true, key, fx.seq});
    }
    for (size_t k = 0; k < full.obls.size(); ++k)
      if (full.obls[k].flag_of >= 0) all.push_back({full.obls[k].flag_of, false, 2 * (place.obl[k] + 1), full.obls[k].seq});
    std::stable_sort(all.begin(), all.end(), [](const Access& x, const Access& y) { return x.seq < y.seq; });  // creation order
    if (opt_.perturb_access_order && !all.empty()) {  // self-test hook (0011 item 13): the first access moves behind everything
      const size_t first = all.front().seq;
      for (Access& a : all)
        if (a.seq == first) a.key += 1000;
    }
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

  // Several expressions in ONE full expression (a store's address and value, decision 0032 item 9); the others are one expression.
  // `copy_of`, when given, is the first expression, whose pointer value is being copied: its liveness check joins the full expression.
  std::vector<LV> full_exprs(const std::vector<const Expr*>& es, std::vector<low::LStmt>& out, const Expr* copy_of = nullptr,
                             bool share_first = false) {
    Full full;
    const std::set<int> saved_written = std::move(written_);
    const bool saved_in_full = in_full_;
    written_.clear();
    for (const Expr* e : es) collect_writes(*e, written_);
    in_full_ = true;
    std::vector<LV> vs;
    for (const Expr* e : es) {
      vs.push_back(lower(*e, body_env(), nullptr, {}, full));
      if (share_first && vs.size() == 1) shared_[e] = vs.front();  // the later expressions read this node's value, not a second evaluation
    }
    if (share_first) shared_.erase(es.front());
    if (copy_of != nullptr) ptr_use(*copy_of, vs.front(), nullptr, {}, full);
    in_full_ = saved_in_full;
    written_ = saved_written;
    flush(full, out);
    return vs;
  }

  LV full_expr(const Expr& e, std::vector<low::LStmt>& out, bool pointer_copy = false) {
    return full_exprs({&e}, out, pointer_copy ? &e : nullptr).front();
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
        if (s.var->address_taken) {  // a cell: written by name (decision 0032 item 6); a Decl without an initializer writes nothing
          if (s.expr == nullptr) break;
          LV v = full_expr(*s.expr, out);
          write_cell(out, *s.var, v.e, s.loc);
          break;
        }
        if (s.expr == nullptr) {  // a Decl without an initializer (check_program allows no other)
          reset(out, *s.var);
          break;
        }
        LV v = full_expr(*s.expr, out, s.var->type.is_ptr());  // a pointer copy is a use of the pointer's value
        assign(out, var_name(*s.var), v.e);
        if (flagged(*s.var)) assign(out, flag_name(*s.var), low::bool_lit(true));
        break;
      }
      case Stmt::Kind::ExprStmt:
        // Only its obligations and calls matter; a pointer value that is evaluated and discarded is still a use of it (decision 0032 item 6).
        full_expr(*s.expr, out, s.expr->type.is_ptr());
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
      case Stmt::Kind::Store: store(s, out); break;
      case Stmt::Kind::Return: ret(s, out); break;
      case Stmt::Kind::Check: {
        LV v = full_expr(*s.expr, out);
        emit_assert(out, v.e, meta("check", s.loc, s.from_assert ? "this `assert` may not hold" : "this `check` may not hold"));
        break;
      }
      case Stmt::Kind::Block: {
        // The cells declared directly in this block are allocated when it is entered (decision 0032 item 4).
        const std::vector<const Var*> cells = cells_of(s);
        for (const Var* c : cells) alloc_cell(out, *c);
        if (!cells.empty()) fn_.scopes.push_back(Fn::Scope{cells, s.end_loc});
        // A goto past a declaration lands in this block with the variable in scope: it starts indeterminate at the block's
        // entry (decision 0019 item 4).
        for (const Stmt* c : s.stmts)
          if (c->kind == Stmt::Kind::Decl && c->expr == nullptr && !c->var->address_taken) reset(out, *c->var);
        for (const Stmt* c : s.stmts) stmt(*c, out);
        if (!cells.empty()) {  // fall-through: reported at the closing brace (decision 0032 item 8)
          scope_exit(out, cells, s.end_loc);
          fn_.scopes.pop_back();
        }
        break;
      }
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
    fn_.temps.push_back(low::LVar{t, low::LType::Int});
    assign(out, t, v.e);
    // The cells of the body are allocated before the dispatch: a case label may jump past their declarations (decision 0032 item 4).
    const std::vector<const Var*> body_cells = cells_of(*s.body);
    for (const Var* c : body_cells) alloc_cell(out, *c);
    // A case label may jump past a declaration of the body without an initializer: reset them before the dispatch (0019 item 4).
    for (const Stmt* c : s.body->stmts)
      if (c->kind == Stmt::Kind::Decl && c->expr == nullptr && !c->var->address_taken) reset(out, *c->var);
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
    if (!body_cells.empty()) {
      if (breaks.count(s.jump_id) != 0) internal_error(s.loc, "a `break` out of a switch body that declares a cell reached lowering (decision 0032 item 10)");
      fn_.scopes.push_back(Fn::Scope{body_cells, s.body->end_loc});
    }
    const bool need_break_label = !has_default || breaks.count(s.jump_id) != 0;
    jump(out, has_default ? "l_default__" + n : "l_brk__" + n);
    index = 0;
    for (const Stmt* c : s.body->stmts) {
      if (c->kind == Stmt::Kind::Case) label(out, c->expr == nullptr ? "l_default__" + n : "l_case__" + n + "_" + std::to_string(index));
      else stmt(*c, out);
      ++index;
    }
    if (need_break_label) label(out, "l_brk__" + n);  // before the scope exit: the dispatch of an unmatched value passes through it
    if (!body_cells.empty()) {
      scope_exit(out, body_cells, s.body->end_loc);
      fn_.scopes.pop_back();
    }
  }

  void ret(const Stmt& s, std::vector<low::LStmt>& out) {
    if (s.expr != nullptr) {
      LV v = full_expr(*s.expr, out, s.expr->type.is_ptr());  // a returned pointer is a use of its value (decision 0037 item 3)
      assign(out, kResult, v.e);
    }
    // Leaving every open block that holds cells: innermost first (decision 0032 item 8), after the value and before the postconditions.
    for (size_t k = fn_.scopes.size(); k-- > 0;) scope_exit(out, fn_.scopes[k].cells, s.loc);
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

  static E amount_bounds(const std::string& name) {
    return low::bin(low::BinOp::And, low::bin(low::BinOp::Lt, low::no_perm(), low::var(name)),
                    low::bin(low::BinOp::Lt, low::var(name), low::write_perm()));
  }

  // A pointer the function obtained from elsewhere (a call's result, a parameter without an atom of its own) is some cell the function holds, but
  // Silicon finds the permission through a Ref only when it knows which one: a no-op `if (r == X) { }` for each cell X in scope and each pointer
  // parameter with an atom, of the same field, branches on it (decision 0037 item 3, the owner's choice after the exit 3 of `*pick(...)`).
  void result_splits(const Type& ptr_type, const E& r, std::vector<low::LStmt>& out) {
    if (ptr_type.pointee == nullptr || ptr_type.pointee->is_void()) return;
    const std::string sfx = cell_suffix(*ptr_type.pointee);
    auto split = [&](const E& x) {
      low::LStmt st;
      st.kind = low::LStmt::Kind::CaseSplit;
      st.expr = low::bin(low::BinOp::Eq, r, x);
      out.push_back(std::move(st));
    };
    for (const Fn::Scope& sc : fn_.scopes)
      for (const Var* cell : sc.cells)
        if (cell_suffix(cell->type) == sfx) split(low::var(var_name(*cell)));
    const FnAtoms& fa = atoms_of(*fn_.f);
    std::set<int> done;
    for (int i = 0; i < fa.nreq; ++i) {
      const AtomSite& site = fa.sites[static_cast<size_t>(i)];
      if (!site_takes_cell(site) || cell_suffix(site_pointee(site)) != sfx || !done.insert(site_param(site)).second) continue;
      split(low::var(formal_name(*fn_.f, site_param(site))));
    }
  }

  // The entry of a method with atoms (decision 0037 item 9): the case splits on the liveness cells of every pair of atoms of the `requires`, and
  // the separation of every pair of `owns` atoms of one field under both guards (an `internal` row: provable from the `requires`).
  void entry_checks(const Function& f, std::vector<low::LStmt>& out) {
    const FnAtoms& fa = atoms_of(f);
    const size_t n = static_cast<size_t>(fa.nreq);
    if (n == 0) return;
    const Env env = contract_env();
    std::vector<E> g;
    for (size_t i = 0; i < n; ++i) g.push_back(site_guard(fa.sites[i], env));
    auto p = [&](size_t i) { return env.formals[static_cast<size_t>(site_param(fa.sites[i]))]; };
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + 1; j < n; ++j) {
        if (site_param(fa.sites[i]) == site_param(fa.sites[j])) continue;
        low::LStmt st;
        st.kind = low::LStmt::Kind::CaseSplit;
        st.expr = low::bin(low::BinOp::Eq, live_cell(p(i)), live_cell(p(j)));
        out.push_back(std::move(st));
      }
    // A pointer parameter without an atom of its own may be the place an atom names: split on each one of the same field.
    std::set<int> with_atom;
    for (size_t i = 0; i < n; ++i) with_atom.insert(site_param(fa.sites[i]));
    for (size_t k = 0; k < f.params.size(); ++k) {
      const Type& t = f.params[k].type;
      if (!t.is_ptr() || with_atom.count(static_cast<int>(k)) != 0 || t.pointee == nullptr || t.pointee->is_void()) continue;
      std::set<int> seen;
      for (size_t i = 0; i < n; ++i) {
        const AtomSite& a = fa.sites[i];
        if (!site_takes_cell(a) || cell_suffix(site_pointee(a)) != cell_suffix(*t.pointee) || !seen.insert(site_param(a)).second) continue;
        low::LStmt st;
        st.kind = low::LStmt::Kind::CaseSplit;
        st.expr = low::bin(low::BinOp::Eq, env.formals[k], p(i));
        out.push_back(std::move(st));
      }
    }
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + 1; j < n; ++j) {
        const AtomSite& a = fa.sites[i];
        const AtomSite& b = fa.sites[j];
        if (a.atom->atom != AtomKind::Owns || b.atom->atom != AtomKind::Owns || site_param(a) == site_param(b)) continue;
        if (cell_suffix(site_pointee(a)) != cell_suffix(site_pointee(b))) continue;
        emit_assert(out, low::implies(low::and_(g[i], g[j]), low::bin(low::BinOp::Ne, p(i), p(j))),
                    meta("internal", fn_.f->loc, "internal check: two places the contract of `" + fn_.f->c_name + "` owns are different"));
      }
  }

  // The `ensures` atoms at a return (decision 0037 item 10): the separation of the atoms that must not share a cell, then each atom under its
  // guard, part by part, with the sums of item 6; the facts about WF and the block's length as internal rows.
  void atom_returns(const SrcLoc& at, std::vector<low::LStmt>& out, const Env& env) {
    using low::BinOp;
    const Function& f = *fn_.f;
    const FnAtoms& fa = atoms_of(f);
    std::vector<size_t> ens;
    for (size_t i = 0; i < fa.sites.size(); ++i)
      if (fa.sites[i].in_ensures) ens.push_back(i);
    if (ens.empty()) return;
    std::vector<E> g, p;
    for (size_t i : ens) {
      g.push_back(site_guard(fa.sites[i], env));
      p.push_back(env.formals[static_cast<size_t>(site_param(fa.sites[i]))]);
    }
    auto name = [&](size_t k) {  // the user's spelling of the place, for messages
      const std::string& c = fa.sites[ens[k]].atom->a->c_text;
      return c.empty() ? std::string() : "*" + c;
    };
    auto perm_meta = [&](const std::string& message, size_t k) {
      low::Meta m = meta("permission", at, message);
      const Clause& cl = f.ensures_[static_cast<size_t>(fa.sites[ens[k]].clause)];
      if (cl.loc.known()) {
        m.has_note = true;
        m.note_loc = cl.loc;
        m.note_text = "this is the postcondition";
      }
      return m;
    };
    auto internal = [&](const std::string& what) { return meta("internal", at, "internal check: " + what); };
    const E none = low::no_perm(), write = low::write_perm();
    for (size_t a = 0; a < ens.size(); ++a)
      for (size_t b = a + 1; b < ens.size(); ++b) {
        if (!exclusive_pair(fa.sites[ens[a]], fa.sites[ens[b]])) continue;
        emit_assert(out, low::implies(low::and_(g[a], g[b]), low::bin(BinOp::Ne, p[a], p[b])),
                    perm_meta("returns " + quoted(name(a), "the object the pointer points to") + " and " + quoted(name(b), "the object the pointer points to") +
                                  " as owned places, but they may be the same object",
                              a));
      }
    auto term = [&](const E& cond, const E& amount) { return cond ? low::ite(cond, amount, none) : amount; };
    for (size_t k = 0; k < ens.size(); ++k) {
      const AtomSite& site = fa.sites[ens[k]];
      const Type& t = site_pointee(site);
      const E amt = low::var(amount_name(site.atom->amount));
      const bool owns = site.atom->atom == AtomKind::Owns, reads = site.atom->atom == AtomKind::Reads;
      const std::string pl = quoted(name(k), "the object the pointer points to");
      const std::string msg = owns ? "returns without owning " + pl + ", which its contract promises"
                              : reads ? "returns without the permission to read " + pl + ", which it borrowed"
                                      : "returns without " + quoted(site.atom->a->c_text, "the pointer") + " pointing to a live object, which its contract promises";
      auto add = [&](E cond, const low::Meta& m) { emit_assert(out, low::implies(g[k], std::move(cond)), m); };
      add(low::bin(BinOp::Ne, p[k], low::null_()), perm_meta(msg, k));
      add(wf_expr(p[k]), internal("WF of a pointer parameter"));
      if (site_takes_cell(site)) {
        add(in_block(p[k]), internal("a pointer parameter points into its block"));
        if (owns) {
          add(low::bin(BinOp::Eq, value_perm(p[k], t), write), perm_meta(msg, k));
          add(low::bin(BinOp::Eq, flag_perm(p[k], t), write), perm_meta(msg, k));
        } else {
          for (const bool flag : {false, true}) {
            E total = amt;
            for (size_t j = 0; j < ens.size(); ++j) {
              if (j == k || fa.sites[ens[j]].atom->atom != AtomKind::Reads || cell_suffix(site_pointee(fa.sites[ens[j]])) != cell_suffix(t)) continue;
              E same = same_expr(p[k], p[j]) ? nullptr : low::bin(BinOp::Eq, p[k], p[j]);
              total = low::bin(BinOp::Add, total, term(low::and_(g[j], same), low::var(amount_name(fa.sites[ens[j]].atom->amount))));
            }
            add(low::bin(BinOp::Ge, flag ? flag_perm(p[k], t) : value_perm(p[k], t), total), perm_meta(msg, k));
          }
        }
        add(cell_flag(p[k], t), perm_meta(msg, k));
        if (const E r = cell_range(p[k], t)) add(r, perm_meta(msg, k));
      }
      E total = amt;
      for (size_t j = 0; j < ens.size(); ++j) {
        if (j == k) continue;
        E same = same_expr(p[k], p[j]) ? nullptr : low::bin(BinOp::Eq, live_cell(p[k]), live_cell(p[j]));
        total = low::bin(BinOp::Add, total, term(low::and_(g[j], same), low::var(amount_name(fa.sites[ens[j]].atom->amount))));
      }
      add(low::bin(BinOp::Ge, live_perm(p[k]), total), perm_meta(msg, k));
    }
  }

  // Assert each user `ensures` at the point where the function returns (0012 item 9), after the permissions it promises (decision 0037 item 10).
  void postconditions(const SrcLoc& at, std::vector<low::LStmt>& out) {
    const Function& f = *fn_.f;
    const Env env = contract_env();
    atom_returns(at, out, env);
    if (f.ret.is_ptr()) emit_assert(out, wf_expr(env.result), meta("internal", at, "internal check: the pointer a function returns is well-formed"));
    for (size_t ci = 0; ci < f.ensures_.size(); ++ci) {
      const Clause& c = f.ensures_[ci];
      Full scratch;
      LV v;
      {
        CtxSave save(*this);
        set_ctx(f, CMode::Ret, true, static_cast<int>(ci));
        v = lower(*c.expr, env, nullptr, {}, scratch);
      }
      pure_full(scratch, *c.expr, "a postcondition");
      if (has_atom(*c.expr) && always_true(v.e)) continue;  // nothing but atoms: asserted above
      low::Meta m = meta("postcondition", at, "the postcondition of `" + f.c_name + "` may not hold");
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
      fn_.snapshots.push_back(low::LVar{snap, low::LType::Bool});
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
    // A static assertion (decision 0024 item 2) is checked like a named constant whose value is true: the condition's obligations,
    // then the guard that it holds (skipped for the bare literal `true`).
    const bool assertion = c.kind == NamedConst::Kind::StaticAssertion;
    const char* what = assertion ? "the condition of a static assertion" : "the initializer of a named constant";
    if (!closed_expr(*c.init, what)) return;
    Full full;
    LV v = lower(*c.init, body_env(), nullptr, {}, full);
    if (!closed_result(full, *c.init, what)) return;
    for (Obl& o : full.obls) emit_assert(out, low::implies(o.guard, o.cond), o.meta);
    if (bare_const_equals(*c.init, c.value)) return;
    internal_guard(out, v.e, c.type, c.value, c.loc,
                   assertion ? std::string("internal check: static assertion condition as written differs from the front end's (it holds)")
                             : "internal check: the value of `" + c.c_name + "` as written differs from the front end's");
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
    const FnAtoms& fa = atoms_of(f);
    for (size_t k = 0; k < f.params.size(); ++k)
      m.formals.push_back(low::LVar{formal_name(f, static_cast<int>(k)), ltype(f.params[k].type)});
    for (int k = 0; k < fa.nreq; ++k) m.formals.push_back(low::LVar{amount_name(k), low::LType::Perm});  // the ghost amounts (decision 0037 item 6)
    if (!f.ret.is_void()) m.results.push_back(low::LVar{kResult, ltype(f.ret)});

    // requires: the parameters' type ranges, the ghost amounts' bounds (before every clause that names one), WF of the pointers, then the
    // user's (0012 item 1; decision 0037 items 2 and 6).
    for (size_t k = 0; k < f.params.size(); ++k) {
      if (!f.params[k].type.is_int()) continue;
      m.requires_.push_back(clause_checks(range_check(low::var(formal_name(f, static_cast<int>(k))), *f.params[k].type.int_type),
                                          f.loc, false, f.c_name));
    }
    for (int k = 0; k < fa.nreq; ++k) m.requires_.push_back(clause_checks(amount_bounds(amount_name(k)), f.loc, false, f.c_name));
    for (size_t k = 0; k < f.params.size(); ++k) {
      if (!f.params[k].type.is_ptr()) continue;
      m.requires_.push_back(clause_checks(wf_expr(low::var(formal_name(f, static_cast<int>(k)))), f.loc, false, f.c_name));
    }
    const Env cenv = contract_env();
    for (size_t ci = 0; ci < f.requires_.size(); ++ci) {
      const Clause& c = f.requires_[ci];
      Full scratch;
      LV v;
      {
        CtxSave save(*this);
        set_ctx(f, CMode::Method, false, static_cast<int>(ci));
        v = lower(*c.expr, cenv, nullptr, {}, scratch);
      }
      pure_full(scratch, *c.expr, "a precondition");
      m.requires_.push_back(clause_checks(v.e, c.loc.known() ? c.loc : f.loc, false, f.c_name));
    }
    if (f.ret.is_int())
      m.ensures_.push_back(clause_checks(range_check(low::var(kResult), *f.ret.int_type), f.loc, true, f.c_name));
    if (f.ret.is_ptr()) m.ensures_.push_back(clause_checks(wf_expr(low::var(kResult)), f.loc, true, f.c_name));
    for (size_t ci = 0; ci < f.ensures_.size(); ++ci) {
      const Clause& c = f.ensures_[ci];
      Full scratch;
      LV v;
      {
        CtxSave save(*this);
        set_ctx(f, CMode::Method, true, static_cast<int>(ci));
        v = lower(*c.expr, cenv, nullptr, {}, scratch);
      }
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
    // The parameters' local copies (0012 item 2); an address-taken parameter is copied into a fresh one-slot block (decision 0037 item 4), whose
    // scope is the body's.
    std::vector<const Var*> param_cells;
    for (size_t k = 0; k < f.params.size(); ++k) {
      const Var& copy = *f.param_var(static_cast<int>(k));
      if (copy.address_taken) {
        param_cells.push_back(&copy);
        alloc_cell(m.body, copy);
        write_fields(m.body, var_name(copy), copy.type, low::var(formal_name(f, static_cast<int>(k))));
      } else {
        assign(m.body, var_name(copy), low::var(formal_name(f, static_cast<int>(k))));
      }
    }
    if (!param_cells.empty()) fn_.scopes.push_back(Fn::Scope{param_cells, f.end_loc});
    entry_checks(f, m.body);
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
      if (!param_cells.empty()) scope_exit(m.body, param_cells, f.end_loc);  // fall-through (decision 0037 item 4)
      postconditions(f.end_loc, m.body);
    } else {
      emit_assert(m.body, low::bool_lit(false),
                  meta("missing-return", f.end_loc, "control may reach the end of `" + f.c_name + "` without returning a value"));
    }
    low::LStmt end;
    end.kind = low::LStmt::Kind::Label;
    end.name = kEndLabel;
    m.body.push_back(std::move(end));
    for (const auto& v : f.locals()) {
      // A cell's Viper local holds the Ref of its one-slot block (decision 0032 item 4).
      m.locals.push_back(low::LVar{fn_.var_names[static_cast<size_t>(v->id)], v->address_taken ? low::LType::Ref : ltype(v->type)});
      if (v->address_taken || v->type.is_ptr()) out_.memory = true;
    }
    for (int id : fn_.flagged) m.locals.push_back(low::LVar{flag_name(*f.locals()[static_cast<size_t>(id)]), low::LType::Bool});
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
    const FnAtoms& fa = atoms_of(f);
    wf_ghosts_.clear();
    low::LMethod m;
    m.name = fn_.method + "__wf";
    m.comment = "well-formedness of the contract of C/C++ function `" + f.c_name + "`";
    m.has_body = true;
    prologue(m.body, &f);  // the same checks as the function's own method (the driver removes identical reports); a function whose
                           // `requires` is unsatisfiable never reaches its own prologue
    const Env env = contract_env();
    E prior;
    auto go = [&](const Clause& c, bool ens, int idx) {
      Full full;
      LV v;
      {
        CtxSave save(*this);
        set_ctx(f, CMode::Wf, ens, idx);
        v = lower(*c.expr, env, nullptr, {}, full);
      }
      pure_full(full, *c.expr, "a contract clause");
      for (Obl& o : full.obls) emit_assert(m.body, low::implies(low::and_(prior, o.guard), o.cond), o.meta);
      prior = low::and_(prior, v.e);
    };
    for (size_t k = 0; k < f.requires_.size(); ++k) go(f.requires_[k], false, static_cast<int>(k));
    E requires_only = prior;
    for (size_t k = 0; k < f.ensures_.size(); ++k) go(f.ensures_[k], true, static_cast<int>(k));
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
    // The formals and what the method assumes: the parameters, the ghost amounts, the result, the ghost values of the reads of the contract.
    const std::string what = f.c_name + " (well-formedness)";
    for (size_t k = 0; k < f.params.size(); ++k)
      m.formals.push_back(low::LVar{formal_name(f, static_cast<int>(k)), ltype(f.params[k].type)});
    for (int k = 0; k < fa.nreq; ++k) m.formals.push_back(low::LVar{amount_name(k), low::LType::Perm});
    if (!f.ret.is_void()) m.formals.push_back(low::LVar{kResult, ltype(f.ret)});
    for (const auto& [key, t] : wf_ghosts_) m.formals.push_back(low::LVar{ghost_name(key.first, key.second), t.is_bool() ? low::LType::Bool : low::LType::Int});
    for (int k = 0; k < fa.nreq; ++k) m.requires_.push_back(clause_checks(amount_bounds(amount_name(k)), f.loc, false, what));
    for (size_t k = 0; k < f.params.size(); ++k) {
      if (f.params[k].type.is_ptr()) m.requires_.push_back(clause_checks(wf_expr(low::var(formal_name(f, static_cast<int>(k)))), f.loc, false, what));
      if (!f.params[k].type.is_int()) continue;
      m.requires_.push_back(clause_checks(range_check(low::var(formal_name(f, static_cast<int>(k))), *f.params[k].type.int_type), f.loc, false, what));
    }
    if (f.ret.is_int()) m.requires_.push_back(clause_checks(range_check(low::var(kResult), *f.ret.int_type), f.loc, false, what));
    if (f.ret.is_ptr()) m.requires_.push_back(clause_checks(wf_expr(low::var(kResult)), f.loc, false, what));
    for (const auto& [key, t] : wf_ghosts_)
      if (!t.is_bool())
        m.requires_.push_back(clause_checks(range_check(low::var(ghost_name(key.first, key.second)), *t.int_type), f.loc, false, what));
    // Facts Silicon derives that the ghost values must not lack (decision 0037 item 7): two places that are one cell have one value in a state,
    // and a cell a `reads` atom borrows, returned by a `reads` atom, has the value it had.
    for (auto a = wf_ghosts_.begin(); a != wf_ghosts_.end(); ++a)
      for (auto b = std::next(a); b != wf_ghosts_.end(); ++b) {
        if (a->first.second != b->first.second || cell_suffix(a->second) != cell_suffix(b->second)) continue;
        const E pa = env.formals[static_cast<size_t>(a->first.first)], pb = env.formals[static_cast<size_t>(b->first.first)];
        m.requires_.push_back(clause_checks(
            low::implies(low::bin(low::BinOp::Eq, pa, pb),
                         low::bin(low::BinOp::Eq, low::var(ghost_name(a->first.first, a->first.second)), low::var(ghost_name(b->first.first, b->first.second)))),
            f.loc, false, what));
      }
    for (const auto& [key, t] : wf_ghosts_) {
      if (!key.second || wf_ghosts_.count({key.first, false}) == 0) continue;
      E gr, ge;
      bool have_r = false, have_e = false;
      for (const AtomSite& s : fa.sites) {
        if (s.atom->atom != AtomKind::Reads || site_param(s) != key.first) continue;
        if (!s.in_ensures && !have_r) { gr = site_guard(s, env); have_r = true; }
        if (s.in_ensures && !have_e) { ge = site_guard(s, env); have_e = true; }
      }
      if (!have_r || !have_e) continue;
      m.requires_.push_back(clause_checks(
          low::implies(low::and_(gr, ge), low::bin(low::BinOp::Eq, low::var(ghost_name(key.first, true)), low::var(ghost_name(key.first, false)))),
          f.loc, false, what));
    }
    out_.methods.push_back(std::move(m));
  }

  // The contract being lowered (decision 0037): the mode, the function whose atoms the sites are, whether it is an `ensures`, whether under `old`,
  // and how many atoms (of `contract_atoms(*cf_)`) the lowering has passed so far.
  CMode cmode_ = CMode::None;
  const Function* cf_ = nullptr;
  bool cens_ = false;
  bool cold_ = false;
  size_t cseen_ = 0;
  std::map<const Function*, FnAtoms> atoms_;
  std::map<std::pair<int, bool>, Type> wf_ghosts_;  // the ghost value formals of the `__wf` method being built: (parameter, post-state) -> pointee type
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
  bool exact_dropped_ = false;  // the self-test hook `drop_exact_set` has fired
  std::set<int> written_;
  Deps floor_;
  std::map<const Expr*, LV> shared_;
  std::map<int, std::string> user_names_;  // the user's name of each label of the current function, by id
  std::set<int> user_targets_;  // the ids of the user labels some goto of the current function targets
  // The memory model's bookkeeping (decision 0032 item 5): the suffix of each enumeration's cell fields, the Type of each cell-type suffix
  // used, and where the first address-taken declaration of each type is (the map rows of its `alloc_<t>`).
  std::map<const IntType*, std::string> enum_suffix_;
  std::map<std::string, Type> cell_type_of_;
  std::map<std::string, SrcLoc> alloc_at_;
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
