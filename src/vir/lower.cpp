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
#include <cstdint>
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

std::string sanitize(const std::string& s)
{
    std::string out;
    for (char c : s)
    {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')
        {
            out += c;
        }
        else if (c == '$')
        {
            out += "_D_";
        }
        else
        {
            static const char* hex {"0123456789abcdef"};
            out += "_x";
            out += hex[(static_cast<unsigned char>(c) >> 4U) & 15];
            out += hex[static_cast<unsigned char>(c) & 15];
        }
    }
    return out.empty() ? "anon" : out;
}

std::string method_name(const Function& f)
{
    return "f_" + sanitize(f.c_name) + "__" + std::to_string(f.id);
}
std::string formal_name(const Function& f, int k)
{
    const std::string& n {f.params[static_cast<size_t>(k)].name};
    return "p_" + sanitize(n.empty() ? "arg" : n) + "__" + std::to_string(k);
}
// A compound literal's hidden local (Var::compound_literal, decision 0044 item 7) is named by the literal's spelling; its Viper name is `v_lit__<id>`.
std::string local_name(const Var& v)
{
    if (v.compound_literal)
    {
        return "v_lit__" + std::to_string(v.id);
    }
    return "v_" + sanitize(v.name) + "__" + std::to_string(v.id);
}
// Decision 0044 items 3 and 4: a struct value is the tuple of its leaves (struct_leaves order).  A leaf's Viper name is its parameter's (or `res`) with one
// `_m<k>` per member index on its path: `p_s__0_m0`, `p_s__0_m1_m0`, `res_m1`.
std::string path_suffix(const std::vector<int>& path)
{
    std::string r;
    for (int k : path)
    {
        r += "_m" + std::to_string(k);
    }
    return r;
}
// The number of leaves of a struct (1 for any other type): the width of a value's tuple.
size_t leaf_count(const Type& t)
{
    if (!t.is_struct())
    {
        return 1U;
    }
    size_t n {0U};
    for (const StructMember& m : t.struct_type->members)
    {
        n += leaf_count(m.type);
    }
    return n;
}
// The position of member `k` of struct `st` in its tuple of leaves, and its width.
std::pair<size_t, size_t> leaf_span(const StructType& st, int k)
{
    size_t start {0U};
    for (int i {0}; i < k; ++i)
    {
        start += leaf_count(st.members[static_cast<size_t>(i)].type);
    }
    return {start, leaf_count(st.members[static_cast<size_t>(k)].type)};
}
// A struct with a pointer leaf: its value carries pointers the caller must treat as uses (0037 item 3).
bool has_pointer_leaf(const Type& t)
{
    if (!t.is_struct())
    {
        return t.is_ptr();
    }
    for (const StructMember& m : t.struct_type->members)
    {
        if (has_pointer_leaf(m.type))
        {
            return true;
        }
    }
    return false;
}
// the E of a struct-typed LV: a value of a struct is its `leaves`; one that reaches another consumer prints this
// (decision 0044 item 2)
const char* kPoison {"__struct_value__"};
// The Viper type of a VIR type: a pointer is a Ref (decision 0032 item 5).
// A struct has no value of its own (decision 0043 item 3); its only Viper variable is the Ref of its slot, which is what a struct local holds.
low::LType ltype(const Type& t)
{
    return t.is_bool() ? low::LType::Bool : ((t.is_ptr() || t.is_struct()) ? low::LType::Ref : low::LType::Int);
}
const char* kEndLabel {"l_end"};
const char* kResult {"res"};

// ---------------------------------------------------------------------------------------------------------------
// Integer type facts
// ---------------------------------------------------------------------------------------------------------------

std::string spell(const Type& t, const std::string& c_name)
{
    if (!c_name.empty())
    {
        return c_name;
    }
    return t.str();
}
std::string spell(const Expr& e)
{
    return spell(e.type, e.c_type_name);
}

E range_check(const E& v, const IntType& t)
{
    const dec::Range r {dec::range_of(t)};
    return low::in_range(v, r.lo, r.hi);
}
// The same, as one atomic assertion (for obligations).
E range_obligation(const E& v, const IntType& t)
{
    const dec::Range r {dec::range_of(t)};
    return low::in_range_atomic(v, r.lo, r.hi);
}

// ---------------------------------------------------------------------------------------------------------------
// Contracts with pointers (decision 0037): structural helpers over lowered expressions
// ---------------------------------------------------------------------------------------------------------------

// Structural equality of two lowered expressions (a null is "true", equal only to a null).
bool same_expr(const E& a, const E& b)
{
    if (a == b)
    {
        return true;
    }
    if (!a || !b)
    {
        return false;
    }
    if (a->kind != b->kind || a->text != b->text || a->bool_value != b->bool_value || a->un != b->un ||
        a->bin != b->bin)
    {
        return false;
    }
    if (!same_expr(a->a, b->a) || !same_expr(a->b, b->b) || !same_expr(a->c, b->c))
    {
        return false;
    }
    if (a->args.size() != b->args.size())
    {
        return false;
    }
    for (size_t i {0U}; i < a->args.size(); ++i)
    {
        if (!same_expr(a->args[i], b->args[i]))
        {
            return false;
        }
    }
    return true;
}

// The conjuncts of `e` (left-associated `&&` flattened); empty for a null (true).
void conjuncts(const E& e, std::vector<E>& out)
{
    if (!e)
    {
        return;
    }
    if (e->kind == low::LExpr::Kind::Bin && e->bin == low::BinOp::And)
    {
        conjuncts(e->a, out);
        conjuncts(e->b, out);
        return;
    }
    out.push_back(e);
}

// Whether every conjunct of `d` is a conjunct of `guard`: `guard ==> d` structurally.
bool implied_by(const E& d, const E& guard)
{
    std::vector<E> dc;
    std::vector<E> gc;
    conjuncts(d, dc);
    conjuncts(guard, gc);
    for (const E& x : dc)
    {
        bool found {false};
        for (const E& y : gc)
        {
            if (same_expr(x, y))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            return false;
        }
    }
    return true;
}

// Whether a lowered Bool expression is always true by its shape (an atom of a value clause lowers to `true`): a literal, a conjunction of two,
// a disjunction with one, a conditional with both arms.
bool always_true(const E& e)
{
    if (!e)
    {
        return true;
    }
    switch (e->kind)
    {
        case low::LExpr::Kind::BoolLit:
            return e->bool_value;
        case low::LExpr::Kind::Bin:
            if (e->bin == low::BinOp::And)
            {
                return always_true(e->a) && always_true(e->b);
            }
            if (e->bin == low::BinOp::Or)
            {
                return always_true(e->a) || always_true(e->b);
            }
            if (e->bin == low::BinOp::Implies)
            {
                return always_true(e->b);
            }
            return false;
        case low::LExpr::Kind::Ite:
            return always_true(e->b) && always_true(e->c);
        default:
            return false;
    }
}

bool has_atom(const Expr& e)
{
    if (e.kind == Expr::Kind::Atom)
    {
        return true;
    }
    for (const Expr* x : {e.a, e.b, e.c})
    {
        if (x != nullptr && has_atom(*x))
        {
            return true;
        }
    }
    for (const Expr* x : e.args)
    {
        if (x != nullptr && has_atom(*x))
        {
            return true;
        }
    }
    return false;
}

E min_perm(const E& a, const E& b)
{
    return low::ite(low::bin(low::BinOp::Lt, a, b), a, b);
}

// ---------------------------------------------------------------------------------------------------------------
// Constant masks (decision 0026 part B)
// ---------------------------------------------------------------------------------------------------------------

// Exact integers for the evaluation of a lowered constant operand: a sign and a magnitude of decimal digits.  Values reach 2^128 and
// beyond (2^N - 1 for N = 128, and products of such values), so no host integer is used.
struct Z
{
    bool neg {false};
    std::string mag {"0"}; // digits without leading zeros
};

int mag_cmp(const std::string& a, const std::string& b)
{
    if (a.size() != b.size())
    {
        return a.size() < b.size() ? -1 : 1;
    }
    return a < b ? -1 : (a > b ? 1 : 0);
}
std::string mag_add(const std::string& a, const std::string& b)
{
    std::string r;
    int carry {0};
    for (size_t i {0U}; i < std::max(a.size(), b.size()) || carry != 0; ++i)
    {
        const int x {i < a.size() ? a[a.size() - 1U - i] - '0' : 0};
        const int y {i < b.size() ? b[b.size() - 1U - i] - '0' : 0};
        r += static_cast<char>('0' + (x + y + carry) % 10);
        carry = (x + y + carry) / 10;
    }
    std::reverse(r.begin(), r.end());
    return r.empty() ? "0" : r;
}
std::string mag_sub(const std::string& a, const std::string& b)
{ // a >= b
    std::string r;
    int borrow {0};
    for (size_t i {0U}; i < a.size(); ++i)
    {
        int x {a[a.size() - 1U - i] - '0' - borrow};
        const int y {i < b.size() ? b[b.size() - 1U - i] - '0' : 0};
        borrow = x < y ? 1 : 0;
        if (x < y)
        {
            x += 10;
        }
        r += static_cast<char>('0' + x - y);
    }
    std::reverse(r.begin(), r.end());
    const size_t first {r.find_first_not_of('0')};
    return first == std::string::npos ? "0" : r.substr(first);
}
std::string mag_mul(const std::string& a, const std::string& b)
{
    if (a == "0" || b == "0")
    {
        return "0";
    }
    std::vector<int> d(a.size() + b.size(), 0);
    for (size_t i {0U}; i < a.size(); ++i)
    {
        for (size_t j {0U}; j < b.size(); ++j)
        {
            d[i + j] += (a[a.size() - 1U - i] - '0') * (b[b.size() - 1U - j] - '0');
        }
    }
    int carry {0};
    for (int& x : d)
    {
        x += carry;
        carry = x / 10;
        x %= 10;
    }
    std::string r;
    for (auto it {d.rbegin()}; it != d.rend(); ++it)
    {
        r += static_cast<char>('0' + *it);
    }
    return r.substr(r.find_first_not_of('0'));
}
Z z_neg(const Z& a)
{
    return a.mag == "0" ? a : Z {!a.neg, a.mag};
}
Z z_add(const Z& a, const Z& b)
{
    if (a.neg == b.neg)
    {
        return Z {a.neg, mag_add(a.mag, b.mag)};
    }
    const int c {mag_cmp(a.mag, b.mag)};
    if (c == 0)
    {
        return Z {};
    }
    return c > 0 ? Z {a.neg, mag_sub(a.mag, b.mag)} : Z {b.neg, mag_sub(b.mag, a.mag)};
}
Z z_mul(const Z& a, const Z& b)
{
    Z r {a.neg != b.neg, mag_mul(a.mag, b.mag)};
    if (r.mag == "0")
    {
        r.neg = false;
    }
    return r;
}
std::string z_text(const Z& a)
{
    return a.neg ? "-" + a.mag : a.mag;
}

// The value of a lowered operand that is a closed expression of integer literals: Add, Sub, Mul, Neg and `c_pow2` of a literal (what
// a constant, a `~`, a shift, a cast of constants and the parentheses of them lower to; cerastes never wraps, so a cast changes no
// value).  False for anything else: such an operand is not a constant mask and keeps the prelude function (decision 0026 item 7).
bool closed_value(const E& e, Z& out)
{
    if (!e)
    {
        return false;
    }
    switch (e->kind)
    {
        case low::LExpr::Kind::IntLit:
        {
            if (!vir::dec::valid(e->text))
            {
                return false;
            }
            out = vir::dec::negative(e->text) ? Z {true, e->text.substr(1U)} : Z {false, e->text};
            return true;
        }
        case low::LExpr::Kind::Un:
        {
            Z a;
            if (e->un != low::UnOp::Neg || !closed_value(e->a, a))
            {
                return false;
            }
            out = z_neg(a);
            return true;
        }
        case low::LExpr::Kind::Bin:
        {
            Z a;
            Z b;
            if (e->bin != low::BinOp::Add && e->bin != low::BinOp::Sub && e->bin != low::BinOp::Mul)
            {
                return false;
            }
            if (!closed_value(e->a, a) || !closed_value(e->b, b))
            {
                return false;
            }
            out = e->bin == low::BinOp::Add ? z_add(a, b)
                                            : (e->bin == low::BinOp::Sub ? z_add(a, z_neg(b)) : z_mul(a, b));
            return true;
        }
        case low::LExpr::Kind::Fn:
        {
            Z n;
            if (e->text != "c_pow2" || e->args.size() != 1U || !closed_value(e->args[0U], n) || n.neg ||
                n.mag.size() > 3U)
            {
                return false;
            }
            const int k {std::atoi(n.mag.c_str())};
            if (k > 128)
            {
                return false; // the prelude's c_pow2 is 2^128 from n = 128 on (pow2_table), not 2^n
            }
            out = Z {false, vir::dec::pow2(k)};
            return true;
        }
        default:
            return false;
    }
}

enum class MaskOp
{
    And,
    Or,
    Xor
};

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
E mask_rewrite(MaskOp op, const IntType& t, const E& other, const E& mask,
               const std::function<void(const std::string&)>& used)
{
    Z m;
    if (!closed_value(mask, m))
    {
        return nullptr;
    }
    const std::string text {z_text(m)};
    if (!vir::dec::in_range(text, vir::dec::range_of(t)))
    {
        return nullptr;
    }
    using low::BinOp;
    auto xor_fn = [&](bool complement, int k)
    {
        const std::string name {std::string("c_") + (t.is_signed ? "" : "u") + "xor_" +
                                (complement ? "cmask" : "mask") + "_" + std::to_string(t.bits) + "_" +
                                std::to_string(k)};
        used(name);
        return low::fn(name, {other});
    };
    for (int k {0}; k <= t.bits; ++k)
    {
        if (m.neg || text != vir::dec::dec1(vir::dec::pow2(k)))
        {
            continue;
        }
        const std::string p {vir::dec::pow2(k)};
        if (op == MaskOp::And)
        {
            return low::mod_pow2(other, k);
        }
        if (op == MaskOp::Or)
        {
            return low::bin(BinOp::Add, low::bin(BinOp::Mul, low::int_lit(p), low::div_pow2(other, k)),
                            low::int_lit(text));
        }
        return xor_fn(false, k);
    }
    for (int k {0}; k <= t.bits; ++k)
    {
        const std::string p {vir::dec::pow2(k)};
        const Z complement {t.is_signed ? Z {true, p} : z_add(Z {false, vir::dec::pow2(t.bits)}, Z {true, p})};
        if (text != z_text(complement))
        {
            continue;
        }
        if (op == MaskOp::And)
        {
            return low::bin(BinOp::Mul, low::int_lit(p), low::div_pow2(other, k));
        }
        if (op == MaskOp::Xor)
        {
            return xor_fn(true, k);
        }
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
struct Deps
{
    Ids ex, base;
    void add(int id)
    {
        ex.insert(id);
        base.insert(id);
    }
};

Ids join(const Ids& a, const Ids& b)
{
    Ids r {a};
    r.insert(b.begin(), b.end());
    return r;
}
Deps join(const Deps& a, const Deps& b)
{
    return Deps {join(a.ex, b.ex), join(a.base, b.base)};
}

enum class Half
{
    Exact,
    Base
};
const Ids& half_of(const Deps& d, Half h)
{
    return h == Half::Exact ? d.ex : d.base;
}

struct LV
{
    LV() = default;
    LV(E e_, Deps d_) : e(std::move(e_)), dep(std::move(d_)) {}
    E e;      // the Viper value
    Deps dep; // the fixed points whose results it mentions (D of each, so the set is closed)
    // A STRUCT value (decision 0044 item 2): its leaves in struct_leaves order, depth first; then `e` is a poison variable (kPoison) that makes any consumer
    // other than item 2's an internal error (see Lowerer::struct_value).  Empty for every other value.
    std::vector<E> leaves;
};

// The items of a full expression are templates over the set type: lowering builds them with `Deps` (both halves), `flush` projects
// them onto one half (`Ids`).  `Obl`, `Effect` and `Full` are the lowering's; `FlatObl`, `FlatEffect` and `FlatFull` are what the
// placement, emission and fail-closed checks run on.
// Every field that is not a set is in a `...Core`, which `project` copies whole: a field added there reaches the projected item without
// being listed.  A new field that holds a set goes in the templated part, and in `project`.
struct OblCore
{
    E guard; // may be null
    E cond;
    low::Meta meta;
    size_t seq {0U}; // creation order within the Full (items of one slot keep it)
    // the fixed point (a call) whose explicit termination check this is (decision 0021 item 4), else -1
    int term_of {-1};
    // the local (Var id) whose `uninitialized` flag this reads, else -1: a read of that local (decision 0025 item 1)
    int flag_of {-1};
    // Non-empty: a statement item (a case split, decision 0037 item 8), emitted in the item's place instead of an assert; `cond` and `meta` are unused.
    std::vector<low::LStmt> stmts;
};
template <class S>
struct BasicObl : OblCore
{
    S dep; // its set: emitted after the fixed points of the set, before every other
};

struct CallItem
{
    std::string method;
    std::string target; // empty for a void call
    // a struct result: one target per leaf (decision 0044 item 4); `target` is then empty
    std::vector<std::string> targets;
    std::vector<E> args;
    low::Check check;
    bool has_termination {false};
    low::Check termination;
    // emitted right after the call: the case splits on a pointer result (decision 0037 item 3)
    std::vector<low::LStmt> after;
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
enum class EffectKind
{
    Call,  // a call (CallItem): a fixed point
    Write, // `name := value`, then `flag := true` when `flag` is set: an assignment used as a value
    // `name := value` where `name` is a fresh temporary and `value` the current value of a local this expression writes
    Snapshot,
    // `stmts`: the guarded operand of `&&`, `||` or `?:` with its own effects, as an `if`, assigning a temporary
    Branch,
    // `stmts`: a write of a cell used as a value (decision 0039 item 2): the receiver copy and the two field writes;
    // movable like a Write
    Store
};

// The position of an access in the expression tree: the nodes from the root of its full expression to the node that makes the access, each with
// its slot in its parent (0: `a`, 1: `b`, 2: `c`, 3 + k: `args[k]`).  Decision 0039 item 4's order between two accesses is read from two paths.
using Path = std::vector<std::pair<const Expr*, int>>;

// What an effect (or the effects inside a branch) accesses, for decision 0039 item 4's fail-closed check of the order against the tree:
// a cell write or snapshot (`call` false) or a memory call, on the cell field `field`, at `path`.
struct MemTag
{
    int key {0};
    std::string field;
    bool call {false};
    bool write {false};
    Path path;
    // the access reads through values computed at these nodes (a shared address): it follows what is inside them
    std::vector<Path> after;
};

struct EffectCore
{
    using Kind = EffectKind;
    Kind kind {Kind::Call};
    CallItem call;
    std::string name;
    E value;
    std::string flag;
    std::vector<low::LStmt> stmts;
    int fixed {-1};  // the id of the fixed point (a Call, or a Branch with a call); -1 for the movable items
    size_t seq {0U}; // creation order within the Full
    // the locals (Var ids) a snapshot or write, or a branch's inner effects, read and write; the keys (< 0) of cell
    // accesses
    std::set<int> reads, writes;
    // the cell accesses and memory calls of the effect, or of a branch's inner effects (decision 0039 item 4)
    std::vector<MemTag> tags;
};
template <class S>
struct BasicEffect : EffectCore
{
    S deps; // movable items: their set; fixed points: PRE (what C completes before it)
};

// What a rejection says about a fixed point.
struct FixedInfo
{
    SrcLoc loc;
    std::string callee; // the callee's name; empty for a branch
    std::string what;   // `the call to "f"`, or `the call to "g" in the right operand of "&&"`
};

// What one full expression leaves behind.
struct FullCore
{
    int nfixed {0};                   // fixed points so far: the next one's id
    std::vector<size_t> fixed_effect; // the index in `effects` of each fixed point, by id
    std::vector<FixedInfo> info;      // by id
    size_t next_seq {0U};
};
template <class S>
struct BasicFull : FullCore
{
    std::vector<BasicObl<S>> obls;
    std::vector<BasicEffect<S>> effects;
    std::vector<S> pre;           // PRE of each fixed point, by id
    std::map<int, S> last_write;  // Var id -> the set of the latest write (item order of the expression so far)
    std::map<int, S> last_access; // Var id -> the union of the sets of the accesses (snapshots and writes) of it so far
};

using Obl = BasicObl<Deps>;
using Effect = BasicEffect<Deps>;
using Full = BasicFull<Deps>;
using FlatObl = BasicObl<Ids>;
using FlatEffect = BasicEffect<Ids>;
using FlatFull = BasicFull<Ids>;

// The full expression on one half of its sets (the step `flush` chose); the last_write/last_access records are not needed after lowering.
FlatFull project(const Full& full, Half h)
{
    FlatFull r;
    static_cast<FullCore&>(r) = full;
    for (const Obl& o : full.obls)
    {
        FlatObl x;
        static_cast<OblCore&>(x) = o;
        x.dep = half_of(o.dep, h);
        r.obls.push_back(std::move(x));
    }
    for (const Effect& fx : full.effects)
    {
        FlatEffect x;
        static_cast<EffectCore&>(x) = fx;
        x.deps = half_of(fx.deps, h);
        r.effects.push_back(std::move(x));
    }
    for (const Deps& d : full.pre)
    {
        r.pre.push_back(half_of(d, h));
    }
    return r;
}

// How Local/Formal/Result are read.
struct Env
{
    // Formal(k): a name (contract of the function itself) or an argument value (null for a by-value struct parameter)
    std::vector<E> formals;
    std::vector<Deps> formal_dep; // the set of each (empty: none)
    E result;                     // Result (null for a struct result)
    // Decision 0044 items 3-4: the leaves of each by-value struct parameter (empty for the others), and of a struct result.
    std::vector<std::vector<E>> formal_leaves;
    std::vector<E> result_leaves;
};

class Lowerer
{
public:
    Lowerer(const Program& p, const LowerOptions& o) : prog_(p), opt_(o) {}

    LowerResult run()
    {
        compute_cycles();
        heap_ = is_heap_program(); // decision 0047 item 7: any view call or block site in any contract
        // Decision 0050 item 4: a vlibc heap program holds ANY function of vlibc's allocator, an allocation view or the deallocation view `vlibc_free`: only there are a block's
        // allocator (the value of its token) checked and carried through loops; every other program's Viper is unchanged.
        for (const Function& f : prog_.functions())
        {
            if (f.allocator == Allocator::Vlibc)
            {
                vlibc_heap_ = true;
            }
        }
        if (heap_)
        {
            out_.memory = true;
            out_.heap = true;
        }
        for (const Function& f : prog_.functions())
        {
            function(f);
        }
        for (const Function& f : prog_.functions())
        {
            if (!f.requires_.empty() || !f.ensures_.empty() || !f.decreases_.empty())
            {
                wf_method(f);
            }
        }
        check_cycles();
        alloc_methods();
        program_start_method();
        check_poison();
        LowerResult r;
        r.program = std::move(out_);
        r.errors = std::move(errors_);
        r.rejections = std::move(rejections_);
        return r;
    }

private:
    // Decision 0044 item 2: a struct value (LV::leaves) that reached a consumer other than item 2's carries the poison variable into the output; no Viper may
    // mention it (an internal error, exit 3, never a silent truncation).
    static bool poison_in(const E& e)
    {
        if (e == nullptr)
        {
            return false;
        }
        if (e->kind == low::LExpr::Kind::Var && e->text == kPoison)
        {
            return true;
        }
        if (poison_in(e->a) || poison_in(e->b) || poison_in(e->c))
        {
            return true;
        }
        for (const E& x : e->args)
        {
            if (poison_in(x))
            {
                return true;
            }
        }
        return false;
    }
    static bool poison_in(const std::vector<low::LStmt>& body)
    {
        for (const low::LStmt& s : body)
        {
            if (poison_in(s.expr))
            {
                return true;
            }
            for (const E& x : s.args)
            {
                if (poison_in(x))
                {
                    return true;
                }
            }
            for (const E& x : s.decreases)
            {
                if (poison_in(x))
                {
                    return true;
                }
            }
            for (const low::LInvariant& i : s.invariants)
            {
                if (poison_in(i.expr))
                {
                    return true;
                }
            }
            if (poison_in(s.then_body) || poison_in(s.else_body) || poison_in(s.stmts))
            {
                return true;
            }
        }
        return false;
    }
    void check_poison()
    {
        for (const low::LMethod& m : out_.methods)
        {
            bool bad {poison_in(m.body)};
            for (const low::LClause& c : m.requires_)
            {
                bad = bad || poison_in(c.expr);
            }
            for (const low::LClause& c : m.ensures_)
            {
                bad = bad || poison_in(c.expr);
            }
            for (const E& d : m.decreases_)
            {
                bad = bad || poison_in(d);
            }
            if (bad)
            {
                internal_error(SrcLoc {},
                               "a struct value reached a consumer lowering does not take (decision 0044 item 2) in `" +
                                   m.name + "`");
            }
        }
    }

    // The trusted allocation of each cell type used (decision 0032 item 4; 0028 item 17), before every other method: a fresh one-slot block, its
    // cell uninitialized (full permission to the value and the flag, the flag false), and its liveness cell with `lvloc(b) == loc(b, -2)`.
    // The map rows of its clauses are at the first address-taken declaration of the type.
    void alloc_methods()
    {
        std::vector<low::LMethod> methods;
        for (const auto& [suffix, at] : alloc_at_)
        {
            if (const auto sa {struct_alloc_.find(suffix)}; sa != struct_alloc_.end())
            {
                methods.push_back(alloc_struct_method(*sa->second, at));
                continue;
            }
            const Type& t {cell_type_of_.at(suffix)};
            const E r {low::var("r")};
            const E b {block_of(r)};
            auto clause = [&](E expr)
            {
                low::LClause c;
                c.expr = std::move(expr);
                for (const char* id : {"not.wellformed", "postcondition.violated"})
                {
                    low::Meta m;
                    m.category = "internal";
                    m.loc = at;
                    m.message =
                        std::string("internal check: ") +
                        (std::string(id) == "not.wellformed" ? "well-formedness of" : "Silicon's own check of") +
                        " the trusted contract of the allocation of a variable of type `" + t.str() +
                        "` whose address is taken";
                    c.checks.push_back(low::Check {id, std::move(m)});
                }
                return c;
            };
            using low::BinOp;
            low::LMethod m;
            m.name = "alloc_" + suffix;
            m.trusted_reason =
                "allocation of an address-taken local of type " + t.str() +
                ": a fresh one-slot block, the cell uninitialized and the block's liveness token, full permission (decision 0032 item 4)";
            m.results.push_back(low::LVar {"r", low::LType::Ref});
            m.requires_.clear();
            m.ensures_.push_back(
                clause(low::bin(BinOp::And,
                                low::bin(BinOp::And, low::bin(BinOp::Ne, r, low::null_()),
                                         low::bin(BinOp::Eq, r, low::dom("loc", {b, low::int_lit("0")}))),
                                low::bin(BinOp::Eq, low::dom("len", {b}), low::int_lit("1")))));
            E rest {low::bin(BinOp::Eq, live_cell(r), low::dom("loc", {b, low::int_lit("-2")}))};
            rest = low::bin(BinOp::And, rest, low::acc(r, "v_" + suffix));
            rest = low::bin(BinOp::And, rest, low::acc(r, "d_" + suffix));
            rest = low::bin(BinOp::And, rest, low::not_(cell_flag(r, t)));
            rest = low::bin(BinOp::And, rest, low::acc(live_cell(r), "lv"));
            m.ensures_.push_back(clause(rest));
            m.decreases_assumed = true; // 0013 item 5
            methods.push_back(std::move(m));
        }
        out_.methods.insert(out_.methods.begin(), std::make_move_iterator(methods.begin()),
                            std::make_move_iterator(methods.end()));
    }

    // The trusted allocation of a struct local (decision 0043 items 4 and 13), in the shape the lint pins: the root facts as `alloc_<t>`'s, then in declaration
    // order, depth first, one clause per member block (`len(member(X, k)) == 1`, and for a leaf `acc(L.v_t) && acc(L.d_t) && !L.d_t` with `L = loc(member(X, k), 0)`;
    // a nested struct member's slot `loc(member(X, k), 0)` is the `X` of the clauses that follow it), then the liveness cell.
    low::LMethod alloc_struct_method(const StructType& st, const SrcLoc& at)
    {
        using low::BinOp;
        const E r {low::var("r")};
        const E b {block_of(r)};
        const std::string name {struct_suffix(st)};
        auto clause = [&](E expr)
        {
            low::LClause c;
            c.expr = std::move(expr);
            for (const char* id : {"not.wellformed", "postcondition.violated"})
            {
                low::Meta m;
                m.category = "internal";
                m.loc = at;
                m.message = std::string("internal check: ") +
                            (std::string(id) == "not.wellformed" ? "well-formedness of" : "Silicon's own check of") +
                            " the trusted contract of the allocation of a variable of type `" +
                            Type::structure(&st).str() + "`";
                c.checks.push_back(low::Check {id, std::move(m)});
            }
            return c;
        };
        low::LMethod m;
        m.name = "alloc_" + name;
        m.trusted_reason =
            "allocation of a local of type " + Type::structure(&st).str() +
            ": a fresh one-slot block whose members are blocks of length 1, every leaf uninitialized with full permission, and the block's liveness token (decision 0043 item 4)";
        m.results.push_back(low::LVar {"r", low::LType::Ref});
        m.ensures_.push_back(clause(low::bin(BinOp::And,
                                             low::bin(BinOp::And, low::bin(BinOp::Ne, r, low::null_()),
                                                      low::bin(BinOp::Eq, r, low::dom("loc", {b, low::int_lit("0")}))),
                                             low::bin(BinOp::Eq, low::dom("len", {b}), low::int_lit("1")))));
        // the slot facts, right after the root facts: one clause per pair `X != Y`, the root's pairs first, then each
        // nested slot's, depth first (S33 (i))
        {
            std::vector<std::pair<E, E>> pairs;
            slot_pairs(r, st, pairs);
            for (const auto& [x, y] : pairs)
            {
                m.ensures_.push_back(clause(low::bin(BinOp::Ne, x, y)));
            }
        }
        std::function<void(const E&, const StructType&)> members = [&](const E& slot, const StructType& s)
        {
            for (size_t k {0U}; k < s.members.size(); ++k)
            {
                const E mb {low::dom("member", {slot, low::int_lit(std::to_string(k))})};
                E row {low::bin(BinOp::Eq, low::dom("len", {mb}), low::int_lit("1"))};
                const Type& mt {s.members[k].type};
                const E leaf {member_ref(slot, static_cast<int>(k))};
                if (mt.is_struct())
                {
                    m.ensures_.push_back(clause(row));
                    members(leaf, *mt.struct_type);
                    continue;
                }
                const std::string sfx {use_cell_type(mt)};
                row = low::bin(BinOp::And, row, low::acc(leaf, "v_" + sfx));
                row = low::bin(BinOp::And, row, low::acc(leaf, "d_" + sfx));
                row = low::bin(BinOp::And, row, low::not_(cell_flag(leaf, mt)));
                m.ensures_.push_back(clause(row));
            }
        };
        members(r, st);
        E rest {low::bin(BinOp::Eq, live_cell(r), low::dom("loc", {b, low::int_lit("-2")}))};
        rest = low::bin(BinOp::And, rest, low::acc(live_cell(r), "lv"));
        m.ensures_.push_back(clause(rest));
        m.decreases_assumed = true; // 0013 item 5
        return m;
    }

    // The trusted `program_start` (decision 0041 item 4), after the allocation methods: the global facts first, then for each global and `static` local, in
    // declaration order, one row at its declaration: full permission to its value and flag cells, the flag set, its initial value (or its range, `WF` for a
    // pointer, when the unit does not know it), and its liveness cell.  Emitted for every unit with a global (the lint pins one `program_start` per such program); only the entry function calls it.
    void program_start_method()
    {
        if (!has_globals())
        {
            return;
        }
        out_.memory = true;
        for (const Global& g : prog_.globals())
        {
            out_.globals.push_back(g.viper_name());
        }
        low::LMethod m;
        m.name = "program_start";
        m.trusted_reason =
            "program start: every global and static local holds its initial value with full permission, and its liveness cell (decision 0041 item 4); the pure facts first";
        m.decreases_assumed = true; // 0013 item 5
        auto clause = [&](E expr, const SrcLoc& at, const std::string& what)
        {
            low::LClause c;
            c.expr = std::move(expr);
            for (const char* id : {"not.wellformed", "postcondition.violated"})
            {
                low::Meta mt;
                mt.category = "internal";
                mt.loc = at;
                mt.message = std::string("internal check: ") +
                             (std::string(id) == "not.wellformed" ? "well-formedness of" : "Silicon's own check of") +
                             " the trusted contract of program_start for " + what;
                c.checks.push_back(low::Check {id, std::move(mt)});
            }
            return c;
        };
        m.ensures_.push_back(clause(global_facts(), prog_.globals().front().loc, "the facts of the globals"));
        for (const Global& g : prog_.globals())
        {
            const E recv {global_ref(g.id)};
            const Type& t {g.type};
            // decision 0044 item 8: one row, a conjunct per leaf (each with its own initial value), then the one
            // liveness cell
            if (t.is_struct())
            {
                E all;
                const std::vector<StructLeaf> ls {struct_leaves(*t.struct_type)};
                for (size_t i {0U}; i < ls.size(); ++i)
                {
                    const E cell {chain_ref(recv, ls[i].path)};
                    const Type& lt {ls[i].type};
                    const std::string lsfx {use_cell_type(lt)};
                    E leaf {low::and_(low::and_(low::acc(cell, "v_" + lsfx), low::acc(cell, "d_" + lsfx)),
                                      cell_flag(cell, lt))};
                    const GlobalLeaf dflt;
                    const GlobalLeaf& gl {i < g.leaves.size() ? g.leaves[i] : dflt};
                    switch (gl.kind)
                    {
                        case GlobalInit::Value:
                            leaf = low::and_(leaf,
                                             low::bin(low::BinOp::Eq, cell_value(cell, lt), literal_of(lt, gl.value)));
                            break;
                        case GlobalInit::Null:
                            leaf = low::and_(leaf, low::bin(low::BinOp::Eq, cell_value(cell, lt), low::null_()));
                            break;
                        case GlobalInit::Addr:
                            leaf = low::and_(leaf,
                                             low::bin(low::BinOp::Eq, cell_value(cell, lt),
                                                      gl.target != nullptr ? global_ref(gl.target->id) : low::null_()));
                            break;
                        case GlobalInit::Unknown:
                            if (const E r = cell_range_inv(cell, lt))
                            {
                                leaf = low::and_(leaf, r);
                            }
                            break;
                    }
                    all = low::and_(all, leaf);
                }
                all = low::and_(all, low::acc(low::dom("lvloc", {global_block(g.id)}), "lv"));
                m.ensures_.push_back(clause(all, g.loc, "`" + g.c_name + "`"));
                continue;
            }
            const std::string sfx {use_cell_type(t)};
            E row {low::and_(low::and_(low::acc(recv, "v_" + sfx), low::acc(recv, "d_" + sfx)), cell_flag(recv, t))};
            switch (g.init_kind)
            {
                case Global::Init::Value:
                    row = low::and_(row, low::bin(low::BinOp::Eq, cell_value(recv, t), literal_of(t, g.value)));
                    break;
                case Global::Init::Null:
                    row = low::and_(row, low::bin(low::BinOp::Eq, cell_value(recv, t), low::null_()));
                    break;
                case Global::Init::Addr:
                    row = low::and_(row, low::bin(low::BinOp::Eq, cell_value(recv, t),
                                                  g.target != nullptr ? global_ref(g.target->id) : low::null_()));
                    break;
                case Global::Init::Unknown:
                    if (const E r = cell_range_inv(recv, t))
                    {
                        row = low::and_(row, r);
                    }
                    break;
            }
            row = low::and_(row, low::acc(low::dom("lvloc", {global_block(g.id)}), "lv"));
            m.ensures_.push_back(clause(row, g.loc, "`" + g.c_name + "`"));
        }
        // After the allocation methods (they come first, decision 0032 item 4).
        out_.methods.insert(out_.methods.begin() + static_cast<std::ptrdiff_t>(alloc_at_.size()), std::move(m));
    }

    // ---- recursion cycles (decision 0021 item 4) ----

    // The functions a statement or expression calls (every Call node, dead code included: Silicon's call graph has an edge for every
    // MethodCall of a body, silver's MethodCheck.scala).  `skip_nested`: the self-test hook that forgets the calls inside an `if`, a loop
    // or a block (an under-approximation, which the check against the emitted methods must catch).
    static void calls_in_expr(const Expr* e, std::vector<const Function*>& out)
    {
        if (e == nullptr)
        {
            return;
        }
        if (e->kind == Expr::Kind::Call && e->callee != nullptr)
        {
            out.push_back(e->callee);
        }
        for (const Expr* x : {e->a, e->b, e->c})
        {
            calls_in_expr(x, out);
        }
        for (const Expr* x : e->args)
        {
            calls_in_expr(x, out);
        }
    }

    static void calls_in_stmt(const Stmt* s, std::vector<const Function*>& out, bool skip_nested, bool top = true)
    {
        if (s == nullptr)
        {
            return;
        }
        calls_in_expr(s->expr, out);
        calls_in_expr(s->addr, out); // a Store's address (decision 0032)
        for (const Clause& c : s->invariants)
        {
            calls_in_expr(c.expr, out);
        }
        for (const Clause& c : s->measure)
        {
            calls_in_expr(c.expr, out);
        }
        if (skip_nested && !top)
        {
            return;
        }
        calls_in_stmt(s->then_body, out, skip_nested, false);
        calls_in_stmt(s->else_body, out, skip_nested, false);
        calls_in_stmt(s->body, out, skip_nested, false);
        calls_in_stmt(s->step, out, skip_nested, false);
        for (const Stmt* c : s->stmts)
        {
            calls_in_stmt(c, out, skip_nested, false);
        }
    }

    // The strongly connected components of a graph (Tarjan): the component number of each vertex.
    static std::vector<int> components(const std::vector<std::vector<int>>& adj)
    {
        const size_t n {adj.size()};
        std::vector<int> number(n, -1);
        std::vector<int> low(n, 0);
        std::vector<int> comp(n, -1);
        std::vector<char> on_stack(n, 0);
        std::vector<int> stack;
        int counter {0};
        int ncomp {0};
        std::function<void(int)> visit = [&](int v)
        {
            number[static_cast<size_t>(v)] = low[static_cast<size_t>(v)] = counter++;
            stack.push_back(v);
            on_stack[static_cast<size_t>(v)] = 1;
            for (int w : adj[static_cast<size_t>(v)])
            {
                if (number[static_cast<size_t>(w)] < 0)
                {
                    visit(w);
                    low[static_cast<size_t>(v)] = std::min(low[static_cast<size_t>(v)], low[static_cast<size_t>(w)]);
                }
                else if (on_stack[static_cast<size_t>(w)] != 0)
                {
                    low[static_cast<size_t>(v)] = std::min(low[static_cast<size_t>(v)], number[static_cast<size_t>(w)]);
                }
            }
            if (low[static_cast<size_t>(v)] == number[static_cast<size_t>(v)])
            {
                int w;
                do
                {
                    w = stack.back();
                    stack.pop_back();
                    on_stack[static_cast<size_t>(w)] = 0;
                    comp[static_cast<size_t>(w)] = ncomp;
                } while (w != v);
                ++ncomp;
            }
        };
        for (size_t v {0U}; v < n; ++v)
        {
            if (number[v] < 0)
            {
                visit(static_cast<int>(v));
            }
        }
        return comp;
    }

    // The smallest vertex of each vertex's component: a canonical form of the partition.
    static std::vector<int> canonical(const std::vector<int>& comp)
    {
        std::map<int, int> first;
        std::vector<int> out(comp.size());
        for (size_t v {0U}; v < comp.size(); ++v)
        {
            out[v] = first.emplace(comp[v], static_cast<int>(v)).first->second;
        }
        return out;
    }

    // The components of the call graph of the VIR functions (the `__wf` methods and the methods without a body call nothing): the same
    // sets Silicon's termination plugin computes over the emitted methods.  A call needs the explicit check exactly when the callee is in
    // the caller's component (a self-call included).  `check_cycles` compares them with the emitted program's.
    void compute_cycles()
    {
        const std::deque<Function>& fns {prog_.functions()};
        std::map<const Function*, int> index;
        for (const Function& f : fns)
        {
            index[&f] = static_cast<int>(index.size());
        }
        std::vector<std::vector<int>> adj(fns.size());
        for (const Function& f : fns)
        {
            std::vector<const Function*> callees;
            calls_in_stmt(f.body, callees, opt_.cycles_ignore_nested_calls);
            for (const Function* c : callees)
            {
                const auto it {index.find(c)};
                if (it != index.end())
                {
                    adj[static_cast<size_t>(index[&f])].push_back(it->second);
                }
            }
        }
        const std::vector<int> comp {components(adj)};
        for (const Function& f : fns)
        {
            component_[&f] = comp[static_cast<size_t>(index[&f])];
        }
    }

    static void calls_in_lowered(const std::vector<low::LStmt>& body, std::vector<std::string>& out)
    {
        for (const low::LStmt& st : body)
        {
            if (st.kind == low::LStmt::Kind::Call)
            {
                out.push_back(st.name);
            }
            calls_in_lowered(st.then_body, out);
            calls_in_lowered(st.else_body, out);
            calls_in_lowered(st.stmts, out);
        }
    }

    // Decision 0021 item 4: an under-approximated cycle would be unsound (a recursive call whose explicit check is not written is hidden
    // by an earlier call that does not return), so the components used are compared with those of the emitted methods' Call statements,
    // which are what Silicon sees; a difference is an internal error.
    void check_cycles()
    {
        const std::deque<Function>& fns {prog_.functions()};
        std::map<std::string, int> index;
        for (const Function& f : fns)
        {
            index[method_name(f)] = static_cast<int>(index.size());
        }
        std::vector<std::vector<int>> adj(fns.size());
        for (const low::LMethod& m : out_.methods)
        {
            std::vector<std::string> callees;
            calls_in_lowered(m.body, callees);
            const auto it {index.find(m.name)};
            if (it == index.end())
            { // a `__wf` method: Silicon's graph includes it, and it must make no call (contracts hold none)
                if (!callees.empty())
                {
                    internal_error(
                        SrcLoc {},
                        "the well-formedness method `" + m.name +
                            "` contains a call: it would be an edge of the call graph that the recursion components lack");
                }
                continue;
            }
            for (const std::string& c : callees)
            {
                const auto t {index.find(c)};
                if (t != index.end())
                {
                    adj[static_cast<size_t>(it->second)].push_back(t->second);
                }
            }
        }
        std::vector<int> used(fns.size());
        for (const Function& f : fns)
        {
            used[static_cast<size_t>(index[method_name(f)])] = component_[&f];
        }
        if (canonical(components(adj)) != canonical(used))
        {
            internal_error(
                SrcLoc {},
                "the recursion components the explicit termination checks assumed differ from the call graph of the emitted methods");
        }
    }

    bool in_cycle(const Function* caller, const Function* callee) const
    {
        const auto a {component_.find(caller)};
        const auto b {component_.find(callee)};
        return a != component_.end() && b != component_.end() && a->second == b->second;
    }

    // ---- per-function state ----
    struct Fn
    {
        const Function* f {nullptr};
        std::string method;
        std::vector<std::string> var_names; // by Var::id
        std::vector<low::LVar> temps;
        int ntemp {0};
        // Decision 0019: the locals declared without an initializer (by Var::id) each get a ghost flag `d_<name>`, true exactly
        // when the object holds a value; a loop that assigns one declared outside it gets a snapshot `ds_<name>__<loop>`.
        std::set<int> flagged;
        std::vector<low::LVar> snapshots;
        int nloop {0};
        // The blocks that declare cells and are open at the statement being lowered, outermost first (decision 0032 item 8).
        struct Scope
        {
            std::vector<const Var*> cells; // in declaration order
            SrcLoc end;                    // the closing brace
        };
        std::vector<Scope> scopes;
        // Decision 0038 item 7: the number of open scopes at the target of each break/continue (by jump id: for a loop the depth at the loop,
        // for a switch the depth inside its body's scope) and at each user label (by label id), so a jump exits exactly the scopes above them.
        std::map<int, size_t> jump_depth;
        std::map<int, size_t> label_depth;
        // Decision 0038 item 3: the pointer parameters (by index) the function assigns; their formal's place is not automatic.
        std::set<int> assigned_params;
    };

    // A place a loop frames (decision 0038 item 3): a cell, or the place a pointer parameter's `requires` atoms name through the formal.
    struct Place
    {
        bool cell {false};
        const Var* var {nullptr}; // a cell
        int param {-1};           // a parameter place: the index of the formal
        E recv;                   // the Ref: the cell's variable or the formal
        Type type;                // the cell's type, or the pointee of the parameter
        bool value {false};       // the value and flag cells are held (a cell; a parameter with an owns/reads atom)
        std::string suffix;       // the cell field suffix of `type`
        std::string label;        // the user's spelling, for messages: `x` or `*p`
        E gv, gl;                 // the guards of the value/flag and of the liveness conjuncts (null: true)
        std::string kv, kd, kl, vs, ds; // the ghost snapshots
        bool reach {false};             // a write in the loop can reach the value
        E wg;                           // a written atom's place (decision 0040 item 5): the atom's guard (null: true)
        // Decision 0043 item 11: a struct object's leaves are places of their own.  `lroot` is the pointer whose liveness cell the place's row is (null: `recv`: a scalar
        // place); `live` whether this place carries the liveness row (the first leaf of each root: one row per liveness cell, never one per leaf); `mpath` the member path
        // of the leaf from the root; `slots` the struct objects (type, Ref) the place is the first leaf of (what a pointer to a struct may equal).
        E lroot;
        bool live {true};
        std::vector<int> mpath;
        std::vector<std::pair<Type, E>> slots;
        // Decision 0047 item 10 (heap places): the object a pointer VARIABLE designates at the loop's entry, through its base `lroot` (the variable, or its entry ghost).
        // The first leaf (`live`) also carries the block's free token: the snapshot `kt` of its permission, guarded by `gt`, and `tptr` is the pointee type.
        bool heap {false};
        std::string kt;
        // decision 0050 item 5 (a vlibc heap program): the snapshot of the token's value, taken when `kt > none` (false
        // otherwise); empty: none
        std::string ktv;
        E gt;
        Type ptee;
    };
    static E lv_of(const Place& pl)
    {
        return pl.lroot ? pl.lroot : pl.recv;
    }
    // A written atom of a loop invariant (decision 0040 item 5): `owns(*q)`, `reads(*q)` or `live(q)` of a pointer variable `q`, with its ghost
    // amounts (taken at the loop's entry).
    struct WAtom
    {
        AtomKind kind {AtomKind::Owns};
        E q, g;           // the pointer variable, and the atom's guard (null: true)
        Type pt;          // q's pointee
        std::string sfx;  // the cell field suffix of the pointee (not for Live)
        std::string text; // `*q`, or `q` for Live
        SrcLoc loc;
        std::string av, ad, al; // the ghost amounts
        // the value snapshot (decision 0040 item 5, "Values"); empty: none (the loop assigns q, or Live)
        std::string vs;
        bool reach {false}; // a write in the loop can reach the value
        // Decision 0045 item 12.  An atom on a place reached through a load is read in the invariant through the loop's places (`q` is that chain); the body names it by a ghost
        // Ref taken at the top of each iteration (`qb`).  A struct atom is one entry per leaf cell (`q` the leaf's Ref, `lq` the struct slot, `lpt` its pointer type): the
        // first entry carries the atom's one liveness amount and its basic row (`first`); `atom_id` numbers the atom for the separation of entries of one atom.
        E qb;     // the body's name of `q` (the same as `q` unless the atom loads)
        E lq;     // the pointer the atom is on (the slot of a struct atom): its liveness cell, its basic row
        Type lpt; // its type
        bool first {true};
        int atom_id {0};
        bool loads {false};
        const Expr* atom {nullptr};
        std::vector<AtomGuard> guards;
        std::vector<int> lpath; // a struct atom's leaf: the member path from the slot
        std::string aq;         // the ghost Ref of `qb` (empty unless the atom loads)
    };
    // A heap read of an invariant or a measure, for the explicit framing assert (decision 0038 item 6).
    struct InvRead
    {
        E ptr; // the pointer read through (null: a cell by name)
        const Var* cell {nullptr};
        Type type;
        SrcLoc loc;
        std::string text; // the place as the user wrote it
        E guard;          // the condition under which the read is evaluated
    };
    struct Framing
    {
        E cond;
        low::Meta meta;
    };

    void internal_error(const SrcLoc& loc, std::string what)
    {
        errors_.push_back(Diag {loc, std::move(what)});
    }

    const std::string& var_name(const Var& v)
    {
        return fn_.var_names[static_cast<size_t>(v.id)];
    }

    // ---- locals declared without an initializer (decision 0019) ----

    static void find_flagged(const Stmt& s, std::set<int>& out)
    {
        switch (s.kind)
        {
            // a cell's flag is a field of the cell (decision 0032 item 4, 0019 item 13), not a local
            case Stmt::Kind::Decl:
                if (s.expr == nullptr && s.var != nullptr && !s.var->address_taken)
                {
                    out.insert(s.var->id);
                }
                break;
            case Stmt::Kind::If:
                if (s.then_body != nullptr)
                {
                    find_flagged(*s.then_body, out);
                }
                if (s.else_body != nullptr)
                {
                    find_flagged(*s.else_body, out);
                }
                break;
            case Stmt::Kind::While:
                if (s.body != nullptr)
                {
                    find_flagged(*s.body, out);
                }
                if (s.step != nullptr)
                {
                    find_flagged(*s.step, out);
                }
                break;
            case Stmt::Kind::Switch:
                if (s.body != nullptr)
                {
                    find_flagged(*s.body, out);
                }
                break;
            case Stmt::Kind::Block:
                for (const Stmt* c : s.stmts)
                {
                    if (c != nullptr)
                    {
                        find_flagged(*c, out);
                    }
                }
                break;
            default:
                break;
        }
    }

    bool flagged(const Var& v) const
    {
        return fn_.flagged.count(v.id) != 0U;
    }
    std::string flag_name(const Var& v)
    {
        return "d_" + var_name(v);
    }

    // The fixed value a variable holds while it has none: 0 (`false`), or the low end of an enumeration's value range when 0 is
    // not in it.  It only keeps the variable inside its type's range (0019 item 5).
    E dummy_value(const Var& v)
    {
        if (v.type.is_bool())
        {
            return low::bool_lit(false);
        }
        if (v.type.is_ptr())
        {
            return low::null_();
        }
        const dec::Range r {dec::range_of(*v.type.int_type)};
        return low::int_lit(dec::in_range("0", r) ? "0" : r.lo);
    }

    // `d := false; v := <dummy>`: the variable is indeterminate (at its declaration, at the entry of its block, before a switch's
    // dispatch).
    void reset(std::vector<low::LStmt>& out, const Var& v)
    {
        assign(out, flag_name(v), low::bool_lit(false));
        assign(out, var_name(v), dummy_value(v));
    }

    low::Meta meta(const char* category, const SrcLoc& loc, std::string message)
    {
        low::Meta m;
        m.category = category;
        m.loc = loc;
        m.message = std::move(message);
        if (report_at_ != nullptr)
        {
            m.has_note = true;
            m.note_loc = m.loc;
            m.note_text = report_note_ != nullptr && !report_note_->empty() ? *report_note_ : "in the default argument";
            m.loc = *report_at_;
        }
        return m;
    }

    void use(const std::string& prelude_fn)
    {
        out_.prelude_used.insert(prelude_fn);
    }

    // ---- obligations ----

    // `exact_set`, when given, is joined into the EXACT half of the set (decision 0026 item 1: a `requires` clause's obligation has PRE of
    // its call); the BASE half is always the guard's, the value's and the floor's.
    void obl(Full& full, const E& guard, const Deps& gdep, E cond, const Deps& cdep, low::Meta m,
             const Ids* exact_set = nullptr)
    {
        Obl o;
        o.guard = guard;
        o.cond = std::move(cond);
        o.meta = std::move(m);
        // floor_: after the left operand of a comma, what it depends on came first (E6)
        o.dep = join(join(gdep, cdep), floor_);
        if (exact_set != nullptr)
        {
            o.dep.ex = join(o.dep.ex, *exact_set);
            if (call_pre_both_)
            {
                // decision 0039 item 2 (audit S3): the arguments write a cell
                o.dep.base = join(o.dep.base, *exact_set);
            }
        }
        o.seq = full.next_seq++;
        full.obls.push_back(std::move(o));
    }

    // ---- the memory model (decision 0032; decision 0028 items 1-4 and 11) ----

    // The name of a struct in a field or method name: `struct_<n>_<tag>` (StructType::name), never ending in `__<digits>` (so `v_ptr_<name>` cannot be a local's).
    static std::string struct_suffix(const StructType& st)
    {
        std::string name {st.name};
        const size_t us {name.rfind("__")};
        if (us != std::string::npos && name.find_first_not_of("0123456789", us + 2U) == std::string::npos)
        {
            name += "_e";
        }
        return name;
    }

    // The field-name suffix `<t>` of a cell type (decision 0032 item 5): the VIR type by width and signedness (`long` and `long long` share
    // `long`), an enumeration by its own, numbered name.  It never ends in `__<digits>`, so `v_<t>` cannot be the name of a local.
    std::string cell_suffix(const Type& t)
    {
        if (t.is_ptr())
        {
            // a pointer-typed object: its field is named by its pointee, at any depth (decision 0040 item 2)
            return "ptr_" + cell_suffix(*t.pointee);
        }
        if (t.is_struct())
        {
            // a pointer to a struct: `ptr_struct_<n>_<tag>` (decision 0043 item 3); a struct has no cell of its own
            return struct_suffix(*t.struct_type);
        }
        if (t.is_bool())
        {
            return "bool";
        }
        const IntType& it {*t.int_type};
        if (it.is_enum)
        {
            auto found {enum_suffix_.find(&it)};
            if (found == enum_suffix_.end())
            {
                std::string name {"enum_" + std::to_string(enum_suffix_.size() + 1U) + "_" + sanitize(it.name)};
                const size_t us {name.rfind("__")};
                if (us != std::string::npos && name.find_first_not_of("0123456789", us + 2U) == std::string::npos)
                {
                    name += "_e";
                }
                found = enum_suffix_.emplace(&it, std::move(name)).first;
            }
            return found->second;
        }
        switch (it.bits)
        {
            case 8:
                return it.is_signed ? "schar" : "uchar";
            case 16:
                return it.is_signed ? "short" : "ushort";
            case 32:
                return it.is_signed ? "int" : "uint";
            case 64:
                return it.is_signed ? "long" : "ulong";
            case 128:
                return it.is_signed ? "i128" : "u128";
            default:
                return (it.is_signed ? "i" : "u") + std::to_string(it.bits);
        }
    }

    // Record that the program uses cells of type `t` (its fields, and the memory prelude); `at` is a position of the first use that the
    // trusted `alloc_<t>` method's map rows can name: the earliest address-taken declaration of the type (a cell's type).
    std::string use_cell_type(const Type& t)
    {
        if (t.is_struct())
        {
            internal_error(SrcLoc {}, "a struct has no cell fields of its own: its leaves do (decision 0043 item 3)");
        }
        const std::string suffix {cell_suffix(t)};
        out_.memory = true;
        out_.cell_types[suffix] = ltype(t);
        cell_type_of_[suffix] = t;
        return suffix;
    }

    // The trusted allocation `alloc_struct_<n>_<tag>` of a struct local (decision 0043 item 4); its leaf types' fields are used.
    void alloc_struct_site(const StructType& st, const SrcLoc& at)
    {
        out_.memory = true;
        for (const StructLeaf& l : struct_leaves(st))
        {
            use_cell_type(l.type);
        }
        const std::string suffix {struct_suffix(st)};
        struct_alloc_[suffix] = &st;
        const auto it {alloc_at_.find(suffix)};
        const bool earlier {it == alloc_at_.end() || (at.file == it->second.file &&
                                                      (at.line < it->second.line ||
                                                       (at.line == it->second.line && at.column < it->second.column)))};
        if (earlier)
        {
            alloc_at_[suffix] = at;
        }
    }

    void alloc_site(const Type& t, const SrcLoc& at)
    {
        const std::string suffix {use_cell_type(t)};
        const auto it {alloc_at_.find(suffix)};
        const bool earlier {it == alloc_at_.end() || (at.file == it->second.file &&
                                                      (at.line < it->second.line ||
                                                       (at.line == it->second.line && at.column < it->second.column)))};
        if (earlier)
        {
            alloc_at_[suffix] = at;
        }
    }

    // `recv.v_<t>`, `recv.d_<t>`, `perm(recv.v_<t>)`, `lvloc(block_of(recv)).lv`.
    E cell_value(const E& recv, const Type& t)
    {
        return low::field(recv, "v_" + use_cell_type(t));
    }
    E cell_flag(const E& recv, const Type& t)
    {
        return low::field(recv, "d_" + use_cell_type(t));
    }
    E value_perm(const E& recv, const Type& t)
    {
        return low::perm_of(recv, "v_" + use_cell_type(t));
    }
    E flag_perm(const E& recv, const Type& t)
    {
        return low::perm_of(recv, "d_" + use_cell_type(t));
    }
    E block_of(const E& ptr)
    {
        out_.memory = true;
        return low::dom("block_of", {ptr});
    }
    E live_cell(const E& ptr)
    {
        out_.memory = true;
        return low::dom("lvloc", {block_of(ptr)});
    }
    E live_perm(const E& ptr)
    {
        return low::perm_of(live_cell(ptr), "lv");
    }
    // ---- structures (decision 0043 items 3-5) ----

    // `loc(member(base, k), 0)`: the leaf cell or nested slot of member `k` of the struct slot `base`.
    E member_ref(const E& base, int k)
    {
        out_.memory = true;
        return low::dom("loc", {low::dom("member", {base, low::int_lit(std::to_string(k))}), low::int_lit("0")});
    }
    // The same for a path of member indices (empty: `base` itself).
    E chain_ref(E base, const std::vector<int>& path)
    {
        for (int k : path)
        {
            base = member_ref(base, k);
        }
        return base;
    }
    // One cell of a struct object: the Ref of a leaf, its type and its spelling.
    struct Cell
    {
        E recv;
        Type type;
        std::string text;
        std::vector<int> path; // from the struct slot the cells were taken from
        std::string rel;       // its spelling from that slot: `x`, `t.x` (empty for a scalar place's own cell)
    };
    // The leaf cells of the struct object at `slot` of type `st`, in declaration order, depth first; `root_text` and `arrow` as struct_leaves.
    std::vector<Cell> leaf_cells(const E& slot, const StructType& st, const std::string& root_text, bool arrow)
    {
        std::vector<Cell> out;
        const std::vector<StructLeaf> rels {struct_leaves(st)};
        size_t i {0U};
        for (StructLeaf& l : struct_leaves(st, root_text, arrow))
        {
            out.push_back(Cell {chain_ref(slot, l.path), l.type, std::move(l.text), l.path, rels[i].text});
            ++i;
        }
        return out;
    }
    // The nested slots strictly below the struct slot `x`, at any depth, in declaration order, depth first (decision 0043 items 4 and 8: the slot facts).
    void slots_below(const E& x, const StructType& st, std::vector<E>& out)
    {
        for (size_t k {0U}; k < st.members.size(); ++k)
        {
            if (!st.members[k].type.is_struct())
            {
                continue;
            }
            const E y {member_ref(x, static_cast<int>(k))};
            out.push_back(y);
            slots_below(y, *st.members[k].type.struct_type, out);
        }
    }
    // `X != Y` for every slot X of the object at `slot` (the root first, then each nested slot depth first) and every nested slot Y below X: the order of the allocation's
    // slot clauses and of the slot facts of WF_S.  A struct with no nested member has none.  (S33 (i): without them Silicon cannot separate leaves at different depths.)
    void slot_pairs(const E& x, const StructType& st, std::vector<std::pair<E, E>>& out)
    {
        std::vector<E> below;
        slots_below(x, st, below);
        for (const E& y : below)
        {
            out.emplace_back(x, y);
        }
        for (size_t k {0U}; k < st.members.size(); ++k)
        {
            if (st.members[k].type.is_struct())
            {
                slot_pairs(member_ref(x, static_cast<int>(k)), *st.members[k].type.struct_type, out);
            }
        }
    }
    // `len(member(X, k)) == 1` for every member block of the struct slot `slot`, recursively through nested members, then the slot facts (WF_S, decision 0043 item 8).
    E struct_lens(const E& slot, const StructType& st)
    {
        E r;
        std::function<void(const E&, const StructType&)> lens = [&](const E& x, const StructType& s)
        {
            for (size_t k {0U}; k < s.members.size(); ++k)
            {
                const E mb {low::dom("member", {x, low::int_lit(std::to_string(k))})};
                r = low::and_(r, low::bin(low::BinOp::Eq, low::dom("len", {mb}), low::int_lit("1")));
                if (s.members[k].type.is_struct())
                {
                    lens(member_ref(x, static_cast<int>(k)), *s.members[k].type.struct_type);
                }
            }
        };
        lens(slot, st);
        std::vector<std::pair<E, E>> pairs;
        slot_pairs(slot, st, pairs);
        for (const auto& [x, y] : pairs)
        {
            r = low::and_(r, low::bin(low::BinOp::Ne, x, y));
        }
        out_.memory = true;
        return r;
    }

    // perm(recv.v_<t>) > none
    E may_read(const E& recv, const Type& t)
    {
        return low::bin(low::BinOp::Gt, value_perm(recv, t), low::no_perm());
    }
    // The write permission on both fields as ONE atomic assertion: !(perm(v) != write || perm(d) != write [|| perm(lv) != write]).
    E may_write(const E& recv, const Type& t, bool with_liveness)
    {
        using low::BinOp;
        E bad {low::bin(BinOp::Or, low::bin(BinOp::Ne, value_perm(recv, t), low::write_perm()),
                        low::bin(BinOp::Ne, flag_perm(recv, t), low::write_perm()))};
        if (with_liveness)
        {
            bad = low::bin(BinOp::Or, bad, low::bin(BinOp::Ne, live_perm(recv), low::write_perm()));
        }
        return low::not_(bad);
    }

    static std::string quoted(const std::string& text, const char* fallback)
    {
        return text.empty() ? std::string(fallback) : "`" + text + "`";
    }

    // The cells declared directly in a block (a Decl of an address-taken variable), in declaration order.
    static std::vector<const Var*> cells_of(const Stmt& block)
    {
        std::vector<const Var*> cells;
        for (const Stmt* c : block.stmts)
        {
            if (c != nullptr && c->kind == Stmt::Kind::Decl && c->var != nullptr && c->var->address_taken)
            {
                cells.push_back(c->var);
            }
        }
        return cells;
    }

    // `x := alloc_<t>()` at the entry of the block that declares the cell `x` (decision 0032 item 4).
    void alloc_cell(std::vector<low::LStmt>& out, const Var& x)
    {
        if (x.type.is_struct())
        {
            alloc_struct_site(*x.type.struct_type, x.loc);
        }
        else
        {
            alloc_site(x.type, x.loc);
        }
        low::LStmt c;
        c.kind = low::LStmt::Kind::Call;
        c.name = "alloc_" + cell_suffix(x.type);
        c.target = var_name(x);
        c.check = low::Check {
            "call.precondition",
            meta("internal", x.loc, "internal check: Silicon's own check of the allocation of `" + x.name + "`")};
        out.push_back(std::move(c));
    }

    // How a message names a cell or one of its members (`text`): `x`, or for the hidden object of a compound literal (Var::compound_literal, decision 0044 item 7: the encoder names the
    // local by the literal's spelling) "the compound literal `(struct S){1, 2}`", "a member of the compound
    // literal `...`" for a member: invariant 7, the hidden local's own name is never printed.
    static std::string object_phrase(const Var& x, const std::string& text)
    {
        if (!x.compound_literal)
        {
            return "`" + text + "`";
        }
        return text == x.name ? "the compound literal `" + x.name + "`"
                              : "a member of the compound literal `" + x.name + "`";
    }

    // Leaving the scope of `cells` (decision 0032 item 8): full permission to each cell, its flag and its liveness cell is back (`lifetime`),
    // then all three are exhaled; the cells in reverse declaration order.
    void scope_exit(std::vector<low::LStmt>& out, const std::vector<const Var*>& cells, const SrcLoc& at)
    {
        for (size_t k {cells.size()}; k-- > 0U;)
        {
            const Var& x {*cells[k]};
            const E r {low::var(var_name(x))};
            // decision 0043 item 4: every leaf, then the liveness cell, checked at `write` (one atomic assertion each)
            // and exhaled together
            if (x.type.is_struct())
            {
                E gone;
                for (const Cell& c : leaf_cells(r, *x.type.struct_type, x.name, false))
                {
                    emit_assert(out, may_write(c.recv, c.type, false),
                                meta("lifetime", at,
                                     "the address of " + object_phrase(x, c.text) +
                                         " may still be in use when its lifetime ends"));
                    gone = low::and_(gone, low::and_(low::acc(c.recv, "v_" + cell_suffix(c.type)),
                                                     low::acc(c.recv, "d_" + cell_suffix(c.type))));
                }
                emit_assert(
                    out, low::bin(low::BinOp::Eq, live_perm(r), low::write_perm()),
                    meta("lifetime", at,
                         "the address of " + object_phrase(x, x.name) + " may still be in use when its lifetime ends"));
                low::LStmt ex;
                ex.kind = low::LStmt::Kind::Exhale;
                ex.expr = low::and_(gone, low::acc(live_cell(r), "lv"));
                out.push_back(std::move(ex));
                continue;
            }
            emit_assert(
                out, may_write(r, x.type, true),
                meta("lifetime", at,
                     "the address of " + object_phrase(x, x.name) + " may still be in use when its lifetime ends"));
            low::LStmt ex;
            ex.kind = low::LStmt::Kind::Exhale;
            ex.expr = low::bin(low::BinOp::And,
                               low::bin(low::BinOp::And, low::acc(r, "v_" + cell_suffix(x.type)),
                                        low::acc(r, "d_" + cell_suffix(x.type))),
                               low::acc(live_cell(r), "lv"));
            out.push_back(std::move(ex));
        }
    }

    void field_assign(std::vector<low::LStmt>& out, const std::string& recv, const std::string& field, E value)
    {
        low::LStmt s;
        s.kind = low::LStmt::Kind::FieldAssign;
        s.name = recv;
        s.field = field;
        s.expr = std::move(value);
        out.push_back(std::move(s));
    }

    // `x.v_<t> := value; x.d_<t> := true` for a Ref variable (the lint requires the flag write to follow the value's directly).
    void write_fields(std::vector<low::LStmt>& out, const std::string& recv, const Type& t, E value)
    {
        const std::string suffix {use_cell_type(t)};
        field_assign(out, recv, "v_" + suffix, std::move(value));
        field_assign(out, recv, "d_" + suffix, low::bool_lit(true));
    }

    // A cell read by name: its own permission is the function's (`internal`), then the flag (`uninitialized`), value `x.v_<t>`.
    LV read_cell(const Expr& e, const E& guard, const Deps& gdep, Full& full)
    {
        const E r {low::var(var_name(*e.var))};
        obl(full, guard, gdep, may_read(r, e.var->type), {},
            meta("permission", e.loc,
                 "reads `" + e.var->name + "` without owning it: a callee may have kept its object"));
        obl(full, guard, gdep, cell_flag(r, e.var->type), {},
            meta("uninitialized", e.loc, "`" + e.var->name + "` may be read here before it is given a value"));
        return mem_read(e, key_of_cell(*e.var), r, e.var->type, e.var->name, guard, gdep, {}, full);
    }

    // ---- loop invariants and measures over memory (decision 0038 item 6) ----

    // The loop's places and the heap reads recorded so far, set by loop() around the lowering of an invariant or a measure.
    const std::vector<Place>* inv_places_ {nullptr};
    // The loop-entry snapshots of the loops being lowered (decision 0045 item 10): a `requires` site and the Ref of its place through `lb`; the body's loads are split
    // against them like against every automatic place.
    struct LbPlace
    {
        const AtomSite* site;
        E ref;
    };
    std::vector<LbPlace> active_lbs_;
    std::vector<const WAtom*> active_w_; // the written atoms of the loops being lowered, outermost first
    std::vector<InvRead> inv_reads_;
    std::string inv_what_ {"invariant"}; // "invariant" or "measure": the word of the messages

    static E inv_dummy(const Type& t)
    {
        return t.is_ptr() ? low::null_() : (t.is_bool() ? low::bool_lit(false) : low::int_lit("0"));
    }
    E inv_amount_positive(const Place& pl)
    {
        return low::bin(low::BinOp::Gt, low::var(pl.kv), low::no_perm());
    }

    // A cell read by name in an invariant or a measure: the value, guarded by its own condition `kv > none` (a chain of one disjunct), and its flag as an
    // `uninitialized` obligation clause (chain-guarded too).  The explicit framing assert (permission) is loop()'s.
    LV inv_cell(const Expr& e, const E& guard, const Deps& gdep, Full& full)
    {
        const Place* pl {nullptr};
        if (inv_places_ != nullptr)
        {
            for (const Place& p : *inv_places_)
            {
                if (p.cell && p.var == e.var)
                {
                    pl = &p;
                }
            }
        }
        if (pl == nullptr)
        {
            internal_error(e.loc, "a cell read in a loop annotation has no place in the loop's frame");
            return {inv_dummy(e.type), {}};
        }
        InvRead r;
        r.cell = e.var;
        r.type = e.var->type;
        r.loc = e.loc;
        r.text = e.var->name;
        r.guard = guard;
        inv_reads_.push_back(std::move(r));
        // The flag (decision 0038 item 6, amended): an invariant that reads a cell states that it is initialized.  The obligation is an invariant
        // clause (`uninitialized`), its flag read chain-guarded like the value, so no `perm` appears.
        obl(full, guard, gdep,
            low::ite(low::bin(low::BinOp::Gt, low::var(pl->kd), low::no_perm()),
                     cell_flag(low::var(var_name(*e.var)), e.var->type), low::bool_lit(false)),
            {},
            meta("uninitialized", e.loc,
                 "this loop " + inv_what_ + " reads `" + e.var->name + "` before it is given a value"));
        return {
            low::ite(inv_amount_positive(*pl), cell_value(low::var(var_name(*e.var)), e.var->type), inv_dummy(e.type)),
            {}};
    }

    // `*q` in an invariant or a measure: the chain `(q == x_1 && kv_1 > none ? q.v : (q == x_2 && ... : dummy))` over the places of the field.
    LV inv_deref(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        LV p {lower(*e.a, env, guard, gdep, full)};
        const Type& t {e.type};
        const std::string sfx {cell_suffix(t)};
        InvRead r;
        r.ptr = p.e;
        r.type = t;
        r.loc = e.loc;
        r.text = e.c_text.empty() ? (e.a->c_text.empty() ? std::string() : "*" + e.a->c_text) : e.c_text;
        r.guard = guard;
        inv_reads_.push_back(r);
        const E read {cell_value(p.e, t)};
        E v {inv_dummy(t)};
        if (inv_places_ != nullptr)
        {
            std::vector<const Place*> ps;
            for (const Place& pl : *inv_places_)
            {
                if (pl.value && pl.suffix == sfx)
                {
                    ps.push_back(&pl);
                }
            }
            for (size_t i {ps.size()}; i-- > 0U;)
            {
                v = low::ite(inv_place_disjunct(p.e, *ps[i]), read, v);
            }
            // The flag, chain-guarded the same way (see inv_cell): an `uninitialized` obligation clause.
            E f {low::bool_lit(false)};
            for (size_t i {ps.size()}; i-- > 0U;)
            {
                f = low::ite(
                    low::and_(ps[i]->wg, low::and_(low::bin(low::BinOp::Eq, p.e, ps[i]->recv),
                                                   low::bin(low::BinOp::Gt, low::var(ps[i]->kd), low::no_perm()))),
                    cell_flag(p.e, t), f);
            }
            obl(full, guard, gdep, f, p.dep,
                meta("uninitialized", e.loc,
                     "this loop " + inv_what_ + " reads " + quoted(r.text, "an object") +
                         " before it is given a value"));
        }
        else
        {
            obl(full, guard, gdep, low::bool_lit(false), p.dep,
                meta("uninitialized", e.loc,
                     "this loop " + inv_what_ + " reads " + quoted(r.text, "an object") +
                         " before it is given a value"));
        }
        return {v, p.dep};
    }

    // `q == place && kv > none`: the pointer designates the place, which the loop holds.
    E inv_place_disjunct(const E& q, const Place& pl)
    {
        if (is_global_ref(q) && same_expr(q, pl.recv))
        {
            return low::and_(pl.wg, inv_amount_positive(pl)); // a global's own place: `q == q` is true (decision 0041)
        }
        return low::and_(pl.wg,
                         low::bin(low::BinOp::And, low::bin(low::BinOp::Eq, q, pl.recv), inv_amount_positive(pl)));
    }

    // The explicit framing condition of one read (decision 0038 item 6): `prior && guard ==> OR_i D_i`.
    E framing_condition(const InvRead& r, const E& prior)
    {
        E any;
        if (r.cell != nullptr)
        {
            for (const Place& pl : *inv_places_)
            {
                if (pl.cell && pl.var == r.cell)
                {
                    any = inv_amount_positive(pl);
                }
            }
            if (!any)
            {
                any = low::bool_lit(false);
            }
        }
        else
        {
            const std::string sfx {cell_suffix(r.type)};
            for (const Place& pl : *inv_places_)
            {
                if (!pl.value || pl.suffix != sfx)
                {
                    continue;
                }
                const E d {inv_place_disjunct(r.ptr, pl)};
                any = any ? low::bin(low::BinOp::Or, any, d) : d;
            }
            if (!any)
            {
                any = low::bool_lit(false);
            }
        }
        return low::implies(low::and_(prior, r.guard), any);
    }

    // A cell written by name (`x = e;`, the declaration's initializer, `x += e;`), after the full expression of its value: the permission is
    // the function's (`internal`), then both fields are written.
    void write_cell(std::vector<low::LStmt>& out, const Var& x, const E& value, const SrcLoc& at)
    {
        const E r {low::var(var_name(x))};
        emit_assert(
            out, may_write(r, x.type, false),
            meta("permission", at, "writes `" + x.name + "` without owning it: a callee may have kept its object"));
        write_fields(out, var_name(x), x.type, value);
    }

    static bool is_global_addr(const Expr& a)
    {
        return a.kind == Expr::Kind::GlobalAddr && a.global != nullptr;
    }
    // The address names a global or a member of one (`g`, `g.x`, `g.t.y`): the messages say what the contract needs (decision 0044 item 8).
    static bool rooted_at_global(const Expr& a)
    {
        const Expr* root {member_root(&a, nullptr)};
        return root != nullptr && root->kind == Expr::Kind::GlobalAddr && root->global != nullptr;
    }

    // `*p` as a read (decision 0032 item 6): the null check, the permission check (which is also the liveness check of this use, 0028 item
    // 3a), the flag; the value `p.v_<t>`.
    LV deref(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        if (cmode_ == CMode::Inv)
        {
            return inv_deref(e, env, guard, gdep, full);
        }
        if (cmode_ != CMode::None)
        {
            return contract_read(e, env, guard, gdep, full);
        }
        // `p->x`: the null check is the chain's, on its root pointer
        const bool member {e.a->kind == Expr::Kind::MemberAddr};
        if (member)
        {
            place_nodes_[e.a] = e.loc;
        }
        LV p {lower(*e.a, env, guard, gdep, full)};
        const std::string ptr_text {e.a->c_text};
        const std::string place {e.c_text.empty() ? (ptr_text.empty() ? std::string() : "*" + ptr_text) : e.c_text};
        const Type& t {e.type};
        if (!member)
        {
            obl(full, guard, gdep, low::bin(low::BinOp::Ne, p.e, low::null_()), p.dep,
                meta("null-dereference", e.loc,
                     "dereferences " + quoted(ptr_text, "the pointer") + ", which may be null"));
        }
        obl(full, guard, gdep, may_read(p.e, t), p.dep,
            meta("permission", e.loc,
                 rooted_at_global(*e.a) ? "reads `" + place + "` without owning it: `" + fn_.f->c_name +
                                              "` needs `reads(" + place + ")` or `owns(" + place + ")` in its contract"
                                        : "reads " + quoted(place, "the object the pointer points to") +
                                              " without owning it: the object it points to may have ended"));
        obl(full, guard, gdep, cell_flag(p.e, t), p.dep,
            meta("uninitialized", e.loc,
                 quoted(place, "the object the pointer points to") + " may be read here before it is given a value"));
        return mem_read(e, key_of_addr(*e.a), p.e, t, place, guard, gdep, p.dep, full, e.a);
    }

    // Any other use of a pointer value (`==`, `!=`, a truth test, a copy): `p != null ==> perm(lvloc(block_of(p)).lv) > none` (`lifetime`),
    // skipped for `&x` and a null constant, which cannot have a dead object (decision 0032 item 6).
    static bool address_or_null(const Expr& e)
    {
        // A MemberAddr is an address formed from its root pointer, whose own liveness was checked at the formation (decision 0043 item 5).
        if (e.kind == Expr::Kind::AddrOf || e.kind == Expr::Kind::Null || e.kind == Expr::Kind::GlobalAddr ||
            e.kind == Expr::Kind::MemberAddr)
        {
            return true;
        }
        return e.kind == Expr::Kind::Cond && e.b != nullptr && e.c != nullptr && address_or_null(*e.b) &&
               address_or_null(*e.c);
    }

    // `with_wf`: the use is a store into a pointer cell, whose value must also be well-formed (decision 0040 item 2): `WF(v)` joins the same row, so a
    // pointer nothing says is well-formed (a reassigned pointer after a loop with no invariant about it) is the user's `lifetime` error, never an internal one.
    void ptr_use(const Expr& operand, const LV& v, const E& guard, const Deps& gdep, Full& full, bool with_wf = false,
                 const SrcLoc* at = nullptr)
    {
        if (cmode_ != CMode::None)
        {
            return; // a contract has no heap and no liveness to check (decision 0037 item 7)
        }
        if (address_or_null(operand))
        {
            return;
        }
        if (heap_ && track_mem())
        {
            // Decision 0047 item 9: a pointer use reads the liveness cell of its block; a call that ends the block (free, a consuming `owns_block` site) writes it, so the
            // two in one full expression are unsequenced unless the pointers name different blocks (a row `lvloc(block_of(p)) != lvloc(block_of(q))`).
            Acc a;
            a.key = -1000000 - (++next_other_key_);
            a.field = "lv";
            a.place = live_cell(v.e);
            a.text = operand.c_text;
            a.dep = join(gdep, v.dep);
            a.loc = at != nullptr ? *at : operand.loc;
            register_access(std::move(a), full, guard);
        }
        E live {low::bin(low::BinOp::Gt, live_perm(v.e), low::no_perm())};
        // A unit with globals: a pointer into a global's block is live whatever the function holds (decision 0041 item 6).
        if (const E g = global_live_disjunct(v.e))
        {
            live = low::bin(low::BinOp::Or, live, g);
        }
        if (with_wf)
        {
            // one atomic assertion
            live = low::not_(low::bin(low::BinOp::Or, low::not_(live), low::not_(wf_expr(v.e, &operand.type))));
        }
        obl(full, guard, gdep, low::implies(low::bin(low::BinOp::Ne, v.e, low::null_()), live), v.dep,
            meta("lifetime", at != nullptr ? *at : operand.loc,
                 "uses " + quoted(operand.c_text, "the pointer") + " after the object it points to may have ended"));
    }

    // 0028 item 3's exception for `==` and `!=` (decision 0032 item 7): two non-null pointers into different blocks, one of them one past the
    // end of its block, are not decided by the model.  In b1a every pointer is null or offset 0 of a one-slot block, so it is provable; as an
    // `internal` row, a failure is a bug (exit 3).  Never for a null operand.
    void one_past_end(const Expr& e, const LV& a, const LV& b, const E& guard, const Deps& gdep, const Deps& dep,
                      Full& full)
    {
        using low::BinOp;
        if (e.a->kind == Expr::Kind::Null || e.b->kind == Expr::Kind::Null)
        {
            return;
        }
        out_.memory = true;
        auto past = [&](const E& p)
        {
            return low::bin(BinOp::Eq, low::dom("offset_of", {p}), low::dom("len", {block_of(p)}));
        };
        E bad {low::bin(BinOp::And, low::bin(BinOp::Ne, a.e, low::null_()), low::bin(BinOp::Ne, b.e, low::null_()))};
        bad = low::bin(BinOp::And, bad, low::bin(BinOp::Ne, block_of(a.e), block_of(b.e)));
        bad = low::bin(BinOp::And, bad, low::bin(BinOp::Or, past(a.e), past(b.e)));
        obl(full, guard, gdep, low::not_(bad), dep,
            meta("pointer-comparison", e.loc,
                 "compares " + quoted(e.a->c_text, "a pointer") + " and " + quoted(e.b->c_text, "a pointer") +
                     ", which may point into different objects, one of them one past its end"));
    }

    static bool shares_node(const Expr& root, const Expr* needle)
    {
        if (root.kind == Expr::Kind::Deref && root.a == needle)
        {
            return true;
        }
        for (const Expr* x : {root.a, root.b, root.c})
        {
            if (x != nullptr && shares_node(*x, needle))
            {
                return true;
            }
        }
        for (const Expr* x : root.args)
        {
            if (x != nullptr && shares_node(*x, needle))
            {
                return true;
            }
        }
        return false;
    }

    // `*addr = value;` and its compound forms (decision 0032 items 6 and 9): the address and the value in one full expression, then (a
    // non-variable address copied into a Ref temporary) the null check, the write permission on both fields, the two writes.  A compound
    // store's value reads `*addr` through the same node: the pointer is lowered once and the read reuses its value (`shared_`).
    // `*addr = <struct value>;` (decision 0044 item 5): the address and the value in one full expression (the value a call, an aggregate or a member chain over a
    // call), every value leaf into a temporary before the first destination leaf is written (`s = (struct S){s.y, s.x}`), then the base's null check once and, per
    // leaf, its write permission and the two field writes through a Ref temporary.
    void struct_store(const Stmt& s, std::vector<low::LStmt>& out)
    {
        const StructType& st {*s.expr->type.struct_type};
        StoreTarget target;
        target.addr = s.addr;
        target.loc = s.loc;
        target.st = &st;
        // `p->t = f();`: the null check is the chain's (decision 0043 item 5)
        const bool member {s.addr->kind == Expr::Kind::MemberAddr};
        if (member)
        {
            place_nodes_[s.addr] = s.loc;
        }
        agg_wf_ = true;
        const std::vector<LV> vs {full_exprs({s.addr, s.expr}, out, nullptr, false, &target, nullptr, false, 1, true)};
        agg_wf_ = false;
        E base {vs[0U].e};
        if (base->kind != low::LExpr::Kind::Var)
        {
            const std::string t {new_temp(low::LType::Ref)};
            assign(out, t, base);
            base = low::var(t);
        }
        const auto [root_text, arrow] = struct_dest_root(*s.addr);
        const std::vector<StructLeaf> plain {struct_leaves(st)};
        const std::vector<StructLeaf> named {struct_leaves(st, root_text, arrow)};
        if (vs[1U].leaves.size() != plain.size())
        {
            internal_error(
                s.loc,
                "a struct store whose value has a different number of leaves than its destination reached lowering");
            return;
        }
        if (!member)
        {
            emit_assert(out, low::bin(low::BinOp::Ne, base, low::null_()),
                        meta("null-dereference", s.loc,
                             "dereferences " + quoted(s.addr->c_text, "the pointer") + ", which may be null"));
        }
        std::vector<E> values;
        for (size_t i {0U}; i < plain.size(); ++i)
        {
            E v {vs[1U].leaves[i]};
            if (v->kind != low::LExpr::Kind::Var)
            {
                const std::string t {new_temp(ltype(plain[i].type))};
                assign(out, t, v);
                v = low::var(t);
            }
            values.push_back(v);
        }
        for (size_t i {0U}; i < plain.size(); ++i)
        {
            const std::string t {new_temp(low::LType::Ref)};
            assign(out, t, chain_ref(base, plain[i].path));
            emit_assert(out, may_write(low::var(t), plain[i].type, false),
                        meta("permission", s.loc,
                             rooted_at_global(*s.addr)
                                 ? "writes `" + named[i].text + "` without owning it: `" + fn_.f->c_name +
                                       "` needs `owns(" + named[i].text + ")` in its contract"
                                 : "writes " + quoted(named[i].text, "the object the pointer points to") +
                                       " without owning it: the object it points to may have ended"));
            write_fields(out, t, plain[i].type, values[i]);
        }
    }

    void store(const Stmt& s, std::vector<low::LStmt>& out)
    {
        if (s.expr->type.is_struct())
        {
            struct_store(s, out);
            return;
        }
        // the value reads `*addr` through the same node: the pointer is evaluated once
        const bool compound {shares_node(*s.expr, s.addr)};
        StoreTarget target;
        target.addr = s.addr;
        target.loc = s.loc;
        // `p->x = e;`: the null check is the chain's (decision 0043 item 5)
        const bool member {s.addr->kind == Expr::Kind::MemberAddr};
        if (member)
        {
            place_nodes_[s.addr] = s.loc;
        }
        const std::vector<LV> vs {
            full_exprs({s.addr, s.expr}, out, nullptr, compound, &target, s.expr->type.is_ptr() ? s.expr : nullptr)};
        E recv {vs[0U].e};
        if (recv->kind != low::LExpr::Kind::Var)
        {
            const std::string t {new_temp(low::LType::Ref)};
            assign(out, t, recv);
            recv = low::var(t);
        }
        const Type& t {s.expr->type};
        const std::string ptr_text {s.addr->c_text};
        const std::string place {place_text(*s.addr)};
        // Always checked: the read of a compound store checked the same value, but the write's own check costs nothing and is never skipped.
        if (!member)
        {
            emit_assert(out, low::bin(low::BinOp::Ne, recv, low::null_()),
                        meta("null-dereference", s.loc,
                             "dereferences " + quoted(ptr_text, "the pointer") + ", which may be null"));
        }
        emit_assert(out, may_write(recv, t, false),
                    meta("permission", s.loc,
                         rooted_at_global(*s.addr) ? "writes `" + place + "` without owning it: `" + fn_.f->c_name +
                                                         "` needs `owns(" + place + ")` in its contract"
                                                   : "writes " + quoted(place, "the object the pointer points to") +
                                                         " without owning it: the object it points to may have ended"));
        write_fields(out, recv->text, t, vs[1U].e);
    }

    // ---- contracts with pointers (decision 0037) ----

    // How a contract expression is being lowered.  Method: a clause of the function's own Viper method (atoms are `acc` assertions, reads are heap
    // reads, `old` is Viper's); Wf: the well-formedness method (no heap: reads are ghost value formals, atoms their pure facts); Call: a callee's
    // value `requires` over the argument values at a call (the atoms were asserted one by one, so they lower to `true`); Ret: a value `ensures`
    // asserted at a `return` (atoms were asserted, `old` is Viper's).  None: code.  Inv: a loop invariant or measure (decision 0038 item 6): `==` is
    // identity, a pointer use has no liveness check, and a cell or `*q` read is a chain over the framing disjuncts with an `uninitialized` obligation clause for its flag.
    enum class CMode
    {
        None,
        Method,
        Wf,
        Call,
        Ret,
        Inv
    };

    struct FnAtoms
    {
        std::vector<AtomSite> sites;         // contract_atoms: the requires atoms first
        std::map<const Expr*, size_t> index; // the Atom node -> its position in `sites`
        int nreq {0};                        // the number of requires atoms (the ghost amounts)
    };

    // Decision 0045 item 2 (places reached through loads or pointer leaves): every distinct (root, via) of a contract place gets a per-function DERIVED key
    // `kDerivedKeyBase + n` (n global to the lowering; `local` is the function's own number, which names the ghosts and the entry snapshots), so the integer-keyed
    // consumers treat a base as one more root.  `base` is the base pointer expression of the first place that named it; lowering it in the current contract mode
    // gives the base pointer's value (a chain-guarded read, a ghost value or an argument's read), so a derived key's `Ref` is never a variable.
    struct Derived
    {
        const Function* f {nullptr};
        PlaceKey key;               // root and via; the path is empty
        const Expr* base {nullptr}; // the base pointer (a Deref, a Field, ...)
        Type base_type;             // its type (a pointer)
        bool has_load {false};      // the steps contain a heap read
        int local {0};              // the function's own number of this entry
        // >= 0: this key is the LOOP-ENTRY snapshot `lb` of the derived key `lb_of` (decision 0045 item 10)
        int lb_of {-1};
    };
    static bool is_derived_key(int k)
    {
        return k >= kDerivedKeyBase;
    }
    const Derived& derived_of(int k) const
    {
        return derived_[static_cast<size_t>(k - kDerivedKeyBase)];
    }
    // The key of the place (`root`, `via`) in `f`, created in evaluation order the first time.
    int derived_key(const Function& f, int root, const std::vector<int>& via, const Expr* base)
    {
        const auto id {std::make_tuple(&f, root, via)};
        const auto it {derived_index_.find(id)};
        if (it != derived_index_.end())
        {
            return it->second;
        }
        Derived d;
        d.f = &f;
        d.key.root = root;
        d.key.via = via;
        d.base = base;
        d.base_type = base->type;
        d.has_load = steps_have_load(via);
        d.local = derived_count_[&f]++;
        derived_.push_back(d);
        return derived_index_[id] = kDerivedKeyBase + static_cast<int>(derived_.size()) - 1;
    }
    static int root_key_of(const Expr* root)
    {
        if (root == nullptr)
        {
            return kNoPlaceKey;
        }
        switch (root->kind)
        {
            case Expr::Kind::Formal:
                return root->index;
            case Expr::Kind::GlobalAddr:
                return root->global != nullptr ? kGlobalSiteBase + root->global->id : kNoPlaceKey;
            case Expr::Kind::Result:
                return kResultSiteKey;
            default:
                return kNoPlaceKey;
        }
    }
    // The derived entries of a contract expression in evaluation order: every atom whose place has steps, every read through a load or a pointer leaf.
    void collect_derived(const Function& f, const Expr* e)
    {
        if (e == nullptr)
        {
            return;
        }
        if (e->kind == Expr::Kind::Atom && e->a != nullptr)
        {
            const PlaceKey pk {atom_place(*e)};
            if (!pk.via.empty() && pk.valid())
            {
                const Expr* base {member_root(e->a)};
                derived_of_atom_[e] = derived_key(f, pk.root, pk.via, base);
            }
        }
        else if (e->kind == Expr::Kind::Deref && e->a != nullptr)
        {
            std::vector<int> path;
            std::vector<int> via;
            const Expr* base {member_root(e->a, &path)};
            const Expr* root {place_root(base, &via)};
            if (!via.empty() && root_key_of(root) != kNoPlaceKey)
            {
                derived_key(f, root_key_of(root), via, base);
            }
        }
        for (const Expr* x : {e->a, e->b, e->c})
        {
            collect_derived(f, x);
        }
        for (const Expr* x : e->args)
        {
            collect_derived(f, x);
        }
    }

    const FnAtoms& atoms_of(const Function& f)
    {
        const auto it {atoms_.find(&f)};
        if (it != atoms_.end())
        {
            return it->second;
        }
        FnAtoms fa;
        fa.sites = contract_atoms(f);
        for (size_t i {0U}; i < fa.sites.size(); ++i)
        {
            fa.index[fa.sites[i].atom] = i;
            site_fn_[fa.sites[i].atom] = &f;
            if (!fa.sites[i].in_ensures)
            {
                ++fa.nreq;
            }
        }
        for (const Clause& c : f.requires_)
        {
            collect_derived(f, c.expr);
        }
        for (const Clause& c : f.ensures_)
        {
            collect_derived(f, c.expr);
        }
        for (const Clause& c : f.decreases_)
        {
            collect_derived(f, c.expr);
        }
        index_block_order(f);
        return atoms_.emplace(&f, std::move(fa)).first->second;
    }

    // ---- the heap (decision 0047, M4.5d2a) ----

    // The `OwnsBlock` sites of a function's contract (`contract_block_atoms`: the `requires` ones first); they have no ghost amount and are not in FnAtoms.
    struct FnBlocks
    {
        std::vector<AtomSite> sites;
        std::map<const Expr*, size_t> index;
        int nreq {0}; // the `requires` sites come first
    };
    const FnBlocks& blocks_of(const Function& f)
    {
        const auto it {blocks_.find(&f)};
        if (it != blocks_.end())
        {
            return it->second;
        }
        FnBlocks fb;
        fb.sites = contract_block_atoms(f);
        for (size_t i {0U}; i < fb.sites.size(); ++i)
        {
            fb.index[fb.sites[i].atom] = i;
            site_fn_[fb.sites[i].atom] = &f;
            if (!fb.sites[i].in_ensures)
            {
                ++fb.nreq;
            }
        }
        return blocks_.emplace(&f, std::move(fb)).first->second;
    }
    // One evaluation order over both site lists (decision 0047 item 4; audit round 2, S3): for each permission atom of `f`'s contract, the number of `owns_block` sites before
    // it, which is the position in `blocks_of(f).sites` of the first block site after it.  The walk is vir.cpp's AtomCollector's (`&&`; `||` and `?:` into their arms), the
    // requires clauses first and then the ensures; a place or a guard of an atom that is read through a load is framed by the atoms before it, block sites included.
    void index_block_order(const Function& f)
    {
        const FnBlocks& fb {blocks_of(f)};
        size_t n {0U};
        std::function<void(const Expr*)> walk = [&](const Expr* e)
        {
            if (e == nullptr)
            {
                return;
            }
            if (e->kind == Expr::Kind::Atom)
            {
                if (fb.index.count(e) != 0U)
                {
                    ++n;
                }
                else
                {
                    blocks_before_[e] = n;
                }
            }
            else if (e->kind == Expr::Kind::Binary && (e->bop == BinaryOp::LAnd || e->bop == BinaryOp::LOr))
            {
                walk(e->a);
                walk(e->b);
            }
            else if (e->kind == Expr::Kind::Cond)
            {
                walk(e->b);
                walk(e->c);
            }
        };
        for (const Clause& c : f.requires_)
        {
            walk(c.expr);
        }
        for (const Clause& c : f.ensures_)
        {
            walk(c.expr);
        }
    }
    size_t block_sites_before(const Expr* atom) const
    {
        const auto it {blocks_before_.find(atom)};
        return it == blocks_before_.end() ? 0U : it->second;
    }
    // The position of the first block site at or after clause `clause` of the requires (`ens` false) or ensures list (first_site's twin).
    static size_t first_block_site(const FnBlocks& fb, bool ens, int clause)
    {
        for (size_t i {0U}; i < fb.sites.size(); ++i)
        {
            const AtomSite& s {fb.sites[i]};
            if (s.in_ensures == ens && s.clause >= clause)
            {
                return i;
            }
            if (s.in_ensures && !ens)
            {
                return i;
            }
        }
        return fb.sites.size();
    }
    // A heap program (item 7, audit S8): some function's contract holds an `owns_block` site (a view's, an owning function's, a consumer's), called or not.
    bool is_heap_program()
    {
        for (const Function& f : prog_.functions())
        {
            if (!blocks_of(f).sites.empty())
            {
                return true;
            }
        }
        return false;
    }
    // The key of a block site's pointer: the formal's index, or kResultSiteKey.
    static int block_key(const AtomSite& s)
    {
        return atom_place(*s.atom).root;
    }
    static const Type& block_ptr_type(const AtomSite& s)
    {
        return s.atom->a->type;
    }
    static const Type& block_pointee(const AtomSite& s)
    {
        return *s.atom->a->type.pointee;
    }
    E block_ptr(const AtomSite& s, const Env& env) const
    {
        return place_of(env, block_key(s));
    }
    // P4222's marker of the pointer a block site names: a parameter's, or the result's (`result_init`).
    static InitMarker block_marker(const Function& f, const AtomSite& s)
    {
        const int k {block_key(s)};
        if (k < 0)
        {
            return f.result_init;
        }
        return static_cast<size_t>(k) < f.params.size() ? f.params[static_cast<size_t>(k)].init : InitMarker::None;
    }
    // The `requires` block site on the same formal as the `ensures` site `s` (its partner: the block is borrowed), or null.
    const AtomSite* block_partner(const Function& f, const AtomSite& s)
    {
        if (!s.in_ensures || block_key(s) < 0)
        {
            return nullptr;
        }
        const FnBlocks& fb {blocks_of(f)};
        for (int i {0}; i < fb.nreq; ++i)
        {
            if (block_key(fb.sites[static_cast<size_t>(i)]) == block_key(s))
            {
                return &fb.sites[static_cast<size_t>(i)];
            }
        }
        return nullptr;
    }
    // A consuming site: a `requires` block site without an `ensures` partner (the function frees the block or hands it on).
    bool block_consumes(const Function& f, const AtomSite& s)
    {
        if (s.in_ensures)
        {
            return false;
        }
        for (const AtomSite& o : blocks_of(f).sites)
        {
            if (o.in_ensures && block_key(o) == block_key(s))
            {
                return false;
            }
        }
        return true;
    }
    // The leaf cells of the block `p` points to (the pointee of `pt`): the leaves of a struct (`p->x`), or the one cell of a scalar (`*p`).  `text` is the pointer's spelling.
    std::vector<Cell> block_cells(const E& p, const Type& pt, const std::string& text)
    {
        const Type& t {*pt.pointee};
        if (t.is_struct())
        {
            return leaf_cells(p, *t.struct_type, text, true);
        }
        return {Cell {p, t, text.empty() ? std::string() : "*" + text, {}, ""}};
    }
    std::vector<Cell> block_cells(const AtomSite& s, const E& p)
    {
        return block_cells(p, block_ptr_type(s), s.atom->a->c_text);
    }
    // `loc(block_of(p), -1)`: the slot of a block's free token (decision 0028 item 6).
    E tok_slot(const E& p)
    {
        return low::dom("loc", {block_of(p), low::int_lit("-1")});
    }
    E tok_loc_slot(const E& p)
    {
        return low::dom("loc", {block_of(p), low::int_lit("-2")});
    }
    // `len(block_of(p)) == 1`, and for a structure 0043's member lengths and slot facts.
    E block_lens(const E& p, const Type& t)
    {
        E r {low::bin(low::BinOp::Eq, low::dom("len", {block_of(p)}), low::int_lit("1"))};
        if (t.is_struct())
        {
            r = low::and_(r, struct_lens(p, *t.struct_type));
        }
        return r;
    }
    // The block part of `owns_block(p)` (item 4), pure facts only: `p != null`, WF, offset 0, the liveness cell at `loc(b, -2)`, the lengths.
    E block_pure(const E& p, const Type& pt)
    {
        E r {low::bin(low::BinOp::Ne, p, low::null_())};
        r = low::and_(r, wf_expr(p));
        r = low::and_(r, low::bin(low::BinOp::Eq, low::dom("offset_of", {p}), low::int_lit("0")));
        r = low::and_(r, low::bin(low::BinOp::Eq, live_cell(p), tok_loc_slot(p)));
        r = low::and_(r, block_lens(p, *pt.pointee));
        return r;
    }
    // The range (or WF, for a pointer) of a leaf's value; null for a bool.
    E leaf_range(const Cell& c)
    {
        if (c.type.is_ptr())
        {
            return wf_expr(cell_value(c.recv, c.type), &c.type);
        }
        return c.type.is_bool() ? E() : range_check(cell_value(c.recv, c.type), *c.type.int_type);
    }
    // `owns_block(p)` as a conjunction, in the shape S36 verified: the block part (`block_head`: the token, the liveness cell at full permission), the lengths, then per leaf
    // full permission to the value and the flag and `flag ==> range`, then (`Plain`) the flags.  `Uninit`: no flag (a `ref_to_uninit` pointer's block: malloc's and free's);
    // `Kept`: an `ensures` site on a `ref_to_uninit` formal: a flag that was set on entry (under the partner's guard `kept_guard`) is still set.
    enum class BlockMode
    {
        Plain,
        Uninit,
        Kept
    };
    // `vlibc`: a view of vlibc's allocator (decision 0050 item 3): its token's value is `false`; every other `owns_block` states `true`, the platform's allocator.
    E block_conj(const E& p, const Type& pt, const std::string& text, BlockMode mode, const E& kept_guard, bool vlibc)
    {
        E r {low::bin(low::BinOp::Ne, p, low::null_())};
        r = low::and_(r, wf_expr(p));
        r = low::and_(r, low::bin(low::BinOp::Eq, low::dom("offset_of", {p}), low::int_lit("0")));
        r = low::and_(r, low::acc(tok_slot(p), "tok"));
        r = low::and_(r, vlibc ? low::not_(low::field(tok_slot(p), "tok")) : low::field(tok_slot(p), "tok"));
        r = low::and_(r, low::bin(low::BinOp::Eq, live_cell(p), tok_loc_slot(p)));
        r = low::and_(r, low::acc(live_cell(p), "lv"));
        r = low::and_(r, block_lens(p, *pt.pointee));
        const std::vector<Cell> cells {block_cells(p, pt, text)};
        for (const Cell& c : cells)
        {
            const std::string sfx {use_cell_type(c.type)};
            r = low::and_(r, low::acc(c.recv, "v_" + sfx));
            r = low::and_(r, low::acc(c.recv, "d_" + sfx));
            if (mode == BlockMode::Kept)
            {
                E kept {low::old_(cell_flag(c.recv, c.type))};
                if (kept_guard)
                {
                    kept = low::and_(old_entry(kept_guard), kept);
                }
                r = low::and_(r, low::implies(kept, cell_flag(c.recv, c.type)));
            }
            if (const E range = leaf_range(c))
            {
                r = low::and_(r, low::implies(cell_flag(c.recv, c.type), range));
            }
        }
        if (mode == BlockMode::Plain)
        {
            for (const Cell& c : cells)
            {
                r = low::and_(r, cell_flag(c.recv, c.type));
            }
        }
        return r;
    }
    // The mode of a block site: a `requires` site on a marked pointer, and the result's when `result_init` is set, have no flags; an `ensures` site on a `ref_to_uninit`
    // formal keeps a set flag; every other site (including `must_init`'s `ensures`) states the flags.
    BlockMode block_mode(const Function& f, const AtomSite& s)
    {
        const InitMarker mk {block_marker(f, s)};
        if (mk == InitMarker::None)
        {
            return BlockMode::Plain;
        }
        if (s.in_ensures)
        {
            if (block_key(s) < 0)
            {
                return BlockMode::Uninit;
            }
            return mk == InitMarker::MustInit ? BlockMode::Plain : BlockMode::Kept;
        }
        return BlockMode::Uninit;
    }

    // An `owns_block` atom in the context being lowered (item 4).  Method: the whole conjunction; Wf: its pure facts (no heap); Call and Ret: `true`, the rows were asserted
    // one by one at the call or the return.
    LV block_atom(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full* full)
    {
        (void)guard;
        (void)gdep;
        (void)full;
        const FnBlocks& fb {blocks_of(*cf_)};
        const auto it {fb.index.find(&e)};
        if (it == fb.index.end())
        {
            internal_error(e.loc, "an owns_block atom in a position the contract rules forbid reached lowering");
            return {low::bool_lit(true), {}};
        }
        cbseen_ = std::max(cbseen_, it->second + 1U);
        if (cmode_ == CMode::Call || cmode_ == CMode::Ret)
        {
            return {low::bool_lit(true), {}};
        }
        const AtomSite& site {fb.sites[it->second]};
        const E p {block_ptr(site, env)};
        if (cmode_ == CMode::Wf)
        {
            return {block_pure(p, block_ptr_type(site)), {}};
        }
        E kept;
        if (const AtomSite* partner = block_partner(*cf_, site))
        {
            kept = site_guard(*partner, env);
        }
        return {block_conj(p, block_ptr_type(site), e.a->c_text, block_mode(*cf_, site), kept,
                           cf_->allocator == Allocator::Vlibc),
                {}};
    }

    // `perm(loc(b, -1).tok) == write && loc(b, -1).tok` as ONE atomic assertion (Silicon reports each conjunct of an asserted `&&` as its own error).
    E tok_value(const E& p)
    {
        return low::field(tok_slot(p), "tok");
    } // the token's value: true for a block of the platform's allocator, false for vlibc's
    E token_held(const E& p)
    {
        return low::not_(low::bin(low::BinOp::Or,
                                  low::bin(low::BinOp::Ne, low::perm_of(tok_slot(p), "tok"), low::write_perm()),
                                  low::not_(low::field(tok_slot(p), "tok"))));
    }

    // The rows of one block site at a call (`ret` false: item 6's free rows when the callee `deallocates`, else `permission` rows) or at a return (`ret` true, the `ensures`
    // site of this function): one explicit assertion per part of `owns_block(p)`, atomic, in the order of S36: the null test, WF, the offset, the token, the liveness
    // equation, the whole liveness cell, the lengths, then per leaf the write permission, the flag and the range.  A part whose failure is a bug of the encoder is
    // `internal`.  `add(category, cond, message)` makes the row (under the site's guard, which the caller applies).
    // Decision 0050 item 5 (a vlibc heap program only): the token row is two rows, the permission and then the token's VALUE, which names the allocator: `want` is the
    // allocator the site requires (the callee's for a deallocation view, the platform's for every `owns_block` of a contract) and `routine` the freeing routine's name.
    void block_rows(const E& p, const Type& pt, const std::string& text, BlockMode mode, bool dealloc,
                    const std::function<void(const char*, E, const std::string&)>& add, const std::string& who_needs,
                    bool at_return, Allocator want = Allocator::Platform, const std::string& routine = std::string())
    {
        const Type& t {*pt.pointee};
        const std::string pl {text.empty() ? std::string("the pointer") : "`" + text + "`"};
        const char* cat {dealloc ? "invalid-free" : "permission"};
        const std::string whole {
            at_return ? "returns without owning the whole allocation " + pl + " points to, which its contract promises"
                      : who_needs + " to own the whole allocation " + pl + " points to, without owning it"};
        if (!dealloc)
        {
            add("permission", low::bin(low::BinOp::Ne, p, low::null_()), whole);
        }
        // WF of the pointer: `internal` (record 0047 item 10, re-audit S3): the contract's own pointers at a return, and at a call a pointer variable a loop reassigned,
        // which the loop's automatic invariant `flag ==> WF(v)` keeps well-formed
        add("internal", wf_expr(p), dealloc ? "frees " + pl + ", which may not point to an allocation" : whole);
        // The token row first (it ends its path when the token is not held: a double free, a local, a global, a member, a borrowed pointer), then the offset: a pointer
        // that is not known to point to the start of its block (`drop(struct S *s) requires(owns(*s))`) is one error, the token's, not two.
        // In a vlibc heap program the token row is two rows that each READ the token under the permission (record 0050 item 5, audit rounds 2 and 3): `!(perm != write || tok != tok)`
        // keeps today's message and category, and `!(perm != write || <the other allocator's value>)` is the heap mismatch.  What the read buys is shape-specific: for a pointer
        // from an allocation (a local, a call's result tied to its arguments once split against them) and for a pointer loaded from a cell, a row that reads `tok` makes Silicon
        // resolve which held token chunk the pointer designates, where an assertion on `perm(tok)` only, or `perm == write ==> value`, leaves it unresolved and the next read of
        // `tok` is Silicon's own permission error (an exit 3 on a correct program).  In general it does not: a pointer that may be any of several held receivers (a member's
        // address, a one-line function's parameters) needs a case split on the candidate receivers, as the loop bodies and the call results have.
        add(cat,
            vlibc_heap_
                ? low::not_(low::bin(low::BinOp::Or,
                                     low::bin(low::BinOp::Ne, low::perm_of(tok_slot(p), "tok"), low::write_perm()),
                                     low::bin(low::BinOp::Ne, tok_value(p), tok_value(p))))
                : token_held(p),
            dealloc ? "frees " + pl +
                          ", which may already be freed or may not be an allocation (a local, a global or a member)"
                    : whole);
        if (vlibc_heap_)
        {
            const E value {tok_value(p)};
            std::string why;
            if (dealloc)
            {
                why = "frees " + pl + " with `" + routine + "`, but it may be a block of " +
                      (want == Allocator::Vlibc ? "the platform's" : "vlibc's") + " allocator";
            }
            else if (at_return)
            {
                why =
                    "returns " + pl +
                    " as an `owns_block` allocation of the platform's allocator, but it may be a block of vlibc's, which a contract cannot name yet";
            }
            else
            {
                why =
                    who_needs + " " + pl +
                    " to be a block of the platform's allocator (`owns_block`); it may be a block of vlibc's, which a contract cannot name yet";
            }
            // `perm != write` first: a pointer that holds no token fails the row above and passes this one (the read of `tok` is under the permission)
            add(cat,
                low::not_(low::bin(low::BinOp::Or,
                                   low::bin(low::BinOp::Ne, low::perm_of(tok_slot(p), "tok"), low::write_perm()),
                                   want == Allocator::Vlibc ? value : low::not_(value))),
                why);
        }
        add(cat, low::bin(low::BinOp::Eq, low::dom("offset_of", {p}), low::int_lit("0")),
            dealloc ? "frees " + pl + ", which may not be the start of an allocation" : whole);
        add("internal", low::bin(low::BinOp::Eq, live_cell(p), tok_loc_slot(p)),
            "the liveness cell of a block is at slot -2");
        add(cat, low::bin(low::BinOp::Eq, live_perm(p), low::write_perm()),
            dealloc ? "frees " + pl + " while a pointer to it may still be in use elsewhere" : whole);
        add("internal", block_lens(p, t), "the lengths of a block");
        for (const Cell& c : block_cells(p, pt, text))
        {
            const std::string part {c.text.empty() ? pl : "`" + c.text + "`"};
            add(cat, may_write(c.recv, c.type, false),
                dealloc ? "frees " + pl + " without owning all of it: " + part + " is not owned here" : whole);
            if (mode == BlockMode::Plain)
            {
                add("uninitialized", cell_flag(c.recv, c.type),
                    at_return ? "returns " + part +
                                    " without a value, but its contract promises the whole allocation with its contents"
                              : who_needs + " " + part + " to have a value; it may not have one here");
            }
            if (const E r = leaf_range(c))
            {
                add("internal", low::implies(cell_flag(c.recv, c.type), r), "the range of the value under its flag");
            }
        }
    }

    // The rows of a call's `requires` block sites (item 6), before the call: first the separation of every block site from every other site of the callee's `requires`
    // (`lvloc` rows: two sites never name one block, S36 finding 6; a failed row ends its path, so Silicon's own check of the call never fails first), then each block site's
    // rows.  Obligations of the call with the exact set PRE(c), like call_atoms'.
    void block_call_rows(const Expr& e, const Function& callee, const Env& cenv, const std::vector<LV>& args,
                         const Deps& pre, Full& full)
    {
        const FnBlocks& fb {blocks_of(callee)};
        if (fb.nreq == 0)
        {
            return;
        }
        Deps adep;
        for (const LV& a : args)
        {
            adep = join(adep, a.dep);
        }
        CtxSave call_ctx(*this);
        cmode_ = CMode::Call;
        cf_ = &callee;
        cens_ = false;
        cold_ = false;
        struct B
        {
            const AtomSite* site;
            E ptr, g;
            std::string text;
        };
        std::vector<B> bs;
        for (int i {0}; i < fb.nreq; ++i)
        {
            const AtomSite& s {fb.sites[static_cast<size_t>(i)]};
            B b;
            b.site = &s;
            b.ptr = block_ptr(s, cenv);
            b.g = site_guard(s, cenv);
            const Expr& arg {*e.args[static_cast<size_t>(block_key(s))]};
            b.text = arg.c_text;
            bs.push_back(std::move(b));
        }
        auto row = [&](const char* category, const E& guard, E cond, const std::string& message)
        {
            obl(full, guard, adep, std::move(cond), {}, meta(category, e.loc, message), &pre.ex);
        };
        // separation: block against block, and block against every other `requires` atom's root
        const FnAtoms& fa {atoms_of(callee)};
        for (size_t i {0U}; i < bs.size(); ++i)
        {
            for (size_t j {i + 1U}; j < bs.size(); ++j)
            {
                row("permission", low::and_(bs[i].g, bs[j].g),
                    low::bin(low::BinOp::Ne, live_cell(bs[i].ptr), live_cell(bs[j].ptr)),
                    "passes " + (bs[i].text.empty() ? std::string("a pointer") : "`" + bs[i].text + "`") + " and " +
                        (bs[j].text.empty() ? std::string("a pointer") : "`" + bs[j].text + "`") + " to `" +
                        callee.c_name +
                        "`, which needs to own the whole allocation of both, but they may be the same object");
            }
            for (int j {0}; j < fa.nreq; ++j)
            {
                const AtomSite& o {fa.sites[static_cast<size_t>(j)]};
                const E og {site_guard(o, cenv)};
                const E oroot {site_root(o, cenv)};
                row("permission", low::and_(bs[i].g, og),
                    low::bin(low::BinOp::Ne, live_cell(bs[i].ptr), live_cell(oroot)),
                    "passes " + (bs[i].text.empty() ? std::string("a pointer") : "`" + bs[i].text + "`") + " and " +
                        quoted(site_call_text(o, e), "another object") + " to `" + callee.c_name +
                        "`, which needs to own the whole allocation of the first, but they may be the same object");
            }
        }
        for (const B& b : bs)
        {
            const std::string who {"calls `" + callee.c_name + "`, which needs"};
            block_rows(
                b.ptr, block_ptr_type(*b.site), b.text, block_mode(callee, *b.site), callee.deallocates,
                [&](const char* category, E cond, const std::string& message)
                {
                    row(category, b.g, std::move(cond), message);
                },
                who, false, callee.deallocates ? callee.allocator : Allocator::Platform, callee.c_name);
        }
    }

    // Restores the contract context at the end of a scope.
    struct CtxSave
    {
        Lowerer& l;
        CMode mode;
        const Function* f;
        bool ens, old;
        size_t seen, bseen;
        explicit CtxSave(Lowerer& lw)
            : l(lw), mode(lw.cmode_), f(lw.cf_), ens(lw.cens_), old(lw.cold_), seen(lw.cseen_), bseen(lw.cbseen_)
        {}
        ~CtxSave()
        {
            l.cmode_ = mode;
            l.cf_ = f;
            l.cens_ = ens;
            l.cold_ = old;
            l.cseen_ = seen;
            l.cbseen_ = bseen;
        }
    };

    // The position in `sites` of the first atom at or after clause `clause` of the requires (`ens` false) or ensures list.
    static size_t first_site(const FnAtoms& fa, bool ens, int clause)
    {
        for (size_t i {0U}; i < fa.sites.size(); ++i)
        {
            const AtomSite& s {fa.sites[i]};
            if (s.in_ensures == ens && s.clause >= clause)
            {
                return i;
            }
            if (s.in_ensures && !ens)
            {
                return i;
            }
        }
        return fa.sites.size();
    }

    void set_ctx(const Function& f, CMode mode, bool ens, int clause)
    {
        cmode_ = mode;
        cf_ = &f;
        cens_ = ens;
        cold_ = false;
        cseen_ = first_site(atoms_of(f), ens, clause);
        cbseen_ = first_block_site(blocks_of(f), ens, clause);
    }

    static std::string amount_name(int k)
    {
        return "amt_" + std::to_string(k);
    }

    // `p == null || (p == loc(block_of(p), offset_of(p)) && 0 <= offset_of(p) && offset_of(p) <= len(block_of(p)))`: decision 0028 item 2, inline.
    // `ptr_type`, when given and a pointer to a struct, makes it WF_S (decision 0043 item 8): the member blocks of the object `p` points to have length 1, for a pointer
    // that points to an object (a one-past-the-end pointer designates none): `WF(p) && (p != null && offset_of(p) < len(block_of(p)) ==> len(member(p, k)) == 1 ...)`.
    E wf_expr(const E& p, const Type* ptr_type = nullptr)
    {
        using low::BinOp;
        out_.memory = true;
        const E off {low::dom("offset_of", {p})};
        E inner {low::bin(BinOp::Eq, p, low::dom("loc", {block_of(p), off}))};
        inner = low::bin(BinOp::And, inner, low::bin(BinOp::Le, low::int_lit("0"), off));
        inner = low::bin(BinOp::And, inner, low::bin(BinOp::Le, off, low::dom("len", {block_of(p)})));
        E r {low::bin(BinOp::Or, low::bin(BinOp::Eq, p, low::null_()), inner)};
        if (ptr_type != nullptr && ptr_type->is_ptr() && ptr_type->pointee != nullptr && ptr_type->pointee->is_struct())
        {
            const E at_object {low::bin(BinOp::And, low::bin(BinOp::Ne, p, low::null_()), in_block(p))};
            r = low::bin(BinOp::And, r, low::implies(at_object, struct_lens(p, *ptr_type->pointee->struct_type)));
        }
        return r;
    }
    E in_block(const E& p)
    { // offset_of(p) < len(block_of(p))
        return low::bin(low::BinOp::Lt, low::dom("offset_of", {p}), low::dom("len", {block_of(p)}));
    }
    // The range of a cell's value as one atomic condition (none for bool).
    E cell_range(const E& p, const Type& t)
    {
        if (t.is_ptr())
        {
            return wf_expr(cell_value(p, t), &t); // WF takes the place of the range (decision 0040 item 2)
        }
        if (!t.is_bool())
        {
            return range_obligation(cell_value(p, t), *t.int_type);
        }
        return nullptr;
    }

    static const Type& site_pointee(const AtomSite& s)
    {
        return *s.atom->a->type.pointee;
    }
    // A site is identified by a key and a receiver, never by a formal index directly (decision 0041 item 8).  The key (atom_place_key): the formal index
    // k >= 0 of a parameter place; a negative key is the place `result` (see `place_of`); keys >= kGlobalKeyBase are global places (`g`: its id is
    // key - kGlobalKeyBase), whose receiver is `loc(GB_g(), 0)`.
    static constexpr int kGlobalKeyBase {kGlobalSiteBase}; // vir.h: the keys of global places start here
    // Decision 0043 item 9: a place is a root (the key) and a member path (`AtomSite::path`); `site_key` is the root whatever the path is.
    int site_key(const AtomSite& s) const
    {
        if (!s.via.empty())
        { // decision 0045: a place with steps has its function's derived key
            const auto it {derived_of_atom_.find(s.atom)};
            if (it != derived_of_atom_.end())
            {
                return it->second;
            }
        }
        return atom_place(*s.atom).root;
    }
    bool site_is_global(const AtomSite& s) const
    {
        return key_is_global(site_key(s));
    }
    // One member path is a prefix of the other (an atom on `p->t` holds the cells of `p->t.x`).
    static bool paths_nested(const std::vector<int>& a, const std::vector<int>& b)
    {
        const size_t n {std::min(a.size(), b.size())};
        return std::equal(a.begin(), a.begin() + static_cast<std::ptrdiff_t>(n), b.begin());
    }
    // Two atoms name one place: one root, one path.
    bool same_place(const AtomSite& a, const AtomSite& b) const
    {
        return site_key(a) == site_key(b) && a.path == b.path;
    }
    // The root pointer in `env` (a global's is its address, not a formal), and the place's own address (the root's member chain).
    // Decision 0045: a site with steps has a BASE pointer (`s.base`) that is lowered in the current contract mode, framed by the atoms before the site: a
    // chain-guarded read (Method, Call, Ret), a ghost value with its framing obligation in `full` (Wf), the argument's read at a call.  Outside a contract (the
    // body) it is the entry snapshot `eb_N` of a loading `requires` place, or the leaf formal itself (a pointer leaf of a by-value parameter reads no heap).
    E site_root(const AtomSite& s, const Env& env, const E& guard = nullptr, const Deps& gdep = Deps {},
                Full* full = nullptr)
    {
        if (s.via.empty())
        {
            return place_of(env, site_key(s));
        }
        Full scratch;
        Full& fl {full != nullptr ? *full : scratch};
        if (cmode_ == CMode::None || cmode_ == CMode::Inv)
        {
            return derived_body_root(s, env);
        }
        const Function* sf {site_fn_.at(s.atom)};
        const FnAtoms& fa {atoms_of(*sf)};
        CtxSave save(*this);
        cf_ = sf;
        cens_ = s.in_ensures;
        cold_ = false;
        cseen_ = fa.index.at(s.atom);
        cbseen_ = block_sites_before(s.atom); // audit round 2, S3: the block sites before the atom frame its loads too
        return lower(*s.base, env, guard, gdep, fl).e;
    }
    E site_ref(const AtomSite& s, const Env& env, const E& guard = nullptr, const Deps& gdep = Deps {},
               Full* full = nullptr)
    {
        return chain_ref(site_root(s, env, guard, gdep, full), s.path);
    }
    std::string lb_name(const Derived& d) const
    {
        return "lb_" + std::to_string(d.f->params.size() + static_cast<size_t>(d.local)) + "__" +
               std::to_string(cur_loop_);
    }
    static std::string eb_name(const Derived& d)
    {
        return "eb_" + std::to_string(d.f->params.size() + static_cast<size_t>(d.local));
    }
    // The base of a `requires` site in the body: the entry snapshot of a loading place, the leaf formal otherwise.
    E derived_body_root(const AtomSite& s, const Env& env)
    {
        const int k {site_key(s)};
        if (!is_derived_key(k))
        {
            return place_of(env, k);
        }
        const Derived& d {derived_of(k)};
        if (d.has_load)
        {
            return low::var(eb_name(d));
        }
        CtxSave save(*this);
        cmode_ = CMode::Wf; // no heap is read: the leaf formal or the result's leaf
        Full scratch;
        return lower(*s.base, env, nullptr, Deps {}, scratch).e;
    }
    // The same as a variable of the function being lowered (its own formals; a global's address; a derived place's entry snapshot).
    E site_var(const AtomSite& s)
    {
        if (!s.via.empty())
        {
            return chain_ref(derived_body_root(s, contract_env()), s.path);
        }
        return chain_ref(site_is_global(s) ? place_of(Env {}, site_key(s)) : low::var(formal_name(*fn_.f, site_key(s))),
                         s.path);
    }
    // The cells of a site, over `place` (its own address): the leaves of a struct place, or the one cell of a scalar place.  `text` of each is its spelling for messages.
    std::vector<Cell> site_cells(const AtomSite& s, const E& place)
    {
        const Type& t {site_pointee(s)};
        if (!site_takes_cell(s))
        {
            return {};
        }
        if (!t.is_struct())
        {
            return {Cell {place, t, site_text(s), {}, ""}};
        }
        std::string root;
        bool arrow {false};
        if (s.path.empty())
        {
            root = s.atom->a->c_text;
            arrow = !site_is_global(s);
            if (site_is_global(s))
            {
                root = s.atom->a->global != nullptr ? s.atom->a->global->c_name : std::string();
            }
        }
        else
        {
            root = place_text(*s.atom->a);
        }
        return leaf_cells(place, *t.struct_type, root, arrow);
    }
    std::vector<Cell> site_cells(const AtomSite& s, const Env& env)
    {
        return site_cells(s, site_ref(s, env));
    }
    // The VIR field (its Viper suffix) of every cell of a site, without an environment: what `owns` and `reads` atoms are matched on (decision 0043 item 9, N2).
    std::set<std::string> site_fields(const AtomSite& s)
    {
        std::set<std::string> out;
        if (!site_takes_cell(s))
        {
            return out;
        }
        const Type& t {site_pointee(s)};
        if (!t.is_struct())
        {
            out.insert(cell_suffix(t));
        }
        else
        {
            for (const StructLeaf& l : struct_leaves(*t.struct_type))
            {
                out.insert(cell_suffix(l.type));
            }
        }
        return out;
    }
    bool shares_field(const AtomSite& a, const AtomSite& b)
    {
        const std::set<std::string> fa {site_fields(a)};
        const std::set<std::string> fb {site_fields(b)};
        for (const std::string& f : fa)
        {
            if (fb.count(f) != 0U)
            {
                return true;
            }
        }
        return false;
    }
    // The pairs of places (one cell of `a`, one of `b`) that must differ for two atoms not to share a cell, over their own addresses `pa` and `pb` (decision 0043 item 9:
    // overlap by leaf cell and field, never by the atoms' pointee types): the two addresses when both atoms name objects of one type (equal slots are the only way two such
    // objects share a leaf; the member axioms make a leaf's chain injective), else every pair of cells of one field.  Never fewer pairs than needed: a missed pair costs
    // Silicon's own error, but an assumed distinctness fact would be unsound.
    std::vector<std::pair<E, E>> overlap_pairs(const AtomSite& a, const E& pa, const AtomSite& b, const E& pb)
    {
        const Type& ta {site_pointee(a)};
        const Type& tb {site_pointee(b)};
        if (ta == tb)
        {
            return {{pa, pb}};
        }
        std::vector<std::pair<E, E>> out;
        for (const Cell& x : site_cells(a, pa))
        {
            for (const Cell& y : site_cells(b, pb))
            {
                if (cell_suffix(x.type) == cell_suffix(y.type))
                {
                    out.emplace_back(x.recv, y.recv);
                }
            }
        }
        return out;
    }
    // The argument of a call that a site's pointer parameter receives (a global site has none).
    const Expr& site_arg(const AtomSite& s, const Expr& call) const
    {
        return *call.args[static_cast<size_t>(site_key(s))];
    }
    // The user's spelling of the place a site names in the contract: `*p` for a parameter's pointee (empty if unknown), `g` for a global.
    std::string site_text(const AtomSite& s) const
    {
        if (site_is_global(s))
        {
            return s.atom->a->global != nullptr ? s.atom->a->global->c_name : std::string();
        }
        if (!s.path.empty())
        {
            return place_text(*s.atom->a); // `p->x`, `p->t.x`
        }
        const std::string& c {s.atom->a->c_text};
        return c.empty() ? std::string() : "*" + c;
    }
    // What a message calls a site's place: the quoted spelling, or for the implicit atom of a `static` local "the `static` local `s` of `f`" (decision 0041
    // item 12; `quoted_text` is what the caller would have quoted).
    static std::string site_phrase(const AtomSite& s, const std::string& quoted_text)
    {
        if (s.atom->implicit_atom && s.atom->a->global != nullptr && s.atom->a->global->owner != nullptr)
        {
            return "the `static` local `" + s.atom->a->global->c_name + "` of `" + s.atom->a->global->owner->c_name +
                   "`";
        }
        return quoted_text;
    }
    // The same, from the argument a call passes: `x` for `&x`, `*q` for `q`.
    std::string site_call_text(const AtomSite& s, const Expr& call) const
    {
        if (site_is_global(s) || !s.via.empty())
        {
            // a global's call text is its own name, a place reached through steps is the callee's spelling (decision
            // 0045)
            return site_text(s);
        }
        const std::string& txt {site_arg(s, call).c_text};
        const std::string base {txt.empty() ? std::string() : (txt[0U] == '&' ? txt.substr(1U) : "*" + txt)};
        if (s.path.empty() || base.empty())
        {
            return base;
        }
        // The member path of a struct place, from the root struct's member names: `s.t.x` for `&s`, `p->t.x` for `p`.
        const Expr* root {member_root(s.atom->a)};
        if (root == nullptr || !root->type.is_ptr() || root->type.pointee == nullptr ||
            !root->type.pointee->is_struct())
        {
            return base;
        }
        const StructType* st {root->type.pointee->struct_type};
        std::string out {base[0U] == '*' ? base.substr(1U) : base};
        bool first {true};
        for (int k : s.path)
        {
            if (st == nullptr || k < 0 || static_cast<size_t>(k) >= st->members.size())
            {
                return base;
            }
            out += (first && base[0U] == '*' ? "->" : ".") + st->members[static_cast<size_t>(k)].name;
            first = false;
            st = st->members[static_cast<size_t>(k)].type.is_struct()
                     ? st->members[static_cast<size_t>(k)].type.struct_type
                     : nullptr;
        }
        return out;
    }
    static bool site_takes_cell(const AtomSite& s)
    {
        return s.atom->atom != AtomKind::Live;
    }
    // Every atom on a struct root, `live(p)` included, states `offset_of(root) < len(block_of(root))` (decision 0043 item 9): a struct pointer a function holds an atom on points to an
    // object, never one past the end, so `&p->x` is in bounds; the scalar `live(p)` stays as it was.
    static bool needs_root_block(const AtomSite& s)
    {
        const Type& rt {member_root(s.atom->a)->type};
        return site_takes_cell(s) || (rt.is_ptr() && rt.pointee != nullptr && rt.pointee->is_struct());
    }
    // The initialization marker (decision 0042 item 4) a site carries: the `owns(*p)` atom of a parameter `p` of `f` that is marked `ref_to_uninit` or
    // `must_init`.  Every other site (a `reads` or `live` atom, a global's, `*result`'s) is unmarked.
    InitMarker site_marker(const Function& f, const AtomSite& s) const
    {
        const int k {site_key(s)};
        if (k < 0 || k >= kGlobalSiteBase || !s.path.empty() || s.atom->atom != AtomKind::Owns ||
            static_cast<size_t>(k) >= f.params.size())
        {
            return InitMarker::None;
        }
        return f.params[static_cast<size_t>(k)].init;
    }

    // The guard of a site over `env`'s formals: the conjunction of its `||`/`?:` conditions (null: unconditional).
    E site_guard(const AtomSite& s, const Env& env)
    {
        E g;
        CtxSave save(*this);
        // A guard that reads the heap (decision 0045 item 9, question 109) is lowered in the contract mode of the caller with its loads framed like any contract
        // read, by the atoms before the site; one that does not is a pure contract expression whatever the context (a pointer `?:` is an ite, never an effect).
        bool reads {false};
        for (const AtomGuard& x : s.guards)
        {
            reads = reads || reads_heap(*x.cond);
        }
        if (reads && cmode_ == CMode::Inv)
        {
            internal_error(
                s.atom->loc,
                "a guard that reads the heap reached site_guard in a loop annotation (decision 0045 item 11: written atoms lower their own)");
            return nullptr;
        }
        if (reads && (cmode_ == CMode::Method || cmode_ == CMode::Call || cmode_ == CMode::Ret || cmode_ == CMode::Wf ||
                      cmode_ == CMode::None))
        {
            if (cmode_ == CMode::None)
            {
                cmode_ = CMode::Method;
            }
            const Function* sf {site_fn_.at(s.atom)};
            cf_ = sf;
            cens_ = s.in_ensures;
            cold_ = false;
            const FnAtoms& sfa {atoms_of(*sf)};
            if (const auto si {sfa.index.find(s.atom)}; si != sfa.index.end())
            {
                cseen_ = si->second;
                cbseen_ = block_sites_before(s.atom);
            }
            else
            {
                internal_error(s.atom->loc, "an owns_block site whose guard reads the heap reached lowering");
            }
        }
        else
        {
            cmode_ = CMode::Wf;
        }
        for (const AtomGuard& x : s.guards)
        {
            Full scratch;
            LV v {lower(*x.cond, env, nullptr, {}, scratch)};
            g = low::and_(g, x.holds ? v.e : low::not_(v.e));
        }
        return g;
    }

    // Two atoms that must not name one cell: two `owns`, or an `owns` and a `reads`, of one field (decision 0037 items 8 and 10).
    bool exclusive_pair(const AtomSite& a, const AtomSite& b)
    {
        if (!site_takes_cell(a) || !site_takes_cell(b))
        {
            return false;
        }
        if (a.atom->atom == AtomKind::Reads && b.atom->atom == AtomKind::Reads)
        {
            return false;
        }
        if (site_is_global(a) && site_is_global(b) && site_key(a) != site_key(b))
        {
            return false; // two globals are two `unique` blocks (decision 0041 item 8)
        }
        // a cell of one field in both (decision 0043 item 9: the leaves of a struct atom count, not its pointee type)
        return shares_field(a, b);
    }

    // Decision 0045 item 7 (owner question 108): the amount an `ensures` atom returns.  Places without a load return the amount of their `requires` partner (decision
    // 0037 item 6); a place reached through a load follows the OBJECT: `ite(q_post == q_1pre, a_1, ite(q_post == q_2pre, a_2, ... a_partner))` over the `requires`
    // atoms of its kind and a field it shares, `q_ipre` each one's entry place (`entry_place`), the partner last.
    E ens_amount(const FnAtoms& fa, const AtomSite& site, const E& q_post,
                 const std::function<E(const AtomSite&)>& entry_place, const Env& env)
    {
        E r {low::var(amount_name(site.atom->amount))};
        if (!site.in_ensures || site.via.empty() || !is_derived_key(site_key(site)) ||
            !derived_of(site_key(site)).has_load)
        {
            return r;
        }
        if (site.atom->amount < 0 || site.atom->amount >= fa.nreq)
        {
            return r;
        }
        // The object a `requires` atom took, in the return heap's terms: its guard in the entry state, and the same object as `q_post`: the leaf cells of one field
        // coincide (the places when both name objects of one type), a struct atom matches only an atom of its own type, a `live` atom by liveness cell (audit S4).
        auto cond_of = [&](const AtomSite& rs) -> E
        {
            E same;
            if (!site_takes_cell(site))
            {
                same = low::bin(low::BinOp::Eq, live_cell(q_post), live_cell(entry_place(rs)));
            }
            else
            {
                if ((site_pointee(rs).is_struct() || site_pointee(site).is_struct()) &&
                    !(site_pointee(rs) == site_pointee(site)))
                {
                    return nullptr;
                }
                for (const auto& [x, y] : overlap_pairs(site, q_post, rs, entry_place(rs)))
                {
                    const E eq {low::bin(low::BinOp::Eq, x, y)};
                    same = same ? low::bin(low::BinOp::Or, same, eq) : eq;
                }
            }
            if (!same)
            {
                return nullptr;
            }
            const E g {site_guard(rs, env)};
            return g ? low::and_(old_entry(g), same) : same;
        };
        const int partner {site.atom->amount};
        for (int i {fa.nreq - 1}; i >= 0; --i)
        {
            if (i == partner)
            {
                continue;
            }
            const AtomSite& rs {fa.sites[static_cast<size_t>(i)]};
            if (rs.atom->atom != site.atom->atom || site_takes_cell(rs) != site_takes_cell(site) ||
                (site_takes_cell(rs) && !shares_field(rs, site)))
            {
                continue;
            }
            if (const E c = cond_of(rs))
            {
                r = low::ite(c, low::var(amount_name(i)), r);
            }
        }
        // The partner first (and, as the else of the others, last): a callee that kept the object keeps the amount it was given for it.
        if (!(r->kind == low::LExpr::Kind::Var))
        {
            if (const E c = cond_of(fa.sites[static_cast<size_t>(partner)]))
            {
                r = low::ite(c, low::var(amount_name(partner)), r);
            }
        }
        return r;
    }

    static bool guards_read_heap(const AtomSite& a, const AtomSite& b)
    {
        for (const AtomSite* x : {&a, &b})
        {
            for (const AtomGuard& g : x->guards)
            {
                if (reads_heap(*g.cond))
                {
                    return true;
                }
            }
        }
        return false;
    }
    // Whether a site's place is reached through a heap read.
    bool site_loads(const AtomSite& s) const
    {
        return !s.via.empty() && is_derived_key(site_key(s)) && derived_of(site_key(s)).has_load;
    }

    // An atom (decision 0037 items 5 and 6) in the context being lowered.
    LV atom(const Expr& e, const Env& env, const E& guard = nullptr, const Deps& gdep = Deps {}, Full* full = nullptr)
    {
        using low::BinOp;
        if (e.atom == AtomKind::OwnsBlock)
        {
            return block_atom(e, env, guard, gdep, full);
        }
        const FnAtoms& fa {atoms_of(*cf_)};
        const auto it {fa.index.find(&e)};
        if (it == fa.index.end())
        {
            internal_error(e.loc, "a permission atom in a position the contract rules forbid reached lowering");
            return {low::bool_lit(true), {}};
        }
        const size_t idx {it->second};
        cseen_ = std::max(cseen_, idx + 1U);
        if (cmode_ == CMode::Call || cmode_ == CMode::Ret)
        {
            return {low::bool_lit(true), {}}; // asserted one by one at the call or the return
        }
        const AtomSite& site {fa.sites[idx]};
        // The facts about the ROOT pointer (decision 0043 item 9: each atom, a member atom too, gives `root != null`, `WF_S(root)` and `offset_of(root) < len(block_of(root))`);
        // for a place without a path the root is the place.  A derived place's root is its lowered base (decision 0045): the loads of the atom's own place are read
        // once here, framed by the atoms before it (in the well-formedness method with their framing obligations).
        const E root {site_root(site, env, guard, gdep, full)};
        const E p {chain_ref(root, site.path)};
        const Type& t {site_pointee(site)};
        E pure {low::and_(low::bin(BinOp::Ne, root, low::null_()), wf_expr(root, &member_root(site.atom->a)->type))};
        if (needs_root_block(site))
        {
            pure = low::and_(pure, in_block(root));
        }
        if (cmode_ == CMode::Wf)
        {
            // Two full atoms of one list name two cells (the fact Silicon derives from the two `acc`s), added after the second one (decision 0037 item 7); per pair of
            // cells of one field (decision 0043 item 9).  Atoms on one root are distinct places by the member axioms (overlapping ones are the checker's).
            if (e.atom == AtomKind::Owns)
            {
                for (size_t i {0U}; i < idx; ++i)
                {
                    const AtomSite& o {fa.sites[i]};
                    if (o.in_ensures != site.in_ensures || o.atom->atom != AtomKind::Owns ||
                        site_key(o) == site_key(site))
                    {
                        continue;
                    }
                    if (site_is_global(o) && site_is_global(site))
                    {
                        continue; // `unique` blocks
                    }
                    if (!shares_field(o, site))
                    {
                        continue;
                    }
                    // Decision 0045 item 5 (audit B1): the `ensures` atoms of a function WITHOUT a body are promises nothing checks, so their distinctness is not a fact but an
                    // assertion (`f__wf`) when one of the two places is reached through a load.
                    if (site.in_ensures && cf_->body == nullptr && (site_loads(o) || site_loads(site)))
                    {
                        continue;
                    }
                    for (const auto& [x, y] : overlap_pairs(o, site_ref(o, env), site, p))
                    {
                        pure = low::and_(pure, low::implies(site_guard(o, env), low::bin(BinOp::Ne, x, y)));
                    }
                }
            }
            // Decision 0045 item 5 (audit B1): a function without a body is trusted, nothing checks what its `ensures` returns.  An atom whose place is reached through a load
            // must say it is the object its `requires` partner took (`q_post == q_partner_pre`, the user's `*pp == old(*pp)`), and two `owns` atoms one of which loads must
            // name different objects.  Both are obligations at the atom's position, under the path condition (the earlier conjuncts of its clause count) and the clauses before.
            // Decision 0045 item 3 (audit B2, completeness): when either guard of a pair reads the heap, 0039 item 7's partner row is an obligation here too (the earlier conjuncts
            // of the clause, the user's ties `*pp == old(*pp)`, count); guards that read no heap keep the row of `f__wf` as it was.
            if (full != nullptr && site.in_ensures && e.amount >= 0 && e.amount < fa.nreq &&
                guards_read_heap(site, fa.sites[static_cast<size_t>(e.amount)]))
            {
                const AtomSite& partner {fa.sites[static_cast<size_t>(e.amount)]};
                if (const E gp = site_guard(partner, env))
                {
                    const std::string txt {e.a != nullptr ? site_text(site) : std::string()};
                    obl(*full, guard, gdep, gp, {},
                        meta("permission", e.loc,
                             "the contract of `" + cf_->c_name + "` returns " + quoted(txt, "an object") +
                                 " on a path where its `requires` did not take it"));
                }
            }
            if (full != nullptr && site.in_ensures && cf_->body == nullptr)
            {
                if (site_loads(site) && e.amount >= 0 && e.amount < fa.nreq)
                {
                    const AtomSite& partner {fa.sites[static_cast<size_t>(e.amount)]};
                    const std::string base {e.a != nullptr ? e.a->c_text : std::string()};
                    const E same {site_takes_cell(site)
                                      ? low::bin(BinOp::Eq, p, site_ref(partner, env))
                                      : low::bin(BinOp::Eq, live_cell(p), live_cell(site_ref(partner, env)))};
                    obl(*full, guard, gdep, same, {},
                        meta("permission", e.loc,
                             "the contract of `" + cf_->c_name + "` returns " +
                                 quoted(site_takes_cell(site) ? site_text(site) : base, "an object") +
                                 " without saying it is the object its `requires` took" +
                                 (base.empty() ? std::string() : " (write `" + base + " == old(" + base + ")`)")));
                }
                if (e.atom == AtomKind::Owns)
                {
                    for (size_t i {0U}; i < idx; ++i)
                    {
                        const AtomSite& o {fa.sites[i]};
                        if (!o.in_ensures || o.atom->atom != AtomKind::Owns || site_key(o) == site_key(site) ||
                            !(site_loads(o) || site_loads(site)) || !shares_field(o, site))
                        {
                            continue;
                        }
                        const E go_ {site_guard(o, env)};
                        for (const auto& [x, y] : overlap_pairs(o, site_ref(o, env), site, p))
                        {
                            obl(*full, guard, gdep,
                                go_ ? low::implies(go_, low::bin(BinOp::Ne, x, y)) : low::bin(BinOp::Ne, x, y), {},
                                meta("permission", e.loc,
                                     "the contract of `" + cf_->c_name + "` returns " +
                                         quoted(site_text(o), "an object") + " and " +
                                         quoted(site_text(site), "an object") +
                                         " as owned places, but they may be the same object"));
                        }
                    }
                }
            }
            return {pure, {}};
        }
        if (e.amount < 0)
        {
            internal_error(e.loc, "a permission atom without its ghost amount reached lowering");
            return {low::bool_lit(true), {}};
        }
        const E amount {cmode_ == CMode::Method && site.in_ensures
                            ? ens_amount(
                                  fa, site, p,
                                  [&](const AtomSite& rs)
                                  {
                                      return rs.via.empty() ? site_ref(rs, env) : low::old_(site_ref(rs, env));
                                  },
                                  env)
                            : low::var(amount_name(e.amount))};
        E r {pure};
        if (site_takes_cell(site))
        {
            const bool owns {e.atom == AtomKind::Owns};
            // Decision 0042 item 5: a marked parameter's pointee need not have a value on entry, so its `requires` atom has no flag row and states the
            // range (or WF) under the flag; `must_init`'s `ensures` atom is the plain one; `ref_to_uninit`'s `ensures` atom keeps a flag that was set
            // (`old(flag) ==> flag`, on the paths where the `requires` partner took the cell) and states the range under the flag.  Every leaf of a struct pointee likewise
            // (decision 0043 item 9).
            const InitMarker mk {site_marker(*cf_, site)};
            const bool plain {mk == InitMarker::None || (mk == InitMarker::MustInit && site.in_ensures)};
            if (t.is_struct())
            {
                // the member blocks have length 1 (a leaf's address is then in its block)
                r = low::and_(r, struct_lens(p, *t.struct_type));
            }
            for (const Cell& c : site_cells(site, p))
            {
                const std::string sfx {use_cell_type(c.type)};
                r = low::and_(r, owns ? low::acc(c.recv, "v_" + sfx) : low::acc(c.recv, "v_" + sfx, amount));
                r = low::and_(r, owns ? low::acc(c.recv, "d_" + sfx) : low::acc(c.recv, "d_" + sfx, amount));
                if (!plain && site.in_ensures && e.amount >= 0 && e.amount < atoms_of(*cf_).nreq)
                {
                    E kept {low::old_(cell_flag(c.recv, c.type))};
                    if (const E gp = site_guard(atoms_of(*cf_).sites[static_cast<size_t>(e.amount)], env))
                    {
                        kept = low::and_(old_entry(gp), kept); // the partner's guard in the entry state (audit B2)
                    }
                    r = low::and_(r, low::implies(kept, cell_flag(c.recv, c.type)));
                }
                if (plain)
                {
                    r = low::and_(r, cell_flag(c.recv, c.type));
                }
                const E range {
                    c.type.is_ptr()
                        ? wf_expr(cell_value(c.recv, c.type), &c.type)
                        : (!c.type.is_bool() ? range_check(cell_value(c.recv, c.type), *c.type.int_type) : E())};
                if (range)
                {
                    r = low::and_(r, plain ? range : low::implies(cell_flag(c.recv, c.type), range));
                }
            }
        }
        r = low::and_(r, low::acc(live_cell(p), "lv", amount));
        return {r, {}};
    }

    // Decision 0039 item 7: an `owns(*p)` and a `reads(*p)` of one parameter in one clause list are allowed where their guards exclude each other; the
    // `__wf` method asserts `prior ==> !(g_o && g_r)` at the later of the two.  Two atoms of one kind on one place are the checker's.
    void overlap_obligations(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        const FnAtoms& fa {atoms_of(*cf_)};
        const auto it {fa.index.find(&e)};
        if (it == fa.index.end())
        {
            return;
        }
        const AtomSite& site {fa.sites[it->second]};
        if (!site_takes_cell(site))
        {
            return;
        }
        for (size_t i {0U}; i < it->second; ++i)
        {
            const AtomSite& o {fa.sites[i]};
            if (o.in_ensures != site.in_ensures || !site_takes_cell(o) || !same_place(o, site) ||
                o.atom->atom == site.atom->atom)
            {
                continue;
            }
            const AtomSite& own {o.atom->atom == AtomKind::Owns ? o : site};
            const AtomSite& rd {o.atom->atom == AtomKind::Owns ? site : o};
            const E both {low::and_(site_guard(own, env), site_guard(rd, env))};
            const std::string txt {e.a != nullptr ? site_text(site) : std::string()};
            obl(full, guard, gdep, both ? low::not_(both) : low::bool_lit(false), {},
                meta("permission", e.loc,
                     "the contract of `" + cf_->c_name + "` both owns " + quoted(txt, "an object") +
                         " and reads it on one path"));
        }
    }

    // Whether a lowered expression reads the heap (a field read, a `perm`, an `acc`).
    static bool lreads_heap(const E& e)
    {
        if (e == nullptr)
        {
            return false;
        }
        if (e->kind == low::LExpr::Kind::Field || e->kind == low::LExpr::Kind::PermOf ||
            e->kind == low::LExpr::Kind::Acc)
        {
            return true;
        }
        if (lreads_heap(e->a) || lreads_heap(e->b) || lreads_heap(e->c))
        {
            return true;
        }
        for (const E& x : e->args)
        {
            if (lreads_heap(x))
            {
                return true;
            }
        }
        return false;
    }
    // Decision 0045 item 3 (audit B2): a requires atom's guard (or place) that the contract uses outside the entry state is read in the entry state, as ONE
    // `old(...)` over the whole expression; an expression that reads no heap stays as it is.
    static E old_entry(const E& e)
    {
        return lreads_heap(e) ? low::old_(e) : e;
    }

    // The ghost value of a read of `*p` for parameter `p`, or (param -1) of `*result` (decision 0040 item 4).
    // A ghost value of `f__wf` is keyed by its root, its member path and its state (decision 0043 item 9, audit S1): `p->x` and `p->y` are two.
    using GKey = std::tuple<int, std::vector<int>, bool>;
    std::string ghost_name(int param, const std::vector<int>& path, bool post) const
    {
        std::string mem;
        for (int k : path)
        {
            mem += "_m" + std::to_string(k);
        }
        if (is_derived_key(param))
        { // decision 0045: a read through a load or a pointer leaf, numbered after the function's parameters
            const Derived& d {derived_of(param)};
            return "hv_" + std::to_string(d.f->params.size() + static_cast<size_t>(d.local)) + mem +
                   (post ? "_post" : "_pre");
        }
        if (key_is_global(param))
        {
            // a global's read (decision 0041)
            return "hv_g" + std::to_string(param - kGlobalKeyBase) + mem + (post ? "_post" : "_pre");
        }
        return param < 0 ? "hv_r" + mem + "_post" : "hv_" + std::to_string(param) + mem + (post ? "_post" : "_pre");
    }
    std::string ghost_name(const GKey& k) const
    {
        return ghost_name(std::get<0>(k), std::get<1>(k), std::get<2>(k));
    }
    E place_of(const Env& env, int k) const
    {
        if (key_is_global(k))
        {
            return global_ref(k - kGlobalKeyBase);
        }
        return k < 0 ? env.result : env.formals[static_cast<size_t>(k)];
    }

    // ---- globals (decision 0041) ----

    // The block constant of global `id`: `GB_<name>__<id>()`; its address `loc(GB_g(), 0)`.
    E global_block(int id) const
    {
        return low::dom(prog_.globals()[static_cast<size_t>(id)].viper_name(), {});
    }
    E global_ref(int id) const
    {
        return low::dom("loc", {global_block(id), low::int_lit("0")});
    }
    static bool is_global_ref(const E& e)
    {
        return e != nullptr && e->kind == low::LExpr::Kind::Dom && e->text == "loc" && e->args.size() == 2U &&
               e->args[0U]->kind == low::LExpr::Kind::Dom && e->args[0U]->text.compare(0U, 3U, "GB_") == 0;
    }
    bool has_globals() const
    {
        return !prog_.globals().empty();
    }
    // The pure facts of every global (decision 0041 item 4): one block of one slot whose liveness cell is `loc(b, -2)`; null without globals.
    E global_facts()
    {
        E r;
        for (const Global& g : prog_.globals())
        {
            const E b {global_block(g.id)};
            out_.memory = true;
            r = low::and_(r, low::and_(low::bin(low::BinOp::Eq, low::dom("len", {b}), low::int_lit("1")),
                                       low::bin(low::BinOp::Eq, low::dom("lvloc", {b}),
                                                low::dom("loc", {b, low::int_lit("-2")}))));
            // A struct global (decision 0044 item 8): the member lengths of every slot of `loc(GB_g(), 0)` and the slot facts, right after its own facts (WF_S needs them)
            if (g.type.is_struct())
            {
                r = low::and_(r, struct_lens(global_ref(g.id), *g.type.struct_type));
            }
        }
        return r;
    }
    // The liveness disjunct of a unit with globals (decision 0041 item 6): `lvloc(block_of(p)) == lvloc(GB_g())` for some global; null without globals.
    E global_live_disjunct(const E& p)
    {
        E r;
        for (const Global& g : prog_.globals())
        {
            const E d {low::bin(low::BinOp::Eq, live_cell(p), low::dom("lvloc", {global_block(g.id)}))};
            r = r ? low::bin(low::BinOp::Or, r, d) : d;
        }
        return r;
    }
    // The type of the place a site key names: a parameter's pointee, or a global's own type.
    Type key_pointee(int k) const
    {
        if (is_derived_key(k))
        {
            return *derived_of(k).base_type.pointee;
        }
        if (key_is_global(k))
        {
            return prog_.globals()[static_cast<size_t>(k - kGlobalKeyBase)].type;
        }
        return *fn_.f->params[static_cast<size_t>(k)].type.pointee;
    }
    static bool key_is_global(int k)
    {
        return is_global_site_key(k);
    } // a derived key is never a global (decision 0045 item 2)

    // A read of `*p` in a contract (decision 0037 item 7).  Wf: a ghost value formal, and the framing obligation; otherwise the heap read, guarded
    // by itself: the chain `(D_1 ? read : (D_2 ? read : ... : dummy))` over the framing disjuncts of the earlier atoms of the same field, or the
    // plain read when the framing is trivial (an unconditional atom on the same place) or implied by the guard the read is evaluated under.
    LV contract_read(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        using low::BinOp;
        // Under `old` (Method, Ret) the OUTERMOST read decides how it is written: everything inside it (the base's own loads, the disjuncts' places and guards) is lowered
        // plain, and when any of it reads the heap the whole chain is ONE `old(...)` (audit B2: `old` around the last read only would use the return heap for the
        // framing); a read whose chain reads no heap keeps `old` around the read alone, as before decision 0045.
        const bool old_state {cold_ && (cmode_ == CMode::Method || cmode_ == CMode::Ret)};
        const bool outer_old {old_state && old_depth_ == 0};
        struct DepthGuard
        {
            int& d;
            bool on;
            DepthGuard(int& dd, bool o) : d(dd), on(o)
            {
                if (on)
                {
                    ++d;
                }
            }
            ~DepthGuard()
            {
                if (on)
                {
                    --d;
                }
            }
        } depth_guard(old_depth_, old_state);
        std::vector<int> path; // decision 0043 item 9: a place is a root and a member path
        const Expr* root {e.a == nullptr ? nullptr : member_root(e.a, &path)};
        int k {kNoPlaceKey};
        E p;
        if (root != nullptr && (root->kind == Expr::Kind::Formal || root->kind == Expr::Kind::Result ||
                                root->kind == Expr::Kind::GlobalAddr))
        {
            // -1: the place `result` (decision 0040 item 4); a global is a place of its own (decision 0041 item 7)
            k = root->kind == Expr::Kind::Result
                    ? -1
                    : (root->kind == Expr::Kind::GlobalAddr ? kGlobalKeyBase + root->global->id : root->index);
            p = chain_ref(place_of(env, k), path);
        }
        else
        {
            // Decision 0045 item 2: a place reached through loads or pointer leaves; its base pointer is lowered in this mode (a ghost value, a chain-guarded read)
            std::vector<int> via;
            const Expr* r {root == nullptr ? nullptr : place_root(root, &via)};
            if (root == nullptr || via.empty() || root_key_of(r) == kNoPlaceKey)
            {
                internal_error(
                    e.loc,
                    "a dereference in a contract of anything but a pointer parameter, a global or the result (or a member of one) reached lowering");
                return {low::int_lit("0"), {}};
            }
            k = derived_key(*cf_, root_key_of(r), via, root);
            const LV b {lower(*root, env, guard, gdep, full)};
            p = chain_ref(b.e, path);
        }
        const Type& t {e.type};
        const bool post {cens_ && !cold_};
        const FnAtoms& fa {atoms_of(*cf_)};

        const std::string sfx {use_cell_type(t)};
        std::vector<E> ds; // the framing disjuncts D_i; a null one is `true`
        for (size_t i {0U}; i < fa.sites.size(); ++i)
        {
            const AtomSite& s {fa.sites[i]};
            if (!site_takes_cell(s))
            {
                continue;
            }
            if (post)
            {
                if (!s.in_ensures || i >= cseen_)
                {
                    continue;
                }
            }
            else
            {
                if (s.in_ensures)
                {
                    continue;
                }
                if (!cens_ && i >= cseen_)
                {
                    continue;
                }
            }
            if (site_fields(s).count(sfx) == 0U)
            {
                continue;
            }
            // Decision 0042 item 5: a marked atom's site never covers a read of the pre-state, and covers one of the post-state only for `must_init`
            // (the cell has a value on return); a read through an alias of a marked parameter then fails the framing like any unowned read.
            if (const InitMarker mk {site_marker(*cf_, s)};
                mk != InitMarker::None && !(post && mk == InitMarker::MustInit))
            {
                continue;
            }
            const E g {site_guard(s, env)};
            // One disjunct per cell of the atom with this read's field (decision 0043 item 9): the guard and, unless the cell is the very place read (the same root and
            // path), `place == cell`.
            for (const Cell& c : site_cells(s, site_ref(s, env)))
            {
                if (cell_suffix(c.type) != sfx)
                {
                    continue;
                }
                std::vector<int> full_path {s.path};
                full_path.insert(full_path.end(), c.path.begin(), c.path.end());
                E d {g};
                if (!(site_key(s) == k && full_path == path))
                {
                    d = low::and_(d, low::bin(BinOp::Eq, c.recv, p));
                }
                ds.push_back(d);
            }
        }
        // decision 0047 item 4: an `owns_block` site frames the reads of the cells of its block (`r->balance == 0`
        // after `owns_block(r)`), as an `owns` atom does
        {
            const FnBlocks& fb {blocks_of(*cf_)};
            for (size_t i {0U}; i < fb.sites.size(); ++i)
            {
                const AtomSite& s {fb.sites[i]};
                if (post)
                {
                    if (!s.in_ensures || i >= cbseen_)
                    {
                        continue;
                    }
                }
                else
                {
                    if (s.in_ensures)
                    {
                        continue;
                    }
                    if (!cens_ && i >= cbseen_)
                    {
                        continue;
                    }
                }
                // as 0042 item 5: a marked pointer's block never covers a read of the pre-state, and covers one of the post-state only for `must_init` (the result's: never)
                if (const InitMarker mk {block_marker(*cf_, s)};
                    mk != InitMarker::None && !(post && mk == InitMarker::MustInit && block_key(s) >= 0))
                {
                    continue;
                }
                const E g {site_guard(s, env)};
                for (const Cell& c : block_cells(s, block_ptr(s, env)))
                {
                    if (cell_suffix(c.type) != sfx)
                    {
                        continue;
                    }
                    E d {g};
                    if (!(block_key(s) == k && c.path == path))
                    {
                        d = low::and_(d, low::bin(BinOp::Eq, c.recv, p));
                    }
                    ds.push_back(d);
                }
            }
        }
        bool trivial {false};
        for (const E& d : ds)
        {
            // Under the one outer `old` the disjunct (a requires atom's, read plain) and the guard (in the return state) are in different states: spelling alike proves nothing (audit 2).
            if (!d || (!(old_state && lreads_heap(d)) && implied_by(d, guard)))
            {
                trivial = true;
            }
        }
        if (cmode_ == CMode::Wf)
        {
            const GKey key {k, path, post};
            wf_ghosts_[key] = t;
            if (!trivial)
            {
                E any;
                for (const E& d : ds)
                {
                    any = any ? low::bin(BinOp::Or, any, d) : d;
                }
                if (!any)
                {
                    any = low::bool_lit(false);
                }
                obl(full, guard, gdep, any, {},
                    meta("permission", e.loc,
                         cmeasure_ ? "the termination measure of `" + cf_->c_name + "` reads " +
                                         quoted(e.c_text.empty() ? std::string() : e.c_text, "a pointer's object") +
                                         " without owning it"
                                   : "the contract of `" + cf_->c_name + "` reads " +
                                         quoted(e.c_text.empty() ? std::string() : e.c_text, "a pointer's object") +
                                         " without owning it"));
            }
            return {low::var(ghost_name(key)), {}};
        }
        E read {cell_value(p, t)};
        bool wrap_whole {false};
        if (outer_old)
        {
            wrap_whole = lreads_heap(p);
            for (const E& d : ds)
            {
                wrap_whole = wrap_whole || lreads_heap(d);
            }
            if (!wrap_whole)
            {
                read = low::old_(read);
            }
        }
        if (trivial)
        {
            return {wrap_whole ? low::old_(read) : read, {}};
        }
        E v {inv_dummy(t)};
        for (size_t i {ds.size()}; i-- > 0U;)
        {
            v = low::ite(ds[i], read, v);
        }
        return {wrap_whole ? low::old_(v) : v, {}};
    }

    // ---- cell accesses inside expressions (decision 0039) ----

    struct Acc
    {
        int key {0};
        std::string field;
        bool write {false};
        bool call {false};
        E place;          // a read or a write: the Ref
        std::string text; // the user's spelling of the place
        E guard;          // the condition under which it happens (null: always), branch conditions included
        Deps dep;         // what its place and guard use
        Path path;
        SrcLoc loc;
        std::vector<Path> after; // see MemTag::after
        // the Full the access was recorded in; a closed branch's accesses move to the enclosing one
        const void* owner {nullptr};
        struct Atom
        {
            E place, guard;
            bool owns {false};
            std::string field, text;
            bool loads {false};
        }; // `loads`: the place is reached through a heap read (decision 0045)
        std::string callee;
        std::vector<Atom> atoms; // a memory call's atoms
    };
    struct BranchCtx
    {
        const void* outer {nullptr}; // the Full the branch effect will be added to
        Deps extra; // what the branch must follow beyond its condition (the places of earlier accesses its checks name)
    };

    // Whether the cell accesses of the node being lowered are tracked: code in a full expression (a contract and an annotation have no heap effects).
    bool track_mem() const
    {
        return in_full_ && cmode_ == CMode::None;
    }

    // The cell field of a type, as an identity that does not number an enumeration the way `cell_suffix` does (a name is only made when the program
    // needs one, so the Viper of a program without a store inside an expression stays as it was).
    static std::string fkey(const Type& t)
    {
        if (t.is_ptr())
        {
            return "ptr(" + fkey(*t.pointee) + ")";
        }
        if (t.is_struct())
        {
            return "struct@" + std::to_string(t.struct_type->id);
        }
        if (t.is_bool())
        {
            return "bool";
        }
        const IntType& it {*t.int_type};
        if (it.is_enum)
        {
            return "enum@" + std::to_string(reinterpret_cast<uintptr_t>(&it));
        }
        return std::to_string(it.bits) + (it.is_signed ? "s" : "u");
    }

    int key_id(const std::string& k)
    {
        auto it {key_ids_.find(k)};
        if (it == key_ids_.end())
        {
            it = key_ids_.emplace(k, -static_cast<int>(key_ids_.size()) - 1).first;
        }
        return it->second;
    }
    int key_of_cell(const Var& v)
    {
        return key_id("c:" + std::to_string(v.id));
    }
    // The root spelling of the struct object a struct store writes, and whether its members are reached by `->`: `x` of `&x`, `p->t` of `&p->t`, `g` of `&g`, `p` of `p`.
    static std::pair<std::string, bool> struct_dest_root(const Expr& addr)
    {
        if (addr.kind == Expr::Kind::GlobalAddr && addr.global != nullptr)
        {
            return {addr.global->c_name, false};
        }
        if (addr.kind == Expr::Kind::MemberAddr)
        {
            return {place_text(addr), false};
        }
        if (addr.kind == Expr::Kind::AddrOf && !addr.c_text.empty() && addr.c_text[0U] == '&')
        {
            return {addr.c_text.substr(1U), false};
        }
        return {addr.c_text, true};
    }

    // The key of the leaf at `more` below the struct object the address `a` names (decision 0043 item 12: `root.path`); an object of its own when the root is a pointer.
    int key_of_addr_leaf(const Expr& a, const std::vector<int>& more)
    {
        std::string k;
        std::vector<int> path;
        if (a.kind == Expr::Kind::AddrOf && a.var != nullptr && a.var->address_taken)
        {
            k = "c:" + std::to_string(a.var->id);
        }
        else if (a.kind == Expr::Kind::GlobalAddr && a.global != nullptr)
        {
            k = "g:" + std::to_string(a.global->id);
        }
        else if (a.kind == Expr::Kind::MemberAddr)
        {
            const Expr* root {member_root(&a, &path)};
            if (root != nullptr && root->kind == Expr::Kind::AddrOf && root->var != nullptr && root->var->address_taken)
            {
                k = "c:" + std::to_string(root->var->id);
            }
            else if (root != nullptr && root->kind == Expr::Kind::GlobalAddr && root->global != nullptr)
            {
                k = "g:" + std::to_string(root->global->id);
            }
        }
        if (k.empty())
        {
            return -1000000 - (++next_other_key_);
        }
        for (int x : path)
        {
            k += "." + std::to_string(x);
        }
        for (int x : more)
        {
            k += "." + std::to_string(x);
        }
        return key_id(k);
    }

    // The object a write or a read through the pointer expression `a` names (decision 0039 item 2): `&x` is the cell x, a pointer variable `v` is `*v`,
    // anything else is an object of its own.
    int key_of_addr(const Expr& a)
    {
        if (a.kind == Expr::Kind::GlobalAddr && a.global != nullptr)
        {
            return key_id("g:" + std::to_string(a.global->id)); // a global is one object (decision 0041 item 11)
        }
        if (a.kind == Expr::Kind::AddrOf && a.var != nullptr && a.var->address_taken)
        {
            return key_of_cell(*a.var);
        }
        if (a.kind == Expr::Kind::Local && a.var != nullptr && !a.var->address_taken)
        {
            return key_id("v:" + std::to_string(a.var->id));
        }
        // decision 0043 item 12: a member of a local or global object is the object `root.path` (`s.x` twice is one
        // object, `s.x` and `s.y` two)
        if (a.kind == Expr::Kind::MemberAddr)
        {
            std::vector<int> path;
            const Expr* root {member_root(&a, &path)};
            std::string k;
            if (root != nullptr && root->kind == Expr::Kind::AddrOf && root->var != nullptr && root->var->address_taken)
            {
                k = "c:" + std::to_string(root->var->id);
            }
            else if (root != nullptr && root->kind == Expr::Kind::GlobalAddr && root->global != nullptr)
            {
                k = "g:" + std::to_string(root->global->id);
            }
            if (!k.empty())
            {
                for (int x : path)
                {
                    k += "." + std::to_string(x);
                }
                return key_id(k);
            }
        }
        return -1000000 - (++next_other_key_);
    }

    // The cell fields of the atoms of `callee`'s `requires` that reach a value cell (`owns` and `reads`): its memory calls (decision 0039 item 4).
    std::set<std::string> memory_fields(const Function& callee)
    {
        std::set<std::string> r;
        for (const AtomSite& s : atoms_of(callee).sites)
        {
            if (!s.in_ensures && site_takes_cell(s))
            {
                // every leaf field of a struct atom (decision 0043 item 12, audit B3: a memory call on each of them, not on one field `struct_S`)
                const Type& t {site_pointee(s)};
                if (t.is_struct())
                {
                    for (const StructLeaf& l : struct_leaves(*t.struct_type))
                    {
                        r.insert(fkey(l.type));
                    }
                }
                else
                {
                    r.insert(fkey(t));
                }
            }
        }
        // decision 0047 item 9: a `requires` block site holds every leaf cell of its block at full permission (audit B3)
        for (const AtomSite& s : blocks_of(callee).sites)
        {
            if (s.in_ensures)
            {
                continue;
            }
            const Type& t {block_pointee(s)};
            if (t.is_struct())
            {
                for (const StructLeaf& l : struct_leaves(*t.struct_type))
                {
                    r.insert(fkey(l.type));
                }
            }
            else
            {
                r.insert(fkey(t));
            }
        }
        return r;
    }

    static bool writes_cell(const Expr* e)
    {
        if (e == nullptr)
        {
            return false;
        }
        if (e->kind == Expr::Kind::StoreE ||
            (e->kind == Expr::Kind::AssignE && e->var != nullptr && e->var->address_taken))
        {
            return true;
        }
        for (const Expr* x : {e->a, e->b, e->c})
        {
            if (writes_cell(x))
            {
                return true;
            }
        }
        for (const Expr* x : e->args)
        {
            if (writes_cell(x))
            {
                return true;
            }
        }
        return false;
    }

    // Whether an expression writes a cell or holds a memory call: what makes it a floor for the cell accesses after it (audit round 2, R2-B1).
    bool touches_cells(const Expr* e)
    {
        if (e == nullptr)
        {
            return false;
        }
        if (e->kind == Expr::Kind::StoreE ||
            (e->kind == Expr::Kind::AssignE && e->var != nullptr && e->var->address_taken))
        {
            return true;
        }
        if (e->kind == Expr::Kind::Call && e->callee != nullptr && !memory_fields(*e->callee).empty())
        {
            return true;
        }
        for (const Expr* x : {e->a, e->b, e->c})
        {
            if (touches_cells(x))
            {
                return true;
            }
        }
        for (const Expr* x : e->args)
        {
            if (touches_cells(x))
            {
                return true;
            }
        }
        return false;
    }

    // The writes of cells and the memory calls of a full expression, by field (E3's trigger, decision 0039 item 2): branch bodies included.
    void mem_scan(const Expr* e)
    {
        if (e == nullptr)
        {
            return;
        }
        if (e->kind == Expr::Kind::AssignE && e->var != nullptr && e->var->address_taken)
        {
            mem_w_.insert(fkey(e->var->type));
        }
        if (e->kind == Expr::Kind::StoreE)
        {
            if (e->type.is_struct())
            { // decision 0044 item 7: a struct write is a write of every leaf field
                for (const StructLeaf& l : struct_leaves(*e->type.struct_type))
                {
                    mem_w_.insert(fkey(l.type));
                }
            }
            else
            {
                mem_w_.insert(fkey(e->type));
            }
        }
        if (e->kind == Expr::Kind::Call && e->callee != nullptr)
        {
            for (const std::string& f : memory_fields(*e->callee))
            {
                ++mem_calls_[f];
            }
        }
        for (const Expr* x : {e->a, e->b, e->c})
        {
            mem_scan(x);
        }
        for (const Expr* x : e->args)
        {
            mem_scan(x);
        }
    }

    bool needs_snapshot(const std::string& f) const
    {
        if (mem_w_.count(f) != 0U)
        {
            return true;
        }
        const auto c {mem_calls_.find(f)};
        if (c == mem_calls_.end())
        {
            return false;
        }
        const auto d {mem_depth_.find(f)};
        return c->second > (d == mem_depth_.end() ? 0 : d->second);
    }

    // Decision 0039 item 4: the order C gives two accesses, from their paths (`p` created first): 1 `p` is before `q`, 2 they are never both made (the
    // arms of one `?:`), 0 C does not order them.  An operand comes before its operation (a node's own access follows what is inside it), the left of
    // `&&`, `||` and `,` and the condition of `?:` before the rest; every other pair of operands is unsequenced.
    // `pw`, `qw`: the access is a write of a cell (not a call).  A write inside the operands of another write (`*p = (*q)++`) is ordered with it only by
    // a sequence point between them, as 0020 item 4's stored-expression rule has it: the value computations are ordered, the side effects are not.
    static int order_rel(const Path& p, bool pw, const Path& q, bool qw, const std::vector<Path>& q_after)
    {
        // `q` reads through a value computed at u: what is inside u comes first (audit S2)
        for (const Path& u : q_after)
        {
            if (u.size() <= p.size() && std::equal(u.begin(), u.end(), p.begin()))
            {
                return 1;
            }
        }
        size_t i {0U};
        while (i < p.size() && i < q.size() && p[i].first == q[i].first)
        {
            ++i;
        }
        if (i == q.size() && i < p.size() && pw && qw)
        { // q's node is an ancestor of p's: a sequence point on the way down orders the two writes
            for (size_t l {i}; l < p.size(); ++l)
            {
                const Expr* par {l == 0U ? nullptr : p[l - 1U].first};
                if (par == nullptr)
                {
                    continue; // the statement's own store: the roots are its operands
                }
                const int slot {p[l].second};
                if (par->kind == Expr::Kind::Call)
                {
                    return 1;
                }
                if (slot == 0 &&
                    (par->kind == Expr::Kind::Comma || par->kind == Expr::Kind::Cond ||
                     (par->kind == Expr::Kind::Binary && (par->bop == BinaryOp::LAnd || par->bop == BinaryOp::LOr))))
                {
                    return 1;
                }
            }
            return 0;
        }
        if (i == q.size() || i == p.size())
        {
            return 1;
        }
        if (i == 0U)
        {
            return 0;
        }
        const Expr* n {p[i - 1U].first};
        const int sp {p[i].second};
        const int sq {q[i].second};
        switch (n->kind)
        {
            case Expr::Kind::Binary:
                return (n->bop == BinaryOp::LAnd || n->bop == BinaryOp::LOr) && sp < sq ? 1 : 0;
            case Expr::Kind::Comma:
                return sp < sq ? 1 : 0;
            case Expr::Kind::Cond:
                if (sp == 0 && sq > 0)
                {
                    return 1;
                }
                if (sp > 0 && sq > 0 && sp != sq)
                {
                    return 2;
                }
                return 0;
            default:
                return 0;
        }
    }

    E and_guard(const E& g) const
    {
        E r {g};
        for (const E& b : branch_guards_)
        {
            r = low::and_(r, b);
        }
        return r;
    }

    // Record an access and make the obligations of decision 0039 items 3 and 4 against every earlier access of the field that C does not order with
    // it: `g_a && g_b ==> place_a != place_b`, category `unsequenced`, at this (the later-created) access.  An earlier access recorded in an enclosing
    // full expression is named in a branch's set (audit S5): the branch follows what its places use.
    void register_access(Acc a, Full& full, const E& pure_guard)
    {
        a.guard = and_guard(pure_guard);
        a.path = path_;
        a.owner = &full;
        const size_t n {accs_.size()};
        for (size_t k {0U}; k < n; ++k)
        {
            const Acc& b {accs_[k]};
            if (!a.call && !b.call && b.field != a.field)
            {
                continue;
            }
            if (order_rel(b.path, b.write && !b.call, a.path, a.write && !a.call, a.after) != 0)
            {
                continue;
            }
            auto dep_of = [&](const Acc& x)
            {
                if (x.owner == &full)
                {
                    return x.dep;
                }
                for (BranchCtx& c : branch_ctx_)
                {
                    if (c.outer == x.owner)
                    {
                        c.extra = join(c.extra, x.dep);
                        break;
                    }
                }
                return Deps {};
            };
            auto emit = [&](E cond, E g, const std::string& message, const Acc& other, const SrcLoc* at = nullptr)
            {
                obl(full, g, {}, std::move(cond), join(dep_of(other), a.dep),
                    meta("unsequenced", at != nullptr ? *at : a.loc, message));
            };
            const E g_ab {low::and_(a.guard, b.guard)};
            if (!a.call && !b.call)
            {
                if (!a.write && !b.write)
                {
                    continue;
                }
                if (a.key == b.key)
                {
                    continue; // the static rule (0039 item 3) leaves no unsequenced conflicting pair of one object
                }
                const Acc& w {a.write ? a : b};
                const Acc& r {a.write ? b : a};
                emit(low::bin(low::BinOp::Ne, b.place, a.place), g_ab,
                     "modifies " + quoted(w.text, "the object `*(...)` designates") + " and accesses " +
                         quoted(r.text, "the object `*(...)` designates") +
                         " with no sequence point between, and they may be one object",
                     b);
            }
            else if (a.call != b.call)
            {
                const Acc& x {a.call ? b : a};
                const Acc& c {a.call ? a : b};
                for (const Acc::Atom& k : c.atoms)
                {
                    if (k.field != x.field)
                    {
                        continue;
                    }
                    if (!k.owns && !x.write)
                    {
                        continue;
                    }
                    const std::string callee {quoted(c.callee, "a function")};
                    std::string what {x.write ? "writes " + quoted(x.text, "the object `*(...)` designates") +
                                                    ", which the call to " + callee +
                                                    (k.owns ? " may change, " : " reads, ")
                                              : "accesses " + quoted(x.text, "the object `*(...)` designates") +
                                                    ", which the call to " + callee + " may change, "};
                    // decision 0047 item 9: a use of a pointer beside a call that may end the lifetime of its object
                    if (x.field == "lv")
                    {
                        what = "uses " + quoted(x.text, "a pointer") + ", whose object the call to " + callee +
                               " may end, ";
                    }
                    emit(low::bin(low::BinOp::Ne, x.place, k.place), low::and_(low::and_(x.guard, k.guard), c.guard),
                         what + "in an order C leaves unspecified", b, &x.loc); // at the access
                }
            }
            else
            {
                for (const Acc::Atom& k1 : b.atoms)
                {
                    for (const Acc::Atom& k2 : a.atoms)
                    {
                        if (k1.field != k2.field || (!k1.owns && !k2.owns))
                        {
                            continue;
                        }
                        emit(low::bin(low::BinOp::Ne, k1.place, k2.place),
                             low::and_(low::and_(k1.guard, k2.guard), g_ab),
                             "the calls to " + quoted(b.callee, "a function") + " and " +
                                 quoted(a.callee, "a function") + " may both " +
                                 (k1.field == "lv" ? "use or end the lifetime of the object " : "access ") +
                                 quoted(k2.text.empty() ? k1.text : k2.text, "the same object") +
                                 ", in an order C leaves unspecified",
                             b);
                    }
                }
            }
        }
        accs_.push_back(std::move(a));
    }

    // A cell read in code (by name or through a pointer; the obligations were made): the value, read where its consumer is emitted, or (E3 for memory,
    // decision 0039 item 2) a snapshot `t := g ? r.v_T : 0` when the full expression writes a cell of the field or holds a memory call of the field
    // that does not contain the read in its arguments.  The snapshot follows its guard's set (audit B1), the comma floor, its receiver's set and the
    // earlier writes of its key.
    LV mem_read(const Expr& e, int key, const E& recv, const Type& t, const std::string& text, const E& guard,
                const Deps& gdep, const Deps& pdep, Full& full, const Expr* via = nullptr)
    {
        const E value {cell_value(recv, t)};
        if (!track_mem())
        {
            return {value, pdep};
        }
        Acc a;
        a.key = key;
        a.field = fkey(t);
        a.place = recv;
        a.text = text;
        a.dep = join(gdep, pdep);
        a.loc = e.loc;
        std::vector<Path> after;
        if (via != nullptr && shared_.count(via) != 0U && shared_path_.count(via) != 0U)
        {
            after.push_back(shared_path_[via]);
        }
        a.after = after;
        register_access(std::move(a), full, guard);
        if (!needs_snapshot(fkey(t)) && !t.is_ptr())
        {
            return {value, pdep};
        }
        const std::string tmp {new_temp(ltype(t))};
        const auto lw {full.last_write.find(key)};
        Deps slot {join(floor_, pdep)};
        if (!opt_.drop_snapshot_guard_set)
        {
            slot = join(slot, gdep);
        }
        if (lw != full.last_write.end())
        {
            slot = join(slot, lw->second);
        }
        Effect fx;
        fx.kind = Effect::Kind::Snapshot;
        fx.name = tmp;
        fx.value = guard ? low::ite(guard, value, inv_dummy(t)) : value;
        if (t.is_ptr())
        {
            // a pointer loaded from a cell is split like a call's pointer result (decision 0040 item 2, audit B1)
            result_splits(t, low::var(tmp), fx.stmts);
        }
        fx.deps = slot;
        fx.seq = full.next_seq++;
        fx.reads.insert(key);
        fx.tags.push_back(MemTag {key, fkey(t), false, false, path_, after});
        full.effects.push_back(std::move(fx));
        touch(full, key, slot);
        return {low::var(tmp), slot};
    }

    // `*c := <struct value>` used as a value (decision 0044 item 7: the store of a compound literal `&(T){...}`, the left operand of the comma `(*cl := {...}, &cl)`;
    // check_program admits exactly that shape): the address and the elements are unsequenced operands of the full expression, every element is evaluated first
    // (audit B1), and then every leaf of the destination is written, zero leaves included.  Like `struct_store` inside an expression: a write access per destination
    // leaf (the sequencer's keys), the value leaves into temporaries first, per leaf the write permission as an obligation with the write's set, then the leaf
    // writes through Ref temporaries as one movable `Store` effect.  There is no null check (the address is a hidden local's), and the node's own value, a struct, is not used.
    LV struct_cell_write(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        if (guard != nullptr)
        {
            internal_error(e.loc, "an effect in a conditionally evaluated pure operand reached lowering");
            return struct_value({}, {});
        }
        if (!in_full_)
        {
            internal_error(e.loc, "a write inside an expression outside a full expression reached lowering");
            return struct_value({}, {});
        }
        if (e.c == nullptr || e.a == nullptr || e.yields_old || e.b != nullptr)
        {
            internal_error(e.loc, "a struct write used as a value that is not a plain store reached lowering");
            return struct_value({}, {});
        }
        const StructType& st {*e.type.struct_type};
        const size_t effect_from {full.effects.size()};
        const size_t obl_from {full.obls.size()};
        const LV addr {lower(*e.c, env, guard, gdep, full)};
        agg_wf_ = true; // a pointer element stored into a pointer leaf must be well-formed (decision 0040 item 2)
        const LV stored {lower(*e.a, env, guard, gdep, full)};
        agg_wf_ = false;
        if (!is_aggregate_value(*e.a))
        {
            struct_ptr_uses(*e.a, stored, guard, gdep, full, true);
        }
        const std::vector<StructLeaf> plain {struct_leaves(st)};
        if (stored.leaves.size() != plain.size())
        {
            internal_error(
                e.loc,
                "a struct write whose value has a different number of leaves than its destination reached lowering");
            return struct_value({}, {});
        }
        const std::pair<std::string, bool> root {struct_dest_root(*e.c)};
        const std::vector<StructLeaf> named {struct_leaves(st, root.first, root.second)};
        const Deps operands {since(full, effect_from, obl_from)};
        std::vector<int> keys;
        Deps wset {join(join(floor_, stored.dep), join(addr.dep, operands))};
        for (const StructLeaf& l : plain)
        {
            keys.push_back(key_of_addr_leaf(*e.c, l.path));
            const auto la {full.last_access.find(keys.back())};
            if (la != full.last_access.end())
            {
                wset = join(wset, la->second);
            }
        }
        Effect fx;
        fx.kind = Effect::Kind::Store;
        for (size_t i {0U}; i < plain.size(); ++i)
        {
            Acc acc;
            acc.key = keys[i];
            acc.field = fkey(plain[i].type);
            acc.write = true;
            acc.place = chain_ref(addr.e, plain[i].path);
            acc.text = named[i].text;
            acc.dep = addr.dep;
            acc.loc = e.loc;
            register_access(std::move(acc), full, nullptr);
        }
        // Every element's value into a temporary before the first leaf is written (an element may read a leaf of the destination's own object through an alias).
        std::string recv_name;
        if (addr.e->kind == low::LExpr::Kind::Var)
        {
            recv_name = addr.e->text;
        }
        else
        {
            recv_name = new_temp(low::LType::Ref);
            assign(fx.stmts, recv_name, addr.e);
        }
        const E base {low::var(recv_name)};
        std::vector<E> values;
        for (size_t i {0U}; i < plain.size(); ++i)
        {
            E v {stored.leaves[i]};
            if (v->kind != low::LExpr::Kind::Var)
            {
                const std::string t {new_temp(ltype(plain[i].type))};
                assign(fx.stmts, t, v);
                v = low::var(t);
            }
            values.push_back(v);
        }
        for (size_t i {0U}; i < plain.size(); ++i)
        {
            const E place {chain_ref(base, plain[i].path)};
            obl(full, nullptr, {}, may_write(place, plain[i].type, false), wset,
                meta("permission", e.loc,
                     "writes " +
                         (e.c->kind == Expr::Kind::AddrOf && e.c->var != nullptr
                              ? object_phrase(*e.c->var, named[i].text)
                              : quoted(named[i].text, "a member of the compound literal")) +
                         " without owning it: its object may have ended"));
            const std::string t {new_temp(low::LType::Ref)};
            assign(fx.stmts, t, place);
            write_fields(fx.stmts, t, plain[i].type, values[i]);
        }
        fx.deps = wset;
        fx.seq = full.next_seq++;
        for (size_t i {0U}; i < plain.size(); ++i)
        {
            fx.writes.insert(keys[i]);
            fx.tags.push_back(MemTag {keys[i], fkey(plain[i].type), false, true, path_, {}});
            full.last_write[keys[i]] = wset;
        }
        full.effects.push_back(std::move(fx));
        for (int k : keys)
        {
            touch(full, k, wset);
        }
        return struct_value(values, wset);
    }

    // A write of a cell used as a value (`AssignE` of a cell, `StoreE`; decision 0039 item 2): the operands, then the obligations with the write's
    // set (the null check for a pointer, the write permission), then the write as a movable effect.  The set holds the comma floor, the operands' sets,
    // every fixed point created in the operands (both halves, audit S3) and every earlier access of the key.
    LV cell_write(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        if (e.kind == Expr::Kind::StoreE && e.type.is_struct())
        {
            return struct_cell_write(e, env, guard, gdep, full);
        }
        if (guard != nullptr)
        {
            internal_error(e.loc, "an effect in a conditionally evaluated pure operand reached lowering");
            return {low::int_lit("0"), {}};
        }
        if (!in_full_)
        {
            internal_error(e.loc, "a write inside an expression outside a full expression reached lowering");
            return {low::int_lit("0"), {}};
        }
        const bool store {e.kind == Expr::Kind::StoreE};
        const size_t effect_from {full.effects.size()};
        const size_t obl_from {full.obls.size()};
        LV addr;
        const Type& t {store ? e.type : e.var->type};
        int key;
        std::string text;
        const bool member {store && e.c->kind == Expr::Kind::MemberAddr};
        if (member)
        {
            place_nodes_[e.c] = e.loc;
        }
        if (store)
        {
            addr = lower(*e.c, env, guard, gdep, full);
            key = key_of_addr(*e.c);
            text = place_text(*e.c);
            if (e.b != nullptr || shares_node(*e.a, e.c))
            { // a compound store's value reads `*addr` through this node: one evaluation
                shared_[e.c] = addr;
                Path where {path_};
                where.emplace_back(e.c, 2); // the slot of `c` in a StoreE
                shared_path_[e.c] = where;
            }
        }
        else
        {
            addr = {low::var(var_name(*e.var)), {}};
            key = key_of_cell(*e.var);
            text = e.var->name;
        }
        LV old_value;
        if (e.b != nullptr)
        {
            old_value = lower(*e.b, env, guard, gdep, full);
            shared_[e.b] = old_value;
        }
        LV stored {lower(*e.a, env, guard, gdep, full)};
        if (e.b != nullptr)
        {
            shared_.erase(e.b);
        }
        if (store)
        {
            shared_.erase(e.c);
            shared_path_.erase(e.c);
        }
        if (t.is_ptr())
        {
            // a pointer stored in a cell is a use of its value, and must be well-formed (decision 0040 item 2)
            ptr_use(*e.a, stored, guard, gdep, full, true);
        }
        const Deps operands {since(full, effect_from, obl_from)};
        const auto la {full.last_access.find(key)};
        Deps wset {join(join(join(floor_, stored.dep), addr.dep), operands)};
        if (la != full.last_access.end())
        {
            wset = join(wset, la->second);
        }
        Acc acc;
        acc.key = key;
        acc.field = fkey(t);
        acc.write = true;
        acc.place = addr.e;
        acc.text = text;
        // what the place value uses, not the value computation: an obligation naming this write must not follow the
        // stored value's calls (0021 item 4)
        acc.dep = addr.dep;
        acc.loc = e.loc;
        register_access(std::move(acc), full, nullptr);
        if (store && !member)
        {
            obl(full, nullptr, {}, low::bin(low::BinOp::Ne, addr.e, low::null_()), wset,
                meta("null-dereference", e.loc,
                     "dereferences " + quoted(e.c->c_text, "the pointer") + ", which may be null"));
        }
        obl(full, nullptr, {}, may_write(addr.e, t, false), wset,
            meta("permission", e.loc,
                 store ? (rooted_at_global(*e.c) ? "writes `" + text + "` without owning it: `" + fn_.f->c_name +
                                                       "` needs `owns(" + text + ")` in its contract"
                                                 : "writes " + quoted(text, "the object the pointer points to") +
                                                       " without owning it: the object it points to may have ended")
                       : "writes `" + text + "` without owning it: a callee may have kept its object"));
        Effect fx;
        fx.kind = Effect::Kind::Store;
        std::string recv_name;
        if (addr.e->kind == low::LExpr::Kind::Var)
        {
            recv_name = addr.e->text;
        }
        else
        {
            recv_name = new_temp(low::LType::Ref);
            assign(fx.stmts, recv_name, addr.e);
        }
        write_fields(fx.stmts, recv_name, t, stored.e);
        fx.deps = wset;
        fx.seq = full.next_seq++;
        fx.writes.insert(key);
        fx.tags.push_back(MemTag {key, fkey(t), false, true, path_, {}});
        full.effects.push_back(std::move(fx));
        full.last_write[key] = wset;
        touch(full, key, wset);
        return e.yields_old ? old_value : stored;
    }

    // ---- expressions ----

    // `lower` records the node's place in the tree (decision 0039 item 4) and lowers it.
    LV lower(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        int slot {0};
        if (!path_.empty())
        {
            const Expr* par {path_.back().first};
            if (par->a == &e)
            {
                slot = 0;
            }
            else if (par->b == &e)
            {
                slot = 1;
            }
            else if (par->c == &e)
            {
                slot = 2;
            }
            else
            {
                for (size_t k {0U}; k < par->args.size(); ++k)
                {
                    if (par->args[k] == &e)
                    {
                        slot = 3 + static_cast<int>(k);
                    }
                }
            }
        }
        path_.emplace_back(&e, slot);
        LV r {lower_node(e, env, guard, gdep, full)};
        path_.pop_back();
        return r;
    }

    LV lower_node(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        if (!shared_.empty())
        { // a node used twice (the pointer of a compound store, decision 0032 item 9): lowered once
            const auto it {shared_.find(&e)};
            if (it != shared_.end() && e.kind != Expr::Kind::Local)
            {
                return it->second;
            }
        }
        switch (e.kind)
        {
            case Expr::Kind::Const:
                if (e.type.is_bool())
                {
                    return {low::bool_lit(e.value == "true"), {}};
                }
                return {low::int_lit(e.value), {}};
            case Expr::Kind::Local:
                if (!shared_.empty())
                {
                    const auto it {shared_.find(&e)}; // the old value of `x++`: read once (decision 0020)
                    if (it != shared_.end())
                    {
                        return it->second;
                    }
                }
                if (e.var->address_taken)
                {
                    return cmode_ == CMode::Inv ? inv_cell(e, guard, gdep, full) : read_cell(e, guard, gdep, full);
                }
                if (in_full_ && written_.count(e.var->id) != 0U)
                {
                    return snapshot(e, guard, gdep, full);
                }
                // a read of a local declared without an initializer: its flag must be set (decision 0019 item 4)
                if (flagged(*e.var))
                {
                    obl(full, guard, gdep, low::var(flag_name(*e.var)), {},
                        meta("uninitialized", e.loc,
                             "`" + e.var->name + "` may be read here before it is given a value"));
                }
                return {low::var(var_name(*e.var)), {}};
            case Expr::Kind::Formal:
            {
                const size_t k {static_cast<size_t>(e.index)};
                if (e.type.is_struct())
                {
                    return struct_value(env.formal_leaves[k], k < env.formal_dep.size() ? env.formal_dep[k] : Deps {});
                }
                return {env.formals[k], k < env.formal_dep.size() ? env.formal_dep[k] : Deps {}};
            }
            case Expr::Kind::Result:
                if (e.type.is_struct())
                {
                    return struct_value(env.result_leaves, {});
                }
                return {env.result, {}};
            case Expr::Kind::Old:
                if (cmode_ != CMode::None && reads_heap(*e.a))
                { // old(*p): the cell's value on entry (decision 0037 item 5)
                    const bool saved {cold_};
                    cold_ = true;
                    LV r {lower(*e.a, env, guard, gdep, full)};
                    cold_ = saved;
                    return r;
                }
                return lower(*e.a, env, guard, gdep, full);
            case Expr::Kind::Unary:
                return unary(e, env, guard, gdep, full);
            case Expr::Kind::Binary:
                return binary(e, env, guard, gdep, full);
            case Expr::Kind::Cond:
            {
                if (cmode_ == CMode::None && (e.type.is_ptr() || has_effect(*e.b) || has_effect(*e.c)))
                {
                    return branch_cond(e, env, guard, gdep, full);
                }
                const size_t cond_effect_from {full.effects.size()};
                const size_t cond_obl_from {full.obls.size()};
                LV c {lower(*e.a, env, guard, gdep, full)};
                E gt {low::and_(guard, c.e)};
                E gf {low::and_(guard, low::not_(c.e))};
                Deps gd {join(gdep, c.dep)};
                // A condition that writes a cell or calls a function that may is complete, effects included, before the arms read a cell (audit R2-B1).
                if (track_mem() && !opt_.drop_left_floor && touches_cells(e.a))
                {
                    gd = join(gd, since(full, cond_effect_from, cond_obl_from));
                }
                LV t {lower(*e.b, env, gt, gd, full)};
                LV f {lower(*e.c, env, gf, gd, full)};
                return {low::ite(c.e, t.e, f.e), join(join(c.dep, t.dep), f.dep)};
            }
            case Expr::Kind::Convert:
                return convert(e, env, guard, gdep, full);
            case Expr::Kind::ToBool:
            {
                LV a {lower(*e.a, env, guard, gdep, full)};
                if (e.a->type.is_ptr())
                { // a pointer as a truth value is `p != null` (decision 0032 item 7), a use of its value
                    ptr_use(*e.a, a, guard, gdep, full);
                    return {low::bin(low::BinOp::Ne, a.e, low::null_()), a.dep};
                }
                return {low::bin(low::BinOp::Ne, a.e, low::int_lit("0")), a.dep};
            }
            case Expr::Kind::BoolToInt:
            {
                LV a {lower(*e.a, env, guard, gdep, full)};
                return {low::ite(a.e, low::int_lit("1"), low::int_lit("0")), a.dep};
            }
            case Expr::Kind::ReportAt:
            {
                const SrcLoc* saved {report_at_};
                const std::string* saved_note {report_note_};
                const SrcLoc where {e.loc};
                // The innermost ReportAt wins only if no outer one is active: an outer one already names the call.
                if (report_at_ == nullptr)
                {
                    report_at_ = &where;
                    report_note_ = &e.value;
                }
                LV a {lower(*e.a, env, guard, gdep, full)};
                report_at_ = saved;
                report_note_ = saved_note;
                return a;
            }
            case Expr::Kind::Call:
                return call(e, env, guard, gdep, full);
            case Expr::Kind::AssignE:
                if (e.var != nullptr && e.var->address_taken)
                {
                    return cell_write(e, env, guard, gdep, full);
                }
                return assign_value(e, env, guard, gdep, full);
            case Expr::Kind::StoreE:
                return cell_write(e, env, guard, gdep, full);
            case Expr::Kind::Comma:
                return comma(e, env, guard, gdep, full);
            case Expr::Kind::Null:
                return {low::null_(), {}};
            case Expr::Kind::AddrOf:
            { // the cell's Ref: a pointer value, no obligation of its own (decision 0032 item 6)
                if (e.var == nullptr || !e.var->address_taken)
                {
                    internal_error(e.loc, "the address of a variable that is not a cell reached lowering");
                    return {low::null_(), {}};
                }
                return {low::var(var_name(*e.var)), {}};
            }
            case Expr::Kind::MemberAddr:
                return member_addr(e, env, guard, gdep, full);
            // `&g`: the address of the global's cell, a pointer value with no obligation (decision 0041 item 6)
            case Expr::Kind::GlobalAddr:
                if (e.global == nullptr)
                {
                    internal_error(e.loc, "the address of a global without a global reached lowering");
                    return {low::null_(), {}};
                }
                return {global_ref(e.global->id), {}};
            case Expr::Kind::Deref:
                return deref(e, env, guard, gdep, full);
            case Expr::Kind::Field:
                return field_of(e, env, guard, gdep, full);
            case Expr::Kind::Aggregate:
                return aggregate(e, env, guard, gdep, full);
            case Expr::Kind::Atom:
                // a written atom's rows are the loop's own (decision 0040 item 5); its own loads are reads of the
                // invariant, framed like any (decision 0045 item 12)
                if (cmode_ == CMode::Inv)
                {
                    if (e.a != nullptr && reads_heap(*e.a))
                    {
                        lower(*e.a, env, guard, gdep, full);
                    }
                    return {low::bool_lit(true), {}};
                }
                if (cmode_ == CMode::None)
                {
                    internal_error(e.loc, "a permission atom outside a contract reached lowering");
                    return {low::bool_lit(true), {}};
                }
                {
                    LV r {atom(e, env, guard, gdep, &full)};
                    if (cmode_ == CMode::Wf)
                    {
                        overlap_obligations(e, env, guard, gdep, full);
                    }
                    return r;
                }
        }
        internal_error(e.loc, "unknown expression kind");
        return {low::int_lit("0"), {}};
    }

    // ---- struct values (decision 0044 item 2) ----

    // A struct value as an LV: its leaves, and the poison `e` (a struct value that reaches a consumer other than item 2's prints it; `check_poison` makes that an internal error).
    static LV struct_value(std::vector<E> leaves, const Deps& dep)
    {
        LV r;
        r.e = low::var(kPoison);
        r.dep = dep;
        r.leaves = std::move(leaves);
        return r;
    }

    // `a.m` (decision 0044 item 2): the member's leaf (or the leaves of a nested struct member) of the value's tuple; no obligation of its own.
    LV field_of(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        if (e.a == nullptr || !e.a->type.is_struct() || e.index < 0 ||
            static_cast<size_t>(e.index) >= e.a->type.struct_type->members.size())
        {
            internal_error(e.loc, "a member of anything but a struct value and one of its members reached lowering");
            return {low::int_lit("0"), {}};
        }
        const LV a {lower(*e.a, env, guard, gdep, full)};
        const auto [start, width] = leaf_span(*e.a->type.struct_type, e.index);
        if (a.leaves.size() < start + width)
        {
            internal_error(e.loc, "a struct value with fewer leaves than its type reached lowering");
            return {low::int_lit("0"), {}};
        }
        std::vector<E> part(a.leaves.begin() + static_cast<std::ptrdiff_t>(start),
                            a.leaves.begin() + static_cast<std::ptrdiff_t>(start + width));
        if (e.type.is_struct())
        {
            return struct_value(std::move(part), a.dep);
        }
        return {part.front(), a.dep};
    }

    // `(T){a0, a1, ...}` as a value: one argument per leaf, lowered as unsequenced siblings (decision 0044 item 2); a pointer leaf is a use of the pointer's value
    // (a copy into a temporary, decision 0032 item 6), and `agg_wf_` (a struct store's own operand) also requires it to be well-formed (decision 0040 item 2).
    LV aggregate(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        if (cmode_ != CMode::None)
        {
            internal_error(e.loc, "an aggregate in a contract reached lowering");
            return struct_value({}, {});
        }
        const bool wf {agg_wf_};
        agg_wf_ = false;
        std::vector<E> leaves;
        Deps dep;
        for (const Expr* a : e.args)
        {
            const LV v {lower(*a, env, guard, gdep, full)};
            if (a->type.is_ptr())
            {
                ptr_use(*a, v, guard, gdep, full, wf);
            }
            leaves.push_back(v.e);
            dep = join(dep, v.dep);
        }
        return struct_value(std::move(leaves), dep);
    }

    // `&a->m` (decision 0043 item 5): `loc(member(a, k), 0)`.  A PLACE ACCESS (the consumer put the node in `place_nodes_`: the address of a Deref, a Store or a
    // StoreE, or the operand of another place MemberAddr) is `p->x`, `s.t.x`: the chain's root pointer is checked non-null (`null-dereference`; the chain's own
    // address is never null, `loc_nonnull`), nothing about its liveness (0032 item 6: not a use of `p`'s value).  Any other MemberAddr is an address used as a
    // value: arithmetic on the root pointer (0028 item 3a), `null-dereference` and `lifetime`.  The root is checked once, by the innermost MemberAddr of a chain; the
    // address of a local struct (`AddrOf`) or of a global has no obligation.  In a contract the node is a pure address chain.
    LV member_addr(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        const auto pn {place_nodes_.find(&e)};
        const bool place {pn != place_nodes_.end()};
        // a place access is reported at its consumer (the read, the store), as `*p` is
        const SrcLoc at {place ? pn->second : e.loc};
        if (place)
        {
            place_nodes_.erase(pn);
        }
        if (e.a == nullptr || !e.a->type.is_ptr() || e.a->type.pointee == nullptr || !e.a->type.pointee->is_struct() ||
            e.index < 0 || static_cast<size_t>(e.index) >= e.a->type.pointee->struct_type->members.size())
        {
            internal_error(
                e.loc,
                "a member address of anything but a pointer to a struct and one of its members reached lowering");
            return {low::null_(), {}};
        }
        const Expr& base {*e.a};
        const bool inner {base.kind == Expr::Kind::MemberAddr};
        if (inner && place)
        {
            place_nodes_[&base] = at;
        }
        const LV b {lower(base, env, guard, gdep, full)};
        if (!inner && cmode_ == CMode::None && !address_or_null(base))
        {
            const std::string ptr_text {base.c_text};
            // The address of a member evaluates `*p` as an lvalue, undefined for a null pointer and for one past the end (decision 0043 item 5, audit S3): one row.
            obl(full, guard, gdep,
                place ? low::bin(low::BinOp::Ne, b.e, low::null_())
                      : low::and_(low::bin(low::BinOp::Ne, b.e, low::null_()), in_block(b.e)),
                b.dep,
                meta("null-dereference", at,
                     place ? "dereferences " + quoted(ptr_text, "the pointer") + ", which may be null"
                           : "forms " + quoted(e.c_text, "the address of a member") + ", but " +
                                 quoted(ptr_text, "the pointer") + " may be null or point past the end of its object"));
            if (!place)
            {
                ptr_use(base, b, guard, gdep, full, false, &e.loc);
            }
        }
        return {member_ref(b.e, e.index), b.dep};
    }

    LV unary(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        LV a {lower(*e.a, env, guard, gdep, full)};
        switch (e.uop)
        {
            case UnaryOp::Not:
                return {low::not_(a.e), a.dep};
            case UnaryOp::Plus:
                return a;
            case UnaryOp::Neg:
            {
                const IntType& t {*e.type.int_type};
                if (t.is_signed)
                {
                    // -a fits iff a != T_MIN (T_MIN = -2^(bits-1), and the range is asymmetric).
                    const dec::Range r {dec::range_of(t)};
                    obl(full, guard, gdep, low::bin(low::BinOp::Ne, a.e, low::int_lit(r.lo)), a.dep,
                        meta("overflow", e.loc, "negating the smallest value of `" + spell(e) + "` overflows"));
                    return {low::neg(a.e), a.dep};
                }
                // Unsigned (decision 0022 item 1): -a fits iff a == 0; cerastes does not wrap.
                obl(full, guard, gdep, low::bin(low::BinOp::Eq, a.e, low::int_lit("0")), a.dep,
                    meta("overflow", e.loc,
                         "negating a nonzero `" + spell(e) + "` value overflows (cerastes does not wrap)"));
                return {low::neg(a.e), a.dep};
            }
            case UnaryOp::BitNot:
            {
                // Decision 0023 item 1: -1 - a (signed) or MAX - a (unsigned): linear, in range for every a, no obligation.
                const IntType& t {*e.type.int_type};
                // The same guard as the binary bitwise operators (M4.4b1 audit note 1): check_program rejects an enumeration operand and the
                // encoder converts to a computed type of 32, 64 or 128 bits, so this is unreachable, and an internal error if it is not.
                if (t.is_enum || (t.bits != 32 && t.bits != 64 && t.bits != 128))
                {
                    internal_error(e.loc,
                                   "a bitwise complement on a " + std::to_string(t.bits) +
                                       "-bit type (the encoder converts to a computed type of 32, 64 or 128 bits)");
                }
                const E k {low::int_lit(t.is_signed ? "-1" : dec::range_of(t).hi)};
                return {low::bin(low::BinOp::Sub, k, a.e), a.dep};
            }
        }
        internal_error(e.loc, "unknown unary operator");
        return a;
    }

    static const char* op_noun(BinaryOp op)
    {
        switch (op)
        {
            case BinaryOp::Add:
                return "addition";
            case BinaryOp::Sub:
                return "subtraction";
            case BinaryOp::Mul:
                return "multiplication";
            case BinaryOp::Div:
                return "division";
            case BinaryOp::Rem:
                return "remainder";
            default:
                return "operation";
        }
    }

    LV binary(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        using low::BinOp;
        const bool lazy_and {e.bop == BinaryOp::LAnd};
        const bool lazy_or {e.bop == BinaryOp::LOr};
        if ((lazy_and || lazy_or) && cmode_ == CMode::None && has_effect(*e.b))
        {
            return branch_logical(e, env, guard, gdep, full);
        }
        const size_t left_effect_from {full.effects.size()};
        const size_t left_obl_from {full.obls.size()};
        LV a {lower(*e.a, env, guard, gdep, full)};
        E gb {guard};
        Deps gbd {gdep};
        if (lazy_and)
        {
            gb = low::and_(guard, a.e);
            gbd = join(gdep, a.dep);
        }
        if (lazy_or)
        {
            gb = low::and_(guard, low::not_(a.e));
            gbd = join(gdep, a.dep);
        }
        // The left operand's cell writes and memory calls are complete, with everything they follow, before the right operand reads a cell (audit R2-B1);
        // in both halves, whatever the left operand's value carries.
        if ((lazy_and || lazy_or) && track_mem() && !opt_.drop_left_floor && touches_cells(e.a))
        {
            gbd = join(gbd, since(full, left_effect_from, left_obl_from));
        }
        LV b {lower(*e.b, env, gb, gbd, full)};
        const Deps dep {join(a.dep, b.dep)};
        if ((e.bop == BinaryOp::Eq || e.bop == BinaryOp::Ne) && e.a->type.is_ptr())
        {
            // Decision 0032 items 6 and 7: both operands are uses of a pointer value, then the one-past-the-end exception of 0028 item 3.
            ptr_use(*e.a, a, guard, gdep, full);
            ptr_use(*e.b, b, guard, gdep, full);
            if (cmode_ == CMode::None)
            {
                // in a contract `==` is identity in the model (decision 0037 item 12)
                one_past_end(e, a, b, guard, gdep, dep, full);
            }
        }
        switch (e.bop)
        {
            case BinaryOp::LAnd:
                if ((cmode_ == CMode::Call || cmode_ == CMode::Ret) && !atoms_of(*cf_).sites.empty())
                { // an atom lowers to `true` here (asserted one by one): keep the value part alone
                    if (always_true(a.e))
                    {
                        return {b.e, dep};
                    }
                    if (always_true(b.e))
                    {
                        return {a.e, dep};
                    }
                }
                return {low::bin(BinOp::And, a.e, b.e), dep};
            case BinaryOp::LOr:
                // Viper refuses `acc` under `||`: `a || atom` is `!a ==> atom` (decision 0037 item 5).
                if (cmode_ == CMode::Method && has_atom(*e.b))
                {
                    return {low::bin(BinOp::Implies, low::not_(a.e), b.e), dep};
                }
                return {low::bin(BinOp::Or, a.e, b.e), dep};
            case BinaryOp::Lt:
                return {low::bin(BinOp::Lt, a.e, b.e), dep};
            case BinaryOp::Le:
                return {low::bin(BinOp::Le, a.e, b.e), dep};
            case BinaryOp::Gt:
                return {low::bin(BinOp::Gt, a.e, b.e), dep};
            case BinaryOp::Ge:
                return {low::bin(BinOp::Ge, a.e, b.e), dep};
            case BinaryOp::Eq:
                return {low::bin(BinOp::Eq, a.e, b.e), dep};
            case BinaryOp::Ne:
                return {low::bin(BinOp::Ne, a.e, b.e), dep};
            case BinaryOp::Add:
            case BinaryOp::Sub:
            case BinaryOp::Mul:
            {
                const IntType& t {*e.type.int_type};
                const BinOp op {e.bop == BinaryOp::Add ? BinOp::Add
                                                       : (e.bop == BinaryOp::Sub ? BinOp::Sub : BinOp::Mul)};
                E v {low::bin(op, a.e, b.e)};
                if (t.is_signed)
                {
                    obl(full, guard, gdep, range_obligation(v, t), dep,
                        meta("overflow", e.loc,
                             std::string("signed `") + spell(e) + "` " + op_noun(e.bop) + " may overflow"));
                    return {v, dep};
                }
                // Unsigned (decision 0022 item 1): the exact result must fit; cerastes does not wrap.
                obl(full, guard, gdep, range_obligation(v, t), dep,
                    meta("overflow", e.loc,
                         std::string("unsigned `") + spell(e) + "` " + op_noun(e.bop) +
                             " may overflow (cerastes does not wrap)"));
                return {v, dep};
            }
            case BinaryOp::Div:
            case BinaryOp::Rem:
            {
                const IntType& t {*e.type.int_type};
                const bool div {e.bop == BinaryOp::Div};
                obl(full, guard, gdep, low::bin(BinOp::Ne, b.e, low::int_lit("0")), b.dep,
                    meta("division-by-zero", e.loc,
                         div ? "the divisor may be zero" : "the divisor of the remainder may be zero"));
                if (t.is_signed)
                {
                    const dec::Range r {dec::range_of(t)};
                    E bad {low::and_(low::bin(BinOp::Eq, a.e, low::int_lit(r.lo)),
                                     low::bin(BinOp::Eq, b.e, low::int_lit("-1")))};
                    obl(full, guard, gdep, low::not_(bad), dep,
                        meta("overflow", e.loc,
                             std::string(div ? "dividing" : "taking the remainder of") + " the smallest value of `" +
                                 spell(e) + "` by -1 overflows"));
                }
                // Unsigned operands are non-negative, and the plain functions are easier for the solver (prelude.h).
                const char* name {t.is_signed ? (div ? "c_div" : "c_rem") : (div ? "c_udiv" : "c_urem")};
                use(name);
                return {low::fn(name, {a.e, b.e}), dep};
            }
            case BinaryOp::Shl:
            case BinaryOp::Shr:
            {
                const IntType& t {*e.type.int_type}; // the (promoted) left operand's type
                const bool left {e.bop == BinaryOp::Shl};
                const std::string width {std::to_string(t.bits)};
                obl(full, guard, gdep, low::in_range_atomic(b.e, "0", dec::dec1(width)), b.dep,
                    meta("shift", e.loc,
                         "the shift count may be negative or at least the width (" + width + " bits) of `" + spell(e) +
                             "`"));
                use("c_pow2");
                if (!left)
                {
                    use("c_shr");
                    return {low::fn("c_shr", {a.e, b.e}), dep};
                }
                E v {low::bin(BinOp::Mul, a.e, low::fn("c_pow2", {b.e}))};
                if (t.is_signed)
                {
                    // C23 6.5.8: a negative left operand, or a result that does not fit, is undefined; cerastes uses
                    // that rule in C++20 too (0010 item 8).
                    obl(full, guard, gdep, low::bin(BinOp::Ge, a.e, low::int_lit("0")), a.dep,
                        meta("shift", e.loc, "the left operand of `<<` may be negative"));
                    const dec::Range r {dec::range_of(t)};
                    obl(full, guard, gdep, low::bin(BinOp::Le, v, low::int_lit(r.hi)), dep,
                        meta("shift", e.loc, "the result of `<<` may not fit in `" + spell(e) + "`"));
                    return {v, dep};
                }
                // Unsigned (decision 0022 item 1): a >= 0 by type, so only the result must fit; cerastes does not wrap.
                obl(full, guard, gdep, low::bin(BinOp::Le, v, low::int_lit(dec::range_of(t).hi)), dep,
                    meta("shift", e.loc, "the result of `<<` may not fit in `" + spell(e) + "`"));
                return {v, dep};
            }
            case BinaryOp::BitAnd:
            case BinaryOp::BitOr:
            case BinaryOp::BitXor:
            {
                // Decision 0023 item 2 (S21): total prelude functions c_[u]<op>_N by the computed type's signedness and width; the
                // result is always in range, so no obligation.
                const IntType& t {*e.type.int_type};
                if (t.is_enum || (t.bits != 32 && t.bits != 64 && t.bits != 128))
                {
                    internal_error(e.loc,
                                   "a bitwise operator on a " + std::to_string(t.bits) +
                                       "-bit type (the encoder converts to a computed type of 32, 64 or 128 bits)");
                }
                // Decision 0026 item 7 (`&`) and decision 0027 item 1 (`|`, `^`): a constant mask (either operand, as lowered; both operands
                // are lowered as before, so their obligations stay) is a remainder or a quotient; anything else keeps the prelude function,
                // which is exact for every operand.
                {
                    const MaskOp mop {e.bop == BinaryOp::BitAnd
                                          ? MaskOp::And
                                          : (e.bop == BinaryOp::BitOr ? MaskOp::Or : MaskOp::Xor)};
                    const std::function<void(const std::string&)> used = [this](const std::string& n)
                    {
                        use(n);
                    };
                    E r {mask_rewrite(mop, t, a.e, b.e, used)};
                    if (r == nullptr)
                    {
                        r = mask_rewrite(mop, t, b.e, a.e, used);
                    }
                    if (r != nullptr)
                    {
                        return {r, dep};
                    }
                }
                const char* op {e.bop == BinaryOp::BitAnd ? "and" : (e.bop == BinaryOp::BitOr ? "or" : "xor")};
                const std::string name {std::string("c_") + (t.is_signed ? "" : "u") + op + "_" +
                                        std::to_string(t.bits)};
                use(name);
                return {low::fn(name, {a.e, b.e}), dep};
            }
        }
        internal_error(e.loc, "unknown binary operator");
        return a;
    }

    LV convert(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        LV a {lower(*e.a, env, guard, gdep, full)};
        const IntType& s {*e.a->type.int_type};
        const IntType& t {*e.type.int_type};
        if (dec::contains(dec::range_of(t), dec::range_of(s)))
        {
            return a; // every source value is a target value
        }
        if (t.is_enum)
        {
            // Into an enumeration: one of its values, whatever the underlying type's signedness; never a wrap (decision 0018
            // item 5).
            const dec::Range r {dec::range_of(t)};
            obl(full, guard, gdep, range_obligation(a.e, t), a.dep,
                meta("conversion", e.loc,
                     "the value may not be one of the values of `" + spell(e) + "` (" + r.lo + " to " + r.hi + ")"));
            return a;
        }
        if (t.is_signed)
        {
            obl(full, guard, gdep, range_obligation(a.e, t), a.dep,
                meta("conversion", e.loc,
                     std::string("the value may not fit in `") + spell(e) +
                         "` (a conversion to a signed type must be exact)"));
            return a;
        }
        // To an unsigned type (decision 0022 items 1 and 4): the value must fit; cerastes does not wrap.  An integer constant that
        // lies in the target's range needs no obligation.
        if (e.a->kind == Expr::Kind::Const && dec::in_range(e.a->value, dec::range_of(t)))
        {
            return a;
        }
        obl(full, guard, gdep, range_obligation(a.e, t), a.dep,
            meta("conversion", e.loc,
                 std::string("the value may not fit in `") + spell(e) + "` (a conversion must be exact)"));
        return a;
    }

    // ---- calls (decisions 0012 item 6 and 0021) ----

    // The fixed points an item created from `effect_from` / `obl_from` on depends on, and those fixed points themselves: what a
    // later item must follow when it uses, or is sequenced after, everything created since (the arguments of a call, the left operand
    // of a comma, the condition of a branch).  The sets are closed downward, so the union is.
    static Deps since(const Full& full, size_t effect_from, size_t obl_from)
    {
        Deps r;
        for (size_t k {effect_from}; k < full.effects.size(); ++k)
        {
            r = join(r, full.effects[k].deps);
            if (full.effects[k].fixed >= 0)
            {
                r.add(full.effects[k].fixed);
            }
        }
        for (size_t k {obl_from}; k < full.obls.size(); ++k)
        {
            r = join(r, full.obls[k].dep);
        }
        return r;
    }

    // Silicon's own decrease check (silver's DecreasesCheck.scala, `createLexDecreaseCheck`, both tuples extended with a top element) of a
    // call, from N (the callee's measure over the arguments) to M (the caller's measure over its parameters' entry values):
    //   OR_{i < min(|N|,|M|)} (AND_{l < i} N_l == M_l && N_i < M_i && M_i >= 0)   OR   [|N| > |M|] AND_{l < |M|} N_l == M_l
    // (decision 0021 item 4; for equal lengths it is 0013 item 2).  Written in the De Morgan form !(!alt1 && !alt2 ...), one atomic
    // assertion: Silicon reports each conjunct of an asserted `&&` as its own error (S19 reran its equivalence matrix with this form).
    static E lex_decreases(const std::vector<E>& n, const std::vector<E>& m)
    {
        using low::BinOp;
        const size_t k {std::min(n.size(), m.size())};
        E none; // the conjunction of the negated alternatives
        bool any {false};
        auto add = [&](E alt)
        {
            none = low::and_(none, low::not_(std::move(alt)));
            any = true;
        };
        for (size_t i {0U}; i < k; ++i)
        {
            E alt;
            for (size_t l {0U}; l < i; ++l)
            {
                alt = low::and_(alt, low::bin(BinOp::Eq, n[l], m[l]));
            }
            alt = low::and_(alt, low::bin(BinOp::Lt, n[i], m[i]));
            alt = low::and_(alt, low::bin(BinOp::Ge, m[i], low::int_lit("0")));
            add(std::move(alt));
        }
        if (n.size() > m.size())
        {
            E alt;
            for (size_t l {0U}; l < k; ++l)
            {
                alt = low::and_(alt, low::bin(BinOp::Eq, n[l], m[l]));
            }
            add(alt == nullptr ? low::bool_lit(true) : alt);
        }
        return any ? low::not_(none) : low::bool_lit(false);
    }

    // The explicit checks of the permissions a callee's atoms need at a call (decision 0037 item 8): each part of each atom under its guard,
    // the separation of atoms that must not share a cell, the case splits, the amounts' bounds and the sums over the cells they are taken from.
    // They are obligations of the call with the exact set PRE(c), in the order they are created.  Returns the ghost amounts, one per `requires`
    // atom, `(g ? m : write) / (N + 1)` (decision 0037 item 6).
    std::vector<E> call_atoms(const Expr& e, const Function& callee, const std::vector<LV>& args, const Env& cenv,
                              const Deps& pre, Full& full)
    {
        using low::BinOp;
        std::vector<E> amounts;
        const FnAtoms& fa {atoms_of(callee)};
        const size_t n {static_cast<size_t>(fa.nreq)};
        if (n == 0U)
        {
            return amounts;
        }
        Deps adep;
        for (const LV& a : args)
        {
            adep = join(adep, a.dep);
        }
        struct S
        {
            const AtomSite* site {nullptr};
            E p, g;
            E root;        // the root pointer (the place's own address when its path is empty)
            Type root_ptr; // its type
            const Type* t {nullptr};
            std::string place;
            std::vector<Cell> cells; // the leaf cells of a struct place, or the one cell (decision 0043 item 9)
        };
        std::vector<S> ss;
        // The callee's places reached through loads are read in the caller's current heap (decision 0045 item 6): the contract mode of a call, over the arguments.
        CtxSave call_ctx(*this);
        cmode_ = CMode::Call;
        cf_ = &callee;
        cens_ = false;
        cold_ = false;
        for (size_t i {0U}; i < n; ++i)
        { // the requires atoms come first in `sites`
            S x;
            x.site = &fa.sites[i];
            x.root = site_root(*x.site, cenv);
            x.p = chain_ref(x.root, x.site->path);
            x.root_ptr = member_root(x.site->atom->a)->type;
            x.g = site_guard(*x.site, cenv);
            x.t = &site_pointee(*x.site);
            x.place = site_call_text(*x.site, e);
            x.cells = site_cells(*x.site, x.p);
            ss.push_back(std::move(x));
        }
        auto text_of = [&](const S& x)
        {
            const std::string pl {site_phrase(*x.site, quoted(x.place, "the object the pointer points to"))};
            switch (x.site->atom->atom)
            {
                case AtomKind::Owns:
                    return "calls `" + callee.c_name + "`, which needs to own " + pl + ", without owning it";
                case AtomKind::Reads:
                    return "calls `" + callee.c_name + "`, which needs to read " + pl +
                           ", without the permission to read it";
                case AtomKind::Live:
                {
                    if (!x.site->via.empty())
                    {
                        return "calls `" + callee.c_name + "`, which needs `" + x.site->atom->a->c_text +
                               "` to point to a live object"; // decision 0045
                    }
                    const Expr& arg {site_arg(*x.site, e)};
                    const std::string who {arg.kind == Expr::Kind::Null ? std::string("the null pointer")
                                           : arg.c_text.empty()         ? std::string("the pointer")
                                                                        : "`" + arg.c_text + "`"};
                    return "calls `" + callee.c_name + "`, which needs " + who + " to point to a live object";
                }
                case AtomKind::OwnsBlock:
                    break; // never a site of contract_atoms (decision 0047 item 4: contract_block_atoms lists it)
            }
            return std::string();
        };
        auto add_as = [&](const char* category, const E& guard, E cond, const std::string& message)
        {
            obl(full, guard, adep, std::move(cond), {}, meta(category, e.loc, message), &pre.ex);
        };
        auto add = [&](const E& guard, E cond, const std::string& message)
        {
            add_as("permission", guard, std::move(cond), message);
        };
        // The spelling of one cell of a struct place for the argument the parameter receives: `s.y` for `&s`, `p->y` for `p` (decision 0043 item 14).
        auto cell_place = [&](const S& x, const Cell& c)
        {
            if (c.rel.empty() || x.place.empty())
            {
                return x.place;
            }
            return x.place[0U] == '*' ? x.place.substr(1U) + "->" + c.rel : x.place + "." + c.rel;
        };
        // The row that the cell the callee takes has a value (decision 0042 item 6, owner questions 82 and 85): category `uninitialized`, worded for the
        // argument the parameter's place receives, or for the global the callee needs.
        auto uninit_text = [&](const S& x, const Cell& c)
        {
            if (site_is_global(*x.site))
            {
                return "calls `" + callee.c_name + "`, which needs " +
                       site_phrase(*x.site, quoted(x.place, "a global")) + " to have a value";
            }
            if (!x.site->via.empty()) // decision 0045: a place reached through steps, in the callee's spelling
            {
                return "calls `" + callee.c_name + "`, which needs " + quoted(x.place, "the object it points to") +
                       " to have a value; it may not have one here";
            }
            const Expr& arg {site_arg(*x.site, e)};
            const std::string who {arg.kind == Expr::Kind::Null ? std::string("the null pointer")
                                   : arg.c_text.empty()         ? std::string("a pointer")
                                                                : "`" + arg.c_text + "`"};
            const std::string& pname {callee.params[static_cast<size_t>(site_key(*x.site))].name};
            return "passes " + who + " to `" + callee.c_name + "`, whose parameter " +
                   (pname.empty() ? std::string() : "`" + pname + "` ") + "needs " +
                   quoted(x.cells.size() > 1U || !c.rel.empty() ? cell_place(x, c) : site_text(*x.site),
                          "the object it points to") +
                   " to have a value; it may not have one here";
        };
        auto split = [&](E cond)
        {
            obl(full, nullptr, adep, low::bool_lit(true), {}, meta("internal", e.loc, "internal check: a case split"),
                &pre.ex);
            low::LStmt st;
            st.kind = low::LStmt::Kind::CaseSplit;
            st.expr = std::move(cond);
            full.obls.back().stmts.push_back(std::move(st));
        };
        const E none {low::no_perm()};
        const E write {low::write_perm()};
        // 1. each part of each atom, under its guard: the root's facts, then each leaf cell, then the liveness cell
        for (const S& x : ss)
        {
            const std::string m {text_of(x)};
            add(x.g, low::bin(BinOp::Ne, x.root, low::null_()), m);
            add(x.g, wf_expr(x.root, &x.root_ptr), m);
            if (needs_root_block(*x.site))
            {
                add(x.g, in_block(x.root), m);
            }
            if (site_takes_cell(*x.site))
            {
                const bool owns {x.site->atom->atom == AtomKind::Owns};
                for (const Cell& c : x.cells)
                {
                    add(x.g, low::bin(owns ? BinOp::Eq : BinOp::Gt, value_perm(c.recv, c.type), owns ? write : none),
                        m);
                    add(x.g, low::bin(owns ? BinOp::Eq : BinOp::Gt, flag_perm(c.recv, c.type), owns ? write : none), m);
                    // A marked parameter's atom has no flag row, and its range holds under the flag (decision 0042 item 5).
                    if (site_marker(callee, *x.site) == InitMarker::None)
                    {
                        add_as("uninitialized", x.g, cell_flag(c.recv, c.type), uninit_text(x, c));
                        if (const E r = cell_range(c.recv, c.type))
                        {
                            add(x.g, r, m);
                        }
                    }
                    else if (const E r = cell_range(c.recv, c.type))
                    {
                        add(x.g, low::implies(cell_flag(c.recv, c.type), r), m);
                    }
                }
            }
            add(x.g, low::bin(BinOp::Gt, live_perm(x.root), none), m);
        }
        // 2. the separation of atoms that must not name one cell: `swap(&x, &x)` fails here; per pair of cells of one field (decision 0043 item 9)
        for (size_t i {0U}; i < ss.size(); ++i)
        {
            for (size_t j {i + 1U}; j < ss.size(); ++j)
            {
                if (!exclusive_pair(*ss[i].site, *ss[j].site))
                {
                    continue;
                }
                const std::string pl {quoted(ss[i].place, "the object the pointer points to")};
                if (site_key(*ss[i].site) == site_key(*ss[j].site))
                { // one parameter under two atoms: their guards must exclude each other (decision 0039 item 7)
                    // two member places of one root are two cells, unless one holds the other (the checker refuses
                    // that: audit B1)
                    if (!same_place(*ss[i].site, *ss[j].site))
                    {
                        if (paths_nested(ss[i].site->path, ss[j].site->path))
                        {
                            internal_error(e.loc,
                                           "two permission atoms of one root, one inside the other, reached lowering");
                        }
                        continue;
                    }
                    add(low::and_(ss[i].g, ss[j].g), low::bin(BinOp::Ne, ss[i].p, ss[j].p),
                        "passes " + pl + " to `" + callee.c_name +
                            "`, whose contract both owns and reads it on this path");
                }
                else
                {
                    for (const auto& [x, y] : overlap_pairs(*ss[i].site, ss[i].p, *ss[j].site, ss[j].p))
                    {
                        add(low::and_(ss[i].g, ss[j].g), low::bin(BinOp::Ne, x, y),
                            "passes " + pl + " to `" + callee.c_name +
                                "` twice, but one of the two places is to be owned and they may be the same object");
                    }
                }
            }
        }
        // 3. the case splits: on the liveness cells of every pair of atoms, on the cells of every pair of `reads`
        for (size_t i {0U}; i < ss.size(); ++i)
        {
            for (size_t j {i + 1U}; j < ss.size(); ++j)
            {
                if (!same_expr(ss[i].root, ss[j].root) && !(site_is_global(*ss[i].site) && site_is_global(*ss[j].site)))
                {
                    split(low::bin(BinOp::Eq, live_cell(ss[i].root), live_cell(ss[j].root)));
                }
            }
        }
        for (size_t i {0U}; i < ss.size(); ++i)
        {
            for (size_t j {i + 1U}; j < ss.size(); ++j)
            {
                const AtomSite& a {*ss[i].site};
                const AtomSite& b {*ss[j].site};
                if (a.atom->atom == AtomKind::Reads && b.atom->atom == AtomKind::Reads && shares_field(a, b) &&
                    !same_expr(ss[i].root, ss[j].root) && !(site_is_global(a) && site_is_global(b)))
                {
                    for (const auto& [x, y] : overlap_pairs(a, ss[i].p, b, ss[j].p))
                    {
                        split(low::bin(BinOp::Eq, x, y));
                    }
                }
            }
        }
        // 3b. each pointer argument that feeds a cell atom, against each `reads` parameter of the caller of the same field: the caller may hold the
        // argument's cell as that parameter's, at a fraction, and Silicon takes a fraction computed from two chunks only when it knows which (S26).
        // Only the paths that reach a call branch (decision 0037 item 9, the re-audit's blow-up of entry splits).
        {
            const FnAtoms& caller {atoms_of(*fn_.f)};
            std::vector<std::pair<E, E>> done;
            for (const S& x : ss)
            {
                if (!site_takes_cell(*x.site))
                {
                    continue;
                }
                for (int i {0}; i < caller.nreq; ++i)
                {
                    const AtomSite& cs {caller.sites[static_cast<size_t>(i)]};
                    if (cs.atom->atom != AtomKind::Reads || !shares_field(*x.site, cs))
                    {
                        continue;
                    }
                    const E q {site_var(cs)};
                    for (const auto& [px, pq] : overlap_pairs(*x.site, x.p, cs, q))
                    {
                        if (same_expr(px, pq))
                        {
                            continue;
                        }
                        bool seen {false};
                        for (const auto& d : done)
                        {
                            if (same_expr(d.first, px) && same_expr(d.second, pq))
                            {
                                seen = true;
                            }
                        }
                        if (seen)
                        {
                            continue;
                        }
                        done.emplace_back(px, pq);
                        split(low::bin(BinOp::Eq, px, pq));
                    }
                }
                // the same against the `reads` places of the enclosing loops' written atoms (decision 0040 item 5, audit N4)
                for (const WAtom* a : active_w_)
                {
                    for (const Cell& c : x.cells)
                    {
                        if (a->kind != AtomKind::Reads || a->sfx != cell_suffix(c.type) || same_expr(c.recv, a->qb))
                        {
                            continue;
                        }
                        bool seen {false};
                        for (const auto& d : done)
                        {
                            if (same_expr(d.first, c.recv) && same_expr(d.second, a->qb))
                            {
                                seen = true;
                            }
                        }
                        if (seen)
                        {
                            continue;
                        }
                        done.emplace_back(c.recv, a->qb);
                        split(low::bin(BinOp::Eq, c.recv, a->qb));
                    }
                }
            }
        }
        // 4. the amounts: a `reads` atom's is a fraction of the least the caller holds of every value and flag cell the atom takes, and of the liveness cell
        for (const S& x : ss)
        {
            E m {live_perm(x.root)};
            if (x.site->atom->atom == AtomKind::Reads)
            {
                E least;
                for (const Cell& c : x.cells)
                {
                    const E v {value_perm(c.recv, c.type)};
                    const E d {flag_perm(c.recv, c.type)};
                    least = least ? min_perm(min_perm(least, v), d) : min_perm(v, d);
                }
                if (least)
                {
                    m = min_perm(least, m);
                }
            }
            amounts.push_back(low::perm_div(x.g ? low::ite(x.g, m, write) : m, static_cast<int>(n) + 1));
        }
        for (size_t i {0U}; i < ss.size(); ++i)
        {
            const std::string m {text_of(ss[i])};
            add(nullptr, low::bin(BinOp::Lt, none, amounts[i]), m);
            add(nullptr, low::bin(BinOp::Lt, amounts[i], write), m);
        }
        // 5. the sums over each cell an atom takes a fraction of (decision 0037 item 8, audit round 1 S3), per leaf cell
        auto term = [&](const E& cond, const E& amount)
        {
            return cond ? low::ite(cond, amount, none) : amount;
        };
        for (size_t i {0U}; i < ss.size(); ++i)
        {
            const std::string m {text_of(ss[i])};
            if (ss[i].site->atom->atom == AtomKind::Reads)
            {
                for (const Cell& ci : ss[i].cells)
                {
                    for (const bool flag : {false, true})
                    {
                        E total {amounts[i]};
                        for (size_t j {0U}; j < ss.size(); ++j)
                        {
                            if (j == i || ss[j].site->atom->atom != AtomKind::Reads)
                            {
                                continue;
                            }
                            const bool one_type {site_pointee(*ss[i].site) == site_pointee(*ss[j].site) &&
                                                 site_pointee(*ss[i].site).is_struct()};
                            for (const Cell& cj : ss[j].cells)
                            {
                                if (cell_suffix(cj.type) != cell_suffix(ci.type))
                                {
                                    continue;
                                }
                                if (one_type && cj.path != ci.path)
                                {
                                    continue; // leaves of two objects of one struct type share a cell only at one path
                                }
                                E same {same_expr(ci.recv, cj.recv) ? nullptr : low::bin(BinOp::Eq, ci.recv, cj.recv)};
                                total = low::bin(BinOp::Add, total, term(low::and_(ss[j].g, same), amounts[j]));
                            }
                        }
                        add(ss[i].g,
                            low::bin(BinOp::Ge, flag ? flag_perm(ci.recv, ci.type) : value_perm(ci.recv, ci.type),
                                     total),
                            m);
                    }
                }
            }
            E total {amounts[i]};
            for (size_t j {0U}; j < ss.size(); ++j)
            {
                if (j == i)
                {
                    continue;
                }
                E same {same_expr(ss[i].root, ss[j].root)
                            ? nullptr
                            : low::bin(BinOp::Eq, live_cell(ss[i].root), live_cell(ss[j].root))};
                total = low::bin(BinOp::Add, total, term(low::and_(ss[j].g, same), amounts[j]));
            }
            add(ss[i].g, low::bin(BinOp::Ge, live_perm(ss[i].root), total), m);
        }
        return amounts;
    }

    LV call(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        const Function& callee {*e.callee};
        if (guard != nullptr)
        {
            internal_error(e.loc, "a call in a conditionally evaluated operand reached lowering");
        }
        (void)gdep;
        const size_t effect_from {full.effects.size()};
        const size_t obl_from {full.obls.size()};
        std::vector<LV> args;
        // A read in the arguments of a memory call on its field is sequenced before the call: it is no snapshot's reason (decision 0039 item 2).
        const std::set<std::string> mfields {track_mem() ? memory_fields(callee) : std::set<std::string> {}};
        for (const std::string& f : mfields)
        {
            ++mem_depth_[f];
        }
        for (const Expr* a : e.args)
        {
            args.push_back(lower(*a, env, nullptr, {}, full));
        }
        for (const std::string& f : mfields)
        {
            --mem_depth_[f];
        }
        // PRE(c): what C completes before it makes the call: the comma floor, and everything its arguments use or contain (the items
        // created while they were lowered, the fixed points among them, and what the argument values depend on).
        // Decision 0026 item 1: it is the EXACT set of each `requires` clause's obligation (below).  The BASE set of a clause, which step 2
        // of `flush` falls back to, is the one of 0021 item 10: only what the clause's argument values depend on, and the floor.
        Deps pre {join(floor_, since(full, effect_from, obl_from))};
        for (const LV& a : args)
        {
            pre = join(pre, a.dep);
        }
        // The callee's `requires`, over the argument values, are obligations at the call (0012 item 6).
        Env cenv;
        for (const LV& a : args)
        {
            cenv.formals.push_back(a.leaves.empty() ? a.e : nullptr);
            cenv.formal_dep.push_back(a.dep);
            cenv.formal_leaves.push_back(a.leaves); // a by-value struct argument: its leaves (decision 0044 item 5)
        }
        // The permissions the callee's atoms need (decision 0037 item 8): explicit, atomic, in order, before the value preconditions, all with the
        // exact set PRE(c) like them.
        bool args_write_cell {false};
        for (const Expr* a : e.args)
        {
            args_write_cell = args_write_cell || writes_cell(a);
        }
        const bool saved_both {call_pre_both_};
        // audit S3: the arguments write a cell, so the checks of the call follow the write in both halves
        call_pre_both_ = args_write_cell;
        std::vector<E> amounts {call_atoms(e, callee, args, cenv, pre, full)};
        // decision 0047 item 6: the free rows, or the rows of the `owns_block` sites the callee takes
        block_call_rows(e, callee, cenv, args, pre, full);
        for (size_t k {0U}; k < e.args.size(); ++k)
        {
            // A pointer argument is a use (decision 0032 item 6), except the pointer a `free` takes: its token and liveness rows check more, so a second free is `invalid-free`
            // (decision 0047 item 6); any other use inside the argument (`free(q = p)`, a call) keeps its own checks.
            if (e.args[k]->type.is_ptr() && !(callee.deallocates && !has_effect(*e.args[k])))
            {
                ptr_use(*e.args[k], args[k], nullptr, {}, full);
            }
            // A struct argument built by an aggregate had each pointer leaf used when it was lowered; one a call returned has not (decision 0044 item 5).
            if (e.args[k]->type.is_struct() && !is_aggregate_value(*e.args[k]))
            {
                struct_ptr_uses(*e.args[k], args[k], nullptr, {}, full, false);
            }
        }
        for (size_t ci {0U}; ci < callee.requires_.size(); ++ci)
        {
            const Clause& c {callee.requires_[ci]};
            Full scratch; // the clause's own obligations belong to the callee's well-formedness method
            LV v;
            {
                CtxSave save(*this);
                set_ctx(callee, CMode::Call, false, static_cast<int>(ci));
                v = lower(*c.expr, cenv, nullptr, {}, scratch);
            }
            pure_full(scratch, *c.expr, "a precondition");
            if (has_atom(*c.expr) && always_true(v.e))
            {
                continue; // nothing but atoms: asserted above
            }
            low::Meta m {meta("precondition", e.loc, "the precondition of `" + callee.c_name + "` may not hold")};
            m.has_note = true;
            m.note_loc = c.loc.known() ? c.loc : callee.loc;
            m.note_text = "this is the requirement of `" + callee.c_name + "`";
            // EXACT (decision 0026 item 1): C completes everything in PRE(c) before it makes the call, so the clause is asserted after all of it.
            // BASE (step 2): only the argument values the clause reads, and the floor (0021 item 10).
            obl(full, nullptr, {}, v.e, v.dep, std::move(m), &pre.ex);
            if (opt_.drop_exact_set && !exact_dropped_ && !full.obls.back().dep.base.empty())
            { // self-test hook (decision 0026)
                full.obls.back().dep.ex.clear();
                exact_dropped_ = true;
            }
        }
        call_pre_both_ = saved_both;
        CallItem item;
        item.method = method_name(callee);
        for (const LV& a : args)
        {
            if (!a.leaves.empty())
            { // a struct argument passes its leaves (decision 0044 item 5)
                for (const E& l : a.leaves)
                {
                    item.args.push_back(l);
                }
            }
            else
            {
                item.args.push_back(a.e);
            }
        }
        for (const E& a : amounts)
        {
            item.args.push_back(a); // the ghost amounts, after the user's arguments
        }
        low::Meta backstop {meta("internal", e.loc,
                                 "internal check: Silicon's own check of the precondition of `" + callee.c_name + "`")};
        item.check = low::Check {"call.precondition", std::move(backstop)};
        const int id {full.nfixed++};
        if (callee.body != nullptr && !callee.decreases_.empty())
        {
            // Silicon checks, at a call from a function with a measure, that the callee's measure over the arguments
            // decreases when the callee is in the same recursion cycle (decision 0013 items 2 and 8).
            low::Meta tm {
                meta("termination", e.loc,
                     "the recursion through this call to `" + callee.c_name +
                         "` may not terminate: its measure may not decrease, or may not be bounded below by zero")};
            tm.has_note = true;
            tm.note_loc = callee.decreases_.front().loc.known() ? callee.decreases_.front().loc : callee.loc;
            tm.note_text = "this is the measure of `" + callee.c_name + "`";
            item.has_termination = true;
            item.termination = low::Check {"termination.failed", tm};
            if (in_cycle(fn_.f, &callee))
            {
                explicit_termination(full, id, pre, callee, cenv, std::move(tm));
            }
        }
        else if (callee.body != nullptr && in_cycle(fn_.f, &callee))
        {
            // Silicon would check the call against an empty tuple; check_program's `missing_decreases` rule makes this unreachable, and a
            // call that got neither its PRE in F nor an explicit check would be unsound, so fail closed.
            internal_error(e.loc, "a call in a recursion cycle to `" + callee.c_name +
                                      "`, which has no termination measure, reached lowering");
        }
        E value;
        std::vector<E> value_leaves;
        if (callee.ret.is_struct())
        {
            // decision 0044 item 4: one target per leaf, and 0037 item 3's case splits after each pointer leaf
            for (const StructLeaf& l : struct_leaves(*callee.ret.struct_type))
            {
                const std::string t {new_temp(ltype(l.type))};
                item.targets.push_back(t);
                value_leaves.push_back(low::var(t));
                if (l.type.is_ptr())
                {
                    result_splits(l.type, value_leaves.back(), item.after);
                }
            }
        }
        else if (!callee.ret.is_void())
        {
            item.target = "t__" + std::to_string(fn_.ntemp++);
            fn_.temps.push_back(low::LVar {item.target, ltype(callee.ret)});
            value = low::var(item.target);
            // A result that is a fresh block (an `ensures` `owns_block` on it: a view, an owning function) needs no split: Silicon derives from the chunks it inhales that it
            // differs from every cell the caller holds (S36 (k)); the splits would multiply the paths of every allocation.
            bool block_result {false};
            for (const AtomSite& bs : blocks_of(callee).sites)
            {
                if (bs.in_ensures && block_key(bs) < 0)
                {
                    block_result = true;
                }
            }
            if (callee.ret.is_ptr() && !block_result)
            {
                // The pointer arguments of the callee's result type: a result the callee's contract ties to its arguments (`r == a || r == b`) may be any of them.
                std::vector<E> same_type;
                for (size_t i {0U}; i < args.size() && i < callee.params.size(); ++i)
                {
                    const Type& pt {callee.params[i].type};
                    if (!pt.is_ptr() || pt.pointee == nullptr || callee.ret.pointee == nullptr ||
                        !(*pt.pointee == *callee.ret.pointee) || !args[i].leaves.empty() || args[i].e == nullptr)
                    {
                        continue;
                    }
                    bool dup {false};
                    for (const E& x : same_type)
                    {
                        dup = dup || same_expr(x, args[i].e);
                    }
                    if (!dup)
                    {
                        same_type.push_back(args[i].e);
                    }
                }
                result_splits(callee.ret, value, item.after, &same_type);
            }
        }
        Effect fx;
        fx.kind = Effect::Kind::Call;
        fx.call = std::move(item);
        fx.fixed = id;
        fx.deps = pre;
        fx.seq = full.next_seq++;
        // a call with a consuming `owns_block` site (free's view included) ends the lifetime of a block (decision 0047
        // item 9)
        bool ends_lifetime {false};
        if (heap_)
        {
            for (const AtomSite& bs : blocks_of(callee).sites)
            {
                if (block_consumes(callee, bs))
                {
                    ends_lifetime = true;
                }
            }
        }
        if (track_mem() && (!mfields.empty() || ends_lifetime))
        {
            // A memory call is an access to the cells of its atoms (decision 0039 item 4): an `owns` atom may write, a `reads` atom reads.
            Acc ca;
            ca.call = true;
            ca.callee = callee.c_name;
            for (const LV& arg : args)
            {
                ca.dep = join(ca.dep, arg.dep); // the places the atoms name, not everything PRE(c) holds
            }
            ca.loc = e.loc;
            const FnAtoms& fa {atoms_of(callee)};
            for (int i {0}; i < fa.nreq; ++i)
            {
                const AtomSite& site {fa.sites[static_cast<size_t>(i)]};
                if (!site_takes_cell(site))
                {
                    continue;
                }
                // The callee's places are read in the contract mode of a call, over the arguments (decision 0045 item 6).
                E guard_of_site;
                std::vector<Cell> site_cs;
                {
                    CtxSave call_ctx(*this);
                    cmode_ = CMode::Call;
                    cf_ = &callee;
                    cens_ = false;
                    cold_ = false;
                    guard_of_site = site_guard(site, cenv);
                    site_cs = site_cells(site, site_ref(site, cenv));
                }
                const std::string base_text {site_call_text(site, e)};
                // One access per cell of the atom (decision 0043 items 9 and 12): a struct atom is a memory call on every leaf field, with an obligation per leaf.
                for (const Cell& c : site_cs)
                {
                    Acc::Atom k;
                    k.loads = site_loads(site);
                    E place_now {c.recv};
                    if (k.loads)
                    {
                        // Decision 0045 item 6 (owner question 111): the callee's place is read in the caller's heap after the arguments are evaluated and the call rows have
                        // checked its cells (a snapshot of the cell's Ref, chain-guarded like the rows), so the pair checks of 0039 item 4 compare the objects the call is about
                        // to take, not what the heap names after the call.
                        const std::string snap {new_temp(low::LType::Ref)};
                        Effect sx;
                        sx.kind = Effect::Kind::Snapshot;
                        sx.name = snap;
                        sx.value = c.recv;
                        sx.deps = pre;
                        sx.seq = full.next_seq++;
                        full.effects.push_back(std::move(sx));
                        place_now = low::var(snap);
                        ca.dep = join(ca.dep, pre); // the pair checks that name it follow the snapshot
                    }
                    k.place = place_now;
                    k.guard = guard_of_site;
                    k.owns = site.atom->atom == AtomKind::Owns;
                    k.field = fkey(c.type);
                    k.text =
                        c.rel.empty() || base_text.empty()
                            ? base_text
                            : (base_text[0U] == '*' ? base_text.substr(1U) + "->" + c.rel : base_text + "." + c.rel);
                    fx.tags.push_back(MemTag {0, k.field, true, k.owns, path_, {}});
                    ca.atoms.push_back(std::move(k));
                }
            }
            // decision 0047 item 9: a `requires` block site is full permission to every leaf cell of its block (the pair checks of 0039 item 4 compare the cells), and a
            // consuming one also writes its liveness cell (a use of any pointer beside it reads it)
            {
                const FnBlocks& fb {blocks_of(callee)};
                for (int i {0}; i < fb.nreq; ++i)
                {
                    const AtomSite& site {fb.sites[static_cast<size_t>(i)]};
                    E guard_of_site;
                    E ptr;
                    {
                        CtxSave call_ctx(*this);
                        cmode_ = CMode::Call;
                        cf_ = &callee;
                        cens_ = false;
                        cold_ = false;
                        guard_of_site = site_guard(site, cenv);
                        ptr = block_ptr(site, cenv);
                    }
                    const std::string base_text {e.args[static_cast<size_t>(block_key(site))]->c_text};
                    for (const Cell& c : block_cells(site, ptr))
                    {
                        Acc::Atom k;
                        k.place = c.recv;
                        k.guard = guard_of_site;
                        k.owns = true;
                        k.field = fkey(c.type);
                        k.text = c.rel.empty() || base_text.empty()
                                     ? (base_text.empty() ? std::string() : "*" + base_text)
                                     : base_text + "->" + c.rel;
                        fx.tags.push_back(MemTag {0, k.field, true, true, path_, {}});
                        ca.atoms.push_back(std::move(k));
                    }
                    if (block_consumes(callee, site))
                    {
                        Acc::Atom k;
                        k.place = live_cell(ptr);
                        k.guard = guard_of_site;
                        k.owns = true;
                        k.field = "lv";
                        k.text = base_text;
                        ca.atoms.push_back(std::move(k));
                    }
                }
            }
            ca.field = ca.atoms.empty() ? std::string() : ca.atoms.front().field;
            register_access(std::move(ca), full, nullptr);
        }
        full.pre.push_back(pre);
        full.fixed_effect.push_back(full.effects.size());
        full.info.push_back(FixedInfo {report_at_ != nullptr ? *report_at_ : e.loc, callee.c_name,
                                       "the call to \"" + callee.c_name + "\""});
        full.effects.push_back(std::move(fx));
        Deps d {pre};
        d.add(id);
        if (callee.ret.is_struct())
        {
            return struct_value(std::move(value_leaves), d);
        }
        return {value, d};
    }

    // Whether a struct-valued expression is (a chain of members over) an aggregate: its pointer leaves were used where it was lowered.
    static bool is_aggregate_value(const Expr& e)
    {
        const Expr* x {&e};
        while (x->kind == Expr::Kind::Field && x->a != nullptr)
        {
            x = x->a;
        }
        return x->kind == Expr::Kind::Aggregate;
    }

    // Each pointer leaf of the struct value `v` (a call's result or a member chain over one) is a use of the pointer, as a pointer argument or a copy is
    // (decision 0032 item 6): `p != null ==> perm(lvloc(block_of(p)).lv) > none`, with WF for a store into a pointer cell (decision 0040 item 2).
    void struct_ptr_uses(const Expr& operand, const LV& v, const E& guard, const Deps& gdep, Full& full, bool with_wf)
    {
        if (cmode_ != CMode::None || !has_pointer_leaf(operand.type))
        {
            return;
        }
        const std::vector<StructLeaf> ls {operand.type.is_struct() ? struct_leaves(*operand.type.struct_type)
                                                                   : std::vector<StructLeaf> {}};
        for (size_t i {0U}; i < ls.size() && i < v.leaves.size(); ++i)
        {
            if (!ls[i].type.is_ptr())
            {
                continue;
            }
            E live {low::bin(low::BinOp::Gt, live_perm(v.leaves[i]), low::no_perm())};
            if (const E g = global_live_disjunct(v.leaves[i]))
            {
                live = low::bin(low::BinOp::Or, live, g);
            }
            if (with_wf)
            {
                // one atomic assertion
                live =
                    low::not_(low::bin(low::BinOp::Or, low::not_(live), low::not_(wf_expr(v.leaves[i], &ls[i].type))));
            }
            const std::string text {operand.c_text.empty() ? std::string() : operand.c_text + "." + ls[i].text};
            obl(full, guard, gdep, low::implies(low::bin(low::BinOp::Ne, v.leaves[i], low::null_()), live), v.dep,
                meta("lifetime", operand.loc,
                     "uses " + quoted(text, "the pointer") + " after the object it points to may have ended"));
        }
    }

    // Decision 0021 item 4: the explicit check that a recursive call's measure decreases, an obligation with the set PRE(c).  It is
    // printed by flush only when another fixed point, outside PRE(c), precedes the call; otherwise Silicon's own check at the call (which
    // stays) is the one that reports.
    void explicit_termination(Full& full, int id, const Deps& pre, const Function& callee, const Env& cenv,
                              low::Meta tm)
    {
        std::vector<E> n;
        std::vector<E> m;
        for (const Clause& c : callee.decreases_)
        {
            Full scratch; // its own obligations belong to the callee's well-formedness method
            LV v;
            {
                CtxSave save(*this);
                // The callee's measure over the arguments in the current heap, chain-guarded over the callee's atoms at the arguments (decision 0039 item 8).
                if (reads_heap(*c.expr))
                {
                    set_ctx(callee, CMode::Method, false, static_cast<int>(callee.requires_.size()));
                }
                v = lower(*c.expr, cenv, nullptr, {}, scratch);
            }
            pure_full(scratch, *c.expr, "a termination measure");
            n.push_back(v.e);
        }
        const Env own {contract_env()};
        for (const Clause& c : fn_.f->decreases_)
        {
            Full scratch;
            LV v;
            {
                CtxSave save(*this);
                // The caller's measure under `old(...)`: its entry heap, framed by the caller's `requires`.
                if (reads_heap(*c.expr))
                {
                    set_ctx(*fn_.f, CMode::Method, false, static_cast<int>(fn_.f->requires_.size()));
                    cold_ = true;
                }
                v = lower(*c.expr, own, nullptr, {}, scratch);
            }
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
    static bool has_effect(const Expr& e)
    {
        if (e.kind == Expr::Kind::AssignE || e.kind == Expr::Kind::StoreE || e.kind == Expr::Kind::Comma ||
            e.kind == Expr::Kind::Call)
        {
            return true;
        }
        if (e.kind == Expr::Kind::Cond && e.type.is_ptr())
        {
            return true;
        }
        for (const Expr* x : {e.a, e.b, e.c})
        {
            if (x != nullptr && has_effect(*x))
            {
                return true;
            }
        }
        for (const Expr* x : e.args)
        {
            if (x != nullptr && has_effect(*x))
            {
                return true;
            }
        }
        return false;
    }

    // The locals a full expression writes (its AssignE targets, those in guarded operands included).
    static void collect_writes(const Expr& e, std::set<int>& out)
    {
        if (e.kind == Expr::Kind::AssignE && e.var != nullptr && !e.var->address_taken)
        {
            out.insert(e.var->id); // a cell's write is a memory effect (0039)
        }
        for (const Expr* x : {e.a, e.b, e.c})
        {
            if (x != nullptr)
            {
                collect_writes(*x, out);
            }
        }
        for (const Expr* x : e.args)
        {
            if (x != nullptr)
            {
                collect_writes(*x, out);
            }
        }
    }

    std::string new_temp(low::LType type)
    {
        const std::string t {"t__" + std::to_string(fn_.ntemp++)};
        fn_.temps.push_back(low::LVar {t, type});
        return t;
    }

    // E3: a read of a local that the full expression writes is a snapshot, taken where C reads it; its `uninitialized` obligation
    // (decision 0019) is the read of the flag, which is part of the read of the local: it has the set of the read (the snapshot's own
    // set, its guard's and the comma floor's), and that set, not just the snapshot's, is what a later write of the local must follow
    // (decision 0025 item 1: a comma drops its left operand's calls, so a later write could otherwise be emitted before the check).
    LV snapshot(const Expr& e, const E& guard, const Deps& gdep, Full& full)
    {
        const std::string t {new_temp(ltype(e.var->type))};
        const int id {e.var->id};
        // A read after a write of the same local follows that write's set; the comma floor too.
        const auto w {full.last_write.find(id)};
        const Deps slot {join(floor_, w == full.last_write.end() ? Deps {} : w->second)};
        Effect fx;
        fx.kind = Effect::Kind::Snapshot;
        fx.name = t;
        fx.value = low::var(var_name(*e.var));
        fx.deps = slot;
        fx.seq = full.next_seq++;
        fx.reads.insert(id);
        full.effects.push_back(std::move(fx));
        if (flagged(*e.var))
        {
            obl(full, guard, gdep, low::var(flag_name(*e.var)), slot,
                meta("uninitialized", e.loc, "`" + e.var->name + "` may be read here before it is given a value"));
            Obl& check {full.obls.back()};
            check.flag_of = id;
            // gdep + slot + floor_: every later access follows the check (hook: only the snapshot's)
            touch(full, id, opt_.drop_flag_access ? slot : check.dep);
        }
        else
        {
            touch(full, id, slot);
        }
        return {low::var(t), slot};
    }

    // `x = a` used as a value: the old value (for x++) is read once, then the stored value is lowered, then the write is an effect.
    // E4: the node's value is the stored value as lowered (or the old value's snapshot), never `v_x`.
    // Record an access (a snapshot or a write) of a local with the set `slot`.
    static void touch(Full& full, int id, const Deps& slot)
    {
        const auto it {full.last_access.find(id)};
        if (it == full.last_access.end())
        {
            full.last_access[id] = slot;
        }
        else
        {
            it->second = join(it->second, slot);
        }
    }

    LV assign_value(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        if (!in_full_ || e.var == nullptr || written_.count(e.var->id) == 0U)
        {
            internal_error(e.loc, "an assignment used as a value outside a full expression reached lowering");
            return {low::int_lit("0"), {}};
        }
        if (guard != nullptr)
        {
            internal_error(e.loc, "an effect in a conditionally evaluated pure operand reached lowering");
            return {low::int_lit("0"), {}};
        }
        LV old_value;
        if (e.b != nullptr)
        {
            old_value = lower(*e.b, env, guard, gdep, full);
            shared_[e.b] = old_value;
        }
        LV stored {lower(*e.a, env, guard, gdep, full)};
        if (e.b != nullptr)
        {
            shared_.erase(e.b);
        }
        if (e.var->type.is_ptr())
        {
            ptr_use(*e.a, stored, guard, gdep, full); // a pointer copy uses the value (decision 0032 item 6)
        }
        // The write follows what its value depends on, the comma floor and every earlier access to the same local.
        const int id {e.var->id};
        const auto a {full.last_access.find(id)};
        const Deps slot {join(join(floor_, stored.dep), a == full.last_access.end() ? Deps {} : a->second)};
        Effect fx;
        fx.kind = Effect::Kind::Write;
        fx.name = var_name(*e.var);
        fx.value = stored.e;
        if (flagged(*e.var))
        {
            fx.flag = flag_name(*e.var);
        }
        fx.deps = slot;
        fx.seq = full.next_seq++;
        fx.writes.insert(id);
        full.effects.push_back(std::move(fx));
        full.last_write[id] = slot;
        touch(full, id, slot);
        if (e.yields_old)
        {
            return old_value;
        }
        return stored;
    }

    // `a, b`: `a` for its effects and obligations, then `b`; whatever follows the left operand is placed after its last effect (E6).
    // The set of the value (decision 0026 item 1): EXACT, its own set and the floor in force while `b` was lowered, which is what C
    // completes before the value exists; BASE (the fallback of step 2), its own set only, as before M4.4d.  Everything inside `b`
    // has the floor in both halves.
    LV comma(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        if (!in_full_)
        {
            internal_error(e.loc, "a comma outside a full expression reached lowering");
            return {low::int_lit("0"), {}};
        }
        const size_t effect_from {full.effects.size()};
        const size_t obl_from {full.obls.size()};
        {
            const LV left {lower(*e.a, env, guard, gdep, full)};
            if (e.a->type.is_ptr())
            {
                ptr_use(*e.a, left, guard, gdep, full); // a discarded pointer value is a use (decision 0032 item 6)
            }
        }
        const Deps saved {floor_};
        // Only what the left operand itself depends on or contains: a call elsewhere in the enclosing expression is unsequenced with the
        // right operand (decision 0021: what the left operand created, fixed points included).
        floor_ = join(floor_, since(full, effect_from, obl_from));
        LV b {lower(*e.b, env, guard, gdep, full)};
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
    std::vector<low::LStmt> branch_body(const Expr& e, const Env& env, const std::string& t, std::set<int>& reads,
                                        std::set<int>& writes, FixedInfo& first_call, bool& has_fixed, const E& bguard,
                                        std::vector<MemTag>& tags)
    {
        const Deps saved {floor_};
        floor_ = {};
        Full inner;
        branch_guards_.push_back(bguard); // the accesses inside happen only on this arm (decision 0039 item 3)
        LV v {lower(e, env, nullptr, {}, inner)};
        branch_guards_.pop_back();
        floor_ = saved;
        if (inner.nfixed != 0 && !has_fixed)
        {
            has_fixed = true;
            first_call = inner.info.front();
        }
        for (const Effect& fx : inner.effects)
        {
            reads.insert(fx.reads.begin(), fx.reads.end());
            writes.insert(fx.writes.begin(), fx.writes.end());
            tags.insert(tags.end(), fx.tags.begin(), fx.tags.end());
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
    Deps add_branch(Full& full, Effect&& fx, const Deps& cond_dep, const std::set<int>& reads,
                    const std::set<int>& writes, bool has_fixed, FixedInfo info, size_t effect_from, size_t obl_from,
                    size_t acc_from)
    {
        Deps slot {join(floor_, cond_dep)};
        for (int id : reads)
        {
            const auto w {full.last_write.find(id)};
            if (w != full.last_write.end())
            {
                slot = join(slot, w->second);
            }
        }
        for (int id : writes)
        {
            const auto a {full.last_access.find(id)};
            if (a != full.last_access.end())
            {
                slot = join(slot, a->second);
            }
        }
        Deps value {slot};
        fx.deps = slot;
        fx.seq = full.next_seq++;
        fx.reads = reads;
        fx.writes = writes;
        if (has_fixed)
        {
            slot = join(slot, since(full, effect_from, obl_from));
            fx.deps = slot;
            const int id {full.nfixed++};
            fx.fixed = id;
            full.pre.push_back(slot);
            full.fixed_effect.push_back(full.effects.size());
            full.info.push_back(std::move(info));
            value = slot;
            value.add(id);
        }
        for (int id : writes)
        {
            full.last_write[id] = value;
        }
        for (int id : reads)
        {
            touch(full, id, value);
        }
        for (int id : writes)
        {
            touch(full, id, value);
        }
        full.effects.push_back(std::move(fx));
        // The cell accesses made inside the branch now belong to this full expression, with the branch's value as what they depend on (audit S5).
        for (size_t k {acc_from}; k < accs_.size(); ++k)
        {
            accs_[k].owner = &full;
            accs_[k].dep = value;
        }
        return value;
    }

    // `a && b` / `a || b` where `b` has effects (E5): `t := a; if (t) { b's effects and obligations; t := b }` (`!t` for ||).
    LV branch_logical(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        if (guard != nullptr)
        {
            internal_error(e.loc, "an effect in a conditionally evaluated operand reached lowering");
            return {low::bool_lit(false), {}};
        }
        if (!in_full_)
        {
            internal_error(e.loc, "an effect in a logical operand outside a full expression reached lowering");
            return {low::bool_lit(false), {}};
        }
        const size_t effect_from {full.effects.size()};
        const size_t obl_from {full.obls.size()};
        LV a {lower(*e.a, env, guard, gdep, full)};
        Deps left_floor; // audit R2-B1: the left operand's cell writes and memory calls come before the branch
        if (track_mem() && !opt_.drop_left_floor && touches_cells(e.a))
        {
            left_floor = since(full, effect_from, obl_from);
        }
        const std::string t {new_temp(low::LType::Bool)};
        Effect fx;
        fx.kind = Effect::Kind::Branch;
        assign(fx.stmts, t, a.e);
        low::LStmt i;
        i.kind = low::LStmt::Kind::If;
        i.expr = e.bop == BinaryOp::LAnd ? low::var(t) : low::not_(low::var(t));
        std::set<int> reads;
        std::set<int> writes;
        FixedInfo first;
        bool has_fixed {false};
        const size_t acc_from {accs_.size()};
        branch_ctx_.push_back(BranchCtx {&full, {}});
        i.then_body = branch_body(*e.b, env, t, reads, writes, first, has_fixed, i.expr, fx.tags);
        const Deps extra {branch_ctx_.back().extra};
        branch_ctx_.pop_back();
        fx.stmts.push_back(std::move(i));
        if (has_fixed)
        {
            first.what += e.bop == BinaryOp::LAnd ? " in the right operand of `&&`" : " in the right operand of `||`";
        }
        const Deps value {add_branch(full, std::move(fx), join(join(a.dep, extra), left_floor), reads, writes,
                                     has_fixed, first, effect_from, obl_from, acc_from)};
        return {low::var(t), value};
    }

    // `c ? a : b` where an arm has effects (E5): `if (c) { a...; t := a } else { b...; t := b }`.
    LV branch_cond(const Expr& e, const Env& env, const E& guard, const Deps& gdep, Full& full)
    {
        if (guard != nullptr)
        {
            internal_error(e.loc, "an effect in a conditionally evaluated operand reached lowering");
            return {low::int_lit("0"), {}};
        }
        if (!in_full_)
        {
            internal_error(e.loc, "an effect in a conditional operand outside a full expression reached lowering");
            return {low::int_lit("0"), {}};
        }
        const size_t effect_from {full.effects.size()};
        const size_t obl_from {full.obls.size()};
        LV c {lower(*e.a, env, guard, gdep, full)};
        Deps cond_floor; // audit R2-B1
        if (track_mem() && !opt_.drop_left_floor && touches_cells(e.a))
        {
            cond_floor = since(full, effect_from, obl_from);
        }
        const std::string t {new_temp(ltype(e.type))};
        Effect fx;
        fx.kind = Effect::Kind::Branch;
        low::LStmt i;
        i.kind = low::LStmt::Kind::If;
        i.expr = c.e;
        std::set<int> reads;
        std::set<int> writes;
        FixedInfo first;
        bool has_fixed {false};
        const size_t acc_from {accs_.size()};
        branch_ctx_.push_back(BranchCtx {&full, {}});
        i.then_body = branch_body(*e.b, env, t, reads, writes, first, has_fixed, c.e, fx.tags);
        i.else_body = branch_body(*e.c, env, t, reads, writes, first, has_fixed, low::not_(c.e), fx.tags);
        const Deps extra {branch_ctx_.back().extra};
        branch_ctx_.pop_back();
        fx.stmts.push_back(std::move(i));
        if (has_fixed)
        {
            first.what += " in an arm of `?:`";
        }
        const Deps value {add_branch(full, std::move(fx), join(join(c.dep, extra), cond_floor), reads, writes,
                                     has_fixed, first, effect_from, obl_from, acc_from)};
        return {low::var(t), value};
    }

    // ---- emitting a full expression (0012 item 6) ----

    low::LStmt assert_stmt(E cond, low::Meta m)
    {
        low::LStmt s;
        s.kind = low::LStmt::Kind::Assert;
        s.expr = std::move(cond);
        s.check = low::Check {"assert.failed", std::move(m)};
        return s;
    }

    void emit_assert(std::vector<low::LStmt>& out, E cond, low::Meta m)
    {
        if (assert_count_++ == opt_.drop_assert)
        {
            return; // self-test hook (0011 item 13)
        }
        out.push_back(assert_stmt(std::move(cond), std::move(m)));
    }
    // A row whose failure ENDS its path (decision 0047 item 10, amended in M4.5d2v): `if (!cond) { assert false }`.  A failed `assert cond` is assumed afterwards, and an assumption
    // that the heap does not back leaves the path alive: Silicon then goes on to the automatic invariants of the loop, whose failure is an `internal` error, and reads a
    // field it holds no permission to.  A failed `assert false` assumes false: the user's error is the path's last.  The check is the assert's own (category, message, position).
    void emit_ending_assert(std::vector<low::LStmt>& out, const E& cond, low::Meta m)
    {
        if (assert_count_++ == opt_.drop_assert)
        {
            return;
        }
        low::LStmt ifs;
        ifs.kind = low::LStmt::Kind::If;
        ifs.expr = low::not_(cond);
        ifs.then_body.push_back(assert_stmt(low::bool_lit(false), std::move(m)));
        out.push_back(std::move(ifs));
    }

    // What the items of F that have exactly the set `set` are: `term_call` is the recursive call whose explicit termination check has it as
    // PRE (-1 if none), `ordinary` whether any other item (an obligation, a branch) has it.  A write or a snapshot is not in F.
    struct SetOrigin
    {
        bool ordinary {false};
        int term_call {-1};
    };
    static SetOrigin origin_of(const FlatFull& full, const Ids& set)
    {
        SetOrigin o;
        for (const FlatObl& x : full.obls)
        {
            if (x.dep != set)
            {
                continue;
            }
            if (x.term_of >= 0)
            {
                if (o.term_call < 0)
                {
                    o.term_call = x.term_of;
                }
            }
            else
            {
                o.ordinary = true;
            }
        }
        for (const FlatEffect& fx : full.effects)
        {
            if (fx.kind != FlatEffect::Kind::Branch)
            {
                continue;
            }
            if (fx.fixed < 0)
            {
                if (fx.deps == set)
                {
                    o.ordinary = true;
                }
            }
            else
            {
                Ids d {fx.deps};
                d.insert(fx.fixed);
                if (d == set)
                {
                    o.ordinary = true;
                }
            }
        }
        return o;
    }

    // The refusal of a full expression whose sets do not form a chain (decision 0021 item 3): `a` and `b` are two neighbours of the sorted
    // family, `a` not a subset of `b`.  One fixed point of each side witnesses it; the report is at the later-created one.  A side whose set
    // only a recursive call's termination check has is worded by that call (nothing visible depends on the witness alone): `the call to "h"
    // and the recursive call to "f" (its termination check needs "g" first) are not ordered`.
    void report_conflict(const FlatFull& full, const Ids& a, const Ids& b)
    {
        int p {-1};
        int q {-1};
        for (int x : a)
        {
            if (b.count(x) == 0U)
            {
                p = x;
                break;
            }
        }
        for (int x : b)
        {
            if (a.count(x) == 0U)
            {
                q = x;
                break;
            }
        }
        if (p < 0 || q < 0)
        {
            internal_error(SrcLoc {}, "the sets of a full expression are not a chain, but no witness was found");
            return;
        }
        const SetOrigin oa {origin_of(full, a)};
        const SetOrigin ob {origin_of(full, b)};
        const bool ta {!oa.ordinary && oa.term_call >= 0};
        const bool tb {!ob.ordinary && ob.term_call >= 0};
        // The fixed point each side is reported as: the recursive call itself for a termination side, else the witness.
        const int ra {ta ? oa.term_call : p};
        const int rb {tb ? ob.term_call : q};
        auto info = [&full](int id) -> const FixedInfo&
        {
            return full.info[static_cast<size_t>(id)];
        };
        auto quoted = [&info](int id)
        {
            return info(id).callee.empty() ? info(id).what : "\"" + info(id).callee + "\"";
        };
        auto recursive = [&](int call, int witness)
        {
            return "the recursive call to \"" + info(call).callee + "\" (its termination check needs " +
                   quoted(witness) + " first)";
        };
        const int lo {std::min(ra, rb)};
        const int hi {std::max(ra, rb)};
        const FixedInfo& later {info(hi)};
        std::string why;
        if (!ta && !tb)
        {
            const FixedInfo& first {info(std::min(p, q))};
            const FixedInfo& second {info(std::max(p, q))};
            if (!first.callee.empty() && !second.callee.empty())
            {
                why = "the calls to \"" + first.callee + "\" and \"" + second.callee + "\" are not ordered";
            }
            else
            {
                why = first.what + " and " + second.what + " are not ordered";
            }
            rejections_.push_back(Diag {second.loc, std::move(why)});
            return;
        }
        auto text = [&](int side_id, bool term, int call, int witness)
        {
            return term ? recursive(call, witness) : info(side_id).what;
        };
        const std::string left {text(ra, ta, oa.term_call, p)};
        const std::string right {text(rb, tb, ob.term_call, q)};
        if (ta != tb)
        {
            why = (ta ? right + " and " + left : left + " and " + right) + " are not ordered"; // the plain call first
        }
        else
        {
            why = (lo == ra ? left + " and " + right : right + " and " + left) + " are not ordered";
        }
        rejections_.push_back(Diag {later.loc, std::move(why)});
    }

    // Where each item of a full expression is emitted: `obl[k]` for `full.obls[k]`, `eff[k]` for a movable `full.effects[k]` (a write, a
    // snapshot or a call-free branch; unused, -2, for a fixed point).  Slot s is right after pi[s], before pi[s + 1]; -1 is before every
    // fixed point.  Computed once by flush; emission and both order checks read this one vector, so a check sees the order emission uses.
    struct Placement
    {
        std::vector<int> obl;
        std::vector<int> eff;
    };

    // The family F of a full expression on one half of its sets (decision 0021 item 3, decision 0025 item 2): the sets of every obligation
    // (the explicit termination checks among them), of the call-free branches, and D(B) of every branch that holds a call; sorted by size,
    // without duplicates.  A write or a snapshot is not in F, and a call is a fixed point.
    static std::vector<Ids> family(const FlatFull& full)
    {
        std::vector<Ids> sets;
        for (const FlatObl& o : full.obls)
        {
            sets.push_back(o.dep);
        }
        for (const FlatEffect& fx : full.effects)
        {
            if (fx.kind != FlatEffect::Kind::Branch)
            {
                continue;
            }
            if (fx.fixed < 0)
            {
                sets.push_back(fx.deps);
            }
            else
            {
                Ids d {fx.deps};
                d.insert(fx.fixed);
                sets.push_back(std::move(d));
            }
        }
        std::sort(sets.begin(), sets.end(),
                  [](const Ids& a, const Ids& b)
                  {
                      return a.size() != b.size() ? a.size() < b.size() : a < b;
                  });
        sets.erase(std::unique(sets.begin(), sets.end()), sets.end());
        return sets;
    }

    // The index i of the first neighbours sets[i], sets[i + 1] with sets[i] not a subset of sets[i + 1], or kChain when F is a chain under inclusion.
    static constexpr size_t kChain {~size_t {0U}};
    static size_t first_break(const std::vector<Ids>& sets)
    {
        for (size_t i {0U}; i + 1U < sets.size(); ++i)
        {
            if (!std::includes(sets[i + 1U].begin(), sets[i + 1U].end(), sets[i].begin(), sets[i].end()))
            {
                return i;
            }
        }
        return kChain;
    }

    // The fail-closed check of decision 0026 item 3: every base half is contained in its exact half, for every item of the full expression
    // (the set of each obligation and effect, and PRE of each fixed point).  Step 2 relies on it (it is today's smaller set), and so does the
    // claim that a rejection after step 1 failed is the base family's.  A violation is an internal error (hook: `drop_exact_set`).
    void check_halves(const Full& full)
    {
        auto inside = [](const Deps& d)
        {
            return std::includes(d.ex.begin(), d.ex.end(), d.base.begin(), d.base.end());
        };
        bool ok {true};
        for (const Obl& o : full.obls)
        {
            ok = ok && inside(o.dep);
        }
        for (const Effect& fx : full.effects)
        {
            ok = ok && inside(fx.deps);
        }
        for (const Deps& d : full.pre)
        {
            ok = ok && inside(d);
        }
        if (!ok)
        {
            internal_error(SrcLoc {}, "the base half of a set of a full expression is not contained in its exact half");
        }
    }

    // Write a full expression in two steps (decision 0026 item 2).  Step 1: project it onto the EXACT halves of its sets (what C completes
    // before each item) and test that F is a chain; if it is, those are its sets for everything that follows.  Step 2: otherwise project it
    // onto the BASE halves (the smaller sets of 0021 item 10, as M4.4c had them): if F is a chain, those are its sets (and the placement is
    // what M4.4c emitted, apart from the constant masks of 0026 part B).  Otherwise the program is refused with the base family's conflict (today's message and position; nothing is
    // reported by step 1).  Both choices are sound (a smaller set is safe, 0021 item 10); each full expression, and the inner one of each
    // branch with effects, decides on its own.  `LowerOptions::force_base_sets` skips step 1.
    void flush(const Full& full, std::vector<low::LStmt>& out)
    {
        check_halves(full);
        FlatFull flat {project(full, Half::Exact)};
        std::vector<Ids> sets {family(flat)};
        size_t broken {first_break(sets)};
        // self-test hook force_base_sets: skip step 1
        const bool exact_chain {broken == kChain && !opt_.force_base_sets};
        if (!exact_chain)
        {
            flat = project(full, Half::Base);
            sets = family(flat);
            broken = first_break(sets);
            if (broken != kChain)
            {
                report_conflict(flat, sets[broken], sets[broken + 1U]);
            }
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
    void emit_full(FlatFull& full, const std::vector<Ids>& sets, bool chain, std::vector<low::LStmt>& out)
    {
        const int n {full.nfixed};
        // pi: the fixed points in the order they are emitted.
        std::vector<int> pi;
        std::vector<char> placed(static_cast<size_t>(n), 0);
        if (chain)
        {
            for (const Ids& s : sets)
            {
                for (int id : s)
                {
                    if (placed[static_cast<size_t>(id)] == 0)
                    {
                        placed[static_cast<size_t>(id)] = 1;
                        pi.push_back(id);
                    }
                }
            }
        }
        for (int id {0}; id < n; ++id)
        {
            if (placed[static_cast<size_t>(id)] == 0)
            {
                pi.push_back(id);
            }
        }
        if (opt_.perturb_fixed_order && chain && pi.size() >= 2U)
        {
            std::swap(pi[0U], pi[1U]); // self-test hook (0011 item 13)
        }
        std::vector<int> pos(static_cast<size_t>(n), 0); // the index of each fixed point in pi
        for (size_t k {0U}; k < pi.size(); ++k)
        {
            pos[static_cast<size_t>(pi[k])] = static_cast<int>(k);
        }
        // The slot of an item of F: right after pi[|S| - 1] (-1: before every fixed point).  Of a write or a snapshot: right after the member
        // of its set that comes last in pi (-1 for an empty set).
        auto prefix_slot = [n](const Ids& s)
        {
            return std::min(static_cast<int>(s.size()) - 1, n - 1);
        };
        auto last_member = [&pos](const Ids& s)
        {
            int m {-1};
            for (int id : s)
            {
                m = std::max(m, pos[static_cast<size_t>(id)]);
            }
            return m;
        };
        Placement place;
        for (const FlatObl& o : full.obls)
        {
            place.obl.push_back(prefix_slot(o.dep));
        }
        bool perturbed {false};
        bool delayed {false};
        bool mem_perturbed {false};
        // Whether an item created after `w` has a set that contains w's set.
        auto depended_on = [&full](const FlatEffect& w)
        {
            auto contains = [&w](const Ids& big)
            {
                return std::includes(big.begin(), big.end(), w.deps.begin(), w.deps.end());
            };
            for (const FlatObl& o : full.obls)
            {
                if (o.seq > w.seq && contains(o.dep))
                {
                    return true;
                }
            }
            for (const FlatEffect& x : full.effects)
            {
                if (&x != &w && x.seq > w.seq && contains(x.deps))
                {
                    return true;
                }
            }
            return false;
        };
        for (const FlatEffect& fx : full.effects)
        {
            int slot {-2};
            if (fx.fixed < 0)
            {
                slot = fx.kind == FlatEffect::Kind::Branch ? prefix_slot(fx.deps) : last_member(fx.deps);
                if (opt_.perturb_write_slot && !perturbed && fx.kind == FlatEffect::Kind::Write && !fx.deps.empty())
                { // self-test hook (0025)
                    slot = -1;
                    perturbed = true;
                }
                if (opt_.perturb_memory_order && !mem_perturbed && fx.kind == FlatEffect::Kind::Store &&
                    !fx.deps.empty())
                { // self-test hook (0039)
                    slot = -1;
                    mem_perturbed = true;
                }
                if (opt_.delay_write_slot && !delayed && fx.kind == FlatEffect::Kind::Write && !fx.deps.empty() &&
                    depended_on(fx))
                { // self-test hook (0025)
                    slot = n - 1;
                    delayed = true;
                }
            }
            place.eff.push_back(slot);
        }
        if (chain)
        {
            check_variable_order(full, pos, place);
            check_fixed_order(full, pi, pos, place);
            check_memory_order(full, pos, place);
        }
        for (int slot {-1}; slot < n; ++slot)
        {
            // The items of this slot, obligations and movable effects, in creation order.
            std::vector<std::pair<size_t, std::pair<bool, size_t>>> items; // (seq, (is obligation, index))
            for (size_t k {0U}; k < full.obls.size(); ++k)
            {
                if (place.obl[k] == slot)
                {
                    items.push_back({full.obls[k].seq, {true, k}});
                }
            }
            for (size_t k {0U}; k < full.effects.size(); ++k)
            {
                if (full.effects[k].fixed < 0 && place.eff[k] == slot)
                {
                    items.push_back({full.effects[k].seq, {false, k}});
                }
            }
            std::sort(items.begin(), items.end());
            for (const auto& it : items)
            {
                if (it.second.first)
                {
                    FlatObl& o {full.obls[it.second.second]};
                    // The explicit termination check of a call (decision 0021 item 4) is written only when another fixed point, outside its set,
                    // comes before the call: it then asserts what Silicon's own check at the call could not reach.  Not written: nothing is
                    // counted (the self-test hook counts asserts) and no temporary is made.
                    if (o.term_of >= 0 && pos[static_cast<size_t>(o.term_of)] <= static_cast<int>(o.dep.size()))
                    {
                        continue;
                    }
                    if (!o.stmts.empty())
                    {
                        for (const low::LStmt& st : o.stmts)
                        {
                            out.push_back(st);
                        }
                        continue;
                    }
                    emit_assert(out, low::implies(o.guard, o.cond), o.meta);
                    continue;
                }
                FlatEffect& fx {full.effects[it.second.second]};
                switch (fx.kind)
                {
                    case FlatEffect::Kind::Call:
                        break;
                    case FlatEffect::Kind::Write:
                        assign(out, fx.name, fx.value);
                        if (!fx.flag.empty())
                        {
                            assign(out, fx.flag, low::bool_lit(true));
                        }
                        break;
                    case FlatEffect::Kind::Snapshot:
                        assign(out, fx.name, fx.value);
                        for (const low::LStmt& st : fx.stmts)
                        {
                            out.push_back(st); // a loaded pointer's case splits
                        }
                        break;
                    case FlatEffect::Kind::Branch:
                    case FlatEffect::Kind::Store:
                        for (low::LStmt& st : fx.stmts)
                        {
                            out.push_back(std::move(st));
                        }
                        break;
                }
            }
            if (slot + 1 < n)
            {
                FlatEffect& fx {
                    full.effects[full.fixed_effect[static_cast<size_t>(pi[static_cast<size_t>(slot + 1)])]]};
                if (fx.kind == FlatEffect::Kind::Branch)
                {
                    for (low::LStmt& st : fx.stmts)
                    {
                        out.push_back(std::move(st));
                    }
                    continue;
                }
                CallItem& c {fx.call};
                low::LStmt s;
                s.kind = low::LStmt::Kind::Call;
                s.name = c.method;
                s.target = c.target;
                s.targets = c.targets;
                s.args = c.args;
                s.check = c.check;
                s.has_termination = c.has_termination;
                s.termination = c.termination;
                out.push_back(std::move(s));
                for (const low::LStmt& st : c.after)
                {
                    out.push_back(st);
                }
            }
        }
    }

    // The fail-closed check of the main property of decision 0021 (like check_variable_order): the fixed points emitted before an
    // obligation or a call-free branch are exactly its set; every fixed point follows its PRE; and a branch with a call, and a recursive
    // call whose explicit termination check is not written, come right after their PRE.  A violation would be a wrong Viper program (a
    // check hidden by a call that does not return), so it is an internal error.  For a write and a snapshot (decision 0025 item 2): every
    // member of its set is emitted before it, and every item created later whose set (for a fixed point: its PRE) contains that set is
    // emitted after it.  `place` is the placement emission uses.
    void check_fixed_order(const FlatFull& full, const std::vector<int>& pi, const std::vector<int>& pos,
                           const Placement& place)
    {
        auto prefix_is = [&pi](const Ids& s)
        {
            if (s.size() > pi.size())
            {
                return false;
            }
            for (size_t k {0U}; k < s.size(); ++k)
            {
                if (s.count(pi[k]) == 0U)
                {
                    return false;
                }
            }
            return true;
        };
        constexpr const char* kSet {
            "the fixed points emitted before an item of a full expression are not exactly the set it depends on"};
        for (const FlatObl& o : full.obls)
        {
            if (!prefix_is(o.dep))
            {
                internal_error(SrcLoc {}, kSet);
                return;
            }
        }
        for (const FlatEffect& fx : full.effects)
        {
            if (fx.fixed < 0 && fx.kind == FlatEffect::Kind::Branch && !prefix_is(fx.deps))
            {
                internal_error(SrcLoc {}, kSet);
                return;
            }
        }
        for (int id {0}; id < full.nfixed; ++id)
        {
            const Ids& pre {full.pre[static_cast<size_t>(id)]};
            for (int p : pre)
            {
                if (pos[static_cast<size_t>(p)] >= pos[static_cast<size_t>(id)])
                {
                    internal_error(SrcLoc {},
                                   "a fixed point of a full expression is emitted before something it follows");
                    return;
                }
            }
            const FlatEffect& fx {full.effects[full.fixed_effect[static_cast<size_t>(id)]]};
            if (fx.kind == FlatEffect::Kind::Branch &&
                !(pos[static_cast<size_t>(id)] == static_cast<int>(pre.size()) && prefix_is(pre)))
            {
                internal_error(SrcLoc {},
                               "a branch with a call is not emitted right after what its condition depends on");
                return;
            }
        }
        for (const FlatObl& o : full.obls) // a recursive call without a written explicit check: right after its PRE
        {
            if (o.term_of >= 0 && pos[static_cast<size_t>(o.term_of)] == static_cast<int>(o.dep.size()) &&
                !prefix_is(o.dep))
            {
                internal_error(
                    SrcLoc {},
                    "a recursive call is emitted without its explicit termination check and not right after what it depends on");
                return;
            }
        }
        // Writes and snapshots.  The emission order of an item: its slot, then movable items (0) before the fixed point that follows the slot
        // (1), then creation order; the fixed point at index p of pi follows the items of slot p - 1.
        using Order = std::tuple<int, int, size_t>;
        auto movable = [](int slot, size_t seq)
        {
            return Order {slot, 0, seq};
        };
        auto fixed_at = [](int p)
        {
            return Order {p - 1, 1, 0};
        };
        auto contains = [](const Ids& big, const Ids& small)
        {
            return std::includes(big.begin(), big.end(), small.begin(), small.end());
        };
        for (size_t i {0U}; i < full.effects.size(); ++i)
        {
            const FlatEffect& w {full.effects[i]};
            if (w.fixed >= 0 || w.kind == FlatEffect::Kind::Branch)
            {
                continue;
            }
            const int slot {place.eff[i]};
            for (int id : w.deps)
            {
                if (pos[static_cast<size_t>(id)] > slot)
                {
                    internal_error(
                        SrcLoc {},
                        "a write or a snapshot of a full expression is emitted before a fixed point of its set");
                    return;
                }
            }
            const Order mine {movable(slot, w.seq)};
            constexpr const char* kLater {
                "a write or a snapshot of a full expression is emitted after an item that follows it"};
            for (size_t k {0U}; k < full.obls.size(); ++k)
            {
                const FlatObl& o {full.obls[k]};
                if (o.seq > w.seq && contains(o.dep, w.deps) && !(movable(place.obl[k], o.seq) > mine))
                {
                    internal_error(SrcLoc {}, kLater);
                    return;
                }
            }
            for (size_t k {0U}; k < full.effects.size(); ++k)
            {
                const FlatEffect& x {full.effects[k]};
                if (k == i || x.seq <= w.seq || !contains(x.deps, w.deps))
                {
                    continue;
                }
                const Order theirs {x.fixed >= 0 ? fixed_at(pos[static_cast<size_t>(x.fixed)])
                                                 : movable(place.eff[k], x.seq)};
                if (!(theirs > mine))
                {
                    internal_error(SrcLoc {}, kLater);
                    return;
                }
            }
        }
    }

    // The fail-closed check of the order: for every local, its snapshots and writes, its flag reads (an obligation that reads the
    // `uninitialized` flag, decision 0025 item 1) and the branches that touch it (a branch counts as the accesses of its inner effects) are
    // emitted in the order they were created, except that two reads may swap.  A hoisted write that overtook an earlier read or write of the
    // same local would change a value, so it is an internal error rather than a wrong Viper program.  The key of an item is its place in the
    // emitted order: a movable item in slot s comes after pi[s] (key 2s + 1) and before pi[s + 1] (key 2s + 3), so its key is 2s + 2, the
    // same as 2|S| for an item of F; the fixed point at index i has the key 2i + 1.
    void check_variable_order(const FlatFull& full, const std::vector<int>& pos, const Placement& place)
    {
        struct Access
        {
            int id;
            bool write;
            int key;
            size_t seq;
        };
        std::vector<Access> all;
        for (size_t i {0U}; i < full.effects.size(); ++i)
        {
            const FlatEffect& fx {full.effects[i]};
            if (fx.reads.empty() && fx.writes.empty())
            {
                continue;
            }
            const int key {fx.fixed >= 0 ? 2 * pos[static_cast<size_t>(fx.fixed)] + 1 : 2 * (place.eff[i] + 1)};
            for (int id : fx.reads)
            {
                all.push_back({id, false, key, fx.seq});
            }
            for (int id : fx.writes)
            {
                all.push_back({id, true, key, fx.seq});
            }
        }
        for (size_t k {0U}; k < full.obls.size(); ++k)
        {
            if (full.obls[k].flag_of >= 0)
            {
                all.push_back({full.obls[k].flag_of, false, 2 * (place.obl[k] + 1), full.obls[k].seq});
            }
        }
        std::stable_sort(all.begin(), all.end(),
                         [](const Access& x, const Access& y)
                         {
                             return x.seq < y.seq;
                         }); // creation order
        if (opt_.perturb_access_order && !all.empty())
        { // self-test hook (0011 item 13): the first access moves behind everything
            const size_t first {all.front().seq};
            for (Access& a : all)
            {
                if (a.seq == first)
                {
                    a.key += 1000;
                }
            }
        }
        for (size_t i {0U}; i < all.size(); ++i)
        {
            for (size_t j {i + 1U}; j < all.size(); ++j)
            {
                if (all[i].id != all[j].id || (!all[i].write && !all[j].write))
                {
                    continue;
                }
                // emitted order is (key, seq): the earlier-created access must not come later
                if (all[i].key > all[j].key)
                {
                    internal_error(
                        SrcLoc {},
                        "an access to a local was emitted before an earlier access to it in the same full expression");
                }
            }
        }
    }

    // The fail-closed check of decision 0039 item 4 (audit S4 (ii)): for each write or snapshot of a cell field and each memory call on that field
    // that C orders with it, read from the expression tree (the paths of the accesses), the emitted order agrees.  Per key, the order of the writes
    // and snapshots is `check_variable_order`'s (keys are negative ids there).  An E5 branch counts as the accesses inside it (its tags).
    void check_memory_order(const FlatFull& full, const std::vector<int>& pos, const Placement& place)
    {
        auto key_of = [&](size_t i)
        {
            const FlatEffect& fx {full.effects[i]};
            return fx.fixed >= 0 ? 2 * pos[static_cast<size_t>(fx.fixed)] + 1 : 2 * (place.eff[i] + 1);
        };
        for (size_t i {0U}; i < full.effects.size(); ++i)
        {
            for (size_t j {0U}; j < full.effects.size(); ++j)
            {
                if (i == j || full.effects[i].seq > full.effects[j].seq)
                {
                    continue; // `i` was created first
                }
                const FlatEffect& x {full.effects[i]};
                const FlatEffect& y {full.effects[j]};
                for (const MemTag& a : x.tags)
                {
                    for (const MemTag& b : y.tags)
                    {
                        if (a.field != b.field)
                        {
                            continue;
                        }
                        // A call and a cell write or snapshot of one field, or two cell accesses of one field and different keys (one a write) that C orders (audit R2-B1).
                        const bool call_pair {a.call != b.call};
                        const bool access_pair {!a.call && !b.call && a.key != b.key && (a.write || b.write)};
                        if (!call_pair && !access_pair)
                        {
                            continue;
                        }
                        if (order_rel(a.path, a.write && !a.call, b.path, b.write && !b.call, b.after) != 1)
                        {
                            continue;
                        }
                        if (key_of(i) > key_of(j))
                        {
                            internal_error(
                                SrcLoc {},
                                call_pair
                                    ? "a cell access of a full expression was emitted on the wrong side of a memory call (decision 0039 item 4)"
                                    : "a cell access of a full expression was emitted on the wrong side of another access C orders before it (decision 0039 item 4)");
                            return;
                        }
                    }
                }
            }
        }
    }

    Env body_env() const
    {
        return Env {};
    }

    Env contract_env() const
    {
        Env env;
        env.formal_leaves.resize(fn_.f->params.size());
        for (size_t k {0U}; k < fn_.f->params.size(); ++k)
        {
            const Type& t {fn_.f->params[k].type};
            if (t.is_struct())
            { // decision 0044 item 3: the leaves are the formals, holding the entry values
                env.formals.push_back(nullptr);
                for (const StructLeaf& l : struct_leaves(*t.struct_type))
                {
                    env.formal_leaves[k].push_back(
                        low::var(formal_name(*fn_.f, static_cast<int>(k)) + path_suffix(l.path)));
                }
            }
            else
            {
                env.formals.push_back(low::var(formal_name(*fn_.f, static_cast<int>(k))));
            }
        }
        if (fn_.f->ret.is_struct())
        { // decision 0044 item 4
            for (const StructLeaf& l : struct_leaves(*fn_.f->ret.struct_type))
            {
                env.result_leaves.push_back(low::var(std::string(kResult) + path_suffix(l.path)));
            }
        }
        else if (!fn_.f->ret.is_void())
        {
            env.result = low::var(kResult);
        }
        return env;
    }

    // Several expressions in ONE full expression (a store's address and value, decision 0032 item 9); the others are one expression.
    // `copy_of`, when given, is the first expression, whose pointer value is being copied: its liveness check joins the full expression.
    // A statement's own store of a cell (`x = e;`, `*p = e;`): no effect, no E3 trigger (audit S6), but an access for item 3's pairs: its side effect is
    // unsequenced with a write inside `e` that no sequence point separates from it (`*p = (*q)++;`).
    struct StoreTarget
    {
        const Var* var {nullptr};   // a cell by name
        const Expr* addr {nullptr}; // or the address of `*addr = ...`, the first root
        SrcLoc loc;
        // the struct of a struct store (decision 0044 item 5): `addr` is its slot, and every leaf is a write
        const StructType* st {nullptr};
    };
    std::vector<LV> full_exprs(const std::vector<const Expr*>& es, std::vector<low::LStmt>& out,
                               const Expr* copy_of = nullptr, bool share_first = false,
                               const StoreTarget* target = nullptr, const Expr* use_second = nullptr,
                               bool use_wf = false, int struct_use = -1, bool struct_wf = false)
    {
        Full full;
        const std::set<int> saved_written {std::move(written_)};
        const bool saved_in_full {in_full_};
        written_.clear();
        for (const Expr* e : es)
        {
            collect_writes(*e, written_);
        }
        accs_.clear(); // decision 0039: the cell accesses and memory calls of this full expression
        mem_w_.clear();
        mem_calls_.clear();
        mem_depth_.clear();
        branch_guards_.clear();
        branch_ctx_.clear();
        path_.clear();
        for (const Expr* e : es)
        {
            mem_scan(e);
        }
        in_full_ = true;
        std::vector<LV> vs;
        for (const Expr* e : es)
        {
            vs.push_back(lower(*e, body_env(), nullptr, {}, full));
            if (share_first && vs.size() == 1U)
            { // the later expressions read this node's value, not a second evaluation
                shared_[e] = vs.front();
                shared_path_[e] = Path {{e, 0}};
            }
        }
        if (share_first)
        {
            shared_.erase(es.front());
            shared_path_.erase(es.front());
        }
        if (target != nullptr)
        {
            Acc a;
            a.write = true;
            a.loc = target->loc;
            if (target->var != nullptr)
            {
                a.key = key_of_cell(*target->var);
                a.field = fkey(target->var->type);
                a.place = low::var(var_name(*target->var));
                a.text = target->var->name;
            }
            else if (target->addr->type.pointee != nullptr)
            {
                a.key = key_of_addr(*target->addr);
                a.field = fkey(*target->addr->type.pointee);
                a.place = vs.front().e;
                a.dep = vs.front().dep;
                a.text = place_text(*target->addr);
            }
            path_.clear();
            if (!a.field.empty())
            {
                register_access(std::move(a), full, nullptr);
            }
        }
        if (target != nullptr && target->st != nullptr)
        { // decision 0044 item 5: a struct store writes every destination leaf's key
            const std::string base_text {struct_dest_root(*target->addr).first};
            const bool base_arrow {struct_dest_root(*target->addr).second};
            const std::vector<StructLeaf> rels {struct_leaves(*target->st, base_text, base_arrow)};
            std::vector<StructLeaf> plain {struct_leaves(*target->st)};
            for (size_t i {0U}; i < plain.size(); ++i)
            {
                Acc a;
                a.write = true;
                a.loc = target->loc;
                a.key = key_of_addr_leaf(*target->addr, plain[i].path);
                a.field = fkey(plain[i].type);
                a.place = chain_ref(vs.front().e, plain[i].path);
                a.dep = vs.front().dep;
                a.text = rels[i].text;
                path_.clear();
                register_access(std::move(a), full, nullptr);
            }
        }
        if (struct_use >= 0 && !is_aggregate_value(*es[static_cast<size_t>(struct_use)]))
        {
            struct_ptr_uses(*es[static_cast<size_t>(struct_use)], vs[static_cast<size_t>(struct_use)], nullptr, {},
                            full, struct_wf);
        }
        if (copy_of != nullptr)
        {
            ptr_use(*copy_of, vs.front(), nullptr, {}, full, use_wf);
        }
        if (use_second != nullptr)
        {
            // a pointer stored through a pointer is a use, and must be well-formed (decision 0040 item 2)
            ptr_use(*use_second, vs[1U], nullptr, {}, full, true);
        }
        in_full_ = saved_in_full;
        written_ = saved_written;
        flush(full, out);
        return vs;
    }

    LV full_expr(const Expr& e, std::vector<low::LStmt>& out, bool pointer_copy = false)
    {
        return full_exprs({&e}, out, pointer_copy ? &e : nullptr).front();
    }

    // ---- statements ----

    void assign(std::vector<low::LStmt>& out, const std::string& name, E value)
    {
        low::LStmt s;
        s.kind = low::LStmt::Kind::Assign;
        s.name = name;
        s.expr = std::move(value);
        out.push_back(std::move(s));
    }

    void stmt(const Stmt& s, std::vector<low::LStmt>& out)
    {
        switch (s.kind)
        {
            case Stmt::Kind::Decl:
            case Stmt::Kind::Assign:
            {
                if (s.var->address_taken)
                { // a cell: written by name (decision 0032 item 6); a Decl without an initializer writes nothing
                    if (s.expr == nullptr)
                    {
                        break;
                    }
                    StoreTarget target;
                    target.var = s.var;
                    target.loc = s.loc;
                    LV v {full_exprs({s.expr}, out, s.var->type.is_ptr() ? s.expr : nullptr, false,
                                     s.kind == Stmt::Kind::Assign ? &target : nullptr, nullptr, true)
                              .front()};
                    write_cell(out, *s.var, v.e, s.loc);
                    break;
                }
                if (s.expr == nullptr)
                { // a Decl without an initializer (check_program allows no other)
                    reset(out, *s.var);
                    break;
                }
                LV v {full_expr(*s.expr, out, s.var->type.is_ptr())}; // a pointer copy is a use of the pointer's value
                assign(out, var_name(*s.var), v.e);
                if (flagged(*s.var))
                {
                    assign(out, flag_name(*s.var), low::bool_lit(true));
                }
                break;
            }
            case Stmt::Kind::ExprStmt:
                // Only its obligations and calls matter; a pointer value that is evaluated and discarded is still a use of it (decision 0032 item 6).
                full_expr(*s.expr, out, s.expr->type.is_ptr());
                break;
            case Stmt::Kind::If:
            {
                LV c {full_expr(*s.expr, out)};
                low::LStmt i;
                i.kind = low::LStmt::Kind::If;
                i.expr = c.e;
                if (s.then_body != nullptr)
                {
                    stmt(*s.then_body, i.then_body);
                }
                if (s.else_body != nullptr)
                {
                    stmt(*s.else_body, i.else_body);
                }
                out.push_back(std::move(i));
                break;
            }
            case Stmt::Kind::While:
                loop(s, out);
                break;
            case Stmt::Kind::Store:
                store(s, out);
                break;
            case Stmt::Kind::Return:
                ret(s, out);
                break;
            case Stmt::Kind::Check:
            {
                LV v {full_expr(*s.expr, out)};
                emit_assert(
                    out, v.e,
                    meta("check", s.loc, s.from_assert ? "this `assert` may not hold" : "this `check` may not hold"));
                break;
            }
            case Stmt::Kind::Block:
            {
                // The cells declared directly in this block are allocated when it is entered (decision 0032 item 4).
                const std::vector<const Var*> cells {cells_of(s)};
                for (const Var* c : cells)
                {
                    alloc_cell(out, *c);
                }
                if (!cells.empty())
                {
                    fn_.scopes.push_back(Fn::Scope {cells, s.end_loc});
                }
                // A goto past a declaration lands in this block with the variable in scope: it starts indeterminate at the block's
                // entry (decision 0019 item 4).
                for (const Stmt* c : s.stmts)
                {
                    if (c->kind == Stmt::Kind::Decl && c->expr == nullptr && !c->var->address_taken)
                    {
                        reset(out, *c->var);
                    }
                }
                for (const Stmt* c : s.stmts)
                {
                    stmt(*c, out);
                }
                if (!cells.empty())
                { // fall-through: reported at the closing brace (decision 0032 item 8)
                    scope_exit(out, cells, s.end_loc);
                    fn_.scopes.pop_back();
                }
                break;
            }
            case Stmt::Kind::Switch:
                switch_stmt(s, out);
                break;
            case Stmt::Kind::Case:
                internal_error(s.loc, "a case label outside the body of its switch reached lowering");
                break;
            case Stmt::Kind::Break:
                leave_to(out, fn_.jump_depth, s, s.loc);
                jump(out, "l_brk__" + std::to_string(s.jump_id));
                break;
            case Stmt::Kind::Continue:
                leave_to(out, fn_.jump_depth, s, s.loc);
                jump(out, "l_cont__" + std::to_string(s.jump_id));
                break;
            case Stmt::Kind::Goto:
                leave_to(out, fn_.label_depth, s, s.loc);
                jump(out, user_label(s));
                break;
            case Stmt::Kind::Label:
                if (user_targets_.count(s.jump_id) != 0U)
                {
                    label(out, user_label(s));
                }
                break;
        }
    }

    // ---- jumps (decision 0014) ----

    // A jump leaves every open block above the depth of its target (decision 0038 item 7): the scope exit of 0032 item 8 for each block that holds
    // cells, innermost first, reported at the jump.  `depths` maps the jump's id to the number of scopes open at its target.
    void leave_to(std::vector<low::LStmt>& out, const std::map<int, size_t>& depths, const Stmt& s, const SrcLoc& at)
    {
        const auto it {depths.find(s.jump_id)};
        if (it == depths.end())
        {
            internal_error(s.loc, "a jump whose target was not seen reached lowering");
            return;
        }
        for (size_t k {fn_.scopes.size()}; k-- > it->second;)
        {
            scope_exit(out, fn_.scopes[k].cells, at);
        }
    }

    // The number of scopes open at each user label (the lowering opens a scope for each block, and each switch body, that declares a cell).
    void label_depths(const Stmt& s, size_t depth)
    {
        switch (s.kind)
        {
            case Stmt::Kind::Label:
                fn_.label_depth[s.jump_id] = depth;
                break;
            case Stmt::Kind::If:
                if (s.then_body != nullptr)
                {
                    label_depths(*s.then_body, depth);
                }
                if (s.else_body != nullptr)
                {
                    label_depths(*s.else_body, depth);
                }
                break;
            case Stmt::Kind::While:
                if (s.body != nullptr)
                {
                    label_depths(*s.body, depth);
                }
                if (s.step != nullptr)
                {
                    label_depths(*s.step, depth);
                }
                break;
            case Stmt::Kind::Switch:
                if (s.body != nullptr)
                {
                    const size_t d {depth + (cells_of(*s.body).empty() ? 0 : 1)};
                    for (const Stmt* c : s.body->stmts)
                    {
                        label_depths(*c, d);
                    }
                }
                break;
            case Stmt::Kind::Block:
            {
                const size_t d {depth + (cells_of(s).empty() ? 0 : 1)};
                for (const Stmt* c : s.stmts)
                {
                    label_depths(*c, d);
                }
                break;
            }
            default:
                break;
        }
    }

    void jump(std::vector<low::LStmt>& out, std::string label)
    {
        low::LStmt g;
        g.kind = low::LStmt::Kind::Goto;
        g.name = std::move(label);
        out.push_back(std::move(g));
    }

    void label(std::vector<low::LStmt>& out, std::string name)
    {
        low::LStmt l;
        l.kind = low::LStmt::Kind::Label;
        l.name = std::move(name);
        out.push_back(std::move(l));
    }

    // The `break`s and `continue`s inside `s` (a step is not searched: it holds none), by target id.  A label is emitted only
    // for a construct that something jumps to, so programs without these statements lower exactly as before.
    static void collect_jumps(const Stmt& s, std::set<int>& breaks, std::set<int>& continues)
    {
        switch (s.kind)
        {
            case Stmt::Kind::Break:
                breaks.insert(s.jump_id);
                break;
            case Stmt::Kind::Continue:
                continues.insert(s.jump_id);
                break;
            case Stmt::Kind::If:
                if (s.then_body != nullptr)
                {
                    collect_jumps(*s.then_body, breaks, continues);
                }
                if (s.else_body != nullptr)
                {
                    collect_jumps(*s.else_body, breaks, continues);
                }
                break;
            case Stmt::Kind::While:
            case Stmt::Kind::Switch:
                if (s.body != nullptr)
                {
                    collect_jumps(*s.body, breaks, continues);
                }
                break;
            case Stmt::Kind::Block:
                for (const Stmt* c : s.stmts)
                {
                    collect_jumps(*c, breaks, continues);
                }
                break;
            default:
                break;
        }
    }

    // The user labels some `goto` of the function targets (a label is emitted only then, decision 0015 item 3).
    void collect_gotos(const Stmt& s, std::set<int>& targets)
    {
        switch (s.kind)
        {
            case Stmt::Kind::Goto:
                targets.insert(s.jump_id);
                break;
            case Stmt::Kind::Label:
                user_names_[s.jump_id] = s.label_name;
                break;
            case Stmt::Kind::If:
                if (s.then_body != nullptr)
                {
                    collect_gotos(*s.then_body, targets);
                }
                if (s.else_body != nullptr)
                {
                    collect_gotos(*s.else_body, targets);
                }
                break;
            case Stmt::Kind::While:
                if (s.body != nullptr)
                {
                    collect_gotos(*s.body, targets);
                }
                if (s.step != nullptr)
                {
                    collect_gotos(*s.step, targets);
                }
                break;
            case Stmt::Kind::Switch:
                if (s.body != nullptr)
                {
                    collect_gotos(*s.body, targets);
                }
                break;
            case Stmt::Kind::Block:
                for (const Stmt* c : s.stmts)
                {
                    collect_gotos(*c, targets);
                }
                break;
            default:
                break;
        }
    }

    // A Goto carries only the label's id: its name is the Label's (collect_gotos found them all before lowering the body).
    std::string user_label(const Stmt& s) const
    {
        const auto it {user_names_.find(s.jump_id)};
        return "l_user_" + sanitize(it != user_names_.end() ? it->second : s.label_name) + "__" +
               std::to_string(s.jump_id);
    }

    // `switch (e) { ... }`: the controlling expression is evaluated once into a temporary (its obligations and calls first),
    // then a chain of tests jumps to the label of the matching `case` (in source order; case values are distinct), else to the
    // `default` label, else past the switch.  The body is lowered in order with a label at each case marker, so falling through
    // is falling into the next label and `break` jumps to the label after the body.
    void switch_stmt(const Stmt& s, std::vector<low::LStmt>& out)
    {
        const std::string n {std::to_string(s.jump_id)};
        LV v {full_expr(*s.expr, out)};
        const std::string t {"t__" + std::to_string(fn_.ntemp++)};
        fn_.temps.push_back(low::LVar {t, low::LType::Int});
        assign(out, t, v.e);
        // The cells of the body are allocated before the dispatch: a case label may jump past their declarations (decision 0032 item 4).
        const std::vector<const Var*> body_cells {cells_of(*s.body)};
        for (const Var* c : body_cells)
        {
            alloc_cell(out, *c);
        }
        // A case label may jump past a declaration of the body without an initializer: reset them before the dispatch (0019 item 4).
        for (const Stmt* c : s.body->stmts)
        {
            if (c->kind == Stmt::Kind::Decl && c->expr == nullptr && !c->var->address_taken)
            {
                reset(out, *c->var);
            }
        }
        std::set<int> breaks;
        std::set<int> continues;
        collect_jumps(*s.body, breaks, continues);
        // The case values' obligations and the guards that each equals the front end's value are in the prologue of the method
        // (decision 0017 item 5), so the dispatch compares with the front end's value and asserts nothing.
        bool has_default {false};
        size_t index {0U};
        for (const Stmt* c : s.body->stmts)
        {
            if (c->kind == Stmt::Kind::Case)
            {
                if (c->expr == nullptr)
                {
                    has_default = true;
                }
                else
                {
                    low::LStmt i;
                    i.kind = low::LStmt::Kind::If;
                    i.expr = low::bin(low::BinOp::Eq, low::var(t), low::int_lit(c->folded));
                    jump(i.then_body, "l_case__" + n + "_" + std::to_string(index));
                    out.push_back(std::move(i));
                }
            }
            ++index;
        }
        if (!body_cells.empty())
        {
            fn_.scopes.push_back(Fn::Scope {body_cells, s.body->end_loc});
        }
        // A `break` lands before the body's own scope exit, which runs once at the label: it leaves only the blocks nested inside the body (0038 item 7).
        fn_.jump_depth[s.jump_id] = fn_.scopes.size();
        const bool need_break_label {!has_default || breaks.count(s.jump_id) != 0U};
        jump(out, has_default ? "l_default__" + n : "l_brk__" + n);
        index = 0U;
        for (const Stmt* c : s.body->stmts)
        {
            if (c->kind == Stmt::Kind::Case)
            {
                label(out, c->expr == nullptr ? "l_default__" + n : "l_case__" + n + "_" + std::to_string(index));
            }
            else
            {
                stmt(*c, out);
            }
            ++index;
        }
        if (need_break_label)
        {
            label(out, "l_brk__" + n); // before the scope exit: the dispatch of an unmatched value passes through it
        }
        if (!body_cells.empty())
        {
            scope_exit(out, body_cells, s.body->end_loc);
            fn_.scopes.pop_back();
        }
    }

    void ret(const Stmt& s, std::vector<low::LStmt>& out)
    {
        if (s.expr != nullptr && s.expr->type.is_struct())
        {
            // decision 0044 item 4: every leaf of the value (an aggregate of leaf reads, a call's or a member chain's results) goes into its result, before the scope
            // exits and the postconditions; a call's pointer leaf is a use of the pointer, as a returned pointer is
            const LV v {full_exprs({s.expr}, out, nullptr, false, nullptr, nullptr, false, 0).front()};
            const std::vector<StructLeaf> ls {struct_leaves(*s.expr->type.struct_type)};
            if (v.leaves.size() != ls.size())
            {
                internal_error(
                    s.loc, "a returned struct value with a different number of leaves than its type reached lowering");
            }
            for (size_t i {0U}; i < ls.size() && i < v.leaves.size(); ++i)
            {
                assign(out, std::string(kResult) + path_suffix(ls[i].path), v.leaves[i]);
            }
        }
        else if (s.expr != nullptr)
        {
            // a returned pointer is a use of its value (decision 0037 item 3)
            LV v {full_expr(*s.expr, out, s.expr->type.is_ptr())};
            assign(out, kResult, v.e);
        }
        // Leaving every open block that holds cells: innermost first (decision 0032 item 8), after the value and before the postconditions.
        for (size_t k {fn_.scopes.size()}; k-- > 0U;)
        {
            scope_exit(out, fn_.scopes[k].cells, s.loc);
        }
        if (fn_.f->noreturn)
        {
            // decision 0015 item 4: a function declared not to return must not return; a reachable `return` is the error.
            emit_assert(
                out, low::bool_lit(false),
                meta("noreturn", s.loc, "`" + fn_.f->c_name + "` is declared not to return, but it may return here"));
        }
        else
        {
            postconditions(s.loc, out);
        }
        low::LStmt g;
        g.kind = low::LStmt::Kind::Goto;
        g.name = kEndLabel;
        out.push_back(std::move(g));
    }

    static E amount_bounds(const std::string& name)
    {
        return low::bin(low::BinOp::And, low::bin(low::BinOp::Lt, low::no_perm(), low::var(name)),
                        low::bin(low::BinOp::Lt, low::var(name), low::write_perm()));
    }

    // A pointer the function obtained from elsewhere (a call's result, a parameter without an atom of its own) is some cell the function holds, but
    // Silicon finds the permission through a Ref only when it knows which one: a no-op `if (r == X) { }` for each cell X in scope and each pointer
    // parameter with an atom, of the same field, branches on it (decision 0037 item 3, the owner's choice after the exit 3 of `*pick(...)`).
    // The Refs a pointer to `target` may equal among the cells and slots of an object of type `t` at `slot` (decision 0043 item 9, 0037 item 3): the object itself or one
    // of its nested slots when `target` is a struct, every leaf cell of the target's field when it is not (the cell itself for a scalar object).
    void split_candidates(const E& slot, const Type& t, const Type& target, std::vector<E>& out)
    {
        if (t.is_struct())
        {
            if (target.is_struct())
            {
                for (const std::vector<int>& path : struct_slots_of(*t.struct_type, *target.struct_type))
                {
                    out.push_back(chain_ref(slot, path));
                }
            }
            else
            {
                const std::string sfx {cell_suffix(target)};
                for (const StructLeaf& l : struct_leaves(*t.struct_type))
                {
                    if (cell_suffix(l.type) == sfx)
                    {
                        out.push_back(chain_ref(slot, l.path));
                    }
                }
            }
        }
        else if (cell_suffix(t) == cell_suffix(target))
        {
            out.push_back(slot);
        }
    }

    // `args`: the call's own pointer arguments of the result's type (a call's own result only; audit round 2 S4 and round 3 B1): in a heap program a result the contract
    // ties to its arguments (`r == a || r == b`) is one of the blocks the caller passed, and Silicon cannot resolve which token a later `free(r)` reads
    // (`int *r = pick(p, q, c); free(r);` was an exit 3 also at 9255b93): one split of the result against each such argument.  Against every pointer local of the type it
    // was quadratic (k locals, m calls: a timeout on programs that verified before).
    void result_splits(const Type& ptr_type, const E& r, std::vector<low::LStmt>& out,
                       const std::vector<E>* args = nullptr)
    {
        if (ptr_type.pointee == nullptr || ptr_type.pointee->is_void())
        {
            return;
        }
        const Type& target {*ptr_type.pointee};
        const std::string sfx {cell_suffix(target)};
        auto split = [&](const E& x)
        {
            low::LStmt st;
            st.kind = low::LStmt::Kind::CaseSplit;
            st.expr = low::bin(low::BinOp::Eq, r, x);
            out.push_back(std::move(st));
        };
        for (const Fn::Scope& sc : fn_.scopes)
        {
            for (const Var* cell : sc.cells)
            {
                std::vector<E> cands;
                split_candidates(low::var(var_name(*cell)), cell->type, target, cands);
                for (const E& c : cands)
                {
                    split(c);
                }
            }
        }
        for (const WAtom* a : active_w_)
        {
            if (a->kind != AtomKind::Live && a->sfx == sfx)
            {
                split(a->qb); // the places of the enclosing loops' written atoms (decision 0040 item 5)
            }
        }
        if (heap_ && args != nullptr)
        {
            for (const E& x : *args)
            {
                split(x);
            }
        }
        const FnAtoms& fa {atoms_of(*fn_.f)};
        std::set<std::pair<int, std::vector<int>>> done;
        for (int i {0}; i < fa.nreq; ++i)
        {
            const AtomSite& site {fa.sites[static_cast<size_t>(i)]};
            if (!site_takes_cell(site))
            {
                continue;
            }
            std::vector<E> cands;
            split_candidates(site_var(site), site_pointee(site), target, cands);
            if (cands.empty() || !done.insert({site_key(site), site.path}).second)
            {
                continue;
            }
            for (const E& c : cands)
            {
                split(c);
            }
        }
        for (const LbPlace& l : active_lbs_)
        { // the places of the enclosing loops' loop-entry snapshots (decision 0045 item 10)
            std::vector<E> cands;
            split_candidates(l.ref, site_pointee(*l.site), target, cands);
            for (const E& c : cands)
            {
                split(c);
            }
        }
        // The entry function holds every global (program_start): a pointer it obtained may be any of them (decision 0041 item 10).
        if (fn_.f == prog_.entry())
        {
            for (const Global& g : prog_.globals())
            {
                // a struct global: its slots and leaf cells, as an object of its type (decision 0043 item 9)
                std::vector<E> cands;
                split_candidates(global_ref(g.id), g.type, target, cands);
                if (!cands.empty() && done.insert({kGlobalKeyBase + g.id, {}}).second)
                {
                    for (const E& c : cands)
                    {
                        split(c);
                    }
                }
            }
        }
    }

    // Decision 0045 item 6 (entry): right after the `requires`, the base of each loading `requires` place is snapshotted into a ghost `Ref` (`eb_N`) from the
    // `requires`-framed chain; the body, the entry splits and the call splits name the place by it (a read of the heap in a split condition would not be
    // well-defined where the inner cell is no longer held).
    void entry_snapshots(const Function& f, std::vector<low::LStmt>& out, const Env& env)
    {
        const FnAtoms& fa {atoms_of(f)};
        std::set<int> done;
        for (int i {0}; i < fa.nreq; ++i)
        {
            const AtomSite& s {fa.sites[static_cast<size_t>(i)]};
            if (s.via.empty())
            {
                continue;
            }
            const int k {site_key(s)};
            if (!is_derived_key(k) || !derived_of(k).has_load || !done.insert(k).second)
            {
                continue;
            }
            E chain;
            {
                CtxSave save(*this);
                cmode_ = CMode::Method;
                chain = site_root(s, env);
            }
            const Derived& d {derived_of(k)};
            fn_.snapshots.push_back(low::LVar {eb_name(d), low::LType::Ref});
            assign(out, eb_name(d), chain);
        }
    }

    // The entry of a method with atoms (decision 0037 item 9): the case splits on the liveness cells of every pair of atoms of the `requires`, and
    // the separation of every pair of `owns` atoms of one field under both guards (an `internal` row: provable from the `requires`).
    void entry_checks(const Function& f, std::vector<low::LStmt>& out)
    {
        const FnAtoms& fa {atoms_of(f)};
        const size_t n {static_cast<size_t>(fa.nreq)};
        if (n == 0U)
        {
            return;
        }
        const Env env {contract_env()};
        std::vector<E> g;
        for (size_t i {0U}; i < n; ++i)
        {
            g.push_back(site_guard(fa.sites[i], env));
        }
        auto p = [&](size_t i)
        {
            return site_ref(fa.sites[i], env);
        };
        auto root = [&](size_t i)
        {
            return site_root(fa.sites[i], env);
        };
        for (size_t i {0U}; i < n; ++i)
        {
            for (size_t j {i + 1U}; j < n; ++j)
            {
                if (site_key(fa.sites[i]) == site_key(fa.sites[j]))
                {
                    continue;
                }
                if (site_is_global(fa.sites[i]) && site_is_global(fa.sites[j]))
                {
                    continue; // two globals are two `unique` blocks
                }
                low::LStmt st;
                st.kind = low::LStmt::Kind::CaseSplit;
                st.expr = low::bin(low::BinOp::Eq, live_cell(root(i)), live_cell(root(j)));
                out.push_back(std::move(st));
            }
        }
        // A pointer parameter without an atom of its own may be the place an atom names: split on each one of the same field (a slot, for a struct pointer).
        std::set<int> with_atom;
        for (size_t i {0U}; i < n; ++i)
        {
            with_atom.insert(site_key(fa.sites[i]));
        }
        for (const AtomSite& bs : blocks_of(f).sites)
        {
            if (!bs.in_ensures)
            {
                // a pointer that owns its block needs no split against another's cells (decision 0047 item 4)
                with_atom.insert(block_key(bs));
            }
        }
        for (size_t k {0U}; k < f.params.size(); ++k)
        {
            const Type& t {f.params[k].type};
            if (!t.is_ptr() || with_atom.count(static_cast<int>(k)) != 0U || t.pointee == nullptr ||
                t.pointee->is_void())
            {
                continue;
            }
            std::set<std::pair<int, std::vector<int>>> seen;
            for (size_t i {0U}; i < n; ++i)
            {
                const AtomSite& a {fa.sites[i]};
                if (!site_takes_cell(a))
                {
                    continue;
                }
                std::vector<E> cands;
                split_candidates(p(i), site_pointee(a), *t.pointee, cands);
                if (cands.empty() || !seen.insert({site_key(a), a.path}).second)
                {
                    continue;
                }
                for (const E& c : cands)
                {
                    low::LStmt st;
                    st.kind = low::LStmt::Kind::CaseSplit;
                    st.expr = low::bin(low::BinOp::Eq, env.formals[k], c);
                    out.push_back(std::move(st));
                }
            }
        }
        for (size_t i {0U}; i < n; ++i)
        {
            for (size_t j {i + 1U}; j < n; ++j)
            {
                const AtomSite& a {fa.sites[i]};
                const AtomSite& b {fa.sites[j]};
                if (a.atom->atom != AtomKind::Owns || b.atom->atom != AtomKind::Owns || site_key(a) == site_key(b))
                {
                    continue;
                }
                if (site_is_global(a) && site_is_global(b))
                {
                    continue;
                }
                if (!shares_field(a, b))
                {
                    continue;
                }
                for (const auto& [x, y] : overlap_pairs(a, p(i), b, p(j)))
                {
                    emit_assert(
                        out, low::implies(low::and_(g[i], g[j]), low::bin(low::BinOp::Ne, x, y)),
                        meta("internal", fn_.f->loc,
                             "internal check: two places the contract of `" + fn_.f->c_name + "` owns are different"));
                }
            }
        }
    }

    // The `ensures` atoms at a return (decision 0037 item 10): the separation of the atoms that must not share a cell, then each atom under its
    // guard, part by part, with the sums of item 6; the facts about WF and the block's length as internal rows.
    void atom_returns(const SrcLoc& at, std::vector<low::LStmt>& out, const Env& env)
    {
        using low::BinOp;
        const Function& f {*fn_.f};
        const FnAtoms& fa {atoms_of(f)};
        std::vector<size_t> ens;
        for (size_t i {0U}; i < fa.sites.size(); ++i)
        {
            if (fa.sites[i].in_ensures)
            {
                ens.push_back(i);
            }
        }
        if (ens.empty())
        {
            return;
        }
        // The places of the `ensures` are read in the return heap (decision 0045 item 3): a place reached through a load is the object its path designates on return.
        CtxSave ret_ctx(*this);
        cmode_ = CMode::Ret;
        cf_ = &f;
        cens_ = true;
        cold_ = false;
        std::vector<E> g;
        std::vector<E> p;
        std::vector<E> root;
        for (size_t i : ens)
        {
            g.push_back(site_guard(fa.sites[i], env));
            root.push_back(site_root(fa.sites[i], env));
            p.push_back(chain_ref(root.back(), fa.sites[i].path));
        }
        auto loads = [&](size_t k)
        {
            const AtomSite& x {fa.sites[ens[k]]};
            return !x.via.empty() && is_derived_key(site_key(x)) && derived_of(site_key(x)).has_load;
        };
        auto amt_of = [&](size_t k)
        {
            return ens_amount(
                fa, fa.sites[ens[k]], p[k],
                [&](const AtomSite& rs)
                {
                    return site_var(rs);
                },
                env);
        };
        // the user's spelling of the place, for messages
        auto name = [&](size_t k)
        {
            return site_text(fa.sites[ens[k]]);
        };
        auto perm_meta = [&](const std::string& message, size_t k)
        {
            low::Meta m {meta("permission", at, message)};
            const Clause& cl {f.ensures_[static_cast<size_t>(fa.sites[ens[k]].clause)]};
            if (cl.loc.known())
            {
                m.has_note = true;
                m.note_loc = cl.loc;
                m.note_text = "this is the postcondition";
            }
            return m;
        };
        auto internal = [&](const std::string& what)
        {
            return meta("internal", at, "internal check: " + what);
        };
        // The same with a category of its own, for `must_init`'s promise (decision 0042 item 5); the note points at the postcondition like perm_meta's.
        auto cat_meta = [&](const char* category, const std::string& message, size_t k)
        {
            low::Meta m {perm_meta(message, k)};
            m.category = category;
            return m;
        };
        const E none {low::no_perm()};
        const E write {low::write_perm()};
        // Pass 0 (decision 0045 item 6, audit S3): when an `ensures` place is reached through a load, or an `ensures` guard reads the heap, each earlier atom that holds the
        // cells they read is checked held first, in clause order (a pointer cell for a load, any cell for a guard), so that no later row reads through a load before the
        // row that checks the cell it reads (else Silicon's own well-definedness check would fail first).
        {
            std::set<std::pair<size_t, size_t>> asserted; // (atom, cell) already checked
            for (size_t k {0U}; k < ens.size(); ++k)
            {
                bool guard_reads {false};
                for (const AtomGuard& x : fa.sites[ens[k]].guards)
                {
                    guard_reads = guard_reads || reads_heap(*x.cond);
                }
                if (!loads(k) && !guard_reads)
                {
                    continue;
                }
                for (size_t j {0U}; j < k; ++j)
                {
                    const AtomSite& fj {fa.sites[ens[j]]};
                    if (!site_takes_cell(fj))
                    {
                        continue;
                    }
                    const std::vector<Cell> cs {site_cells(fj, p[j])};
                    const bool owns_j {fj.atom->atom == AtomKind::Owns};
                    const std::string plj {site_phrase(fj, quoted(name(j), "the object the pointer points to"))};
                    const std::string mj {owns_j ? "returns without owning " + plj + ", which its contract promises"
                                                 : "returns without the permission to read " + plj +
                                                       ", which it borrowed"};
                    for (size_t ci {0U}; ci < cs.size(); ++ci)
                    {
                        const Cell& c {cs[ci]};
                        if (!(guard_reads || c.type.is_ptr()) || !asserted.insert({j, ci}).second)
                        {
                            continue;
                        }
                        emit_assert(out, low::implies(g[j], low::bin(BinOp::Gt, value_perm(c.recv, c.type), none)),
                                    perm_meta(mj, j));
                    }
                }
            }
        }
        for (size_t a {0U}; a < ens.size(); ++a)
        {
            for (size_t b {a + 1U}; b < ens.size(); ++b)
            {
                if (!exclusive_pair(fa.sites[ens[a]], fa.sites[ens[b]]))
                {
                    continue;
                }
                if (same_expr(p[a], p[b]))
                {
                    // one parameter's owns and reads: `f__wf` asserts their guards exclude each other (decision 0039
                    // item 7)
                    continue;
                }
                if (site_key(fa.sites[ens[a]]) == site_key(fa.sites[ens[b]]))
                { // two member places of one root are two cells (nested ones are the checker's)
                    if (paths_nested(fa.sites[ens[a]].path, fa.sites[ens[b]].path))
                    {
                        internal_error(at, "two permission atoms of one root, one inside the other, reached lowering");
                    }
                    continue;
                }
                for (const auto& [x, y] : overlap_pairs(fa.sites[ens[a]], p[a], fa.sites[ens[b]], p[b]))
                {
                    emit_assert(out, low::implies(low::and_(g[a], g[b]), low::bin(BinOp::Ne, x, y)),
                                perm_meta("returns " + quoted(name(a), "the object the pointer points to") + " and " +
                                              quoted(name(b), "the object the pointer points to") +
                                              " as owned places, but they may be the same object",
                                          a));
                }
            }
        }
        auto term = [&](const E& cond, const E& amount)
        {
            return cond ? low::ite(cond, amount, none) : amount;
        };
        for (size_t k {0U}; k < ens.size(); ++k)
        {
            const AtomSite& site {fa.sites[ens[k]]};
            const Type& t {site_pointee(site)};
            const E amt {amt_of(k)};
            const bool owns {site.atom->atom == AtomKind::Owns};
            const bool reads {site.atom->atom == AtomKind::Reads};
            const std::string pl {site_phrase(site, quoted(name(k), "the object the pointer points to"))};
            const std::string msg {owns    ? "returns without owning " + pl + ", which its contract promises"
                                   : reads ? "returns without the permission to read " + pl + ", which it borrowed"
                                           : "returns without " + quoted(site.atom->a->c_text, "the pointer") +
                                                 " pointing to a live object, which its contract promises"};
            auto add = [&](E cond, const low::Meta& m)
            {
                emit_assert(out, low::implies(g[k], std::move(cond)), m);
            };
            const Type& root_ptr {member_root(site.atom->a)->type};
            add(low::bin(BinOp::Ne, root[k], low::null_()), perm_meta(msg, k));
            add(wf_expr(root[k], &root_ptr), internal("WF of a pointer parameter"));
            // A pointer loaded from memory need not point into an object the function owns (a callee that moves `*pp` to a pointer it was not given an object for): its row is a user's, not internal.
            if (needs_root_block(site))
            {
                add(in_block(root[k]),
                    loads(k) ? perm_meta(msg, k) : internal("a pointer parameter points into its block"));
            }
            if (site_takes_cell(site))
            {
                // Decision 0042 item 5.  `must_init`: the flag row is the promise (`postcondition`) and the range holds under it, so a missing write is reported
                // once.  `ref_to_uninit`: a flag that was set on entry is still set (`internal`: nothing makes a held cell indeterminate again), and the range holds
                // under the flag.
                const InitMarker mk {site_marker(f, site)};
                for (const Cell& c : site_cells(site, p[k]))
                {
                    if (owns)
                    {
                        add(low::bin(BinOp::Eq, value_perm(c.recv, c.type), write), perm_meta(msg, k));
                        add(low::bin(BinOp::Eq, flag_perm(c.recv, c.type), write), perm_meta(msg, k));
                    }
                    else
                    {
                        for (const bool flag : {false, true})
                        {
                            E total {amt};
                            for (size_t j {0U}; j < ens.size(); ++j)
                            {
                                if (j == k || fa.sites[ens[j]].atom->atom != AtomKind::Reads)
                                {
                                    continue;
                                }
                                const bool one_type {site_pointee(fa.sites[ens[j]]) == t && t.is_struct()};
                                for (const Cell& cj : site_cells(fa.sites[ens[j]], p[j]))
                                {
                                    if (cell_suffix(cj.type) != cell_suffix(c.type))
                                    {
                                        continue;
                                    }
                                    if (one_type && cj.path != c.path)
                                    {
                                        // leaves of two objects of one struct type share a cell only at one path
                                        continue;
                                    }
                                    E same {same_expr(c.recv, cj.recv) ? nullptr
                                                                       : low::bin(BinOp::Eq, c.recv, cj.recv)};
                                    total = low::bin(BinOp::Add, total, term(low::and_(g[j], same), amt_of(j)));
                                }
                            }
                            add(low::bin(BinOp::Ge, flag ? flag_perm(c.recv, c.type) : value_perm(c.recv, c.type),
                                         total),
                                perm_meta(msg, k));
                        }
                    }
                    if (mk == InitMarker::None)
                    {
                        add(cell_flag(c.recv, c.type), perm_meta(msg, k));
                        if (const E r = cell_range(c.recv, c.type))
                        {
                            add(r, perm_meta(msg, k));
                        }
                    }
                    else
                    {
                        if (mk == InitMarker::MustInit)
                        {
                            const std::string& pname {f.params[static_cast<size_t>(site_key(site))].name};
                            add(cell_flag(c.recv, c.type),
                                cat_meta(
                                    "postcondition",
                                    "returns without initializing " +
                                        quoted(t.is_struct() ? c.text : name(k), "the object the pointer points to") +
                                        ", which `cerastes::must_init` on " +
                                        (pname.empty() ? std::string("the parameter") : "`" + pname + "`") +
                                        " promises",
                                    k));
                        }
                        else
                        {
                            E kept {low::old_(cell_flag(c.recv, c.type))};
                            if (site.atom->amount >= 0 && site.atom->amount < fa.nreq)
                            {
                                if (const E gp = site_guard(fa.sites[static_cast<size_t>(site.atom->amount)], env))
                                {
                                    // the partner's guard in the entry state (audit B2)
                                    kept = low::and_(old_entry(gp), kept);
                                }
                            }
                            add(low::implies(kept, cell_flag(c.recv, c.type)),
                                internal("a pointee that had a value on entry keeps it"));
                        }
                        if (const E r = cell_range(c.recv, c.type))
                        {
                            add(low::implies(cell_flag(c.recv, c.type), r), perm_meta(msg, k));
                        }
                    }
                }
            }
            E total {amt};
            for (size_t j {0U}; j < ens.size(); ++j)
            {
                if (j == k)
                {
                    continue;
                }
                E same {same_expr(root[k], root[j]) ? nullptr
                                                    : low::bin(BinOp::Eq, live_cell(root[k]), live_cell(root[j]))};
                total = low::bin(BinOp::Add, total, term(low::and_(g[j], same), amt_of(j)));
            }
            add(low::bin(BinOp::Ge, live_perm(root[k]), total), perm_meta(msg, k));
        }
    }

    // The exemption of the leak row (item 7): the tokens a function hands back, `g && t == loc(block_of(q), -1)` for each `ensures` block site (`g` its guard as written, `q`
    // its pointer: the result, or a borrowed formal, which never changes), or `false` without one.
    E leak_exemption(const Function& f, const Env& env)
    {
        E r;
        CtxSave save(*this);
        for (const AtomSite& s : blocks_of(f).sites)
        {
            if (!s.in_ensures)
            {
                continue;
            }
            E term {low::bin(low::BinOp::Eq, low::var("t"), tok_slot(block_ptr(s, env)))};
            if (const E g = site_guard(s, env))
            {
                term = low::and_(g, term);
            }
            r = r ? low::bin(low::BinOp::Or, r, term) : term;
        }
        return r ? r : low::bool_lit(false);
    }
    // The one leak row of a method (item 7), after the end label where every return and the fall-through land.
    void leak_row(const Function& f, std::vector<low::LStmt>& out)
    {
        if (!heap_ || f.noreturn)
        {
            return;
        }
        const Env env {contract_env()};
        emit_assert(out, low::for_perm(leak_exemption(f, env)),
                    meta("leak", f.end_loc,
                         "`" + f.c_name +
                             "` may return while it still owns an allocation that its `ensures` does not hand back"));
    }

    // The method entry's distinctness of the blocks (item 4, an `internal` row: provable from the `requires`, as Silicon derives it when two full liveness cells are inhaled).
    void block_entry_checks(const Function& f, std::vector<low::LStmt>& out)
    {
        const FnBlocks& fb {blocks_of(f)};
        if (fb.nreq == 0)
        {
            return;
        }
        const Env env {contract_env()};
        const FnAtoms& fa {atoms_of(f)};
        std::vector<E> g;
        std::vector<E> ptr;
        for (int i {0}; i < fb.nreq; ++i)
        {
            g.push_back(site_guard(fb.sites[static_cast<size_t>(i)], env));
            ptr.push_back(block_ptr(fb.sites[static_cast<size_t>(i)], env));
        }
        for (size_t i {0U}; i < g.size(); ++i)
        {
            for (size_t j {i + 1U}; j < g.size(); ++j)
            {
                emit_assert(
                    out,
                    low::implies(low::and_(g[i], g[j]), low::bin(low::BinOp::Ne, live_cell(ptr[i]), live_cell(ptr[j]))),
                    meta("internal", f.loc,
                         "internal check: two blocks the contract of `" + f.c_name + "` owns are different"));
            }
            for (int j {0}; j < fa.nreq; ++j)
            {
                const AtomSite& o {fa.sites[static_cast<size_t>(j)]};
                emit_assert(
                    out,
                    low::implies(low::and_(g[i], site_guard(o, env)),
                                 low::bin(low::BinOp::Ne, live_cell(ptr[i]), live_cell(site_root(o, env)))),
                    meta("internal", f.loc,
                         "internal check: a block and a place the contract of `" + f.c_name + "` owns are different"));
            }
        }
    }

    // The rows of the `ensures` block sites at a return (item 4; 0037 item 10), after atom_returns' rows: the separation of every `ensures` block site from every other
    // `ensures` site (`lvloc` rows, audit S4: `return p` with owns_block(p) and owns_block(r) fails here, never in Silicon's own postcondition check), then each site's rows.
    void block_returns(const SrcLoc& at, std::vector<low::LStmt>& out, const Env& env)
    {
        const Function& f {*fn_.f};
        const FnBlocks& fb {blocks_of(f)};
        const FnAtoms& fa {atoms_of(f)};
        struct R
        {
            const AtomSite* site;
            E ptr, g;
        };
        std::vector<R> rs;
        CtxSave ret_ctx(*this);
        cmode_ = CMode::Ret;
        cf_ = &f;
        cens_ = true;
        cold_ = false;
        for (size_t i {static_cast<size_t>(fb.nreq)}; i < fb.sites.size(); ++i)
        {
            const AtomSite& s {fb.sites[i]};
            rs.push_back(R {&s, block_ptr(s, env), site_guard(s, env)});
        }
        if (rs.empty())
        {
            return;
        }
        auto note = [&](low::Meta m, const AtomSite& s)
        {
            const Clause& cl {f.ensures_[static_cast<size_t>(s.clause)]};
            if (cl.loc.known())
            {
                m.has_note = true;
                m.note_loc = cl.loc;
                m.note_text = "this is the postcondition";
            }
            return m;
        };
        auto name = [&](const R& r)
        {
            return r.site->atom->a->c_text.empty() ? std::string("a pointer") : "`" + r.site->atom->a->c_text + "`";
        };
        for (size_t i {0U}; i < rs.size(); ++i)
        {
            for (size_t j {i + 1U}; j < rs.size(); ++j)
            {
                emit_assert(out,
                            low::implies(low::and_(rs[i].g, rs[j].g),
                                         low::bin(low::BinOp::Ne, live_cell(rs[i].ptr), live_cell(rs[j].ptr))),
                            note(meta("permission", at,
                                      "returns " + name(rs[i]) + " and " + name(rs[j]) +
                                          " as owned allocations, but they may be the same object"),
                                 *rs[i].site));
            }
            for (const AtomSite& o : fa.sites)
            {
                if (!o.in_ensures)
                {
                    continue;
                }
                emit_assert(out,
                            low::implies(low::and_(rs[i].g, site_guard(o, env)),
                                         low::bin(low::BinOp::Ne, live_cell(rs[i].ptr), live_cell(site_root(o, env)))),
                            note(meta("permission", at,
                                      "returns " + name(rs[i]) + " as an owned allocation and " +
                                          quoted(site_text(o), "another object") +
                                          " as an owned place, but they may be the same object"),
                                 *rs[i].site));
            }
        }
        for (const R& r : rs)
        {
            block_rows(
                r.ptr, block_ptr_type(*r.site), r.site->atom->a->c_text, block_mode(f, *r.site), false,
                [&](const char* category, E cond, const std::string& message)
                {
                    emit_assert(out, low::implies(r.g, std::move(cond)), note(meta(category, at, message), *r.site));
                },
                std::string(), true);
        }
    }

    // Assert each user `ensures` at the point where the function returns (0012 item 9), after the permissions it promises (decision 0037 item 10).
    void postconditions(const SrcLoc& at, std::vector<low::LStmt>& out)
    {
        const Function& f {*fn_.f};
        const Env env {contract_env()};
        atom_returns(at, out, env);
        block_returns(at, out, env); // decision 0047 item 4: the `owns_block` sites of the `ensures`
        if (f.ret.is_ptr())
        {
            emit_assert(out, wf_expr(env.result, &f.ret),
                        meta("internal", at, "internal check: the pointer a function returns is well-formed"));
        }
        if (f.ret.is_struct())
        {
            const std::vector<StructLeaf> ls {struct_leaves(*f.ret.struct_type)};
            for (size_t i {0U}; i < ls.size(); ++i)
            {
                if (ls[i].type.is_ptr())
                {
                    emit_assert(out, wf_expr(env.result_leaves[i], &ls[i].type),
                                meta("internal", at, "internal check: the pointer a function returns is well-formed"));
                }
            }
        }
        for (size_t ci {0U}; ci < f.ensures_.size(); ++ci)
        {
            const Clause& c {f.ensures_[ci]};
            Full scratch;
            LV v;
            {
                CtxSave save(*this);
                set_ctx(f, CMode::Ret, true, static_cast<int>(ci));
                v = lower(*c.expr, env, nullptr, {}, scratch);
            }
            pure_full(scratch, *c.expr, "a postcondition");
            if (has_atom(*c.expr) && always_true(v.e))
            {
                continue; // nothing but atoms: asserted above
            }
            low::Meta m {meta("postcondition", at, "the postcondition of `" + f.c_name + "` may not hold")};
            m.has_note = true;
            m.note_loc = c.loc;
            m.note_text = "this is the postcondition";
            emit_assert(out, v.e, std::move(m));
        }
    }

    // ---- loops over memory (decision 0038) ----

    // Whether an expression or a statement touches memory: a pointer-typed node, a cell, `&`, `*` or a store (decision 0038 item 3).
    bool expr_touches(const Expr* e)
    {
        if (e == nullptr)
        {
            return false;
        }
        if (e->type.is_ptr() || (e->type.is_struct() && has_pointer_leaf(e->type)))
        {
            return true; // a struct value with a pointer leaf carries pointers (decision 0044)
        }
        // A call whose callee has an atom in its `requires` touches memory even with no pointer argument: an atom on a global or an implicit `static`
        // local's (decision 0041; audit SF1).  A callee with a parameter atom has a pointer argument, so nothing changes for programs without globals.
        if (e->kind == Expr::Kind::Call && e->callee != nullptr && requires_atom_count(*e->callee) > 0)
        {
            return true;
        }
        switch (e->kind)
        {
            case Expr::Kind::Local:
                if (e->var != nullptr && (e->var->address_taken || e->var->type.is_ptr()))
                {
                    return true;
                }
                break;
            case Expr::Kind::AddrOf:
            case Expr::Kind::Deref:
            case Expr::Kind::Atom:
            case Expr::Kind::StoreE:
                return true;
            case Expr::Kind::AssignE:
                if (e->var != nullptr && e->var->address_taken)
                {
                    return true;
                }
                break; // a cell written inside an expression (0039 item 2)
            default:
                break;
        }
        for (const Expr* x : {e->a, e->b, e->c})
        {
            if (expr_touches(x))
            {
                return true;
            }
        }
        for (const Expr* x : e->args)
        {
            if (expr_touches(x))
            {
                return true;
            }
        }
        return false;
    }

    bool stmt_touches(const Stmt* s)
    {
        if (s == nullptr)
        {
            return false;
        }
        switch (s->kind)
        {
            case Stmt::Kind::Store:
                return true;
            case Stmt::Kind::Decl:
            case Stmt::Kind::Assign:
                if (s->var != nullptr && (s->var->address_taken || s->var->type.is_ptr()))
                {
                    return true;
                }
                return expr_touches(s->expr);
            case Stmt::Kind::If:
                return expr_touches(s->expr) || stmt_touches(s->then_body) || stmt_touches(s->else_body);
            case Stmt::Kind::While:
            {
                if (expr_touches(s->expr) || stmt_touches(s->body) || stmt_touches(s->step))
                {
                    return true;
                }
                for (const Clause& c : s->invariants)
                {
                    if (expr_touches(c.expr))
                    {
                        return true;
                    }
                }
                for (const Clause& c : s->measure)
                {
                    if (expr_touches(c.expr))
                    {
                        return true;
                    }
                }
                return false;
            }
            case Stmt::Kind::Switch:
                return expr_touches(s->expr) || stmt_touches(s->body);
            case Stmt::Kind::Block:
                for (const Stmt* c : s->stmts)
                {
                    if (stmt_touches(c))
                    {
                        return true;
                    }
                }
                return false;
            case Stmt::Kind::Case:
            case Stmt::Kind::Break:
            case Stmt::Kind::Continue:
            case Stmt::Kind::Goto:
            case Stmt::Kind::Label:
                return false;
            default:
                return expr_touches(s->expr); // ExprStmt, Return, Check
        }
    }

    // The `return`s and `goto`s of a statement, and the labels in it (decision 0038 item 3: a `return`, or a `goto` to a label outside the loop).
    static void exits_of(const Stmt* s, bool& has_return, std::set<int>& gotos, std::set<int>& labels)
    {
        if (s == nullptr)
        {
            return;
        }
        switch (s->kind)
        {
            case Stmt::Kind::Return:
                has_return = true;
                break;
            case Stmt::Kind::Goto:
                gotos.insert(s->jump_id);
                break;
            case Stmt::Kind::Label:
                labels.insert(s->jump_id);
                break;
            case Stmt::Kind::If:
                exits_of(s->then_body, has_return, gotos, labels);
                exits_of(s->else_body, has_return, gotos, labels);
                break;
            case Stmt::Kind::While:
                exits_of(s->body, has_return, gotos, labels);
                exits_of(s->step, has_return, gotos, labels);
                break;
            case Stmt::Kind::Switch:
                exits_of(s->body, has_return, gotos, labels);
                break;
            case Stmt::Kind::Block:
                for (const Stmt* c : s->stmts)
                {
                    exits_of(c, has_return, gotos, labels);
                }
                break;
            default:
                break;
        }
    }

    // The writes a loop can make to memory (decision 0038 item 3): the cells it assigns by name, and the cell fields a store through a pointer, or a
    // call of a function that owns a cell of that field (Viper havocs what a callee owns), can write.
    struct LoopWrites
    {
        std::set<std::string> fields;
        std::set<int> cells;
        // the globals (ids) written by name, or owned by a callee's atom (decision 0041 item 10): a write through any
        // other pointer is by field
        std::set<int> globals;
        // A member of a struct global written by name (`g.x = e`, `g.t = f()`; decision 0044 item 8): (global id, the leaf's member path from the global's slot).  The
        // address is a fixed term (`loc(member(...loc(GB_g(), 0)...), 0)`, no load), so only the place of that very leaf is reached; every other automatic place is another
        // leaf (distinct member blocks), another global (`unique` blocks) or an object of another block, and a write through a pointer is `fields` as before.
        std::set<std::pair<int, std::vector<int>>> gleaves;
    };

    // The function holds the leaf `path` of the global `id` as one of its own automatic places (decision 0044 item 8): it is the entry function (every global is held from
    // program_start), or a `requires` atom of this function names that global at a path that is a prefix of the leaf's (`owns(g)`, `owns(g.t)`, `owns(g.t.y)`).
    bool holds_global_leaf(int id, const std::vector<int>& path)
    {
        if (fn_.f == prog_.entry())
        {
            return true;
        }
        const FnAtoms& fa {atoms_of(*fn_.f)};
        for (int i {0}; i < fa.nreq; ++i)
        {
            const AtomSite& site {fa.sites[static_cast<size_t>(i)]};
            if (site_key(site) != kGlobalKeyBase + id || !site_takes_cell(site))
            {
                continue;
            }
            if (site.path.size() <= path.size() && std::equal(site.path.begin(), site.path.end(), path.begin()))
            {
                return true;
            }
        }
        return false;
    }

    // The address `addr` is a member chain over a global (`&g.x`, `&g.t`, `&g.t.y`; no load on the way, member_root): a store of a value of type `stored` through it writes
    // exactly the leaves at and below that member of that global.  When the function holds such a leaf as its own place (holds_global_leaf), only that place is reached and
    // it is recorded in `w.gleaves`; otherwise the global's cell may also be held through a pointer parameter (`owns(*s) && s == &g`), so the write is field-wide as a
    // write through a pointer is: the leaf's field goes to `w.fields`.  False for every other address.
    bool note_global_member_write(const Expr& addr, const Type& stored, LoopWrites& w)
    {
        if (addr.kind != Expr::Kind::MemberAddr)
        {
            return false;
        }
        std::vector<int> path;
        const Expr* root {member_root(&addr, &path)};
        if (root == nullptr || root->kind != Expr::Kind::GlobalAddr || root->global == nullptr)
        {
            return false;
        }
        const int id {root->global->id};
        auto note = [&](std::vector<int> full, const Type& t)
        {
            if (holds_global_leaf(id, full))
            {
                w.gleaves.insert({id, std::move(full)});
            }
            else
            {
                w.fields.insert(cell_suffix(t));
            }
        };
        if (stored.is_struct())
        {
            for (const StructLeaf& l : struct_leaves(*stored.struct_type))
            {
                std::vector<int> full {path};
                full.insert(full.end(), l.path.begin(), l.path.end());
                note(std::move(full), l.type);
            }
        }
        else
        {
            note(std::move(path), stored);
        }
        return true;
    }

    void writes_in_expr(const Expr* e, LoopWrites& w)
    {
        if (e == nullptr)
        {
            return;
        }
        if (e->kind == Expr::Kind::AssignE && e->var != nullptr && e->var->address_taken)
        {
            w.cells.insert(e->var->id); // decision 0039 item 2
        }
        if (e->kind == Expr::Kind::StoreE)
        {
            if (e->c != nullptr && is_global_addr(*e->c))
            {
                w.globals.insert(e->c->global->id);
            }
            else if (e->c != nullptr && note_global_member_write(*e->c, e->type, w))
            { // by name: exactly that leaf (decision 0044 item 8)
            }
            else if (e->type.is_struct())
            { // decision 0044 item 7
                for (const StructLeaf& l : struct_leaves(*e->type.struct_type))
                {
                    w.fields.insert(cell_suffix(l.type));
                }
            }
            else
            {
                w.fields.insert(cell_suffix(e->type));
            }
        }
        if (e->kind == Expr::Kind::Call && e->callee != nullptr)
        {
            for (const AtomSite& site : atoms_of(*e->callee).sites)
            {
                if (site.in_ensures || site.atom->atom != AtomKind::Owns)
                {
                    continue;
                }
                if (site_is_global(site))
                {
                    w.globals.insert(site_key(site) - kGlobalKeyBase);
                }
                else
                {
                    for (const std::string& f : site_fields(site))
                    {
                        w.fields.insert(f); // every leaf field of a struct atom (decision 0043 item 11)
                    }
                }
            }
        }
        // decision 0047 item 10: a `requires` block site owns every leaf cell of its block, so the callee may write
        // them
        if (e->kind == Expr::Kind::Call && e->callee != nullptr)
        {
            for (const AtomSite& site : blocks_of(*e->callee).sites)
            {
                if (site.in_ensures)
                {
                    continue;
                }
                const Type& t {block_pointee(site)};
                if (t.is_struct())
                {
                    for (const StructLeaf& l : struct_leaves(*t.struct_type))
                    {
                        w.fields.insert(cell_suffix(l.type));
                    }
                }
                else
                {
                    w.fields.insert(cell_suffix(t));
                }
            }
        }
        for (const Expr* x : {e->a, e->b, e->c})
        {
            writes_in_expr(x, w);
        }
        for (const Expr* x : e->args)
        {
            writes_in_expr(x, w);
        }
    }

    void writes_in(const Stmt* s, LoopWrites& w)
    {
        if (s == nullptr)
        {
            return;
        }
        switch (s->kind)
        {
            case Stmt::Kind::Store:
                if (s->addr != nullptr && is_global_addr(*s->addr))
                {
                    w.globals.insert(s->addr->global->id);
                }
                else if (s->addr != nullptr && s->expr != nullptr &&
                         note_global_member_write(*s->addr, s->expr->type, w))
                { // by name: exactly that leaf (decision 0044 item 8)
                }
                else if (s->addr != nullptr && s->addr->type.pointee != nullptr && s->addr->type.pointee->is_struct())
                { // decision 0044 item 5: a struct store writes every leaf field
                    for (const StructLeaf& l : struct_leaves(*s->addr->type.pointee->struct_type))
                    {
                        w.fields.insert(cell_suffix(l.type));
                    }
                }
                else if (s->addr != nullptr && s->addr->type.pointee != nullptr)
                {
                    w.fields.insert(cell_suffix(*s->addr->type.pointee));
                }
                writes_in_expr(s->addr, w);
                writes_in_expr(s->expr, w);
                break;
            case Stmt::Kind::Decl:
            case Stmt::Kind::Assign:
                if (s->var != nullptr && s->var->address_taken && s->expr != nullptr)
                {
                    w.cells.insert(s->var->id);
                }
                writes_in_expr(s->expr, w);
                break;
            case Stmt::Kind::If:
                writes_in_expr(s->expr, w);
                writes_in(s->then_body, w);
                writes_in(s->else_body, w);
                break;
            case Stmt::Kind::While:
                writes_in_expr(s->expr, w);
                writes_in(s->body, w);
                writes_in(s->step, w);
                break;
            case Stmt::Kind::Switch:
                writes_in_expr(s->expr, w);
                writes_in(s->body, w);
                break;
            case Stmt::Kind::Block:
                for (const Stmt* c : s->stmts)
                {
                    writes_in(c, w);
                }
                break;
            case Stmt::Kind::Case:
            case Stmt::Kind::Break:
            case Stmt::Kind::Continue:
            case Stmt::Kind::Goto:
            case Stmt::Kind::Label:
                break;
            default:
                writes_in_expr(s->expr, w);
                break; // ExprStmt, Return, Check
        }
    }

    // Whether `site` (a `requires` atom) names the place key `k`: its own key, or the loop-entry snapshot of its derived key.
    bool key_has_site(const AtomSite& site, int k) const
    {
        const int sk {site_key(site)};
        return sk == k || (is_derived_key(k) && derived_of(k).lb_of >= 0 && derived_of(k).lb_of == sk);
    }
    // The loop-entry snapshot key of the derived key `k` (created the first time).
    int lb_key_of(int k)
    {
        const auto it {lb_keys_.find(k)};
        if (it != lb_keys_.end())
        {
            return it->second;
        }
        Derived d {derived_of(k)};
        d.lb_of = k;
        d.local = derived_count_[d.f]++;
        derived_.push_back(d);
        const int lk {kDerivedKeyBase + static_cast<int>(derived_.size()) - 1};
        lb_keys_[k] = lk;
        return lk;
    }
    // Decision 0045 item 10: a derived place needs a loop-entry snapshot where the function may write a cell of a loaded field of its path anywhere in its body (a
    // store or a call reaching that field, by the havoc rule), a cell by name, or the global its root is: what the path designates when the loop starts may differ from
    // what the `requires` named.
    bool need_lb(int k)
    {
        const Derived& d {derived_of(k)};
        if (!d.has_load || fn_.f->body == nullptr)
        {
            return false;
        }
        LoopWrites w;
        writes_in(fn_.f->body, w);
        if (!w.cells.empty())
        {
            return true;
        }
        if (key_is_global(d.key.root) && w.globals.count(d.key.root - kGlobalKeyBase) != 0U)
        {
            return true;
        }
        if (key_is_global(d.key.root))
        {
            for (const auto& gl : w.gleaves)
            {
                if (gl.first == d.key.root - kGlobalKeyBase)
                {
                    return true;
                }
            }
        }
        for (const Expr* x {d.base}; x != nullptr; x = x->a)
        {
            if (x->kind == Expr::Kind::Deref && w.fields.count(cell_suffix(x->type)) != 0U)
            {
                return true;
            }
            if (x->kind != Expr::Kind::Deref && x->kind != Expr::Kind::MemberAddr && x->kind != Expr::Kind::Field)
            {
                break;
            }
        }
        return false;
    }
    // A base expression read in the CURRENT heap with no framing, and the condition (accumulated in `cond`) that every load on the way is held (the loop-entry snapshot).
    E lb_chain(const Expr* e, E& cond)
    {
        switch (e->kind)
        {
            case Expr::Kind::Formal:
                return contract_env().formals[static_cast<size_t>(e->index)];
            case Expr::Kind::GlobalAddr:
                return global_ref(e->global->id);
            case Expr::Kind::MemberAddr:
                return member_ref(lb_chain(e->a, cond), e->index);
            case Expr::Kind::Deref:
            {
                const E a {lb_chain(e->a, cond)};
                cond = low::and_(cond, low::bin(low::BinOp::Gt, value_perm(a, e->type), low::no_perm()));
                return cell_value(a, e->type);
            }
            default:
            {
                CtxSave save(*this);
                cmode_ = CMode::Wf; // a by-value parameter's leaf: a fixed value
                Full scratch;
                return lower(*e, contract_env(), nullptr, Deps {}, scratch).e;
            }
        }
    }
    // The root `Ref` a site key stands for in the body: a formal, a global's address, or (decision 0045 item 10) a derived place's base: its entry snapshot `eb_N`
    // when it loads, the leaf formal otherwise (a by-value parameter's pointer member is a fixed value).
    E key_root_ref(int k)
    {
        if (key_is_global(k))
        {
            return global_ref(k - kGlobalKeyBase);
        }
        if (is_derived_key(k) && derived_of(k).lb_of >= 0)
        {
            return low::var(lb_name(derived_of(k)));
        }
        if (is_derived_key(k))
        {
            const FnAtoms& fa {atoms_of(*fn_.f)};
            for (int i {0}; i < fa.nreq; ++i)
            {
                if (site_key(fa.sites[static_cast<size_t>(i)]) == k)
                {
                    return derived_body_root(fa.sites[static_cast<size_t>(i)], contract_env());
                }
            }
            internal_error(fn_.f->loc, "a derived key without a requires site reached the loop places");
            return low::null_();
        }
        return low::var(formal_name(*fn_.f, k));
    }
    // The name a place's snapshots carry in a loop (the formal's, the global's or the derived place's).
    std::string key_tag(int k)
    {
        if (key_is_global(k))
        {
            return prog_.globals()[static_cast<size_t>(k - kGlobalKeyBase)].viper_name();
        }
        if (is_derived_key(k) && derived_of(k).lb_of >= 0)
        {
            return lb_name(derived_of(k));
        }
        if (is_derived_key(k))
        {
            return eb_name(derived_of(k));
        }
        return formal_name(*fn_.f, k);
    }

    // The places the function holds on entry that a loop frames: the parameters (and globals) with an atom in the `requires`, and in the entry function
    // every global and `static` local of the unit (it holds them from program_start, decision 0041 item 10); `value_keys`: those whose value cells are held.
    void held_keys(std::set<int>& atom_keys, std::set<int>& value_keys)
    {
        const FnAtoms& fa {atoms_of(*fn_.f)};
        for (int i {0}; i < fa.nreq; ++i)
        {
            const AtomSite& site {fa.sites[static_cast<size_t>(i)]};
            atom_keys.insert(site_key(site));
            if (site_takes_cell(site))
            {
                value_keys.insert(site_key(site));
            }
            if (is_derived_key(site_key(site)) && derived_of(site_key(site)).lb_of < 0 && need_lb(site_key(site)))
            { // decision 0045 item 10: also through the loop-entry snapshot
                const int lk {lb_key_of(site_key(site))};
                atom_keys.insert(lk);
                if (site_takes_cell(site))
                {
                    value_keys.insert(lk);
                }
            }
        }
        if (fn_.f == prog_.entry())
        {
            for (const Global& g : prog_.globals())
            {
                atom_keys.insert(kGlobalKeyBase + g.id);
                value_keys.insert(kGlobalKeyBase + g.id);
            }
        }
    }

    // The places a loop frames (decision 0038 item 3), in order: the parameters' places (the formal's value and flag cells of an `owns`/`reads` atom of
    // the `requires`, and one liveness cell per parameter with an atom) of the parameters the function never assigns, then every cell of every open
    // block.  `with_atoms` lists every parameter with an atom (assigned or not), for the case splits.
    std::vector<Place> loop_places(int loop_number, const LoopWrites& writes, std::vector<int>* with_atoms,
                                   const Stmt* loop = nullptr, const std::set<int>* assigned = nullptr,
                                   const std::set<int>* declared = nullptr)
    {
        std::vector<Place> places;
        heap_entry_.clear();
        heap_holders_.clear();
        const Function& f {*fn_.f};
        const FnAtoms& fa {atoms_of(f)};
        const std::string suffix_of_loop {"__" + std::to_string(loop_number)};
        std::set<int> atom_params;
        std::set<int> value_params;
        held_keys(atom_params, value_params);
        (void)fa;
        if (with_atoms != nullptr)
        {
            with_atoms->assign(atom_params.begin(), atom_params.end());
        }
        auto name = [&](Place& pl, const std::string& tag)
        {
            pl.kv = "kv_" + tag + suffix_of_loop;
            pl.kd = "kd_" + tag + suffix_of_loop;
            pl.kl = "kl_" + tag + suffix_of_loop;
            pl.vs = "vs_" + tag + suffix_of_loop;
            pl.ds = "ds_" + tag + suffix_of_loop;
        };
        std::vector<size_t> earlier; // the indexes in `places` of the parameter places so far
        auto join_path = [](const std::vector<int>& path)
        {
            std::string t;
            for (int x : path)
            {
                t += "_m" + std::to_string(x);
            }
            return t;
        };
        for (int k : atom_params)
        {
            const bool global {key_is_global(k)};
            if (!global && fn_.assigned_params.count(k) != 0U)
            {
                continue;
            }
            const bool derived {is_derived_key(k)};
            if (!global && !derived && f.params[static_cast<size_t>(k)].type.pointee == nullptr)
            {
                continue;
            }
            const E rootref {key_root_ref(k)};
            // The place's label as the user spells it (`*p`, or `g`): the first atom of the key that has a spelling.
            std::string label;
            if (global)
            {
                label = prog_.globals()[static_cast<size_t>(k - kGlobalKeyBase)].c_name;
            }
            else if (derived)
            {
                label.clear();
                for (const AtomSite& site : fa.sites)
                {
                    if (key_has_site(site, k) && !site_text(site).empty())
                    {
                        label = site_text(site);
                        break;
                    }
                }
            }
            else
            {
                const std::string& c {f.params[static_cast<size_t>(k)].name};
                label = c.empty() ? std::string() : "*" + c;
                for (const AtomSite& site : fa.sites)
                {
                    if (key_has_site(site, k) && !site_text(site).empty())
                    {
                        label = site_text(site);
                        break;
                    }
                }
            }
            // The value cells the key's atoms take, each once (decision 0043 items 9 and 11): a struct atom's leaves, a member atom's leaf, a scalar place's cell.
            struct VCell
            {
                Cell c;
                std::vector<int> full;
            };
            std::vector<VCell> vcells;
            std::vector<std::pair<Type, E>> slots;
            for (int i {0}; i < fa.nreq; ++i)
            {
                const AtomSite& site {fa.sites[static_cast<size_t>(i)]};
                if (!key_has_site(site, k))
                {
                    continue;
                }
                if (site_pointee(site).is_struct())
                {
                    bool seen {false};
                    for (const auto& sl : slots)
                    {
                        seen = seen || same_expr(sl.second, chain_ref(rootref, site.path));
                    }
                    if (!seen)
                    {
                        slots.emplace_back(site_pointee(site), chain_ref(rootref, site.path));
                    }
                }
                if (!site_takes_cell(site))
                {
                    continue;
                }
                for (Cell& c : site_cells(site, chain_ref(rootref, site.path)))
                {
                    std::vector<int> full {site.path};
                    full.insert(full.end(), c.path.begin(), c.path.end());
                    bool dup {false};
                    for (const VCell& v : vcells)
                    {
                        dup = dup || v.full == full;
                    }
                    if (!dup)
                    {
                        vcells.push_back(VCell {std::move(c), std::move(full)});
                    }
                }
            }
            // The entry function holds every global with no atom (program_start): its one value cell (decision 0041 item 10).
            if (global && value_params.count(k) != 0U && key_pointee(k).is_struct() && fn_.f == prog_.entry())
            {
                // decision 0044 item 8: the entry function holds every leaf of a struct global (program_start), each a place of its own; the root slot is what a pointer may equal
                for (Cell& c : leaf_cells(rootref, *key_pointee(k).struct_type, label, false))
                {
                    std::vector<int> full {c.path};
                    bool dup {false};
                    for (const VCell& v : vcells)
                    {
                        dup = dup || v.full == full;
                    }
                    if (!dup)
                    {
                        vcells.push_back(VCell {std::move(c), std::move(full)});
                    }
                }
                bool seen {false};
                for (const auto& sl : slots)
                {
                    seen = seen || same_expr(sl.second, rootref);
                }
                if (!seen)
                {
                    slots.emplace_back(key_pointee(k), rootref);
                }
            }
            else if (vcells.empty() && global && value_params.count(k) != 0U)
            {
                vcells.push_back(VCell {Cell {rootref, key_pointee(k), label, {}, ""}, {}});
            }
            const size_t first_of_key {places.size()};
            auto add_place = [&](Place pl, const std::vector<int>& mpath, bool live)
            {
                pl.param = k;
                pl.lroot = rootref;
                pl.live = live;
                pl.mpath = mpath;
                pl.suffix = cell_suffix(pl.type);
                name(pl, key_tag(k) + join_path(mpath));
                // A global's address is never null: no guard of its own (decision 0041 item 10); the guards below keep it apart from a parameter's place.
                if (!global)
                {
                    pl.gv = low::bin(low::BinOp::Ne, rootref, low::null_());
                    if (live)
                    {
                        pl.gl = pl.gv;
                    }
                }
                for (size_t j : earlier)
                {
                    const Place& o {places[j]};
                    if (global && key_is_global(o.param))
                    {
                        continue; // two globals are two `unique` blocks
                    }
                    if (pl.value && o.value && o.suffix == pl.suffix && o.param != k)
                    {
                        pl.gv = low::and_(pl.gv, low::bin(low::BinOp::Ne, pl.recv, o.recv));
                    }
                    if (live && o.live && o.param != k)
                    {
                        pl.gl = low::and_(
                            pl.gl, low::implies(low::bin(low::BinOp::Ne, lv_of(o), low::null_()),
                                                low::bin(low::BinOp::Ne, live_cell(lv_of(pl)), live_cell(lv_of(o)))));
                    }
                }
                earlier.push_back(places.size());
                places.push_back(std::move(pl));
            };
            if (vcells.empty())
            { // only a `live` atom: the liveness place, as before
                Place pl;
                pl.recv = rootref;
                pl.type = (global || derived) ? key_pointee(k) : *f.params[static_cast<size_t>(k)].type.pointee;
                pl.value = false;
                pl.label = label;
                add_place(std::move(pl), {}, true);
            }
            else
            {
                for (const VCell& v : vcells)
                {
                    Place pl;
                    pl.recv = v.c.recv;
                    pl.type = v.c.type;
                    pl.value = true;
                    pl.label = v.full.empty() ? label : v.c.text;
                    add_place(std::move(pl), v.full, places.size() == first_of_key);
                }
            }
            places[first_of_key].slots = slots;
            if (places[first_of_key].slots.empty() && !places[first_of_key].value &&
                places[first_of_key].type.is_struct())
            {
                places[first_of_key].slots.emplace_back(places[first_of_key].type, rootref);
            }
        }
        for (const Fn::Scope& sc : fn_.scopes)
        {
            for (const Var* cell : sc.cells)
            {
                const E ref {low::var(var_name(*cell))};
                auto push = [&](const E& recv, const Type& t, const std::string& label, const std::vector<int>& mpath,
                                bool live)
                {
                    Place pl;
                    pl.cell = true;
                    pl.var = cell;
                    pl.recv = recv;
                    pl.lroot = ref;
                    pl.type = t;
                    pl.value = true;
                    pl.suffix = cell_suffix(t);
                    pl.label = label;
                    pl.live = live;
                    pl.mpath = mpath;
                    name(pl, var_name(*cell) + join_path(mpath));
                    places.push_back(std::move(pl));
                };
                if (cell->type.is_struct())
                { // a struct local: every leaf is a place; the first carries the liveness row (decision 0043 item 11)
                    const size_t at {places.size()};
                    bool first {true};
                    for (const Cell& c : leaf_cells(ref, *cell->type.struct_type, cell->name, false))
                    {
                        push(c.recv, c.type, c.text, c.path, first);
                        first = false;
                    }
                    if (places.size() > at)
                    {
                        places[at].slots.emplace_back(cell->type, ref);
                    }
                }
                else
                {
                    push(ref, cell->type, cell->name, {}, true);
                }
            }
        }
        // Decision 0047 item 10 (heap functions only): the object each pointer variable the loop uses designates at its entry is a place of the loop, with its leaf cells,
        // its liveness cell and its free token: a block held through a local (`p = malloc(..)`) survives the loop and can be freed after it.
        if (heap_ && loop != nullptr && assigned != nullptr && declared != nullptr)
        {
            std::set<int> used;
            collect_local_uses(loop, used);
            for (const auto& vp : f.locals())
            {
                const Var& v {*vp};
                if (!v.type.is_ptr() || v.type.pointee == nullptr || v.type.pointee->is_void() || v.address_taken ||
                    v.compound_literal)
                {
                    continue;
                }
                if (declared->count(v.id) != 0U)
                {
                    continue;
                }
                if (v.loc.known() && loop->loc.known() && v.loc.file == loop->loc.file &&
                    (v.loc.line > loop->loc.line || (v.loc.line == loop->loc.line && v.loc.column > loop->loc.column)))
                {
                    continue; // declared after the loop
                }
                if (used.count(v.id) == 0U)
                { // a holder beside the loop's places (B1 item 3)
                    const Type& ht {*v.type.pointee};
                    const E h {low::var(var_name(v))};
                    heap_holders_.push_back(HeapHolder {h, ht,
                                                        ht.is_struct()
                                                            ? leaf_cells(h, *ht.struct_type, v.name, true)
                                                            : std::vector<Cell> {Cell {h, ht, "*" + v.name, {}, ""}}});
                    continue;
                }
                // Record 0047 item 10 (re-audit S1): the local copy of a pointer parameter that has an atom place and that the function never assigns is the parameter's
                // own place (its `acc`s, the liveness and the token cell are the parameter place's); a second place on the copy has a guard that is false at once
                // (`v_a != p_a`) and multiplies the case splits of every unrelated function of a file that calls `malloc`.
                if (v.is_param_copy && atom_params.count(v.param_index) != 0U &&
                    fn_.assigned_params.count(v.param_index) == 0U)
                {
                    continue;
                }
                const Type& pt {*v.type.pointee};
                const std::string tag {"h" + std::to_string(v.id)};
                E hb {low::var(var_name(v))};
                if (assigned->count(v.id) != 0U)
                { // the loop moves the variable: its entry value is the base
                    const std::string g {"hb_" + var_name(v) + suffix_of_loop};
                    fn_.snapshots.push_back(low::LVar {g, low::LType::Ref});
                    assign(heap_entry_, g, hb);
                    hb = low::var(g);
                }
                std::vector<Cell> cells {pt.is_struct() ? leaf_cells(hb, *pt.struct_type, v.name, true)
                                                        : std::vector<Cell> {Cell {hb, pt, "*" + v.name, {}, ""}}};
                const E gnull {low::bin(low::BinOp::Ne, hb, low::null_())};
                const size_t first_of_var {places.size()};
                for (size_t ci {0U}; ci < cells.size(); ++ci)
                {
                    const Cell& c {cells[ci]};
                    Place pl;
                    pl.heap = true;
                    pl.param = kHeapPlaceBase - static_cast<int>(places.size());
                    pl.recv = c.recv;
                    pl.lroot = hb;
                    pl.type = c.type;
                    pl.ptee = pt;
                    pl.value = true;
                    pl.suffix = cell_suffix(c.type);
                    pl.label = c.text;
                    pl.live = ci == 0U;
                    pl.mpath = c.path;
                    name(pl, tag + join_path(c.path));
                    pl.gv = gnull;
                    if (pl.live)
                    {
                        pl.gl = gnull;
                    }
                    for (size_t j {0U}; j < places.size(); ++j)
                    {
                        const Place& o {places[j]};
                        if (o.cell || (o.heap && same_expr(o.lroot, hb)))
                        {
                            continue;
                        }
                        if (o.value && o.suffix == pl.suffix)
                        {
                            pl.gv = low::and_(pl.gv, low::bin(low::BinOp::Ne, pl.recv, o.recv));
                        }
                        if (pl.live && o.live)
                        {
                            pl.gl = low::and_(pl.gl, low::implies(low::bin(low::BinOp::Ne, lv_of(o), low::null_()),
                                                                  low::bin(low::BinOp::Ne, live_cell(lv_of(pl)),
                                                                           live_cell(lv_of(o)))));
                        }
                    }
                    if (pl.live)
                    { // the token place of the block: one per variable, apart from every other heap place's block
                        pl.kt = "kt_" + tag + suffix_of_loop;
                        if (vlibc_heap_)
                        {
                            pl.ktv = "ktv_" + tag + suffix_of_loop;
                        }
                        pl.gt = gnull;
                        for (const Place& o : places)
                        {
                            if (o.heap && !o.kt.empty() && !same_expr(o.lroot, hb))
                            {
                                pl.gt = low::and_(pl.gt, low::bin(low::BinOp::Ne, block_of(hb), block_of(o.lroot)));
                            }
                        }
                        if (pt.is_struct())
                        {
                            pl.slots.emplace_back(pt, hb);
                        }
                    }
                    places.push_back(std::move(pl));
                }
                (void)first_of_var;
            }
        }
        if (heap_ && loop != nullptr)
        {
            for (int k : atom_params)
            {
                if (key_is_global(k) || is_derived_key(k))
                {
                    continue;
                }
                const Param& pa {f.params[static_cast<size_t>(k)]};
                if (pa.type.pointee == nullptr || pa.type.pointee->is_void())
                {
                    continue;
                }
                bool block_site {false};
                for (const AtomSite& site : fa.sites)
                {
                    block_site = block_site || (key_has_site(site, k) && site.atom->atom == AtomKind::OwnsBlock);
                }
                if (!block_site)
                {
                    continue;
                }
                const E h {key_root_ref(k)};
                const Type& ht {*pa.type.pointee};
                heap_holders_.push_back(HeapHolder {h, ht,
                                                    ht.is_struct()
                                                        ? leaf_cells(h, *ht.struct_type, pa.name, true)
                                                        : std::vector<Cell> {Cell {h, ht, "*" + pa.name, {}, ""}}});
            }
        }
        // A parameter place whose atom can be false (`c == 0 || owns(*p)`) holds nothing on that path, and nothing tells Silicon that `p` is not a cell:
        // its snapshot would pick up the cell's permission and the invariant would demand it twice.  A parameter place is therefore counted only when it
        // is no cell (when it is one, the cell's own conjunct holds it); audit round 2, B1.
        for (Place& pl : places)
        {
            if (pl.cell)
            {
                continue;
            }
            for (const Place& c : places)
            {
                if (!c.cell)
                {
                    continue;
                }
                if (pl.value && c.suffix == pl.suffix)
                {
                    pl.gv = low::and_(pl.gv, low::bin(low::BinOp::Ne, pl.recv, c.recv));
                }
                if (pl.live && c.live)
                {
                    pl.gl = low::and_(pl.gl, low::bin(low::BinOp::Ne, live_cell(lv_of(pl)), live_cell(lv_of(c))));
                }
            }
        }
        for (Place& pl : places)
        {
            if (!pl.value)
            {
                continue;
            }
            pl.reach = writes.fields.count(pl.suffix) != 0U || (pl.cell && writes.cells.count(pl.var->id) != 0U) ||
                       (!pl.cell && key_is_global(pl.param) &&
                        (writes.globals.count(pl.param - kGlobalKeyBase) != 0U ||
                         writes.gleaves.count({pl.param - kGlobalKeyBase, pl.mpath}) != 0U));
        }
        return places;
    }

    // One automatic conjunct: an `internal` row (provable by construction; a failure is a cerastes bug, decision 0038 item 3).
    low::LInvariant auto_invariant(E expr, const SrcLoc& loc)
    {
        low::LInvariant inv;
        inv.expr = std::move(expr);
        for (const char* id : {"invariant.not.established", "invariant.not.preserved"})
        {
            low::Meta m;
            m.category = "internal";
            m.loc = loc;
            m.message = "internal check: an automatic permission invariant of this loop (decision 0038 item 3)";
            inv.checks.push_back(low::Check {id, std::move(m)});
        }
        return inv;
    }

    // A case split that `out` does not hold yet (audit round 2, N9: `v == p` came twice when the place's receiver is its base).
    void case_split_once(std::vector<low::LStmt>& out, E cond)
    {
        for (const low::LStmt& st : out)
        {
            if (st.kind == low::LStmt::Kind::CaseSplit && same_expr(st.expr, cond))
            {
                return;
            }
        }
        case_split(out, std::move(cond));
    }
    void case_split(std::vector<low::LStmt>& out, E cond)
    {
        low::LStmt st;
        st.kind = low::LStmt::Kind::CaseSplit;
        st.expr = std::move(cond);
        out.push_back(std::move(st));
    }

    // ---- written atoms in loop invariants (decision 0040 item 5) ----

    std::vector<WAtom> written_atoms(const Stmt& s, int loop_number, const std::set<int>& assigned,
                                     const std::set<int>& declared, const LoopWrites& writes)
    {
        std::vector<WAtom> out;
        int atom_id {0};
        for (const AtomSite& site : loop_atoms(s))
        {
            const Expr& qa {*site.atom->a};
            const AtomKind kind {site.atom->atom};
            const Type& pt {*qa.type.pointee};
            const bool loads {reads_heap(qa) ||
                              (qa.kind == Expr::Kind::Local && qa.var != nullptr && qa.var->address_taken)};
            // `moved` (no value snapshot): the loop assigns the pointer variable the place hangs from, writes a cell of the path by name, or writes a loaded field of it.
            bool moved {false};
            {
                const Expr* root {member_root(&qa, nullptr)};
                // a global the place hangs from by a load (`*gp`): the loop moves the place when it writes the global
                bool under_load {false};
                for (const Expr* x {root}; x != nullptr;
                     x = (x->kind == Expr::Kind::Deref || x->kind == Expr::Kind::Field ||
                          x->kind == Expr::Kind::MemberAddr)
                             ? x->a
                             : nullptr)
                {
                    if (x->kind == Expr::Kind::Local && x->var != nullptr && assigned.count(x->var->id) != 0U &&
                        declared.count(x->var->id) == 0U)
                    {
                        moved = true;
                    }
                    if (x->kind == Expr::Kind::Deref && writes.fields.count(cell_suffix(x->type)) != 0U)
                    {
                        moved = true;
                    }
                    if (x->kind == Expr::Kind::Deref)
                    {
                        under_load = true;
                    }
                    if (x->kind == Expr::Kind::GlobalAddr && x->global != nullptr && under_load)
                    {
                        if (writes.globals.count(x->global->id) != 0U)
                        {
                            moved = true;
                        }
                        for (const auto& gl : writes.gleaves)
                        {
                            if (gl.first == x->global->id)
                            {
                                moved = true;
                            }
                        }
                    }
                }
                if (loads && !writes.cells.empty())
                {
                    moved = true;
                }
            }
            // The entries: one for a scalar or `live` atom, one per leaf cell for a struct atom.
            std::vector<std::pair<std::vector<int>, Cell>> entries;
            if (kind != AtomKind::Live && pt.is_struct())
            {
                for (Cell& c : leaf_cells(low::null_(), *pt.struct_type, qa.c_text, true))
                {
                    entries.emplace_back(c.path, std::move(c));
                }
            }
            else
            {
                entries.emplace_back(std::vector<int> {}, Cell {nullptr, pt, std::string(), {}, ""});
            }
            bool first {true};
            for (auto& [lpath, leaf] : entries)
            {
                WAtom a;
                a.kind = kind;
                a.atom = site.atom;
                a.guards = site.guards;
                a.loads = loads;
                a.atom_id = atom_id;
                a.first = first;
                a.lpath = lpath;
                a.lpt = qa.type;
                a.pt = kind != AtomKind::Live && pt.is_struct() ? leaf.type : pt;
                a.sfx = kind == AtomKind::Live ? std::string() : cell_suffix(a.pt);
                a.text = kind == AtomKind::Live ? qa.c_text : (pt.is_struct() ? leaf.text : "*" + qa.c_text);
                a.loc = site.atom->loc;
                const std::string tag {std::to_string(out.size()) + "__" + std::to_string(loop_number)};
                a.av = "aw_v" + tag;
                a.ad = "aw_d" + tag;
                a.al = "aw_l" + tag;
                if (loads)
                {
                    a.aq = "aw_q" + tag;
                }
                if (kind != AtomKind::Live && !moved)
                {
                    a.vs = "aw_s" + tag;
                }
                a.reach = kind != AtomKind::Live && (writes.fields.count(a.sfx) != 0U || !writes.cells.empty());
                out.push_back(std::move(a));
                first = false;
            }
            ++atom_id;
        }
        return out;
    }

    // The pointers of the written atoms, once the loop's places are known (decision 0045 item 12): an atom that loads is read in the invariant through the loop's places and
    // the earlier written atoms (chains framed by their `acc`s); the guards the same when they read the heap.  Each entry then gets its leaf `Ref`, and the body's ghost.
    void resolve_written(std::vector<WAtom>& w, const std::vector<Place>& places)
    {
        std::vector<Place> all {places};
        const std::vector<Place>* saved {inv_places_};
        inv_places_ = &all;
        E slot;
        E slot_g;
        for (size_t j {0U}; j < w.size(); ++j)
        {
            WAtom& a {w[j]};
            if (a.first)
            {
                const Expr& qa {*a.atom->a};
                if (a.loads)
                {
                    CtxSave save(*this);
                    cmode_ = CMode::Inv;
                    cf_ = fn_.f;
                    inv_reads_.clear();
                    inv_what_ = "invariant";
                    Full scratch;
                    slot = lower(qa, body_env(), nullptr, {}, scratch).e;
                    inv_reads_.clear();
                }
                else if (qa.kind == Expr::Kind::Local && qa.var != nullptr)
                {
                    slot = low::var(var_name(*qa.var));
                }
                else
                {
                    slot = lower_pure(qa);
                }
                AtomSite site; // the guards only
                site.atom = a.atom;
                site.guards = a.guards;
                bool guard_reads {false};
                for (const AtomGuard& x : a.guards)
                {
                    guard_reads = guard_reads || reads_heap(*x.cond);
                }
                if (guard_reads)
                {
                    CtxSave save(*this);
                    cmode_ = CMode::Inv;
                    cf_ = fn_.f;
                    inv_reads_.clear();
                    E g;
                    for (const AtomGuard& x : a.guards)
                    {
                        Full scratch;
                        const LV v {lower(*x.cond, body_env(), nullptr, {}, scratch)};
                        g = low::and_(g, x.holds ? v.e : low::not_(v.e));
                    }
                    inv_reads_.clear();
                    slot_g = g;
                }
                else
                {
                    slot_g = site_guard(site, body_env());
                }
            }
            a.lq = slot;
            a.g = slot_g;
            a.q = chain_ref(slot, a.lpath);
            a.qb = a.loads ? low::var(a.aq) : a.q;
            if (a.loads)
            {
                fn_.snapshots.push_back(low::LVar {a.aq, low::LType::Ref});
            }
            if (a.kind != AtomKind::Live)
            {
                Place wp;
                wp.recv = a.q;
                wp.type = a.pt;
                wp.value = true;
                wp.suffix = a.sfx;
                wp.kv = a.av;
                wp.kd = a.ad;
                wp.kl = a.al;
                wp.wg = a.g;
                all.push_back(std::move(wp));
            }
        }
        inv_places_ = saved;
    }

    E lower_pure(const Expr& x)
    {
        CtxSave save(*this);
        cmode_ = CMode::Wf;
        Full scratch;
        return lower(x, body_env(), nullptr, {}, scratch).e;
    }

    // NOTAUTO for the value and flag cells: `q` is no automatic place of its field (a place with a false guard holds nothing, so it does not count), nor
    // the place of an earlier `reads` atom of a true guard (audit B3, S1).
    E written_notauto_v(const std::vector<WAtom>& w, size_t j, const std::vector<Place>& places)
    {
        using low::BinOp;
        const WAtom& a {w[j]};
        E r;
        for (const Place& pl : places)
        {
            if (pl.value && pl.suffix == a.sfx)
            {
                r = low::and_(r, low::implies(pl.gv, low::bin(BinOp::Ne, a.q, pl.recv)));
            }
        }
        if (a.kind == AtomKind::Reads)
        {
            for (size_t k {0U}; k < j; ++k)
            {
                if (w[k].kind != AtomKind::Live && w[k].sfx == a.sfx && w[k].atom_id != a.atom_id)
                {
                    r = low::and_(r, low::implies(w[k].g, low::bin(BinOp::Ne, a.q, w[k].q)));
                }
            }
        }
        return r;
    }
    // NOTAUTO for the liveness cell: its block is no automatic place's (a true `gl`), nor an earlier atom's (a true guard).
    E written_notauto_l(const std::vector<WAtom>& w, size_t j, const std::vector<Place>& places)
    {
        using low::BinOp;
        const WAtom& a {w[j]};
        E r;
        for (const Place& pl : places)
        {
            if (pl.live)
            {
                r = low::and_(r, low::implies(pl.gl, low::bin(BinOp::Ne, live_cell(a.lq), live_cell(lv_of(pl)))));
            }
        }
        for (size_t k {0U}; k < j; ++k)
        {
            if (w[k].first && w[k].atom_id != a.atom_id)
            {
                r = low::and_(r, low::implies(w[k].g, low::bin(BinOp::Ne, live_cell(a.lq), live_cell(w[k].lq))));
            }
        }
        return r;
    }

    // The no-op case splits (S28 P8): the atom's place against every automatic place of its field and against an earlier atom's, and its liveness cell
    // against every other liveness cell.
    void written_splits(const std::vector<WAtom>& w, const std::vector<Place>& places, std::vector<low::LStmt>& out,
                        bool body = false)
    {
        using low::BinOp;
        // `body`: the splits at the top of an iteration name an atom that loads by its ghost Ref (`qb`), never by a heap read.
        auto qn = [&](const WAtom& a)
        {
            return body ? a.qb : a.q;
        };
        auto ln = [&](const WAtom& a)
        {
            return body && a.loads ? a.qb : a.lq;
        }; // the slot's ghost is the leaf's for a scalar atom; a struct atom's slot is read like q
        for (size_t j {0U}; j < w.size(); ++j)
        {
            const WAtom& a {w[j]};
            for (const Place& pl : places)
            {
                if (a.kind != AtomKind::Live && pl.value && pl.suffix == a.sfx)
                {
                    case_split(out, low::bin(BinOp::Eq, qn(a), pl.recv));
                }
                if (pl.live && a.first)
                {
                    case_split(out, low::bin(BinOp::Eq, live_cell(ln(a)), live_cell(lv_of(pl))));
                }
            }
            for (size_t k {0U}; k < j; ++k)
            {
                if (a.atom_id == w[k].atom_id)
                {
                    continue;
                }
                if (a.kind != AtomKind::Live && w[k].kind != AtomKind::Live && w[k].sfx == a.sfx)
                {
                    case_split(out, low::bin(BinOp::Eq, qn(a), qn(w[k])));
                }
                if (a.first && w[k].first)
                {
                    case_split(out, low::bin(BinOp::Eq, live_cell(ln(a)), live_cell(ln(w[k]))));
                }
            }
        }
    }

    // The ghost amounts, taken at the entry (decision 0040 item 5): `owns` takes write, `reads` what is held, every kind the liveness cell's.
    void written_amounts(const std::vector<WAtom>& w, std::vector<low::LStmt>& out)
    {
        for (const WAtom& a : w)
        {
            if (a.kind != AtomKind::Live)
            {
                fn_.snapshots.push_back(low::LVar {a.av, low::LType::Perm});
                fn_.snapshots.push_back(low::LVar {a.ad, low::LType::Perm});
                assign(out, a.av, a.kind == AtomKind::Owns ? low::write_perm() : value_perm(a.q, a.pt));
                assign(out, a.ad, a.kind == AtomKind::Owns ? low::write_perm() : flag_perm(a.q, a.pt));
            }
            if (!a.first)
            {
                continue;
            }
            fn_.snapshots.push_back(low::LVar {a.al, low::LType::Perm});
            assign(out, a.al, live_perm(a.lq));
        }
    }
    // The value snapshots (decision 0040 item 5, "Values"): the value the place holds at the entry, when it is held there.  Taken after the splits
    // and the amounts, under the atom's guard and NOTAUTO (an automatic place's own snapshot covers the others).
    void written_values(const std::vector<WAtom>& w, const std::vector<Place>& places, std::vector<low::LStmt>& out)
    {
        for (size_t j {0U}; j < w.size(); ++j)
        {
            const WAtom& a {w[j]};
            if (a.vs.empty())
            {
                continue;
            }
            fn_.snapshots.push_back(low::LVar {a.vs, ltype(a.pt)});
            low::LStmt st;
            st.kind = low::LStmt::Kind::If;
            st.expr = low::and_(low::and_(a.g, written_notauto_v(w, j, places)),
                                low::bin(low::BinOp::Gt, value_perm(a.q, a.pt), low::no_perm()));
            assign(st.then_body, a.vs, cell_value(a.q, a.pt));
            out.push_back(std::move(st));
        }
    }

    // The invariant conjuncts of the written atoms, after the automatic ones.
    void written_rows(const std::vector<WAtom>& w, const std::vector<Place>& places, const SrcLoc& at,
                      std::vector<low::LInvariant>& out)
    {
        using low::BinOp;
        const E none {low::no_perm()};
        const E write {low::write_perm()};
        for (size_t j {0U}; j < w.size(); ++j)
        {
            const WAtom& a {w[j]};
            const bool live {a.kind == AtomKind::Live};
            const bool owns {a.kind == AtomKind::Owns};
            if (a.first)
            { // the atom's basic row (WF_S and the member lengths for a struct atom), once
                E basic {low::and_(low::bin(BinOp::Ne, a.lq, low::null_()), wf_expr(a.lq, &a.lpt))};
                if (!live || (a.lpt.pointee != nullptr && a.lpt.pointee->is_struct()))
                {
                    basic = low::and_(basic, in_block(a.lq));
                }
                out.push_back(auto_invariant(low::implies(a.g, basic), at));
            }
            const E nv {low::and_(a.g, written_notauto_v(w, j, places))};
            const E nl {low::and_(a.g, written_notauto_l(w, j, places))};
            if (!live)
            {
                out.push_back(auto_invariant(low::implies(nv, low::and_(low::bin(BinOp::Gt, low::var(a.av), none),
                                                                        low::bin(BinOp::Gt, low::var(a.ad), none))),
                                             at));
                E acc {low::and_(low::acc(a.q, "v_" + a.sfx, low::var(a.av)),
                                 low::acc(a.q, "d_" + a.sfx, low::var(a.ad)))};
                acc = low::and_(acc, cell_flag(a.q, a.pt));
                if (const E r = cell_range_inv(a.q, a.pt))
                {
                    acc = low::and_(acc, r);
                }
                out.push_back(auto_invariant(low::implies(nv, acc), at));
                if (!a.vs.empty())
                {
                    const E same {low::bin(BinOp::Eq, cell_value(a.q, a.pt), low::var(a.vs))};
                    out.push_back(auto_invariant(
                        low::implies(a.reach ? low::and_(nv, low::bin(BinOp::Lt, low::var(a.av), write)) : nv, same),
                        at));
                }
                for (const Place& pl : places)
                {
                    if (!pl.value || pl.suffix != a.sfx)
                    {
                        continue;
                    }
                    const E hold {owns ? low::and_(low::bin(BinOp::Eq, low::var(pl.kv), write),
                                                   low::bin(BinOp::Eq, low::var(pl.kd), write))
                                       : low::and_(low::bin(BinOp::Gt, low::var(pl.kv), none),
                                                   low::bin(BinOp::Gt, low::var(pl.kd), none))};
                    out.push_back(auto_invariant(
                        low::implies(low::and_(low::and_(a.g, pl.gv), low::bin(BinOp::Eq, a.q, pl.recv)), hold), at));
                }
            }
            if (!a.first)
            {
                continue; // one liveness amount per atom
            }
            out.push_back(auto_invariant(low::implies(nl, low::bin(BinOp::Gt, low::var(a.al), none)), at));
            out.push_back(auto_invariant(low::implies(nl, low::acc(live_cell(a.lq), "lv", low::var(a.al))), at));
            for (const Place& pl : places)
            {
                if (pl.live)
                {
                    out.push_back(
                        auto_invariant(low::implies(low::and_(low::and_(a.g, pl.gl), low::bin(BinOp::Eq, a.q, pl.recv)),
                                                    low::bin(BinOp::Gt, low::var(pl.kl), none)),
                                       at));
                }
            }
        }
    }

    // The explicit checks (decision 0028 item 11: Silicon's own invariant checks never fail first), at the entry and, with `back`, at the end of an
    // iteration (reported at `at`, the body's closing brace); then the separation of the pairs that may not share a place.
    void written_checks(const std::vector<WAtom>& w, const std::vector<Place>& places, bool back, const SrcLoc& at,
                        std::vector<low::LStmt>& out)
    {
        using low::BinOp;
        const E none {low::no_perm()};
        const E write {low::write_perm()};
        for (size_t j {0U}; j < w.size(); ++j)
        {
            const WAtom& a {w[j]};
            const bool live {a.kind == AtomKind::Live};
            const bool owns {a.kind == AtomKind::Owns};
            const std::string pl {quoted(a.text, "the object the pointer points to")};
            const std::string what {owns   ? "owning " + pl
                                    : live ? quoted(a.text, "the pointer") + " pointing to a live object"
                                           : "the permission to read " + pl};
            const std::string msg {back ? "an iteration of this loop ends without " + what +
                                              ", which its invariant needs"
                                        : "enters this loop without " + what + ", which its invariant needs"};
            const SrcLoc where {back ? at : a.loc};
            auto add = [&](E cond)
            {
                emit_assert(out, low::implies(a.g, std::move(cond)), meta("permission", where, msg));
            };
            if (a.first)
            {
                add(low::bin(BinOp::Ne, a.lq, low::null_()));
                add(wf_expr(a.lq, &a.lpt));
                if (!live || (a.lpt.pointee != nullptr && a.lpt.pointee->is_struct()))
                {
                    add(in_block(a.lq));
                }
            }
            if (!live)
            {
                if (back)
                {
                    add(low::bin(BinOp::Ge, value_perm(a.q, a.pt), low::var(a.av)));
                    add(low::bin(BinOp::Ge, flag_perm(a.q, a.pt), low::var(a.ad)));
                }
                else if (owns)
                {
                    add(low::bin(BinOp::Eq, value_perm(a.q, a.pt), write));
                    add(low::bin(BinOp::Eq, flag_perm(a.q, a.pt), write));
                }
                else
                {
                    add(low::bin(BinOp::Gt, value_perm(a.q, a.pt), none));
                    add(low::bin(BinOp::Gt, flag_perm(a.q, a.pt), none));
                }
                add(cell_flag(a.q, a.pt));
                if (const E r = cell_range(a.q, a.pt))
                {
                    add(r);
                }
                emit_assert(out,
                            low::implies(low::and_(a.g, written_notauto_v(w, j, places)),
                                         low::and_(low::bin(BinOp::Gt, low::var(a.av), none),
                                                   low::bin(BinOp::Gt, low::var(a.ad), none))),
                            meta("permission", where, msg));
            }
            if (a.first)
            {
                add(back ? low::bin(BinOp::Ge, live_perm(a.lq), low::var(a.al))
                         : low::bin(BinOp::Gt, live_perm(a.lq), none));
                emit_assert(out,
                            low::implies(low::and_(a.g, written_notauto_l(w, j, places)),
                                         low::bin(BinOp::Gt, low::var(a.al), none)),
                            meta("permission", where, msg));
            }
        }
        for (size_t j {0U}; j < w.size(); ++j)
        {
            for (size_t k {0U}; k < j; ++k)
            {
                const WAtom& a {w[j]};
                const WAtom& b {w[k]};
                if (a.kind == AtomKind::Live || b.kind == AtomKind::Live || a.sfx != b.sfx || a.atom_id == b.atom_id)
                {
                    continue;
                }
                if (a.kind != AtomKind::Owns && b.kind != AtomKind::Owns)
                {
                    continue;
                }
                emit_assert(out, low::implies(low::and_(a.g, b.g), low::bin(BinOp::Ne, a.q, b.q)),
                            meta("permission", back ? at : a.loc,
                                 "the invariant of this loop owns " + quoted(b.text, "an object") + " and " +
                                     quoted(a.text, "an object") + ", which may be one object"));
            }
        }
    }

    // A loop.  Its invariants are, in order: the automatic range invariant of each outer local the loop assigns; the automatic permission
    // invariants of the loop's places (decision 0038 item 3); then, for each user invariant, the obligations of its own expression (guarded by the
    // earlier user invariants) and the invariant itself; then the obligations of the loop condition (guarded by all user
    // invariants).  An obligation as an invariant clause is checked exactly where 0012 item 8 asks: before the
    // first evaluation (on entry) and before each next one (preservation), and it is then known inside the body.
    //
    // Decision 0014: a `do` loop is `while (true)` with the condition's obligations and the exit test at the end of the body
    // (item 5); a `for` loop's step follows the `continue` label; `do ... while (0)` is no loop at all (do_once).  Decision 0038 item 2: a
    // condition that touches memory is tested at the top of the body, like one with a side effect.
    void loop(const Stmt& s, std::vector<low::LStmt>& out)
    {
        // a break or continue leaves every scope opened inside the loop (decision 0038 item 7)
        fn_.jump_depth[s.jump_id] = fn_.scopes.size();
        if (s.form == LoopForm::DoOnce)
        {
            do_once(s, out);
            return;
        }
        const bool is_do {s.form == LoopForm::DoWhile};
        low::LStmt w;
        w.kind = low::LStmt::Kind::While;

        std::set<int> assigned;
        std::set<int> declared;
        scan_assigned(*s.body, assigned, declared);
        if (s.step != nullptr)
        {
            scan_assigned(*s.step, assigned, declared);
        }
        scan_assigned_expr(s.expr, assigned); // the loop's own condition (a side effect in it assigns, decision 0020)

        // Decision 0038 item 3: a loop that touches memory, or whose body leaves the function or a block that holds a place while the function holds
        // one, carries every place the function holds.
        const int loop_number {fn_.nloop++};
        const int saved_loop {cur_loop_};
        cur_loop_ = loop_number;
        LoopWrites writes;
        writes_in(&s, writes);
        std::vector<int> atom_params;
        std::vector<Place> places {loop_places(loop_number, writes, &atom_params, &s, &assigned, &declared)};
        bool frame {stmt_touches(&s)};
        if (!frame && !places.empty())
        {
            bool has_return {false};
            std::set<int> gotos;
            std::set<int> labels;
            exits_of(s.body, has_return, gotos, labels);
            exits_of(s.step, has_return, gotos, labels);
            bool outward {false};
            for (int g : gotos)
            {
                if (labels.count(g) == 0U)
                {
                    outward = true;
                }
            }
            frame = has_return || outward;
        }
        if (!frame)
        {
            places.clear();
        }
        // B1 item 3: the pointer variables that hold blocks beside the loop's heap places
        std::vector<HeapHolder> holders {std::move(heap_holders_)};
        heap_holders_.clear();
        // decision 0047 item 10: `hb := v` for each heap place whose variable the loop assigns
        std::vector<low::LStmt> hbstmts {std::move(heap_entry_)};
        heap_entry_.clear();
        if (places.empty())
        {
            hbstmts.clear();
        }
        std::vector<WAtom> wats {written_atoms(s, loop_number, assigned, declared, writes)}; // decision 0040 item 5
        resolve_written(wats, places);
        std::vector<low::LStmt> wsplits;
        std::vector<low::LStmt> bsplits;
        const size_t active_before {active_w_.size()};
        for (const WAtom& a : wats)
        {
            active_w_.push_back(&a);
        }
        // Decision 0045 item 10 (owner question 110, audit S6): the loop-entry snapshot of each place that needs one, assigned FIRST (`lb := null; if (<every inner cell held>)
        // { lb := <its base> }`), before the case splits against it and the `perm` snapshots.
        const size_t lbs_before {active_lbs_.size()};
        std::vector<low::LStmt> lbstmts;
        if (!places.empty())
        {
            const FnAtoms& lfa {atoms_of(*fn_.f)};
            for (int k : atom_params)
            {
                if (!is_derived_key(k) || derived_of(k).lb_of < 0)
                {
                    continue;
                }
                const Derived& d {derived_of(k)};
                const std::string nm {lb_name(d)};
                fn_.snapshots.push_back(low::LVar {nm, low::LType::Ref});
                assign(lbstmts, nm, low::null_());
                E cond;
                const E val {lb_chain(derived_of(d.lb_of).base, cond)};
                low::LStmt ifs;
                ifs.kind = low::LStmt::Kind::If;
                ifs.expr = cond ? cond : low::bool_lit(true);
                assign(ifs.then_body, nm, val);
                lbstmts.push_back(std::move(ifs));
                for (int i {0}; i < lfa.nreq; ++i)
                {
                    if (key_has_site(lfa.sites[static_cast<size_t>(i)], k))
                    {
                        active_lbs_.push_back(
                            LbPlace {&lfa.sites[static_cast<size_t>(i)],
                                     chain_ref(low::var(nm), lfa.sites[static_cast<size_t>(i)].path)});
                    }
                }
            }
        }

        // A condition with a side effect, a call or a memory access is evaluated inside the body, at the top (the `do` shape of 0014 item 5 moved
        // up): the invariants and the measure stay where C evaluates the condition, and its obligations are asserted at every evaluation.
        const bool top_test {!is_do && (has_effect(*s.expr) || expr_touches(s.expr))};
        for (int id : assigned)
        {
            if (declared.count(id) != 0U)
            {
                continue;
            }
            const Var& v {*fn_.f->locals()[static_cast<size_t>(id)]};
            if (!v.type.is_int() || v.address_taken)
            {
                continue; // a cell is a Ref: its range is the place's conjunct (decision 0038 item 3)
            }
            const dec::Range r {dec::range_of(*v.type.int_type)};
            low::LInvariant inv;
            inv.expr = low::in_range(low::var(var_name(v)), r.lo, opt_.weaken_auto_invariant ? r.lo : r.hi);
            for (const char* id2 : {"invariant.not.established", "invariant.not.preserved"})
            {
                low::Meta m;
                m.category = "internal";
                m.loc = s.loc;
                m.message = "internal check: automatic range invariant of `" + v.name + "`";
                inv.checks.push_back(low::Check {id2, std::move(m)});
            }
            w.invariants.push_back(std::move(inv));
        }

        // Decision 0019 item 4: Viper havocs the flag of a variable the loop assigns and that is declared outside it.  A snapshot of
        // the flag before the loop and the invariant `snapshot ==> flag` keep a value given before the loop known inside it.
        for (int id : assigned)
        {
            if (declared.count(id) != 0U || fn_.flagged.count(id) == 0U)
            {
                continue;
            }
            const Var& v {*fn_.f->locals()[static_cast<size_t>(id)]};
            const std::string snap {"ds_" + var_name(v) + "__" + std::to_string(loop_number)};
            fn_.snapshots.push_back(low::LVar {snap, low::LType::Bool});
            assign(out, snap, low::var(flag_name(v)));
            low::LInvariant inv;
            inv.expr = low::implies(low::var(snap), low::var(flag_name(v)));
            for (const char* id2 : {"invariant.not.established", "invariant.not.preserved"})
            {
                low::Meta m;
                m.category = "internal";
                m.loc = s.loc;
                m.message = "internal check: the loop keeps `" + v.name + "` given a value";
                inv.checks.push_back(low::Check {id2, std::move(m)});
            }
            if (!opt_.drop_snapshot_invariant)
            {
                w.invariants.push_back(std::move(inv));
            }
        }

        // Record 0047 item 10 (re-audit S3; heap programs only): a pointer variable the loop assigns and that is declared outside it stays well-formed.  Viper havocs
        // it at the loop head, and the rows after the loop (a `free`'s `WF` row, a block site's at a call) read it: without this conjunct they fail for a correct
        // program, and the free row's `WF` row (`block_rows`) is `internal` again.  Every assignment of a pointer is well-formed by construction (a view's or a
        // contract's result, `&` of a place, a load under the WF of its place, another pointer variable), a variable with a flag only once it has a value.
        if (heap_)
        {
            for (int id : assigned)
            {
                if (declared.count(id) != 0U)
                {
                    continue;
                }
                const Var& v {*fn_.f->locals()[static_cast<size_t>(id)]};
                if (!v.type.is_ptr() || v.address_taken || v.compound_literal)
                {
                    continue;
                }
                E wf {wf_expr(low::var(var_name(v)))};
                low::LInvariant inv;
                inv.expr =
                    fn_.flagged.count(id) != 0U ? low::implies(low::var(flag_name(v)), std::move(wf)) : std::move(wf);
                for (const char* id2 : {"invariant.not.established", "invariant.not.preserved"})
                {
                    low::Meta m;
                    m.category = "internal";
                    m.loc = s.loc;
                    m.message = "internal check: the loop keeps `" + v.name + "` well-formed";
                    inv.checks.push_back(low::Check {id2, std::move(m)});
                }
                w.invariants.push_back(std::move(inv));
            }
        }

        // The loop's places (decision 0038 item 3): the case splits, the snapshots, then the conjuncts.
        std::vector<low::LStmt> entry; // right before the Viper loop
        std::vector<Framing> framings;
        if (!places.empty())
        {
            using low::BinOp;
            // Silicon must branch on which places are one before it reads their permissions (S28 P3): the pairs of parameter places of one field, the
            // pairs of liveness cells, and a place against every other parameter with an atom of its field.
            std::vector<const Place*> pparams;
            for (const Place& pl : places)
            {
                if (!pl.cell)
                {
                    pparams.push_back(&pl);
                }
            }
            auto kref = [&](int k)
            {
                return key_root_ref(k);
            };
            std::set<int> held_atoms;
            std::set<int> value_params;
            held_keys(held_atoms, value_params);
            // The value cells of the key `k`'s atoms (assigned parameters included), each with its member path: what a parameter place may be one of (decision 0043 item 11).
            auto key_cells = [&](int k)
            {
                struct KC
                {
                    E recv;
                    Type type;
                    std::vector<int> full;
                };
                std::vector<KC> out_cells;
                if (key_is_global(k) && value_params.count(k) != 0U && key_pointee(k).is_struct() &&
                    fn_.f == prog_.entry())
                { // decision 0044 item 8: every leaf of a struct global
                    for (Cell& c : leaf_cells(kref(k), *key_pointee(k).struct_type, std::string(), false))
                    {
                        out_cells.push_back(KC {c.recv, c.type, c.path});
                    }
                    return out_cells;
                }
                if (key_is_global(k) && value_params.count(k) != 0U && !key_pointee(k).is_struct())
                { // a global of the entry function: its one cell
                    out_cells.push_back(KC {kref(k), key_pointee(k), {}});
                    return out_cells;
                }
                const FnAtoms& fa2 {atoms_of(*fn_.f)};
                for (int i {0}; i < fa2.nreq; ++i)
                {
                    const AtomSite& site {fa2.sites[static_cast<size_t>(i)]};
                    if (!key_has_site(site, k) || !site_takes_cell(site))
                    {
                        continue;
                    }
                    for (Cell& c : site_cells(site, chain_ref(kref(k), site.path)))
                    {
                        std::vector<int> full {site.path};
                        full.insert(full.end(), c.path.begin(), c.path.end());
                        bool dup {false};
                        for (const KC& x : out_cells)
                        {
                            dup = dup || x.full == full;
                        }
                        if (!dup)
                        {
                            out_cells.push_back(KC {c.recv, c.type, std::move(full)});
                        }
                    }
                }
                return out_cells;
            };
            std::set<std::tuple<int, int, std::vector<int>, std::vector<int>>> split_cells;
            std::set<std::pair<int, int>> split_lives;
            for (const Place* a : pparams)
            {
                for (int k : atom_params)
                {
                    if (k == a->param || (key_is_global(k) && key_is_global(a->param)))
                    {
                        continue; // two globals are two `unique` blocks
                    }
                    if (!a->value || value_params.count(k) == 0U)
                    {
                        continue;
                    }
                    for (const auto& c : key_cells(k))
                    {
                        if (cell_suffix(c.type) != a->suffix)
                        {
                            continue;
                        }
                        if (!split_cells
                                 .insert({std::min(a->param, k), std::max(a->param, k),
                                          a->param < k ? a->mpath : c.full, a->param < k ? c.full : a->mpath})
                                 .second)
                        {
                            continue;
                        }
                        case_split(entry, a->param < k ? low::bin(BinOp::Eq, c.recv, a->recv)
                                                       : low::bin(BinOp::Eq, a->recv, c.recv));
                    }
                }
            }
            for (const Place* a : pparams)
            {
                for (int k : atom_params)
                {
                    if (is_heap_param(a->param))
                    {
                        continue; // a heap place's own splits are below
                    }
                    if (k == a->param || (key_is_global(k) && key_is_global(a->param)))
                    {
                        continue;
                    }
                    const std::pair<int, int> key {std::min(a->param, k), std::max(a->param, k)};
                    if (split_lives.insert(key).second)
                    {
                        case_split(entry, low::bin(BinOp::Eq, live_cell(kref(key.second)), live_cell(kref(key.first))));
                    }
                }
            }
            // Decision 0047 item 10: a heap place against every parameter place and every earlier heap place: Silicon must branch on which liveness cells and which cells are one.
            for (size_t i {0U}; i < pparams.size(); ++i)
            {
                const Place* a {pparams[i]};
                if (!is_heap_param(a->param))
                {
                    continue;
                }
                if (a->live)
                {
                    for (int k : atom_params)
                    {
                        case_split(entry, low::bin(BinOp::Eq, live_cell(kref(k)), live_cell(lv_of(*a))));
                    }
                }
                for (size_t j {0U}; j < i; ++j)
                {
                    const Place* b {pparams[j]};
                    if (!b->heap || same_expr(b->lroot, a->lroot))
                    {
                        continue;
                    }
                    if (a->live && b->live)
                    {
                        case_split(entry, low::bin(BinOp::Eq, live_cell(lv_of(*b)), live_cell(lv_of(*a))));
                    }
                    // B1 (re-audit): two token places are one block or two; the token invariants are guarded by `block_of(hb_i) != block_of(hb_j)`
                    if (!a->kt.empty() && !b->kt.empty())
                    {
                        case_split(entry, low::bin(BinOp::Eq, block_of(b->lroot), block_of(a->lroot)));
                    }
                    if (a->suffix == b->suffix &&
                        (!(a->ptee == b->ptee && a->ptee.is_struct()) || a->mpath == b->mpath))
                    {
                        case_split(entry, low::bin(BinOp::Eq, b->recv, a->recv));
                    }
                }
            }
            // B1 item 3: a heap place against each holder: its liveness cell, its block (for the token) and its cells.
            for (const Place* a : pparams)
            {
                if (!is_heap_param(a->param))
                {
                    continue;
                }
                for (const HeapHolder& h : holders)
                {
                    if (a->live)
                    {
                        case_split(entry, low::bin(BinOp::Eq, live_cell(lv_of(*a)), live_cell(h.ref)));
                    }
                    if (!a->kt.empty())
                    {
                        case_split(entry, low::bin(BinOp::Eq, block_of(a->lroot), block_of(h.ref)));
                    }
                    if (!a->value)
                    {
                        continue;
                    }
                    for (const Cell& c : h.cells)
                    {
                        if (a->suffix == cell_suffix(c.type) &&
                            (!(a->ptee == h.pointee && a->ptee.is_struct()) || a->mpath == c.path))
                        {
                            case_split(entry, low::bin(BinOp::Eq, c.recv, a->recv));
                        }
                    }
                }
            }
            // A parameter place against each cell (B1): its snapshot must not pick up a cell's permission.
            for (const Place* a : pparams)
            {
                for (const Place& c : places)
                {
                    if (!c.cell)
                    {
                        continue;
                    }
                    if (a->value && c.suffix == a->suffix)
                    {
                        case_split(entry, low::bin(BinOp::Eq, a->recv, c.recv));
                    }
                }
            }
            for (const Place* a : pparams)
            {
                for (const Place& c : places)
                {
                    if (c.cell && a->live && c.live)
                    {
                        case_split(entry, low::bin(BinOp::Eq, live_cell(lv_of(*a)), live_cell(lv_of(c))));
                    }
                }
            }
            // Record 0047 item 10 (re-audit B1): a heap place's snapshot holds a symbolic permission, and Silicon cannot decide `k == none` for a holder it never
            // branched on (a member address `&s->b`, a pointer reached through a load, a cursor parameter): the invariant's `acc(.., k)` then fails on entry
            // ("insufficient permission") for a correct program.  A catch-all `if (k > none) { }` after each heap snapshot gives it the branch.
            auto catch_all = [&](const Place& pl, const std::string& k)
            {
                if (pl.heap)
                {
                    case_split(entry, low::bin(BinOp::Gt, low::var(k), low::no_perm()));
                }
            };
            for (const Place& pl : places)
            {
                if (pl.live)
                {
                    fn_.snapshots.push_back(low::LVar {pl.kl, low::LType::Perm});
                    assign(entry, pl.kl, live_perm(lv_of(pl)));
                    catch_all(pl, pl.kl);
                }
                if (!pl.kt.empty())
                { // the block's free token (decision 0047 item 10)
                    fn_.snapshots.push_back(low::LVar {pl.kt, low::LType::Perm});
                    assign(entry, pl.kt, low::perm_of(tok_slot(pl.lroot), "tok"));
                    catch_all(pl, pl.kt);
                    if (!pl.ktv.empty())
                    { // decision 0050 item 5: the block's allocator (the token's value), when the token is held at all
                        fn_.snapshots.push_back(low::LVar {pl.ktv, low::LType::Bool});
                        assign(entry, pl.ktv, low::bool_lit(false));
                        low::LStmt tv;
                        tv.kind = low::LStmt::Kind::If;
                        tv.expr = low::bin(BinOp::Gt, low::var(pl.kt), low::no_perm());
                        assign(tv.then_body, pl.ktv, low::field(tok_slot(pl.lroot), "tok"));
                        entry.push_back(std::move(tv));
                    }
                }
                if (!pl.value)
                {
                    continue;
                }
                fn_.snapshots.push_back(low::LVar {pl.kv, low::LType::Perm});
                fn_.snapshots.push_back(low::LVar {pl.kd, low::LType::Perm});
                fn_.snapshots.push_back(low::LVar {pl.vs, ltype(pl.type)});
                fn_.snapshots.push_back(low::LVar {pl.ds, low::LType::Bool});
                assign(entry, pl.kv, value_perm(pl.recv, pl.type));
                catch_all(pl, pl.kv);
                assign(entry, pl.kd, flag_perm(pl.recv, pl.type));
                catch_all(pl, pl.kd);
                assign(entry, pl.ds, low::bool_lit(false));
                low::LStmt fl;
                fl.kind = low::LStmt::Kind::If;
                fl.expr = low::bin(BinOp::Gt, low::var(pl.kd), low::no_perm());
                assign(fl.then_body, pl.ds, cell_flag(pl.recv, pl.type));
                entry.push_back(std::move(fl));
                low::LStmt vl;
                vl.kind = low::LStmt::Kind::If;
                vl.expr = low::bin(BinOp::Gt, low::var(pl.kv), low::no_perm());
                assign(vl.then_body, pl.vs, cell_value(pl.recv, pl.type));
                entry.push_back(std::move(vl));
            }
            // The conjuncts: every `acc` first (so every read after it is framed), then the values and the flags.
            const E none {low::no_perm()};
            const E write {low::write_perm()};
            for (const Place& pl : places)
            {
                if (pl.value)
                {
                    E acc {low::bin(BinOp::And, low::acc(pl.recv, "v_" + pl.suffix, low::var(pl.kv)),
                                    low::acc(pl.recv, "d_" + pl.suffix, low::var(pl.kd)))};
                    w.invariants.push_back(auto_invariant(low::implies(pl.gv, acc), s.loc));
                }
                if (pl.live)
                {
                    w.invariants.push_back(auto_invariant(
                        low::implies(pl.gl, low::acc(live_cell(lv_of(pl)), "lv", low::var(pl.kl))), s.loc));
                }
                if (!pl.kt.empty())
                {
                    w.invariants.push_back(auto_invariant(
                        low::implies(pl.gt, low::acc(tok_slot(pl.lroot), "tok", low::var(pl.kt))), s.loc));
                    // 0047 item 10's conjunct is `tok`; in a vlibc heap program it is REPLACED (kept beside it, it fails on entry for a vlibc block, audit S1 of 0050) by the stable value
                    const E held {low::and_(pl.gt, low::bin(BinOp::Gt, low::var(pl.kt), none))};
                    const E tok {low::field(tok_slot(pl.lroot), "tok")};
                    w.invariants.push_back(auto_invariant(
                        low::implies(held, pl.ktv.empty() ? tok : low::bin(BinOp::Eq, tok, low::var(pl.ktv))), s.loc));
                }
            }
            for (const Place& pl : places)
            {
                if (!pl.value)
                {
                    continue;
                }
                const E kv {low::var(pl.kv)};
                const E kd {low::var(pl.kd)};
                const E v {cell_value(pl.recv, pl.type)};
                const E same {low::bin(BinOp::Eq, v, low::var(pl.vs))};
                // A parameter place's rows are guarded by its `gv` like its `acc` (false only for a null place, or one that is the same Ref as an earlier
                // place of the field, whose own rows constrain that location with the same snapshot); a cell's `gv` is null (audit re-audit of B1).
                auto rows = [&](const E& antecedent)
                {
                    return low::and_(pl.gv, antecedent);
                };
                if (!pl.reach)
                {
                    w.invariants.push_back(
                        auto_invariant(low::implies(rows(low::bin(BinOp::Gt, kv, none)), same), s.loc));
                }
                else
                {
                    w.invariants.push_back(auto_invariant(
                        low::implies(rows(low::and_(low::bin(BinOp::Lt, none, kv), low::bin(BinOp::Lt, kv, write))),
                                     same),
                        s.loc));
                }
                // Every value place keeps `held && flag ==> range`, one the loop reaches or not (owner question 89, audit round 2; decision 0042 item 5): a
                // place the loop does not reach keeps only its value, so after the loop a cell whose flag was false on entry could hold `d` with an out-of-range
                // value, and a `flag ==> range` row after the loop (a marked parameter's call or return rows) would fail on a correct program; and an outer
                // loop's row would be lost across an inner loop that only reads.  Inductive: a write is range-checked, and an uninitialized cell satisfies it
                // vacuously.
                if (const E range = cell_range_inv(pl.recv, pl.type))
                {
                    const E held {low::and_(low::and_(low::bin(BinOp::Gt, kv, none), low::bin(BinOp::Gt, kd, none)),
                                            cell_flag(pl.recv, pl.type))};
                    w.invariants.push_back(auto_invariant(low::implies(rows(held), range), s.loc));
                }
                w.invariants.push_back(
                    auto_invariant(low::implies(rows(low::and_(low::bin(BinOp::Gt, kd, none), low::var(pl.ds))),
                                                cell_flag(pl.recv, pl.type)),
                                   s.loc));
            }
        }

        // the loop's places, then the written atoms' (the framing chains of decision 0038 item 6)
        std::vector<Place> inv_all {places};
        if (!wats.empty())
        {
            written_splits(wats, places, wsplits);
            written_splits(wats, places, bsplits, true);
            entry.insert(entry.begin(), wsplits.begin(), wsplits.end());
            written_amounts(wats, entry);
            written_values(wats, places, entry);
            written_checks(wats, places, false, s.loc, entry);
            written_rows(wats, places, s.loc, w.invariants);
            for (const WAtom& a : wats)
            {
                if (a.kind == AtomKind::Live)
                {
                    continue;
                }
                Place wp;
                wp.recv = a.q;
                wp.type = a.pt;
                wp.value = true;
                wp.suffix = a.sfx;
                wp.label = a.text;
                wp.kv = a.av;
                wp.kd = a.ad;
                wp.kl = a.al;
                wp.wg = a.g;
                inv_all.push_back(std::move(wp));
            }
        }

        E prior; // the conjunction of the user invariants so far
        // Invariants and the measure are lowered in `Inv` mode (decision 0038 item 6): the reads of memory in them are chains over the loop's places.
        const std::vector<Place>* saved_places {inv_places_};
        inv_places_ = &inv_all;
        for (const Clause& c : s.invariants)
        {
            Full full;
            LV v;
            {
                CtxSave save(*this);
                cmode_ = CMode::Inv;
                cf_ = fn_.f;
                inv_reads_.clear();
                inv_what_ = "invariant";
                v = lower(*c.expr, body_env(), nullptr, {}, full);
            }
            for (const InvRead& r : inv_reads_)
            {
                low::Meta m;
                m.category = "permission";
                m.loc = r.loc.known() ? r.loc : c.loc;
                m.message = r.cell != nullptr
                                ? "this loop invariant reads `" + r.cell->name + "`, whose object a callee kept"
                                : "this loop invariant reads " + quoted(r.text, "an object") +
                                      ", which this loop does not hold";
                framings.push_back(Framing {framing_condition(r, prior), std::move(m)});
            }
            inv_reads_.clear();
            pure_full(full, *c.expr, "a loop invariant");
            for (Obl& o : full.obls)
            {
                w.invariants.push_back(obligation_clause(low::and_(prior, o.guard), o));
            }
            low::LInvariant inv;
            inv.expr = v.e;
            low::Meta entry_meta;
            entry_meta.category = "invariant-entry";
            entry_meta.loc = c.loc;
            entry_meta.message = "this loop invariant may not hold on entry";
            low::Meta kept;
            kept.category = "invariant-preserved";
            kept.loc = c.loc;
            kept.message = "this loop invariant may not be preserved by an iteration";
            inv.checks.push_back(low::Check {"invariant.not.established", std::move(entry_meta)});
            inv.checks.push_back(low::Check {"invariant.not.preserved", std::move(kept)});
            w.invariants.push_back(std::move(inv));
            prior = low::and_(prior, v.e);
        }

        // The measure (decision 0013): its own obligations are invariant clauses at the loop head, like the condition's
        // (the measure is evaluated where the condition is, and at the end of each iteration); the measure itself is the
        // loop's `decreases`, one line, which Silicon reports a failure at.
        for (const Clause& c : s.measure)
        {
            Full mfull;
            LV mv;
            {
                CtxSave save(*this);
                cmode_ = CMode::Inv;
                cf_ = fn_.f;
                inv_reads_.clear();
                inv_what_ = "measure";
                mv = lower(*c.expr, body_env(), nullptr, {}, mfull);
            }
            for (const InvRead& r : inv_reads_)
            {
                low::Meta m;
                m.category = "permission";
                m.loc = r.loc.known() ? r.loc : c.loc;
                m.message =
                    r.cell != nullptr
                        ? "this loop measure reads `" + r.cell->name + "`, whose object a callee kept"
                        : "this loop measure reads " + quoted(r.text, "an object") + ", which this loop does not hold";
                framings.push_back(Framing {framing_condition(r, prior), std::move(m)});
            }
            inv_reads_.clear();
            pure_full(mfull, *c.expr, "a loop measure");
            for (Obl& o : mfull.obls)
            {
                w.invariants.push_back(obligation_clause(low::and_(prior, o.guard), o));
            }
            w.decreases.push_back(mv.e);
        }
        inv_places_ = saved_places;
        if (!s.measure.empty())
        {
            low::Meta m;
            m.category = "termination";
            m.loc = s.measure.front().loc.known() ? s.measure.front().loc : s.loc;
            m.message = "the measure of this loop may not decrease, or may not be bounded below by zero";
            w.decreases_check = low::Check {"termination.failed", std::move(m)};
        }

        // Decision 0038 item 6: a pointer the loop assigns is havocked at the loop head; Silicon does not branch on a disjunction an invariant gives
        // (`p == &x || p == &y`), so the no-op case splits `if (p == X) { }` against each place of its field come first in the body and after the loop.
        std::vector<low::LStmt> splits;
        if (!places.empty())
        {
            for (int id : assigned)
            {
                if (declared.count(id) != 0U)
                {
                    continue;
                }
                const Var& v {*fn_.f->locals()[static_cast<size_t>(id)]};
                if (!v.type.is_ptr() || v.type.pointee == nullptr || v.type.pointee->is_void())
                {
                    continue;
                }
                const std::string sfx {cell_suffix(*v.type.pointee)};
                for (const Place& pl : places)
                {
                    if (v.type.pointee->is_struct())
                    { // a pointer to a struct may be a slot of any object the loop frames (decision 0043 item 11)
                        std::vector<E> cands;
                        for (const auto& sl : pl.slots)
                        {
                            split_candidates(sl.second, sl.first, *v.type.pointee, cands);
                        }
                        for (const E& c : cands)
                        {
                            case_split(splits, low::bin(low::BinOp::Eq, low::var(var_name(v)), c));
                        }
                    }
                    else if (pl.suffix == sfx)
                    {
                        case_split(splits, low::bin(low::BinOp::Eq, low::var(var_name(v)), pl.recv));
                    }
                    // Decision 0050 item 5 (a vlibc heap program): the value row of a free, a call or a back edge reads the token's value of a pointer the loop reassigned; Silicon
                    // must know which token place it is (or none) before the read, else the read is its own permission error (an exit 3 instead of a row).
                    if (vlibc_heap_ && pl.heap && !pl.kt.empty())
                    {
                        case_split_once(splits, low::bin(low::BinOp::Eq, low::var(var_name(v)), pl.lroot));
                        case_split_once(splits,
                                        low::bin(low::BinOp::Eq, block_of(low::var(var_name(v))), block_of(pl.lroot)));
                    }
                }
            }
        }

        // The framing of the invariants and the measure, asserted right after the snapshots (decision 0038 item 6).
        for (const Framing& f : framings)
        {
            emit_assert(entry, f.cond, f.meta);
        }
        entry.insert(entry.begin(), lbstmts.begin(), lbstmts.end());
        entry.insert(entry.begin(), hbstmts.begin(), hbstmts.end());
        for (low::LStmt& st : entry)
        {
            out.push_back(std::move(st));
        }

        std::set<int> breaks;
        std::set<int> continues;
        collect_jumps(*s.body, breaks, continues);
        const std::string n {std::to_string(s.jump_id)};
        // The ghost of each written atom that loads, taken at the top of the iteration (decision 0045 item 12): the body's splits name the atom's object by it.
        for (const WAtom& a : wats)
        {
            if (a.loads)
            {
                assign(w.then_body, a.aq, a.q);
            }
        }
        for (const low::LStmt& st : bsplits)
        {
            w.then_body.push_back(st);
        }
        for (const low::LStmt& st : splits)
        {
            w.then_body.push_back(st);
        }
        if (top_test)
        {
            w.expr = low::bool_lit(true);
            LV cv {full_expr(*s.expr, w.then_body)};
            low::LStmt exit;
            exit.kind = low::LStmt::Kind::If;
            exit.expr = low::not_(cv.e);
            jump(exit.then_body, "l_brk__" + n);
            w.then_body.push_back(std::move(exit));
        }
        else if (!is_do)
        {
            Full cfull;
            LV cv {lower(*s.expr, body_env(), nullptr, {}, cfull)};
            pure_full(cfull, *s.expr, "a loop condition");
            for (Obl& o : cfull.obls)
            {
                w.invariants.push_back(obligation_clause(low::and_(prior, o.guard), o));
            }
            w.expr = cv.e;
        }
        else
        {
            w.expr = low::bool_lit(true);
        }
        stmt(*s.body, w.then_body);
        if (continues.count(s.jump_id) != 0U)
        {
            label(w.then_body, "l_cont__" + n);
        }
        if (s.step != nullptr)
        {
            stmt(*s.step, w.then_body);
        }
        if (is_do)
        {
            // C evaluates the condition here, after the body and after each `continue`; its obligations are asserts.
            LV cv {full_expr(*s.expr, w.then_body)};
            low::LStmt exit;
            exit.kind = low::LStmt::Kind::If;
            exit.expr = low::not_(cv.e);
            jump(exit.then_body, "l_brk__" + n);
            w.then_body.push_back(std::move(exit));
        }
        // The back edge (decision 0038 items 4 and 6): the loop still holds what it began with, then the invariants' framing.
        if (!places.empty())
        {
            using low::BinOp;
            const SrcLoc at {s.body != nullptr && s.body->end_loc.known() ? s.body->end_loc : s.loc};
            for (const Place& pl : places)
            {
                const std::string msg {"an iteration of this loop ends without the permission to " +
                                       quoted(pl.label, "an object") + " that it began with"};
                auto lacks = [&](const E& have, const std::string& amount)
                {
                    return low::bin(BinOp::Lt, have, low::var(amount));
                };
                const bool one {pl.live && !pl.gv && !pl.gl};
                // A heap place's rows end their path (see emit_ending_assert); the other places' rows keep today's form.
                auto row = [&](E cond, low::Meta m)
                {
                    if (pl.heap)
                    {
                        emit_ending_assert(w.then_body, cond, std::move(m));
                    }
                    else
                    {
                        emit_assert(w.then_body, std::move(cond), std::move(m));
                    }
                };
                if (pl.value)
                {
                    E bad {low::bin(BinOp::Or, lacks(value_perm(pl.recv, pl.type), pl.kv),
                                    lacks(flag_perm(pl.recv, pl.type), pl.kd))};
                    if (one)
                    {
                        bad = low::bin(BinOp::Or, bad, lacks(live_perm(lv_of(pl)), pl.kl));
                    }
                    row(low::implies(pl.gv, low::not_(bad)), meta("permission", at, msg));
                }
                if (pl.live && (!one || !pl.value))
                {
                    row(low::implies(pl.gl, low::not_(lacks(live_perm(lv_of(pl)), pl.kl))),
                        meta("permission", at, msg));
                }
                // the block's token (decision 0047 item 10): a body that frees the block the loop holds fails here,
                // before Viper exhales the invariant
                if (!pl.kt.empty())
                {
                    row(low::implies(
                            pl.gt,
                            low::not_(
                                pl.ktv.empty()
                                    ? lacks(low::perm_of(tok_slot(pl.lroot), "tok"), pl.kt)
                                    // decision 0050 item 5: also a block of the OTHER allocator in the variable (the body freed it and allocated again)
                                    : low::bin(BinOp::Or, lacks(low::perm_of(tok_slot(pl.lroot), "tok"), pl.kt),
                                               low::and_(low::bin(BinOp::Gt, low::var(pl.kt), low::no_perm()),
                                                         low::bin(BinOp::Ne, low::field(tok_slot(pl.lroot), "tok"),
                                                                  low::var(pl.ktv)))))),
                        meta("permission", at,
                             "an iteration of this loop ends without the allocation " + quoted(pl.label, "an object") +
                                 " that it began with"));
                }
                // A heap place's VALUE invariants have a user row of their own (M4.5d2v, re-audit of 0047 item 10): the body may free the block and a later allocation may be
                // the same block again (Silicon has no freshness), so the permission rows above hold and the place holds a different object; its flag, its value and its range
                // are then not what the invariants (`internal`) say.  Each row mirrors one invariant, after the permission rows (which end their path when they fail, so a read
                // here holds the permission the row before it checked).
                if (pl.heap && pl.value)
                {
                    const E none {low::no_perm()};
                    const E write {low::write_perm()};
                    const E kv {low::var(pl.kv)};
                    const E kd {low::var(pl.kd)};
                    const E v {cell_value(pl.recv, pl.type)};
                    const std::string vmsg {"an iteration of this loop ends with " + quoted(pl.label, "an object") +
                                            " holding something other than what it began with"};
                    emit_assert(
                        w.then_body,
                        low::implies(low::and_(pl.gv, low::and_(low::bin(BinOp::Gt, kd, none), low::var(pl.ds))),
                                     cell_flag(pl.recv, pl.type)),
                        meta("permission", at, vmsg));
                    const E part {pl.reach ? low::and_(low::bin(BinOp::Lt, none, kv), low::bin(BinOp::Lt, kv, write))
                                           : low::bin(BinOp::Gt, kv, none)};
                    emit_assert(w.then_body,
                                low::implies(low::and_(pl.gv, part), low::bin(BinOp::Eq, v, low::var(pl.vs))),
                                meta("permission", at, vmsg));
                    if (const E range = cell_range_inv(pl.recv, pl.type))
                    {
                        const E held {low::and_(low::and_(low::bin(BinOp::Gt, kv, none), low::bin(BinOp::Gt, kd, none)),
                                                cell_flag(pl.recv, pl.type))};
                        emit_assert(w.then_body, low::implies(low::and_(pl.gv, held), range),
                                    meta("permission", at, vmsg));
                    }
                }
            }
        }
        // Decision 0047 item 7 (audit B1 (a)): a block allocated in an iteration and not freed would be dropped by the loop's invariant exhale at the back edge, with no row.
        // One `leak` row at the end of the body, where the fall-through and every `continue` have landed (`l_cont__` is inside the body): exempt exactly the tokens the
        // invariant carries.  Only a loop whose body can make a token has the row (a call of a function with an `ensures` block site: a view, an owning function).
        if (heap_ && loop_may_allocate(s))
        {
            emit_assert(
                w.then_body, low::for_perm(loop_token_exemption(places)),
                meta(
                    "leak", s.body != nullptr && s.body->end_loc.known() ? s.body->end_loc : s.loc,
                    "an iteration of this loop may end while it still owns an allocation that the loop does not carry to the next iteration"));
        }
        if (!wats.empty())
        {
            written_checks(wats, places, true, s.body != nullptr && s.body->end_loc.known() ? s.body->end_loc : s.loc,
                           w.then_body);
        }
        for (const Framing& f : framings)
        {
            emit_assert(w.then_body, f.cond, f.meta); // also when the loop has no place: the read is then unframed
        }
        out.push_back(std::move(w));
        for (const low::LStmt& st : wsplits)
        {
            out.push_back(st);
        }
        if (is_do || top_test || breaks.count(s.jump_id) != 0U)
        {
            label(out, "l_brk__" + n);
        }
        for (const low::LStmt& st : wsplits)
        {
            out.push_back(st);
        }
        for (low::LStmt& st : splits)
        {
            out.push_back(std::move(st));
        }
        active_w_.resize(active_before);
        active_lbs_.resize(lbs_before);
        cur_loop_ = saved_loop;
    }

    // the `param` of a heap place (decision 0047 item 10): never a real key
    static constexpr int kHeapPlaceBase {-2000000};
    static bool is_heap_param(int k)
    {
        return k <= kHeapPlaceBase;
    }
    std::vector<low::LStmt> heap_entry_; // `hb := v` of the loop being lowered (loop_places fills it)
    // Record 0047 item 10 (re-audit B1, item 3): a pointer variable that holds a block (or a member of one) beside a loop's heap places without being one of them: a local
    // declared before the loop that the loop does not use, and the formal of a parameter with an `owns_block` site.  Silicon must branch on whether a heap place is
    // one of its cells, its liveness cell or its block before it reads the place's permissions; the places' own splits cover only the places.
    struct HeapHolder
    {
        E ref;
        Type pointee;
        std::vector<Cell> cells;
    };
    std::vector<HeapHolder> heap_holders_; // loop_places fills it for the loop being lowered
    // The locals (Var ids) a statement or expression reads or writes by name.
    static void collect_local_uses_expr(const Expr* e, std::set<int>& out)
    {
        if (e == nullptr)
        {
            return;
        }
        if (e->var != nullptr)
        {
            out.insert(e->var->id);
        }
        for (const Expr* x : {e->a, e->b, e->c})
        {
            collect_local_uses_expr(x, out);
        }
        for (const Expr* x : e->args)
        {
            collect_local_uses_expr(x, out);
        }
    }
    static void collect_local_uses(const Stmt* s, std::set<int>& out)
    {
        if (s == nullptr)
        {
            return;
        }
        if (s->var != nullptr)
        {
            out.insert(s->var->id);
        }
        collect_local_uses_expr(s->expr, out);
        collect_local_uses_expr(s->addr, out);
        for (const Clause& c : s->invariants)
        {
            collect_local_uses_expr(c.expr, out);
        }
        for (const Clause& c : s->measure)
        {
            collect_local_uses_expr(c.expr, out);
        }
        collect_local_uses(s->then_body, out);
        collect_local_uses(s->else_body, out);
        collect_local_uses(s->body, out);
        collect_local_uses(s->step, out);
        for (const Stmt* c : s->stmts)
        {
            collect_local_uses(c, out);
        }
    }
    // Whether a loop's body (condition and step included) can call a function that hands out a block (an `ensures` block site).
    bool loop_may_allocate(const Stmt& s)
    {
        std::vector<const Function*> cs;
        calls_in_stmt(&s, cs, false);
        for (const Function* c : cs)
        {
            for (const AtomSite& b : blocks_of(*c).sites)
            {
                if (b.in_ensures)
                {
                    return true;
                }
            }
        }
        return false;
    }
    // The tokens the loop's invariant carries (item 10: the token places of the loop's places), or `false`.
    E loop_token_exemption(const std::vector<Place>& places)
    {
        E r;
        for (const Place& pl : places)
        {
            if (!pl.heap || pl.kt.empty())
            {
                continue;
            }
            E term {low::and_(pl.gt, low::bin(low::BinOp::Gt, low::var(pl.kt), low::no_perm()))};
            term = low::and_(term, low::bin(low::BinOp::Eq, low::var("t"), tok_slot(pl.lroot)));
            r = r ? low::bin(low::BinOp::Or, r, term) : term;
        }
        return r ? r : low::bool_lit(false);
    }

    // The range of a cell's value as an invariant conjunct (none for `bool`).
    E cell_range_inv(const E& p, const Type& t)
    {
        if (t.is_ptr())
        {
            return wf_expr(cell_value(p, t), &t);
        }
        if (!t.is_bool())
        {
            return range_check(cell_value(p, t), *t.int_type);
        }
        return nullptr;
    }

    // `do { B } while (c)` where `c` is a constant zero (decision 0014 item 5): the body runs once and no iteration follows
    // another, so there is no loop and no measure.  A written invariant is asserted once, at entry (the start of the only
    // iteration), a written measure only has its obligations checked there; `continue` goes to the condition, which is evaluated
    // (for its obligations) after the body, and `break` to the end.
    void do_once(const Stmt& s, std::vector<low::LStmt>& out)
    {
        E prior;
        for (const Clause& c : s.invariants)
        {
            Full full;
            LV v {lower(*c.expr, body_env(), nullptr, {}, full)};
            pure_full(full, *c.expr, "a loop invariant");
            for (Obl& o : full.obls)
            {
                emit_assert(out, low::implies(low::and_(prior, o.guard), o.cond), o.meta);
            }
            low::Meta entry;
            entry.category = "invariant-entry";
            entry.loc = c.loc;
            entry.message = "this loop invariant may not hold on entry";
            emit_assert(out, v.e, std::move(entry));
            prior = low::and_(prior, v.e);
        }
        for (const Clause& c : s.measure)
        {
            Full mfull;
            lower(*c.expr, body_env(), nullptr, {}, mfull);
            pure_full(mfull, *c.expr, "a loop measure");
            for (Obl& o : mfull.obls)
            {
                emit_assert(out, low::implies(low::and_(prior, o.guard), o.cond), o.meta);
            }
        }
        std::set<int> breaks;
        std::set<int> continues;
        collect_jumps(*s.body, breaks, continues);
        const std::string n {std::to_string(s.jump_id)};
        stmt(*s.body, out);
        if (continues.count(s.jump_id) != 0U)
        {
            label(out, "l_cont__" + n);
        }
        LV cv {full_expr(*s.expr, out)}; // the condition's obligations, where C evaluates it
        // Defense in depth: the body ran once because the condition is false.  If EDG's folded zero ever disagrees with the value
        // cerastes encoded, the run fails (an internal check, exit 3) instead of silently following the wrong path.
        low::Meta internal;
        internal.category = "internal";
        internal.loc = s.loc;
        internal.message = "internal check: the condition of a do-once loop is not false";
        emit_assert(out, low::not_(cv.e), std::move(internal));
        if (breaks.count(s.jump_id) != 0U)
        {
            label(out, "l_brk__" + n);
        }
    }

    low::LInvariant obligation_clause(const E& guard, const Obl& o)
    {
        low::LInvariant inv;
        inv.expr = low::implies(guard, o.cond);
        for (const char* id : {"invariant.not.established", "invariant.not.preserved"})
        {
            inv.checks.push_back(low::Check {id, o.meta});
        }
        return inv;
    }

    // ---- the prologue (decision 0017 items 4 and 5) ----

    static bool bare_const_equals(const Expr& x, const std::string& value)
    {
        if (x.kind != Expr::Kind::Const)
        {
            return false;
        }
        if (x.type.is_bool())
        {
            return (x.value == "true") == (value == "1" || value == "true");
        }
        return x.value == value;
    }

    static E literal_of(const Type& t, const std::string& value)
    {
        if (t.is_bool())
        {
            return low::bool_lit(value == "1" || value == "true");
        }
        return low::int_lit(value);
    }

    // `assert <e> == <value>` as an internal check at `loc` (a failure is a bug of the encoder or of the front end's folding).
    void internal_guard(std::vector<low::LStmt>& out, const E& e, const Type& t, const std::string& value,
                        const SrcLoc& loc, std::string message)
    {
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
    bool closed_expr(const Expr& e, const char* what)
    {
        switch (e.kind)
        {
            case Expr::Kind::Call:
            case Expr::Kind::Formal:
            case Expr::Kind::Old:
            case Expr::Kind::Result:
            case Expr::Kind::Local:
            case Expr::Kind::AssignE:
            case Expr::Kind::Comma:
                internal_error(e.loc, std::string("a call, variable, parameter, result, `old` or side effect in ") +
                                          what + " reached the prologue");
                return false;
            default:
                break;
        }
        for (const Expr* x : {e.a, e.b, e.c})
        {
            if (x != nullptr && !closed_expr(*x, what))
            {
                return false;
            }
        }
        return true;
    }

    // A Full that is lowered only for its obligations (a contract, an invariant, a measure, a pure loop condition): it must hold no effect,
    // or the effect would be dropped.
    void pure_full(const Full& full, const Expr& e, const char* what)
    {
        if (!full.effects.empty())
        {
            internal_error(e.loc, std::string("a call or side effect in ") + what +
                                      " reached lowering outside a full expression");
        }
    }

    bool closed_result(const Full& full, const Expr& e, const char* what)
    {
        if (full.effects.empty())
        {
            return true;
        }
        internal_error(e.loc, std::string("a call in ") + what + " reached the prologue");
        return false;
    }

    // `guard`: also assert that the initializer as written equals the front end's value (false for a global whose value is unknown, decision 0041 item 5).
    void named_const_check(const NamedConst& c, std::vector<low::LStmt>& out, bool guard = true)
    {
        if (c.init == nullptr)
        {
            return;
        }
        // A static assertion (decision 0024 item 2) is checked like a named constant whose value is true: the condition's obligations,
        // then the guard that it holds (skipped for the bare literal `true`).
        const bool assertion {c.kind == NamedConst::Kind::StaticAssertion};
        const char* what {assertion ? "the condition of a static assertion" : "the initializer of a named constant"};
        if (!closed_expr(*c.init, what))
        {
            return;
        }
        Full full;
        LV v {lower(*c.init, body_env(), nullptr, {}, full)};
        if (!closed_result(full, *c.init, what))
        {
            return;
        }
        for (Obl& o : full.obls)
        {
            emit_assert(out, low::implies(o.guard, o.cond), o.meta);
        }
        if (!guard || bare_const_equals(*c.init, c.value))
        {
            return;
        }
        internal_guard(
            out, v.e, c.type, c.value, c.loc,
            assertion
                ? std::string(
                      "internal check: static assertion condition as written differs from the front end's (it holds)")
                : "internal check: the value of `" + c.c_name + "` as written differs from the front end's");
    }

    // One `case value:` label: the value as written, its obligations at the label (with a note when an obligation's own position
    // is elsewhere, as in decision 0014 item 6), then the guard that it equals the front end's value.
    void case_check(const Stmt& c, std::vector<low::LStmt>& out)
    {
        if (!closed_expr(*c.expr, "a case value"))
        {
            return;
        }
        Full full;
        LV v {lower(*c.expr, body_env(), nullptr, {}, full)};
        if (!closed_result(full, *c.expr, "a case value"))
        {
            return;
        }
        for (Obl& o : full.obls)
        {
            low::Meta m {o.meta};
            if (!m.has_note && m.loc.known() && (m.loc.line != c.loc.line || m.loc.file != c.loc.file))
            {
                m.has_note = true;
                m.note_loc = m.loc;
                m.note_text = "in the value of this case label";
            }
            m.loc = c.loc;
            emit_assert(out, low::implies(o.guard, o.cond), std::move(m));
        }
        if (bare_const_equals(*c.expr, c.folded))
        {
            return;
        }
        internal_guard(out, v.e, c.expr->type, c.folded, c.loc,
                       "internal check: case value as written differs from the front end's");
    }

    // The case labels of a body in source order (a nested switch's labels follow the outer label before them).
    void case_checks(const Stmt* s, std::vector<low::LStmt>& out)
    {
        if (s == nullptr)
        {
            return;
        }
        switch (s->kind)
        {
            case Stmt::Kind::Case:
                if (s->expr != nullptr)
                {
                    case_check(*s, out);
                }
                break;
            case Stmt::Kind::Block:
                for (const Stmt* c : s->stmts)
                {
                    case_checks(c, out);
                }
                break;
            case Stmt::Kind::If:
                case_checks(s->then_body, out);
                case_checks(s->else_body, out);
                break;
            case Stmt::Kind::While:
                case_checks(s->body, out);
                case_checks(s->step, out);
                break;
            case Stmt::Kind::Switch:
                case_checks(s->body, out);
                break;
            default:
                break;
        }
    }

    // The prologue of a method with a body: the unit's named constants, then (with `own`: a function's method and its
    // `__wf` method) its local constants and its case values.
    void prologue(std::vector<low::LStmt>& out, const Function* own)
    {
        for (const NamedConst& c : prog_.named_consts())
        {
            named_const_check(c, out);
        }
        // Decision 0041 item 5: a global's initializer as written is checked like a named constant's (a mutable global is never folded at its uses).
        for (const Global& g : prog_.globals())
        {
            if (g.type.is_struct())
            { // decision 0044 item 8: each leaf's initializer element as written, named `g.x`, checked like a scalar's
                const std::vector<StructLeaf> ls {struct_leaves(*g.type.struct_type, g.c_name)};
                for (size_t i {0U}; i < ls.size() && i < g.leaves.size(); ++i)
                {
                    const GlobalLeaf& gl {g.leaves[i]};
                    if ((gl.kind != GlobalInit::Value && gl.kind != GlobalInit::Unknown) || gl.init == nullptr)
                    {
                        continue;
                    }
                    NamedConst c;
                    c.id = g.id;
                    c.c_name = ls[i].text;
                    c.type = ls[i].type;
                    c.loc = gl.loc.known() ? gl.loc : g.loc;
                    c.init = gl.init;
                    c.value = gl.value;
                    named_const_check(c, out, gl.kind == GlobalInit::Value);
                }
                continue;
            }
            if ((g.init_kind != Global::Init::Value && g.init_kind != Global::Init::Unknown) || g.init == nullptr)
            {
                continue;
            }
            NamedConst c;
            c.id = g.id;
            c.c_name = g.c_name;
            c.type = g.type;
            c.loc = g.loc;
            c.init = g.init;
            c.value = g.value;
            named_const_check(c, out, g.init_kind == Global::Init::Value);
        }
        if (own == nullptr)
        {
            return;
        }
        for (const NamedConst& c : own->local_consts())
        {
            named_const_check(c, out);
        }
        case_checks(own->body, out);
    }

    // ---- functions ----

    low::LClause clause_checks(E expr, const SrcLoc& loc, bool ensures, const std::string& what)
    {
        low::LClause c;
        c.expr = std::move(expr);
        auto mk = [&](const char* id, const char* text)
        {
            low::Meta m;
            m.category = "internal";
            m.loc = loc;
            m.message = std::string("internal check: ") + text + " of `" + what + "`";
            return low::Check {id, std::move(m)};
        };
        c.checks.push_back(mk("not.wellformed", "well-formedness of the contract"));
        if (ensures)
        {
            c.checks.push_back(mk("postcondition.violated", "Silicon's own check of the postcondition"));
        }
        return c;
    }

    // Parameter `k` as Viper formals (decision 0044 item 3): itself, or one per leaf of a by-value struct.
    struct LeafVar
    {
        E var;
        Type type;
        std::string name;
        std::vector<int> path;
    };
    std::vector<LeafVar> param_vars(const Function& f, int k) const
    {
        const Type& t {f.params[static_cast<size_t>(k)].type};
        std::vector<LeafVar> r;
        if (t.is_struct())
        {
            for (const StructLeaf& l : struct_leaves(*t.struct_type))
            {
                const std::string n {formal_name(f, k) + path_suffix(l.path)};
                r.push_back(LeafVar {low::var(n), l.type, n, l.path});
            }
        }
        else
        {
            r.push_back(LeafVar {low::var(formal_name(f, k)), t, formal_name(f, k), {}});
        }
        return r;
    }
    // The result as Viper results (decision 0044 item 4): `res`, or `res_m<path>` per leaf of a struct result; empty for void.
    std::vector<LeafVar> result_vars(const Function& f) const
    {
        std::vector<LeafVar> r;
        if (f.ret.is_struct())
        {
            for (const StructLeaf& l : struct_leaves(*f.ret.struct_type))
            {
                const std::string n {std::string(kResult) + path_suffix(l.path)};
                r.push_back(LeafVar {low::var(n), l.type, n, l.path});
            }
        }
        else if (!f.ret.is_void())
        {
            r.push_back(LeafVar {low::var(kResult), f.ret, kResult, {}});
        }
        return r;
    }

    void function(const Function& f)
    {
        fn_ = Fn {};
        fn_.f = &f;
        fn_.method = method_name(f);
        for (const auto& v : f.locals())
        {
            fn_.var_names.push_back(local_name(*v));
        }
        if (f.body != nullptr)
        {
            find_flagged(*f.body, fn_.flagged);
        }

        low::LMethod m;
        m.name = fn_.method;
        m.comment = "C/C++ function `" + f.c_name + "`";
        const FnAtoms& fa {atoms_of(f)};
        for (size_t k {0U}; k < f.params.size(); ++k)
        {
            for (const LeafVar& lv : param_vars(f, static_cast<int>(k)))
            {
                m.formals.push_back(low::LVar {lv.name, ltype(lv.type)});
            }
        }
        for (int k {0}; k < fa.nreq; ++k)
        {
            // the ghost amounts (decision 0037 item 6)
            m.formals.push_back(low::LVar {amount_name(k), low::LType::Perm});
        }
        for (const LeafVar& lv : result_vars(f))
        {
            m.results.push_back(low::LVar {lv.name, ltype(lv.type)});
        }

        // requires: the facts of the unit's globals (decision 0041 item 4: not `main`'s, which gets them from program_start), the parameters' type
        // ranges, the ghost amounts' bounds (before every clause that names one), WF of the pointers, then the user's (0012 item 1; decision 0037
        // items 2 and 6).
        if (&f != prog_.entry())
        {
            if (const E facts = global_facts())
            {
                m.requires_.push_back(clause_checks(facts, f.loc, false, f.c_name));
            }
        }
        for (size_t k {0U}; k < f.params.size(); ++k)
        {
            for (const LeafVar& lv : param_vars(f, static_cast<int>(k)))
            { // a by-value struct: the range of each integer leaf (decision 0044 item 3)
                if (!lv.type.is_int())
                {
                    continue;
                }
                m.requires_.push_back(clause_checks(range_check(lv.var, *lv.type.int_type), f.loc, false, f.c_name));
            }
        }
        for (int k {0}; k < fa.nreq; ++k)
        {
            m.requires_.push_back(clause_checks(amount_bounds(amount_name(k)), f.loc, false, f.c_name));
        }
        for (size_t k {0U}; k < f.params.size(); ++k)
        {
            for (const LeafVar& lv : param_vars(f, static_cast<int>(k)))
            { // and the WF of each pointer leaf
                if (!lv.type.is_ptr())
                {
                    continue;
                }
                m.requires_.push_back(clause_checks(wf_expr(lv.var, &lv.type), f.loc, false, f.c_name));
            }
        }
        const Env cenv {contract_env()};
        for (size_t ci {0U}; ci < f.requires_.size(); ++ci)
        {
            const Clause& c {f.requires_[ci]};
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
        // the range of the result, or of each integer leaf of a struct result, and WF of each pointer (decision 0044
        // item 4)
        for (const LeafVar& lv : result_vars(f))
        {
            if (lv.type.is_int())
            {
                m.ensures_.push_back(clause_checks(range_check(lv.var, *lv.type.int_type), f.loc, true, f.c_name));
            }
            if (lv.type.is_ptr())
            {
                m.ensures_.push_back(clause_checks(wf_expr(lv.var, &lv.type), f.loc, true, f.c_name));
            }
        }
        for (size_t ci {0U}; ci < f.ensures_.size(); ++ci)
        {
            const Clause& c {f.ensures_[ci]};
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
        if (f.noreturn)
        {
            m.ensures_.push_back(clause_checks(low::bool_lit(false), f.loc, true, f.c_name)); // decision 0015 item 4
        }

        if (f.body == nullptr)
        {
            m.trusted_reason = f.trusted_reason;
            m.decreases_assumed = true; // 0013 item 5
            out_.methods.push_back(std::move(m));
            return;
        }
        for (const Clause& c : f.decreases_)
        {
            Full scratch; // its own obligations belong to the well-formedness method
            LV v;
            {
                CtxSave save(*this);
                // A measure that reads memory is written plainly, each read guarded by its chain over the framing disjuncts (decision 0039 item 8).
                if (reads_heap(*c.expr))
                {
                    set_ctx(f, CMode::Method, false, static_cast<int>(f.requires_.size()));
                }
                v = lower(*c.expr, cenv, nullptr, {}, scratch);
            }
            pure_full(scratch, *c.expr, "a termination measure");
            m.decreases_.push_back(v.e);
        }
        if (!f.decreases_.empty())
        {
            low::Meta dm;
            dm.category = "termination";
            dm.loc = f.decreases_.front().loc.known() ? f.decreases_.front().loc : f.loc;
            dm.message = "the measure of `" + f.c_name +
                         "` may not decrease at a recursive call, or may not be bounded below by zero";
            m.decreases_check = low::Check {"termination.failed", std::move(dm)};
        }
        m.has_body = true;
        if (&f == prog_.entry() && has_globals())
        { // decision 0041 item 4: the globals are given their initial state first
            low::LStmt c;
            c.kind = low::LStmt::Kind::Call;
            c.name = "program_start";
            c.check = low::Check {
                "call.precondition",
                meta("internal", f.loc, "internal check: Silicon's own check of the call of program_start")};
            m.body.push_back(std::move(c));
        }
        // The parameters' local copies (0012 item 2); an address-taken parameter is copied into a fresh one-slot block (decision 0037 item 4), whose
        // scope is the body's.
        std::vector<const Var*> param_cells;
        for (size_t k {0U}; k < f.params.size(); ++k)
        {
            const Var& copy {*f.param_var(static_cast<int>(k))};
            if (copy.address_taken && copy.type.is_struct())
            {
                // decision 0044 item 3: the copy is a struct block like every struct local; each leaf is written from its formal through a Ref temporary, the flag set
                param_cells.push_back(&copy);
                alloc_cell(m.body, copy);
                const E root {low::var(var_name(copy))};
                const std::vector<LeafVar> fv {param_vars(f, static_cast<int>(k))};
                for (const LeafVar& lv : fv)
                {
                    const std::string t {new_temp(low::LType::Ref)};
                    assign(m.body, t, chain_ref(root, lv.path));
                    emit_assert(m.body, may_write(low::var(t), lv.type, false),
                                meta("internal", copy.loc,
                                     "internal check: the entry copy of `" + copy.name + "` is writable"));
                    write_fields(m.body, t, lv.type, lv.var);
                }
            }
            else if (copy.address_taken)
            {
                param_cells.push_back(&copy);
                alloc_cell(m.body, copy);
                write_fields(m.body, var_name(copy), copy.type, low::var(formal_name(f, static_cast<int>(k))));
            }
            else
            {
                assign(m.body, var_name(copy), low::var(formal_name(f, static_cast<int>(k))));
            }
        }
        if (!param_cells.empty())
        {
            fn_.scopes.push_back(Fn::Scope {param_cells, f.end_loc});
        }
        entry_snapshots(f, m.body, cenv);
        entry_checks(f, m.body);
        block_entry_checks(f, m.body); // decision 0047 item 4
        prologue(m.body, &f);
        user_targets_.clear();
        user_names_.clear();
        collect_gotos(*f.body, user_targets_);
        label_depths(*f.body, fn_.scopes.size());
        // the pointer parameters the body assigns: their formal's place is not automatic in a loop (decision 0038 item 3)
        fn_.assigned_params = assigned_params(f);
        stmt(*f.body, m.body);
        // The end of the body: a non-void function must not reach it; a void one returns.
        if (f.noreturn)
        {
            emit_assert(m.body, low::bool_lit(false),
                        meta("noreturn", f.end_loc,
                             "`" + f.c_name + "` is declared not to return, but control may reach its end"));
        }
        else if (f.ret.is_void())
        {
            if (!param_cells.empty())
            {
                scope_exit(m.body, param_cells, f.end_loc); // fall-through (decision 0037 item 4)
            }
            postconditions(f.end_loc, m.body);
        }
        else
        {
            emit_assert(m.body, low::bool_lit(false),
                        meta("missing-return", f.end_loc,
                             "control may reach the end of `" + f.c_name + "` without returning a value"));
        }
        low::LStmt end;
        end.kind = low::LStmt::Kind::Label;
        end.name = kEndLabel;
        m.body.push_back(std::move(end));
        leak_row(f, m.body); // decision 0047 item 7: one row after the end label
        for (const auto& v : f.locals())
        {
            // A cell's Viper local holds the Ref of its one-slot block (decision 0032 item 4).
            m.locals.push_back(low::LVar {fn_.var_names[static_cast<size_t>(v->id)],
                                          v->address_taken ? low::LType::Ref : ltype(v->type)});
            if (v->address_taken || v->type.is_ptr())
            {
                out_.memory = true;
            }
        }
        for (int id : fn_.flagged)
        {
            m.locals.push_back(low::LVar {flag_name(*f.locals()[static_cast<size_t>(id)]), low::LType::Bool});
        }
        for (const low::LVar& t : fn_.snapshots)
        {
            m.locals.push_back(t);
        }
        for (const low::LVar& t : fn_.temps)
        {
            m.locals.push_back(t);
        }
        out_.methods.push_back(std::move(m));
    }

    // A key names the result's place: the result itself or a place whose innermost root is the result (`*r.q`, decision 0045).
    bool is_result_key(int k) const
    {
        return k < 0 || (is_derived_key(k) && derived_of(k).key.root == kResultSiteKey);
    }
    // The `Ref` of the place (key, path) in the well-formedness method in a state: the ghost-built chain for a derived key (the base lowered in Wf mode), else the root's chain.
    E wf_place(const Env& env, int k, const std::vector<int>& path, bool post)
    {
        if (!is_derived_key(k))
        {
            return chain_ref(place_of(env, k), path);
        }
        const Derived& d {derived_of(k)};
        CtxSave save(*this);
        cmode_ = CMode::Wf;
        cf_ = d.f;
        cens_ = post;
        cold_ = false;
        cseen_ = atoms_of(*d.f).sites.size();
        cbseen_ = blocks_of(*d.f).sites.size();
        Full scratch;
        return chain_ref(lower(*d.base, env, nullptr, Deps {}, scratch).e, path);
    }

    // The contract's well-formedness method (0010 item 11, 0012 item 10): the obligations of each clause, asserted
    // under the clauses before it, in a method whose only assumptions are the type ranges.
    void wf_method(const Function& f)
    {
        fn_ = Fn {};
        fn_.f = &f;
        fn_.method = method_name(f);
        const FnAtoms& fa {atoms_of(f)};
        wf_ghosts_.clear();
        low::LMethod m;
        m.name = fn_.method + "__wf";
        m.comment = "well-formedness of the contract of C/C++ function `" + f.c_name + "`";
        m.has_body = true;
        // the same checks as the function's own method (the driver removes identical reports); a function whose
        // `requires` is unsatisfiable never reaches its own prologue
        prologue(m.body, &f);
        const Env env {contract_env()};
        E prior;
        auto go = [&](const Clause& c, bool ens, int idx)
        {
            Full full;
            LV v;
            {
                CtxSave save(*this);
                set_ctx(f, CMode::Wf, ens, idx);
                v = lower(*c.expr, env, nullptr, {}, full);
            }
            pure_full(full, *c.expr, "a contract clause");
            for (Obl& o : full.obls)
            {
                emit_assert(m.body, low::implies(low::and_(prior, o.guard), o.cond), o.meta);
            }
            if (ens)
            {
                // Partners by path (decision 0039 item 7, owner question 72): an `ensures` atom returns what its partner `requires` atom took, on every path
                // where it is returned: `prior ==> (g_ens ==> g_partner)`.  Nothing is written where the partner is unconditional or has the very guard.
                CtxSave guard_ctx(*this);
                // the guards over ghosts: the `ensures` atom's over the post-state ghosts, the partner's over the
                // pre-state ones (decision 0045, audit B2)
                cmode_ = CMode::Wf;
                for (const AtomSite& site : fa.sites)
                {
                    if (!site.in_ensures || site.clause != idx || site.atom->amount < 0 || site.atom->amount >= fa.nreq)
                    {
                        continue;
                    }
                    const AtomSite& partner {fa.sites[static_cast<size_t>(site.atom->amount)]};
                    if (guards_read_heap(site, partner))
                    {
                        continue; // an obligation at the atom (decision 0045 item 3)
                    }
                    const E ge {site_guard(site, env)};
                    const E gp {site_guard(partner, env)};
                    if (!gp || same_expr(ge, gp))
                    {
                        continue;
                    }
                    const std::string txt {site.atom->a != nullptr ? site_text(site) : std::string()};
                    emit_assert(m.body, low::implies(prior, low::implies(ge, gp)),
                                meta("permission", site.atom->loc.known() ? site.atom->loc : c.loc,
                                     "the contract of `" + f.c_name + "` returns " + quoted(txt, "an object") +
                                         " on a path where its `requires` did not take it"));
                }
            }
            prior = low::and_(prior, v.e);
        };
        for (size_t k {0U}; k < f.requires_.size(); ++k)
        {
            go(f.requires_[k], false, static_cast<int>(k));
        }
        E requires_only {prior};
        for (size_t k {0U}; k < f.ensures_.size(); ++k)
        {
            go(f.ensures_[k], true, static_cast<int>(k));
        }
        // The measure is evaluated over the entry values: its obligations are checked under the requires (0013 item 7).
        for (const Clause& c : f.decreases_)
        {
            Full full;
            {
                CtxSave save(*this);
                // decision 0039 item 8: the heap reads of a measure are framed by the requires atoms, over the
                // pre-state ghosts
                if (reads_heap(*c.expr))
                {
                    set_ctx(f, CMode::Wf, false, static_cast<int>(f.requires_.size()));
                    cmeasure_ = true;
                }
                LV v {lower(*c.expr, env, nullptr, {}, full)};
                cmeasure_ = false;
            }
            pure_full(full, *c.expr, "a termination measure");
            for (Obl& o : full.obls)
            {
                emit_assert(m.body, low::implies(low::and_(requires_only, o.guard), o.cond), o.meta);
            }
        }
        if (m.body.empty())
        {
            low::LStmt nothing;
            nothing.kind = low::LStmt::Kind::Comment;
            nothing.name = "the contract has nothing that can be undefined";
            m.body.push_back(std::move(nothing));
        }
        // The formals and what the method assumes: the parameters, the ghost amounts, the result, the ghost values of the reads of the contract.
        const std::string what {f.c_name + " (well-formedness)"};
        // Facts Silicon derives that the ghost values must not lack (decision 0037 item 7): two places that are one cell have one value in a state,
        // and a cell a `reads` atom borrows, returned by a `reads` atom, has the value it had.  Per place, root plus path (decision 0043 item 9, audit S1).
        // Computed before the formals are listed: the base of a place reached through a load (decision 0045) is lowered again here and must not add a ghost late.
        std::vector<low::E> facts_;
        {
            CtxSave facts_ctx(*this);
            cmode_ = CMode::Wf; // every guard and place below is over ghosts (the ensures' over the post-state ones)
            const std::map<GKey, Type> ghosts_now {wf_ghosts_};
            for (auto a {ghosts_now.begin()}; a != ghosts_now.end(); ++a)
            {
                for (auto b {std::next(a)}; b != ghosts_now.end(); ++b)
                {
                    if (std::get<2>(a->first) != std::get<2>(b->first) ||
                        cell_suffix(a->second) != cell_suffix(b->second))
                    {
                        continue;
                    }
                    if (key_is_global(std::get<0>(a->first)) && key_is_global(std::get<0>(b->first)))
                    {
                        continue; // two globals are two blocks
                    }
                    if (std::get<0>(a->first) == std::get<0>(b->first))
                    {
                        continue; // one root: two member paths are two cells (one derived key: one base)
                    }
                    const E pa {wf_place(env, std::get<0>(a->first), std::get<1>(a->first), std::get<2>(a->first))};
                    const E pb {wf_place(env, std::get<0>(b->first), std::get<1>(b->first), std::get<2>(b->first))};
                    facts_.push_back(low::implies(
                        low::bin(low::BinOp::Eq, pa, pb),
                        low::bin(low::BinOp::Eq, low::var(ghost_name(a->first)), low::var(ghost_name(b->first)))));
                }
            }
            // The first `reads` atom of the `requires` (`ens` false) or the `ensures` that covers the place (root, path): its guard.
            auto first_reads = [&](int root, const std::vector<int>& path, bool ens, E& guard)
            {
                for (const AtomSite& s : fa.sites)
                {
                    if (s.atom->atom != AtomKind::Reads || site_key(s) != root || s.in_ensures != ens)
                    {
                        continue;
                    }
                    if (s.path.size() > path.size() || !std::equal(s.path.begin(), s.path.end(), path.begin()))
                    {
                        continue; // the atom's place contains the read's
                    }
                    guard = site_guard(s, env);
                    return true;
                }
                return false;
            };
            for (const auto& [key, t] : ghosts_now)
            {
                const int kr {std::get<0>(key)};
                const std::vector<int>& path {std::get<1>(key)};
                if (is_result_key(kr) && std::get<2>(key))
                {
                    // The result's post-state ghost against the pre-state ghost of a `reads` place the function borrows and returns (audit S4, decision 0040
                    // item 4): `g_r && g_e && result == q ==> h_result == h_q_pre`.  Decision 0045: the result's place may be `r.q`'s base (the innermost root is the result).
                    for (const auto& [k2, t2] : ghosts_now)
                    {
                        if (is_result_key(std::get<0>(k2)) || std::get<2>(k2) || cell_suffix(t2) != cell_suffix(t))
                        {
                            continue;
                        }
                        E gr2;
                        E ge2;
                        if (!first_reads(std::get<0>(k2), std::get<1>(k2), false, gr2) ||
                            !first_reads(std::get<0>(k2), std::get<1>(k2), true, ge2))
                        {
                            continue;
                        }
                        facts_.push_back(low::implies(
                            low::and_(low::and_(gr2, ge2),
                                      low::bin(low::BinOp::Eq,
                                               is_derived_key(kr) ? wf_place(env, kr, path, true)
                                                                  : chain_ref(env.result, path),
                                               wf_place(env, std::get<0>(k2), std::get<1>(k2), false))),
                            low::bin(low::BinOp::Eq, low::var(ghost_name(key)), low::var(ghost_name(k2)))));
                    }
                    continue;
                }
                if (!std::get<2>(key) || ghosts_now.count(GKey {kr, path, false}) == 0U)
                {
                    continue;
                }
                E gr;
                E ge;
                if (!first_reads(kr, path, false, gr) || !first_reads(kr, path, true, ge))
                {
                    continue;
                }
                E cond {low::and_(gr, ge)};
                // A place reached through a load names another object when the loaded pointer moved: the value survives only under `q_post == q_pre` (decision 0045 item 5).
                if (is_derived_key(kr) && derived_of(kr).has_load)
                {
                    cond = low::and_(
                        cond, low::bin(low::BinOp::Eq, wf_place(env, kr, path, true), wf_place(env, kr, path, false)));
                }
                facts_.push_back(low::implies(cond, low::bin(low::BinOp::Eq, low::var(ghost_name(kr, path, true)),
                                                             low::var(ghost_name(kr, path, false)))));
            }
        }
        for (size_t k {0U}; k < f.params.size(); ++k)
        {
            for (const LeafVar& lv : param_vars(f, static_cast<int>(k)))
            {
                m.formals.push_back(low::LVar {lv.name, ltype(lv.type)});
            }
        }
        for (int k {0}; k < fa.nreq; ++k)
        {
            m.formals.push_back(low::LVar {amount_name(k), low::LType::Perm});
        }
        for (const LeafVar& lv : result_vars(f))
        {
            m.formals.push_back(low::LVar {lv.name, ltype(lv.type)});
        }
        for (const auto& [key, t] : wf_ghosts_)
        {
            m.formals.push_back(low::LVar {ghost_name(key), ltype(t)});
        }
        if (&f != prog_.entry())
        {
            if (const E facts = global_facts())
            {
                m.requires_.push_back(clause_checks(facts, f.loc, false, what));
            }
        }
        for (int k {0}; k < fa.nreq; ++k)
        {
            m.requires_.push_back(clause_checks(amount_bounds(amount_name(k)), f.loc, false, what));
        }
        for (size_t k {0U}; k < f.params.size(); ++k)
        {
            for (const LeafVar& lv : param_vars(f, static_cast<int>(k)))
            {
                if (lv.type.is_ptr())
                {
                    m.requires_.push_back(clause_checks(wf_expr(lv.var, &lv.type), f.loc, false, what));
                }
                if (!lv.type.is_int())
                {
                    continue;
                }
                m.requires_.push_back(clause_checks(range_check(lv.var, *lv.type.int_type), f.loc, false, what));
            }
        }
        for (const LeafVar& lv : result_vars(f))
        {
            if (lv.type.is_int())
            {
                m.requires_.push_back(clause_checks(range_check(lv.var, *lv.type.int_type), f.loc, false, what));
            }
            if (lv.type.is_ptr())
            {
                m.requires_.push_back(clause_checks(wf_expr(lv.var, &lv.type), f.loc, false, what));
            }
        }
        for (const auto& [key, t] : wf_ghosts_)
        {
            if (t.is_ptr())
            {
                m.requires_.push_back(clause_checks(wf_expr(low::var(ghost_name(key)), &t), f.loc, false, what));
            }
            else if (!t.is_bool())
            {
                m.requires_.push_back(
                    clause_checks(range_check(low::var(ghost_name(key)), *t.int_type), f.loc, false, what));
            }
        }
        for (const E& fact : facts_)
        {
            m.requires_.push_back(clause_checks(fact, f.loc, false, what));
        }
        out_.methods.push_back(std::move(m));
    }

    // The contract being lowered (decision 0037): the mode, the function whose atoms the sites are, whether it is an `ensures`, whether under `old`,
    // and how many atoms (of `contract_atoms(*cf_)`) the lowering has passed so far.
    CMode cmode_ {CMode::None};
    const Function* cf_ {nullptr};
    bool cens_ {false};
    bool cold_ {false};
    // the number of contract reads under `old` being lowered (decision 0045: one `old(...)` per chain)
    int old_depth_ {0};
    bool cmeasure_ {false}; // the contract expression is a function's termination measure (decision 0039 item 8)
    size_t cseen_ {0U};
    size_t cbseen_ {0U}; // the same for the `owns_block` sites of `cf_` (decision 0047 item 4)
    std::map<const Function*, FnAtoms> atoms_;
    std::map<const Function*, FnBlocks> blocks_;
    // a permission atom -> the number of block sites before it (index_block_order)
    std::map<const Expr*, size_t> blocks_before_;
    // a vlibc heap program (decision 0050 item 4): some view of vlibc's allocator, an allocation or a deallocation, is
    // in the program
    bool vlibc_heap_ {false};
    bool heap_ {false};           // a heap program (decision 0047 item 7): a view call or an owns_block site anywhere
    std::deque<Derived> derived_; // decision 0045: the derived keys (a deque: references to entries stay valid)
    std::map<std::tuple<const Function*, int, std::vector<int>>, int> derived_index_;
    std::map<const Function*, int> derived_count_;
    std::map<int, int> lb_keys_;                     // a derived key -> its loop-entry snapshot key
    int cur_loop_ {0};                               // the number of the loop being lowered (the lb snapshots' names)
    std::map<const Expr*, int> derived_of_atom_;     // an Atom node whose place has steps -> its derived key
    std::map<const Expr*, const Function*> site_fn_; // an Atom node -> the function whose contract holds it
    // the ghost value formals of the `__wf` method being built: (root, member path, post-state) -> the place's type
    std::map<GKey, Type> wf_ghosts_;
    const Program& prog_;
    LowerOptions opt_;
    low::LProgram out_;
    std::vector<Diag> errors_;
    Fn fn_;
    std::vector<Diag> rejections_;
    std::map<const Function*, int> component_; // the recursion component of each function (compute_cycles)
    const SrcLoc* report_at_ {nullptr};
    const std::string* report_note_ {nullptr};
    int assert_count_ {0};
    // The full expression being lowered (decision 0020): the locals it writes (every read of one is a snapshot), the comma floor (E6),
    // and the old-value nodes of `x++` whose read is shared.
    bool in_full_ {false};
    bool exact_dropped_ {false}; // the self-test hook `drop_exact_set` has fired
    std::set<int> written_;
    Deps floor_;
    std::map<const Expr*, LV> shared_;
    // ---- decision 0039: cell accesses of the full expression being lowered (branch bodies included) ----
    // The nodes from the root to the one being lowered (the slot of each in its parent), the accesses made so far, the cell fields the expression writes
    // and its memory calls by field (E3's trigger), the memory calls whose arguments are being lowered (a read in them is no snapshot's reason), the
    // conditions of the E5 branches around the node, the full being filled and the branches open around it (their extra sets, audit S5).
    Path path_;
    // where a shared node (a compound store's address) was lowered the first time
    std::map<const Expr*, Path> shared_path_;
    std::vector<Acc> accs_;
    std::set<std::string> mem_w_;
    std::map<std::string, int> mem_calls_, mem_depth_;
    std::vector<E> branch_guards_;
    std::vector<BranchCtx> branch_ctx_;
    // PRE(c) goes into both halves of the call's permission and heap-reading checks (audit S3)
    bool call_pre_both_ {false};
    std::map<std::string, int> key_ids_;
    int next_other_key_ {0};
    std::map<int, std::string> user_names_; // the user's name of each label of the current function, by id
    std::set<int> user_targets_;            // the ids of the user labels some goto of the current function targets
    // The memory model's bookkeeping (decision 0032 item 5): the suffix of each enumeration's cell fields, the Type of each cell-type suffix
    // used, and where the first address-taken declaration of each type is (the map rows of its `alloc_<t>`).
    std::map<const IntType*, std::string> enum_suffix_;
    std::map<std::string, Type> cell_type_of_;
    // the struct locals' allocations, by `struct_suffix` (a key of alloc_at_ too)
    std::map<std::string, const StructType*> struct_alloc_;
    // MemberAddr nodes the consumer lowers as a place access (decision 0043 item 5), with the consumer's position: see
    // member_addr
    std::map<const Expr*, SrcLoc> place_nodes_;
    std::map<std::string, SrcLoc> alloc_at_;
    // the aggregate about to be lowered is the value of a struct store: its pointer leaves must be well-formed
    // (decision 0044 item 5)
    bool agg_wf_ {false};
};

} // namespace

LowerResult lower_program(const Program& program, const LowerOptions& options)
{
    // Fail closed on its own: a program that does not pass the checker (a call where none is allowed, an operand of
    // the wrong type) may have parts that the lowering's scratch structures would silently drop.
    std::vector<Diag> problems {check_program(program)};
    if (!problems.empty())
    {
        LowerResult refused;
        refused.errors = std::move(problems);
        return refused;
    }
    return Lowerer(program, options).run();
}

} // namespace cerastes::vir
