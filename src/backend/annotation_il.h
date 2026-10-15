// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The only file that knows how the EDG fork's attribute registration shows up
// in the IL (decision 0007), so that a change of that representation is a
// change here and nowhere else:
//
//  * a registered attribute has kind ak_registered (il_def.h:3134); its
//    namespace and name are in the namespace_name and name fields;
//  * the optional result name of `ensures(r, p)` is an aak_token argument
//    (token_kind tok_identifier) before the aak_expression argument;
//  * an expression argument is read with expr_node_from_attribute_arg (il.h:1909),
//    which also handles local_expr_ref (the expression lives in a function
//    memory region while the attribute lives in file scope);
//  * parameter references are enk_param_ref {param_num, levels_up}
//    (il_def.h:15600).  An attribute whose signature starts with "R"
//    (`ensures`) is scanned inside one extra binder scope, so there
//    parameter k is {k,1}, the named result is {1,0} and `this` is {0,0}.
//    For the other attributes parameter k is {k,0} and `this` is {0,0}.
//    Anything else is reported as unmapped; it is never guessed at.
#pragma once

#include <cstring>
#include <string>
#include <vector>

#include "edg_il.h"

namespace cerastes {

enum class Annot { none, requires_, ensures, invariant, check, fold, unfold, predicate, pure };

inline bool is_registered_cerastes(an_attribute_ptr a) {
  return a->kind == ak_registered && a->namespace_name != nullptr &&
         std::strcmp(a->namespace_name, "cerastes") == 0;
}

inline Annot classify(an_attribute_ptr a) {
  if (!is_registered_cerastes(a) || a->name == nullptr) return Annot::none;
  static const struct { const char* name; Annot kind; } kNames[] = {
      {"requires", Annot::requires_}, {"ensures", Annot::ensures},     {"invariant", Annot::invariant},
      {"check", Annot::check},        {"fold", Annot::fold},           {"unfold", Annot::unfold},
      {"predicate", Annot::predicate}, {"pure", Annot::pure},
  };
  for (const auto& n : kNames)
    if (std::strcmp(a->name, n.name) == 0) return n.kind;
  return Annot::none;
}

inline bool is_contract(Annot k) { return k == Annot::requires_ || k == Annot::ensures; }

// One argument of a registered attribute.
struct AnnotArg {
  enum class Kind { binder, expression, other } kind = Kind::other;
  int index = 0;                      // 1-based position in the argument list
  an_attribute_arg_ptr raw = nullptr;
  const char* binder_name = nullptr;  // Kind::binder
  an_expr_node_ptr expr = nullptr;    // Kind::expression (may be null: unreadable)
};

inline std::vector<AnnotArg> arguments_of(an_attribute_ptr a) {
  std::vector<AnnotArg> out;
  int i = 0;
  for (an_attribute_arg_ptr g = a->arguments; g; g = g->next) {
    if (g->kind == aak_empty) continue;
    AnnotArg v;
    v.index = ++i;
    v.raw = g;
    if (g->kind == aak_token) {
      v.kind = AnnotArg::Kind::binder;
      v.binder_name = g->variant.token;
    } else if (g->kind == aak_expression) {
      v.kind = AnnotArg::Kind::expression;
      v.expr = expr_node_from_attribute_arg(g);
    }
    out.push_back(v);
  }
  return out;
}

inline bool has_binder(an_attribute_ptr a) {
  for (const AnnotArg& g : arguments_of(a))
    if (g.kind == AnnotArg::Kind::binder) return true;
  return false;
}

struct ParamRef {
  enum class Kind { param, binder, this_param, unmapped } kind = Kind::unmapped;
  unsigned num = 0;
};

// Interpret an enk_param_ref found in attribute `a` (see the convention above).
// `nesting` is the number of parameter scopes that constructs inside the
// attribute's argument have opened (a requires-expression's parameters, a
// function type): every level of the reference counts them, and a reference
// that stays inside such a scope is never read as one to the declaration's own
// parameters.
inline ParamRef interpret_param_ref(an_expr_node_ptr e, Annot a, bool binder_present, int nesting = 0) {
  ParamRef r;
  const auto& p = e->variant.param_ref;
  r.num = p.param_num;
  if (nesting < 0) return r;
  const unsigned levels = p.levels_up;
  const unsigned inner = static_cast<unsigned>(nesting);
  if (levels < inner) return r;  // a parameter of a nested construct: not ours
  const unsigned up = levels - inner;
  if (p.param_num == 0) {
    if (up == 0) r.kind = ParamRef::Kind::this_param;
    return r;
  }
  if (a == Annot::ensures) {
    if (up == 1) r.kind = ParamRef::Kind::param;
    else if (up == 0 && p.param_num == 1 && binder_present) r.kind = ParamRef::Kind::binder;
  } else if (up == 0) {
    r.kind = ParamRef::Kind::param;
  }
  return r;
}

// ---- Contracts and their root template (0006 item 6) -------------------------

// The `requires`/`ensures` attributes written after the parameter list of
// `r`, in the order the routine carries them.  Contracts of every declaration
// are the same (must-match), so the list on the routine is the contract.
inline std::vector<an_attribute_ptr> contracts_of(a_routine_ptr r) {
  std::vector<an_attribute_ptr> out;
  for (an_attribute_ptr a = r->source_corresp.attributes; a; a = a->next)
    if (is_contract(classify(a)) && a->syntactic_location == al_post_func) out.push_back(a);
  return out;
}

// Whether the attributes of `r` include a registered attribute of kind `k`.
inline bool has_annot(a_routine_ptr r, Annot k) {
  for (an_attribute_ptr a = r->source_corresp.attributes; a; a = a->next)
    if (classify(a) == k) return true;
  return false;
}

// The prototype routine of the original template that `t` descends from: the
// prototype_template chain leads from a member template of a class instance to
// the member template of the class template (il_def.h:17429); the definition
// template holds the prototype instantiation (il_def.h:17387).  Null when there
// is none.
inline a_routine_ptr root_routine_of_template(a_template_ptr t) {
  if (t == nullptr) return nullptr;
  for (int guard = 0; t->prototype_template != nullptr && guard < 64; ++guard) t = t->prototype_template;
  a_template_ptr c = t->canonical_template ? t->canonical_template : t;
  a_template_ptr d = c->definition_template ? c->definition_template : c;
  if (d->kind != templk_function && d->kind != templk_member_function) return nullptr;
  return d->prototype_instantiation.routine;
}

// The root routine of an instance or specialization `r` (through assoc_template),
// null when `r` has none or is its own root.  A routine with no assoc_template has
// no root and is never compared with one, although it may stem from a template:
// a hidden friend defined in a class template, a member of a local class in a
// function template instance, and the call operator of a lambda in an instance
// are plain routines in the IL, and carry their own copy of any contract.  An
// inheriting constructor has no contract of its own: it is the inherited
// constructor (a_routine.friends_or_originator.inherited_routine), whose
// contract applies; neither the walker nor the checks compare it.
inline a_routine_ptr root_routine(a_routine_ptr r) {
  a_routine_ptr root = root_routine_of_template(r->assoc_template);
  return root == r ? nullptr : root;
}

// The shape of a routine's contract, in order: for each requires/ensures its
// name and number of arguments (so a named result differs from an unnamed one).
// This is the comparison that the walker's cross-check and
// `contract_not_propagated` make.  The content of the arguments is checked
// separately: an erroneous argument is annotation_substitution_error.
inline std::vector<std::string> contract_names(a_routine_ptr r) {
  std::vector<std::string> out;
  for (an_attribute_ptr a : contracts_of(r))
    out.push_back(std::string(a->name ? a->name : "?") + "/" + std::to_string(arguments_of(a).size()));
  return out;
}

}  // namespace cerastes
