// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// IL walker behind --cerastes_walk.
//
// OUTPUT CONTRACT (what is and is not reported)
//
// Reported, for every routine whose decl_position is in the primary source
// file (not in any include file, system or user): plain routines, template
// prototype instantiations, template instances, specializations and implicit
// members.  Per routine: signature, template link and arguments, constraints,
// attributes (on the routine, its function type, its parameters, its local
// variables, and its statements), and for bodies the attributed statements and,
// in prototype bodies, every operation (`op`) and template-parameter constant
// (`tconst`) reachable from: the statement tree; constructor mem-initializers;
// default arguments; the expressions recorded by ck_template_param constants
// (e.g. (T)100); and the expressions recorded by UNNAMED folded constants (the
// folded value of `3 + 4` written in the body).  Recorded expressions of NAMED
// constants (enumerators, declared constants) are not followed: they are
// written elsewhere and are not part of the routine.  Also not followed: an
// unnamed constant whose recorded expression is in the file-scope region (a call
// site's copy of a callee's default argument); only constants whose recorded
// expression lives in the routine's own region are followed, plus the folds
// inside the routine's OWN default arguments (which are file scope).  Initializers of
// constant-initialized local statics (scope->local_static_variable_inits) are
// walked; dynamically initialized ones are in the statement tree.
//
// Attributes that no routine owns (on classes, fields, templates, namespaces,
// ...) are reported as top-level `attr ... on <entity> @ pos` lines, found by
// collecting every attribute entry of the file scope and of every function
// memory region.  Only attributes that are unrecognized or in the `cerastes`
// namespace are reported this way; recognized ones are reported when their
// owner is printed.
//
// NOT reported (stated, not silent):
//  - routines declared outside the primary file.  Routines with bodies in user
//    (non-system) files outside the primary file are counted in the final
//    `end walk:` line, and `cerastes::` attributes in such files produce a
//    warning.  System-header routines are neither counted nor reported.
//  - code after a #line directive or GNU linemarker in the primary file: it is
//    attributed to the named file and so filtered out (a linemarker with flag 3
//    even makes it a system file, so it is not counted either).  The walker
//    prints `warning: line directive in the primary file at L<l>; ...` for each
//    such directive, found from EDG's sequence-number lookup entries.
//  - expressions embedded in types: array bounds (`int arr[G + 1]`,
//    `sizeof(int[G * 2])`) and VLA bounds in C (a_vla_dimension);
//  - static_assert expressions in bodies, including dependent ones;
//  - template-argument expressions inside types (`Holder<sizeof(T) / 2>`);
//  - default arguments whose expression EDG has not evaluated yet
//    (a_param_type.has_unevaluated_template_default; default_arg_expr is NULL);
//  - expressions that are not reachable from statements, constructor
//    initializers, default arguments and the constants/expressions they record.
//
// Entities are enumerated with EDG's own IL walker (walk_file_scope_il, which
// does not enter function memory regions, and walk_routine_scope_il for every
// region); bodies are walked with traverse_statement (il_walk.h).  Output is
// deterministic: no addresses, entities sorted by position.  The last line of
// the output is always `end walk: ...`, so a run in which back_end() did not
// run is detectable.
#include "walker.h"

#include <algorithm>
#include <cstdio>
#include <map>
#include <set>
#include <utility>
#include <string>
#include <vector>

#include "il_names.h"
#include "options.h"
#include "render.h"

namespace cerastes {
namespace {

// ---- Entity collection ------------------------------------------------------

struct Collected {
  std::vector<a_routine_ptr> routines;
  std::set<a_routine_ptr> routine_set;
  std::vector<a_template_ptr> templates;
  std::set<a_template_ptr> template_set;
  std::vector<an_attribute_ptr> attrs;  // every attribute entry seen
  std::set<an_attribute_ptr> attr_set;
  std::map<an_attribute_ptr, std::string> attr_owner;  // for orphan reporting
};
Collected g_collected;
std::set<an_attribute_ptr> g_printed;  // attributes already in the output

// Register the attributes of an owner.  Attribute entries are allocated in the
// file-scope region but may be reachable only from function-region entries
// (statements), which the region walk does not follow into, so they are
// registered through their owners as well as through iek_attribute entries.
void note_owner(an_attribute_ptr list, const std::string& owner) {
  for (an_attribute_ptr a = list; a; a = a->next) {
    g_collected.attr_owner.emplace(a, owner);
    if (g_collected.attr_set.insert(a).second) g_collected.attrs.push_back(a);
  }
}

// Record "<label> <name>" as the owner of the attributes on `scp`.  Names are
// qualified (Box<int>::v, outer()::local) so that owners that would otherwise
// repeat stay distinguishable.  Locals and labels have no qualifier of their
// own in EDG's rendering, so they are prefixed with the enclosing routine
// (source_corresp.enclosing_routine, il_def.h:3303).
void note_scp_owner(a_source_correspondence* scp, an_il_entry_kind kind, const char* label,
                    bool qualified) {
  if (!scp->attributes) return;
  std::string name;
  if (scp->enclosing_routine && (kind == iek_variable || kind == iek_label)) {
    name = render_name(&scp->enclosing_routine->source_corresp, iek_routine) + "::" +
           (scp->name ? scp->name : "<unnamed>");
  } else if (qualified) {
    name = render_name(scp, kind);
  } else {
    name = scp->name ? scp->name : "<unnamed>";
  }
  note_owner(scp->attributes, std::string(label) + " " + name);
}

// Entry callback of walk_file_scope_il / walk_routine_scope_il (il_walk.h:32):
// called once per IL entry.  The kinds below either start with a
// source_correspondence (attributes in source_corresp.attributes) or carry an
// `attributes` list of their own.
void collect(char* entry, an_il_entry_kind kind) {
  switch (kind) {
    case iek_routine: {
      auto r = reinterpret_cast<a_routine_ptr>(entry);
      if (g_collected.routine_set.insert(r).second) g_collected.routines.push_back(r);
      note_scp_owner(&r->source_corresp, kind, "routine", true);
      break;
    }
    case iek_template: {
      auto t = reinterpret_cast<a_template_ptr>(entry);
      if (g_collected.template_set.insert(t).second) g_collected.templates.push_back(t);
      note_scp_owner(&t->source_corresp, kind, "template", true);
      break;
    }
    case iek_attribute: {
      auto a = reinterpret_cast<an_attribute_ptr>(entry);
      if (g_collected.attr_set.insert(a).second) g_collected.attrs.push_back(a);
      break;
    }
    case iek_type: {
      auto t = reinterpret_cast<a_type_ptr>(entry);
      // A named typeref is a typedef or alias declaration (typerefs that have
      // no name, such as decltype types, carry no attributes).
      bool alias = t->kind == tk_typeref;
      note_scp_owner(&t->source_corresp, kind, alias ? "typedef" : "type", true);
      break;
    }
    case iek_variable:
      note_scp_owner(&reinterpret_cast<a_variable_ptr>(entry)->source_corresp, kind, "variable", true);
      break;
    case iek_field:
      note_scp_owner(&reinterpret_cast<a_field_ptr>(entry)->source_corresp, kind, "field", true);
      break;
    case iek_namespace:
      note_scp_owner(&reinterpret_cast<a_namespace_ptr>(entry)->source_corresp, kind, "namespace", true);
      break;
    case iek_constant:
      note_scp_owner(&reinterpret_cast<a_constant_ptr>(entry)->source_corresp, kind, "constant", true);
      break;
    case iek_label:
      note_scp_owner(&reinterpret_cast<a_label_ptr>(entry)->source_corresp, kind, "label", false);
      break;
    case iek_template_parameter:
      note_scp_owner(&reinterpret_cast<a_template_parameter_ptr>(entry)->source_corresp, kind,
                     "template-parameter", false);
      break;
    case iek_param_type: {
      auto p = reinterpret_cast<a_param_type_ptr>(entry);
      note_owner(p->attributes, std::string("param ") + (p->name ? p->name : "_"));
      break;
    }
    case iek_statement: {
      auto s = reinterpret_cast<a_statement_ptr>(entry);
      note_owner(s->attributes, std::string("statement ") + name_of(s->kind));
      break;
    }
    case iek_using_decl:
      note_owner(reinterpret_cast<a_using_decl_ptr>(entry)->attributes, "using-declaration");
      break;
    case iek_base_class:
      note_owner(reinterpret_cast<a_base_class_ptr>(entry)->attributes, "base-class");
      break;
    default:
      break;
  }
}

// ---- Classification ---------------------------------------------------------

enum class RKind { plain = 0, prototype = 1, instance = 2, specialization = 3 };

const char* rkind_name(RKind k) {
  switch (k) {
    case RKind::plain: return "plain";
    case RKind::prototype: return "prototype";
    case RKind::instance: return "instance";
    case RKind::specialization: return "specialization";
  }
  return "?";
}

// a_routine.is_prototype_instantiation (il_def.h:12547): the generic body of a
// function template / member of a class template.  is_template_function
// (:12522) marks instances and specializations; is_specialized (:12532) marks
// those whose definition is user-supplied.
RKind classify(a_routine_ptr r) {
  if (r->is_prototype_instantiation) return RKind::prototype;
  if (r->is_template_function || r->assoc_template != nullptr)
    return r->is_specialized ? RKind::specialization : RKind::instance;
  return RKind::plain;
}

// Prototype routine -> the (definition) template that owns it, from
// a_template.prototype_instantiation.routine (il_def.h:17387-17423).
std::map<a_routine_ptr, a_template_ptr> g_proto_owner;

bool is_function_template(a_template_ptr t) {
  return t->kind == templk_function || t->kind == templk_member_function;
}

// ---- Output items -----------------------------------------------------------

struct Item {
  unsigned long line = 0, col = 0;
  int order = 0;
  std::string key;   // stable tie-break
  std::string text;  // complete, newline-terminated
};

// "<pos>"; "-" when the IL records no source position.
std::string at(const a_source_position& p) {
  SrcPos s = resolve(p);
  return s.known ? s.str() : "-";
}

bool is_pseudo(an_attribute_ptr a) {
  return a->kind == ak_empty_attr || a->kind == ak_attr_using_prefix ||
         a->syntactic_location == al_implicit;
}

void add_attrs(std::string& out, an_attribute_ptr list, const std::string& on) {
  for (an_attribute_ptr a = list; a; a = a->next) {
    // Pseudo-attributes (empty group, `using` prefix) and implicit ones that
    // never appeared in the source are not reported.
    if (is_pseudo(a)) continue;
    if (!g_printed.insert(a).second) continue;  // each attribute once
    out += "  attr " + render_attribute(a) + " on " + on + " @ " + at(a->position) + "\n";
  }
}

// ---- Template argument and constraint rendering -----------------------------

std::string param_name(a_template_parameter_ptr tp, int index) {
  if (tp && tp->source_corresp.name) return tp->source_corresp.name;
  return "#" + std::to_string(index);
}

// "T=int, U=short"; parameter names come from assoc_template->template_decl.
// A pack is shown as "Ts=pack{int, long}" (tak_start_of_pack_expansion opens
// it, following is_pack_element arguments belong to it; il_def.h:7683-7790).
std::string render_args(a_template_ptr tmpl, a_template_arg_ptr list) {
  if (list == nullptr) return "none";
  a_template_parameter_ptr tp =
      (tmpl && tmpl->template_decl) ? tmpl->template_decl->param_list : nullptr;
  std::vector<std::string> parts;
  std::vector<std::string> pack;
  std::string pack_name;
  bool in_pack = false;
  int index = 0;
  auto flush_pack = [&] {
    if (!in_pack) return;
    std::string s = pack_name + "=pack{";
    for (size_t i = 0; i < pack.size(); ++i) s += (i ? ", " : "") + pack[i];
    parts.push_back(s + "}");
    in_pack = false;
    pack.clear();
  };
  for (a_template_arg_ptr a = list; a; a = a->next) {
    if (a->kind == tak_start_of_pack_expansion) {
      flush_pack();
      in_pack = true;
      pack_name = param_name(tp, index);
      if (tp) tp = tp->next;
      ++index;
      continue;
    }
    if (in_pack && a->is_pack_element) {
      pack.push_back(render_template_arg(a));
      continue;
    }
    flush_pack();
    parts.push_back(param_name(tp, index) + "=" + render_template_arg(a));
    if (tp) tp = tp->next;
    ++index;
  }
  flush_pack();
  std::string s;
  for (size_t i = 0; i < parts.size(); ++i) s += (i ? ", " : "") + parts[i];
  return s;
}

// Constraint lines for a prototype routine.
void add_constraints(std::string& out, a_routine_ptr r, a_template_ptr tmpl) {
  a_template_decl_ptr d = tmpl ? tmpl->template_decl : nullptr;
  if (d) {
    // Type-constraints and abbreviated `Concept auto` parameters live on the
    // template parameter's placeholder type (il_def.h:9380-9401).
    int index = -1;
    for (a_template_parameter_ptr tp = d->param_list; tp; tp = tp->next) {
      ++index;
      if (tp->kind != tpk_type) continue;
      a_type_ptr ty = tp->variant.type.ptr;
      if (ty) ty = skip_typerefs(ty);
      if (!ty || ty->kind != tk_template_param) continue;
      a_template_param_type_supplement_ptr x = ty->variant.template_param.extra_info;
      if (!x) continue;
      if (x->coordinates.depth == CLASS_TEMPLATE_PLACEHOLDER_NESTING_DEPTH ||
          x->coordinates.depth == BIT_PRECISE_INT_NESTING_DEPTH)
        continue;  // the `constraint` union holds something else
      if (!x->constraint.type_constraint) continue;
      out += std::string("  constraint ") + (tp->is_abbreviated ? "abbreviated" : "type-constraint") +
             " " + param_name(tp, index) + " " + render_constraint(x->constraint.type_constraint) + "\n";
    }
#if MICROSOFT_EXTENSIONS_ALLOWED
    if (!d->is_generic)
#endif
      if (d->constraint.requires_clause)  // il_def.h:17234
        out += "  constraint requires " + render_constraint(d->constraint.requires_clause->constraint) + "\n";
  }
  if (r->trailing_requires_clause)  // il_def.h:13107
    out += "  constraint trailing " + render_constraint(r->trailing_requires_clause->constraint) + "\n";
}

// ---- Body walk --------------------------------------------------------------

// Traversal callbacks carry no user data, so the sink is file-local.
std::string* g_body = nullptr;
bool g_body_is_proto = false;
an_expr_node_ptr g_cur_const_expr = nullptr;  // innermost enk_constant being walked
// Operation nodes already reported for the current routine.  A node can be
// reached through more than one constant (a constant's own recorded expression
// and its template-parameter variant, shared expressions); each node is one
// operation and is reported once.
std::set<an_expr_node_ptr> g_seen_ops;
// Template-parameter constants already reported, keyed by constant and by the
// enk_constant node that uses it: a shared constant used at two places is two
// uses, while following the same node twice is one.
std::set<std::pair<a_constant_ptr, an_expr_node_ptr>> g_seen_consts;
// True while walking the routine's OWN default arguments: those expressions are
// file-scope, and the folds written in them belong to this routine.
bool g_in_own_default_args = false;

// Whether a recorded expression may be followed: it must live in the routine's
// own region (a genuine fold of an expression in this body), or be part of the
// routine's own default arguments.  A call site's copy of a callee's default
// argument also records a file-scope expression and is not followed.
bool followable(an_expr_node_ptr e) { return g_in_own_default_args || !in_file_scope(e); }

void on_statement(a_statement_ptr s, an_expr_or_stmt_traversal_block_ptr) {
  // a_statement.attributes (il_def.h:16540)
  for (an_attribute_ptr a = s->attributes; a; a = a->next) {
    if (is_pseudo(a)) continue;
    if (!g_printed.insert(a).second) continue;
    *g_body += std::string("  stmt ") + name_of(s->kind) + " @ " + at(s->position) + " attr " +
               render_attribute(a) + "\n";
  }
}

void on_expr(an_expr_node_ptr e, an_expr_or_stmt_traversal_block_ptr) {
  if (e->kind == enk_constant) g_cur_const_expr = e;
  if (!g_body_is_proto || e->kind != enk_operation) return;
  if (!g_seen_ops.insert(e).second) return;
  // il_def.h:15126-15144: operation kind and the kind of type it acts on
  // (tk_template_param or tk_unknown for dependent operations).
  auto& op = e->variant.operation;
  *g_body += std::string("  op ") + name_of(op.kind) + " type_kind=" + name_of(op.type_kind) +
             " type=" + render_type(e->type) + " @ " + at(e->position) + "\n";
}

// Budget for following recorded expressions, as traverse_expr_recorded_for_constant
// does for the walk's own following (il_walk.c:2914-2942).
bool spend_shared_budget(an_expr_or_stmt_traversal_block_ptr tb) {
  if (tb->shared_expr_budget != 0 && --tb->shared_expr_budget == 0) {
    tb->terminate = TRUE;
    return false;
  }
  return true;
}

void on_constant(a_constant_ptr c, an_expr_or_stmt_traversal_block_ptr tb) {
  // Follow the expression a folded constant records (il_display shows the
  // operations of `3 + 4`, `1 << 4`, `-1`, `sizeof(T) * 2` only through it).
  // Only UNNAMED constants: a folded constant with a name is the value of a
  // named entity (an enumerator, a named constant) whose initializer is
  // written elsewhere and would be attributed to every routine that uses it.
  // The distinguishing IL property is source_corresp.name: it is NULL for the
  // anonymous constant the folder creates for an expression in this body, and
  // set for enumerators and declared constants.  ck_template_param constants
  // are followed by process_template_parameter_constants_and_expressions.
  // (The traversal's own process_expressions_for_constants flag cannot make
  // this distinction, so it stays off and the following is done here.)
  // A constant created for a call site that uses a default argument is also
  // unnamed but records the callee's default-argument expression, which is
  // allocated in the file-scope region; genuine folds of an expression in this
  // body live in the function-scope region.  So also require the recorded
  // expression not to be in file scope (mem_tables.h:443).
  if (c->expr != nullptr && c->source_corresp.name == nullptr && followable(c->expr)) {
    if (spend_shared_budget(tb)) traverse_expr(c->expr, tb);
    // The recorded expression replaces the constant, except for a
    // ck_template_param constant: it also records a separate expression of its
    // own (its template-parameter variant, il_def.h:5364), which the walk's
    // process_template_parameter_constants_and_expressions flag follows after
    // this callback.  Both are distinct nodes in il_display.
    if (c->kind != ck_template_param) {
      tb->suppress_subtree_walk = TRUE;
      return;
    }
    // expr_node_from_constant (il.c:15154), which the walk uses for
    // ck_template_param, prefers the backing expression c->expr and so never
    // reaches the variant's own expression node; follow that one here too.
    an_expr_node_ptr own = nullptr;
    if (c->variant.template_param.kind == tpck_expression)
      own = c->variant.template_param.variant.expr;
    else if (c->variant.template_param.kind == tpck_sizeof || c->variant.template_param.kind == tpck_datasizeof ||
             c->variant.template_param.kind == tpck_alignof || c->variant.template_param.kind == tpck_uuidof ||
             c->variant.template_param.kind == tpck_typeid || c->variant.template_param.kind == tpck_noexcept)
      own = c->variant.template_param.variant.templ_sizeof.expr;
    if (own && own != c->expr && followable(own) && spend_shared_budget(tb)) traverse_expr(own, tb);
    // Both expressions are done; EDG's own following (expr_node_from_constant
    // returns c->expr again) would only revisit them and their constants.
    if (own || c->variant.template_param.kind == tpck_expression ||
        c->variant.template_param.kind == tpck_sizeof)
      tb->suppress_subtree_walk = TRUE;
  }
  if (!g_body_is_proto || c->kind != ck_template_param) return;
  // il_def.h:5364: kind of template-parameter constant, e.g. (T)100.  A
  // constant has no position of its own in general; use the enclosing
  // enk_constant expression node when it wraps this constant.
  if (!g_seen_consts.insert({c, g_cur_const_expr && g_cur_const_expr->variant.constant.ptr == c ? g_cur_const_expr : nullptr}).second)
    return;  // already reported for this use
  std::string pos = "-";
  if (g_cur_const_expr && g_cur_const_expr->variant.constant.ptr == c) {
    SrcPos p = resolve(g_cur_const_expr->position);
    if (p.known) pos = p.str();
  }
  if (pos == "-") {
    SrcPos p = resolve(c->source_corresp.decl_position);
    if (p.known) pos = p.str();
  }
  *g_body += std::string("  tconst ") + name_of(c->variant.template_param.kind) +
             " type=" + render_type(c->type) + " @ " + pos + "\n";
}

// The routine's own function scope, or null when it has none in the IL.
// function_def_number selects the routine's descriptor (mem_tables.h:80-107);
// unlike il_header.region_scope_entry[memory_region] this is correct for
// lambdas and members of local classes, which share the enclosing function's
// memory region (B1).
a_scope_ptr body_scope(a_routine_ptr r) {
  if (r->function_def_number == NULL_function_def_number) return nullptr;
  a_scope_ptr sp = scope_for_routine_or_null(r);
  if (sp == nullptr || sp->kind != sck_function || sp->variant.routine.ptr != r) return nullptr;
  return sp;
}

// Attributes on the variables of a function scope and its nested block scopes.
void add_variable_attrs(std::string& out, a_scope_ptr sp) {
  auto each = [&](a_variable_ptr list) {
    for (a_variable_ptr v = list; v; v = v->next)
      add_attrs(out, v->source_corresp.attributes,
                std::string("variable ") + (v->source_corresp.name ? v->source_corresp.name : "<unnamed>"));
  };
  each(sp->nonstatic_variables);
  each(sp->variables);
  if (sp->kind == sck_function) each(sp->variant.routine.parameters);
  for (a_scope_ptr c = sp->scopes; c; c = c->next)
    if (c->kind == sck_block || c->kind == sck_condition) add_variable_attrs(out, c);
}

constexpr unsigned long kSharedExprBudget = 1000000;

// Initializers of local statics that are statically (constant) initialized are
// held on the scope's local_static_variable_inits list (il_def.h:18542), not in
// the statement tree.  Dynamically initialized ones appear in the statement
// tree already, so only initk_static is walked here, to avoid reporting them
// twice.  Nested block scopes have their own lists.
void walk_static_inits(a_scope_ptr sp, an_expr_or_stmt_traversal_block_ptr tb) {
  for (a_local_static_variable_init_ptr i = sp->local_static_variable_inits; i; i = i->next)
    if (i->init_kind == initk_static && i->initializer.constant) traverse_constant(i->initializer.constant, tb);
  for (a_scope_ptr c = sp->scopes; c; c = c->next)
    if (c->kind == sck_block || c->kind == sck_condition) walk_static_inits(c, tb);
}

// ---- Per-routine report ------------------------------------------------------

Item describe(a_routine_ptr r) {
  Item it;
  RKind kind = classify(r);
  SrcPos pos = resolve(r->source_corresp.decl_position);
  it.line = pos.line;
  it.col = pos.col;
  it.order = static_cast<int>(kind);

  std::string name = render_name(&r->source_corresp, iek_routine);
  a_scope_ptr scope = body_scope(r);
  std::string out = "routine " + name + " " + rkind_name(kind);
  if (r->compiler_generated) out += " implicit";
  out += scope ? " body" : " nobody";
  out += " @ " + pos.str() + "\n";

  // Signature: routine type is a tk_routine, possibly behind typerefs.
  std::vector<an_expr_node_ptr> default_args;
  a_type_ptr rt = r->type ? skip_typerefs(r->type) : nullptr;
  if (rt && rt->kind == tk_routine) {
    out += "  returns " + render_type(rt->variant.routine.return_type) + "\n";
    a_routine_type_supplement_ptr x = rt->variant.routine.extra_info;
    int i = 1;
    std::string param_attrs;
    for (a_param_type_ptr pt = x ? x->param_type_list : nullptr; pt; pt = pt->next, ++i) {
      out += "  param " + std::to_string(i) + " " + (pt->name ? pt->name : "_") + ": " +
             render_type(pt->type) + "\n";
      add_attrs(param_attrs, pt->attributes, "param " + std::to_string(i));  // il_def.h:6540
      if (pt->default_arg_expr) default_args.push_back(pt->default_arg_expr);
    }
    out += param_attrs;
  } else {
    out += "  returns <not-a-routine-type>\n";
  }

  // Template link.
  a_template_ptr tmpl = r->assoc_template;
  if (kind == RKind::prototype && tmpl == nullptr) {
    auto f = g_proto_owner.find(r);
    if (f != g_proto_owner.end()) tmpl = f->second;
  }
  if (kind == RKind::instance || kind == RKind::specialization) {
    out += "  template " + (tmpl ? render_name(&tmpl->source_corresp, iek_template) : std::string("<none>")) +
           " args <" + render_args(tmpl, r->template_arg_list) + ">\n";
  }
  if (kind == RKind::prototype) add_constraints(out, r, tmpl);

  // Attributes: on the routine (source_corresp.attributes) and on its function
  // type, where C++ puts attributes written after the parameter list: either
  // on the tk_routine type itself or on a trk_for_type_attributes typeref
  // wrapped around it (il_def.h:9713).  Other typerefs on the way (a typedef or
  // alias such as `using FT = int(int)`) are shared by every declaration that
  // uses them; their own attributes belong to the alias, not to this routine,
  // and are reported once as a top-level `on typedef`.
  add_attrs(out, r->source_corresp.attributes, "routine");
  for (a_type_ptr t = r->type; t; t = t->variant.typeref.type) {
    if (t->kind != tk_typeref) {
      add_attrs(out, t->source_corresp.attributes, "type");
      break;
    }
    if (t->variant.typeref.kind == trk_for_type_attributes)
      add_attrs(out, t->source_corresp.attributes, "type");
  }
  if (scope) add_variable_attrs(out, scope);

  // Expressions: default arguments, constructor mem-initializers, the body.
  g_body = &out;
  g_body_is_proto = kind == RKind::prototype;
  g_cur_const_expr = nullptr;
  g_seen_ops.clear();
  g_seen_consts.clear();
  an_expr_or_stmt_traversal_block tb;
  clear_expr_or_stmt_traversal_block(&tb);
  tb.process_statement = on_statement;
  tb.process_expr = on_expr;
  tb.process_constant = on_constant;
  tb.process_non_dynamic_constants = TRUE;
  // Template-parameter constants such as (T)100 record the expression they
  // stand for; without this flag operations inside them are invisible
  // (il_walk.c:3025-3040).
  tb.process_template_parameter_constants_and_expressions = TRUE;
  // process_expressions_for_constants stays FALSE: it follows the recorded
  // expression of every folded constant, including named ones (an enumerator's
  // initializer), which lives outside this routine.  on_constant follows the
  // unnamed ones instead.
  tb.process_expressions_for_constants = FALSE;
  // Expressions recorded by constants may be shared, and following them walks a
  // graph (il_walk.h:293-305).  The intended guard is a visited_shared_exprs
  // set, but Ptr_set uses FE_allocator, which asserts once front-end
  // processing has ended (mem_manage.h:377), as it has when back_end() runs.
  // So the walk is bounded by a budget instead, and a walk that exhausts it is
  // reported, never silently truncated.
  tb.shared_expr_budget = kSharedExprBudget;
  g_in_own_default_args = true;
  for (an_expr_node_ptr e : default_args) traverse_expr(e, &tb);
  g_in_own_default_args = false;
  if (scope) {
    // Constructor mem-initializers (a_scope variant.routine.constructor_inits,
    // il_def.h:18412) are not part of the statement tree.
    for (a_constructor_init_ptr ci = scope->variant.routine.constructor_inits; ci; ci = ci->next) {
      if (ci->initializer) traverse_dynamic_init(ci->initializer, &tb);
      if (ci->source_expr) traverse_expr(ci->source_expr, &tb);
    }
    walk_static_inits(scope, &tb);
    if (scope->assoc_block) traverse_statement(scope->assoc_block, &tb);
  }
  g_body = nullptr;
  if (tb.terminate && tb.shared_expr_budget == 0)
    out += "  warning: expression walk truncated (shared-expression budget exhausted)\n";

  // Tie-break for entities at the same position and kind (e.g. the implicit
  // copy and move assignment of a class): name, template arguments, type.
  it.key = name + "\x01" + (r->template_arg_list ? render_args(tmpl, r->template_arg_list) : std::string()) +
           "\x01" + render_type(r->type);
  it.text = out;
  return it;
}

bool is_cerastes_or_unrecognized(an_attribute_ptr a) {
  return a->kind == ak_unrecognized ||
         (a->namespace_name && std::string(a->namespace_name) == "cerastes");
}

// Line directives (#line, GNU linemarkers) in the PRIMARY file remap the file
// name and line of the code after them, so the primary-file filter attributes
// that code to another file and drops it.  EDG records each remapped region as
// a sequence-number lookup entry whose source file has no full_name ("NULL for
// entries generated by #line directives", il_def.h:144-151).  Such an entry is
// a directive in the primary file, rather than in an include file, when its
// first sequence number lies inside the primary file's range but outside every
// file it directly includes (directive entries are listed among the children of
// the file they appear in, distinguished by their NULL full_name).  Returns one warning item per directive.
std::vector<Item> line_directive_warnings() {
  std::vector<Item> out;
  a_source_file_ptr primary = il_header.primary_source_file;
  if (primary == nullptr) return out;
  for (a_seq_number_lookup_entry_ptr e = il_header.seq_number_lookup_entries; e; e = e->next) {
    a_source_file_ptr f = e->source_file;
    if (f == nullptr || f->full_name != nullptr) continue;  // not directive-generated
    if (e->first < primary->first_seq_number || e->first > primary->last_seq_number) continue;
    bool in_include = false;
    for (a_source_file_ptr c = primary->first_child_file; c; c = c->next)
      if (c->full_name != nullptr && e->first >= c->first_seq_number && e->first <= c->last_seq_number)
        in_include = true;  // a real include; directive entries are children too, with full_name NULL
    if (in_include) continue;
    // The entry starts on the line after the directive.
    a_source_file_ptr phys = nullptr;
    a_line_number line = 0;
    a_boolean at_end = FALSE;
    conv_seq_to_physical_file_and_line(e->first, &phys, &line, &at_end);
    unsigned long dir_line = line > 1 ? line - 1 : line;
    Item w;
    w.line = dir_line;
    w.col = 0;
    w.order = 60;
    w.key = f->file_name ? f->file_name : "";
    w.text = "warning: line directive in the primary file at L" + std::to_string(dir_line) +
             "; code after it is attributed to " + (f->file_name ? f->file_name : "<unknown>") + "\n";
    out.push_back(w);
  }
  return out;
}

}  // namespace

void walk_il_and_print() {
  // File scope, then every function memory region (the file-scope walk does
  // not enter them, il_walk.c:217-219).
  walk_file_scope_il(collect, nullptr, nullptr, nullptr, nullptr, /*clear_fe_pointers=*/FALSE);
  for (a_memory_region_number region = FILE_SCOPE_REGION_NUMBER + 1;
       region <= highest_used_region_number; ++region) {
    if (mem_region_table[region] == nullptr) continue;  // freed, as il_display skips
    walk_routine_scope_il(region, collect, nullptr, nullptr, nullptr, nullptr, /*clear_fe_pointers=*/FALSE);
  }

  // Templates -> prototype routines.
  for (a_template_ptr t : g_collected.templates) {
    if (!is_function_template(t)) continue;
    a_template_ptr def = t->definition_template;
    if (t->canonical_template == t && def && def->prototype_instantiation.routine)
      g_proto_owner[def->prototype_instantiation.routine] = def;
  }

  std::vector<Item> items;
  unsigned long reported = 0, outside_user_with_body = 0;
  for (a_routine_ptr r : g_collected.routines) {
    SrcPos pos = resolve(r->source_corresp.decl_position);
    // Primary-file filter: a routine is reported iff its decl_position is a
    // known position outside any include file.  Consequences (checked with
    // inc<std::int64_t> and a std:: class template):
    //  - an instance of a template DEFINED in the primary file is reported even
    //    when instantiated with a type from a system header, because an
    //    instance's decl_position is the template definition's position;
    //  - an instance of a SYSTEM-header template is not reported: its position
    //    is in the header.
    // What the filter hides is counted: routines with bodies in user
    // (non-system) files outside the primary file go into the `end walk:` line.
    if (!pos.in_primary) {
      if (pos.known && !pos.in_system && body_scope(r)) ++outside_user_with_body;
      continue;
    }
    items.push_back(describe(r));
    ++reported;
  }

  // Cross-checks, deliberately part of the output so that a regression of the
  // IL properties Gate G1 relies on shows up in the goldens.
  for (a_template_ptr t : g_collected.templates) {
    if (!is_function_template(t) || t->canonical_template != t) continue;
    SrcPos pos = resolve(t->source_corresp.decl_position);
    if (!pos.in_primary) continue;
    a_template_ptr def = t->definition_template;
    if (def == nullptr) continue;  // declaration only: no body, no prototype routine expected
    a_routine_ptr proto = def->prototype_instantiation.routine;
    if (proto && g_collected.routine_set.count(proto) && !body_scope(proto)) {
      Item w;
      w.line = pos.line;
      w.col = pos.col;
      w.order = 102;
      w.key = render_name(&t->source_corresp, iek_template);
      w.text = "warning: template " + w.key + " has a definition but its prototype routine has no body\n";
      items.push_back(w);
    }
    if (options().walk_selftest || proto == nullptr || !g_collected.routine_set.count(proto)) {
      Item w;
      w.line = pos.line;
      w.col = pos.col;
      w.order = 100;
      w.key = render_name(&t->source_corresp, iek_template);
      w.text = "warning: template " + w.key + " has no prototype routine\n";
      items.push_back(w);
    }
  }
  for (a_routine_ptr r : g_collected.routines) {
    SrcPos pos = resolve(r->source_corresp.decl_position);
    if (!pos.in_primary || r->is_prototype_instantiation) continue;
    if (options().walk_selftest ? r->is_template_function
                                : (r->is_template_function && r->assoc_template == nullptr)) {
      Item w;
      w.line = pos.line;
      w.col = pos.col;
      w.order = 101;
      w.key = render_name(&r->source_corresp, iek_routine);
      w.text = "warning: instance " + w.key + " has no assoc_template\n";
      items.push_back(w);
    }
  }

  for (Item& w : line_directive_warnings()) items.push_back(w);

  // Attributes not printed with an owner: report them on their own so that no
  // unrecognized or cerastes:: attribute in the primary file is lost.  In user
  // files outside the primary file, warn instead.
  std::vector<Item> outside;
  for (an_attribute_ptr a : g_collected.attrs) {
    if (is_pseudo(a) || g_printed.count(a)) continue;
    SrcPos pos = resolve(a->position);
    if (!pos.known) continue;
    if (pos.in_primary) {
      if (!is_cerastes_or_unrecognized(a)) continue;
      auto o = g_collected.attr_owner.find(a);
      Item w;
      w.line = pos.line;
      w.col = pos.col;
      w.order = 50;
      std::string owner = o != g_collected.attr_owner.end() ? o->second : "<unknown owner>";
      w.key = owner;
      w.text = "attr " + render_attribute(a) + " on " + owner + " @ " + pos.str() + "\n";
      items.push_back(w);
    } else if (!pos.in_system && a->namespace_name && std::string(a->namespace_name) == "cerastes") {
      Item w;
      w.line = pos.line;
      w.col = pos.col;
      w.key = pos.file;
      w.text = "warning: attribute " + render_attribute(a) + " @ " + pos.file + ":" +
               std::to_string(pos.line) + " is outside the primary file and not reported\n";
      outside.push_back(w);
    }
  }

  auto less = [](const Item& a, const Item& b) {
    if (a.line != b.line) return a.line < b.line;
    if (a.col != b.col) return a.col < b.col;
    if (a.order != b.order) return a.order < b.order;
    return a.key < b.key;
  };
  std::stable_sort(items.begin(), items.end(), less);
  std::stable_sort(outside.begin(), outside.end(), [&less](const Item& a, const Item& b) {
    if (a.key != b.key) return a.key < b.key;
    return less(a, b);
  });
  for (const Item& it : items) std::fputs(it.text.c_str(), stdout);
  for (const Item& it : outside) std::fputs(it.text.c_str(), stdout);
  std::printf("end walk: %lu routines reported, %lu routines with bodies in non-system files outside the primary file\n",
              reported, outside_user_with_body);
}

}  // namespace cerastes
