// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The verify-mode encoder: the IL of the translation unit to VIR (decisions 0010, 0011, 0012;
// internal/encoding.md section 7), then through the VIR checker, the lowering and the Viper printer.
//
// It is a recursive descent over the IL of every function body in a verified file, an allow-list in every
// switch: a construct the encoder has no rule for is REJECTED with a diagnostic at its C/C++ position, never
// approximated (invariant 3).  Rejections go through reject(), which reports each (diagnostic, line) once.
// A rejected function is dropped and the run ends with the rejections (the driver exits with status 2); the
// program is only checked, lowered and printed when nothing was rejected.
//
// The IL facts each rule relies on are in internal/il-inventory.md, "M3: scalar code" (probes P1-P15).
#include "encode.h"

#include <cstdlib>
#include <cctype>
#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "annotation_il.h"
#include "check.h"
#include "dec.h"
#include "diagnostics.h"
#include "edg_il.h"
#include "emit.h"
#include "il_names.h"
#include "il_query.h"
#include "lower.h"
#include "render.h"
#include "verify_config.h"
#include "vir.h"

namespace cerastes {
namespace {

using vir::Expr;
using vir::SrcLoc;
using vir::Stmt;

// ---- types -----------------------------------------------------------------------------------------------

// What an EDG type means in VIR, or why it has no meaning.
struct VType {
  bool ok = false;
  vir::Type t = vir::Type::void_type();
  std::string name;                  // the type as the program spells it, for messages
  Diag diag = Diag::unsupported_type;  // the diagnostic when !ok
};

// The attributes a verified file may carry besides cerastes's own (decision 0011 item 7), by name.
bool allowed_attribute_name(std::string n) {
  if (n.size() > 4 && n.compare(0, 2, "__") == 0 && n.compare(n.size() - 2, 2, "__") == 0) n = n.substr(2, n.size() - 4);
  return n == "maybe_unused" || n == "nodiscard" || n == "deprecated" || n == "likely" || n == "unlikely" ||
         n == "unused" || n == "warn_unused_result" ||
         n == "fallthrough";  // 0014 item 11: no run-time meaning (P18)
}

// How a rejection describes an operation.
std::string describe_operation(an_expr_operator_kind k) {
  switch (k) {
    case eok_comma: return "the comma operator";
    case eok_and: return "the bitwise operator &";
    case eok_or: return "the bitwise operator |";
    case eok_xor: return "the bitwise operator ^";
    case eok_complement: return "the bitwise operator ~";
    case eok_and_assign: return "the bitwise assignment &=";
    case eok_or_assign: return "the bitwise assignment |=";
    case eok_xor_assign: return "the bitwise assignment ^=";
    case eok_assign:
    case eok_bassign:
    case eok_add_assign:
    case eok_subtract_assign:
    case eok_multiply_assign:
    case eok_divide_assign:
    case eok_remainder_assign:
    case eok_shiftl_assign:
    case eok_shiftr_assign: return "an assignment used as a value";
    case eok_pre_incr:
    case eok_post_incr:
    case eok_pre_decr:
    case eok_post_decr: return "an increment or decrement used as a value";
    case eok_address_of: return "taking an address";
    case eok_indirect:
    case eok_ref_indirect:
    case eok_reference_to:
    case eok_subscript:
    case eok_padd:
    case eok_psubtract:
    case eok_pdiff: return "a pointer or reference operation";
    case eok_dot_field:
    case eok_points_to_field: return "a member access";
    case eok_dot_member_call:
    case eok_points_to_member_call: return "a member function call";
    case eok_spaceship: return "the operator <=>";
    default: return std::string("the operation ") + name_of(k);
  }
}

class Encoder {
 public:
  Encoder() : b_(prog_) {}

  void run(const IlEntities& ents);

  // After run(): the program, when nothing was rejected.
  vir::Program& program() { return prog_; }
  int verified_functions() const { return static_cast<int>(to_verify_.size()); }

 private:
  // ---- per-function state --------------------------------------------------------------------------------
  struct Ctx {
    a_routine_ptr routine = nullptr;
    vir::Function* fn = nullptr;
    std::map<a_variable_ptr, vir::Var*> vars;
    std::set<a_variable_ptr> pending;  // declared, initializer not yet seen
  std::set<a_variable_ptr> rejected_vars;  // declarations rejected: their uses are not diagnosed again
    a_source_position stmt_pos{};      // the statement being encoded, for nodes without a position
    a_variable_ptr declaring = nullptr;  // the variable whose initializer is being encoded
    int const_depth = 0;               // inside the original expression of a folded constant
    // Annotation mode (a `requires` or `ensures` of the routine):
    bool contract = false;
    bool annotation = false;  // a `check` or an `invariant` of a statement
    Annot annot = Annot::none;
    bool binder_present = false;
    int old_depth = 0;
    bool use_own = false;  // building a node of a user macro body: positions are the definition's
    bool suppress = false;  // after a variable length array: what follows is the compiler's cleanup, not the user's
    // The loops and switches being encoded, innermost last (decision 0014 item 7).  A `break` or `continue` is a
    // compiler-generated goto; its label belongs to the construct whose label statement stands where EDG puts it.
    struct Target {
      bool is_switch = false;
      int id = 0;                            // the VIR jump id
      a_label_ptr break_label = nullptr;     // the label statement right after the construct, if there is one
      a_label_ptr continue_label = nullptr;  // the label statement ending the body of a loop, if there is one
    };
    std::vector<Target> targets;
    // User labels: one VIR id per label entity (decision 0015 item 2); the C/C++ position of each user `goto`, for the
    // diagnostics of vir::goto_problems.
    bool in_for_step = false;  // encoding the increment of a `for` loop
    // Case values (decision 0017 items 5-6): while a case label's value (or the initializer of a local constant it names)
    // is encoded, a use of a variable is noted, and in C++ a `const` local with a constant initializer is a function
    // local constant, whose uses are literals.
    bool case_value = false;
    std::string case_var;  // the first variable the case value names (empty: none)
    struct ConstInit {
      a_constant_ptr c = nullptr;  // the initializer's constant (the declaration's `dik_constant`, P23)
      a_source_position at{};      // the declaration
    };
    std::map<a_variable_ptr, ConstInit> local_const_inits;  // const locals declared so far with a constant initializer
    struct ConstValue {
      vir::Type type;
      std::string value;
    };
    std::map<a_variable_ptr, ConstValue> local_consts;  // those a case value named: registered once per function
    std::map<a_label_ptr, int> label_ids;
    std::map<const Stmt*, a_source_position> goto_positions;
  };

  vir::Program prog_;
  vir::Builder b_;
  std::map<a_routine_ptr, vir::Function*> fns_;
  std::vector<a_routine_ptr> to_verify_;
  std::set<a_routine_ptr> signature_checked_;
  std::map<a_routine_ptr, std::set<a_routine_ptr>> edges_;  // calls between verified functions (decision 0013 item 4)
  std::set<std::tuple<int, std::string, unsigned long>> rejected_;  // (diagnostic, file, line)
  Ctx cx_;
  // Named constants (decision 0017): the file-scope `const` objects of verified files that are encoded, by variable.
  struct NamedInfo {
    vir::Type type;
    std::string value;  // EDG's value, decimal ("0"/"1" for bool)
  };
  std::map<a_variable_ptr, NamedInfo> named_;
  std::set<a_variable_ptr> rejected_globals_;  // file-scope variables rejected at their declaration: uses are not diagnosed again
  a_variable_ptr encoding_named_ = nullptr;     // the named constant whose initializer is being encoded

  // ---- diagnostics and positions ---------------------------------------------------------------------------
  a_source_position pos_or(const a_source_position& p) const {
    if (p.seq != 0) return p;
    if (cx_.stmt_pos.seq != 0) return cx_.stmt_pos;
    if (cx_.routine != nullptr) return cx_.routine->source_corresp.decl_position;
    return p;
  }

  // Where a node is reported.  In code, the position of the node itself: for a token of a user macro's body that is
  // the expansion site (the line the programmer wrote), not the `#define` line; in an annotation, the original
  // position of an argument token of a CERASTES_* macro (0006 item 10).
  DiagPos position_for(const a_source_position& p) const {
    a_source_position q = pos_or(p);
    if (!cx_.contract && !cx_.annotation) {
      q.orig_seq = 0;
      q.orig_column = 0;
    }
    return diag_position(q);
  }

  SrcLoc loc_of(const a_source_position& p) const {
    // A token of a macro body keeps its own position (the `#define`) while its node is built; macro_site() then reports
    // the obligations at the expansion site, with a note here.
    DiagPos d = cx_.use_own ? diag_position(pos_or(p)) : position_for(p);
    SrcLoc l;
    if (d.known) {
      l.file = d.file;
      l.line = static_cast<int>(d.line);
      l.column = static_cast<int>(d.col);
    }
    return l;
  }

  void reject(Diag d, const a_source_position& p, const std::string& what) {
    if (cx_.suppress) return;
    DiagPos dp = position_for(p);
    if (!rejected_.insert({static_cast<int>(d), dp.file, dp.line}).second) return;
    report_at(d, dp, {what});
  }

  // As reject(), at a position already resolved (a line found in the source text).
  void reject_at(Diag d, const DiagPos& dp, const std::string& what) {
    if (cx_.suppress) return;
    if (!rejected_.insert({static_cast<int>(d), dp.file, dp.line}).second) return;
    report_at(d, dp, {what});
  }

  // The position of the keyword of a loop (`do`, `while`, `for`).  In C++ a statement's position starts at its
  // attributes (`CERASTES_INVARIANT(...)` written before the loop), so the keyword is found in the source text: skip
  // `[[...]]` groups and macro calls `NAME(...)`, then the keyword.  Without attributes it is the statement's own position.
  DiagPos loop_keyword_position(a_statement_ptr s) {
    DiagPos d = position_for(s->position);
    if (s->attributes == nullptr || !d.known || d.full_name.empty()) return d;
    std::ifstream in(d.full_name);
    std::vector<std::string> lines;
    for (std::string l; std::getline(in, l) && lines.size() < d.line + 40;) lines.push_back(l);
    size_t li = d.line - 1, ci = d.col > 0 ? d.col - 1 : 0;
    auto at = [&](size_t l, size_t c) -> char { return l < lines.size() && c < lines[l].size() ? lines[l][c] : '\n'; };
    int guard = 0;
    while (li < lines.size() && ++guard < 4000) {
      char c = at(li, ci);
      if (c == '\n') {  // end of line
        ++li;
        ci = 0;
        continue;
      }
      if (c == ' ' || c == '\t') {
        ++ci;
        continue;
      }
      if (c == '[' && at(li, ci + 1) == '[') {
        int depth = 0;
        for (;;) {
          char x = at(li, ci);
          if (li >= lines.size()) return d;
          if (x == '\n') {
            ++li;
            ci = 0;
            continue;
          }
          if (x == '[') ++depth;
          if (x == ']' && --depth == 0) {
            ++ci;
            break;
          }
          ++ci;
        }
        continue;
      }
      if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        size_t j = ci;
        while (std::isalnum(static_cast<unsigned char>(at(li, j))) || at(li, j) == '_') ++j;
        const std::string word = lines[li].substr(ci, j - ci);
        if (word == "do" || word == "while" || word == "for") {
          d.line = li + 1;
          d.col = ci + 1;
          return d;
        }
        ci = j;
        while (at(li, ci) == ' ' || at(li, ci) == '\t') ++ci;
        if (at(li, ci) == '(') {
          int depth = 0;
          for (;;) {
            if (li >= lines.size()) return d;
            char x = at(li, ci);
            if (x == '\n') {
              ++li;
              ci = 0;
              continue;
            }
            if (x == '(') ++depth;
            if (x == ')' && --depth == 0) {
              ++ci;
              break;
            }
            ++ci;
          }
        }
        continue;
      }
      return d;  // something this scan does not know
    }
    return d;
  }

  void reject_type(const VType& t, const a_source_position& p) {
    if (t.diag == Diag::unsupported_type) reject(Diag::unsupported_type, p, t.name);
    else reject(t.diag, p, "the type \"" + t.name + "\"");
  }

  // ---- types ----------------------------------------------------------------------------------------------
  VType vtype(a_type_ptr type) {
    VType r;
    if (type == nullptr) {
      r.name = "<unknown>";
      return r;
    }
    a_type_ptr t = skip_typerefs(type);
    r.name = render_type(t);
    const a_type_qualifier_set q = f_get_type_qualifiers(type, TRUE);
    if ((q & TQ_VOLATILE) != 0) {
      r.name = "volatile " + r.name;
      return r;
    }
    if ((q & TQ_C11_ATOMIC) != 0) {
      r.name = "_Atomic " + r.name;
      return r;
    }
    switch (t->kind) {
      case tk_void:
        r.ok = true;
        r.t = vir::Type::void_type();
        return r;
      case tk_integer: {
        const auto& i = t->variant.integer;
        if (i.enum_type) return r;
        if (i.int_kind == ik_bit_precise || i.int_kind == ik_unsigned_bit_precise) return r;
        if (i.bool_type) {
          r.ok = true;
          r.t = vir::Type::bool_type();
          return r;
        }
        const int bits = static_cast<int>(t->size) * static_cast<int>(targ_char_bit);
        if (bits <= 0 || bits > 128) return r;
        r.ok = true;
        r.t = vir::Type::integer(prog_.int_type(r.name, bits, int_kind_is_signed[i.int_kind] != 0));
        return r;
      }
      case tk_class:
      case tk_struct:
      case tk_union:
      case tk_array:
        r.diag = Diag::unsupported_declaration;
        return r;
      default:
        return r;
    }
  }

  // A value type (not void) or a rejection at `p`.
  bool value_type(a_type_ptr type, const a_source_position& p, VType* out) {
    VType t = vtype(type);
    if (!t.ok || t.t.is_void()) {
      if (t.ok) {
        t.ok = false;
        t.name = "void";
      }
      reject_type(t, p);
      return false;
    }
    *out = t;
    return true;
  }

  // ---- attributes (decision 0011 item 7) -----------------------------------------------------------------------
  void check_attributes(an_attribute_ptr list, bool on_noreturn_routine = false) {
    for (an_attribute_ptr a = list; a != nullptr; a = a->next) {
      if (is_registered_cerastes(a)) continue;
      if (a->kind == ak_empty_attr) continue;
      // `noreturn`, `gnu::noreturn`, `__attribute__((noreturn))` and C23 `[[_Noreturn]]` (decision 0015 item 1, P21): they set
      // the function type's "does not return" flag, which check_signature reads.  `[[_Noreturn]]` in C++ is unrecognized.
      // Only on a routine whose type carries the flag (the caller says so); anywhere else it is rejected below.
      if (a->kind == ak_noreturn && a->name != nullptr && on_noreturn_routine) {
        std::string n = a->name;
        if (n.size() > 4 && n.compare(0, 2, "__") == 0 && n.compare(n.size() - 2, 2, "__") == 0) n = n.substr(2, n.size() - 4);
        if (n == "noreturn" || n == "_Noreturn") continue;
      }
      const char* ns = a->namespace_name;
      const bool ns_ok = ns == nullptr || *ns == '\0' || std::strcmp(ns, "gnu") == 0;
      if (ns_ok && a->name != nullptr && allowed_attribute_name(a->name)) continue;
      const std::string text = "\"" + render_attribute(a) + "\"";
      reject(a->kind == ak_unrecognized ? Diag::unrecognized_attribute : Diag::unsupported_attribute, a->position, text);
    }
  }

  // ---- routines -----------------------------------------------------------------------------------------------
  static bool is_main(a_routine_ptr r) {
    return r->source_corresp.name != nullptr && std::strcmp(r->source_corresp.name, "main") == 0 &&
           !r->source_corresp.is_class_member;
  }

  static a_routine_type_supplement_ptr routine_supplement(a_routine_ptr r) {
    a_type_ptr rt = r->type ? skip_typerefs(r->type) : nullptr;
    return (rt && rt->kind == tk_routine) ? rt->variant.routine.extra_info : nullptr;
  }

  static a_type_ptr return_type(a_routine_ptr r) {
    a_type_ptr rt = r->type ? skip_typerefs(r->type) : nullptr;
    return (rt && rt->kind == tk_routine) ? rt->variant.routine.return_type : nullptr;
  }

  // The body of a routine in a verified file, or null.
  a_statement_ptr verified_body(a_routine_ptr r) {
    a_scope_ptr sp = body_scope(r);
    if (sp == nullptr || sp->assoc_block == nullptr) return nullptr;
    if (!in_verified_file(sp->assoc_block->position)) return nullptr;
    return sp->assoc_block;
  }

  // Rejects what the signature cannot say in VIR; true when it can be encoded.
  bool check_signature(a_routine_ptr r, const a_source_position& at) {
    if (!signature_checked_.insert(r).second) return signature_ok_.count(r) != 0;
    bool ok = true;
    // Attributes are held to the allow-list only on what this run verifies: a callee declared in a system
    // header keeps whatever it carries.
    const bool ours = in_verified_file(r->source_corresp.decl_position) || verified_body(r) != nullptr;
    a_routine_type_supplement_ptr x = routine_supplement(r);
    if (x == nullptr) {
      reject(Diag::unsupported_declaration, at, "a function whose type cannot be read");
      return false;
    }
    if (x->has_ellipsis) {
      reject(Diag::unsupported_declaration, at, "a variadic function");
      ok = false;
    }
    if (!x->prototyped) {
      reject(Diag::unsupported_declaration, at, "a function declared without a prototype");
      ok = false;
    }
    if (ours && x->does_not_return && is_main(r)) {
      reject(Diag::unsupported_declaration, at, "a \"main\" declared not to return");
      ok = false;
    }
    VType ret = vtype(return_type(r));
    if (!ret.ok) {
      reject_type(ret, at);
      ok = false;
    }
    for (a_param_type_ptr p = x->param_type_list; p != nullptr; p = p->next) {
      VType pt = vtype(p->type);
      if (pt.ok && (p->qualifiers & TQ_VOLATILE) != 0) {
        pt.ok = false;
        pt.name = "volatile " + pt.name;
      }
      if (!pt.ok || pt.t.is_void()) {
        if (pt.ok) pt.name = "void";
        reject_type(pt, at);
        ok = false;
      }
      if (ours) check_attributes(p->attributes);
    }
    if (ok) signature_ok_.insert(r);
    return ok;
  }
  std::set<a_routine_ptr> signature_ok_;

  // The VIR function of a routine, created with its signature and contract the first time it is needed; null
  // after a rejection.  A routine without a body in a verified file is a trusted contract, which `call` needs
  // (`callee_for_call`); creating it here does not by itself make it verified.
  vir::Function* function_of(a_routine_ptr r, bool has_body_here, const a_source_position& at) {
    auto it = fns_.find(r);
    if (it != fns_.end()) return it->second;
    if (!check_signature(r, at)) {
      fns_[r] = nullptr;
      return nullptr;
    }
    VType ret = vtype(return_type(r));
    a_routine_type_supplement_ptr x = routine_supplement(r);
    const a_source_position decl = r->source_corresp.decl_position;
    SrcLoc end = loc_of(decl);
    if (has_body_here) {
      a_scope_ptr sp = body_scope(r);
      if (sp && sp->assoc_block && sp->assoc_block->kind == stmk_block && sp->assoc_block->variant.block.extra_info)
        end = loc_of(sp->assoc_block->variant.block.extra_info->final_position);
    }
    vir::Function* f = prog_.add_function(rname(r), ret.t, loc_of(decl), end);
    fns_[r] = f;
    for (a_param_type_ptr p = x->param_type_list; p != nullptr; p = p->next) {
      VType pt = vtype(p->type);
      const int k = f->add_param(p->name ? p->name : "", pt.t, loc_of(decl));
      f->params[static_cast<size_t>(k)].c_type_name = pt.name;
      const_cast<vir::Var*>(f->param_var(k))->c_type_name = pt.name;
    }
    f->noreturn = x->does_not_return != 0;  // decision 0015 item 4
    if (!has_body_here) {
      f->trusted_reason = in_verified_file(decl) ? (f->noreturn ? "declared noreturn here, defined in another unit"
                                                                 : "declared here, defined in another unit")
                                                  : "trusted, never verified: declared in a system file";
    }
    encode_contract(r, f);
    return f;
  }

  // ---- contracts --------------------------------------------------------------------------------------------
  const Expr* clause_bool(const Expr* x, const a_source_position& p) {
    if (x == nullptr) return nullptr;
    if (x->type.is_bool()) return x;
    if (x->type.is_int()) return b_.to_bool(x, loc_of(p));
    reject(Diag::unsupported_expression, p, "a condition that is not a number or a bool");
    return nullptr;
  }

  void encode_contract(a_routine_ptr r, vir::Function* f) {
    Ctx saved = std::move(cx_);
    cx_ = Ctx();
    cx_.routine = r;
    cx_.fn = f;
    for (an_attribute_ptr a : contracts_of(r)) {
      const Annot k = classify(a);
      cx_.contract = true;
      cx_.annot = k;
      cx_.binder_present = has_binder(a);
      cx_.stmt_pos = a->position;
      cx_.old_depth = 0;
      if (k == Annot::decreases) {
        // The termination measure of the function (decision 0013): one or more integer expressions.
        encode_measure(a, f->decreases_);
        continue;
      }
      an_expr_node_ptr arg = nullptr;
      int exprs = 0, others = 0;
      for (const AnnotArg& g : arguments_of(a)) {
        if (g.kind == AnnotArg::Kind::expression) {
          arg = g.expr;
          ++exprs;
        } else if (g.kind != AnnotArg::Kind::binder) {
          ++others;
        }
      }
      if (exprs != 1 || others != 0 || arg == nullptr) {
        reject(Diag::unsupported_expression, a->position, "this form of \"cerastes::" + std::string(a->name ? a->name : "?") + "\"");
        continue;
      }
      if (k == Annot::ensures && f->noreturn) {
        // It never returns, so the postcondition would never be checked (decision 0015 item 4).
        reject(Diag::ensures_on_noreturn, a->position, "");
        continue;
      }
      const Expr* c = clause_bool(expr(arg), arg->position);
      if (c == nullptr) continue;
      vir::Clause cl{c, loc_of(a->position)};
      (k == Annot::requires_ ? f->requires_ : f->ensures_).push_back(cl);
    }
    cx_ = std::move(saved);
  }

  // The components of a `decreases` (contract mode or annotation mode is already set by the caller): each an integer
  // expression, none of type bool (decision 0013 item 1).
  void encode_measure(an_attribute_ptr a, std::vector<vir::Clause>& out) {
    int n = 0;
    bool ok = true;
    std::vector<vir::Clause> parts;
    const SrcLoc where = loc_of(a->position);
    for (const AnnotArg& g : arguments_of(a)) {
      if (g.kind != AnnotArg::Kind::expression || g.expr == nullptr) {
        reject(Diag::unsupported_expression, a->position, "this form of \"cerastes::decreases\"");
        ok = false;
        continue;
      }
      ++n;
      const Expr* x = expr(g.expr);
      if (x == nullptr) {
        ok = false;
        continue;
      }
      if (!x->type.is_int()) {
        reject(Diag::unsupported_expression, g.expr->position, "a measure that is not an integer");
        ok = false;
        continue;
      }
      parts.push_back(vir::Clause{x, where});
    }
    if (n == 0) {
      reject(Diag::unsupported_expression, a->position, "an empty \"cerastes::decreases\"");
      return;
    }
    if (ok) out.insert(out.end(), parts.begin(), parts.end());
  }

  // ---- constants --------------------------------------------------------------------------------------------
  const Expr* literal(a_constant_ptr c, const VType& t, const a_source_position& p) {
    a_number_buffer buf = decimal_str_for_integer_constant(c);
    const std::string text = buf.as_temp_characters();
    if (t.t.is_bool()) return b_.bool_const(text != "0", loc_of(p));
    return b_.int_const(t.t, text, loc_of(p));
  }

  // EDG's value of an integer constant as the decimal text VIR holds ("0"/"1" for a bool).
  static std::string constant_text(a_constant_ptr c, const vir::Type& t) {
    a_number_buffer buf = decimal_str_for_integer_constant(c);
    const std::string text = buf.as_temp_characters();
    if (t.is_bool()) return text == "0" ? "0" : "1";
    return text;
  }

  // A literal of `t` holding `value` (a named constant's use).
  const Expr* value_literal(const vir::Type& t, const std::string& value, const a_source_position& p) {
    if (t.is_bool()) return b_.bool_const(value != "0", loc_of(p));
    return b_.int_const(t, value, loc_of(p));
  }

  // 0011 item 13, perturb_folded: a neighbouring value of the same type, so that an `internal` guard of decision 0017 fails.
  static std::string perturb_text(const vir::Type& t, const std::string& v) {
    if (t.is_bool()) return v == "0" ? "1" : "0";
    if (!t.is_int() || t.int_type == nullptr) return v;
    const vir::dec::Range r = vir::dec::range_of(*t.int_type);
    auto inc = [](const std::string& a) {  // a + 1 for a non-negative decimal
      std::string s = a;
      size_t i = s.size();
      while (i > 0) {
        --i;
        if (s[i] == '9') {
          s[i] = '0';
        } else {
          ++s[i];
          return s;
        }
      }
      return "1" + s;
    };
    if (vir::dec::cmp(v, r.lo) <= 0) {  // at the minimum: +1
      if (vir::dec::negative(r.lo)) return vir::dec::neg(vir::dec::dec1(r.lo.substr(1)));
      return inc(v);
    }
    if (v == "0") return "-1";
    if (vir::dec::negative(v)) return "-" + inc(v.substr(1));
    return vir::dec::dec1(v);
  }
  static std::string maybe_perturb(const vir::Type& t, const std::string& v) {
    return verify_config().hook == "perturb_folded" ? perturb_text(t, v) : v;
  }

  const Expr* constant(an_expr_node_ptr e) { return constant_value(e->variant.constant.ptr, e->position); }

  // A folded constant `c` at `pos`: its original expression when EDG recorded one (0010 item 12), else its literal value.
  // Also used for the value of a `case` label (0014 item 6).
  const Expr* constant_value(a_constant_ptr c, const a_source_position& pos) {
    if (c == nullptr) {
      reject(Diag::unsupported_expression, pos, "a constant that cannot be read");
      return nullptr;
    }
    if (c->kind == ck_address && c->variant.address.kind == abk_label) {
      reject(Diag::unsupported_expression, pos, "the address of a label (GNU)");
      return nullptr;
    }
    if (c->kind != ck_integer) {
      reject(Diag::unsupported_expression, pos, "a constant that is not an integer or a bool");
      return nullptr;
    }
    {
      a_type_ptr st = c->type ? skip_typerefs(c->type) : nullptr;
      if (st != nullptr && st->kind == tk_integer && st->variant.integer.enum_type) {
        reject(Diag::unsupported_expression, pos, "an enumeration constant");
        return nullptr;
      }
    }
    VType t;
    if (!value_type(c->type, pos, &t)) return nullptr;
    an_expr_node_ptr original = (c->expr != nullptr || c->local_expr_ref) ? expr_node_from_constant(c) : nullptr;
    const bool hook = verify_config().hook == "constant_without_expr";
    if (original != nullptr && !hook) {
      // sizeof and alignof give their value: no run-time operation (0010 item 12).
      if (original->kind == enk_sizeof || original->kind == enk_alignof || original->kind == enk_datasizeof)
        return literal(c, t, pos);
      // Encode what was written, so that the checks of items 3-8 apply as written; a call in it ran at compile
      // time and is rejected by `call`.
      ++cx_.const_depth;
      const Expr* x = expr(original);
      --cx_.const_depth;
      if (x == nullptr) return nullptr;
      if (x->type != t.t) {
        reject(Diag::unsupported_expression, pos, "a folded constant whose original expression has another type");
        return nullptr;
      }
      return x;
    }
    if (original != nullptr || c->orig_type != nullptr) {
      // A constant converted from another type whose original expression is not recorded: its value could hide
      // an overflowing conversion, so it is not trusted (0010 item 12).
      reject(Diag::unsupported_expression, pos, "a folded constant whose original expression is not recorded");
      return nullptr;
    }
    return literal(c, t, pos);
  }

  // ---- expressions --------------------------------------------------------------------------------------------
  static std::vector<an_expr_node_ptr> operands_of(an_expr_node_ptr e) {
    std::vector<an_expr_node_ptr> out;
    for (an_expr_node_ptr o = e->variant.operation.operands; o != nullptr; o = o->next) out.push_back(o);
    return out;
  }

  bool operand_count(an_expr_node_ptr e, size_t n, const std::vector<an_expr_node_ptr>& ops) {
    if (ops.size() == n) return true;
    reject(Diag::unsupported_expression, e->position, "an operation with an unexpected number of operands");
    return false;
  }

  const Expr* as_bool(const Expr* x, const a_source_position& p) { return clause_bool(x, p); }

  // The value of a logical operation (a Bool) as the node's own type: `bool` in C++, `int` in C (0010 item 6).
  const Expr* logical_result(an_expr_node_ptr e, const Expr* x) {
    if (x == nullptr) return nullptr;
    VType nt;
    if (!value_type(e->type, e->position, &nt)) return nullptr;
    if (nt.t.is_bool()) return x;
    return b_.bool_to_int(nt.t, x, loc_of(e->position));
  }

  // Checks the node's type against the VIR type of what was built (a mismatch fails closed).
  const Expr* finish(an_expr_node_ptr e, const Expr* x) {
    if (x == nullptr) return nullptr;
    VType nt;
    if (!value_type(e->type, e->position, &nt)) return nullptr;
    if (x->type != nt.t) {
      reject(Diag::unsupported_expression, e->position, "an operation whose type is not what its operands give");
      return nullptr;
    }
    return x;
  }

  const Expr* variable(an_expr_node_ptr e) {
    a_variable_ptr v = e->variant.variable.ptr;
    if (v != nullptr) {
      const char* vn = v->source_corresp.name;
      if (cx_.case_value && cx_.case_var.empty()) cx_.case_var = vn ? vn : "?";
      // A named constant (decision 0017 item 3) is its value: a literal of its type at the use, also in a contract.
      auto nc = named_.find(v);
      if (nc != named_.end()) return value_literal(nc->second.type, nc->second.value, e->position);
      if (v == encoding_named_) {
        reject(Diag::unsupported_declaration, v->source_corresp.decl_position, "a constant named in its own initializer");
        return nullptr;
      }
      if (rejected_globals_.count(v) != 0) return nullptr;  // diagnosed at its declaration
    }
    if (v == nullptr || cx_.contract) {
      reject(Diag::unsupported_expression, e->position, "a variable reference that cannot be read");
      return nullptr;
    }
    if (cx_.rejected_vars.count(v) != 0) return nullptr;  // diagnosed at its declaration
    auto it = cx_.vars.find(v);
    if (it == cx_.vars.end()) {
      const char* n = v->source_corresp.name;
      const a_source_position dp = v->source_corresp.decl_position;
      if (v->source_corresp.enclosing_routine == nullptr && dp.seq != 0 && !in_verified_file(dp))
        reject(Diag::unsupported_expression, e->position,
               std::string("a global declared in a system file (`") + (n ? n : "?") + "`; system files are used only through contracts)");
      else
        reject(Diag::unsupported_expression, e->position,
               std::string("the variable \"") + (n ? n : "?") + "\", which is not a local or a parameter of this function");
      return nullptr;
    }
    if (cx_.case_value && il_header.source_language != sl_C && cx_.declaring != v && cx_.pending.count(v) == 0) {
      bool failed = false;
      if (const Expr* x = local_const_use(v, e->position, &failed)) return x;
      if (failed) return nullptr;  // diagnosed
    }
    if (cx_.declaring == v || cx_.pending.count(v) != 0) {
      reject(Diag::unsupported_declaration, v->source_corresp.decl_position, "a local variable used in its own initializer");
      return nullptr;
    }
    return b_.local(it->second, loc_of(e->position));
  }

  // A reference to a `const`/`constexpr` local from a case value (C++, decision 0017 item 6): the local becomes a function
  // local constant once (its initializer checked as written in the function's prologue), and the use is its value.  Null
  // with `*failed` false when `v` is not such a local (the use stays a local read, which the case label then rejects).
  const Expr* local_const_use(a_variable_ptr v, const a_source_position& pos, bool* failed) {
    auto done = cx_.local_consts.find(v);
    if (done != cx_.local_consts.end()) return value_literal(done->second.type, done->second.value, pos);
    auto in = cx_.local_const_inits.find(v);
    if (in == cx_.local_const_inits.end() || cx_.fn == nullptr) return nullptr;
    const Ctx::ConstInit init = in->second;
    VType t;
    if (!value_type(v->type, init.at, &t)) {
      *failed = true;
      return nullptr;
    }
    const Expr* x = constant_initializer(init.c, init.at);
    if (x == nullptr) {
      *failed = true;
      return nullptr;
    }
    if (x->type != t.t) {
      reject(Diag::unsupported_expression, init.at, "an initializer whose type is not the variable's type");
      *failed = true;
      return nullptr;
    }
    const std::string value = constant_text(init.c, t.t);
    const char* nm = v->source_corresp.name;
    cx_.fn->add_local_const(nm ? nm : "", t.t, loc_of(init.at), x, maybe_perturb(t.t, value));
    cx_.local_consts[v] = Ctx::ConstValue{t.t, value};
    return value_literal(t.t, value, pos);
  }

  // The kinds of constants decision 0017 item 1 names: const-qualified (before typedefs are skipped) or constexpr.
  static bool is_const_object(a_variable_ptr v) {
    return (f_get_type_qualifiers(v->type, TRUE) & TQ_CONST) != 0 || v->is_constexpr;
  }

  // Classifies and encodes the file-scope variables of verified files (decision 0017 items 1, 2, 4): each is a named constant
  // (added to the program, in source order) or rejected at its declaration.
  void encode_named_constants(const IlEntities& ents) {
    std::vector<a_variable_ptr> vars;
    for (a_variable_ptr v : ents.variables) {
      if (v->compiler_generated) continue;
      const a_source_position p = v->source_corresp.decl_position;
      if (p.seq == 0 || !in_verified_file(p)) continue;
      vars.push_back(v);
    }
    std::stable_sort(vars.begin(), vars.end(), [](a_variable_ptr a, a_variable_ptr b) {
      return a->source_corresp.decl_position.seq < b->source_corresp.decl_position.seq;
    });
    const bool c_mode = il_header.source_language == sl_C;
    for (a_variable_ptr v : vars) {
      const a_source_position p = v->source_corresp.decl_position;
      const char* nm = v->source_corresp.name;
      const std::string name = nm ? nm : "?";
      cx_ = Ctx();
      cx_.stmt_pos = p;
      auto refuse = [&](const std::string& why) {
        reject(Diag::unsupported_declaration, p, why);
        rejected_globals_.insert(v);
      };
      {
        // Attributes are held to the allow-list like a local's or a parameter's (0011 item 7): `loader_uninitialized` and
        // `section` change what the object holds, an unknown one may.
        const auto before = reported_count();
        check_attributes(v->source_corresp.attributes);
        if (reported_count() != before) {
          rejected_globals_.insert(v);
          continue;
        }
      }
      // The type first, so that the message names the real reason (an array of const is not "not const").
      VType vt = vtype(v->type);
      if (!vt.ok || vt.t.is_void()) {
        if (vt.ok) vt.name = "void";
        reject_type(vt, p);
        rejected_globals_.insert(v);
        continue;
      }
      if (!is_const_object(v)) {
        refuse("a global variable that is not const (mutable globals come with M4.5b)");
        continue;
      }
      if (v->source_corresp.is_class_member) {
        refuse("a static data member");
        continue;
      }
      if (v->is_thread_local) {
        refuse("a thread_local constant");
        continue;
      }
      if (v->is_weak || v->is_weakref) {
        // `__attribute__((weak))`, `[[gnu::weak]]` and `#pragma weak` all set the flag: another unit may replace the value at link time.
        refuse("a weak constant (another unit may replace its value)");
        continue;
      }
      const Expr* init = nullptr;
      std::string value = "0";
      if (v->init_kind == initk_static && v->initializer.constant != nullptr && v->initializer.constant->kind == ck_integer) {
        if (c_mode) {
          // Enforced here too (defence in depth: the driver's `--diag_error=1901` refuses it first, and the diagnostic
          // pragmas that could downgrade it are rejected).  (sizeof and alignof operands are not nodes names_object follows.)
          an_expr_node probe{};
          probe.kind = enk_constant;
          probe.variant.constant.ptr = v->initializer.constant;
          if (names_object(&probe)) {
            reject(Diag::unsupported_expression, p,
                   "in C, a constant's initializer that names another object (it is not a constant expression in C)");
            rejected_globals_.insert(v);
            continue;
          }
        }
        encoding_named_ = v;
        ++cx_.const_depth;  // the initializer is a constant expression: no calls (`call` rejects them)
        init = constant_initializer(v->initializer.constant, p);
        --cx_.const_depth;
        encoding_named_ = nullptr;
        if (init == nullptr) {
          rejected_globals_.insert(v);
          continue;
        }
        if (init->type != vt.t) {
          refuse("an initializer whose type is not the constant's type");
          continue;
        }
        value = constant_text(v->initializer.constant, vt.t);
      } else if (v->init_kind == initk_none && v->storage_class == sc_extern) {
        refuse("an extern const that is not defined in this file (its value is not known here)");
        continue;
      } else if ((v->init_kind == initk_none || v->init_kind == initk_zero) && c_mode && v->storage_class == sc_static) {
        // An internal-linkage tentative definition (C23 6.9.2): zero-initialized, no initializer to check, and no other unit
        // can define it.
      } else if (v->init_kind == initk_none && c_mode && v->storage_class == sc_unspecified) {
        // External linkage: a common symbol under clang's default -fcommon, so another unit's definition wins at link time.
        refuse("a const tentative definition with external linkage (another unit may define it; write `= 0` or make it static)");
        continue;
      } else if (v->init_kind == initk_dynamic) {
        refuse("a const global whose initializer is not a constant expression (dynamic initialization)");
        continue;
      } else {
        refuse("a const whose initialization this version cannot read");
        continue;
      }
      named_[v] = NamedInfo{vt.t, value};
      // No initializer: nothing is checked, so perturb_folded leaves the value 0 (check_program rejects a perturbed one).
      prog_.add_named_const(name, vt.t, loc_of(p), init, init != nullptr ? maybe_perturb(vt.t, value) : value);
    }
    cx_ = Ctx();
  }

  const Expr* param_reference(an_expr_node_ptr e) {
    if (!cx_.contract) {
      reject(Diag::unsupported_expression, e->position, "a parameter reference outside a contract");
      return nullptr;
    }
    const ParamRef r = interpret_param_ref(e, cx_.annot, cx_.binder_present, 0);
    if (r.kind == ParamRef::Kind::param && r.num >= 1 && r.num <= cx_.fn->params.size())
      return b_.formal(*cx_.fn, static_cast<int>(r.num) - 1, loc_of(e->position));
    if (r.kind == ParamRef::Kind::binder && cx_.annot == Annot::ensures) {
      if (cx_.old_depth > 0) {
        reject(Diag::unsupported_expression, e->position, "\"cerastes::old\" applied to the result");
        return nullptr;
      }
      return b_.result(*cx_.fn, loc_of(e->position));
    }
    reject(Diag::unsupported_expression, e->position, "a parameter reference this version cannot map");
    return nullptr;
  }

  // An expression written in a user macro's body is reported where the macro is used, with a note at the token in the
  // `#define` (invariant 7).  Tokens that are macro arguments have the user's own text as their original position; on
  // one line with the use there is nothing to add.
  const Expr* macro_site(an_expr_node_ptr e, const Expr* x) {
    if (x == nullptr) return nullptr;
    const DiagPos use = position_for(e->position);
    const DiagPos own = diag_position(pos_or(e->position));
    if (!use.known || !own.known || (use.file == own.file && use.line == own.line)) return x;
    SrcLoc at;
    at.file = use.file;
    at.line = static_cast<int>(use.line);
    at.column = static_cast<int>(use.col);
    return b_.report_at(x, at, "in the macro (this is where the macro defines it)");
  }

  const Expr* expr(an_expr_node_ptr e) {
    if (e == nullptr) return nullptr;
    const bool macro_node = !cx_.contract && !cx_.annotation && e->kind == enk_operation && e->position.orig_seq != 0;
    const bool saved = cx_.use_own;
    cx_.use_own = macro_node;
    const Expr* x = expr_node(e);
    cx_.use_own = saved;
    return macro_node ? macro_site(e, x) : x;
  }

  const Expr* expr_node(an_expr_node_ptr e) {
    switch (e->kind) {
      case enk_constant: return constant(e);
      case enk_variable: return variable(e);
      case enk_param_ref: return param_reference(e);
      case enk_operation: return operation(e);
      case enk_lambda:
        reject(Diag::unsupported_declaration, e->position, "a lambda expression");
        return nullptr;
      default:
        reject(Diag::unsupported_expression, e->position, std::string("an expression of kind ") + name_of(static_cast<an_expr_node_kind>(e->kind)));
        return nullptr;
    }
  }

  const Expr* binary_arith(an_expr_node_ptr e, vir::BinaryOp op) {
    auto ops = operands_of(e);
    if (!operand_count(e, 2, ops)) return nullptr;
    const Expr* a = expr(ops[0]);
    const Expr* c = expr(ops[1]);
    if (a == nullptr || c == nullptr) return nullptr;
    return finish(e, b_.binary(op, a, c, loc_of(e->position)));
  }

  const Expr* comparison(an_expr_node_ptr e, vir::BinaryOp op) {
    auto ops = operands_of(e);
    if (!operand_count(e, 2, ops)) return nullptr;
    const Expr* a = expr(ops[0]);
    const Expr* c = expr(ops[1]);
    if (a == nullptr || c == nullptr) return nullptr;
    return logical_result(e, b_.binary(op, a, c, loc_of(e->position)));
  }

  const Expr* logical(an_expr_node_ptr e, vir::BinaryOp op) {
    auto ops = operands_of(e);
    if (!operand_count(e, 2, ops)) return nullptr;
    const Expr* a = as_bool(expr(ops[0]), ops[0]->position);
    const Expr* c = as_bool(expr(ops[1]), ops[1]->position);
    if (a == nullptr || c == nullptr) return nullptr;
    return logical_result(e, b_.binary(op, a, c, loc_of(e->position)));
  }

  // `cerastes::old(x)`, as the IL shows it in an annotation: a dereference of a call of the declared-only
  // function template, whose argument is a reference to the (lvalue) operand.
  const Expr* old_call(an_expr_node_ptr e, an_expr_node_ptr call) {
    a_routine_ptr callee = direct_callee(call);
    auto ops = operands_of(call);
    if (callee == nullptr || ops.size() != 2) {
      reject(Diag::unsupported_expression, e->position, "\"cerastes::old\" with other than one argument");
      return nullptr;
    }
    an_expr_node_ptr arg = ops[1];
    if (arg->kind != enk_operation || arg->variant.operation.kind != eok_reference_to) {
      reject(Diag::unsupported_expression, e->position, "\"cerastes::old\" of anything but a parameter");
      return nullptr;
    }
    an_expr_node_ptr inner = arg->variant.operation.operands;
    while (inner != nullptr && inner->kind == enk_operation && inner->variant.operation.kind == eok_lvalue_adjust)
      inner = inner->variant.operation.operands;
    if (inner == nullptr || inner->kind != enk_param_ref) {
      reject(Diag::unsupported_expression, e->position, "\"cerastes::old\" of anything but a parameter");
      return nullptr;
    }
    ++cx_.old_depth;
    const Expr* x = expr(inner);
    --cx_.old_depth;
    if (x == nullptr) return nullptr;
    return b_.old(x, loc_of(e->position));
  }

  const Expr* conversion(an_expr_node_ptr e) {
    auto ops = operands_of(e);
    if (!operand_count(e, 1, ops)) return nullptr;
    VType to;
    if (!value_type(e->type, e->position, &to)) return nullptr;
    VType from;
    if (!value_type(ops[0]->type, ops[0]->position, &from)) return nullptr;
    const Expr* x = expr(ops[0]);
    if (x == nullptr) return nullptr;
    if (x->type != from.t) {
      reject(Diag::unsupported_expression, e->position, "a conversion whose operand has another type than the IL says");
      return nullptr;
    }
    const SrcLoc l = loc_of(e->position);
    if (to.t.is_bool()) return from.t.is_bool() ? x : b_.to_bool(x, l);
    if (from.t.is_bool()) return b_.bool_to_int(to.t, x, l);
    if (from.t == to.t) return x;
    return b_.spelled(b_.convert(to.t, x, !e->compiler_generated, l), to.name);
  }

  const Expr* call(an_expr_node_ptr e) {
    a_routine_ptr callee = direct_callee(e);
    auto ops = operands_of(e);
    if (callee == nullptr || ops.empty()) {
      reject(Diag::unsupported_expression, e->position, "a call that does not name a function (through a pointer, or a member)");
      return nullptr;
    }
    const std::string name = rname(callee);
    if (cx_.const_depth > 0) {
      reject(Diag::unsupported_expression, e->position, "a call of \"" + name + "\" that the compiler evaluated at compile time");
      return nullptr;
    }
    if (cx_.contract || cx_.annotation) {
      reject(Diag::call_in_annotation, e->position, name);
      return nullptr;
    }
    if (is_spec_routine(callee)) {
      reject(Diag::spec_call_in_code, e->position, name);
      return nullptr;
    }
    vir::Function* f = callee_for_call(callee, e->position);
    if (f == nullptr) return nullptr;
    if (cx_.routine != nullptr && verified_body(callee) != nullptr) edges_[cx_.routine].insert(callee);  // the call graph (0013)
    if (ops.size() - 1 != f->params.size()) {
      reject(Diag::unsupported_expression, e->position, "a call whose arguments do not match the declaration of \"" + name + "\"");
      return nullptr;
    }
    std::vector<const Expr*> args;
    bool ok = true;
    for (size_t i = 1; i < ops.size(); ++i) {
      const Expr* a = expr(ops[i]);
      if (a == nullptr) {
        ok = false;
        continue;
      }
      // A default argument is the callee's expression, copied into the call: its obligations are reported at
      // the call, with a note at the default argument itself (0012 item 6).
      if (ops[i]->generated_default_arg) a = b_.report_at(a, loc_of(e->position));
      args.push_back(a);
    }
    if (!ok) return nullptr;
    const Expr* c = b_.call(f, std::move(args), loc_of(e->position));
    VType nt = vtype(e->type);
    if (!nt.ok || nt.t != f->ret) {
      reject(Diag::unsupported_expression, e->position, "a call whose type is not the declared result type");
      return nullptr;
    }
    return c;
  }

  const Expr* operation(an_expr_node_ptr e) {
    const an_expr_operator_kind k = e->variant.operation.kind;
    auto ops = operands_of(e);
    switch (k) {
      case eok_add: return binary_arith(e, vir::BinaryOp::Add);
      case eok_subtract: return binary_arith(e, vir::BinaryOp::Sub);
      case eok_multiply: return binary_arith(e, vir::BinaryOp::Mul);
      case eok_divide: return binary_arith(e, vir::BinaryOp::Div);
      case eok_remainder: return binary_arith(e, vir::BinaryOp::Rem);
      case eok_shiftl: return binary_arith(e, vir::BinaryOp::Shl);
      case eok_shiftr: return binary_arith(e, vir::BinaryOp::Shr);
      case eok_lt: return comparison(e, vir::BinaryOp::Lt);
      case eok_le: return comparison(e, vir::BinaryOp::Le);
      case eok_gt: return comparison(e, vir::BinaryOp::Gt);
      case eok_ge: return comparison(e, vir::BinaryOp::Ge);
      case eok_eq: return comparison(e, vir::BinaryOp::Eq);
      case eok_ne: return comparison(e, vir::BinaryOp::Ne);
      case eok_land: return logical(e, vir::BinaryOp::LAnd);
      case eok_lor: return logical(e, vir::BinaryOp::LOr);
      case eok_negate:
      case eok_unary_plus: {
        if (!operand_count(e, 1, ops)) return nullptr;
        const Expr* a = expr(ops[0]);
        if (a == nullptr) return nullptr;
        return finish(e, b_.unary(k == eok_negate ? vir::UnaryOp::Neg : vir::UnaryOp::Plus, a, loc_of(e->position)));
      }
      case eok_not: {
        if (!operand_count(e, 1, ops)) return nullptr;
        const Expr* a = as_bool(expr(ops[0]), ops[0]->position);
        if (a == nullptr) return nullptr;
        return logical_result(e, b_.unary(vir::UnaryOp::Not, a, loc_of(e->position)));
      }
      case eok_question: {
        if (!operand_count(e, 3, ops)) return nullptr;
        const Expr* c = as_bool(expr(ops[0]), ops[0]->position);
        const Expr* a = expr(ops[1]);
        const Expr* d = expr(ops[2]);
        if (c == nullptr || a == nullptr || d == nullptr) return nullptr;
        return finish(e, b_.cond(c, a, d, loc_of(e->position)));
      }
      case eok_cast: return conversion(e);
      case eok_bool_cast: {
        if (!operand_count(e, 1, ops)) return nullptr;
        const Expr* a = as_bool(expr(ops[0]), ops[0]->position);
        return logical_result(e, a);
      }
      case eok_call: return call(e);
      case eok_ref_indirect:
        if (cx_.contract && ops.size() == 1 && ops[0]->kind == enk_operation && ops[0]->variant.operation.kind == eok_call) {
          a_routine_ptr callee = direct_callee(ops[0]);
          if (callee != nullptr && rname(callee) == "cerastes::old") return old_call(e, ops[0]);
        }
        break;
      default: break;
    }
    reject(Diag::unsupported_expression, e->position, describe_operation(k));
    return nullptr;
  }

  // ---- statements -------------------------------------------------------------------------------------------
  using Out = std::vector<const Stmt*>;

  const Stmt* block_of(Out&& v, const a_source_position& p) { return b_.block(std::move(v), loc_of(p)); }

  const Stmt* sub_statement(a_statement_ptr s, const a_source_position& p) {
    Out out;
    statement(s, out);
    return block_of(std::move(out), p);
  }

  // The "does not return" flag of `r` is the user's claim: the first declaration is at a known position in a file that
  // is placed and verified (builtins and unplaceable files are never trusted here, though in_verified_file is true for the
  // latter), and no `noreturn` attribute entry sits outside a verified file (`void quick_exit(int);` before
  // `#include <stdlib.h>` takes its flag from the SDK's `__dead2`).  `_Noreturn` leaves no entry, so a system header's
  // `_Noreturn` on a function the user declared first cannot be told apart; that is the user's contract (0011 items 5-6).
  bool user_declared_noreturn(a_routine_ptr r) {
    a_routine_type_supplement_ptr x = routine_supplement(r);
    if (x == nullptr || !x->does_not_return) return false;
    const a_source_position decl = r->source_corresp.decl_position;
    if (decl.seq == 0 || physical_path(decl).empty() || !in_verified_file(decl)) return false;
    for (an_attribute_ptr a = r->source_corresp.attributes; a != nullptr; a = a->next)
      if (a->kind == ak_noreturn && (a->position.seq == 0 || physical_path(a->position).empty() || !in_verified_file(a->position)))
        return false;
    return true;
  }

  // The trusted C library table (decision 0015 item 5): `void abort(void)`, `void exit(int)` and `void _Exit(int)`, declared
  // only in system files, with C linkage and EDG's "does not return" flag.  Returns the tag for the emitted Viper (a
  // `libc:` reason; the lint requires a tag on every bodyless method), or null for any other routine: `quick_exit`, the builtins, `std::terminate` and a user's own
  // function of the same name keep their ordinary treatment.
  const char* trusted_libc_reason(a_routine_ptr r) {
    a_routine_type_supplement_ptr x = routine_supplement(r);
    if (x == nullptr || !x->does_not_return || !x->prototyped || x->has_ellipsis) return nullptr;
    const a_source_position decl = r->source_corresp.decl_position;
    if (decl.seq == 0 || in_verified_file(decl)) return nullptr;  // builtins have no position
    if (r->source_corresp.name == nullptr || r->source_corresp.is_class_member) return nullptr;
    if (r->source_corresp.name_linkage != nlk_external) return nullptr;  // C linkage (P21)
    if (r->is_template_function || r->assoc_template != nullptr || r->is_prototype_instantiation) return nullptr;
    a_type_ptr rt = return_type(r);
    if (rt == nullptr || skip_typerefs(rt)->kind != tk_void) return nullptr;
    size_t nparams = 0;
    bool first_is_int = false;
    for (a_param_type_ptr p = x->param_type_list; p != nullptr; p = p->next, ++nparams) {
      a_type_ptr t = p->type ? skip_typerefs(p->type) : nullptr;
      if (nparams == 0)
        first_is_int = t != nullptr && t->kind == tk_integer && !t->variant.integer.enum_type &&
                       !t->variant.integer.bool_type && t->variant.integer.int_kind == ik_int;
    }
    const std::string n = r->source_corresp.name;
    if (n == "abort" && nparams == 0) return "libc: abort never returns (C23 7.24.4.1)";
    if (n == "exit" && nparams == 1 && first_is_int) return "libc: exit never returns (C23 7.24.4.4)";
    if (n == "_Exit" && nparams == 1 && first_is_int) return "libc: _Exit never returns (C23 7.24.4.5)";
    return nullptr;
  }

  // The callee's VIR function for a call, or null after a rejection (0011 item 5).
  vir::Function* callee_for_call(a_routine_ptr callee, const a_source_position& at) {
    if (is_main(callee)) {
      // C allows it; nothing here relies on `main`'s special rules, so the call is rejected in both modes (M8).
      reject(Diag::unsupported_expression, at, "a call of \"main\"");
      return nullptr;
    }
    auto it = fns_.find(callee);
    if (it != fns_.end() && it->second != nullptr) return it->second;
    const bool has_body_here = verified_body(callee) != nullptr;
    if (has_body_here) {
      // Defined in a verified file: created when its turn comes; created here if the call is first.
      vir::Function* f = function_of(callee, true, at);
      return f;
    }
    if (const char* why = trusted_libc_reason(callee)) {
      vir::Function* f = function_of(callee, false, at);
      if (f != nullptr) f->trusted_reason = why;
      return f;
    }
    bool has_contract = false;  // a `decreases` alone is not a contract (0011 item 5)
    for (an_attribute_ptr a : contracts_of(callee)) {
      const Annot k = classify(a);
      if (k == Annot::requires_ || k == Annot::ensures) has_contract = true;
    }
    // A function declared not to return, in a verified file, counts as a contract: it ensures `false` (decision 0015 item 4,
    // amending 0011 item 5).  It is a user contract, trusted like a user's `ensures`, even when a system header declares
    // the function too (EDG keeps only the first declaration's position).  But the flag must be the user's: see
    // user_declared_noreturn.
    if (user_declared_noreturn(callee)) has_contract = true;
    if (!has_contract) {
      reject(Diag::call_without_contract, at, rname(callee));
      return nullptr;
    }
    if (callee->is_template_function || callee->assoc_template != nullptr || callee->is_prototype_instantiation) {
      reject(Diag::unsupported_expression, at, "a call of the function template \"" + rname(callee) + "\"");
      return nullptr;
    }
    return function_of(callee, false, at);
  }

  void annotation_statement(an_attribute_ptr a, a_statement_ptr s, Out& out) {
    const Annot k = classify(a);
    if (k != Annot::check) {
      reject(Diag::unsupported_statement, s->position, "\"cerastes::" + std::string(a->name ? a->name : "?") + "\" here");
      return;
    }
    an_expr_node_ptr arg = nullptr;
    int exprs = 0;
    for (const AnnotArg& g : arguments_of(a))
      if (g.kind == AnnotArg::Kind::expression) {
        arg = g.expr;
        ++exprs;
      }
    if (exprs != 1 || arg == nullptr) {
      reject(Diag::unsupported_expression, a->position, "this form of \"cerastes::check\"");
      return;
    }
    cx_.annotation = true;
    const Expr* c = clause_bool(expr(arg), arg->position);
    cx_.annotation = false;
    if (c != nullptr) out.push_back(b_.check(c, loc_of(s->position)));
  }

  // The `invariant` and `decreases` clauses written before a loop.  A loop without a measure is rejected (0013 item 3).
  // `require_measure` is false only for `do ... while (0)` (0014 item 5).
  bool encode_loop_clauses(a_statement_ptr s, std::vector<vir::Clause>& invariants, std::vector<vir::Clause>& measure,
                           bool require_measure = true) {
    bool ok = true;
    bool has_decreases = false;
    for (an_attribute_ptr a = s->attributes; a != nullptr; a = a->next) {
      if (!is_registered_cerastes(a)) continue;
      const Annot k = classify(a);
      if (k == Annot::decreases) {
        has_decreases = true;
        cx_.annotation = true;
        const size_t before = measure.size();
        encode_measure(a, measure);
        cx_.annotation = false;
        if (measure.size() == before) ok = false;
        continue;
      }
      if (k != Annot::invariant) {
        reject(Diag::unsupported_statement, s->position, "\"cerastes::" + std::string(a->name ? a->name : "?") + "\" on a loop");
        ok = false;
        continue;
      }
      an_expr_node_ptr arg = nullptr;
      int exprs = 0;
      for (const AnnotArg& g : arguments_of(a))
        if (g.kind == AnnotArg::Kind::expression) {
          arg = g.expr;
          ++exprs;
        }
      if (exprs != 1 || arg == nullptr) {
        reject(Diag::unsupported_expression, a->position, "this form of \"cerastes::invariant\"");
        ok = false;
        continue;
      }
      cx_.annotation = true;
      const Expr* c = clause_bool(expr(arg), arg->position);
      const SrcLoc clause_loc = loc_of(a->position);
      cx_.annotation = false;
      if (c == nullptr) {
        ok = false;
        continue;
      }
      invariants.push_back(vir::Clause{c, clause_loc});
    }
    if (!has_decreases && require_measure) {
      reject_at(Diag::missing_decreases, loop_keyword_position(s),
                "this loop has no termination measure: write CERASTES_DECREASES(...) before it");
      ok = false;
    }
    return ok;
  }

  // A declaration: the variable and its type; the initializer comes with the `stmk_init` (P11).
  void declaration(a_statement_ptr s, Out& out) {
    for (an_il_entity_list_entry_ptr en = s->variant.decl.entities; en != nullptr; en = en->next) {
      const a_tagged_pointer& t = en->entity;
      if (t.kind == iek_static_assertion) continue;  // checked by EDG; no run-time effect
      if (t.kind == iek_type) {
        a_type_ptr ty = reinterpret_cast<a_type_ptr>(t.ptr);
        if (ty->kind == tk_typeref) continue;  // a typedef
        reject(Diag::unsupported_declaration, ty->source_corresp.decl_position.seq ? ty->source_corresp.decl_position : s->position,
               "a type declaration");
        continue;
      }
      if (t.kind != iek_variable) {
        reject(Diag::unsupported_declaration, s->position, "this kind of declaration");
        continue;
      }
      a_variable_ptr v = reinterpret_cast<a_variable_ptr>(t.ptr);
      const a_source_position at = v->source_corresp.decl_position;
      check_attributes(v->source_corresp.attributes);
      if (v->storage_class == sc_static || v->storage_class == sc_extern || v->is_thread_local) {
        reject(Diag::unsupported_declaration, at,
               v->storage_class == sc_extern ? "an extern declaration in a function"
               : v->is_thread_local          ? "a thread_local variable"
                                             : "a static local variable");
        cx_.rejected_vars.insert(v);
        continue;
      }
      const bool vla = v->is_vla || v->has_variably_modified_type;
      VType vt = vtype(v->type);
      if (!vt.ok || vt.t.is_void()) {
        if (vt.ok) vt.name = "void";
        reject_type(vt, at);
        if (vla) cx_.suppress = true;  // what follows (the size, the deallocation) is the compiler's
        cx_.rejected_vars.insert(v);
        continue;
      }
      if (v->init_kind == initk_none) {
        reject(Diag::unsupported_declaration, at, "a local variable without an initializer");
        cx_.rejected_vars.insert(v);
        continue;
      }
      const char* nm = v->source_corresp.name;
      vir::Var* var = cx_.fn->new_local(nm ? nm : "", vt.t, loc_of(at));
      var->c_type_name = vt.name;
      cx_.vars[v] = var;
      if (v->init_kind == initk_static) {
        // A constant initializer with no `stmk_init` statement.
        if (is_const_object(v) && v->initializer.constant != nullptr)
          cx_.local_const_inits[v] = Ctx::ConstInit{v->initializer.constant, at};
        initializer(v, var, nullptr, v->initializer.constant, at, out);
      } else {
        cx_.pending.insert(v);
      }
    }
  }

  void initializer(a_variable_ptr v, vir::Var* var, an_expr_node_ptr init_expr, a_constant_ptr init_constant,
                   const a_source_position& at, Out& out) {
    cx_.declaring = v;
    const Expr* x = nullptr;
    if (init_expr != nullptr) {
      x = expr(init_expr);
    } else if (init_constant != nullptr) {
      x = constant_initializer(init_constant, at);
    } else {
      reject(Diag::unsupported_declaration, at, "an initializer this version cannot read");
    }
    cx_.declaring = nullptr;
    cx_.pending.erase(v);
    if (x == nullptr) return;
    if (x->type != var->type) {
      reject(Diag::unsupported_expression, at, "an initializer whose type is not the variable's type");
      return;
    }
    out.push_back(b_.decl(var, x, loc_of(at)));
  }

  const Expr* constant_initializer(a_constant_ptr c, const a_source_position& at) {
    // A constant used as an initializer is read like a constant expression node of its own type.
    an_expr_node node{};
    node.kind = enk_constant;
    node.type = c->type;
    node.variant.constant.ptr = c;
    node.position = at;
    return constant(&node);
  }

  void init_statement(a_statement_ptr s, Out& out) {
    a_dynamic_init_ptr d = s->variant.dynamic_init;
    if (d == nullptr || d->variable == nullptr) {
      reject(Diag::unsupported_declaration, s->position, "an initialization that is not of a local variable");
      return;
    }
    a_variable_ptr v = d->variable;
    auto it = cx_.vars.find(v);
    if (it == cx_.vars.end() || cx_.pending.count(v) == 0) return;  // rejected at its declaration
    const a_source_position at = v->source_corresp.decl_position.seq ? v->source_corresp.decl_position : s->position;
    if (d->kind == dik_expression) {
      initializer(v, it->second, d->variant.expression, nullptr, s->position.seq ? s->position : at, out);
    } else if (d->kind == dik_constant) {
      // A `const` local with a constant initializer: a case value may name it (decision 0017 item 6, C++).
      if (is_const_object(v) && d->variant.constant.ptr != nullptr)
        cx_.local_const_inits[v] = Ctx::ConstInit{d->variant.constant.ptr, at};
      initializer(v, it->second, nullptr, d->variant.constant.ptr, s->position.seq ? s->position : at, out);
    } else {
      cx_.pending.erase(v);
      reject(Diag::unsupported_declaration, at, "this kind of initialization");
    }
  }

  // The left operand of an assignment, an increment or a compound assignment: a mutable local of an integer
  // or bool type.
  vir::Var* lvalue_var(an_expr_node_ptr e) {
    if (e->kind != enk_variable || e->variant.variable.ptr == nullptr) {
      reject(Diag::unsupported_expression, e->position, "an assignment to anything but a local variable");
      return nullptr;
    }
    if (cx_.rejected_vars.count(e->variant.variable.ptr) != 0) return nullptr;
    auto it = cx_.vars.find(e->variant.variable.ptr);
    if (it == cx_.vars.end()) {
      reject(Diag::unsupported_expression, e->position, "an assignment to a variable that is not a local of this function");
      return nullptr;
    }
    return it->second;
  }

  // ---- assert (decision 0015 item 6, P22) ----------------------------------------------------------------------
  static bool is_operation(an_expr_node_ptr e, an_expr_operator_kind k, size_t operands) {
    return e != nullptr && e->kind == enk_operation && e->variant.operation.kind == k && operands_of(e).size() == operands;
  }

  static bool has_type_kind(a_type_ptr t, a_type_kind k) { return t != nullptr && skip_typerefs(t)->kind == k; }

  // A folded constant's original expression, looked through recursively; the node itself when it is not a folded constant.
  static an_expr_node_ptr folded_original(an_expr_node_ptr e) {
    for (int guard = 0; guard < 16 && e != nullptr && e->kind == enk_constant; ++guard) {
      a_constant_ptr c = e->variant.constant.ptr;
      if (c == nullptr || c->expr == nullptr || c->local_expr_ref) break;
      an_expr_node_ptr o = expr_node_from_constant(c);
      if (o == nullptr || o == e) break;
      e = o;
    }
    return e;
  }

  // The integer constant 0 as the macro writes it: no original expression (0010 item 12) except the implicit conversion of
  // the literal `0` to the type of `__builtin_expect`'s parameter.  A folded `(void)(1 - 1)` is not plain.
  static bool is_plain_zero(an_expr_node_ptr e) {
    if (!is_zero_constant(e)) return false;
    a_constant_ptr c = e->variant.constant.ptr;
    if (c->local_expr_ref) return false;
    if (c->expr == nullptr) return true;
    an_expr_node_ptr o = expr_node_from_constant(c);
    while (o != nullptr && o->kind == enk_operation && o->variant.operation.kind == eok_cast && operands_of(o).size() == 1 &&
           has_type_kind(o->type, tk_integer))
      o = operands_of(o)[0];
    return o != nullptr && o != e && o->kind == enk_constant && o->variant.constant.ptr != nullptr &&
           o->variant.constant.ptr->expr == nullptr && !o->variant.constant.ptr->local_expr_ref && is_zero_constant(o);
  }

  // The system function the SDK's `assert` calls on failure: declared in a system file, C linkage, void, does not return.
  static bool is_assert_failure_function(a_routine_ptr r) {
    if (r == nullptr || r->source_corresp.name == nullptr || std::strcmp(r->source_corresp.name, "__assert_rtn") != 0) return false;
    const a_source_position decl = r->source_corresp.decl_position;
    if (decl.seq == 0 || in_verified_file(decl) || r->source_corresp.is_class_member) return false;
    if (r->source_corresp.name_linkage != nlk_external) return false;
    a_routine_type_supplement_ptr x = routine_supplement(r);
    return x != nullptr && x->does_not_return && has_type_kind(return_type(r), tk_void);
  }

  // `e` of `assert(e)`, when `s` is exactly the SDK's expansion
  //   (__builtin_expect(!(e), 0) ? __assert_rtn(...) : (void)0)
  // (C++ puts a bool conversion on the condition); otherwise null.  Anything else stays rejected as before.
  static an_expr_node_ptr assert_condition(an_expr_node_ptr s) {
    if (!is_operation(s, eok_question, 3) || !has_type_kind(s->type, tk_void)) return nullptr;
    auto q = operands_of(s);
    an_expr_node_ptr cond = q[0];
    if (is_operation(cond, eok_bool_cast, 1)) cond = operands_of(cond)[0];
    if (!is_operation(cond, eok_call, 3)) return nullptr;
    a_routine_ptr expect = direct_callee(cond);
    if (expect == nullptr || expect->source_corresp.name == nullptr ||
        std::strcmp(expect->source_corresp.name, "__builtin_expect") != 0 || expect->special_kind != sfk_none ||
        expect->variant.builtin_function_kind == bfk_none)
      return nullptr;
    auto args = operands_of(cond);  // [0] is the routine
    // `assert(0)`: EDG folds `!(0)` (and the `(long)` conversion of it) to a constant whose original expression is the
    // operation (0010 item 12); see through it.  A constant with no original expression there does not match.
    an_expr_node_ptr first = folded_original(args[1]);
    if (!is_operation(first, eok_cast, 1) || !has_type_kind(first->type, tk_integer)) return nullptr;
    an_expr_node_ptr negated = folded_original(operands_of(first)[0]);
    if (!is_operation(negated, eok_not, 1)) return nullptr;
    if (!is_plain_zero(args[2])) return nullptr;
    an_expr_node_ptr failure = q[1];
    if (failure == nullptr || failure->kind != enk_operation || failure->variant.operation.kind != eok_call) return nullptr;
    if (!is_assert_failure_function(direct_callee(failure))) return nullptr;
    an_expr_node_ptr fallthrough = q[2];
    if (!is_operation(fallthrough, eok_cast, 1) || !has_type_kind(fallthrough->type, tk_void) ||
        !is_plain_zero(operands_of(fallthrough)[0]))
      return nullptr;
    return operands_of(negated)[0];
  }

  // An expression statement: an assignment, a compound assignment, an increment or decrement, a call, or an
  // expression without side effects.
  void expression_statement(an_expr_node_ptr e, const a_source_position& pos, Out& out) {
    // `(void)e;`
    while (e->kind == enk_operation && e->variant.operation.kind == eok_cast) {
      a_type_ptr to = skip_typerefs(e->type);
      if (to == nullptr || to->kind != tk_void) break;
      auto ops = operands_of(e);
      if (ops.size() != 1) break;
      e = ops[0];
    }
    if (an_expr_node_ptr cond = assert_condition(e)) {
      // `assert(cond)` from <assert.h> (decision 0015 item 6): `cond` is code, evaluated as an `if` condition is; the
      // arguments of `__assert_rtn` are never encoded.
      if (cx_.in_for_step) {
        // A check in a loop's step is an internal error of check_program: reject it here, at the source.
        reject(Diag::unsupported_expression, pos, "an assert in the increment of a for loop");
        return;
      }
      const Expr* c = as_bool(expr(cond), cond->position);
      if (c != nullptr) out.push_back(b_.check(c, loc_of(pos), true));
      return;
    }
    if (e->kind == enk_operation) {
      const an_expr_operator_kind k = e->variant.operation.kind;
      auto ops = operands_of(e);
      const SrcLoc l = loc_of(e->position);
      switch (k) {
        case eok_assign: {
          if (ops.size() != 2) break;
          vir::Var* v = lvalue_var(ops[0]);
          const Expr* x = expr(ops[1]);
          if (v == nullptr || x == nullptr) return;
          out.push_back(b_.assign(v, x, l));
          return;
        }
        case eok_add_assign:
        case eok_subtract_assign:
        case eok_multiply_assign:
        case eok_divide_assign:
        case eok_remainder_assign:
        case eok_shiftl_assign:
        case eok_shiftr_assign: {
          if (ops.size() != 2) break;
          vir::Var* v = lvalue_var(ops[0]);
          const Expr* x = expr(ops[1]);
          if (v == nullptr || x == nullptr) return;
          if (!v->type.is_int()) {
            reject(Diag::unsupported_expression, e->position, "a compound assignment to a bool");
            return;
          }
          if (x->type.is_bool()) {
            // `i += flag`: the bool is promoted to int first.
            const vir::IntType* i = prog_.int_type("int", prog_.c_int_bits(), true);
            x = b_.bool_to_int(vir::Type::integer(i), x, l);
          }
          vir::BinaryOp op = k == eok_add_assign        ? vir::BinaryOp::Add
                             : k == eok_subtract_assign ? vir::BinaryOp::Sub
                             : k == eok_multiply_assign ? vir::BinaryOp::Mul
                             : k == eok_divide_assign   ? vir::BinaryOp::Div
                             : k == eok_remainder_assign ? vir::BinaryOp::Rem
                             : k == eok_shiftl_assign   ? vir::BinaryOp::Shl
                                                        : vir::BinaryOp::Shr;
          out.push_back(b_.compound_assign(v, op, x, l));
          return;
        }
        case eok_pre_incr:
        case eok_post_incr:
        case eok_pre_decr:
        case eok_post_decr: {
          if (ops.size() != 1) break;
          vir::Var* v = lvalue_var(ops[0]);
          if (v == nullptr) return;
          if (!v->type.is_int()) {
            reject(Diag::unsupported_expression, e->position, "an increment or decrement of a bool");
            return;
          }
          out.push_back(b_.inc_dec(v, k == eok_pre_incr || k == eok_post_incr, l));
          return;
        }
        default: break;
      }
    }
    const Expr* x = expr(e);
    if (x != nullptr) out.push_back(b_.expr_stmt(x, loc_of(pos)));
  }

  void return_statement(a_statement_ptr s, Out& out) {
    vir::Function* f = cx_.fn;
    if (s->variant.return_dynamic_init != nullptr) {
      reject(Diag::unsupported_statement, s->position, "a return that initializes its result by a constructor");
      return;
    }
    if (s->expr == nullptr) {
      if (!f->ret.is_void()) {
        // Reaching the closing brace of a non-void function: the end of the body (lowering appends the
        // missing-return check).  A written `return;` is a constraint violation.
        if (s->compiler_generated) return;
        reject(Diag::unsupported_statement, s->position, "a return without a value in a function that returns a value");
        return;
      }
      if (s->compiler_generated) return;  // the end of a void function; lowering appends the postconditions
      out.push_back(b_.return_stmt(nullptr, loc_of(s->position)));
      return;
    }
    if (f->ret.is_void()) {
      reject(Diag::unsupported_statement, s->position, "a return with a value in a function that returns void");
      return;
    }
    const Expr* x = expr(s->expr);
    if (x == nullptr) return;
    // `main`'s generated `return 0` has no position: its postconditions are reported at the closing brace.
    SrcLoc l = s->position.seq != 0 ? loc_of(s->position) : f->end_loc;
    out.push_back(b_.return_stmt(x, l));
  }

  void rejected_statement(a_statement_ptr s, const std::string& what) {
    if (s->kind == stmk_end_test_while || s->kind == stmk_range_based_for || s->kind == stmk_for_each)
      reject_at(Diag::unsupported_statement, loop_keyword_position(s), what);
    else
      reject(Diag::unsupported_statement, s->position, what);
  }

  // ---- break, continue, do, switch (decision 0014) -----------------------------------------------------------
  static a_label_ptr label_of(a_statement_ptr s) {
    return s != nullptr && s->kind == stmk_label ? s->variant.label.ptr : nullptr;
  }

  // The label of the `break` of a loop or switch: its next sibling (P13).
  static a_label_ptr break_label_after(a_statement_ptr s, bool is_switch) {
    a_label_ptr l = label_of(s->next);
    return l != nullptr && l->break_label && (l->switch_break_label != 0) == is_switch ? l : nullptr;
  }

  // The label of the `continue` of a loop: the last statement of its body (P13), whether the front end wrapped the body
  // in a generated block or not.
  static a_label_ptr continue_label_of(a_statement_ptr body) {
    a_statement_ptr last = body;
    if (body != nullptr && body->kind == stmk_block) {
      last = body->variant.block.statements;
      while (last != nullptr && last->next != nullptr) last = last->next;
    }
    a_label_ptr l = label_of(last);
    return l != nullptr && l->continue_label && !l->break_label ? l : nullptr;
  }

  Ctx::Target make_target(a_statement_ptr s, bool is_switch, int id, a_statement_ptr body) {
    Ctx::Target t;
    t.is_switch = is_switch;
    t.id = id;
    t.break_label = break_label_after(s, is_switch);
    if (!is_switch) t.continue_label = continue_label_of(body);
    return t;
  }

  int label_id(a_label_ptr l) {
    auto it = cx_.label_ids.find(l);
    if (it != cx_.label_ids.end()) return it->second;
    return cx_.label_ids[l] = b_.jump_id();
  }

  // The rejections of the user's gotos that break the rule of decision 0015 item 2, at the `goto`.
  void report_goto_problems(vir::Function* f) {
    for (const vir::GotoProblem& g : vir::goto_problems(*f)) {
      auto pos = cx_.goto_positions.find(g.go);
      const a_source_position at = pos != cx_.goto_positions.end() ? pos->second : cx_.stmt_pos;
      const std::string to = "a goto to \"" + g.label + "\" that ";
      switch (g.kind) {
        case vir::GotoProblem::Kind::backward: reject(Diag::unsupported_statement, at, to + "jumps backward"); break;
        case vir::GotoProblem::Kind::enters:
          reject(Diag::unsupported_statement, at, to + "jumps into a block, a branch, a loop or a switch body");
          break;
        case vir::GotoProblem::Kind::skips_declaration:
          reject(Diag::unsupported_statement, at, to + "jumps past the declaration of \"" + g.variable + "\"");
          break;
        case vir::GotoProblem::Kind::no_label:
          reject(Diag::unsupported_statement, at, "a goto whose label this version cannot find");
          break;
      }
    }
  }

  // A `break` or `continue`: the generated goto names a label; it is accepted only when the label stands where the front
  // end puts the label of an enclosing loop or switch (so it is the C jump), and rejected otherwise.
  void jump_statement(a_statement_ptr s, a_label_ptr l, Out& out) {
    const bool is_break = l->break_label;
    for (auto it = cx_.targets.rbegin(); it != cx_.targets.rend(); ++it) {
      const bool hit = is_break ? (it->break_label == l && it->is_switch == (l->switch_break_label != 0))
                                : (!it->is_switch && it->continue_label == l);
      if (!hit) continue;
      out.push_back(is_break ? b_.break_stmt(it->id, loc_of(s->position)) : b_.continue_stmt(it->id, loc_of(s->position)));
      return;
    }
    rejected_statement(s, is_break ? "a break statement whose loop or switch this version cannot determine"
                                   : "a continue statement whose loop this version cannot determine");
  }

  // `do { B } while (0)`: the condition is an integer constant that is zero after folding (P14).
  static bool is_zero_constant(an_expr_node_ptr e) {
    if (e == nullptr || e->kind != enk_constant) return false;
    a_constant_ptr c = e->variant.constant.ptr;
    if (c == nullptr || c->kind != ck_integer) return false;
    a_number_buffer buf = decimal_str_for_integer_constant(c);
    return std::string(buf.as_temp_characters()) == "0";
  }

  // Whether an expression names a variable or a parameter, following every folded constant's original expression (0017 item 7).
  static bool names_object(an_expr_node_ptr e, int depth = 0) {
    if (e == nullptr || depth > 64) return false;
    switch (e->kind) {
      case enk_variable:
      case enk_param_ref: return true;
      case enk_constant: {
        a_constant_ptr c = e->variant.constant.ptr;
        if (c == nullptr || (c->expr == nullptr && !c->local_expr_ref)) return false;
        an_expr_node_ptr o = expr_node_from_constant(c);
        return o != nullptr && o != e && names_object(o, depth + 1);
      }
      case enk_operation:
        for (an_expr_node_ptr o = e->variant.operation.operands; o != nullptr; o = o->next)
          if (names_object(o, depth + 1)) return true;
        return false;
      default: return false;
    }
  }

  // The variable a declaration statement declares (looking through generated blocks), or null.
  static a_variable_ptr declared_variable(a_statement_ptr s) {
    if (s->kind == stmk_decl) {
      for (an_il_entity_list_entry_ptr en = s->variant.decl.entities; en != nullptr; en = en->next)
        if (en->entity.kind == iek_variable) return reinterpret_cast<a_variable_ptr>(en->entity.ptr);
      return nullptr;
    }
    if (s->kind == stmk_block && s->compiler_generated)
      for (a_statement_ptr c = s->variant.block.statements; c != nullptr; c = c->next)
        if (a_variable_ptr v = declared_variable(c)) return v;
    return nullptr;
  }

  // The name of a variable that a VIR expression reads, or null.
  static const char* variable_read_by(const Expr* e) {
    if (e == nullptr) return nullptr;
    if (e->kind == Expr::Kind::Local) return e->var != nullptr ? e->var->name.c_str() : "?";
    for (const Expr* x : {e->a, e->b, e->c})
      if (const char* n = variable_read_by(x)) return n;
    for (const Expr* x : e->args)
      if (const char* n = variable_read_by(x)) return n;
    return nullptr;
  }

  void switch_statement(a_statement_ptr s, Out& out) {
    if (s->expr == nullptr || s->expr->kind == enk_condition) {
      rejected_statement(s, "a switch statement with an init-statement or a declaration in its condition");
      return;
    }
    a_statement_ptr body = s->variant.switch_stmt.body_statement;
    // The body is a compound statement; a lone `switch (x) case 1: return 1;` is wrapped in a generated block by the front end.
    if (body == nullptr || body->kind != stmk_block) {
      rejected_statement(s, "a switch statement whose body this version cannot read");
      return;
    }
    const Expr* scrut = expr(s->expr);
    bool ok = scrut != nullptr;
    if (ok && !scrut->type.is_int()) {
      reject(Diag::unsupported_statement, s->expr->position, "a switch on a value that is not an integer");
      ok = false;
    }
    const int id = b_.jump_id();
    cx_.targets.push_back(make_target(s, true, id, nullptr));
    Out inner;
    bool seen_label = false;
    a_statement_ptr skipped = nullptr;  // a declaration of the body that a later label would jump over
    for (a_statement_ptr c = body->variant.block.statements; c != nullptr; c = c->next) {
      if (c->kind == stmk_switch_case) {
        a_switch_case_entry_ptr en = c->variant.switch_case.extra_info;
        if (c->variant.switch_case.switch_statement != s || en == nullptr) {
          reject(Diag::unsupported_statement, c->position, "a case label that this version cannot read");
          ok = false;
          continue;
        }
        check_attributes(c->attributes);  // held to the allow-list like any statement's (0014 item 11)
        for (an_attribute_ptr a = c->attributes; a != nullptr; a = a->next)
          if (is_registered_cerastes(a))
            reject(Diag::unsupported_statement, c->position, "\"cerastes::" + std::string(a->name ? a->name : "?") + "\" on a case label");
        if (en->range_end != nullptr) {
          reject(Diag::unsupported_statement, c->position, "a GNU case range (case a ... b)");
          ok = false;
          continue;
        }
        if (skipped != nullptr) {
          a_variable_ptr v = declared_variable(skipped);
          const DiagPos dp = diag_position(skipped->position);
          reject(Diag::unsupported_statement, c->position,
                 "a case label that jumps over the declaration of \"" +
                     std::string(v != nullptr && v->source_corresp.name != nullptr ? v->source_corresp.name : "?") + "\" (line " +
                     std::to_string(dp.line) + ")");
          ok = false;
          skipped = nullptr;
        }
        seen_label = true;
        if (en->case_value == nullptr) {
          inner.push_back(b_.case_label(nullptr, loc_of(c->position)));
          continue;
        }
        // The value is encoded by its original expression, like every folded constant (0010 item 12): `case 1 << 40` is a
        // shift error at the label, not the 0 that the front end folded it to.
        const a_source_position saved_stmt_pos = cx_.stmt_pos;
        if (c->position.seq != 0) cx_.stmt_pos = c->position;
        cx_.case_value = true;
        cx_.case_var.clear();
        const Expr* v = constant_value(en->case_value, c->position);
        cx_.case_value = false;
        const std::string named_var = cx_.case_var;
        cx_.stmt_pos = saved_stmt_pos;
        if (v == nullptr) {
          ok = false;
          continue;
        }
        if (il_header.source_language == sl_C && !named_var.empty()) {
          // A `const` object is not an integer constant expression in C (C23 6.6p8).  The driver makes EDG's #1901 an error in
          // C and diagnostic pragmas are rejected, so this is defence in depth (reached with a `#pragma diag_*`, which is
          // itself rejected; 0017 item 2).
          reject(Diag::unsupported_expression, c->position,
                 "in C, a case value that names a variable (`" + named_var + "`; it is not an integer constant expression in C)");
          ok = false;
          continue;
        }
        if (const char* name = variable_read_by(v)) {
          // A `const int k = 5; case k:` folds to a constant whose original expression is the variable (fail closed).
          reject(Diag::unsupported_expression, c->position, std::string("a case value that names a variable (`") + name + "`)");
          ok = false;
          continue;
        }
        if (scrut != nullptr && v->type != scrut->type) {
          reject(Diag::unsupported_statement, c->position, "a case value whose type differs from the type of the controlling expression");
          ok = false;
          continue;
        }
        // EDG's value of the label (P24); the prologue proves the value as written equal to it (decision 0017 item 5).
        inner.push_back(b_.case_label(v, maybe_perturb(v->type, constant_text(en->case_value, v->type)), loc_of(c->position)));
        continue;
      }
      if (!seen_label && !(c->kind == stmk_empty && c->attributes == nullptr)) {
        reject(Diag::unsupported_statement, c->position, "a statement before the first case label of a switch (it is unreachable)");
        ok = false;
        continue;
      }
      if (seen_label && skipped == nullptr && declared_variable(c) != nullptr) skipped = c;
      statement(c, inner);
    }
    cx_.targets.pop_back();
    if (ok) out.push_back(b_.switch_stmt(scrut, block_of(std::move(inner), body->position), id, loc_of(s->position)));
  }

  void statement(a_statement_ptr s, Out& out) {
    if (s == nullptr) return;
    if (s->position.seq != 0) cx_.stmt_pos = s->position;
    check_attributes_statement(s, out);
    switch (s->kind) {
      case stmk_empty: return;
      case stmk_block: {
        Out inner;
        if (s->compiler_generated) {
          // Guards (decision 0015 item 2; P20 found neither shape): a flattened block must not hold a user label (its scope
          // would change), and a block kept as a block must not hold a declaration (a goto past it would not see it).
          const bool flattened = s->attributes == nullptr;
          for (a_statement_ptr c = s->variant.block.statements; c != nullptr; c = c->next) {
            if (flattened && c->kind == stmk_label && c->variant.label.ptr != nullptr && !c->variant.label.ptr->break_label &&
                !c->variant.label.ptr->continue_label && !c->variant.label.ptr->switch_break_label)
              reject(Diag::unsupported_statement, c->position, "a label in a block the compiler made");
            // No shape reaches this today (P20): defence in depth.
            if (!flattened && c->kind == stmk_decl) reject(Diag::unsupported_statement, c->position, "a declaration in a block the compiler made");
          }
        }
        for (a_statement_ptr c = s->variant.block.statements; c != nullptr; c = c->next) statement(c, inner);
        if (s->compiler_generated && s->attributes == nullptr) {
          // A block the front end made (a `for` initialization, an `else if`): its statements stay at this level,
          // so that a variable declared by it is in scope for the rest.
          for (const Stmt* x : inner) out.push_back(x);
        } else {
          out.push_back(block_of(std::move(inner), s->position));
        }
        return;
      }
      case stmk_decl: declaration(s, out); return;
      case stmk_init: init_statement(s, out); return;
      case stmk_expr: expression_statement(s->expr, s->position, out); return;
      case stmk_return: return_statement(s, out); return;
      case stmk_if: {
        if (s->expr == nullptr || s->expr->kind == enk_condition) {
          rejected_statement(s, "an if statement with a declaration in its condition");
          return;
        }
        const Expr* c = as_bool(expr(s->expr), s->expr->position);
        const Stmt* t = sub_statement(s->variant.if_stmt.then_statement, s->position);
        const Stmt* f = s->variant.if_stmt.else_statement ? sub_statement(s->variant.if_stmt.else_statement, s->position) : nullptr;
        if (c != nullptr) out.push_back(b_.if_stmt(c, t, f, loc_of(s->position)));
        return;
      }
      case stmk_while: {
        if (s->expr == nullptr || s->expr->kind == enk_condition) {
          rejected_statement(s, "a while statement with a declaration in its condition");
          return;
        }
        std::vector<vir::Clause> inv, measure;
        const bool inv_ok = encode_loop_clauses(s, inv, measure);
        const Expr* c = as_bool(expr(s->expr), s->expr->position);
        const int id = b_.jump_id();
        cx_.targets.push_back(make_target(s, false, id, s->variant.loop_statement));
        const Stmt* body = sub_statement(s->variant.loop_statement, s->position);
        cx_.targets.pop_back();
        if (c != nullptr && inv_ok)
          out.push_back(b_.loop(c, std::move(inv), std::move(measure), body, nullptr, vir::LoopForm::While, id, loc_of(s->position)));
        return;
      }
      case stmk_end_test_while: {
        if (s->expr == nullptr) {
          rejected_statement(s, "a do statement this version cannot read");
          return;
        }
        // `do { B } while (0)` runs once and needs no measure (0014 item 5).
        // ... unless the condition names a variable or parameter (0017 item 7): then it is a measured loop in both modes.
        const bool once = is_zero_constant(s->expr) && !names_object(s->expr);
        std::vector<vir::Clause> inv, measure;
        const bool inv_ok = encode_loop_clauses(s, inv, measure, !once);
        const Expr* c = as_bool(expr(s->expr), s->expr->position);
        const int id = b_.jump_id();
        cx_.targets.push_back(make_target(s, false, id, s->variant.loop_statement));
        const Stmt* body = sub_statement(s->variant.loop_statement, s->position);
        cx_.targets.pop_back();
        if (c != nullptr && inv_ok)
          out.push_back(b_.loop(c, std::move(inv), std::move(measure), body, nullptr,
                                once ? vir::LoopForm::DoOnce : vir::LoopForm::DoWhile, id, loc_of(s->position)));
        return;
      }
      case stmk_for: {
        a_for_loop_ptr fl = s->variant.for_loop.extra_info;
        if (fl == nullptr) {
          rejected_statement(s, "a for statement this version cannot read");
          return;
        }
        if (s->expr != nullptr && s->expr->kind == enk_condition) {
          rejected_statement(s, "a for statement with a declaration in its condition");
          return;
        }
        // init; while (cond) { body; incr }  (0012 item 3)
        // The initialization's scope ends with the loop: it and the loop are one statement of the enclosing block, so
        // that a `for` with a declaration in a switch body is not a declaration a later case label jumps over (0014).
        Out whole;
        if (fl->initialization != nullptr) statement(fl->initialization, whole);
        std::vector<vir::Clause> inv, measure;
        const bool inv_ok = encode_loop_clauses(s, inv, measure);
        const Expr* c = s->expr ? as_bool(expr(s->expr), s->expr->position) : b_.bool_const(true, loc_of(s->position));
        Out body, step;
        const int id = b_.jump_id();
        cx_.targets.push_back(make_target(s, false, id, s->variant.for_loop.statement));
        if (s->variant.for_loop.statement != nullptr) statement(s->variant.for_loop.statement, body);
        cx_.targets.pop_back();
        // The increment is the loop's step: it runs after the body and after each `continue` (0014 item 2).
        cx_.in_for_step = true;
        if (fl->increment != nullptr) expression_statement(fl->increment, fl->increment->position, step);
        cx_.in_for_step = false;
        const Stmt* step_stmt = step.empty() ? nullptr : step.size() == 1 ? step[0] : block_of(std::move(step), fl->increment->position);
        if (c != nullptr && inv_ok)
          whole.push_back(b_.loop(c, std::move(inv), std::move(measure), block_of(std::move(body), s->position), step_stmt,
                                  vir::LoopForm::While, id, loc_of(s->position)));
        if (fl->initialization == nullptr) {
          for (const Stmt* x : whole) out.push_back(x);
        } else if (!whole.empty()) {
          out.push_back(block_of(std::move(whole), s->position));
        }
        return;
      }
      case stmk_goto: {
        a_label_ptr l = s->variant.label.ptr;
        // `break` and `continue` are compiler-generated gotos (P13); every other goto is the user's (decision 0015 item 2):
        // it is checked as a VIR statement once the function is built (goto_problems).  A compiler-generated goto (or label,
        // below) without break/continue flags is treated as a user one, and goto_problems then fails closed on it.
        if (s->compiler_generated && l != nullptr && (l->break_label || l->continue_label)) {
          jump_statement(s, l, out);
        } else if (l == nullptr) {
          rejected_statement(s, "a goto statement whose label this version cannot read");
        } else {
          const Stmt* g = b_.goto_stmt(label_id(l), loc_of(s->position));
          cx_.goto_positions[g] = s->position;
          out.push_back(g);
        }
        return;
      }
      case stmk_assigned_goto: rejected_statement(s, "a computed goto (GNU)"); return;
      case stmk_label: {
        // The labels the front end makes for `break` and `continue` follow the goto that names them (P13); the others are
        // the user's.
        a_label_ptr l = s->variant.label.ptr;
        if (l == nullptr) {
          rejected_statement(s, "a label this version cannot read");
          return;
        }
        if (l->break_label || l->continue_label || l->switch_break_label) return;
        // Label attributes are on the label entity, not on the statement (P19): held to the allow-list like any other.
        check_attributes(l->source_corresp.attributes);
        // EDG rejects a registered cerastes attribute on a label first (#1835); this is defence in depth.
        for (an_attribute_ptr a = l->source_corresp.attributes; a != nullptr; a = a->next)
          if (is_registered_cerastes(a))
            reject(Diag::unsupported_statement, s->position,
                   "\"cerastes::" + std::string(a->name ? a->name : "?") + "\" on a label");
        const char* nm = l->source_corresp.name;
        out.push_back(b_.label_stmt(label_id(l), nm != nullptr ? nm : "?", loc_of(s->position)));
        return;
      }
      case stmk_switch: switch_statement(s, out); return;
      case stmk_switch_case:
        // The labels directly in a switch body are read by switch_statement; any other one is nested (0014 item 1).
        rejected_statement(s, "a case or default label that is not directly in the braces of its switch");
        return;
      case stmk_try_block:
      case stmk_microsoft_try: rejected_statement(s, "a try statement"); return;
      case stmk_asm:
      case stmk_asm_func_body: rejected_statement(s, "an asm statement"); return;
      case stmk_range_based_for:
      case stmk_for_each: rejected_statement(s, "a range-based for statement"); return;
      case stmk_constexpr_if:
      case stmk_if_consteval:
      case stmk_if_not_consteval: rejected_statement(s, "a constexpr or consteval if statement"); return;
      case stmk_vla_decl:
      case stmk_set_vla_size:
        reject(Diag::unsupported_declaration, s->position, "a variable length array");
        return;
      case stmk_coroutine:
      case stmk_coroutine_return: rejected_statement(s, "a coroutine statement"); return;
      default:
        rejected_statement(s, std::string("a statement of kind ") + name_of(s->kind));
        return;
    }
  }

  // The attributes of a statement: cerastes's own are encoded (check) or rejected here; the others are held to
  // the allow-list.  Loop invariants belong to the loop and are read by it.
  void check_attributes_statement(a_statement_ptr s, Out& out) {
    check_attributes(s->attributes);
    for (an_attribute_ptr a = s->attributes; a != nullptr; a = a->next) {
      if (!is_registered_cerastes(a)) continue;
      if (s->kind == stmk_while || s->kind == stmk_for || s->kind == stmk_end_test_while) continue;  // read by the loop
      // The loop statements this version rejects are rejected as such, once.
      if (s->kind == stmk_range_based_for || s->kind == stmk_for_each) continue;
      annotation_statement(a, s, out);
    }
  }

  // ---- functions ----------------------------------------------------------------------------------------------
  void encode_body(a_routine_ptr r) {
    vir::Function* f = fns_[r];
    if (f == nullptr) return;
    a_scope_ptr sp = body_scope(r);
    cx_ = Ctx();
    cx_.routine = r;
    cx_.fn = f;
    cx_.stmt_pos = r->source_corresp.decl_position;
    int k = 0;
    for (a_variable_ptr p = sp->variant.routine.parameters; p != nullptr; p = p->next, ++k) {
      if (static_cast<size_t>(k) >= f->params.size()) break;
      cx_.vars[p] = const_cast<vir::Var*>(f->param_var(k));
      check_attributes(p->source_corresp.attributes);
    }
    if (static_cast<size_t>(k) != f->params.size()) {
      reject(Diag::unsupported_declaration, r->source_corresp.decl_position, "a function whose parameters cannot be matched to its type");
      return;
    }
    Out out;
    const auto reported_before = reported_count();
    statement(sp->assoc_block, out);
    for (a_variable_ptr v : cx_.pending)
      reject(Diag::unsupported_declaration, v->source_corresp.decl_position, "an initialization this version cannot read");
    f->body = b_.block(std::move(out), loc_of(r->source_corresp.decl_position));
    // Only a complete function body can be judged: after a rejection a label or a declaration may be missing.
    if (reported_count() == reported_before) report_goto_problems(f);
    cx_ = Ctx();
  }

  void declaration_level_checks(const IlEntities& ents);
  void require_measures_in_cycles();
  static bool is_vector_type_attribute(an_attribute_ptr a);
  static bool template_declared_here(a_routine_ptr r);
  static bool class_declared_here(a_routine_ptr r);
  static bool has_internal_linkage_or_inline(a_routine_ptr r);
};

// `vector_size` and its relatives only make a vector type, which every use rejects (`unsupported_type`): on a typedef
// they change nothing that is verified.
bool Encoder::is_vector_type_attribute(an_attribute_ptr a) {
  if (a->name == nullptr) return false;
  const std::string n = a->name;
  return n == "vector_size" || n == "__vector_size__" || n == "ext_vector_type" || n == "__ext_vector_type__";
}

bool Encoder::template_declared_here(a_routine_ptr r) {
  if (r->assoc_template != nullptr) {
    const a_source_position p = r->assoc_template->source_corresp.decl_position;
    return p.seq != 0 && in_verified_file(p);
  }
  // A prototype instantiation without a recorded template: the template is where the routine is declared.
  const a_source_position p = r->source_corresp.decl_position;
  return p.seq != 0 && in_verified_file(p);
}

bool Encoder::class_declared_here(a_routine_ptr r) {
  a_scope_ptr sp = r->source_corresp.parent_scope;
  if (sp == nullptr || sp->kind != sck_class_struct_union || sp->variant.assoc_type == nullptr) return false;
  const a_source_position p = sp->variant.assoc_type->source_corresp.decl_position;
  return p.seq != 0 && in_verified_file(p);
}

bool Encoder::has_internal_linkage_or_inline(a_routine_ptr r) {
  return r->storage_class == sc_static || r->is_inline ||
         r->source_corresp.name_linkage == nlk_internal;
}

void Encoder::declaration_level_checks(const IlEntities& ents) {
  // Types, variables and templates declared in verified files (0012 item 12).
  for (a_type_ptr t : ents.types) {
    // Attributes written in a verified file on a type of any kind (a typedef, the unnamed type of
    // `int __attribute__((x)) f(int)`) are held to the allow-list; the position of the attribute decides, since the
    // type may have none.
    for (an_attribute_ptr a = t->source_corresp.attributes; a != nullptr; a = a->next)
      if (a->position.seq != 0 && in_verified_file(a->position) && !is_vector_type_attribute(a)) {
        cx_ = Ctx();
        cx_.stmt_pos = a->position;
        an_attribute_ptr one_next = a->next;
        a->next = nullptr;  // check this entry alone
        check_attributes(a);
        a->next = one_next;
      }
    const a_source_position p = t->source_corresp.decl_position;
    if (p.seq == 0 || !in_verified_file(p)) continue;
    a_type_ptr u = t->kind == tk_typeref ? nullptr : t;
    if (u == nullptr) continue;
    bool is_class = u->kind == tk_class || u->kind == tk_struct || u->kind == tk_union;
    bool is_enum = u->kind == tk_integer && u->variant.integer.enum_type;
    if (!is_class && !is_enum) continue;
    if (is_class && u->variant.class_struct_union.extra_info != nullptr &&
        u->variant.class_struct_union.extra_info->is_lambda_closure_class)
      continue;  // the lambda expression is rejected
    reject(Diag::unsupported_declaration, p, is_enum ? "an enumeration" : "a class, struct or union");
  }
  // File-scope variables are classified by encode_named_constants (decision 0017).
  for (an_asm_entry_ptr a : ents.asm_entries) {
    const a_source_position p = a->source_corresp.decl_position;
    if (p.seq == 0 || !in_verified_file(p)) continue;
    // An asm statement inside a function is rejected as a statement (its scope is the function's).
    a_scope_ptr sp = a->source_corresp.parent_scope;
    if (sp != nullptr && sp->kind != sck_file && sp->kind != sck_namespace) continue;
    reject(Diag::unsupported_declaration, p, "a file-scope asm declaration");
  }
  for (a_template_ptr t : ents.templates) {
    const a_source_position p = t->source_corresp.decl_position;
    if (p.seq == 0 || !in_verified_file(p)) continue;
    if (t->canonical_template != nullptr && t->canonical_template != t) continue;
    reject(Diag::unsupported_declaration, p, "a template");
  }
}

void Encoder::run(const IlEntities& ents) {
  prog_.set_c_int_bits(static_cast<int>(targ_sizeof_int) * static_cast<int>(targ_char_bit));
  declaration_level_checks(ents);
  encode_named_constants(ents);

  // Routines in verified files: signature and attributes of every declaration, bodies verified.  A body in a
  // verified file that is not encoded (a lambda's, a compiler-generated one, one of a template, a specialization
  // or a class that this file does not itself declare) is REJECTED, never skipped: skipping would leave code that
  // runs unchecked (the M3 pre-commit audit: an explicit specialization of a system template, an out-of-line
  // member of a system class).
  for (a_routine_ptr r : ents.routines) {
    a_statement_ptr body = verified_body(r);
    const a_source_position decl = r->source_corresp.decl_position;
    const bool declared_here = decl.seq != 0 && in_verified_file(decl);
    if (body == nullptr && !declared_here) continue;
    cx_ = Ctx();
    cx_.routine = r;
    cx_.stmt_pos = decl;
    const a_source_position at = body != nullptr && body->position.seq != 0 ? body->position : decl;
    const char* skipped = nullptr;
    bool covered = false;  // the declaration that explains the routine is itself rejected
    if (r->is_lambda_body) {
      skipped = "the body of a lambda";
      covered = true;  // the lambda expression is rejected where it is written
    } else if (r->compiler_generated) {
      skipped = "a function the compiler generated";
    } else if (r->is_template_function || r->assoc_template != nullptr || r->is_prototype_instantiation) {
      skipped = "a function body for a template or a specialization";
      covered = template_declared_here(r);  // a template written in this file is rejected at the template
    } else if (r->source_corresp.is_class_member) {
      skipped = "a member function";
      covered = class_declared_here(r);  // a class written in this file is rejected at the class
    }
    if (skipped != nullptr) {
      if (body != nullptr && !covered)
        reject(Diag::unsupported_declaration, at, std::string(skipped) + " (this version verifies plain functions only)");
      continue;
    }
    {
      a_routine_type_supplement_ptr rx = routine_supplement(r);
      check_attributes(r->source_corresp.attributes, rx != nullptr && rx->does_not_return);
    }
    if (is_spec_routine(r)) continue;  // specification, never run: its body is not encoded (0011 item 4)
    if (r->is_weak || r->is_weakref) {
      // `#pragma weak f` sets the flag without an attribute entry: another unit's definition replaces this one at link time.
      reject(Diag::unsupported_declaration, decl, "a weak function (another unit may replace its definition)");
      continue;
    }
    if (r->is_declared_constexpr || r->is_constexpr || r->is_consteval) {
      reject(Diag::unsupported_declaration, decl, r->is_consteval ? "a consteval function" : "a constexpr function");
      continue;
    }
    if (body == nullptr && has_internal_linkage_or_inline(r)) {
      // Declared here and never defined: a function with internal linkage, or an inline one, cannot be defined in
      // another unit, so a contract on it would be trusted for a definition that cannot exist.
      reject(Diag::unsupported_declaration, decl,
             "a static, unnamed-namespace or inline function that is not defined in this file (no other unit can define it)");
      continue;
    }
    if (is_main(r)) {
      for (an_attribute_ptr a : contracts_of(r))
        if (classify(a) == Annot::requires_) reject(Diag::contract_on_main, a->position, "");
    }
    if (!check_signature(r, decl)) continue;
    if (a_routine_type_supplement_ptr nx = routine_supplement(r); nx != nullptr && nx->does_not_return) {
      // Also for a function that is declared and never called (function_of is lazy): decision 0015 item 4.
      for (an_attribute_ptr a : contracts_of(r))
        if (classify(a) == Annot::ensures) reject(Diag::ensures_on_noreturn, a->position, "");
    }
    if (body != nullptr) {
      if (function_of(r, true, decl) != nullptr) to_verify_.push_back(r);
    }
  }
  cx_ = Ctx();
  for (a_routine_ptr r : to_verify_) encode_body(r);
  require_measures_in_cycles();
}

// 0013 item 4: a function that can reach itself through calls of verified functions needs a measure.
void Encoder::require_measures_in_cycles() {
  auto reaches = [&](a_routine_ptr from, a_routine_ptr target) {
    std::set<a_routine_ptr> seen;
    std::vector<a_routine_ptr> todo(edges_[from].begin(), edges_[from].end());
    while (!todo.empty()) {
      a_routine_ptr n = todo.back();
      todo.pop_back();
      if (n == target) return true;
      if (!seen.insert(n).second) continue;
      for (a_routine_ptr m : edges_[n]) todo.push_back(m);
    }
    return false;
  };
  for (a_routine_ptr r : to_verify_) {
    vir::Function* f = fns_[r];
    if (f == nullptr || !f->decreases_.empty() || !reaches(r, r)) continue;
    std::string why = edges_[r].count(r) != 0 ? "it calls itself" : "it is part of a cycle of calls";
    if (edges_[r].count(r) == 0)
      for (a_routine_ptr m : edges_[r])
        if (m != r && reaches(m, r)) {
          why = "it calls \"" + rname(m) + "\", which calls it back";
          break;
        }
    cx_ = Ctx();
    cx_.routine = r;
    reject(Diag::missing_decreases, r->source_corresp.decl_position,
           "function \"" + rname(r) + "\" is recursive (" + why + ") and has no termination measure: write CERASTES_DECREASES(...) after its parameter list");
  }
}

// 0011 item 4: a `#line` directive or linemarker in a verified file gives the lines after it a presumed file
// name, which could name a system file.  Every one makes a source-file record with no `full_name` (il_def.h:150);
// the directive is in the file that record is a child of, one line before the record's first line (P14).
void reject_line_directives_pass() {
  for (a_seq_number_lookup_entry_ptr e = il_header.seq_number_lookup_entries; e; e = e->next) {
    a_source_file_ptr f = e->source_file;
    if (f == nullptr || f->full_name != nullptr) continue;
    a_source_file_ptr phys = nullptr;
    a_line_number line = 0;
    a_boolean at_end = FALSE;
    conv_seq_to_physical_file_and_line(e->first, &phys, &line, &at_end);
    // A directive in a file that cannot be placed is rejected too (fail closed, decision 0011 item 4).
    const std::string c = (phys != nullptr && phys->full_name != nullptr) ? canonical_path(phys->full_name) : std::string();
    const VerifyConfig& cfg = verify_config();
    if (!c.empty() && is_system_file(cfg.system_dirs, cfg.cerastes_header, c)) continue;
    if (phys == nullptr) {
      DiagPos unknown;
      report_at(Diag::line_directive_in_verify, unknown);
      continue;
    }
    DiagPos at;
    at.known = true;
    at.in_primary = (phys == il_header.primary_source_file);
    at.seq = e->first;
    at.file = phys->file_name ? phys->file_name : "";
    at.full_name = phys->full_name ? phys->full_name : "";
    // `#pragma GCC system_header` makes a record of the same file name that is a system header; a `#line` or a
    // linemarker gives the lines another presumed name.  The record of a `#line` starts on the line after the
    // directive; the pragma's starts on the pragma's own line (measured: a pragma on line 2 of a header gives line 2).
    const bool pragma = f->from_system_include_dir && f->file_name != nullptr && phys->file_name != nullptr &&
                        std::strcmp(f->file_name, phys->file_name) == 0;
    at.line = (!pragma && line > 1) ? line - 1 : line;
    report_at(pragma ? Diag::system_header_pragma_in_verify : Diag::line_directive_in_verify, at);
  }
}

}  // namespace

void verify_encode() {
  VerifyConfig& cfg = verify_config();
  if (cfg.hook == "abort") std::abort();  // self-test hook: a crash in the back end is exit status 3
  if (cfg.hook == "internal_error") internal_error("cerastes self-test hook: an internal error raised from the back end");

  reject_line_directives_pass();
  // EDG's own diagnostic pragmas could turn off the front-end errors that the verifier relies on (decision 0017 item 2).
  {
    std::set<unsigned long> seen;
    for (const a_source_position& p : diag_pragma_sites()) {
      if (p.seq == 0) {  // a pragma that cannot be placed is rejected too (fail closed, decision 0011 item 4)
        DiagPos unknown;
        report_at(Diag::diagnostic_pragma_in_verify, unknown);
        continue;
      }
      if (!in_verified_file(p) || !seen.insert(static_cast<unsigned long>(p.seq)).second) continue;
      report_at(Diag::diagnostic_pragma_in_verify, diag_position(p));
    }
  }

  Encoder enc;
  enc.run(collect_entities());
  if (reported_count() != 0) return;

  vir::Program& program = enc.program();
  std::vector<vir::Diag> problems = vir::check_program(program);
  if (!problems.empty()) {
    for (const vir::Diag& d : problems) {
      DiagPos at;
      at.known = d.loc.known();
      at.in_primary = false;
      at.line = static_cast<unsigned long>(d.loc.line);
      at.col = static_cast<unsigned long>(d.loc.column);
      at.file = d.loc.file;
      at.full_name = d.loc.file;
      at.seq = at.line;
      report_at(Diag::unsupported_expression, at, {d.reason});
    }
    return;
  }
  vir::LowerOptions lo;
  lo.weaken_auto_invariant = cfg.hook == "weaken_invariant";
  if (cfg.hook == "drop_assert") lo.drop_assert = 0;
  if (cfg.hook.compare(0, 12, "drop_assert=") == 0) lo.drop_assert = std::atoi(cfg.hook.c_str() + 12);
  vir::LowerResult lowered = vir::lower_program(program, lo);
  if (!lowered.errors.empty()) {
    for (const vir::Diag& d : lowered.errors) {
      DiagPos at;
      at.known = d.loc.known();
      at.line = static_cast<unsigned long>(d.loc.line);
      at.col = static_cast<unsigned long>(d.loc.column);
      at.file = d.loc.file;
      at.full_name = d.loc.file;
      at.seq = at.line;
      report_at(Diag::unsupported_expression, at, {"an internal inconsistency: " + d.reason});
    }
    return;
  }
  viper::Emitted emitted = viper::emit(lowered.program);
  if (!emitted.problems.empty()) {
    for (const std::string& why : emitted.problems) {
      DiagPos at;
      report_at(Diag::unsupported_expression, at, {"an internal inconsistency: " + why});
    }
    return;
  }
  cfg.out.vpr = emitted.vpr;
  cfg.out.map = emitted.map;
  cfg.out.functions = enc.verified_functions();
  cfg.out.encoded = true;
}

}  // namespace cerastes
