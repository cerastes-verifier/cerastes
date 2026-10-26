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

#include <cerrno>
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
  bool silent = false;               // !ok and already reported at the declaration (an enumeration, decision 0018 item 2)
  a_type_ptr computed_at = nullptr;  // !ok because this typeref has a non-type template argument (0024 item 6): reported once per function
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
    case eok_bassign: return "an assignment of a class or reference type";
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
    // The IL nodes with a side effect (assignment, ++/--, call) encoded in this function (decision 0020 item 10).
    std::set<an_expr_node_ptr> encoded_effects;
    // The IL position of each AssignE built (for the report of vir::sequence_problems, decision 0020 item 4).
    std::map<const Expr*, a_source_position> assign_positions;
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
  std::set<std::pair<a_routine_ptr, a_type_ptr>> computed_reported_;  // typerefs reported by reject_type, per function (0024 item 6)
  Ctx cx_;
  // Named constants (decision 0017): the file-scope `const` objects of verified files that are encoded, by variable.
  struct NamedInfo {
    vir::Type type;
    std::string value;  // EDG's value, decimal ("0"/"1" for bool)
  };
  std::map<a_variable_ptr, NamedInfo> named_;
  std::set<a_variable_ptr> rejected_globals_;  // file-scope variables rejected at their declaration: uses are not diagnosed again
  a_variable_ptr encoding_named_ = nullptr;     // the named constant whose initializer is being encoded

  // Enumerations (decision 0018): one VIR type per enumeration type entry, and what was decided about it.
  struct EnumInfo {
    bool ok = false;
    bool silent = false;  // !ok and already reported at the declaration: the uses are not diagnosed again
    vir::Type type = vir::Type::void_type();
    Diag diag = Diag::unsupported_type;  // the diagnostic when !ok and !silent (a system enumeration, at its use)
    std::string name;                    // the type as a message names it when !ok
  };
  std::map<a_type_ptr, EnumInfo> enums_;
  // The enumerators with a written initializer of the enumerations that passed their checks (decision 0018 item 10): each is
  // a named constant of the unit, checked in the prologue.
  std::vector<a_constant_ptr> pending_enumerators_;
  // The implicit enumeration-to-integer conversion nodes a parent allows (decision 0018 item 6 (i)-(iii)), by node: the
  // parent marks exactly the child it allows, and no node inherits a mark.
  std::set<an_expr_node_ptr> enum_conv_ok_;
  void allow_enum_conversion(an_expr_node_ptr n) {
    if (n != nullptr) enum_conv_ok_.insert(n);
  }

  static bool is_enum_type(const vir::Type& t) { return t.is_int() && t.int_type != nullptr && t.int_type->is_enum; }

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

  // As reject(), for a diagnostic whose message takes several arguments.
  void reject_args(Diag d, const a_source_position& p, const std::vector<std::string>& args) {
    if (cx_.suppress) return;
    DiagPos dp = position_for(p);
    if (!rejected_.insert({static_cast<int>(d), dp.file, dp.line}).second) return;
    report_at(d, dp, args);
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
    if (t.silent) return;
    // A type computed from a constant (decision 0024 item 6) is reported where it is first met in a function, not again at each
    // use of a variable declared with it.
    if (t.computed_at != nullptr && !computed_reported_.insert({cx_.routine, t.computed_at}).second) return;
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
    {
      // Decision 0024 item 6: an alias template instance or a member typedef of a class instance with a non-type argument
      // (`std::conditional_t<(65536u * 65536u == 0u), int, long>`) is a type the front end computed alone.
      std::set<a_type_ptr> seen;
      if (const char* why = typeref_chain_reason(type, seen, 0, &r.computed_at)) {
        r.name += std::string(" (") + why + "; decision 0024 item 6)";
        r.diag = Diag::unsupported_type;
        return r;
      }
    }
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
        if (i.enum_type) {
          const EnumInfo& ei = enum_info(t);
          r.ok = ei.ok;
          r.silent = ei.silent;
          r.diag = ei.diag;
          if (ei.ok) r.t = ei.type;
          else if (!ei.name.empty()) r.name = ei.name;
          return r;
        }
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
      detect_or_as_truth_value(arg);  // decision 0023 item 5: `requires(x | 4)`, a clause of `ensures`
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

  // ---- types computed from constants (decision 0024 item 6) -------------------------------------------------------------
  // The front end computes an array bound, a `_BitInt` width or a non-type template argument alone and wraps the result; a
  // type that is, or contains, one of these is not read until arrays are (M4.5e).  Null: the type is fine.
  static const char* computed_type_reason(a_type_ptr t, std::set<a_type_ptr>& seen, int depth = 0);

  // A recursion guard that trips rejects (fail closed): the type is not examined further.
  static const char* too_deep_reason() { return "a type nested too deeply for this version"; }

  static const char* template_args_reason(a_template_arg_ptr list, std::set<a_type_ptr>& seen, int depth) {
    for (a_template_arg_ptr a = list; a != nullptr; a = a->next) {
      if (a->kind == tak_nontype) return "a template instance with a non-type template argument";
      if (a->kind == tak_type)
        if (const char* r = computed_type_reason(a->variant.type, seen, depth + 1)) return r;
    }
    return nullptr;
  }

  // A class instance: its own arguments, not its members (classes of verified files are rejected anyway, 0012).
  static const char* class_instance_reason(a_type_ptr cls, std::set<a_type_ptr>& seen, int depth) {
    if (cls == nullptr) return nullptr;
    cls = skip_typerefs(cls);
    if (cls->kind != tk_class && cls->kind != tk_struct && cls->kind != tk_union) return nullptr;
    a_class_type_supplement_ptr x = cls->variant.class_struct_union.extra_info;
    return x != nullptr ? template_args_reason(x->template_arg_list, seen, depth) : nullptr;
  }

  // The class instance of an entity that is a member of one (a static data member, a member function, a member typedef).
  static const char* member_class_reason(const a_source_correspondence& sc, std::set<a_type_ptr>& seen, int depth) {
    if (!sc.is_class_member || sc.parent_scope == nullptr || sc.parent_scope->kind != sck_class_struct_union) return nullptr;
    return class_instance_reason(sc.parent_scope->variant.assoc_type, seen, depth);
  }

  // The typeref chain of a type: an alias template instance with a non-type argument, or a member typedef of a class instance
  // with one (`std::conditional_t<(65536u * 65536u == 0u), int, long>`).  Also what `vtype` asks of every value type.
  // `where`, when given, receives the typeref of the chain that gave the reason.
  static const char* typeref_chain_reason(a_type_ptr t, std::set<a_type_ptr>& seen, int depth, a_type_ptr* where = nullptr) {
    int guard = 0;
    for (; t != nullptr && t->kind == tk_typeref && guard < 64; ++guard) {
      if (a_typeref_type_supplement_ptr x = t->variant.typeref.extra_info)
        if (const char* r = template_args_reason(x->template_arg_list, seen, depth + 1)) {
          if (where != nullptr) *where = t;
          return r;
        }
      if (const char* r = member_class_reason(t->source_corresp, seen, depth + 1)) {
        if (where != nullptr) *where = t;
        return r;
      }
      t = t->variant.typeref.type;
    }
    if (t != nullptr && t->kind == tk_typeref) {  // the guard tripped: fail closed
      if (where != nullptr) *where = t;
      return too_deep_reason();
    }
    return nullptr;
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
    if (t.is_int() && t.int_type->is_enum && !vir::dec::in_range(value, vir::dec::range_of(*t.int_type))) {
      // A named constant of an enumeration whose front-end value is not one of its values (`const enum color DEF = (enum
      // color)4;`): its initializer's conversion fails in every method's prologue first (decision 0018 item 5).  The use is
      // that conversion again, of a literal of the underlying type, so no Const lies outside its type's values.
      const vir::Type u = vir::Type::integer(prog_.int_type_of(t.int_type->bits, t.int_type->is_signed));
      return b_.convert(t, b_.int_const(u, value, loc_of(p)), false, loc_of(p));
    }
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

  const Expr* constant(an_expr_node_ptr e) {
    a_constant_ptr c = e->variant.constant.ptr;
    // A folded constant is the operand as written (`c == DEF` with a `const enum color DEF`: the implicit conversion is inside
    // its `expr` chain), so a parent's allowance of an implicit conversion out of an enumeration (decision 0018 item 6) passes
    // to its original expression.
    if (c != nullptr && enum_conv_ok_.count(e) != 0 && !is_enumerator(c) && (c->expr != nullptr || c->local_expr_ref))
      allow_enum_conversion(expr_node_from_constant(c));
    const Expr* x = constant_value(c, e->position);
    // The front end folds the conversion of an enumerator into the node: an `enk_constant` of type `int` naming the enumerator
    // constant, whose own type is the enumeration (C++, the unnamed enum of `B = A + 1`).  That is decision 0018 item 6 (iv):
    // the literal of the enumeration converted implicitly to the node's type.
    if (x != nullptr && is_enumerator(c) && e->type != nullptr) {
      VType nt = vtype(e->type);
      if (nt.ok && !nt.t.is_bool() && nt.t != x->type) {
        if (il_header.source_language == sl_C) {
          // In C an enumerator referenced in the enumerator list is `int` when its value fits (C23 6.7.3.3), but the front end
          // types the reference the C++ way, by the initializer's type (`enum { U = 2u, V = U > -1 };` has `U` unsigned): the
          // conversion would give a value C does not.
          reject(Diag::unsupported_expression, e->position,
                 "in C, an enumerator used in the enumerator list with a type other than its own (the front end types it the "
                 "C++ way; decision 0018 item 10)");
          return nullptr;
        }
        x = b_.convert(nt.t, x, false, loc_of(e->position));
      }
    }
    return x;
  }

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
    // An enumerator is its value (decision 0018 item 9), whatever the initializer that gave it: recognized before the folded
    // constant's original expression is read, so the initializer is encoded once (in the prologue), not at each use.  Any other
    // constant of enumeration type (a folded `(enum color)1`) goes the normal way, and its cast is a checked conversion.
    if (is_enumerator(c)) {
      if (c->source_corresp.decl_position.seq == 0 || !in_verified_file(c->source_corresp.decl_position)) {
        reject(Diag::unsupported_expression, pos, "an enumerator of an enumeration declared in a system file (system files are used only through contracts, decision 0018)");
        return nullptr;
      }
      // The enumeration of the enumerator (defence in depth): its type in C++ and for a fixed C enumeration, else the type
      // it is affiliated with (il_def.h:10159-10166, valid when enum_type is FALSE: C's `int` of an unfixed enumeration).  It must
      // have passed its checks (decision 0018 items 2-3, 10); none found is a rejection.
      a_type_ptr ct = c->type != nullptr ? skip_typerefs(c->type) : nullptr;
      a_type_ptr et_enum = nullptr;
      if (ct != nullptr && ct->kind == tk_integer) {
        et_enum = ct->variant.integer.enum_type ? ct : ct->variant.integer.enum_info.affiliated_type;
        if (et_enum != nullptr) et_enum = skip_typerefs(et_enum);
      }
      if (et_enum == nullptr || et_enum->kind != tk_integer || !et_enum->variant.integer.enum_type) {
        reject(Diag::unsupported_expression, pos, "an enumerator whose enumeration this version cannot find (decision 0018 item 9)");
        return nullptr;
      }
      const EnumInfo& ei = enum_info(et_enum);
      if (!ei.ok) {
        if (!ei.silent) reject(Diag::unsupported_type, pos, ei.name);
        return nullptr;
      }
      VType et;
      if (!value_type(c->type, pos, &et)) return nullptr;
      return literal(c, et, pos);
    }
    VType t;
    if (!value_type(c->type, pos, &t)) return nullptr;
    an_expr_node_ptr original = (c->expr != nullptr || c->local_expr_ref) ? expr_node_from_constant(c) : nullptr;
    const bool hook = verify_config().hook == "constant_without_expr";
    if (original != nullptr && !hook) {
      {
        // Decision 0020 item 5: a side effect in an unevaluated operand does not happen (`sizeof(x++)`).
        const char* what = "";
        if (an_expr_node_ptr fx = unevaluated_side_effect(original, &what)) {
          reject(Diag::side_effect_not_evaluated, fx->position, what);
          return nullptr;
        }
      }
      // sizeof and alignof give their value: no run-time operation (0010 item 12).
      if (original->kind == enk_sizeof || original->kind == enk_alignof || original->kind == enk_datasizeof) {
        if (reject_computed_sizeof_operand(original, pos)) return nullptr;  // decision 0024 item 6
        return literal(c, t, pos);
      }
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
    {
      // Decision 0024 item 5: a character literal above the range of its type (`'\xff'`) has no `expr`, no `orig_type` and a
      // negative value the front end sign-extended; only an `int` (C) or a plain `char`/`wchar_t` (C++) constant of this shape is
      // one (any other, an enumeration-typed copy of an enumerator, takes the literal path as before).
      const bool c_mode = il_header.source_language == sl_C;
      const bool char_shaped = !t.t.is_bool() && !is_enum_type(t.t) && (c_mode ? t.name == "int" : (t.name == "char" || t.name == "wchar_t"));
      if (char_shaped && c->expr == nullptr && !c->local_expr_ref && c->orig_type == nullptr) {
        a_number_buffer buf = decimal_str_for_integer_constant(c);
        const std::string text = buf.as_temp_characters();
        if (vir::dec::negative(text)) return wrapped_character_literal(text, c->character_kind == chk_wchar_t, pos, c_mode);
      }
    }
    return literal(c, t, pos);
  }

  // Decision 0024 item 5: the conversion of the literal's code `v + 2^N` (an unsigned N-bit constant) to its character type
  // (and, in C, to `int`).  It fails for every such code, so the literal is a `conversion` error at its position on the paths
  // that reach it.  N is 32 for a wide literal (or a value below -128), else 8.
  const Expr* wrapped_character_literal(const std::string& text, bool wchar_kind, const a_source_position& pos, bool c_mode) {
    const bool wide = wchar_kind || vir::dec::cmp(text, "-128") < 0;
    const int n = wide ? 32 : 8;
    errno = 0;
    char* end = nullptr;
    const long long v = std::strtoll(text.c_str(), &end, 10);
    if (errno != 0 || end == nullptr || *end != '\0' || v >= 0 || v < -(1LL << (n - 1))) {
      reject(Diag::unsupported_expression, pos, "a character literal whose value this version cannot read (decision 0024 item 5)");
      return nullptr;
    }
    const SrcLoc l = loc_of(pos);
    const vir::Type code_type = vir::Type::integer(prog_.int_type(wide ? "unsigned int" : "unsigned char", n, false));
    const vir::Type char_type = vir::Type::integer(prog_.int_type(wide ? "wchar_t" : "char", n, true));
    const Expr* code = b_.spelled(b_.int_const(code_type, std::to_string(v + (1LL << n)), l), wide ? "unsigned int" : "unsigned char");
    const Expr* x = b_.spelled(b_.convert(char_type, code, false, l), wide ? "wchar_t" : "char");
    if (c_mode) x = b_.spelled(b_.convert(vir::Type::integer(prog_.int_type_of(prog_.c_int_bits(), true)), x, false, l), "int");
    return x;
  }

  // Decision 0024 item 6: the type of a `sizeof`, `_Alignof` or `__datasizeof` operand (the type operand, or the type of the
  // expression operand) is, or contains, an array, a `_BitInt`, a vector or a class instance with a non-type argument.
  bool reject_computed_sizeof_operand(an_expr_node_ptr node, const a_source_position& at) {
    const auto& si = node->variant.sizeof_info;
    a_type_ptr t = si.is_type ? si.variant.type : (si.variant.expr != nullptr ? si.variant.expr->type : nullptr);
    std::set<a_type_ptr> seen;
    const char* why = computed_type_reason(t, seen);
    if (why == nullptr) return false;
    reject(Diag::unsupported_type, at,
           render_type(t) + " (" + why + " in sizeof, _Alignof or __datasizeof: the front end computes its size alone; rejected until arrays "
                                          "are encoded, M4.5e; decision 0024 item 6)");
    return true;
  }

  // ---- static assertions (decision 0024 items 2 and 3) ---------------------------------------------------------------------------
  // In a verified file when its position, or its condition's original expression's position, is; with no position at all
  // it counts as in one (fail closed, like in_verified_file).
  static bool static_assertion_in_verified_file(a_static_assertion_ptr sa) {
    bool known = false;
    auto check = [&](const a_source_position& p) {
      if (p.seq == 0) return false;
      known = true;
      return in_verified_file(p);
    };
    if (check(sa->position)) return true;
    a_constant_ptr c = sa->condition;
    if (c != nullptr && (c->expr != nullptr || c->local_expr_ref)) {
      an_expr_node_ptr o = expr_node_from_constant(c);
      if (o != nullptr && check(o->position)) return true;
    }
    return !known;
  }

  // An entity declared in a system file, or with no position (a builtin): its value is the front end's alone.
  static bool declared_in_system(const a_source_position& p) { return p.seq == 0 || !in_verified_file(p); }

  struct SkipScan {
    bool skip = false;      // the assertion needs the front end alone: not encoded, silently (item 3)
    bool rejected = false;  // a type computed from a constant was met (item 6), and diagnosed
  };

  void scan_type(a_type_ptr t, const a_source_position& at, SkipScan& out) {
    std::set<a_type_ptr> seen;
    if (const char* why = computed_type_reason(t, seen)) {
      reject(Diag::unsupported_type, at, render_type(t) + " (" + why + "; decision 0024 item 6)");
      out.rejected = true;
    }
  }
  // `what` names the entity whose template arguments are scanned (a variable template, a routine, a concept).
  void scan_template_args(a_template_arg_ptr list, const std::string& what, const a_source_position& at, SkipScan& out) {
    std::set<a_type_ptr> seen;
    if (const char* why = template_args_reason(list, seen, 0)) {
      reject(Diag::unsupported_type, at, what + " (" + why + "; the front end computes its value alone; decision 0024 item 6)");
      out.rejected = true;
    }
  }
  // The class instance an entity is a member of, named as the type (`std::integral_constant<unsigned int, 0U>`).
  void scan_member_class(const a_source_correspondence& sc, const a_source_position& at, SkipScan& out) {
    std::set<a_type_ptr> seen;
    if (const char* why = member_class_reason(sc, seen, 0)) {
      reject(Diag::unsupported_type, at,
             render_type(sc.parent_scope->variant.assoc_type) + " (" + why + "; the front end computes its members alone; decision 0024 item 6)");
      out.rejected = true;
    }
  }

  // The pre-scan of a static assertion's condition, following folded constants' original expressions.  The operands of `sizeof`,
  // `_Alignof`, `__datasizeof` and `noexcept` are not entered (not evaluated: the encoder's sizeof arm and the type rule decide).
  // A trigger of the skip is a call of a routine declared in a system file or a builtin, a builtin operation, a concept-id, and a
  // variable or enumerator declared in a system file; a user's routine, variable or enumerator is not a trigger (the encoder
  // handles it as it always did).  Types computed from constants are rejected wherever the scan meets one, skipped or not.
  void scan_for_skip(an_expr_node_ptr e, SkipScan& out, int depth = 0) {
    if (e == nullptr || depth > 256) return;
    switch (e->kind) {
      case enk_constant: {
        a_constant_ptr c = e->variant.constant.ptr;
        if (c == nullptr) return;
        if (is_enumerator(c)) {
          if (declared_in_system(c->source_corresp.decl_position)) {
            out.skip = true;
            scan_member_class(c->source_corresp, e->position, out);
          }
          return;
        }
        if (c->expr == nullptr && !c->local_expr_ref) return;
        an_expr_node_ptr o = expr_node_from_constant(c);
        if (o != nullptr && o != e) scan_for_skip(o, out, depth + 1);
        return;
      }
      case enk_variable: {
        a_variable_ptr v = e->variant.variable.ptr;
        if (v == nullptr || !declared_in_system(v->source_corresp.decl_position)) return;
        out.skip = true;
        if (v->template_info != nullptr)
          scan_template_args(v->template_info->template_arg_list, render_name(&v->source_corresp, iek_variable), e->position, out);
        scan_member_class(v->source_corresp, e->position, out);
        return;
      }
      case enk_sizeof:
      case enk_alignof:
      case enk_datasizeof: {
        const auto& si = e->variant.sizeof_info;
        scan_type(si.is_type ? si.variant.type : (si.variant.expr != nullptr ? si.variant.expr->type : nullptr), e->position, out);
        return;
      }
      case enk_type_operand: scan_type(e->variant.type_operand.type, e->position, out); return;
      case enk_builtin_operation:
        out.skip = true;
        for (an_expr_node_ptr o = e->variant.builtin_operation.operands; o != nullptr; o = o->next) scan_for_skip(o, out, depth + 1);
        return;
      case enk_concept_id:
        out.skip = true;
        scan_template_args(e->variant.concept_id.args, "a concept-id", e->position, out);
        return;
      case enk_operation: {
        const an_expr_operator_kind k = e->variant.operation.kind;
        if (k == eok_noexcept) return;
        if (is_call_kind(k)) {
          if (a_routine_ptr r = direct_callee(e)) {
            if (declared_in_system(r->source_corresp.decl_position)) {
              out.skip = true;
              scan_template_args(r->template_arg_list, rname(r), e->position, out);
              scan_member_class(r->source_corresp, e->position, out);
            }
          }
        }
        for (an_expr_node_ptr o = e->variant.operation.operands; o != nullptr; o = o->next) scan_for_skip(o, out, depth + 1);
        return;
      }
      default: return;
    }
  }

  enum class SaResult { encoded, skipped, failed };

  // The condition of a static assertion as a closed Bool expression, as written (decision 0024 item 2); `cx_` is the context of
  // the prologue or of the function.  `skipped`: nothing to check or the front end's alone (item 3); `failed`: diagnosed.
  SaResult static_assertion_condition(a_static_assertion_ptr sa, const Expr** out) {
    a_constant_ptr c = sa->condition;
    if (c == nullptr) {
      reject(Diag::unsupported_declaration, sa->position, "a static assertion without a condition (decision 0024 item 2)");
      return SaResult::failed;
    }
    if (c->kind == ck_template_param) return SaResult::skipped;  // a dependent assertion of a template: the template is rejected
    if (c->kind != ck_integer) {
      // `static_assert(1.5)`: not an integer or a bool; rejected with its type (decision 0024 item 2).
      reject(Diag::unsupported_type, sa->position, c->type != nullptr ? render_type(c->type) : std::string("<unknown>"));
      return SaResult::failed;
    }
    if (c->expr == nullptr && !c->local_expr_ref && c->kind == ck_integer) {
      // A lone literal or enumerator has nothing to check, except a character literal that does not fit its type (item 5).
      a_number_buffer buf = decimal_str_for_integer_constant(c);
      if (is_enumerator(c) || !vir::dec::negative(std::string(buf.as_temp_characters()))) return SaResult::skipped;
    }
    {
      an_expr_node node{};
      node.kind = enk_constant;
      node.type = c->type;
      node.variant.constant.ptr = c;
      node.position = sa->position;
      SkipScan scan;
      scan_for_skip(&node, scan);
      if (scan.rejected) return SaResult::failed;
      if (scan.skip) return SaResult::skipped;
    }
    cx_.case_value = true;  // the case-value path: a named local constant is its value (0017 item 6), a variable is noted
    cx_.case_var.clear();
    ++cx_.const_depth;      // a constant expression: no calls (`call` rejects them)
    const Expr* x = constant_initializer(c, sa->position);
    --cx_.const_depth;
    cx_.case_value = false;
    const std::string named_var = cx_.case_var;
    if (x == nullptr) return SaResult::failed;
    if (il_header.source_language == sl_C && !named_var.empty()) {
      // Defence in depth for #1901 (0017 item 2): a `const` object is not a constant expression in C.
      reject(Diag::unsupported_expression, sa->position,
             "in C, a static assertion whose condition names a variable (`" + named_var + "`; it is not a constant expression in C)");
      return SaResult::failed;
    }
    if (const char* name = variable_read_by(x)) {
      reject(Diag::unsupported_expression, sa->position, std::string("a static assertion whose condition names a variable (`") + name + "`)");
      return SaResult::failed;
    }
    *out = clause_bool(x, sa->position);
    return *out != nullptr ? SaResult::encoded : SaResult::failed;
  }

  // A file-scope or namespace-scope assertion: a prologue item of every method, in source order with the named constants.
  void encode_file_static_assertion(a_static_assertion_ptr sa) {
    cx_ = Ctx();
    cx_.stmt_pos = sa->position;
    const Expr* cond = nullptr;
    if (static_assertion_condition(sa, &cond) != SaResult::encoded) return;
    vir::NamedConst* n = prog_.add_static_assertion(loc_of(sa->position), cond);
    if (verify_config().hook == "perturb_folded") n->value = "0";
  }

  // A block-scope assertion: a local constant of the function (its prologue and its `__wf`), before the case values.
  void encode_block_static_assertion(a_static_assertion_ptr sa) {
    const a_source_position saved = cx_.stmt_pos;
    if (sa->position.seq != 0) cx_.stmt_pos = sa->position;
    const Expr* cond = nullptr;
    if (static_assertion_condition(sa, &cond) == SaResult::encoded) {
      vir::NamedConst* n = cx_.fn->add_local_static_assertion(loc_of(sa->position), cond);
      if (verify_config().hook == "perturb_folded") n->value = "0";
    }
    cx_.stmt_pos = saved;
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
    // The variables and the enumerators with a written initializer (decision 0018 item 10), in source order: the prologue
    // checks them in this order (decision 0017 item 4).
    struct Item {
      unsigned long seq;
      a_variable_ptr v;
      a_constant_ptr c;
      a_static_assertion_ptr sa = nullptr;
    };
    std::vector<Item> items;
    for (a_variable_ptr v : vars) items.push_back({static_cast<unsigned long>(v->source_corresp.decl_position.seq), v, nullptr, nullptr});
    for (a_constant_ptr c : pending_enumerators_) items.push_back({static_cast<unsigned long>(c->source_corresp.decl_position.seq), nullptr, c, nullptr});
    // Static assertions of the file scope's source sequence list (decision 0024 item 2), those of verified files.
    for (a_static_assertion_ptr sa : ents.static_assertions)
      if (static_assertion_in_verified_file(sa)) items.push_back({static_cast<unsigned long>(sa->position.seq), nullptr, nullptr, sa});
    // The fail-closed cross-check: a static assertion the walk reached that is not on the list is not checked.
    for (a_static_assertion_ptr sa : ents.other_static_assertions)
      if (static_assertion_in_verified_file(sa))
        reject(Diag::unsupported_declaration, sa->position, "a static assertion this version does not check (decision 0024 item 2)");
    std::stable_sort(items.begin(), items.end(), [](const Item& a, const Item& b) { return a.seq < b.seq; });
    for (const Item& it : items) {
      if (it.v != nullptr) encode_named_variable(it.v, c_mode);
      else if (it.sa != nullptr) encode_file_static_assertion(it.sa);
      else encode_named_enumerator(it.c);
    }
    cx_ = Ctx();
  }

  // One file-scope variable of a verified file: a named constant or a rejection at its declaration (decision 0017).
  void encode_named_variable(a_variable_ptr v, bool c_mode) {
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
        return;
      }
    }
    // The type first, so that the message names the real reason (an array of const is not "not const").
    VType vt = vtype(v->type);
    if (!vt.ok || vt.t.is_void()) {
      if (vt.ok) vt.name = "void";
      reject_type(vt, p);
      rejected_globals_.insert(v);
      return;
    }
    if (!is_const_object(v)) {
      refuse("a global variable that is not const (mutable globals come with M4.5b)");
      return;
    }
    if (v->source_corresp.is_class_member) {
      refuse("a static data member");
      return;
    }
    if (v->is_thread_local) {
      refuse("a thread_local constant");
      return;
    }
    if (v->is_weak || v->is_weakref) {
      // `__attribute__((weak))`, `[[gnu::weak]]` and `#pragma weak` all set the flag: another unit may replace the value at link time.
      refuse("a weak constant (another unit may replace its value)");
      return;
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
          return;
        }
      }
      encoding_named_ = v;
      ++cx_.const_depth;  // the initializer is a constant expression: no calls (`call` rejects them)
      init = constant_initializer(v->initializer.constant, p);
      --cx_.const_depth;
      encoding_named_ = nullptr;
      if (init == nullptr) {
        rejected_globals_.insert(v);
        return;
      }
      if (init->type != vt.t) {
        refuse("an initializer whose type is not the constant's type");
        return;
      }
      value = constant_text(v->initializer.constant, vt.t);
    } else if (v->init_kind == initk_none && v->storage_class == sc_extern) {
      refuse("an extern const that is not defined in this file (its value is not known here)");
      return;
    } else if ((v->init_kind == initk_none || v->init_kind == initk_zero) && c_mode && v->storage_class == sc_static) {
      // An internal-linkage tentative definition (C23 6.9.2): zero-initialized, no initializer to check, and no other unit
      // can define it.
    } else if (v->init_kind == initk_none && c_mode && v->storage_class == sc_unspecified) {
      // External linkage: a common symbol under clang's default -fcommon, so another unit's definition wins at link time.
      refuse("a const tentative definition with external linkage (another unit may define it; write `= 0` or make it static)");
      return;
    } else if (v->init_kind == initk_dynamic) {
      refuse("a const global whose initializer is not a constant expression (dynamic initialization)");
      return;
    } else {
      refuse("a const whose initialization this version cannot read");
      return;
    }
    named_[v] = NamedInfo{vt.t, value};
    // No initializer: nothing is checked, so perturb_folded leaves the value 0 (check_program rejects a perturbed one).
    prog_.add_named_const(name, vt.t, loc_of(p), init, init != nullptr ? maybe_perturb(vt.t, value) : value);
  }

  // An enumerator with a written initializer (decision 0018 item 10): a named constant of the unit, its initializer encoded by
  // its original expression, its obligations checked in the prologue at the line of the enumerator, then an `internal` guard
  // that the value as written equals the front end's value.  An initializer that is a plain literal records no original
  // expression and has nothing to check.
  void encode_named_enumerator(a_constant_ptr c) {
    const a_source_position p = c->source_corresp.decl_position;
    const char* nm = c->source_corresp.name;
    cx_ = Ctx();
    cx_.stmt_pos = p;
    if (c->expr == nullptr && !c->local_expr_ref) {
      // Decision 0024 item 5: a written initializer that is a character literal above its type's range (`E = '\xff'`) has a
      // negative value and no expression; the front end's value is not trusted, the literal is checked here.  (An enumerator
      // without an initializer is not told apart from it here, but `has_written_initializer` does.)
      if (c->kind == ck_integer && has_written_initializer(c)) {
        a_number_buffer buf = decimal_str_for_integer_constant(c);
        const std::string text = buf.as_temp_characters();
        if (vir::dec::negative(text)) {
          const Expr* init = wrapped_character_literal(text, c->character_kind == chk_wchar_t, p, il_header.source_language == sl_C);
          if (init != nullptr) prog_.add_named_const(nm ? nm : "?", init->type, loc_of(p), init, maybe_perturb(init->type, text));
        }
      }
      return;
    }
    an_expr_node_ptr original = expr_node_from_constant(c);
    if (original == nullptr) {
      reject(Diag::unsupported_declaration, p, "an enumerator whose initializer is not recorded");
      return;
    }
    // sizeof and alignof give their value, no run-time operation (0010 item 12): nothing to check, as for a literal.
    if (il_header.source_language == sl_C && typeof_or_sizeof_of_expression(original)) {
      reject(Diag::unsupported_expression, p,
             "in C, sizeof, _Alignof or typeof of an expression in an enumerator's value (the front end types an earlier "
             "enumerator the C++ way; decision 0018 item 10)");
      return;
    }
    if (original->kind == enk_sizeof || original->kind == enk_alignof || original->kind == enk_datasizeof) {
      reject_computed_sizeof_operand(original, p);  // decision 0024 item 6 (after item 10's C check above)
      return;
    }
    if (il_header.source_language == sl_C && names_object(original)) {
      // Enforced here too (defence in depth: the driver's `--diag_error=1901` refuses it first, decision 0017 item 2).
      reject(Diag::unsupported_expression, p,
             "in C, an enumerator's initializer that names an object (it is not a constant expression in C)");
      return;
    }
    ++cx_.const_depth;  // a constant expression: no calls (`call` rejects them)
    const Expr* init = expr(original);
    --cx_.const_depth;
    if (init == nullptr) return;
    if (!init->type.is_int() && !init->type.is_bool()) {
      reject(Diag::unsupported_expression, p, "an enumerator initializer that is not an integer");
      return;
    }
    const std::string value = constant_text(c, init->type);
    prog_.add_named_const(nm ? nm : "?", init->type, loc_of(p), init, maybe_perturb(init->type, value));
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
    if (is_effect_operation(e) && !cx_.encoded_effects.insert(e).second) {
      // Decision 0020 item 10: an IL node with a side effect is encoded once per function; a second encoding would be a second
      // copy of the effect in the VIR.  Fail closed.
      verify_config().out.internal_inconsistency = true;  // a tool failure, exit status 3 (decision 0022 item 12)
      reject(Diag::internal_inconsistency, e->position,
             "an expression with a side effect would be encoded twice (decision 0020 item 10)");
      return nullptr;
    }
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
    if (is_enum_type(a->type) || is_enum_type(c->type)) {
      reject(Diag::unsupported_expression, e->position, "arithmetic on an enumeration value (decision 0018 item 6)");
      return nullptr;
    }
    return finish(e, b_.binary(op, a, c, loc_of(e->position)));
  }

  const Expr* comparison(an_expr_node_ptr e, vir::BinaryOp op) {
    auto ops = operands_of(e);
    if (!operand_count(e, 2, ops)) return nullptr;
    if (op == vir::BinaryOp::Eq || op == vir::BinaryOp::Ne) {  // decision 0018 item 6 (i)
      allow_enum_conversion(ops[0]);
      allow_enum_conversion(ops[1]);
    }
    if ((op == vir::BinaryOp::Eq || op == vir::BinaryOp::Ne) && detect_tautological_compare(e, ops)) return nullptr;  // 0023 item 5
    if (detect_promoted_complement_compare(e, ops)) return nullptr;                                                   // 0023 item 5 (6c)
    if (op == vir::BinaryOp::Eq || op == vir::BinaryOp::Ne) {
      // Decision 0023 item 7: in C, a `?:` of enumerators of one enumeration is rejected here as C++ rejects it (its type).
      if (reject_enumerator_conditional(ops)) return nullptr;
      // Decision 0022 item 5: an enumeration value compared with a signed integer that may be negative is rejected.
      if (reject_enumeration_sign_compare(e, ops)) return nullptr;
    } else {
      // Decision 0019 item 10: an order of two values of one enumeration (or of its enumerators).
      const Expr* ea = nullptr;
      const Expr* ec = nullptr;
      const int r = ordered_enumeration_operands(e, ops, &ea, &ec);
      if (r < 0) return nullptr;
      if (r > 0) return logical_result(e, b_.binary(op, ea, ec, loc_of(e->position)));
    }
    const Expr* a = expr(ops[0]);
    const Expr* c = expr(ops[1]);
    if (a == nullptr || c == nullptr) return nullptr;
    if (is_enum_type(a->type) || is_enum_type(c->type)) {
      // Raw enumeration operands (decision 0018 items 6-7): the same enumeration on both sides; an order only for a scoped one.
      const bool ordering = op != vir::BinaryOp::Eq && op != vir::BinaryOp::Ne;
      if (a->type != c->type) {
        reject(Diag::unsupported_expression, e->position, "a comparison of an enumeration value with a value of another type (decision 0018 item 6)");
        return nullptr;
      }
      if (ordering && !a->type.int_type->is_scoped) {
        reject(Diag::unsupported_expression, e->position, "a comparison of order of enumeration values of an unscoped enumeration (decision 0018 item 7)");
        return nullptr;
      }
    }
    return logical_result(e, b_.binary(op, a, c, loc_of(e->position)));
  }

  const Expr* logical(an_expr_node_ptr e, vir::BinaryOp op) {
    auto ops = operands_of(e);
    if (!operand_count(e, 2, ops)) return nullptr;
    detect_or_as_truth_value(ops[0]);
    detect_or_as_truth_value(ops[1]);
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
    if (from.t.is_bool()) {
      if (is_enum_type(to.t)) {
        reject(Diag::unsupported_expression, e->position, "a conversion of a bool to an enumeration (decision 0018 item 5)");
        return nullptr;
      }
      return b_.bool_to_int(to.t, x, l);
    }
    if (from.t == to.t) return x;
    if (is_enum_type(from.t) && !is_enum_type(to.t) && e->compiler_generated &&
        enum_conv_ok_.count(e) == 0 && !is_enumerator_node(ops[0])) {
      // An implicit conversion out of an enumeration means the same in C and C++ only where a parent allowed it (decision 0018
      // item 6); C promotes to unsigned int where C++ promotes to int, so everywhere else it is rejected.
      reject(Diag::unsupported_expression, e->position,
             "an implicit conversion of an enumeration value to an integer in arithmetic, a bitwise operator, a comparison of "
             "order, a unary operator, a shift or ?: (decision 0018 item 6, decision 0023 item 4)");
      return nullptr;
    }
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
      allow_enum_conversion(ops[i]);  // decision 0018 item 6 (iii)
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
        if (is_enum_type(a->type)) {
          reject(Diag::unsupported_expression, e->position, "a unary operator on an enumeration value (decision 0018 item 6)");
          return nullptr;
        }
        return finish(e, b_.unary(k == eok_negate ? vir::UnaryOp::Neg : vir::UnaryOp::Plus, a, loc_of(e->position)));
      }
      case eok_not: {
        if (!operand_count(e, 1, ops)) return nullptr;
        detect_or_as_truth_value(ops[0]);
        const Expr* a = as_bool(expr(ops[0]), ops[0]->position);
        if (a == nullptr) return nullptr;
        return logical_result(e, b_.unary(vir::UnaryOp::Not, a, loc_of(e->position)));
      }
      case eok_question: {
        if (!operand_count(e, 3, ops)) return nullptr;
        if (detect_bitwise_condition(ops[0])) return nullptr;  // decision 0023 item 5
        detect_or_as_truth_value(ops[0]);
        const Expr* c = as_bool(expr(ops[0]), ops[0]->position);
        const Expr* a = expr(ops[1]);
        const Expr* d = expr(ops[2]);
        if (c == nullptr || a == nullptr || d == nullptr) return nullptr;
        if (is_enum_type(a->type) || is_enum_type(d->type)) {
          reject(Diag::unsupported_expression, e->position, "a conditional expression of enumeration type (decision 0018 item 8)");
          return nullptr;
        }
        return finish(e, b_.cond(c, a, d, loc_of(e->position)));
      }
      case eok_assign:
      case eok_add_assign:
      case eok_subtract_assign:
      case eok_multiply_assign:
      case eok_divide_assign:
      case eok_remainder_assign:
      case eok_shiftl_assign:
      case eok_shiftr_assign:
      case eok_and_assign:
      case eok_or_assign:
      case eok_xor_assign:
      case eok_pre_incr:
      case eok_post_incr:
      case eok_pre_decr:
      case eok_post_decr: return assignment_value(e, ops);
      case eok_and:
      case eok_or:
      case eok_xor: return bitwise_binary(e, ops);
      case eok_complement: return bitwise_complement(e, ops);
      case eok_comma: return comma_value(e, ops);
      case eok_cast: return conversion(e);
      case eok_bool_cast: {
        if (!operand_count(e, 1, ops)) return nullptr;
        detect_or_as_truth_value(ops[0]);
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
    detect_or_as_truth_value(arg);  // decision 0023 item 5: `check(x | 4)`
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
      detect_or_as_truth_value(arg);  // decision 0023 item 5: `invariant(x | 4)`
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
      if (t.kind == iek_static_assertion) {  // decision 0024 item 2: checked as written, in the prologue
        encode_block_static_assertion(reinterpret_cast<a_static_assertion_ptr>(t.ptr));
        continue;
      }
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
      // Decision 0019 item 1: a non-const local of an accepted scalar type without an initializer is ordinary; reading it
      // before an assignment is an `uninitialized` verification error (Silicon decides it on each path).  A const one can
      // never be given a value (C++ error #257): still rejected.
      if (v->init_kind == initk_none && is_const_object(v)) {
        reject(Diag::unsupported_declaration, at, "a const local variable without an initializer");
        cx_.rejected_vars.insert(v);
        continue;
      }
      const char* nm = v->source_corresp.name;
      vir::Var* var = cx_.fn->new_local(nm ? nm : "", vt.t, loc_of(at));
      var->c_type_name = vt.name;
      cx_.vars[v] = var;
      if (v->init_kind == initk_none) {
        // initk_none and no `stmk_init` statement (P28): the declaration alone, with no initializer expression.
        out.push_back(b_.decl(var, nullptr, loc_of(at)));
      } else if (v->init_kind == initk_static) {
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
      allow_enum_conversion(init_expr);  // decision 0018 item 6 (iii)
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
    // A whole initializer (decision 0018 item 6 (iii)): the mark is on this stack node only, and goes before it does.
    allow_enum_conversion(&node);
    const Expr* x = constant(&node);
    enum_conv_ok_.erase(&node);
    return x;
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
    // As variable() (0012 item 3): `int y = (y = 1);` stays rejected now that an assignment may be nested.
    if (cx_.declaring == e->variant.variable.ptr || cx_.pending.count(e->variant.variable.ptr) != 0) {
      reject(Diag::unsupported_declaration, e->variant.variable.ptr->source_corresp.decl_position,
             "a local variable used in its own initializer");
      return nullptr;
    }
    return it->second;
  }

  // ---- assignments, compound assignments, `++` and `--` (decision 0020 items 1-2, 10) -----------------------------------------
  // The kinds that are encoded: `=`, the ten arithmetic, shift and bitwise compound assignments (decision 0023 item 3), and the four
  // increments and decrements.
  static bool is_encoded_assignment_kind(an_expr_operator_kind k) {
    switch (k) {
      case eok_assign:
      case eok_add_assign:
      case eok_subtract_assign:
      case eok_multiply_assign:
      case eok_divide_assign:
      case eok_remainder_assign:
      case eok_shiftl_assign:
      case eok_shiftr_assign:
      case eok_and_assign:
      case eok_or_assign:
      case eok_xor_assign:
      case eok_pre_incr:
      case eok_post_incr:
      case eok_pre_decr:
      case eok_post_decr: return true;
      default: return false;
    }
  }

  // What an assignment-like node says, checked the same way at statement level and as a value.
  struct AssignParts {
    vir::Var* v = nullptr;                  // the local written
    const Expr* x = nullptr;                // `=` and compound: the (converted) right-hand side; null for `++`/`--`
    vir::BinaryOp op = vir::BinaryOp::Add;  // compound assignment
    bool increment = false;                 // `++`/`--`
  };
  enum class AssignRes { ok, rejected, malformed };

  // `x = x` (decision 0021 item 8, probe P36): the shapes clang's -Wself-assign warns about.  Both operands, once the front end's
  // own conversions are peeled (never an explicit cast, which is its own node without `compiler_generated`; parentheses are a
  // flag, not a node), are the same variable, and neither the `=` nor an operand comes from a macro expansion: a node written
  // outside any macro has orig_seq/orig_column equal to seq/column (basics.h a_source_position), which is not the
  // `orig_seq != 0` test of macro_node.
  static an_expr_node_ptr peel_generated_conversions(an_expr_node_ptr e) {
    for (int guard = 0; e != nullptr && guard < 16 && e->kind == enk_operation && e->compiler_generated &&
                        (e->variant.operation.kind == eok_cast || e->variant.operation.kind == eok_bool_cast) &&
                        e->variant.operation.operands != nullptr && e->variant.operation.operands->next == nullptr;
         ++guard)
      e = e->variant.operation.operands;
    return e;
  }
  static bool written_outside_macros(an_expr_node_ptr e) {
    return e->position.orig_seq == e->position.seq && e->position.orig_column == e->position.column;
  }
  // True when it rejected: `x = x`, and (decision 0023 item 5) `x &= x` and `x |= x`; clang is silent on `x ^= x`.
  bool check_self_assignment(an_expr_node_ptr assign, const std::vector<an_expr_node_ptr>& ops) {
    if (ops.size() != 2) return false;
    an_expr_node_ptr l = peel_generated_conversions(ops[0]);
    an_expr_node_ptr r = peel_generated_conversions(ops[1]);
    if (l == nullptr || r == nullptr || l->kind != enk_variable || r->kind != enk_variable) return false;
    if (l->variant.variable.ptr == nullptr || l->variant.variable.ptr != r->variant.variable.ptr) return false;
    if (!written_outside_macros(assign) || !written_outside_macros(l) || !written_outside_macros(r)) return false;
    detected_.insert(assign);
    reject(Diag::self_assignment, assign->position, "");
    return true;
  }

  AssignRes assignment_parts(an_expr_node_ptr e, const std::vector<an_expr_node_ptr>& ops, AssignParts* p) {
    const an_expr_operator_kind k = e->variant.operation.kind;
    const SrcLoc l = loc_of(e->position);
    if (k == eok_assign) {
      if (ops.size() != 2) return AssignRes::malformed;
      check_self_assignment(e, ops);
      p->v = lvalue_var(ops[0]);
      allow_enum_conversion(ops[1]);  // decision 0018 item 6 (iii)
      p->x = expr(ops[1]);
      return p->v == nullptr || p->x == nullptr ? AssignRes::rejected : AssignRes::ok;
    }
    if (is_incdec_kind(k)) {
      if (ops.size() != 1) return AssignRes::malformed;
      p->v = lvalue_var(ops[0]);
      if (p->v == nullptr) return AssignRes::rejected;
      if (!p->v->type.is_int()) {
        reject(Diag::unsupported_expression, e->position, "an increment or decrement of a bool");
        return AssignRes::rejected;
      }
      if (is_enum_type(p->v->type)) {
        reject(Diag::unsupported_expression, e->position, "an increment or decrement of an enumeration (decision 0018 item 8)");
        return AssignRes::rejected;
      }
      p->increment = k == eok_pre_incr || k == eok_post_incr;
      return AssignRes::ok;
    }
    // A compound assignment.
    if (ops.size() != 2) return AssignRes::malformed;
    p->v = lvalue_var(ops[0]);
    p->x = expr(ops[1]);
    if (p->v == nullptr || p->x == nullptr) return AssignRes::rejected;
    if (!p->v->type.is_int()) {
      reject(Diag::unsupported_expression, e->position, "a compound assignment to a bool");
      return AssignRes::rejected;
    }
    if (is_enum_type(p->v->type) || is_enum_type(p->x->type)) {
      // Both compute in the promoted type, which differs between C and C++ (decision 0018 item 8).
      reject(Diag::unsupported_expression, e->position, "a compound assignment to or with an enumeration value (decision 0018 item 8)");
      return AssignRes::rejected;
    }
    if (p->x->type.is_bool()) {
      // `i += flag`: the bool is promoted to int first.
      const vir::IntType* i = prog_.int_type("int", prog_.c_int_bits(), true);
      p->x = b_.bool_to_int(vir::Type::integer(i), p->x, l);
    }
    // An exhaustive switch: a kind that is not listed is rejected here and never read as `>>=`.  The bitwise forms (decision 0023
    // item 3): the rhs arrives cast to the usual arithmetic conversion of both operands (P40) and the result converts back to the
    // left type with the 0022 check, like the other compound forms; `x &= x` and `x |= x` are rejected first (clang -Wself-assign).
    switch (k) {
      case eok_and_assign:
      case eok_or_assign:
        if (check_self_assignment(e, ops)) return AssignRes::rejected;
        p->op = k == eok_and_assign ? vir::BinaryOp::BitAnd : vir::BinaryOp::BitOr;
        break;
      case eok_xor_assign: p->op = vir::BinaryOp::BitXor; break;
      case eok_add_assign: p->op = vir::BinaryOp::Add; break;
      case eok_subtract_assign: p->op = vir::BinaryOp::Sub; break;
      case eok_multiply_assign: p->op = vir::BinaryOp::Mul; break;
      case eok_divide_assign: p->op = vir::BinaryOp::Div; break;
      case eok_remainder_assign: p->op = vir::BinaryOp::Rem; break;
      case eok_shiftl_assign: p->op = vir::BinaryOp::Shl; break;
      case eok_shiftr_assign: p->op = vir::BinaryOp::Shr; break;
      default:
        reject(Diag::unsupported_expression, e->position, describe_operation(k));
        return AssignRes::rejected;
    }
    return AssignRes::ok;
  }

  // An assignment, compound assignment or `++`/`--` whose value is used (decision 0020 item 1): an AssignE.  Its IL position is
  // kept for vir::sequence_problems.  P33: the node is directly the operand, with the left operand's type, and any conversion
  // the use needs is a generated cast around it.
  const Expr* assignment_value(an_expr_node_ptr e, const std::vector<an_expr_node_ptr>& ops) {
    if (cx_.contract || cx_.annotation || cx_.const_depth > 0) {
      // C++ may read `int y = (y = 1);` as a constant initializer: the own-initializer rule (0012 item 3) still gives its
      // diagnostic, as in C.
      if (!ops.empty() && ops[0]->kind == enk_variable && ops[0]->variant.variable.ptr != nullptr &&
          (cx_.declaring == ops[0]->variant.variable.ptr || cx_.pending.count(ops[0]->variant.variable.ptr) != 0) &&
          cx_.vars.count(ops[0]->variant.variable.ptr) != 0) {
        lvalue_var(ops[0]);
        return nullptr;
      }
      reject(Diag::unsupported_expression, e->position, "an assignment, increment or decrement in an annotation, or in an expression the compiler folded to a constant "
             "(`0 && (x = 1)` and `1 || x++` are never evaluated)");
      return nullptr;
    }
    const an_expr_operator_kind k = e->variant.operation.kind;
    AssignParts a;
    const AssignRes res = assignment_parts(e, ops, &a);
    if (res == AssignRes::malformed) {
      operand_count(e, is_incdec_kind(k) ? 1 : 2, ops);
      return nullptr;
    }
    if (res != AssignRes::ok) return nullptr;
    const SrcLoc l = loc_of(e->position);
    const Expr* r = k == eok_assign        ? b_.assign_expr(a.v, a.x, l)
                    : is_incdec_kind(k)    ? b_.inc_dec_expr(a.v, a.increment, k == eok_post_incr || k == eok_post_decr, l)
                                           : b_.compound_assign_expr(a.v, a.op, a.x, l);
    cx_.assign_positions[r] = e->position;
    return finish(e, r);
  }

  // The comma operator whose value is used (decision 0020 item 1): the left operand is evaluated first, completely (a call of a
  // void function or a `(void)` cast is allowed there); rules B(1) and C of item 5 apply to it.  A comma whose value is discarded is
  // statements instead (expression_statement).
  const Expr* comma_value(an_expr_node_ptr e, const std::vector<an_expr_node_ptr>& ops) {
    if (!operand_count(e, 2, ops)) return nullptr;
    if (cx_.contract || cx_.annotation || cx_.const_depth > 0) {
      reject(Diag::unsupported_expression, e->position, "the comma operator in an annotation, or in an expression the compiler folded to a constant");
      return nullptr;
    }
    if (comma_left_without_effect(ops[0])) reject(Diag::comma_without_effect, e->position, "");
    an_expr_node_ptr left = ops[0];
    bool voided = false;
    while (is_void_cast(left)) {
      left = left->variant.operation.operands;
      voided = true;
    }
    if (!voided) {
      // The left operand's value is discarded: a comma on its left has its own right operand discarded (rule B(2)), and rule C.
      // (discarded_value_with_effect reports B(2) for a comma there.)
      if (an_expr_node_ptr d = discarded_value_with_effect(left)) reject(Diag::value_not_used, d->position, "");
    }
    const Expr* a = expr(left);
    const Expr* b = expr(ops[1]);
    if (a == nullptr || b == nullptr) return nullptr;
    if (b->type.is_void()) {
      reject(Diag::unsupported_expression, e->position, "a comma operator whose right operand is void, used as a value");
      return nullptr;
    }
    return finish(e, b_.comma(a, b, loc_of(e->position)));
  }

  // The rejections of vir::sequence_problems (decision 0020 item 4): an unsequenced modification, at the write.
  void report_sequence_problems(vir::Function* f) {
    for (const vir::SequenceProblem& sp : vir::sequence_problems(*f)) {
      auto pos = cx_.assign_positions.find(sp.at);
      const a_source_position at = pos != cx_.assign_positions.end() ? pos->second : cx_.stmt_pos;
      reject(Diag::unsequenced_modification, at, sp.variable);
    }
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
  //
  // A discarded comma is the statements of its operands in order (decision 0020 item 6: the comma is a sequence point, so this
  // is exact); `voided` says that an enclosing `(void)` cast covers `e` (a comma's right operand).
  void expression_statement(an_expr_node_ptr e, const a_source_position& pos, Out& out, bool voided = false) {
    // `(void)e;`
    while (e->kind == enk_operation && e->variant.operation.kind == eok_cast) {
      a_type_ptr to = skip_typerefs(e->type);
      if (to == nullptr || to->kind != tk_void) break;
      auto ops = operands_of(e);
      if (ops.size() != 1) break;
      e = ops[0];
      voided = true;
    }
    if (an_expr_node_ptr cond = assert_condition(e)) {
      if (an_expr_node_ptr fx = first_effect(cond, false)) {
        // 0015 item 6: `assert` is checked like `check`, and NDEBUG would remove its side effects.
        reject(Diag::unsupported_expression, fx->position, "a side effect in the condition of assert (decision 0015 item 6)");
        return;
      }
      // `assert(cond)` from <assert.h> (decision 0015 item 6): `cond` is code, evaluated as an `if` condition is; the
      // arguments of `__assert_rtn` are never encoded.
      if (cx_.in_for_step) {
        // A check in a loop's step is an internal error of check_program: reject it here, at the source.
        reject(Diag::unsupported_expression, pos, "an assert in the increment of a for loop");
        return;
      }
      detect_or_as_truth_value(cond);  // decision 0023 item 5: `assert(x | 4)`
      const Expr* c = as_bool(expr(cond), cond->position);
      if (c != nullptr) out.push_back(b_.check(c, loc_of(pos), true));
      return;
    }
    if (e->kind == enk_operation) {
      const an_expr_operator_kind k = e->variant.operation.kind;
      auto ops = operands_of(e);
      const SrcLoc l = loc_of(e->position);
      if (is_encoded_assignment_kind(k)) {
        // An assignment, compound assignment or `++`/`--` at the top of an expression statement stays a VIR statement (decision
        // 0020 item 6): code without nested effects builds the VIR it always did.
        AssignParts a;
        const AssignRes r = assignment_parts(e, ops, &a);
        if (r == AssignRes::rejected) return;
        if (r == AssignRes::ok) {
          if (k == eok_assign) out.push_back(b_.assign(a.v, a.x, l));
          else if (is_incdec_kind(k)) out.push_back(b_.inc_dec(a.v, a.increment, l));
          else out.push_back(b_.compound_assign(a.v, a.op, a.x, l));
          return;
        }
      }
      switch (k) {
        case eok_comma: {
          if (ops.size() != 2) break;
          // The shapes the compilers warn about (item 5), once per comma; the operands are still encoded.
          if (comma_left_without_effect(ops[0]) || (!voided && discarded_comma_right_without_effect(ops[1])))
            reject(Diag::comma_without_effect, e->position, "");
          expression_statement(ops[0], ops[0]->position.seq ? ops[0]->position : pos, out);
          expression_statement(ops[1], ops[1]->position.seq ? ops[1]->position : pos, out, voided);
          return;
        }
        default: break;
      }
    }
    if (!voided) {
      if (an_expr_node_ptr d = discarded_value_with_effect(e)) {
        reject(Diag::value_not_used, d->position, "");  // item 5 (b), at the operator that discards the value
        return;
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
    allow_enum_conversion(s->expr);  // decision 0018 item 6 (iii)
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
        if (is_enumerator(c)) return false;  // an enumerator names no object (decision 0018 item 9): not its initializer
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

  // ---- side effects in expressions (decision 0020) ----------------------------------------------------------------------
  // An assignment of any kind, `++` or `--` (the kinds annotation_checks.cpp's side_effect_of names).
  static bool is_assignment_kind(an_expr_operator_kind k) {
    switch (k) {
      case eok_assign:
      case eok_bassign:
      case eok_add_assign:
      case eok_subtract_assign:
      case eok_multiply_assign:
      case eok_divide_assign:
      case eok_remainder_assign:
      case eok_shiftl_assign:
      case eok_shiftr_assign:
      case eok_and_assign:
      case eok_or_assign:
      case eok_xor_assign:
      case eok_padd_assign:
      case eok_psubtract_assign: return true;
      default: return false;
    }
  }
  static bool is_incdec_kind(an_expr_operator_kind k) {
    return k == eok_pre_incr || k == eok_post_incr || k == eok_pre_decr || k == eok_post_decr;
  }
  static bool is_call_kind(an_expr_operator_kind k) {
    return k == eok_call || k == eok_dot_member_call || k == eok_points_to_member_call;
  }
  static bool is_effect_operation(an_expr_node_ptr e, bool with_calls = true) {
    if (e == nullptr || e->kind != enk_operation) return false;
    const an_expr_operator_kind k = e->variant.operation.kind;
    return is_assignment_kind(k) || is_incdec_kind(k) || (with_calls && is_call_kind(k));
  }

  // The first node (in evaluation-agnostic IL order) of `e` that has a side effect: an assignment, an increment or decrement, or
  // a call.  Folded constants and the operands of `sizeof` are not followed (they are not evaluated); null when there is none.
  static an_expr_node_ptr first_effect(an_expr_node_ptr e, bool with_calls = true, int depth = 0) {
    if (e == nullptr || e->kind != enk_operation || depth > 1000) return nullptr;
    if (is_effect_operation(e, with_calls)) return e;
    for (an_expr_node_ptr o = e->variant.operation.operands; o != nullptr; o = o->next)
      if (an_expr_node_ptr f = first_effect(o, with_calls, depth + 1)) return f;
    return nullptr;
  }

  // `(void) x`: an explicit discard (the compilers do not warn about a value discarded this way).
  static bool is_void_cast(an_expr_node_ptr e) {
    if (e == nullptr || e->kind != enk_operation || e->variant.operation.kind != eok_cast) return false;
    a_type_ptr to = skip_typerefs(e->type);
    return to != nullptr && to->kind == tk_void && e->variant.operation.operands != nullptr &&
           e->variant.operation.operands->next == nullptr;
  }

  // ---- the shapes the compilers warn about (decision 0020 item 5; the lists are finalized from probe P35) ----
  // `if (x = a)`: an assignment `=` that is the whole condition of if, while, do or for (after the front end's own bool
  // conversion) and has no parentheses of its own.  clang's and GCC's -Wparentheses warn about exactly these, in C and C++, and
  // about no compound assignment, `?:` condition, `&&` operand or `!` operand.  The node, or null.
  static an_expr_node_ptr assignment_as_condition_node(an_expr_node_ptr c) {
    for (int guard = 0; c != nullptr && guard < 16 && c->kind == enk_operation && c->compiler_generated &&
                        (c->variant.operation.kind == eok_bool_cast || c->variant.operation.kind == eok_cast) &&
                        c->variant.operation.operands != nullptr && c->variant.operation.operands->next == nullptr;
         ++guard)
      c = c->variant.operation.operands;
    // `=`, and (decision 0023 item 5) `|=` as the whole condition (clang); `&=`, `^=` and the arithmetic compounds are silent.
    if (c == nullptr || c->kind != enk_operation ||
        (c->variant.operation.kind != eok_assign && c->variant.operation.kind != eok_or_assign))
      return nullptr;
    return c->is_parenthesized ? nullptr : c;
  }
  void check_condition(an_expr_node_ptr c) {
    if (an_expr_node_ptr a = assignment_as_condition_node(c)) {
      detected_.insert(a);
      reject(Diag::assignment_as_condition, a->position, "");
    }
    detect_or_as_truth_value(c);  // decision 0023 item 5: `if (x | 4)`
  }

  // The left operand of a comma has no effect where the compilers warn: not a `(void)` cast (explicit) and no side effect inside.
  static bool comma_left_without_effect(an_expr_node_ptr l) { return !is_void_cast(l) && first_effect(l) == nullptr; }
  // The right operand of a comma whose value is discarded has no effect (and is not itself a `(void)` cast).  Not asked when the
  // whole comma is cast to void: neither compiler warns then.
  static bool discarded_comma_right_without_effect(an_expr_node_ptr r) { return !is_void_cast(r) && first_effect(r) == nullptr; }
  // Rule C of decision 0020 item 5 (P35 shape (b)): a discarded expression (a whole expression statement, or a discarded comma
  // operand) whose value is thrown away by an operator while a side effect sits inside.  The node of the operator that discards
  // the value, or null.  Fine: a `(void)` cast, an assignment, a compound assignment, `++`/`--`, a call and a comma (its operands
  // are checked by rule B and again by this rule); for `?:` each arm is checked (a pure arm is fine: `a ? x++ : 0;`); for `&&`
  // and `||` with a side effect anywhere in it (also only in the left operand: `x++ && a;`, P37) the right operand must be a plain
  // `=` of an integer constant (`a || (x = 1);`: GCC folds the condition away) or another `&&`/`||` checked the same way, and
  // anything else is rejected (`a && x++;`, `a && (x = b);`, `a && f();`, `g(x) && a;`);
  // any other top operator is rejected when it contains a side effect (`x++ + 1;`, `-x++;`,
  // `(int)x++;`, `x == (y = 1);`, `g(x) + 1;`).  A pure discarded expression (`x + 1;`) stays accepted (0012 item 3).
  // (Not static: a discarded comma inside reports rule B(2) itself.)
  an_expr_node_ptr discarded_value_with_effect(an_expr_node_ptr e, int depth = 0) {
    // The front end's own conversions around an operand (C++ converts the operands of `&&`/`||` to bool) are not the user's operator.
    for (int guard = 0; e != nullptr && guard < 16 && e->kind == enk_operation && e->compiler_generated &&
                        (e->variant.operation.kind == eok_bool_cast || e->variant.operation.kind == eok_cast) &&
                        e->variant.operation.operands != nullptr && e->variant.operation.operands->next == nullptr;
         ++guard)
      e = e->variant.operation.operands;
    if (e == nullptr || e->kind != enk_operation || depth > 64 || is_void_cast(e)) return nullptr;
    const an_expr_operator_kind k = e->variant.operation.kind;
    if (is_assignment_kind(k) || is_incdec_kind(k) || is_call_kind(k)) return nullptr;
    auto ops = operands_of(e);
    if (k == eok_comma) {
      // A discarded comma (an arm of a discarded `?:`, or the left operand of a comma): its right operand is discarded too, so
      // rule B(2) applies to it and rule C to that operand, recursively.  (Its left operand is checked where it is encoded.)
      if (ops.size() != 2) return nullptr;
      if (discarded_comma_right_without_effect(ops[1])) reject(Diag::comma_without_effect, e->position, "");
      return discarded_value_with_effect(ops[1], depth + 1);
    }
    if (k == eok_question && ops.size() == 3) {
      // Each arm is a discarded position (rules B(2) and C apply to it).  When neither arm has an effect, the effect is in the
      // condition alone and its value is thrown away (`x++ ? 1 : 0;`); `a ? x++ : 0;` and `a ? (x = 1) : (x = 2);` are fine.
      if (an_expr_node_ptr x = discarded_value_with_effect(ops[1], depth + 1)) return x;
      if (an_expr_node_ptr x = discarded_value_with_effect(ops[2], depth + 1)) return x;
      if (first_effect(ops[1]) == nullptr && first_effect(ops[2]) == nullptr && first_effect(ops[0]) != nullptr) return e;
      return nullptr;
    }
    if ((k == eok_land || k == eok_lor) && ops.size() == 2) {
      // P37: GCC's C mode warns "value computed is not used" on a discarded `&&`/`||` that has a side effect anywhere in it (also
      // only in the left operand: `x++ && a;`, `g(x) || a;`), unless the right operand is a plain assignment of an integer constant
      // (GCC folds the condition away) or another `&&`/`||` whose right operand is checked the same way.  Reported at the operator
      // whose right operand fails.  With no side effect at all the expression stays accepted (0012 item 3).
      if (first_effect(e) == nullptr) return nullptr;
      an_expr_node_ptr cur = e;
      for (int hops = 0; hops < 64; ++hops) {
        auto cops = operands_of(cur);
        if (cops.size() != 2) return cur;
        an_expr_node_ptr r = peel_generated_conversions(cops[1]);
        if (r == nullptr || r->kind != enk_operation) return cur;
        const an_expr_operator_kind rk = r->variant.operation.kind;
        if (rk == eok_land || rk == eok_lor) {
          cur = r;
          continue;
        }
        if (rk != eok_assign) return cur;
        // `a && (x = 2);` is fine: C turns `(x = 2) != 0` into the constant 1, which GCC folds away.  A constant zero is not
        // (`a && (x = 0);`, `a && (x = 1 - 1);`: the folded value decides), unless the target is a bool (P37, audit round 2).  A
        // non-constant right-hand side is `(x = b) != 0`, whose value is computed and not used.
        auto ro = operands_of(r);
        an_expr_node_ptr rhs = ro.size() == 2 ? peel_generated_conversions(ro[1]) : nullptr;
        if (rhs != nullptr && rhs->kind == enk_constant && rhs->variant.constant.ptr != nullptr &&
            rhs->variant.constant.ptr->kind == ck_integer) {
          a_type_ptr target = ro[0]->type != nullptr ? skip_typerefs(ro[0]->type) : nullptr;
          const bool bool_target = target != nullptr && target->kind == tk_integer && target->variant.integer.bool_type;
          if (bool_target || !is_zero_constant(rhs)) return nullptr;
        }
        return cur;
      }
      return e;
    }
    return first_effect(e) != nullptr ? e : nullptr;
  }
  // A side effect in the operand of `sizeof`, `_Alignof` or `noexcept` (not evaluated), reached through the folded constant that
  // replaces the operator: the node of the effect, or null.  P33: `typeof`/`decltype` operands are not reachable from statements.
  static an_expr_node_ptr unevaluated_side_effect(an_expr_node_ptr original, const char** what) {
    if (original == nullptr) return nullptr;
    if (original->kind == enk_sizeof || original->kind == enk_alignof || original->kind == enk_datasizeof) {
      const auto& si = original->variant.sizeof_info;
      if (si.is_type) return nullptr;
      *what = original->kind == enk_sizeof ? "sizeof" : original->kind == enk_alignof ? "alignof" : "__datasizeof";
      return effect_in_unevaluated(si.variant.expr);
    }
    if (original->kind == enk_operation && original->variant.operation.kind == eok_noexcept && original->variant.operation.operands) {
      *what = "noexcept";
      return effect_in_unevaluated(original->variant.operation.operands);
    }
    return nullptr;
  }
  // A side effect anywhere inside an unevaluated operand, following what first_effect does not: the original expression of a
  // folded constant (`sizeof(sizeof(x++))`: the inner `sizeof` is a constant whose expression is `x++`) and the operand of a nested
  // `sizeof`, `_Alignof` or `noexcept`.
  static an_expr_node_ptr effect_in_unevaluated(an_expr_node_ptr e, int depth = 0) {
    if (e == nullptr || depth > 64) return nullptr;
    switch (e->kind) {
      case enk_operation:
        if (is_effect_operation(e)) return e;
        for (an_expr_node_ptr o = e->variant.operation.operands; o != nullptr; o = o->next)
          if (an_expr_node_ptr f = effect_in_unevaluated(o, depth + 1)) return f;
        return nullptr;
      case enk_constant: {
        a_constant_ptr c = e->variant.constant.ptr;
        if (c == nullptr || (c->expr == nullptr && !c->local_expr_ref)) return nullptr;
        an_expr_node_ptr o = expr_node_from_constant(c);
        return o != nullptr && o != e ? effect_in_unevaluated(o, depth + 1) : nullptr;
      }
      case enk_sizeof:
      case enk_alignof:
      case enk_datasizeof:
        return e->variant.sizeof_info.is_type ? nullptr : effect_in_unevaluated(e->variant.sizeof_info.variant.expr, depth + 1);
      default: return nullptr;
    }
  }

  // Whether a type is, through its typeref chain, a `typeof (expression)` type (il_def.h:9712, trk_is_typeof_with_expression).
  static bool is_typeof_expression_type(a_type_ptr t) {
    for (int guard = 0; t != nullptr && t->kind == tk_typeref && guard < 64; ++guard) {
      if (is_typeref_kind(t, trk_is_typeof_with_expression)) return true;
      t = t->variant.typeref.type;
    }
    return false;
  }

  // The only `sizeof`/`_Alignof` operand a C enumerator's initializer may have: an arithmetic type that is not bit-precise, named
  // directly or through plain typedefs (trk_none, il_def.h:9698).  Anything else (an expression, an array whose bound is an
  // expression, a `typeof` nested in a derived type, `_BitInt(e)`) can observe the front end's C++-style type of an earlier
  // enumerator (decision 0018 item 10; Opus audit, round 3).
  static bool plain_arithmetic_type(a_type_ptr t) {
    for (int guard = 0; t != nullptr && t->kind == tk_typeref && guard < 64; ++guard) {
      if (!is_typeref_kind(t, trk_none)) return false;
      t = t->variant.typeref.type;
    }
    if (t == nullptr) return false;
    if (t->kind == tk_float) return true;
    if (t->kind != tk_integer) return false;
    return t->variant.integer.int_kind != ik_bit_precise && t->variant.integer.int_kind != ik_unsigned_bit_precise;
  }

  // Whether the original expression of an enumerator's value contains a `sizeof`/`_Alignof` of an expression, or any type that is
  // `typeof (expression)` (a node type, a cast target, a `sizeof` type operand), following folded constants as names_object does
  // and stopping at enumerators: in C the front end types an earlier enumerator of the list the C++ way, which these observe
  // (decision 0018 item 10).
  static bool typeof_or_sizeof_of_expression(an_expr_node_ptr e, int depth = 0) {
    if (e == nullptr || depth > 64) return false;
    if (is_typeof_expression_type(e->type)) return true;
    switch (e->kind) {
      case enk_constant: {
        a_constant_ptr c = e->variant.constant.ptr;
        if (c == nullptr || is_enumerator(c)) return false;
        if (is_typeof_expression_type(c->type)) return true;
        if (c->expr == nullptr && !c->local_expr_ref) return false;
        an_expr_node_ptr o = expr_node_from_constant(c);
        return o != nullptr && o != e && typeof_or_sizeof_of_expression(o, depth + 1);
      }
      case enk_sizeof:
      case enk_alignof:
      case enk_datasizeof: {
        const auto& si = e->variant.sizeof_info;
        if (!si.is_type) return true;
        return !plain_arithmetic_type(si.variant.type);
      }
      case enk_operation:
        for (an_expr_node_ptr o = e->variant.operation.operands; o != nullptr; o = o->next)
          if (typeof_or_sizeof_of_expression(o, depth + 1)) return true;
        return false;
      default: return false;
    }
  }

  // The first variable declared WITH an initializer by a declaration statement (every entity is looked at: `int x, y = 1;` is
  // one `stmk_decl` whose later initializers come as `stmk_init`s, P28), or null.  A declaration all of whose entities are
  // variables without an initializer may be jumped over and may stand before the first case label (decision 0019 item 6).
  static a_variable_ptr initialized_variable(a_statement_ptr s) {
    if (s->kind == stmk_decl) {
      for (an_il_entity_list_entry_ptr en = s->variant.decl.entities; en != nullptr; en = en->next)
        if (en->entity.kind == iek_variable) {
          a_variable_ptr v = reinterpret_cast<a_variable_ptr>(en->entity.ptr);
          if (v->init_kind != initk_none) return v;
        }
      return nullptr;
    }
    if (s->kind == stmk_block && s->compiler_generated)
      for (a_statement_ptr c = s->variant.block.statements; c != nullptr; c = c->next)
        if (a_variable_ptr v = initialized_variable(c)) return v;
    return nullptr;
  }

  // A declaration of nothing but variables without an initializer (also inside a block the compiler made).
  static bool is_uninitialized_declaration(a_statement_ptr s) {
    if (s->kind == stmk_decl) {
      if (s->variant.decl.entities == nullptr) return false;
      for (an_il_entity_list_entry_ptr en = s->variant.decl.entities; en != nullptr; en = en->next) {
        if (en->entity.kind != iek_variable) return false;
        if (reinterpret_cast<a_variable_ptr>(en->entity.ptr)->init_kind != initk_none) return false;
      }
      return true;
    }
    if (s->kind == stmk_block && s->compiler_generated && s->attributes == nullptr) {
      if (s->variant.block.statements == nullptr) return false;
      for (a_statement_ptr c = s->variant.block.statements; c != nullptr; c = c->next)
        if (!is_uninitialized_declaration(c)) return false;
      return true;
    }
    return false;
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

  // The value of a case label of a C switch over a bare enumeration value, converted to the promoted type of the condition
  // (C23 6.8.5.3).  EDG's value is `(enum E)(operand)`, an implicit cast into the enumeration (P26); the operand is encoded as a
  // constant expression and converted to `promoted` (decision 0022 item 1: the conversion is checked, so `case -1:` over an `unsigned int` scrutinee is a
  // `conversion` error, never the wrapped value 4294967295).  Any other shape is rejected.
  const Expr* enum_case_value(a_constant_ptr cv, const a_source_position& pos, const vir::Type& enum_type, const vir::Type& promoted) {
    const SrcLoc l = loc_of(pos);
    const bool has_expr = cv->expr != nullptr || cv->local_expr_ref;
    if (is_enumerator(cv) || (!has_expr && cv->kind == ck_integer && cv->type != nullptr)) {
      // A copy of the enumerator's constant (an enumeration with a fixed type of rank >= int: its case values are enumerator
      // copies, enum-typed with no expression): its value, converted to the promoted type.
      VType ct = vtype(cv->type);
      if (is_enumerator(cv) ? !ct.ok : !(ct.ok && ct.t == enum_type)) {
        reject(Diag::unsupported_statement, pos, "a case value this version cannot read");
        return nullptr;
      }
      if (!is_enumerator(cv) && cv->orig_type != nullptr) {
        // Fail closed (probe P38, decision 0022 item 3): a constant converted from another type whose original expression is not
        // recorded could hide a conversion that does not fit; constant_value rejects that shape too (0010 item 12).
        reject(Diag::unsupported_statement, pos, "a folded constant whose original expression is not recorded");
        return nullptr;
      }
      const Expr* x = constant_value(cv, pos);
      if (x == nullptr) return nullptr;
      if (x->type != promoted) x = b_.convert(promoted, x, false, l);
      return x;
    }
    an_expr_node_ptr original = has_expr ? expr_node_from_constant(cv) : nullptr;
    an_expr_node_ptr operand = nullptr;
    if (original != nullptr && original->kind == enk_operation && original->variant.operation.kind == eok_cast &&
        operands_of(original).size() == 1) {
      // A generated cast is the front end's spelling of the conversion to the promoted type: its operand is the case constant.
      // A written one (`case (enum color)2:`) is the user's conversion into the enumeration: encoded as written (checked, its
      // obligation in the prologue), then promoted.
      VType to = vtype(original->type);
      if (to.ok && to.t == enum_type) operand = original->compiler_generated ? operands_of(original)[0] : original;
    }
    if (operand == nullptr) {
      reject(Diag::unsupported_statement, pos, "a case value this version cannot read");
      return nullptr;
    }
    ++cx_.const_depth;  // a constant expression: no calls (`call` rejects them)
    const Expr* x = expr(operand);
    --cx_.const_depth;
    if (x == nullptr) return nullptr;
    // The operand is an integer, or the enumeration itself (a written `case (enum color)2:`, encoded through the ordinary path:
    // a checked conversion into the enumeration, its obligation in the prologue).
    if (!x->type.is_int() || (is_enum_type(x->type) && x->type != enum_type)) {
      reject(Diag::unsupported_statement, pos, "a case value this version cannot read");
      return nullptr;
    }
    if (x->type != promoted) x = b_.convert(promoted, x, false, l);
    return x;
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
    allow_enum_conversion(s->expr);  // decision 0018 item 6 (ii): the controlling expression
    const Expr* scrut = expr(s->expr);
    bool ok = scrut != nullptr;
    if (ok && !scrut->type.is_int()) {
      reject(Diag::unsupported_statement, s->expr->position, "a switch on a value that is not an integer");
      ok = false;
    }
    // C23 6.8.5.3: the controlling expression is promoted and each case constant is converted to the promoted type.  EDG leaves a
    // bare enumeration value in C unconverted and casts each case value into the enumeration (P26), which would make `case 4:`
    // a conversion error: such a switch is encoded in the promoted type, the enumeration's compatible type (same bits and
    // signedness; `int` when narrower than int).
    bool c_enum_switch = false;
    vir::Type promoted = vir::Type::void_type();
    vir::Type enum_scrut_type = vir::Type::void_type();
    // Only where C promotes: an enumeration of rank <= int.  One with a fixed base above int (`: long`, `: unsigned long`) is not
    // promoted: its scrutinee stays the enumeration and its case values are encoded the ordinary way (a cast into the
    // enumeration is a checked conversion, never a wrap).
    if (ok && il_header.source_language == sl_C && is_enum_type(scrut->type) && scrut->type.int_type->bits <= prog_.c_int_bits()) {
      c_enum_switch = true;
      enum_scrut_type = scrut->type;
      const vir::IntType* et = scrut->type.int_type;
      const int cb = prog_.c_int_bits();
      const bool sgn = et->bits < cb ? true : et->is_signed;
      promoted = vir::Type::integer(prog_.int_type(sgn ? "int" : "unsigned int", cb, sgn));
      scrut = b_.convert(promoted, scrut, false, loc_of(s->expr->position));
    }
    // Decision 0019 item 11: a switch over an enumeration (before promotion, in both modes) is held to the two rules of
    // -Wswitch: every case has the value of an enumerator, and without a default every enumerator's value has a case.
    a_type_ptr sw_enum = ok ? switch_enumeration(s->expr) : nullptr;
    std::string sw_enum_name;
    std::vector<std::pair<std::string, std::string>> sw_enumerators;  // (name, value) in declaration order
    std::set<std::string> sw_enum_values, sw_case_values;
    bool sw_has_default = false;
    bool sw_rule1_failed = false;
    if (sw_enum != nullptr) {
      if (!enum_info(sw_enum).ok) {
        sw_enum = nullptr;  // rejected at its declaration or its use
      } else {
        sw_enum_name = render_type(sw_enum);
        for (a_constant_ptr k : enumerators_of(sw_enum)) {
          if (k->kind != ck_integer) {
            sw_enum = nullptr;  // enum_info refuses this: fail closed
            break;
          }
          const std::string val = decimal_value(k);
          sw_enumerators.emplace_back(k->source_corresp.name != nullptr ? k->source_corresp.name : "?", val);
          sw_enum_values.insert(val);
        }
      }
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
          a_variable_ptr v = initialized_variable(skipped);
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
          sw_has_default = true;
          inner.push_back(b_.case_label(nullptr, loc_of(c->position)));
          continue;
        }
        if (sw_enum != nullptr) {
          // Rule 1: the front end's value of the case is the value of an enumerator (`case 4:` over RED, GREEN, BLUE is not).
          if (en->case_value->kind != ck_integer) {
            reject(Diag::unsupported_statement, c->position, "a case value that is not an integer");
            ok = false;
            continue;
          }
          const std::string front_value = decimal_value(en->case_value);
          if (sw_enum_values.count(front_value) == 0) {
            reject_args(Diag::case_not_in_enumeration, c->position, {front_value, sw_enum_name});
            sw_rule1_failed = true;
            continue;
          }
          sw_case_values.insert(front_value);
        }
        // The value is encoded by its original expression, like every folded constant (0010 item 12): `case 1 << 40` is a
        // shift error at the label, not the 0 that the front end folded it to.
        const a_source_position saved_stmt_pos = cx_.stmt_pos;
        if (c->position.seq != 0) cx_.stmt_pos = c->position;
        cx_.case_value = true;
        cx_.case_var.clear();
        // A case value is converted like the controlling expression (decision 0018 item 6 (ii)): `case DEF:` with a `const
        // enum color DEF` in C++ converts the constant to the promoted type of the condition.
        const Expr* v = nullptr;
        if (c_enum_switch) {
          v = enum_case_value(en->case_value, c->position, enum_scrut_type, promoted);
        } else {
          if (!is_enumerator(en->case_value) && (en->case_value->expr != nullptr || en->case_value->local_expr_ref))
            allow_enum_conversion(expr_node_from_constant(en->case_value));
          v = constant_value(en->case_value, c->position);
        }
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
      // Before the first label nothing runs, but a declaration without an initializer puts its variable in scope in every case
      // (decision 0019 item 6, P28); any initialized declarator there stays rejected.
      if (!seen_label && !(c->kind == stmk_empty && c->attributes == nullptr) && !is_uninitialized_declaration(c)) {
        reject(Diag::unsupported_statement, c->position, "a statement before the first case label of a switch (it is unreachable)");
        ok = false;
        continue;
      }
      if (seen_label && skipped == nullptr && initialized_variable(c) != nullptr) skipped = c;
      statement(c, inner);
    }
    cx_.targets.pop_back();
    // Rule 2, judged only when every case could be read (the first enumerator in declaration order with no case).
    if (sw_enum != nullptr && ok && !sw_has_default) {
      for (const auto& en : sw_enumerators)
        if (sw_case_values.count(en.second) == 0) {
          reject_args(Diag::enumerator_not_handled, s->position, {en.first, sw_enum_name});
          sw_rule1_failed = true;
          break;
        }
    }
    if (sw_rule1_failed) ok = false;
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
        check_condition(s->expr);
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
        check_condition(s->expr);
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
        check_condition(s->expr);
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
        if (s->expr != nullptr) check_condition(s->expr);
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
    if (reported_count() == reported_before) {
      report_goto_problems(f);
      report_sequence_problems(f);
    }
    cx_ = Ctx();
  }

  void declaration_level_checks(const IlEntities& ents);
  const EnumInfo& enum_info(a_type_ptr t);

  // ---- enumeration operands and switches (decision 0019 items 10-11) ----
  // The enumeration type entry of a type: the enumeration itself or, for the `int` of a C enumerator, the type it is affiliated
  // with (il_def.h:10159-10166); null when the type is not an enumeration.
  static a_type_ptr enumeration_entry(a_type_ptr t) {
    if (t == nullptr) return nullptr;
    a_type_ptr ct = skip_typerefs(t);
    if (ct == nullptr || ct->kind != tk_integer) return nullptr;
    a_type_ptr et = ct->variant.integer.enum_type ? ct : ct->variant.integer.enum_info.affiliated_type;
    if (et != nullptr) et = skip_typerefs(et);
    return (et != nullptr && et->kind == tk_integer && et->variant.integer.enum_type) ? et : nullptr;
  }

  // An operand that is a value of an enumeration through nothing but the front end's own implicit conversions (generated casts
  // and the original expression of a folded constant that is one): the node left after them or the enumerator, the enumeration
  // and the type of every node passed.  False when it is anything else (fail closed).
  struct EnumOperand {
    an_expr_node_ptr inner = nullptr;     // the value (null when an enumerator)
    a_constant_ptr enumerator = nullptr;  // the enumerator (null otherwise)
    a_type_ptr entry = nullptr;           // its enumeration
    std::vector<a_type_ptr> via;          // the types of the implicit conversions passed, outermost first
  };
  static bool enumeration_operand(an_expr_node_ptr n, EnumOperand* out) {
    std::vector<a_type_ptr> via;
    for (int guard = 0; n != nullptr && guard < 16; ++guard) {
      if (n->kind == enk_constant) {
        a_constant_ptr c = n->variant.constant.ptr;
        if (is_enumerator(c)) {
          via.push_back(n->type);
          out->enumerator = c;
          out->entry = enumeration_entry(c->type);
          out->via = via;
          return out->entry != nullptr;
        }
        // A named constant of the enumeration type (`const enum color k = RED;` folded to a constant of that type) is a value of
        // the enumeration; it is encoded like any constant (its initializer is checked in the prologue, decision 0017).
        if (n->type != nullptr) {
          a_type_ptr nt = skip_typerefs(n->type);
          if (nt != nullptr && nt->kind == tk_integer && nt->variant.integer.enum_type) break;
        }
        // A folded constant (C: `const unsigned 1U {expr = cast(GREEN)}`): only when its original is itself an implicit
        // conversion or a constant; a variable's value or any computation is not an enumerator.
        if (c != nullptr && c->expr != nullptr && !c->local_expr_ref) {
          an_expr_node_ptr o = expr_node_from_constant(c);
          if (o != nullptr && o != n &&
              (o->kind == enk_constant ||
               (o->kind == enk_operation && o->variant.operation.kind == eok_cast && o->compiler_generated))) {
            via.push_back(n->type);
            n = o;
            continue;
          }
        }
        return false;
      }
      if (n->kind == enk_operation && n->variant.operation.kind == eok_cast && n->compiler_generated) {
        an_expr_node_ptr o = n->variant.operation.operands;
        if (o == nullptr || o->next != nullptr) return false;
        via.push_back(n->type);
        n = o;
        continue;
      }
      break;
    }
    if (n == nullptr) return false;
    a_type_ptr t = n->type != nullptr ? skip_typerefs(n->type) : nullptr;
    if (t == nullptr || t->kind != tk_integer || !t->variant.integer.enum_type) return false;
    out->inner = n;
    out->entry = t;
    out->via = via;
    return true;
  }

  // The literal of an enumerator in its enumeration's type.
  const Expr* enumerator_literal(a_constant_ptr c, a_type_ptr entry, const a_source_position& pos) {
    if (c->kind != ck_integer || c->source_corresp.decl_position.seq == 0 || !in_verified_file(c->source_corresp.decl_position)) {
      reject(Diag::unsupported_expression, pos,
             "an enumerator of an enumeration declared in a system file (system files are used only through contracts, decision 0018)");
      return nullptr;
    }
    const EnumInfo& ei = enum_info(entry);
    if (!ei.ok) {
      if (!ei.silent) reject(Diag::unsupported_type, pos, ei.name);
      return nullptr;
    }
    return value_literal(ei.type, constant_text(c, ei.type), pos);
  }

  // `a < b`, `a <= b`, ...: 1 when the operands are values of one enumeration, or one such value and an enumerator of it (`*a`
  // and `*c` are then both values of the enumeration, and are compared as such: every value lies in the value range, which lies
  // in the type both operands are converted to, so the conversions keep each value and their order, 0019 item 10); 0 when that
  // is not the case (the ordinary path decides: it accepts what 0018 accepted and rejects the rest); -1 when it is, but cannot
  // be encoded.
  int ordered_enumeration_operands(an_expr_node_ptr e, const std::vector<an_expr_node_ptr>& ops, const Expr** a, const Expr** c) {
    EnumOperand lo_, ro_;
    if (!enumeration_operand(ops[0], &lo_) || !enumeration_operand(ops[1], &ro_)) return 0;
    if (lo_.entry != ro_.entry) return 0;
    // An order of two enumerators is what 0018 accepted as two literals: it keeps its path (and its meaning).
    if (lo_.enumerator != nullptr && ro_.enumerator != nullptr) return 0;
    const EnumInfo& ei = enum_info(lo_.entry);
    if (!ei.ok) {
      if (!ei.silent) reject(Diag::unsupported_type, e->position, ei.name);
      return -1;
    }
    // Both operands are converted to one type that holds every value of the enumeration (fail closed on anything else).
    VType ta = vtype(ops[0]->type);
    VType tb = vtype(ops[1]->type);
    if (!ta.ok || !tb.ok || ta.t != tb.t || !ta.t.is_int()) return 0;
    const vir::dec::Range value_range = vir::dec::range_of(*ei.type.int_type);
    for (const EnumOperand* o : {&lo_, &ro_})
      for (a_type_ptr via : o->via) {
        VType tv = vtype(via);
        if (!tv.ok || !tv.t.is_int() || !vir::dec::contains(vir::dec::range_of(*tv.t.int_type), value_range)) return 0;
      }
    if (!vir::dec::contains(vir::dec::range_of(*ta.t.int_type), value_range)) return 0;
    const Expr* x[2] = {nullptr, nullptr};
    const EnumOperand* o[2] = {&lo_, &ro_};
    for (int i = 0; i < 2; ++i) {
      if (o[i]->enumerator != nullptr) {
        x[i] = enumerator_literal(o[i]->enumerator, o[i]->entry, ops[i]->position);
      } else {
        x[i] = expr(o[i]->inner);
        if (x[i] != nullptr && x[i]->type != ei.type) {
          reject(Diag::unsupported_expression, e->position,
                 "a comparison of order whose operand has another type than the enumeration (decision 0019 item 10)");
          x[i] = nullptr;
        }
      }
      if (x[i] == nullptr) return -1;
    }
    *a = x[0];
    *c = x[1];
    return 1;
  }

  // The node left after the front end's own implicit conversions: compiler-generated `eok_cast`s and the original expression of a
  // folded constant (never an explicit cast, never an enumerator, whose value has no expression).  Null on a malformed node.
  static an_expr_node_ptr peel_implicit(an_expr_node_ptr n) {
    for (int guard = 0; n != nullptr && guard < 16; ++guard) {
      if (n->kind == enk_operation && n->variant.operation.kind == eok_cast && n->compiler_generated) {
        an_expr_node_ptr o = n->variant.operation.operands;
        if (o == nullptr || o->next != nullptr) return nullptr;
        n = o;
      } else if (n->kind == enk_constant && n->variant.constant.ptr != nullptr && !is_enumerator(n->variant.constant.ptr) &&
                 n->variant.constant.ptr->expr != nullptr && !n->variant.constant.ptr->local_expr_ref) {
        an_expr_node_ptr o = expr_node_from_constant(n->variant.constant.ptr);
        if (o == nullptr || o == n) return n;
        n = o;
      } else {
        return n;
      }
    }
    return nullptr;
  }

  // Whether `n` is a truth value: 0 or 1, never a negative number (decision 0023 item 7, one verdict in both modes).  After the
  // implicit conversions and folded constants' original expressions are peeled (an explicit cast is never peeled, so `(int)b` is
  // not one, and an `int` constant is not one): a comparison, `&&`, `||`, `!` or a conversion to `bool`; a node of type `bool`
  // (in C++ every comparison has it; in C `x ? true : false` has `bool` arms); a `?:` whose two arms are truth values (the
  // condition does not matter); a comma whose right operand is one.
  bool truth_value(an_expr_node_ptr n, int depth = 0) {
    n = peel_implicit(n);
    if (n == nullptr || n->type == nullptr || depth > 16) return false;
    VType t = vtype(n->type);
    if (t.ok && t.t.is_bool()) return true;
    if (n->kind != enk_operation) return false;
    switch (n->variant.operation.kind) {
      case eok_lt: case eok_le: case eok_gt: case eok_ge: case eok_eq: case eok_ne:
      case eok_land: case eok_lor: case eok_not: case eok_bool_cast:
        return true;
      case eok_question: {
        an_expr_node_ptr c = n->variant.operation.operands;
        if (c == nullptr || c->next == nullptr || c->next->next == nullptr) return false;
        return truth_value(c->next, depth + 1) && truth_value(c->next->next, depth + 1);
      }
      case eok_comma: {
        an_expr_node_ptr r = n->variant.operation.operands;
        if (r == nullptr || r->next == nullptr) return false;
        while (r->next != nullptr) r = r->next;
        return truth_value(r, depth + 1);
      }
      default:
        return false;
    }
  }

  // C only (decision 0023 item 7): the `?:` node, if any, that C++ would reject with 0018 item 8's message, reached through the
  // implicit conversions and folded constants' original expressions: a `?:` whose two arms are enumerators of one enumeration
  // (each through its implicit conversions: the front end marks the enumerator constant, also inside a folded `?:`'s expression;
  // P40).  An arm that is itself such a `?:` is searched first, as C++ evaluates the arms first.  C types this `?:` as `int`, so
  // the operand does not look like an enumeration value, while C++ gives it the enumeration's type and rejects it.
  an_expr_node_ptr enumerator_conditional(an_expr_node_ptr n, int depth = 0) {
    n = peel_implicit(n);
    if (n == nullptr || depth > 8 || n->kind != enk_operation || n->variant.operation.kind != eok_question) return nullptr;
    an_expr_node_ptr c = n->variant.operation.operands;
    if (c == nullptr || c->next == nullptr || c->next->next == nullptr) return nullptr;
    an_expr_node_ptr arm[2] = {c->next, c->next->next};
    EnumOperand eo[2];
    for (int i = 0; i < 2; ++i) {
      if (an_expr_node_ptr inner = enumerator_conditional(arm[i], depth + 1)) return inner;
    }
    for (int i = 0; i < 2; ++i) {
      if (enumeration_operand(arm[i], &eo[i]) && eo[i].enumerator != nullptr) continue;
      if (!comma_ending_in_enumerator(arm[i], &eo[i])) return nullptr;  // an arm `(y++, RED)` (item 7, audit round 1)
    }
    return eo[0].entry == eo[1].entry ? n : nullptr;
  }
  bool reject_enumerator_conditional(const std::vector<an_expr_node_ptr>& ops) {
    if (il_header.source_language != sl_C) return false;
    for (an_expr_node_ptr o : ops) {
      if (an_expr_node_ptr q = enumerator_conditional(o)) {
        reject(Diag::unsupported_expression, q->position, "a conditional expression of enumeration type (decision 0018 item 8)");
        return true;
      }
    }
    return false;
  }

  // ---- the bitwise operators and the shapes the compilers warn about (decision 0023 items 3-5; P41) ----
  // Each detector is a rejection with its own tag at the operator.  They read the IL as P40 found it: through the front end's
  // generated casts (peel_implicit, peel_generated_conversions; an explicit cast is a node and a barrier), the flag
  // `is_parenthesized` on the innermost written operation (also inside a folded constant's original expression), and folded
  // constants' original expressions, so a shape EDG folded (`2 ^ 32`, `5 & 3 == 3`) is found where the encoder reads it.  The
  // compilers see nothing in an annotation (the macros expand to nothing) and change their verdicts inside macros; cerastes
  // applies the rules everywhere (the broader shape).
  // The nodes a detector rejected: the operation's own rejection stays silent for them (one diagnostic per shape).
  std::set<an_expr_node_ptr> detected_;
  bool detect(Diag d, an_expr_node_ptr n) {
    detected_.insert(n);
    reject(d, n->position, "");
    return true;
  }

  static bool is_comparison_kind(an_expr_operator_kind k) {
    return k == eok_lt || k == eok_le || k == eok_gt || k == eok_ge || k == eok_eq || k == eok_ne;
  }
  static bool is_bitwise_binary_kind(an_expr_operator_kind k) { return k == eok_and || k == eok_or || k == eok_xor; }
  // The innermost written operation of an operand (null when it is a variable, a literal or a constant without an expression).
  static an_expr_node_ptr written_operation(an_expr_node_ptr n) {
    n = peel_implicit(n);
    return n != nullptr && n->kind == enk_operation ? n : nullptr;
  }
  // As peel_implicit, and the front end's generated conversions to `bool` too (a C++ condition is one).
  static an_expr_node_ptr peel_condition(an_expr_node_ptr n) {
    for (int guard = 0; n != nullptr && guard < 16; ++guard) {
      n = peel_implicit(n);
      if (n != nullptr && n->kind == enk_operation && n->variant.operation.kind == eok_bool_cast && n->compiler_generated &&
          n->variant.operation.operands != nullptr && n->variant.operation.operands->next == nullptr) {
        n = n->variant.operation.operands;
        continue;
      }
      return n;
    }
    return nullptr;
  }
  // The integer constant an operand is, through generated conversions only (the converted value, as the compilers evaluate it).
  static a_constant_ptr integer_constant_operand(an_expr_node_ptr n) {
    n = peel_generated_conversions(n);
    if (n == nullptr || n->kind != enk_constant || n->variant.constant.ptr == nullptr || n->variant.constant.ptr->kind != ck_integer)
      return nullptr;
    return n->variant.constant.ptr;
  }
  // The value of an integer constant as a 128-bit number; false when it does not fit (the rule then does not apply).
  static bool constant_int128(a_constant_ptr c, __int128* out) {
    if (c == nullptr) return false;
    const std::string s = decimal_value(c);
    size_t i = 0;
    bool neg = false;
    if (i < s.size() && (s[i] == '-' || s[i] == '+')) neg = s[i++] == '-';
    if (i >= s.size() || s.size() - i > 36) return false;
    __int128 v = 0;
    for (; i < s.size(); ++i) {
      if (s[i] < '0' || s[i] > '9') return false;
      v = v * 10 + (s[i] - '0');
    }
    *out = neg ? -v : v;
    return true;
  }
  // A non-constant operand (after generated conversions): the `x` of `x | C`.
  static bool is_variable_operand(an_expr_node_ptr n) {
    n = peel_generated_conversions(n);
    return n != nullptr && n->kind != enk_constant;
  }

  // `-Wparentheses`, `-Wbitwise-op-parentheses` (item 5): an unparenthesized operand of binary `&`, `|` or `^` that is a comparison,
  // a `+` or `-`, or a different bitwise operator.
  bool detect_operand_parentheses(an_expr_node_ptr e, const std::vector<an_expr_node_ptr>& ops) {
    const an_expr_operator_kind k = e->variant.operation.kind;
    for (an_expr_node_ptr o : ops) {
      an_expr_node_ptr n = written_operation(o);
      if (n == nullptr || n->is_parenthesized) continue;
      const an_expr_operator_kind nk = n->variant.operation.kind;
      if (is_comparison_kind(nk) || nk == eok_add || nk == eok_subtract || (is_bitwise_binary_kind(nk) && nk != k))
        return detect(Diag::bitwise_operand_parentheses, e);
    }
    return false;
  }
  // `-Wlogical-not-parentheses`, `-Wparentheses` (item 5): `!e` as the left operand of `&` or `|` (never `^`), unless the right
  // operand is a truth value or, for `|` only, the literal 0 or 1.
  bool detect_logical_not(an_expr_node_ptr e, const std::vector<an_expr_node_ptr>& ops) {
    const an_expr_operator_kind k = e->variant.operation.kind;
    if (k != eok_and && k != eok_or) return false;
    an_expr_node_ptr l = written_operation(ops[0]);
    if (l == nullptr || l->variant.operation.kind != eok_not || l->is_parenthesized) return false;
    if (truth_value(ops[1])) return false;
    if (k == eok_or) {
      // GCC is silent on the literal 0 or 1 (by value, whatever the suffix: `1L`), and every compiler on an explicit cast of a
      // comparison (`!x | (int)(y > 0)`); clang warns about that cast only for `&`.
      __int128 v = 0;
      if (constant_int128(integer_constant_operand(ops[1]), &v) && (v == 0 || v == 1)) return false;
      an_expr_node_ptr r = peel_generated_conversions(ops[1]);
      if (r != nullptr && r->kind == enk_operation && r->variant.operation.kind == eok_cast && !r->compiler_generated &&
          r->variant.operation.operands != nullptr && r->variant.operation.operands->next == nullptr) {
        // GCC exempts the cast of a comparison, `!`, `&&` or `||` by form; `(int)q` of a bool variable still warns (P41 `or_castq`).
        an_expr_node_ptr o = peel_generated_conversions(r->variant.operation.operands);
        if (o != nullptr && o->kind == enk_operation &&
            (is_comparison_kind(o->variant.operation.kind) || o->variant.operation.kind == eok_not ||
             o->variant.operation.kind == eok_land || o->variant.operation.kind == eok_lor))
          return false;
      }
    }
    return detect(Diag::logical_not_bitwise, e);
  }
  // `-Wbitwise-instead-of-logical` (item 5, clang): `&` or `|` (never `^`) whose two operands are truth values that both have a
  // side effect.
  bool detect_bitwise_instead_of_logical(an_expr_node_ptr e, const std::vector<an_expr_node_ptr>& ops) {
    const an_expr_operator_kind k = e->variant.operation.kind;
    if (k != eok_and && k != eok_or) return false;
    if (!truth_value(ops[0]) || !truth_value(ops[1])) return false;
    if (first_effect(ops[0]) == nullptr || first_effect(ops[1]) == nullptr) return false;
    return detect(Diag::bitwise_instead_of_logical, e);
  }
  // `-Wxor-used-as-pow` (item 5): `2 ^ n` with n in 0-64 and `10 ^ n` with n >= 0, both operands integer constants.  By value,
  // whatever the spelling (the IL keeps none): the broader shape.
  bool detect_xor_as_power(an_expr_node_ptr e, const std::vector<an_expr_node_ptr>& ops) {
    if (e->variant.operation.kind != eok_xor) return false;
    __int128 a = 0, n = 0;
    if (!constant_int128(integer_constant_operand(ops[0]), &a) || !constant_int128(integer_constant_operand(ops[1]), &n)) return false;
    if ((a == 2 && n >= 0 && n <= 64) || (a == 10 && n >= 0)) return detect(Diag::xor_as_power, e);
    return false;
  }
  // The detectors of the binary operators `&`, `|` and `^`, in a fixed order; true when one rejected.
  bool detect_bitwise_binary(an_expr_node_ptr e, const std::vector<an_expr_node_ptr>& ops) {
    return detect_xor_as_power(e, ops) || detect_operand_parentheses(e, ops) || detect_logical_not(e, ops) ||
           detect_bitwise_instead_of_logical(e, ops);
  }
  // `-Wbool-operation` (item 5): `~e` where `e` is a truth value, a `&`, `|` or `^` of two truth values (clang), or a unary `+` of
  // one.  An explicit cast of the operand is a barrier (peel_implicit stops at it).
  bool detect_complement_of_truth_value(an_expr_node_ptr e, const std::vector<an_expr_node_ptr>& ops) {
    if (truth_value(ops[0])) return detect(Diag::complement_of_truth_value, e);
    an_expr_node_ptr n = written_operation(ops[0]);
    if (n == nullptr) return false;
    auto nops = operands_of(n);
    const an_expr_operator_kind nk = n->variant.operation.kind;
    if (is_bitwise_binary_kind(nk) && nops.size() == 2 && truth_value(nops[0]) && truth_value(nops[1]))
      return detect(Diag::complement_of_truth_value, e);
    if (nk == eok_unary_plus && nops.size() == 1 && truth_value(nops[0])) return detect(Diag::complement_of_truth_value, e);
    return false;
  }
  // `-Wbitwise-conditional-parentheses` (item 5, clang): the condition of a `?:` is an unparenthesized `&` or `|` whose right
  // operand is a truth value.
  bool detect_bitwise_condition(an_expr_node_ptr cond) {
    an_expr_node_ptr n = peel_condition(cond);
    if (n == nullptr || n->kind != enk_operation || n->is_parenthesized) return false;
    const an_expr_operator_kind k = n->variant.operation.kind;
    if (k != eok_and && k != eok_or) return false;
    auto nops = operands_of(n);
    if (nops.size() != 2 || !truth_value(nops[1])) return false;
    return detect(Diag::bitwise_condition_parentheses, n);
  }
  // `-Wtautological-compare`, `-Wtautological-bitwise-compare` (item 5), in `==` and `!=`: `(x & C1) == C2` with `C2 & ~C1 != 0`,
  // and `(x | C1) == C2` with `C1 & ~C2 != 0`, either operand order, any operand type (GCC's union).  Both operands of the
  // bitwise operation constant (a folded constant) is not the shape.
  bool detect_tautological_compare(an_expr_node_ptr e, const std::vector<an_expr_node_ptr>& ops) {
    for (int i = 0; i < 2; ++i) {
      an_expr_node_ptr w = peel_generated_conversions(ops[i]);
      if (w == nullptr || w->kind != enk_operation) continue;
      const an_expr_operator_kind k = w->variant.operation.kind;
      if (k != eok_and && k != eok_or) continue;
      auto wops = operands_of(w);
      if (wops.size() != 2) continue;
      __int128 c2 = 0, c1 = 0;
      if (!constant_int128(integer_constant_operand(ops[1 - i]), &c2)) continue;
      int side = -1;  // the operand of the bitwise operation that is the constant C1
      if (constant_int128(integer_constant_operand(wops[1]), &c1) && is_variable_operand(wops[0])) side = 1;
      else if (constant_int128(integer_constant_operand(wops[0]), &c1) && is_variable_operand(wops[1])) side = 0;
      if (side < 0) continue;
      const bool never = k == eok_and ? ((c2 & ~c1) != 0) : ((c1 & ~c2) != 0);
      if (never) return detect(Diag::tautological_bitwise_compare, e);
    }
    return false;
  }
  // An unsigned integer type of rank below `int` that is not `bool` (`unsigned char`, `unsigned short`): the operand type of clause 6c.
  bool narrow_unsigned(an_expr_node_ptr n) {
    if (n == nullptr || n->type == nullptr) return false;
    VType t = vtype(n->type);
    return t.ok && t.t.is_int() && !is_enum_type(t.t) && !t.t.int_type->is_signed && t.t.int_type->bits < prog_.c_int_bits();
  }
  // `-Wsign-compare`, "comparison of promoted bitwise complement of an unsigned value with constant" (GCC; `-Wextra` in C, `-Wall` in
  // C++; decision 0023 item 5, clause 6c, audit round 1): a comparison (any of the six, either order) of `~e`, where `e` has an
  // unsigned type of rank below `int` (an explicit cast to `int` is looked through, `~(int)uc`; `(unsigned char)~a` is not this
  // shape), with a constant >= 0 in any spelling or a value promoted from such a type: `~uc == 0x0F` is always false, since `~uc` is a
  // negative `int`.  A negative constant, a non-constant `int`, `~u`, `~ul`, `~sc` and `~uc & 0xFF` are silent.
  bool detect_promoted_complement_compare(an_expr_node_ptr e, const std::vector<an_expr_node_ptr>& ops) {
    for (int i = 0; i < 2; ++i) {
      an_expr_node_ptr w = peel_generated_conversions(ops[i]);
      if (w == nullptr || w->kind != enk_operation || w->variant.operation.kind != eok_complement) continue;
      auto wops = operands_of(w);
      if (wops.size() != 1) continue;
      an_expr_node_ptr n = peel_generated_conversions(wops[0]);
      for (int guard = 0; n != nullptr && guard < 8 && n->kind == enk_operation && n->variant.operation.kind == eok_cast &&
                          !n->compiler_generated && n->variant.operation.operands != nullptr && n->variant.operation.operands->next == nullptr;
           ++guard) {
        VType to = vtype(n->type);
        if (!to.ok || !to.t.is_int() || to.t.int_type->is_signed == false || to.t.int_type->bits != prog_.c_int_bits()) break;  // only `(int)`
        n = peel_generated_conversions(n->variant.operation.operands);
      }
      if (!narrow_unsigned(n)) continue;
      __int128 c = 0;
      bool hit = false;
      if (a_constant_ptr k = integer_constant_operand(ops[1 - i])) hit = constant_int128(k, &c) && c >= 0;
      else hit = narrow_unsigned(peel_generated_conversions(ops[1 - i]));
      if (hit) return detect(Diag::tautological_bitwise_compare, e);
    }
    return false;
  }
  // The same warning for `x | C` (or `C | x`) with a nonzero constant used as a truth value: the operand of an `if`, `while`, `do`
  // or `for` condition, of a `?:` condition, of `!`, `&&`, `||` or a conversion to `bool` (clang).
  void detect_or_as_truth_value(an_expr_node_ptr operand) {
    an_expr_node_ptr n = peel_generated_conversions(operand);
    if (n == nullptr || n->kind != enk_operation || n->variant.operation.kind != eok_or) return;
    auto nops = operands_of(n);
    if (nops.size() != 2) return;
    __int128 c = 0;
    const bool left_const = constant_int128(integer_constant_operand(nops[0]), &c) && is_variable_operand(nops[1]);
    const bool right_const = !left_const && constant_int128(integer_constant_operand(nops[1]), &c) && is_variable_operand(nops[0]);
    if ((left_const || right_const) && c != 0) detect(Diag::tautological_bitwise_compare, n);
  }

  // The operators themselves (decision 0023 items 1, 3, 4): after the detectors, the operands are encoded and an enumeration
  // operand is rejected as in arithmetic; the front end has promoted the operands to the computed type (P40), so the VIR operator
  // has no obligations of its own.
  const Expr* bitwise_binary(an_expr_node_ptr e, const std::vector<an_expr_node_ptr>& ops) {
    if (!operand_count(e, 2, ops)) return nullptr;
    if (detected_.count(e) != 0 || detect_bitwise_binary(e, ops)) return nullptr;
    // Item 4: enumerators (or values) of two different enumerations (`RED | NA`; C++20 deprecates the conversion, and C and C++
    // give the operands different types): rejected in both modes.  An enumerator is otherwise an `int` constant, as in arithmetic.
    EnumOperand eo[2];
    if (enumeration_operand(ops[0], &eo[0]) && enumeration_operand(ops[1], &eo[1]) && eo[0].entry != eo[1].entry) {
      reject(Diag::unsupported_expression, e->position,
             "a bitwise operator on values of two different enumerations (decision 0023 item 4): convert both sides explicitly, "
             "for example `(int)RED | (int)NA`");
      return nullptr;
    }
    const Expr* a = expr(ops[0]);
    const Expr* c = expr(ops[1]);
    if (a == nullptr || c == nullptr) return nullptr;
    if (is_enum_type(a->type) || is_enum_type(c->type)) {  // item 4: as arithmetic (0018 item 6)
      reject(Diag::unsupported_expression, e->position, "a bitwise operator on an enumeration value (decision 0023 item 4)");
      return nullptr;
    }
    const an_expr_operator_kind k = e->variant.operation.kind;
    return finish(e, b_.binary(k == eok_and ? vir::BinaryOp::BitAnd : k == eok_or ? vir::BinaryOp::BitOr : vir::BinaryOp::BitXor, a, c,
                               loc_of(e->position)));
  }
  const Expr* bitwise_complement(an_expr_node_ptr e, const std::vector<an_expr_node_ptr>& ops) {
    if (!operand_count(e, 1, ops)) return nullptr;
    if (detected_.count(e) != 0 || detect_complement_of_truth_value(e, ops)) return nullptr;
    const Expr* a = expr(ops[0]);
    if (a == nullptr) return nullptr;
    if (is_enum_type(a->type)) {  // item 4, including C's bare enumeration operand (P26, P40)
      reject(Diag::unsupported_expression, e->position, "a bitwise operator on an enumeration value (decision 0023 item 4)");
      return nullptr;
    }
    return finish(e, b_.unary(vir::UnaryOp::BitNot, a, loc_of(e->position)));
  }

  // C only (decision 0023 item 7, audit round 1): a comma whose right operand is an enumerator.  C++ types `(x++, GREEN)` as the
  // enumeration and C as `int`; for `==` and `!=` in C it counts as a value of the enumerator's enumeration, as in C++.
  static bool comma_ending_in_enumerator(an_expr_node_ptr n, EnumOperand* out) {
    an_expr_node_ptr m = peel_implicit(n);
    if (m == nullptr || m->kind != enk_operation || m->variant.operation.kind != eok_comma) return false;
    an_expr_node_ptr r = m->variant.operation.operands;
    if (r == nullptr || r->next == nullptr) return false;
    while (r->next != nullptr) r = r->next;
    EnumOperand eo;
    if (!enumeration_operand(r, &eo) || eo.enumerator == nullptr) {
      // a nested comma `(a, b, GREEN)`: its right operand is itself one
      return comma_ending_in_enumerator(r, out);
    }
    *out = EnumOperand();
    out->inner = m;
    out->entry = eo.entry;
    return true;
  }
  static bool sign_compare_enum_operand(an_expr_node_ptr n, EnumOperand* out) {
    if (enumeration_operand(n, out)) return true;
    return il_header.source_language == sl_C && comma_ending_in_enumerator(n, out);
  }

  // `c == x` and `c != x` where `c` is a value of an enumeration that C compares in an unsigned type and `x` is a signed integer that
  // may be negative.  In C, EDG computes it as `eq[int](cast[unsigned int](c), cast[unsigned int](x))` (P26): a negative `x` would
  // convert to a large unsigned value, which is why both compilers warn (`-Wsign-compare`, probe P39).  Decision 0022 item 5: it is
  // rejected, in both modes (in C++ the same shape is `cast[int](c) == x`, or `unsigned int` for a fixed unsigned base; the verdict
  // does not depend on the mode).  The rule:
  //  - one operand is a value of an enumeration E (as `enumeration_operand` reads it), not an enumerator, the other is not one;
  //  - E has no negative value and C's promoted type of E is an unsigned type of rank int or above (an unfixed E without negative
  //    enumerators, or a fixed base `unsigned`, `unsigned long`, ...; not `unsigned char`/`unsigned short`, which promote to `int`);
  //  - the other operand's own type (through the front end's generated casts and a folded constant's original expression) is a signed
  //    integer type that is not an enumeration and not `bool`, which C converts to E's unsigned type (promoted width <= E's width:
  //    a wider `long` makes C compare in `long`);
  //  - and it is not an integer constant whose value is >= 0.
  // Returns true when the shape was rejected.
  bool reject_enumeration_sign_compare(an_expr_node_ptr e, const std::vector<an_expr_node_ptr>& ops) {
    if (ops.size() != 2 || ops[0] == nullptr || ops[1] == nullptr) return false;
    EnumOperand eo[2];
    const bool is_enum[2] = {sign_compare_enum_operand(ops[0], &eo[0]), sign_compare_enum_operand(ops[1], &eo[1])};
    if (is_enum[0] && is_enum[1]) {
      // Two values or enumerators of DIFFERENT enumerations (in C an enumerator is an `int`, found through the front end's enumerator
      // mark): C and C++ give different meanings (`unsigned int` against `int` in C, `int` in C++), and every compiler warns
      // (-Wenum-compare): rejected in both modes.  Two values of one enumeration stay accepted.
      if (eo[0].entry == eo[1].entry) return false;
      const EnumInfo& i0 = enum_info(eo[0].entry);
      const EnumInfo& i1 = enum_info(eo[1].entry);
      if (!i0.ok || !i1.ok) return false;  // the ordinary path reports it
      reject(Diag::unsupported_expression, e->position,
             "an equality between values of two different enumerations (`" + i0.type.int_type->name + "` and `" + i1.type.int_type->name +
                 "`): the compilers warn (-Wenum-compare); convert both sides explicitly, for example `(int)c == (int)n` (decision 0022 item 5)");
      return true;
    }
    if (!is_enum[0] && !is_enum[1]) return false;
    const int ei_side = is_enum[0] ? 0 : 1;
    if (eo[ei_side].enumerator != nullptr || eo[ei_side].inner == nullptr) return false;
    const EnumInfo& info = enum_info(eo[ei_side].entry);
    if (!info.ok) return false;  // the ordinary path reports it
    const vir::IntType* et = info.type.int_type;
    if (et == nullptr || vir::dec::negative(vir::dec::range_of(*et).lo)) return false;
    // The unsigned type C compares E's values in.  A fixed base is its own (an unsigned type of rank int or above).  An unfixed
    // enumeration without negative enumerators has the compatible type `unsigned int` in C, whatever the front end calls its
    // underlying type in C++ (there `int` with the "use unsigned" flag, P26): one verdict in both modes.
    const a_type_ptr entry = eo[ei_side].entry;
    const bool fixed = entry->variant.integer.has_explicit_enum_base || entry->variant.integer.is_scoped_enum;
    const int ubits = fixed ? et->bits : prog_.c_int_bits();
    if (fixed && (et->is_signed || et->bits < prog_.c_int_bits())) return false;
    // The other operand's own type, and whether it is a constant that is not negative.  `last_const` is the innermost constant on the
    // path, whose sign decides: every implicit conversion between it and the operand either preserves its value or lands on an unsigned
    // type (EDG's `const unsigned 4294967295U {expr = cast(const int -1)}`), which the rule exempts, so the innermost value is the one
    // the program wrote.
    an_expr_node_ptr n = ops[1 - ei_side];
    a_constant_ptr last_const = nullptr;
    for (int guard = 0; n != nullptr && guard < 16; ++guard) {
      if (n->kind == enk_constant && n->variant.constant.ptr != nullptr && n->variant.constant.ptr->kind == ck_integer) {
        a_constant_ptr c = n->variant.constant.ptr;
        last_const = c;
        if (c->expr != nullptr && !c->local_expr_ref) {
          an_expr_node_ptr o = expr_node_from_constant(c);
          if (o != nullptr && o != n) {
            n = o;
            continue;
          }
        }
        break;
      }
      if (n->kind == enk_operation && n->variant.operation.kind == eok_cast && n->compiler_generated) {
        an_expr_node_ptr o = n->variant.operation.operands;
        if (o == nullptr || o->next != nullptr) return false;
        n = o;
        continue;
      }
      break;
    }
    if (n == nullptr || n->type == nullptr) return false;
    // A truth value is 0 or 1 whatever its type (`int` in C, `bool` in C++): never negative, so no mode-dependent verdict.
    if (truth_value(n)) return false;
    VType ot = vtype(n->type);
    if (!ot.ok || !ot.t.is_int() || is_enum_type(ot.t) || !ot.t.int_type->is_signed) return false;
    if (std::max(ot.t.int_type->bits, prog_.c_int_bits()) > ubits) return false;
    if (last_const != nullptr && !vir::dec::negative(decimal_value(last_const))) return false;  // a constant >= 0 (record 0022 item 4)
    const std::string promoted = ubits == prog_.c_int_bits() ? "unsigned int" : "an unsigned " + std::to_string(ubits) + "-bit integer type";
    reject(Diag::unsupported_expression, e->position,
           "an equality between a value of `" + et->name + "` and a signed integer that may be negative: C compares it as `" + promoted +
               "` (the compilers warn, -Wsign-compare); convert one side explicitly, for example `(int)c == x` (decision 0022 item 5)");
    return true;
  }

  // The enumeration of the controlling expression of a switch (decision 0019 item 11), or null: a value of an enumeration (a
  // variable, parameter, call, cast or named constant), an enumerator (an `int` constant in C), or a `?:` whose arms are values
  // of one enumeration, each through the front end's own implicit conversions only.  `switch ((int)c)` is not one.
  static a_type_ptr switch_enumeration(an_expr_node_ptr n, int depth = 0) {
    if (n == nullptr || depth > 4) return nullptr;
    // Implicit conversions around a `?:` (C promotes its arms and C++ converts the whole): looked through first.
    an_expr_node_ptr m = n;
    for (int guard = 0; m != nullptr && guard < 16; ++guard) {
      if (m->kind == enk_operation && m->variant.operation.kind == eok_cast && m->compiler_generated &&
          m->variant.operation.operands != nullptr && m->variant.operation.operands->next == nullptr) {
        m = m->variant.operation.operands;
      } else if (m->kind == enk_constant && m->variant.constant.ptr != nullptr && !is_enumerator(m->variant.constant.ptr) &&
                 m->variant.constant.ptr->expr != nullptr && !m->variant.constant.ptr->local_expr_ref) {
        an_expr_node_ptr o = expr_node_from_constant(m->variant.constant.ptr);
        if (o == nullptr || o == m) break;
        m = o;
      } else {
        break;
      }
    }
    if (m != nullptr && m->kind == enk_operation && m->variant.operation.kind == eok_question) {
      an_expr_node_ptr cnd = m->variant.operation.operands;
      if (cnd == nullptr || cnd->next == nullptr || cnd->next->next == nullptr) return nullptr;
      a_type_ptr x = switch_enumeration(cnd->next, depth + 1);
      a_type_ptr y = switch_enumeration(cnd->next->next, depth + 1);
      return (x != nullptr && x == y) ? x : nullptr;
    }
    EnumOperand eo;
    return enumeration_operand(n, &eo) ? eo.entry : nullptr;
  }

  // The decimal value of an integer constant.
  static std::string decimal_value(a_constant_ptr c) { return std::string(decimal_str_for_integer_constant(c).as_temp_characters()); }

  // ---- enumerations (decision 0018) ----
  // An enumerator: the constant that defines it (il_def.h:4930, is_named_constant_definition).  Its uses are this constant
  // (decision 0018 item 9; the rule is confirmed by probe P25).
  static bool is_enumerator(a_constant_ptr c) { return c != nullptr && c->is_named_constant_definition; }
  static bool is_enumerator_node(an_expr_node_ptr e) {
    return e != nullptr && e->kind == enk_constant && is_enumerator(e->variant.constant.ptr);
  }
  // Whether the enumerator was written with `= value` (il_def.h:1979: the value range is null when there is none).
  static bool has_written_initializer(a_constant_ptr c) {
    a_decl_position_supplement_ptr x = c->source_corresp.decl_pos_info;
    if (x != nullptr) return x->variant.enum_value_range.start.seq != 0;
    return c->expr != nullptr || c->local_expr_ref;
  }
  // The enumerators of an enumeration type entry, in declaration order (il_def.h:10150-10159: an unscoped enumeration's
  // list, a scoped one's scope).
  static std::vector<a_constant_ptr> enumerators_of(a_type_ptr t) {
    const auto& i = t->variant.integer;
    std::vector<a_constant_ptr> out;
    a_constant_ptr c = nullptr;
    if (i.is_scoped_enum) c = i.enum_info.assoc_scope != nullptr ? i.enum_info.assoc_scope->constants : nullptr;
    else c = i.enum_info.constant_list;
    for (; c != nullptr; c = c->next) out.push_back(c);
    return out;
  }
  // a + 1 for a decimal integer.
  static std::string dec_inc(const std::string& a) {
    if (vir::dec::negative(a)) return vir::dec::neg(vir::dec::dec1(a.substr(1)));
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
  }
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

const char* Encoder::computed_type_reason(a_type_ptr t, std::set<a_type_ptr>& seen, int depth) {
  if (t == nullptr) return nullptr;
  if (depth > 64) return too_deep_reason();
  if (const char* r = typeref_chain_reason(t, seen, depth)) return r;
  t = skip_typerefs(t);
  if (!seen.insert(t).second) return nullptr;  // a cycle, or a type already examined
  switch (t->kind) {
    case tk_array: return "an array type";
    case tk_vector: return "a vector type";
    case tk_integer:
      if (t->variant.integer.int_kind == ik_bit_precise || t->variant.integer.int_kind == ik_unsigned_bit_precise)
        return "a _BitInt type";
      return nullptr;
    case tk_pointer: return computed_type_reason(t->variant.pointer.type, seen, depth + 1);
    case tk_ptr_to_member: return computed_type_reason(t->variant.ptr_to_member.type, seen, depth + 1);
    case tk_routine: {
      if (const char* r = computed_type_reason(t->variant.routine.return_type, seen, depth + 1)) return r;
      if (a_routine_type_supplement_ptr x = t->variant.routine.extra_info)
        for (a_param_type_ptr p = x->param_type_list; p != nullptr; p = p->next) {
          if (const char* r = computed_type_reason(p->type, seen, depth + 1)) return r;
          if (const char* r = computed_type_reason(p->declared_type, seen, depth + 1)) return r;
        }
      return nullptr;
    }
    case tk_class:
    case tk_struct:
    case tk_union: return class_instance_reason(t, seen, depth + 1);
    default: return nullptr;
  }
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

// The VIR type of an enumeration type entry (decision 0018 items 2-3, 10), made once; every rejection of a declaration in a
// verified file is reported here, at the declaration, so the uses stay silent.
const Encoder::EnumInfo& Encoder::enum_info(a_type_ptr t) {
  auto found = enums_.find(t);
  if (found != enums_.end()) return found->second;
  EnumInfo& ei = enums_[t];
  const auto& i = t->variant.integer;
  const a_source_position p = t->source_corresp.decl_position;
  const std::string spelled = render_type(t);
  if (p.seq == 0 || !in_verified_file(p)) {
    ei.name = spelled + " (an enumeration declared in a system file; system files are used only through contracts)";
    return ei;  // reported at the use
  }
  ei.silent = true;
  Ctx saved_cx = std::move(cx_);  // a use may reach here in the middle of a function
  cx_ = Ctx();
  cx_.stmt_pos = p;
  bool ok = true;
  auto refuse = [&](const a_source_position& at, const std::string& why) {
    reject(Diag::unsupported_declaration, at, why);
    ok = false;
  };
  if (i.is_template_enum || i.is_prototype_instantiation || i.is_nonreal) {
    refuse(p, "an enumeration in a template (decision 0018 item 2)");
  } else if (t->source_corresp.enclosing_routine != nullptr) {
    refuse(p, "an enumeration declared in a block (decision 0018 item 2)");
  } else if (t->source_corresp.parent_scope != nullptr && t->source_corresp.parent_scope->kind != sck_file &&
             t->source_corresp.parent_scope->kind != sck_namespace) {
    refuse(p, "an enumeration declared in a class or a block (decision 0018 item 2)");
  }
  const int bits = static_cast<int>(t->size) * static_cast<int>(targ_char_bit);
  const bool is_signed = int_kind_is_signed[i.int_kind] != 0;
  const bool fixed = i.has_explicit_enum_base || i.is_scoped_enum;
  const std::vector<a_constant_ptr> list = enumerators_of(t);
  // An enumeration without a fixed type and without an enumerator list has no value range (and no size yet): said first, since
  // its size is 0.
  const bool incomplete = !fixed && list.empty() && !(i.extra_info != nullptr && i.extra_info->enumerator_list_complete);
  if (incomplete) {
    refuse(p, "an incomplete enumeration without a fixed underlying type (decision 0018 item 2)");
  } else if (bits <= 0 || bits > 128 || i.int_kind == ik_bit_precise || i.int_kind == ik_unsigned_bit_precise) {
    refuse(p, "an enumeration whose underlying type this version cannot read");
  }
  if (i.has_explicit_enum_base) {
    // The explicit underlying type (il_def.h:9570, base_type): not bool, and the type entry's own size and signedness agree.
    a_type_ptr b = i.extra_info != nullptr ? i.extra_info->base_type : nullptr;
    std::set<a_type_ptr> chain_seen;
    const char* computed = b != nullptr ? typeref_chain_reason(b, chain_seen, 0) : nullptr;
    b = b != nullptr ? skip_typerefs(b) : nullptr;
    if (computed != nullptr) {
      // Decision 0024 item 6: an underlying type that reaches its meaning through an alias-template instance or a member typedef
      // with a non-type argument (`: std::conditional_t<(65536u * 65536u == 0u), signed char, long>`) is a type the front end
      // computed alone.
      reject(Diag::unsupported_type, p, render_type(b) + " (the underlying type of an enumeration: " + computed + "; decision 0024 item 6)");
      ok = false;
    } else if (b == nullptr || b->kind != tk_integer || b->variant.integer.enum_type) {
      refuse(p, "an enumeration whose underlying type this version cannot read");
    } else if (b->variant.integer.bool_type) {
      refuse(p, "an enumeration with underlying type bool (decision 0018 item 2)");
    } else if (static_cast<int>(b->size) * static_cast<int>(targ_char_bit) != bits ||
               (int_kind_is_signed[b->variant.integer.int_kind] != 0) != is_signed) {
      refuse(p, "an enumeration whose underlying type differs from its type entry as this version reads it");
    }
  } else if (i.is_scoped_enum && (bits != static_cast<int>(targ_sizeof_int) * static_cast<int>(targ_char_bit) || !is_signed)) {
    refuse(p, "a scoped enumeration whose underlying type is not int");  // [dcl.enum]/5
  }
  // The attributes of the type itself are held to the allow-list by declaration_level_checks.
  vir::IntType underlying;  // the underlying (C++) or compatible (C) type, for the fixed range and the containment check
  underlying.bits = bits;
  underlying.is_signed = is_signed;
  vir::IntType c_int;
  c_int.bits = prog_.c_int_bits();
  c_int.is_signed = true;
  const vir::dec::Range int_range = vir::dec::range_of(c_int);
  std::string lo, hi;
  {
    std::string prev;
    bool have_prev = false;
    std::string emin, emax;
    for (a_constant_ptr c : list) {
      const a_source_position at = c->source_corresp.decl_position.seq != 0 ? c->source_corresp.decl_position : p;
      cx_.stmt_pos = at;
      const auto before = reported_count();
      check_attributes(c->source_corresp.attributes);
      if (reported_count() != before) ok = false;
      if (c->kind != ck_integer) {
        refuse(at, "an enumerator whose value this version cannot read");
        continue;
      }
      const std::string text = std::string(decimal_str_for_integer_constant(c).as_temp_characters());
      if (!vir::dec::in_range(text, int_range)) {
        refuse(at, "an enumerator whose value (" + text + ") is outside int (decision 0018 item 2)");
        continue;
      }
      if (!has_written_initializer(c)) {
        // C23 6.7.3.3, [dcl.enum]/2: the previous value plus one, 0 for the first.  A property of the IL: fail closed.
        const std::string expected = have_prev ? dec_inc(prev) : "0";
        if (text != expected) {
          refuse(at, "an enumerator without an initializer whose value this version cannot derive (decision 0018 item 10)");
          continue;
        }
      }
      if (!have_prev || vir::dec::cmp(text, emin) < 0) emin = text;
      if (!have_prev || vir::dec::cmp(text, emax) > 0) emax = text;
      prev = text;
      have_prev = true;
    }
    if (!have_prev) emin = emax = "0";
    if (fixed) {
      const vir::dec::Range r = vir::dec::range_of(underlying);
      lo = r.lo;
      hi = r.hi;
    } else if (incomplete) {
      // reported above
    } else {
      // [dcl.enum]/8: the values of the smallest bit-field that holds every enumerator (exact, decimal).
      const bool non_negative = !vir::dec::negative(emin);
      int m = 1;
      for (; m <= 128; ++m) {
        vir::dec::Range r = non_negative ? vir::dec::Range{"0", vir::dec::dec1(vir::dec::pow2(m))}
                                         : vir::dec::Range{vir::dec::neg(vir::dec::pow2(m - 1)), vir::dec::dec1(vir::dec::pow2(m - 1))};
        if (vir::dec::in_range(emin, r) && vir::dec::in_range(emax, r)) {
          lo = r.lo;
          hi = r.hi;
          break;
        }
      }
      if (lo.empty()) refuse(p, "an enumeration whose value range this version cannot compute");
    }
  }
  if (ok && !vir::dec::contains(vir::dec::range_of(underlying), vir::dec::Range{lo, hi})) {
    refuse(p, "an enumeration whose values do not fit its underlying type as this version reads it");
  }
  if (ok) {
    for (a_constant_ptr c : list)
      if (has_written_initializer(c)) pending_enumerators_.push_back(c);
    ei.ok = true;
    ei.silent = false;
    ei.type = vir::Type::integer(prog_.enum_type(spelled, bits, is_signed, i.is_scoped_enum != 0, lo, hi));
  }
  cx_ = std::move(saved_cx);
  return ei;
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
    if (u == nullptr) {
      // Decision 0024 item 6: a typedef (or alias) whose type contains an array type is rejected, used or not, until arrays
      // (M4.5e); a vector typedef stays accepted until it is used.
      if (t->source_corresp.name != nullptr) {
        std::set<a_type_ptr> seen;
        const char* why = computed_type_reason(t->variant.typeref.type, seen);
        if (why != nullptr && std::strcmp(why, "an array type") == 0)
          reject(Diag::unsupported_declaration, p, "a typedef or alias of a type that contains an array type (until arrays come, M4.5e; decision 0024 item 6)");
      }
      continue;
    }
    bool is_class = u->kind == tk_class || u->kind == tk_struct || u->kind == tk_union;
    bool is_enum = u->kind == tk_integer && u->variant.integer.enum_type;
    if (!is_class && !is_enum) continue;
    if (is_class && u->variant.class_struct_union.extra_info != nullptr &&
        u->variant.class_struct_union.extra_info->is_lambda_closure_class)
      continue;  // the lambda expression is rejected
    if (is_enum) {
      enum_info(u);  // decision 0018: accepted at file or namespace scope, else rejected here, once
      continue;
    }
    reject(Diag::unsupported_declaration, p, "a class, struct or union");
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
  lo.perturb_fixed_order = cfg.hook == "perturb_fixed_order";
  vir::LowerResult lowered = vir::lower_program(program, lo);
  if (!lowered.errors.empty()) {
    cfg.out.internal_inconsistency = true;  // a tool failure, exit status 3 (decision 0022 item 12)
    for (const vir::Diag& d : lowered.errors) {
      DiagPos at;
      at.known = d.loc.known();
      at.line = static_cast<unsigned long>(d.loc.line);
      at.col = static_cast<unsigned long>(d.loc.column);
      at.file = d.loc.file;
      at.full_name = d.loc.file;
      at.seq = at.line;
      report_at(Diag::internal_inconsistency, at, {d.reason});
    }
    return;
  }
  if (!lowered.rejections.empty()) {
    // Only after the internal errors, which must win.  Decision 0021: the calls of one full expression whose checks do
    // not form a chain.  A rejection like the others (exit status 2); `d.reason` names the two calls, `d.loc` is the later.
    for (const vir::Diag& d : lowered.rejections) {
      DiagPos at;
      at.known = d.loc.known();
      at.in_primary = false;
      at.line = static_cast<unsigned long>(d.loc.line);
      at.col = static_cast<unsigned long>(d.loc.column);
      at.file = d.loc.file;
      at.full_name = d.loc.file;
      at.seq = at.line;
      report_at(Diag::unsequenced_calls, at, {d.reason});
    }
    return;
  }
  viper::Emitted emitted = viper::emit(lowered.program);
  if (!emitted.problems.empty()) {
    cfg.out.internal_inconsistency = true;  // a tool failure, exit status 3 (decision 0022 item 12)
    for (const std::string& why : emitted.problems) {
      DiagPos at;
      report_at(Diag::internal_inconsistency, at, {why});
    }
    return;
  }
  cfg.out.vpr = emitted.vpr;
  cfg.out.map = emitted.map;
  cfg.out.functions = enc.verified_functions();
  cfg.out.encoded = true;
}

}  // namespace cerastes
