// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Checks of annotations that are made on the IL, not by EDG (0006 item 10):
//  * placement of requires/ensures (al_post_func only);
//  * annotation arguments: side effects, statement expressions, `old` outside
//    `ensures`, a loop invariant shadowed by the loop's own variable;
//  * fold/unfold: a direct call of a [[cerastes::predicate]] function;
//  * predicate/pure/contract combinations and return types;
//  * defense in depth: a parameter after a pack, a result name on a void
//    function, a contract that did not reach an instance, a contract on a lambda;
//  * C++20 modules;
//  * calls of specification functions from ordinary code.
//
// A construct whose meaning depends on template arguments is checked in each
// instance (and where it can already be decided, in the prototype); one in a
// template that is never instantiated is not checked (0006 item 10, "Known
// gap").  The diagnostics found in an instance say so.
#include "annotation_checks.h"

#include <cctype>
#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "annotation_il.h"
#include "decls.h"
#include "diagnostics.h"
#include "expr_tree.h"
#include "il_names.h"
#include "il_query.h"
#include "options.h"
#include "render.h"
#include "verify_config.h"

namespace cerastes {
namespace {

// ---- Collection ---------------------------------------------------------------

struct Collected {
  std::vector<a_routine_ptr> routines;
  std::set<a_routine_ptr> routine_set;
  std::vector<a_template_ptr> templates;
  std::vector<a_variable_ptr> variables;  // declared outside any routine
  std::vector<a_field_ptr> fields;
  std::vector<a_statement_ptr> annotated_statements;  // statements with a cerastes attribute
  // R4a/R4b/R4d: every expression node that names a routine, every class type that is an instance of a class
  // template, and every registered cerastes attribute, each once, wherever the IL keeps it.
  std::vector<an_expr_node_ptr> routine_nodes;
  std::vector<a_type_ptr> class_instances;
  std::vector<a_constant_ptr> address_constants;  // the address of a routine, or a pointer to a member function
  std::vector<a_variable_ptr> all_variables;      // every variable, local ones included
  std::vector<an_attribute_ptr> cerastes_attributes;
};
Collected g_c;
// Prototype routine -> the template that owns it.
std::map<a_routine_ptr, a_template_ptr> g_proto_owner;

void collect(char* entry, an_il_entry_kind kind) {
  if (kind == iek_routine) {
    auto r = reinterpret_cast<a_routine_ptr>(entry);
    if (g_c.routine_set.insert(r).second) g_c.routines.push_back(r);
  } else if (kind == iek_template) {
    g_c.templates.push_back(reinterpret_cast<a_template_ptr>(entry));
  } else if (kind == iek_variable) {
    auto v = reinterpret_cast<a_variable_ptr>(entry);
    g_c.all_variables.push_back(v);
    if (v->source_corresp.enclosing_routine == nullptr) g_c.variables.push_back(v);
  } else if (kind == iek_field) {
    g_c.fields.push_back(reinterpret_cast<a_field_ptr>(entry));
  } else if (kind == iek_expr_node) {
    auto e = reinterpret_cast<an_expr_node_ptr>(entry);
    if (e->kind == enk_routine) g_c.routine_nodes.push_back(e);
  } else if (kind == iek_type) {
    auto t = reinterpret_cast<a_type_ptr>(entry);
    if ((t->kind == tk_struct || t->kind == tk_class || t->kind == tk_union) &&
        t->variant.class_struct_union.extra_info != nullptr &&
        t->variant.class_struct_union.extra_info->template_arg_list != nullptr)
      g_c.class_instances.push_back(t);
  } else if (kind == iek_constant) {
    auto c = reinterpret_cast<a_constant_ptr>(entry);
    if ((c->kind == ck_address && c->variant.address.kind == abk_routine) ||
        (c->kind == ck_ptr_to_member && c->variant.ptr_to_member.is_function_ptr))
      g_c.address_constants.push_back(c);
  } else if (kind == iek_attribute) {
    auto a = reinterpret_cast<an_attribute_ptr>(entry);
    if (is_registered_cerastes(a)) g_c.cerastes_attributes.push_back(a);
  } else if (kind == iek_statement) {
    auto st = reinterpret_cast<a_statement_ptr>(entry);
    for (an_attribute_ptr a = st->attributes; a; a = a->next)
      if (is_registered_cerastes(a)) {
        g_c.annotated_statements.push_back(st);
        break;
      }
  }
}

bool has_cerastes_attribute(a_routine_ptr r) {
  for (an_attribute_ptr a = r->source_corresp.attributes; a; a = a->next)
    if (is_registered_cerastes(a)) return true;
  return false;
}

// The constant that names the callee of a dependent call (a template-parameter
// constant of kind tpck_unknown_function, or tpck_template_ref for `f<T>(x)`, which
// holds the unknown function in its `con`), or null.
a_constant_ptr unknown_callee_constant(a_constant_ptr c) {
  if (c == nullptr || c->kind != ck_template_param) return nullptr;
  const auto& tp = c->variant.template_param;
  if (tp.kind == tpck_unknown_function) return c;
  if (tp.kind == tpck_template_ref) return tp.variant.template_ref.con;
  return nullptr;
}

// The specification functions of <cerastes.h> (namespace cerastes).
bool is_spec_name(const char* n) {
  return n != nullptr && (std::strcmp(n, "old") == 0 || std::strcmp(n, "owns") == 0 || std::strcmp(n, "reads") == 0 ||
                          std::strcmp(n, "live") == 0 || std::strcmp(n, "forall") == 0 || std::strcmp(n, "owns_range") == 0);
}

// Whether `ns` (an alias resolved) is the namespace cerastes declared in the
// global namespace.
a_namespace_ptr resolve_alias(a_namespace_ptr ns) {
  for (int guard = 0; ns != nullptr && ns->is_namespace_alias && guard < 64; ++guard) ns = ns->variant.assoc_namespace;
  return ns;
}

bool is_cerastes_namespace(a_namespace_ptr ns) {
  ns = resolve_alias(ns);
  if (ns == nullptr || ns->source_corresp.name == nullptr || std::strcmp(ns->source_corresp.name, "cerastes") != 0)
    return false;
  a_scope_ptr parent = ns->source_corresp.parent_scope;
  return parent == nullptr || parent->kind == sck_file;
}

bool scope_is_cerastes_namespace(a_scope_ptr sp) {
  return sp != nullptr && (sp->kind == sck_namespace || sp->kind == sck_namespace_extension) &&
         is_cerastes_namespace(sp->variant.assoc_namespace);
}

// Whether the translation unit declares namespace cerastes at all (that is,
// <cerastes.h> or an equivalent declaration is visible).  Without it no name can
// denote a specification function.
bool cerastes_namespace_in_tu() {
  static int cached = -1;
  if (cached < 0) {
    cached = 0;
    a_scope_ptr file = il_header.region_scope_entry[FILE_SCOPE_REGION_NUMBER];
    for (a_namespace_ptr ns = file ? file->namespaces : nullptr; ns; ns = ns->next)
      if (is_cerastes_namespace(ns)) { cached = 1; break; }
  }
  return cached == 1;
}

// Whether qualified lookup of `name` in namespace `ns` can find a function of
// namespace cerastes: `ns` is cerastes, or (transitively) has a using-directive
// for it, or a using-declaration of `name` from it.
// `visited` holds the namespaces already searched (CP8: without it, chains of
// directives took exponential time, and a cycle reached the depth limit).
bool namespace_reaches_cerastes(a_namespace_ptr ns, const char* name, int depth,
                                std::set<a_namespace_ptr>& visited) {
  ns = resolve_alias(ns);
  if (ns == nullptr || depth > 64) return ns != nullptr;  // too deep: fail closed
  if (!visited.insert(ns).second) return false;            // searched already
  if (is_cerastes_namespace(ns)) return true;
  a_scope_ptr sp = ns->variant.assoc_scope;
  if (sp == nullptr) return true;  // unknown: fail closed
  for (a_using_decl_ptr u = sp->using_directives; u; u = u->next)
    if (u->entity.kind == iek_namespace &&
        namespace_reaches_cerastes(reinterpret_cast<a_namespace_ptr>(u->entity.ptr), name, depth + 1, visited))
      return true;
  for (a_using_decl_ptr u = sp->using_declarations; u; u = u->next) {
    a_source_correspondence* scp = nullptr;
    if (u->entity.kind == iek_routine) scp = &reinterpret_cast<a_routine_ptr>(u->entity.ptr)->source_corresp;
    else if (u->entity.kind == iek_template) scp = &reinterpret_cast<a_template_ptr>(u->entity.ptr)->source_corresp;
    if (scp && scp->name && std::strcmp(scp->name, name) == 0 && scope_is_cerastes_namespace(scp->parent_scope))
      return true;
  }
  return false;
}

// The specification function that a dependent callee names, or null.  This
// fails CLOSED (CP7 audit): once namespace cerastes is declared in the
// translation unit,
// - an UNQUALIFIED `old(x)` (or owns, reads, forall, owns_range) is taken to be
//   the specification function, whatever using-directives are in effect where
//   it is written (argument-dependent lookup of a user function with the same
//   name is a documented limitation: qualify it);
// - a QUALIFIED name is the specification function unless its qualifier is a
//   namespace from which qualified lookup cannot reach cerastes (`mine::old`),
//   or a class (`S::old`, a member).
// Without namespace cerastes in the translation unit nothing is one.
const char* dependent_spec_callee(a_constant_ptr c) {
  a_constant_ptr k = unknown_callee_constant(c);
  if (k == nullptr || !is_spec_name(k->source_corresp.name)) return nullptr;
  if (!cerastes_namespace_in_tu()) return nullptr;
  const char* name = k->source_corresp.name;
  // A template_ref's `con` need not be a template-parameter constant (CP7 #11):
  // read is_qualified_name only when it is; otherwise treat it as qualified and
  // let the qualifier decide (failing closed when it is unknown).
  const bool qualified = k->kind == ck_template_param ? k->variant.template_param.is_qualified_name : true;
  if (!qualified) return name;
  a_scope_ptr scope = k->source_corresp.parent_scope;
  if (scope == nullptr) return name;  // unknown qualifier: fail closed
  if (scope->kind == sck_namespace || scope->kind == sck_namespace_extension) {
    std::set<a_namespace_ptr> visited;
    return namespace_reaches_cerastes(scope->variant.assoc_namespace, name, 0, visited) ? name : nullptr;
  }
  if (scope->kind == sck_file) return name;  // `::old`: fail closed
  return nullptr;  // a class member
}

bool is_function_template(a_template_ptr t) {
  return t->kind == templk_function || t->kind == templk_member_function;
}

// ---- Helpers -----------------------------------------------------------------

bool is_instance(a_routine_ptr r) {
  return !r->is_prototype_instantiation && (r->is_template_function || r->assoc_template != nullptr);
}

// `detected during instantiation of "inc<int>"`, for a diagnostic found in an
// instance; empty otherwise.
std::string instance_name(a_routine_ptr r) {
  std::string args;
  for (a_template_arg_ptr a = r->template_arg_list; a; a = a->next) {
    if (a->kind == tak_start_of_pack_expansion) continue;
    if (!args.empty()) args += ", ";
    args += render_template_arg(a);
  }
  std::string n = rname(r);
  if (!args.empty()) n += "<" + args + ">";
  return n;
}

std::string instance_context(a_routine_ptr r) {
  if (r == nullptr || !is_instance(r)) return "";
  return "detected during instantiation of \"" + instance_name(r) + "\"";
}

a_type_ptr return_type_of(a_routine_ptr r) {
  a_type_ptr rt = r->type ? skip_typerefs(r->type) : nullptr;
  return (rt && rt->kind == tk_routine) ? rt->variant.routine.return_type : nullptr;
}

void add_scope_variables(a_scope_ptr sp, std::set<std::string>* names, std::set<a_variable_ptr>* vars) {
  auto each = [&](a_variable_ptr list) {
    for (a_variable_ptr v = list; v; v = v->next) {
      if (names && v->source_corresp.name) names->insert(v->source_corresp.name);
      if (vars) vars->insert(v);
    }
  };
  each(sp->nonstatic_variables);
  each(sp->variables);
  if (sp->kind == sck_function) each(sp->variant.routine.parameters);
  for (a_scope_ptr c = sp->scopes; c; c = c->next)
    if (c->kind == sck_block || c->kind == sck_condition) add_scope_variables(c, names, vars);
}

bool is_member_of_class(a_scope_ptr sp) { return sp != nullptr && sp->kind == sck_class_struct_union; }

// Whether the class has a member named `name`: a data member, a static data
// member, a member function, a type or an enumerator declared in its scope.
bool class_has_member(a_type_ptr cls, const char* name) {
  if (cls == nullptr || name == nullptr) return false;
  a_type_ptr t = skip_typerefs(cls);
  if (!is_immediate_class_type(t)) return false;
  for (a_field_ptr f = t->variant.class_struct_union.field_list; f; f = f->next)
    if (f->source_corresp.name && std::strcmp(f->source_corresp.name, name) == 0) return true;
  a_class_type_supplement_ptr x = t->variant.class_struct_union.extra_info;
  a_scope_ptr sp = x ? x->assoc_scope : nullptr;
  if (sp == nullptr) return false;
  for (a_variable_ptr v = sp->variables; v; v = v->next)
    if (v->source_corresp.name && std::strcmp(v->source_corresp.name, name) == 0) return true;
  for (a_routine_ptr r = sp->routines; r; r = r->next)
    if (r->source_corresp.name && std::strcmp(r->source_corresp.name, name) == 0) return true;
  for (a_constant_ptr c = sp->constants; c; c = c->next)
    if (c->source_corresp.name && std::strcmp(c->source_corresp.name, name) == 0) return true;
  for (a_type_ptr ty = sp->types; ty; ty = ty->next)
    if (ty->source_corresp.name && std::strcmp(ty->source_corresp.name, name) == 0) return true;
  return false;
}

// The position of the first member of `cls` named `name` (as class_has_member
// finds it), or a null position.
a_source_position first_member_position(a_type_ptr cls, const char* name) {
  a_source_position none = null_source_position;
  if (cls == nullptr || name == nullptr) return none;
  a_type_ptr t = skip_typerefs(cls);
  if (!is_immediate_class_type(t)) return none;
  a_source_position best = none;
  auto consider = [&](const a_source_correspondence& sc) {
    if (sc.name && std::strcmp(sc.name, name) == 0 && sc.decl_position.seq != 0 &&
        (best.seq == 0 || sc.decl_position.seq < best.seq ||
         (sc.decl_position.seq == best.seq && sc.decl_position.column < best.column)))
      best = sc.decl_position;
  };
  for (a_field_ptr f = t->variant.class_struct_union.field_list; f; f = f->next) consider(f->source_corresp);
  a_class_type_supplement_ptr x = t->variant.class_struct_union.extra_info;
  a_scope_ptr sp = x ? x->assoc_scope : nullptr;
  if (sp != nullptr) {
    for (a_variable_ptr v = sp->variables; v; v = v->next) consider(v->source_corresp);
    for (a_routine_ptr r = sp->routines; r; r = r->next) consider(r->source_corresp);
    for (a_constant_ptr c = sp->constants; c; c = c->next) consider(c->source_corresp);
    for (a_type_ptr ty = sp->types; ty; ty = ty->next) consider(ty->source_corresp);
  }
  return best;
}

// Whether the class declares a member named `name` that the name written at
// `at` could not see (contract_name_hides_member): one declared at or after
// `at`.  A member declared before `at` was visible there, so the name was
// written qualified (`::limit`, CP7).  The position compared is the name's own
// (CP8: not the routine's, which is the definition's for a member defined
// outside its class); equal or unknown positions fail closed.
bool class_declares_member_later(a_type_ptr cls, const char* name, const a_source_position& at) {
  if (!class_has_member(cls, name)) return false;
  a_source_position m = first_member_position(cls, name);
  if (m.seq == 0 || at.seq == 0) return true;  // unknown: fail closed
  return m.seq > at.seq || (m.seq == at.seq && m.column >= at.column);
}

// The object of a member access with casts (the implicit `const S *` of `this`
// in a const member function), conversions to a base class (an inherited
// member, CP7), `*` (`(*this).i`), `&` (the copy of `*this` captured by a
// `[*this]` lambda, CP8b) and the accesses to anonymous members
// (`this-><unnamed>.i`, and a lambda's capture fields, CP8) peeled off.
an_expr_node_ptr peel_member_object(an_expr_node_ptr obj) {
  for (int guard = 0; obj != nullptr && obj->kind == enk_operation && guard < 1024; ++guard) {
    const auto& o = obj->variant.operation;
    if (o.operands == nullptr) break;
    const bool anonymous_member = (o.kind == eok_dot_field || o.kind == eok_points_to_field) &&
                                  o.operands->next != nullptr && o.operands->next->kind == enk_field &&
                                  o.operands->next->variant.field.ptr != nullptr &&
                                  o.operands->next->variant.field.ptr->source_corresp.name == nullptr;
    if (o.kind != eok_cast && o.kind != eok_base_class_cast && o.kind != eok_indirect && o.kind != eok_address_of &&
        !anonymous_member)
      break;
    obj = o.operands;
  }
  return obj;
}

// Whether a member reached through `obj` (peeled) is named unqualified: the
// object is `this`, or an unnamed object (the members of an anonymous union).
bool object_named_unqualified(an_expr_node_ptr obj) {
  obj = peel_member_object(obj);
  if (obj == nullptr) return false;
  if (obj->kind == enk_param_ref) return obj->variant.param_ref.param_num == 0;
  if (obj->kind == enk_variable && obj->variant.variable.ptr != nullptr)
    return obj->variant.variable.ptr->is_this_parameter || obj->variant.variable.ptr->source_corresp.name == nullptr;
  return false;
}

// The routine a constant names by address: `&f` (ck_address) or `&S::f`, a pointer to a member function
// (ck_ptr_to_member, R4b), or null.
a_routine_ptr address_constant_routine(a_constant_ptr c) {
  if (c == nullptr) return nullptr;
  if (c->kind == ck_address && c->variant.address.kind == abk_routine) return c->variant.address.variant.routine;
  if (c->kind == ck_ptr_to_member && c->variant.ptr_to_member.is_function_ptr)
    return c->variant.ptr_to_member.variant.routine;
  return nullptr;
}

// ---- R4d, R4e: where the specification functions are declared, and `old` -------------------------------

// R4d (cerastes_header_in_namespace): a specification function declared in a namespace `cerastes` that is not
// the global one (`namespace foo { #include <cerastes.h> }`).  Its callers name `foo::cerastes::owns`, which
// no check of this file recognizes as a specification function, so the call would be read as ordinary code.
// Reported where such a function is named.
bool is_spec_name_in_nested_cerastes_namespace(a_routine_ptr callee) {
  if (callee == nullptr || !is_spec_name(callee->source_corresp.name)) return false;
  a_scope_ptr sp = callee->source_corresp.parent_scope;
  if (sp == nullptr || (sp->kind != sck_namespace && sp->kind != sck_namespace_extension)) return false;
  a_namespace_ptr ns = resolve_alias(sp->variant.assoc_namespace);
  if (ns == nullptr || ns->source_corresp.name == nullptr || std::strcmp(ns->source_corresp.name, "cerastes") != 0)
    return false;
  a_scope_ptr parent = ns->source_corresp.parent_scope;
  return parent != nullptr && parent->kind != sck_file;
}

void check_namespaced_spec(an_expr_node_ptr e) {
  if (e->kind != enk_routine) return;
  a_routine_ptr callee = e->variant.routine.ptr;
  if (is_spec_name_in_nested_cerastes_namespace(callee))
    report(Diag::cerastes_header_in_namespace, e->position, {rname(callee)});
}

// R4e (old_outside_ensures): `cerastes::old` is for `ensures` only.  In the body of a predicate or pure
// function it is meaningless too (there is no entry state to name): a call, its address, or a dependent
// name that may turn out to be it.
void check_old_in_spec_body(an_expr_node_ptr e) {
  if (e->kind == enk_routine) {
    a_routine_ptr callee = e->variant.routine.ptr;
    if (callee != nullptr && rname(callee) == "cerastes::old") report(Diag::old_outside_ensures, e->position);
  } else if (e->kind == enk_constant) {
    const char* callee = dependent_spec_callee(e->variant.constant.ptr);
    if (callee != nullptr && std::strcmp(callee, "old") == 0) report(Diag::old_outside_ensures, e->position);
    a_routine_ptr by_address = address_constant_routine(e->variant.constant.ptr);
    if (by_address != nullptr && rname(by_address) == "cerastes::old") report(Diag::old_outside_ensures, e->position);
  }
}

// ---- Scanning an annotation argument -----------------------------------------

struct Scan {
  a_type_ptr member_class = nullptr;  // contract of a member function: its class
  std::set<std::pair<int, std::string>> reported;  // one diagnostic per kind and text per attribute
  Annot annot = Annot::none;
  a_routine_ptr owner = nullptr;
  std::string context;
  std::set<std::string> loop_names;        // invariant: names the loop declares
  std::set<a_variable_ptr> lambda_vars;    // variables declared in lambdas of the argument
  std::set<an_expr_node_ptr> seen;
  std::set<a_lambda_ptr> lambdas;
  // Member functions called through an object other than `this` (`v.size()`):
  // named after `.`/`->`, never unqualified (CP7), so no shadowing check.
  std::set<an_expr_node_ptr> object_member_callees;
};
Scan* g_scan = nullptr;
bool g_error_injected = false;  // --cerastes_checks_selftest
bool g_type_injected = false;
// Lambdas written in annotation arguments: their bodies are annotation code,
// not ordinary code (spec_call_in_code).
std::set<a_routine_ptr> g_annotation_lambda_routines;
std::set<a_lambda_ptr> g_lambdas_checked;
// The call operators of every lambda seen: a contract cannot be written on one,
// so its attributes are reported once, as annotation_on_lambda.
std::set<a_routine_ptr> g_lambda_routines;
// R4a: the expression nodes that may name a specification function: those of the argument trees of registered
// attributes (lambdas in them included) and of the bodies of specification functions.  Every other node that
// names one is ordinary code.
std::set<an_expr_node_ptr> g_exempt_nodes;
// Nodes that a body walk already recorded as a use of a specification function (reported or, for the body of a
// lambda in an annotation, excused).
std::set<an_expr_node_ptr> g_handled_nodes;
// The same for constants (`&cerastes::old<int>` in a specification body or an annotation).
std::set<a_constant_ptr> g_exempt_constants;

void note_lambda(a_lambda_ptr l);

// Reports from inside a scan: once per (diagnostic, text) per attribute, so that
// `0 <= i && i <= n && c == i` is one diagnostic about `i`, not three.
void scan_report(Scan& sc, Diag d, const a_source_position& at, std::vector<std::string> args = {}) {
  std::string key;
  for (const std::string& a : args) key += a + "\x01";
  if (!sc.reported.insert({static_cast<int>(d), key}).second) return;
  report(d, at, args, sc.context);
}

const char* side_effect_of(an_expr_operator_kind k) {
  switch (k) {
    case eok_post_incr:
    case eok_post_decr:
    case eok_pre_incr:
    case eok_pre_decr: return "an increment or decrement";
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
    case eok_padd_assign:
    case eok_psubtract_assign:
    case eok_bassign: return "an assignment";
    default: return nullptr;
  }
}

constexpr unsigned long kBudget = 1000000;
// The budget of shared expressions a traversal may follow.  The self-test lowers
// it to one, so that the first folded constant (an unnamed constant that records
// the expression it came from, such as `1 + 1`) exhausts it and the limit is
// reported (analysis_limit_reached).
unsigned long budget() { return options().checks_selftest ? 1 : kBudget; }

void scan_lambda_body(a_lambda_ptr l, an_expr_or_stmt_traversal_block_ptr tb, const a_source_position& at) {
  a_routine_ptr r = l ? l->lambda_routine : nullptr;
  if (r == nullptr) return;
  a_scope_ptr sp = body_scope(r);
  if (sp != nullptr) {
    add_scope_variables(sp, nullptr, &g_scan->lambda_vars);
    if (sp->assoc_block) traverse_statement(sp->assoc_block, tb);
  }
  // The lambda's own signature: the rescan of a lambda in a template's contract (fork series L/M) can leave an
  // error type in a parameter written `decltype(<function parameter>)` without any diagnostic, and a body that
  // does not use the parameter has no node of that type.  So the return type and every parameter type are checked
  // too, not only the nodes of the body.  The body is scanned first: when it already reported a substitution error
  // for this attribute (an erroneous node or type), that error is the one reported (once per attribute).
  bool reported_substitution_error = false;
  for (const auto& rep : g_scan->reported)
    if (rep.first == static_cast<int>(Diag::annotation_substitution_error)) reported_substitution_error = true;
  if (reported_substitution_error) return;
  a_type_ptr rt = r->type ? skip_typerefs(r->type) : nullptr;
  if (rt == nullptr || rt->kind != tk_routine) return;
  bool bad = rt->variant.routine.return_type != nullptr && is_error_type(rt->variant.routine.return_type);
  a_routine_type_supplement_ptr x = rt->variant.routine.extra_info;
  for (a_param_type_ptr p = x ? x->param_type_list : nullptr; p; p = p->next)
    if (p->type != nullptr && is_error_type(p->type)) bad = true;
  if (bad) scan_report(*g_scan, Diag::annotation_substitution_error, at, {"error type"});
}

void scan_on_expr(an_expr_node_ptr e, an_expr_or_stmt_traversal_block_ptr tb) {
  Scan& sc = *g_scan;
  if (!sc.seen.insert(e).second) {
    tb->suppress_subtree_walk = TRUE;
    return;
  }
  g_exempt_nodes.insert(e);
  check_namespaced_spec(e);
  // An erroneous node: the front end gives up on an argument it cannot
  // substitute (for example a lambda in an instance's contract) without always
  // reporting it.  Never read such an annotation as meaning anything.
  an_expr_node_kind seen_kind = static_cast<an_expr_node_kind>(e->kind);
  if (options().checks_selftest && !g_error_injected) {
    // Self-test: the first node of the first annotation is read as an error node,
    // to prove that the check fires (no source reaches it).
    g_error_injected = true;
    seen_kind = enk_error;
  }
  if (seen_kind == enk_error)
    scan_report(sc, Diag::annotation_substitution_error, e->position, {"error expression"});
  else if (options().checks_selftest && !g_type_injected && g_error_injected) {
    g_type_injected = true;  // self-test: the second node has an error type
    scan_report(sc, Diag::annotation_substitution_error, e->position, {"error type"});
  } else if (e->type != nullptr && is_error_type(e->type))
    scan_report(sc, Diag::annotation_substitution_error, e->position, {"error type"});
  // A read of a volatile object: the glvalue-to-prvalue conversion clears
  // is_lvalue and keeps the glvalue's type in orig_lvalue_type (il_def.h:15003).
  if (e->orig_lvalue_type != nullptr && !e->is_lvalue && !e->is_xvalue &&
      is_volatile_qualified_type(e->orig_lvalue_type))
    scan_report(sc, Diag::annotation_side_effect, e->position, {"a read of a volatile object"});
  switch (e->kind) {
    case enk_operation: {
      const auto& op = e->variant.operation;
      if (const char* what = side_effect_of(op.kind))
        scan_report(sc, Diag::annotation_side_effect, e->position, {what});
      // A member named through `this` (implicitly or not) that has the name of a
      // variable the loop declares: the loop's variable cannot be what it names.
      if ((op.kind == eok_dot_field || op.kind == eok_points_to_field) && op.operands != nullptr &&
          op.operands->next != nullptr && op.operands->next->kind == enk_field) {
        a_field_ptr f = op.operands->next->variant.field.ptr;
        if (f && f->source_corresp.name && sc.loop_names.count(f->source_corresp.name) &&
            object_named_unqualified(op.operands))
          scan_report(sc, Diag::invariant_shadowed_by_loop_variable, e->position, {f->source_corresp.name});
      }
      if ((op.kind == eok_dot_member_call || op.kind == eok_points_to_member_call) && op.operands != nullptr &&
          op.operands->next != nullptr && !object_named_unqualified(op.operands->next)) {
        an_expr_node_ptr callee = op.operands;
        if (callee->kind == enk_operation && callee->variant.operation.operands != nullptr)
          callee = callee->variant.operation.operands;  // an address of the routine
        if (callee->kind == enk_routine) sc.object_member_callees.insert(callee);
      }
      if (is_call_kind(op.kind) && sc.annot != Annot::ensures) {
        a_routine_ptr callee = direct_callee(e);
        if (callee && rname(callee) == "cerastes::old") scan_report(sc, Diag::old_outside_ensures, e->position);
      }
      break;
    }
    case enk_new_delete:
      scan_report(sc, Diag::annotation_side_effect, e->position, {"a new or delete expression"});
      break;
    case enk_throw:
      scan_report(sc, Diag::annotation_side_effect, e->position, {"a throw expression"});
      break;
    case enk_statement:
      scan_report(sc, Diag::statement_expression_in_annotation, e->position);
      break;
    case enk_routine: {
      a_routine_ptr r = e->variant.routine.ptr;
      const char* n = r ? r->source_corresp.name : nullptr;
      if (sc.object_member_callees.count(e)) break;
      if (n && sc.loop_names.count(n))
        scan_report(sc, Diag::invariant_shadowed_by_loop_variable, e->position, {n});
      if (n && sc.member_class && !is_member_of_class(r->source_corresp.parent_scope) &&
          class_declares_member_later(sc.member_class, n, e->position))
        scan_report(sc, Diag::contract_name_hides_member, e->position, {n, n});
      break;
    }
    case enk_constant: {
      a_constant_ptr nc = e->variant.constant.ptr;
      // A named constant (an enumerator, a named constant) with the name of a
      // variable the loop declares, or of a member the class declares later.
      if (nc != nullptr && nc->kind != ck_template_param && nc->source_corresp.name != nullptr) {
        const char* n = nc->source_corresp.name;
        if (sc.loop_names.count(n))
          scan_report(sc, Diag::invariant_shadowed_by_loop_variable, e->position, {n});
        if (sc.member_class && !is_member_of_class(nc->source_corresp.parent_scope) &&
            class_declares_member_later(sc.member_class, n, e->position))
          scan_report(sc, Diag::contract_name_hides_member, e->position, {n, n});
      }
      // A call of `cerastes::old` that depends on a template parameter is a
      // constant that names the callee (tpck_unknown_function, or tpck_template_ref
      // for old<T>(x)); in an instance the call is resolved and found above.
      const char* callee = dependent_spec_callee(e->variant.constant.ptr);
      if (sc.annot != Annot::ensures && callee != nullptr && std::strcmp(callee, "old") == 0)
        scan_report(sc, Diag::old_outside_ensures, e->position);
      break;
    }
    case enk_variable: {
      a_variable_ptr v = e->variant.variable.ptr;
      if (v && v->source_corresp.name && sc.loop_names.count(v->source_corresp.name) &&
          !sc.lambda_vars.count(v))
        scan_report(sc, Diag::invariant_shadowed_by_loop_variable, e->position, {v->source_corresp.name});
      if (v && v->source_corresp.name && sc.member_class && !sc.lambda_vars.count(v) &&
          !is_member_of_class(v->source_corresp.parent_scope) && v->source_corresp.enclosing_routine == nullptr &&
          class_declares_member_later(sc.member_class, v->source_corresp.name, e->position))
        scan_report(sc, Diag::contract_name_hides_member, e->position, {v->source_corresp.name, v->source_corresp.name});
      break;
    }
    case enk_lambda: {
      a_lambda_ptr l = e->variant.init.source.lambda;
      if (l && (l->is_generic || l->has_template_param_list))
        scan_report(sc, Diag::generic_lambda_in_annotation, e->position);
      if (l && sc.lambdas.insert(l).second) {
        if (l->lambda_routine) g_annotation_lambda_routines.insert(l->lambda_routine);
        note_lambda(l);
        scan_lambda_body(l, tb, e->position);
      }
      break;
    }
    default: break;
  }
}

// Follow the expression a folded constant records: `({ ... })` or `x = 1, 2`
// may have been folded to a constant, and the front end keeps what it came
// from.  Named constants (enumerators) are written elsewhere.
void scan_on_constant(a_constant_ptr c, an_expr_or_stmt_traversal_block_ptr tb) {
  g_exempt_constants.insert(c);
  if (c->expr != nullptr && c->source_corresp.name == nullptr) {
    if (tb->shared_expr_budget != 0 && --tb->shared_expr_budget == 0) {
      tb->terminate = TRUE;
      return;
    }
    traverse_expr(c->expr, tb);
    if (c->kind == ck_template_param && c->variant.template_param.kind == tpck_expression &&
        c->variant.template_param.variant.expr != nullptr && c->variant.template_param.variant.expr != c->expr)
      traverse_expr(c->variant.template_param.variant.expr, tb);
    tb->suppress_subtree_walk = TRUE;
  }
}

void scan_expression(an_expr_node_ptr root, Scan& sc) {
  if (root == nullptr) return;
  Scan* saved = g_scan;
  g_scan = &sc;
  an_expr_or_stmt_traversal_block tb;
  clear_expr_or_stmt_traversal_block(&tb);
  tb.process_expr = scan_on_expr;
  tb.process_constant = scan_on_constant;
  tb.process_non_dynamic_constants = TRUE;
  tb.process_template_parameter_constants_and_expressions = TRUE;
  tb.process_expressions_for_constants = FALSE;
  tb.shared_expr_budget = budget();
  traverse_expr(root, &tb);
  if (tb.terminate && tb.shared_expr_budget == 0)
    scan_report(sc, Diag::analysis_limit_reached, root->position, {"the budget of shared expressions"});
  g_scan = saved;
}

// ---- fold / unfold -----------------------------------------------------------

an_expr_node_ptr strip_to_call_candidate(an_expr_node_ptr e) {
  for (int guard = 0; e && guard < 32; ++guard) {
    if (e->kind == enk_operation && e->variant.operation.kind == eok_parens && e->variant.operation.operands) {
      e = e->variant.operation.operands;
    } else if (e->kind == enk_constant && e->variant.constant.ptr && e->variant.constant.ptr->expr &&
               e->variant.constant.ptr->source_corresp.name == nullptr &&
               e->variant.constant.ptr->kind != ck_template_param) {
      e = e->variant.constant.ptr->expr;  // a call of a constexpr predicate, folded
    } else {
      break;
    }
  }
  return e;
}

bool dependent(an_expr_node_ptr e) {
  if (e == nullptr) return false;
  if (e->type != nullptr && is_template_dependent_type(e->type)) return true;
  return e->kind == enk_constant && e->variant.constant.ptr && e->variant.constant.ptr->kind == ck_template_param;
}

void check_fold(an_attribute_ptr a, an_expr_node_ptr root, const std::string& context) {
  const std::string what = a->name ? a->name : "fold";
  an_expr_node_ptr e = strip_to_call_candidate(root);
  if (e == nullptr) return;
  const bool is_call = e->kind == enk_operation && is_call_kind(e->variant.operation.kind) &&
                       !e->variant.operation.call_uses_operator_syntax;
  if (!is_call) {
    if (dependent(e)) return;  // decided in each instance
    report(Diag::fold_argument_not_call, e->position, {what}, context);
    return;
  }
  a_routine_ptr callee = direct_callee(e);
  if (callee == nullptr) {
    an_expr_node_ptr first = e->variant.operation.operands;
    if (dependent(e) || dependent(first)) return;  // a dependent callee: decided in each instance
    report(Diag::fold_argument_not_call, e->position, {what}, context);
    return;
  }
  if (!marked(callee, Annot::predicate)) report(Diag::fold_callee_not_predicate, e->position, {rname(callee), what}, context);
}

// ---- Attributes ----------------------------------------------------------------

void check_attribute(an_attribute_ptr a, a_routine_ptr owner, a_statement_ptr stmt) {
  Annot k = classify(a);
  if (k == Annot::none) return;
  if (k == Annot::owning) return;  // a parameter's marker (decision 0040 item 3): the encoder reads it; its argument is no annotation expression
  const std::string context = instance_context(owner);
  // `decreases` before a loop is the loop's measure (like `invariant`); on a function it is a contract.
  const bool function_form = is_contract(k) && stmt == nullptr;
  if (function_form && a->syntactic_location != al_post_func)
    report(Diag::contract_not_after_parameters, a->position, {a->name}, context);

  Scan sc;
  sc.annot = k;
  sc.owner = owner;
  sc.context = context;
  if (function_form && owner != nullptr && is_member_of_class(owner->source_corresp.parent_scope))
    sc.member_class = owner->source_corresp.parent_scope->variant.assoc_type;
  if ((k == Annot::invariant || k == Annot::decreases) && stmt != nullptr) {
    // A variable declared in the condition of a while or for loop.
    if ((stmt->kind == stmk_while || stmt->kind == stmk_for) && stmt->expr != nullptr &&
        stmt->expr->kind == enk_condition && stmt->expr->variant.condition != nullptr &&
        stmt->expr->variant.condition->scope != nullptr)
      add_scope_variables(stmt->expr->variant.condition->scope, &sc.loop_names, nullptr);
    if (stmt->kind == stmk_for) {
      a_for_loop_ptr f = stmt->variant.for_loop.extra_info;
      if (f && f->for_init_scope) add_scope_variables(f->for_init_scope, &sc.loop_names, nullptr);
    } else if (stmt->kind == stmk_range_based_for) {
      a_range_based_for_loop_ptr f = stmt->variant.range_based_for_loop.extra_info;
      if (f && f->iterator && f->iterator->source_corresp.name) sc.loop_names.insert(f->iterator->source_corresp.name);
      if (f && f->range_based_for_scope) {
        std::set<std::string> names;
        add_scope_variables(f->range_based_for_scope, &names, nullptr);
        for (const std::string& n : names)
          if (n != "__range" && n != "__begin" && n != "__end") sc.loop_names.insert(n);
      }
    }
  }
  for (const AnnotArg& g : arguments_of(a)) {
    if (g.kind != AnnotArg::Kind::expression) continue;
    if (g.expr == nullptr) {
      scan_report(sc, Diag::annotation_substitution_error, g.raw->position, {"no expression"});
      continue;
    }
    scan_expression(g.expr, sc);
    if (k == Annot::fold || k == Annot::unfold) check_fold(a, g.expr, context);
  }
  // Fail closed (invariant 3): a part of the tree that the printer has no line
  // for, a parameter reference that the convention does not explain, a shared
  // node, a tree too deep, a lambda that is not a single return.
  bool side_effect_reported = false;
  for (const auto& r : sc.reported)
    if (r.first == static_cast<int>(Diag::annotation_side_effect)) side_effect_reported = true;
  for (const TreeProblem& p : annotation_tree(a, owner, TreeMode::check, nullptr, 0).problems) {
    if (p.explained_by_side_effect && side_effect_reported) continue;
    scan_report(sc, Diag::unsupported_annotation_expression, p.at, {p.what});
  }
}

// ---- Lambdas -------------------------------------------------------------------

// A cerastes attribute written on a lambda (`[] [[cerastes::requires(..)]] () {}`
// attaches to the closure's operator()): a lambda cannot carry a contract.
void note_lambda(a_lambda_ptr l) {
  if (!g_lambdas_checked.insert(l).second) return;
  a_routine_ptr r = l->lambda_routine;
  if (r == nullptr) return;
  g_lambda_routines.insert(r);
  for (an_attribute_ptr a = r->source_corresp.attributes; a; a = a->next)
    if (is_registered_cerastes(a)) report(Diag::annotation_on_lambda, a->position, {a->name ? a->name : "?"});
}

// ---- Routines ------------------------------------------------------------------

// Whether a parameter pack of `r` is followed by another parameter.  The
// self-test perturbs the data, not the test: it appends a parameter to the list.
bool has_pack_before_last(a_routine_ptr r) {
  a_type_ptr rt = r->type ? skip_typerefs(r->type) : nullptr;
  if (rt == nullptr || rt->kind != tk_routine) return false;
  a_routine_type_supplement_ptr x = rt->variant.routine.extra_info;
  std::vector<bool> is_pack;
  for (a_param_type_ptr p = x ? x->param_type_list : nullptr; p; p = p->next) is_pack.push_back(p->is_parameter_pack);
  if (options().checks_selftest && !is_pack.empty()) is_pack.push_back(false);
  for (size_t i = 0; i + 1 < is_pack.size(); ++i)
    if (is_pack[i]) return true;
  return false;
}

// The line and column of the first `module` declaration of the primary source
// file (`module m;`, `export module m;`, `module m:p;`).  The front end drops
// the declaration's position before the back end runs, so the text is read
// again; line 1 if it cannot be found.
DiagPos module_declaration_position() {
  DiagPos p;
  a_source_file_ptr f = il_header.primary_source_file;
  if (f == nullptr || f->full_name == nullptr) return p;
  p.known = true;
  p.file = f->file_name ? f->file_name : f->full_name;
  p.full_name = f->full_name;
  p.line = 1;
  p.col = 0;
  std::ifstream in(f->full_name);
  std::string line;
  auto word = [&](size_t& i, const char* w) {
    size_t n = std::strlen(w);
    if (line.compare(i, n, w) != 0) return false;
    if (i + n < line.size() && (std::isalnum(static_cast<unsigned char>(line[i + n])) || line[i + n] == '_')) return false;
    i += n;
    return true;
  };
  for (unsigned long n = 1; std::getline(in, line); ++n) {
    size_t i = 0;
    while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i]))) ++i;
    const size_t start = i;
    if (word(i, "export")) {
      while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i]))) ++i;
    }
    if (!word(i, "module")) continue;
    while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i]))) ++i;
    if (i < line.size() && (std::isalpha(static_cast<unsigned char>(line[i])) || line[i] == '_')) {
      p.line = n;
      p.col = start + 1;
      break;
    }
  }
  return p;
}

// C++20 modules (0006 item 10): contracts on imported entities are not carried
// through module files, so a module unit or an import is rejected.  A module
// unit is recognised by trans_unit_module (trans_unit.h:257), which the front
// end sets once it has seen a module declaration.
void check_modules() {
  if (trans_unit_module != nullptr && (trans_unit_module->kind == mk_unit || trans_unit_module->kind == mk_unit_partition))
    report_at(Diag::modules_unsupported, module_declaration_position());
  for (a_module_import_decl_ptr i = il_header.imported_modules; i; i = i->next)
    if (!i->impl_unit_importing_self) report(Diag::modules_unsupported, i->position);
}


// A source-level `#undef __CERASTES__` or `#define __CERASTES__` in any file read
// (decision 0008): the front end defines the macro itself, and user code that
// tests it would see another value.  (<cerastes.h> does not depend on it, so no
// annotation is lost; this is about the macro the program sees.)  The macro table
// is gone when the back end runs, so the text of the files is read: logical lines
// (line splices removed), comments blanked, the digraph `%:` read as `#`; a
// directive of that shape is an error even in a skipped #if block.  A file that
// cannot be read, or an include tree deeper than the cap, is an error too (the
// check could not be made).
constexpr int kIncludeDepthCap = 256;

void report_scan_incomplete(const std::string& why, a_source_file_ptr f) {
  DiagPos p;
  p.known = true;
  p.line = 1;
  p.col = 0;
  p.file = f && f->file_name ? f->file_name : "";
  p.full_name = f && f->full_name ? f->full_name : "";
  report_at(Diag::source_scan_incomplete, p, {why});
}

void scan_file_for_macro_change(a_source_file_ptr f) {
  if (f == nullptr) return;
  if (f->full_name == nullptr) return;  // a generated entry (#line): no file of its own
  std::ifstream in(f->full_name, std::ios::binary);
  if (!in) {
    report_scan_incomplete(std::string("cannot read \"") + f->full_name + "\"", f);
    return;
  }
  const std::string raw((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

  // Translation phase 2: backslash-newline splices are removed.
  std::string spliced;
  spliced.reserve(raw.size());
  for (size_t i = 0; i < raw.size(); ++i) {
    if (raw[i] == '\\' && i + 1 < raw.size() && raw[i + 1] == '\n') { ++i; continue; }
    if (raw[i] == '\\' && i + 2 < raw.size() && raw[i + 1] == '\r' && raw[i + 2] == '\n') { i += 2; continue; }
    spliced += raw[i];
  }
  if (spliced.find("__CERASTES__") == std::string::npos) return;  // the common case

  // The same text with the position (line, column) of every character in the
  // original file, so that a directive is reported where it is written.
  std::vector<std::pair<unsigned long, unsigned long>> where;
  std::string text;
  {
    unsigned long line = 1, col = 1;
    where.reserve(raw.size());
    for (size_t i = 0; i < raw.size(); ++i) {
      if (raw[i] == '\\' && i + 1 < raw.size() && raw[i + 1] == '\n') { ++line; col = 1; ++i; continue; }
      if (raw[i] == '\\' && i + 2 < raw.size() && raw[i + 1] == '\r' && raw[i + 2] == '\n') { ++line; col = 1; i += 2; continue; }
      text += raw[i];
      where.push_back({line, col});
      if (raw[i] == '\n') { ++line; col = 1; } else { ++col; }
    }
  }
  // Translation phase 3: comments become one space (a block comment may span
  // lines: the directive continues after it); the newlines inside it go.
  std::string code;
  std::vector<std::pair<unsigned long, unsigned long>> cwhere;
  for (size_t i = 0; i < text.size(); ++i) {
    if (text.compare(i, 2, "/*") == 0) {
      size_t end = text.find("*/", i + 2);
      code += ' ';
      cwhere.push_back(where[i]);
      i = end == std::string::npos ? text.size() : end + 1;
    } else if (text.compare(i, 2, "//") == 0) {
      while (i < text.size() && text[i] != '\n') ++i;
      --i;  // the newline stays
    } else {
      code += text[i];
      cwhere.push_back(where[i]);
    }
  }
  // Directives: logical lines of the comment-free text.
  size_t pos = 0;
  while (pos < code.size()) {
    size_t eol = code.find('\n', pos);
    if (eol == std::string::npos) eol = code.size();
    const std::string line = code.substr(pos, eol - pos);
    const size_t base = pos;
    pos = eol + 1;
    if (line.find("__CERASTES__") == std::string::npos) continue;
    size_t i = 0;
    auto skip_ws = [&] {
      while (i < line.size() && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r' || line[i] == '\f' || line[i] == '\v')) ++i;
    };
    skip_ws();
    const size_t hash = i;
    if (i < line.size() && line[i] == '#') ++i;
    else if (line.compare(i, 2, "%:") == 0) i += 2;
    else continue;
    skip_ws();
    const char* what = nullptr;
    if (line.compare(i, 5, "undef") == 0) { what = "undefined"; i += 5; }
    else if (line.compare(i, 6, "define") == 0) { what = "redefined"; i += 6; }
    if (what == nullptr || i >= line.size() || !std::isspace(static_cast<unsigned char>(line[i]))) continue;
    skip_ws();
    if (line.compare(i, 12, "__CERASTES__") != 0) continue;
    if (i + 12 < line.size() && (std::isalnum(static_cast<unsigned char>(line[i + 12])) || line[i + 12] == '_')) continue;
    DiagPos p;
    p.known = true;
    p.file = f->file_name ? f->file_name : f->full_name;
    p.full_name = f->full_name;
    p.line = cwhere[base + hash].first;
    p.col = cwhere[base + hash].second;
    report_at(Diag::cerastes_macro_changed, p, {what});
  }
}

void scan_files_for_macro_change(a_source_file_ptr f, int depth) {
  for (; f != nullptr; f = f->next) {
    if (depth >= kIncludeDepthCap) {
      report_scan_incomplete("the include tree is deeper than " + std::to_string(kIncludeDepthCap) + " levels", f);
      return;
    }
    scan_file_for_macro_change(f);
    scan_files_for_macro_change(f->first_child_file, depth + 1);
  }
}

void check_macro_unchanged() {
  a_source_file_ptr primary = il_header.primary_source_file;
  if (primary == nullptr) return;
  scan_file_for_macro_change(primary);
  scan_files_for_macro_change(primary->first_child_file, 0);
}

// ---- R4c: a #define or #undef of a name the annotations depend on (decision 0008 item 2) -----------
// The preprocessor's own record (RECORD_MACROS_IN_IL): every #define and #undef that was in an active
// group, with its position.  A directive outside the real <cerastes.h> of this checkout that defines or
// undefines
//  * `cerastes`, an attribute name, a specification-function name, `CERASTES_H` or a `CERASTES_*` macro
//    (the header's own macros),
//  * or any predefined macro (`#define ensures requires` turned postconditions into preconditions;
//    `#undef __cplusplus` made the header expand to nothing).  That includes `__has_cpp_attribute` in C++
//    and `__has_c_attribute` in C, the feature tests that decide whether the header expands the macros.
//    They are protected by being predefined, not by name: in the other language a system header
//    (sys/cdefs.h) defines the one that is not (`#ifndef __has_cpp_attribute`) and it means nothing there,
// is an error.  A macro of the command line has no position: it is reported at none (the drivers refuse
// such options themselves; this keeps the front end honest when run directly).
const std::string& real_cerastes_header() {
  static const std::string path = canonical_path(std::string(CERASTES_SOURCE_INCLUDE_DIR) + "/cerastes.h");
  return path;
}

bool is_protected_macro_name(const std::string& n, const std::set<std::string>& predefined) {
  static const char* const fixed[] = {"cerastes",  "requires", "ensures", "invariant", "check",
                                      "fold",      "unfold",   "predicate", "pure",     "decreases", "old",
                                      "owns",      "reads",    "live",    "forall",  "owns_range", "CERASTES_H",
                                      "cerastes_owns", "cerastes_reads", "cerastes_live", "cerastes_old_", "cerastes_old"};
  for (const char* f : fixed)
    if (n == f) return true;
  if (n.compare(0, 9, "CERASTES_") == 0) return true;
  return predefined.count(n) != 0;
}

std::string macro_name(a_macro_ptr m) {
  if (m->source_corresp.name != nullptr) return m->source_corresp.name;
  // from the text: "#define NAME..." or "#undef NAME"
  if (m->text == nullptr) return "";
  const char* t = m->text;
  t += std::strncmp(t, "#undef", 6) == 0 ? 6 : std::strncmp(t, "#define", 7) == 0 ? 7 : 0;
  while (*t == ' ' || *t == '\t') ++t;
  const char* e = t;
  while (std::isalnum(static_cast<unsigned char>(*e)) || *e == '_') ++e;
  return std::string(t, e);
}

// The logical file record of a directive (a `#pragma GCC system_header` starts a record of its own), or null.
a_source_file_ptr directive_file(a_macro_ptr m) {
  if (m->is_command_line_definition || m->source_corresp.decl_position.seq == 0) return nullptr;
  a_const_char* file = nullptr;
  a_const_char* full = nullptr;
  a_line_number line = 0;
  a_boolean at_end = FALSE;
  return conv_seq_to_file_and_line(m->source_corresp.decl_position.seq, &file, &full, &line, &at_end);
}

// A directive in a system file: EDG's flag on its file record (a --sys_include directory, or the pragma from its line
// on), which is what walk mode calls a system file everywhere (render.cpp); verify mode also requires 0011 item 4's rule
// by path, under which /usr/local/include, the -I directories and a user file that a system header includes are user
// files (audit, S5).
bool directive_in_system_file(a_macro_ptr m) {
  a_source_file_ptr f = directive_file(m);
  if (f == nullptr || !f->from_system_include_dir) return false;
  return !verify_config().on || !in_verified_file(m->source_corresp.decl_position);
}

// Whether a file record (an inclusion, or a pragma's record) starts strictly between two positions of the unit.
bool file_starts_between(a_source_file_ptr f, a_seq_number after, a_seq_number before) {
  for (; f != nullptr; f = f->next) {
    if (f->first_seq_number > after && f->first_seq_number < before) return true;
    if (file_starts_between(f->first_child_file, after, before)) return true;
  }
  return false;
}

void check_macros_in_il() {
  std::set<std::string> predefined;
  std::map<std::string, a_macro_ptr> predefinition;  // the last predefinition of each name: the state user code starts in
  for (a_macro_ptr m = il_header.macros; m != nullptr; m = m->next)
    if (m->is_predefined && !m->is_undef) {
      const std::string n = macro_name(m);
      if (!n.empty()) predefined.insert(n), predefinition[n] = m;
    }
  // S5 (decision 0008 item 2, amended by record 0031 item 9): in a system file, an #undef of a predefined macro whose
  // next directive for that name is a #define of exactly the predefinition's text, in the same inclusion and with no
  // file included in between, is no change, and neither is that #define.  glibc's <stdint.h> undefines __INT64_C and
  // __UINT64_C and defines them again as the compiler did.  EDG's text is normalized (`c##L` and `c ## L` both read
  // "c ## L"; string literals are kept as written), so the comparison is exact.
  auto is_predefinition_text = [&](a_macro_ptr m, const std::string& n) {
    auto p = predefinition.find(n);
    return p != predefinition.end() && !m->is_undef && m->object_like == p->second->object_like && m->text != nullptr &&
           p->second->text != nullptr && p->second->text[0] != '\0' && std::strcmp(m->text, p->second->text) == 0;
  };
  std::set<a_macro_ptr> no_change;
  for (a_macro_ptr m = il_header.macros; m != nullptr; m = m->next) {
    if (m->is_predefined || !m->is_undef || !directive_in_system_file(m)) continue;
    const std::string n = macro_name(m);
    if (n.empty() || predefinition.count(n) == 0) continue;
    a_macro_ptr next = m->next;
    while (next != nullptr && macro_name(next) != n) next = next->next;
    if (next == nullptr || !is_predefinition_text(next, n) || !directive_in_system_file(next)) continue;
    a_source_file_ptr f = directive_file(m);
    if (f == nullptr || directive_file(next) != f) continue;  // the same inclusion, the same record
    const a_seq_number from = m->source_corresp.decl_position.seq, to = next->source_corresp.decl_position.seq;
    if (to <= from || file_starts_between(il_header.primary_source_file, from, to)) continue;
    no_change.insert(m);
    no_change.insert(next);
  }
  const std::string& header = real_cerastes_header();
  for (a_macro_ptr m = il_header.macros; m != nullptr; m = m->next) {
    if (m->is_predefined && !m->is_undef) continue;  // the predefinition itself
    const std::string n = macro_name(m);
    if (n.empty() || !is_protected_macro_name(n, predefined)) continue;
    // `-D__CERASTES__` (the front end's own definition) has its own check.
    if (n == "__CERASTES__") continue;
    if (no_change.count(m) != 0) continue;
    if (!m->is_command_line_definition) {
      const std::string where = physical_path(m->source_corresp.decl_position);
      if (!header.empty() && where == header) continue;
    }
    report(Diag::annotation_macro_changed, m->source_corresp.decl_position, {n, m->is_undef ? "undefined" : "defined"});
  }
}

void check_routine_level(a_routine_ptr r, const std::map<a_routine_ptr, a_template_ptr>& proto_owner) {
  const std::string name = rname(r);
  const std::string context = instance_context(r);
  an_attribute_ptr predicate = nullptr, pure = nullptr;
  for (an_attribute_ptr a = r->source_corresp.attributes; a; a = a->next) {
    if (classify(a) == Annot::predicate && !predicate) predicate = a;
    if (classify(a) == Annot::pure && !pure) pure = a;
  }
  const bool is_pred = predicate != nullptr || marked(r, Annot::predicate);
  const bool is_pure = pure != nullptr || marked(r, Annot::pure);
  a_type_ptr ret = return_type_of(r);
  const bool dependent_ret = ret != nullptr && is_template_dependent_type(ret);

  // The marker of a specification function is a promise about the whole function: it goes on a function of
  // the user's own (not one declared in a system file, such as a library function, or built in), and never on a
  // special member function.  (Instances take the template's marks; they are not checked again.)  That the marker
  // must be on the first declaration of the function is not decidable from the IL: the front end keeps no list
  // of the declarations, and `on_primary_declaration` is the definition's (see the 0006 note on item 10).
  if (!is_instance(r)) {
    const SrcPos decl = resolve(r->source_corresp.decl_position);
    const bool library = !decl.known || decl.in_system;
    for (an_attribute_ptr a = r->source_corresp.attributes; a; a = a->next) {
      const Annot k = classify(a);
      if (k != Annot::predicate && k != Annot::pure) continue;
      const char* what = a->name ? a->name : "?";
      const bool special = r->special_kind == sfk_constructor || r->special_kind == sfk_destructor ||
                           (r->source_corresp.name != nullptr && std::strcmp(r->source_corresp.name, "operator=") == 0);
      if (special) report(Diag::spec_marker_on_special_member, a->position, {what, name}, context);
      const SrcPos where = resolve(a->position);
      if (library || (where.known && where.in_system)) report(Diag::spec_marker_in_system_file, a->position, {what, name}, context);
    }
  }

  // Where a diagnostic about the mark points: the attribute on this routine, or
  // the routine's declaration when the mark comes from the root template only.
  const a_source_position& mark_pos = predicate ? predicate->position : (pure ? pure->position : r->source_corresp.decl_position);
  if (predicate && pure) report(Diag::predicate_and_pure, (pure->position.seq > predicate->position.seq ? pure : predicate)->position, {name}, context);
  if (is_pred && ret != nullptr && !dependent_ret && !is_bool_type(skip_typerefs(ret)))
    report(Diag::predicate_not_bool, mark_pos, {name, render_type(ret)}, context);
  if (is_pure && !is_pred && ret != nullptr && is_void_type(skip_typerefs(ret)))
    report(Diag::pure_returns_void, pure ? pure->position : r->source_corresp.decl_position, {name}, context);
  // Self-test: pretend that the return type is void (the data, not the test).
  a_type_ptr ret_for_result_name = options().checks_selftest ? void_type() : ret;

  for (an_attribute_ptr a = r->source_corresp.attributes; a; a = a->next) {
    Annot k = classify(a);
    if (is_contract(k)) {
      if (is_pred) report(Diag::contract_on_predicate, a->position, {a->name, name}, context);
      if (k == Annot::ensures && ret_for_result_name != nullptr && is_void_type(skip_typerefs(ret_for_result_name))) {
        for (const AnnotArg& g : arguments_of(a))
          if (g.kind == AnnotArg::Kind::binder) report(Diag::result_name_on_void, g.raw->position, {g.binder_name ? g.binder_name : "?", name}, context);
      }
      if (has_pack_before_last(r)) report(Diag::parameter_after_pack, a->position, {name}, context);
    }
  }

  // The contract of an instance, specialization or member-template prototype
  // comes from its root template (0006 item 6).
  // Only for instances and specializations: the prototype of a member template
  // of a class instance (S<long>::g) has no contract of its own when the
  // contract is written on the definition only, which is the usual case; the
  // walker shows it with `contract from root template`.
  (void)proto_owner;
  // An inheriting constructor has no contract of its own: it is the inherited
  // constructor (friends_or_originator.inherited_routine), whose contract applies.
  if (!r->is_prototype_instantiation && !r->is_inheriting_ctor) {
    a_routine_ptr root = root_routine(r);
    if (root != nullptr && !contracts_agree(root, r, /*perturb=*/options().checks_selftest))
      report(Diag::contract_not_propagated, r->source_corresp.decl_position, {rname(root), is_instance(r) ? instance_name(r) : name}, context);
  }
}

// ---- Bodies --------------------------------------------------------------------

// A call of a specification function from a routine body.  It is an error
// unless the body is that of a specification function, or of a lambda written in
// an annotation; the second is known only after every annotation is scanned.
struct SpecCall {
  a_routine_ptr caller;
  a_source_position at;
  std::string callee;
};
std::vector<SpecCall> g_spec_calls;

struct BodyState {
  a_source_position pos{};        // where the constants of an initializer are reported
  a_routine_ptr routine = nullptr;
  bool spec_caller = false;       // the routine is itself a specification function
  std::set<a_statement_ptr> statements;
  std::set<an_expr_node_ptr> seen;
};
BodyState* g_body = nullptr;
std::set<a_statement_ptr> g_checked_statements;  // statements with annotations checked in a body

// The address of a specification function (a constant of kind ck_address on a
// routine: `&p` in an initializer, or a template argument) in ordinary code: a
// call through the pointer would go unseen.
void note_spec_address_constant(a_constant_ptr c, const a_source_position& at) {
  a_routine_ptr r = address_constant_routine(c);
  if (r == nullptr || !is_spec_routine(r) || at.seq == 0) return;
  g_spec_calls.push_back({g_body ? g_body->routine : nullptr, at, rname(r)});
}

// A routine named in ordinary code whose template arguments include the address
// of a specification function (`call<&p>(x)`).
void note_spec_template_arguments(a_routine_ptr callee, const a_source_position& at) {
  for (a_template_arg_ptr a = callee->template_arg_list; a; a = a->next)
    if (a->kind == tak_nontype && !a->is_array_bound_of_unknown_type) note_spec_address_constant(a->variant.constant, at);
}

void body_on_statement(a_statement_ptr s, an_expr_or_stmt_traversal_block_ptr) {
  if (!g_body->statements.insert(s).second) return;
  for (an_attribute_ptr a = s->attributes; a; a = a->next)
    if (is_registered_cerastes(a)) {
      g_checked_statements.insert(s);
      check_attribute(a, g_body->routine, s);
    }
}

void body_on_expr(an_expr_node_ptr e, an_expr_or_stmt_traversal_block_ptr tb) {
  BodyState& b = *g_body;
  if (!b.seen.insert(e).second) {
    tb->suppress_subtree_walk = TRUE;
    return;
  }
  if (e->kind == enk_lambda) {
    a_lambda_ptr l = e->variant.init.source.lambda;
    if (l) note_lambda(l);
    return;
  }
  if (b.spec_caller) {
    g_exempt_nodes.insert(e);
    check_old_in_spec_body(e);
    return;
  }
  // A reference to a specification function in ordinary code: a call, and also
  // taking its address (a call through the pointer would go unseen).  The routine
  // is found by entity, whatever its spelling (using, qualified or not).
  if (e->kind == enk_routine) {
    a_routine_ptr callee = e->variant.routine.ptr;
    if (callee == nullptr) return;
    if (is_spec_routine(callee)) {
      g_spec_calls.push_back({b.routine, e->position, rname(callee)});
      g_handled_nodes.insert(e);
    }
    note_spec_template_arguments(callee, e->position);
  } else if (e->kind == enk_constant) {
    // A dependent call of a specification function in a template that may never be
    // instantiated: only its name is known.
    const char* callee = dependent_spec_callee(e->variant.constant.ptr);
    if (callee != nullptr) g_spec_calls.push_back({b.routine, e->position, std::string("cerastes::") + callee});
    note_spec_address_constant(e->variant.constant.ptr, e->position);
  }
}

void body_on_constant(a_constant_ptr c, an_expr_or_stmt_traversal_block_ptr tb) {
  if (g_body->spec_caller) {
    g_exempt_constants.insert(c);
    a_routine_ptr by_address = address_constant_routine(c);
    if (by_address != nullptr && rname(by_address) == "cerastes::old")
      report(Diag::old_outside_ensures, c->source_corresp.decl_position);
  } else {
    note_spec_address_constant(c, g_body->pos);
  }
  // Only expressions of this body: a call site's copy of a callee's default
  // argument records a file-scope expression.
  if (c->expr != nullptr && c->source_corresp.name == nullptr && !in_file_scope(c->expr)) {
    if (tb->shared_expr_budget != 0 && --tb->shared_expr_budget == 0) {
      tb->terminate = TRUE;
      return;
    }
    traverse_expr(c->expr, tb);
    tb->suppress_subtree_walk = TRUE;
  }
}

void init_traversal(an_expr_or_stmt_traversal_block* tb) {
  clear_expr_or_stmt_traversal_block(tb);
  tb->process_statement = body_on_statement;
  tb->process_expr = body_on_expr;
  tb->process_constant = body_on_constant;
  tb->process_non_dynamic_constants = TRUE;
  tb->process_template_parameter_constants_and_expressions = TRUE;
  tb->process_expressions_for_constants = FALSE;
  tb->shared_expr_budget = budget();
}

// The body of a routine and the expressions that run with it: constructor
// mem-initializers and default arguments.
void check_body(a_routine_ptr r) {
  a_scope_ptr sp = body_scope(r);
  BodyState b;
  b.routine = r;
  b.spec_caller = marked(r, Annot::predicate) || marked(r, Annot::pure);
  BodyState* saved = g_body;
  g_body = &b;
  an_expr_or_stmt_traversal_block tb;
  init_traversal(&tb);
  a_type_ptr rt = r->type ? skip_typerefs(r->type) : nullptr;
  if (rt && rt->kind == tk_routine && rt->variant.routine.extra_info) {
    for (a_param_type_ptr p = rt->variant.routine.extra_info->param_type_list; p; p = p->next)
      if (p->default_arg_expr) traverse_expr(p->default_arg_expr, &tb);
  }
  if (sp != nullptr) {
    for (a_constructor_init_ptr ci = sp->variant.routine.constructor_inits; ci; ci = ci->next) {
      if (ci->initializer) traverse_dynamic_init(ci->initializer, &tb);
      if (ci->source_expr) traverse_expr(ci->source_expr, &tb);
    }
    if (sp->assoc_block != nullptr) traverse_statement(sp->assoc_block, &tb);
  }
  if (tb.terminate && tb.shared_expr_budget == 0)
    report(Diag::analysis_limit_reached, r->source_corresp.decl_position, {"the budget of shared expressions in \"" + rname(r) + "\""});
  g_body = saved;
}

// The initializers outside any routine: variables of namespace and class scope,
// and default member initializers.
void check_initializers() {
  BodyState b;
  BodyState* saved = g_body;
  g_body = &b;
  an_expr_or_stmt_traversal_block tb;
  init_traversal(&tb);
  for (a_variable_ptr v : g_c.variables) {
    SrcPos pos = resolve(v->source_corresp.decl_position);
    if (!pos.known || pos.in_system || v->compiler_generated) continue;
    b.pos = v->source_corresp.decl_position;
    if (v->init_kind == initk_dynamic && v->initializer.dynamic != nullptr)
      traverse_dynamic_init(v->initializer.dynamic, &tb);
    else if (v->init_kind == initk_static && v->initializer.constant != nullptr)
      traverse_constant(v->initializer.constant, &tb);
  }
  for (a_field_ptr f : g_c.fields) {
    SrcPos pos = resolve(f->source_corresp.decl_position);
    if (!pos.known || pos.in_system) continue;
    b.pos = f->source_corresp.decl_position;
    if (f->initializer != nullptr) traverse_dynamic_init(f->initializer, &tb);
  }
  if (tb.terminate && tb.shared_expr_budget == 0)
    report(Diag::analysis_limit_reached, null_source_position, {"the budget of shared expressions in the initializers"});
  g_body = saved;
}

// ---- R4a/R4b: every use of a specification function outside annotations and specification bodies -----
// The walks above find the uses in bodies and initializers.  A use can also hang off a type (an array bound,
// `decltype`), a field (a bit-field width), a template parameter (a default argument), an exception
// specification, an attribute argument that is not a cerastes one (`alignas`, `explicit`), an enumerator, a
// block-scope `static_assert` or a constant variable's initializer: all of them constant-evaluated contexts, which
// are ordinary code (il-inventory, "R4a").  So one more pass looks at every expression node of the IL that names a
// routine, wherever it hangs, and every class instance whose template arguments name one by address.  A node
// is excused only if it lies under the arguments of a registered cerastes attribute (lambdas included) or in the
// body of a specification function: those nodes are marked first.  Calls the walks found were reported there.
void mark_on_expr(an_expr_node_ptr e, an_expr_or_stmt_traversal_block_ptr tb) {
  if (!g_exempt_nodes.insert(e).second) {
    tb->suppress_subtree_walk = TRUE;
    return;
  }
  if (e->kind == enk_lambda) {
    a_lambda_ptr l = e->variant.init.source.lambda;
    a_routine_ptr lr = l ? l->lambda_routine : nullptr;
    a_scope_ptr sp = lr ? body_scope(lr) : nullptr;
    if (sp != nullptr && sp->assoc_block != nullptr) traverse_statement(sp->assoc_block, tb);
  }
}

void mark_on_constant(a_constant_ptr c, an_expr_or_stmt_traversal_block_ptr tb) {
  g_exempt_constants.insert(c);
  if (c->expr != nullptr && c->source_corresp.name == nullptr) {
    if (tb->shared_expr_budget != 0 && --tb->shared_expr_budget == 0) {
      tb->terminate = TRUE;
      return;
    }
    traverse_expr(c->expr, tb);
    tb->suppress_subtree_walk = TRUE;
  }
}

void init_mark_traversal(an_expr_or_stmt_traversal_block* tb) {
  clear_expr_or_stmt_traversal_block(tb);
  tb->process_expr = mark_on_expr;
  tb->process_constant = mark_on_constant;
  tb->process_non_dynamic_constants = TRUE;
  tb->process_template_parameter_constants_and_expressions = TRUE;
  tb->process_expressions_for_constants = FALSE;
  tb->shared_expr_budget = budget();
}

void mark_attribute_arguments(an_attribute_ptr a) {
  if (!is_registered_cerastes(a)) return;
  for (const AnnotArg& g : arguments_of(a)) {
    if (g.kind != AnnotArg::Kind::expression || g.expr == nullptr) continue;
    an_expr_or_stmt_traversal_block tb;
    init_mark_traversal(&tb);
    traverse_expr(g.expr, &tb);
    if (tb.terminate && tb.shared_expr_budget == 0)
      report(Diag::analysis_limit_reached, g.expr->position, {"the budget of shared expressions in an annotation"});
  }
}

void mark_spec_body(a_routine_ptr r) {
  a_scope_ptr sp = body_scope(r);
  an_expr_or_stmt_traversal_block tb;
  init_mark_traversal(&tb);
  a_type_ptr rt = r->type ? skip_typerefs(r->type) : nullptr;
  if (rt && rt->kind == tk_routine && rt->variant.routine.extra_info)
    for (a_param_type_ptr p = rt->variant.routine.extra_info->param_type_list; p; p = p->next)
      if (p->default_arg_expr) traverse_expr(p->default_arg_expr, &tb);
  if (sp != nullptr) {
    for (a_constructor_init_ptr ci = sp->variant.routine.constructor_inits; ci; ci = ci->next) {
      if (ci->initializer) traverse_dynamic_init(ci->initializer, &tb);
      if (ci->source_expr) traverse_expr(ci->source_expr, &tb);
    }
    if (sp->assoc_block != nullptr) traverse_statement(sp->assoc_block, &tb);
  }
  if (tb.terminate && tb.shared_expr_budget == 0)
    report(Diag::analysis_limit_reached, r->source_corresp.decl_position, {"the budget of shared expressions in \"" + rname(r) + "\""});
}

void mark_exempt_nodes() {
  for (an_attribute_ptr a : g_c.cerastes_attributes) mark_attribute_arguments(a);
  for (a_routine_ptr r : g_c.routines) {
    for (an_attribute_ptr a = r->source_corresp.attributes; a; a = a->next) mark_attribute_arguments(a);
    if (marked(r, Annot::predicate) || marked(r, Annot::pure)) mark_spec_body(r);
  }
  for (a_statement_ptr st : g_c.annotated_statements)
    for (an_attribute_ptr a = st->attributes; a; a = a->next) mark_attribute_arguments(a);
}

// The class type a variable's type reaches through typedefs, arrays, pointers and references, or null.
a_type_ptr class_behind(a_type_ptr t) {
  for (int guard = 0; t != nullptr && guard < 32; ++guard) {
    t = skip_typerefs(t);
    if (t->kind == tk_array) t = t->variant.array.element_type;
    else if (t->kind == tk_pointer) t = t->variant.pointer.type;
    else return t;
  }
  return nullptr;
}

// is_spec_routine for a routine named by many nodes: decided once, and the (costly) qualified name is
// rendered only for a routine that has the name of a specification function.
bool is_spec_routine_cached(a_routine_ptr r) {
  static std::map<a_routine_ptr, bool> cache;
  auto it = cache.find(r);
  if (it != cache.end()) return it->second;
  bool result = marked(r, Annot::predicate) || marked(r, Annot::pure) ||
                (is_spec_name(r->source_corresp.name) && is_spec_routine(r));
  cache[r] = result;
  return result;
}

void check_all_spec_references() {
  // What the body walks reported already, by line: a use also seen here as a node or a constant (the operand of
  // `&f` inside the constant that the walk reported at the initializer) is the same use.
  std::set<std::tuple<std::string, unsigned long, std::string>> done;
  for (const SpecCall& c : g_spec_calls) {
    SrcPos p = resolve(c.at);
    if (p.known) done.insert({p.file, p.line, c.callee});
  }
  auto fresh = [&](const a_source_position& at, const std::string& callee) {
    SrcPos p = resolve(at);
    return !p.known || done.insert({p.file, p.line, callee}).second;
  };
  for (an_expr_node_ptr e : g_c.routine_nodes) {
    check_namespaced_spec(e);
    a_routine_ptr callee = e->variant.routine.ptr;
    if (callee == nullptr || !is_spec_routine_cached(callee)) continue;
    if (g_exempt_nodes.count(e) || g_handled_nodes.count(e)) continue;
    // A node with no position is compiler-generated (a copy of a template argument or a default argument whose
    // original, written in the source, has its own node and position).
    if (e->position.seq == 0) continue;
    if (fresh(e->position, rname(callee))) report(Diag::spec_call_in_code, e->position, {rname(callee)});
  }
  // `&f` and `&S::f` (R4b): constants that name a specification function, wherever they are used.
  for (a_constant_ptr c : g_c.address_constants) {
    a_routine_ptr r = address_constant_routine(c);
    if (r == nullptr || !is_spec_routine_cached(r) || c->source_corresp.decl_position.seq == 0) continue;
    if (g_exempt_constants.count(c)) continue;
    if (fresh(c->source_corresp.decl_position, rname(r))) report(Diag::spec_call_in_code, c->source_corresp.decl_position, {rname(r)});
  }
  // `V<&nonneg>`, `W<&S::ok>`: the address of a specification function as a template argument of a class.  The
  // class instance has the position of its template, so the use is reported where a variable of that type is
  // declared; an instance no variable has is reported at its template.
  std::map<a_type_ptr, std::string> flagged;
  for (a_type_ptr t : g_c.class_instances) {
    for (a_template_arg_ptr a = t->variant.class_struct_union.extra_info->template_arg_list; a; a = a->next) {
      if (a->kind != tak_nontype || a->is_array_bound_of_unknown_type) continue;
      a_routine_ptr r = address_constant_routine(a->variant.constant);
      if (r != nullptr && is_spec_routine_cached(r)) flagged[t] = rname(r);
    }
  }
  std::set<a_type_ptr> placed;
  for (a_variable_ptr v : g_c.all_variables) {
    a_type_ptr t = class_behind(v->type);
    auto it = t ? flagged.find(t) : flagged.end();
    // compiler-generated variables (the parameters of implicit members) have no position
    if (it == flagged.end() || v->source_corresp.decl_position.seq == 0) continue;
    placed.insert(t);
    if (fresh(v->source_corresp.decl_position, it->second))
      report(Diag::spec_call_in_code, v->source_corresp.decl_position, {it->second});
  }
  std::set<std::string> placed_callees;
  for (a_type_ptr t : placed) placed_callees.insert(flagged[t]);
  for (const auto& f : flagged) {
    if (placed.count(f.first)) continue;
    // Another type entry of an instance already placed (a qualified or reference variant, with no position)
    if (f.first->source_corresp.decl_position.seq == 0 && placed_callees.count(f.second)) continue;
    if (fresh(f.first->source_corresp.decl_position, f.second))
      report(Diag::spec_call_in_code, f.first->source_corresp.decl_position, {f.second});
  }
}

// ---- R2-1: an explicit specialization of a member template written inside its class ------------------------
// (`template <> int f<int>(int)` in the class template, CWG 727).  When the class template is never instantiated
// EDG checks nothing about it, so a contract that differs from the primary's, one that is added, and one that
// is dropped would all go unseen.  Such a specialization is rejected when it, or the member template of the
// same name in its class, carries a contract (consistent with decision 0006 item 6: explicit specializations of
// member templates with a contract are not supported).  Reported at the specialization.
bool has_contract(a_routine_ptr r) { return !contracts_of(r).empty(); }

void check_in_class_specialization(a_routine_ptr r) {
  if (!r->is_in_class_specialization) return;
  bool contract = has_contract(r);
  const char* name = r->source_corresp.name;
  a_scope_ptr cs = r->source_corresp.parent_scope;
  if (!contract && name != nullptr && cs != nullptr && cs->kind == sck_class_struct_union) {
    for (a_routine_ptr m = cs->routines; m != nullptr && !contract; m = m->next)
      if (m != r && !m->is_in_class_specialization && m->source_corresp.name != nullptr &&
          std::strcmp(m->source_corresp.name, name) == 0 && has_contract(m))
        contract = true;
    if (!contract) {
      // the primary may be a template that is only reachable through the root template of this routine
      a_routine_ptr root = root_routine(r);
      if (root != nullptr && root != r && has_contract(root)) contract = true;
    }
  }
  if (contract) report(Diag::in_class_specialization_contract, r->source_corresp.decl_position, {rname(r)});
}

}  // namespace

void check_annotations() {
  g_c = Collected();
  g_proto_owner.clear();
  g_annotation_lambda_routines.clear();
  g_lambdas_checked.clear();
  g_exempt_nodes.clear();
  g_handled_nodes.clear();
  g_exempt_constants.clear();
  walk_file_scope_il(collect, nullptr, nullptr, nullptr, nullptr, /*clear_fe_pointers=*/FALSE);
  for (a_memory_region_number region = FILE_SCOPE_REGION_NUMBER + 1; region <= highest_used_region_number; ++region) {
    if (mem_region_table[region] == nullptr) continue;
    walk_routine_scope_il(region, collect, nullptr, nullptr, nullptr, nullptr, /*clear_fe_pointers=*/FALSE);
  }
  for (a_template_ptr t : g_c.templates) {
    if (!is_function_template(t) || t->canonical_template != t) continue;
    a_template_ptr def = t->definition_template;
    if (def && def->prototype_instantiation.routine) g_proto_owner[def->prototype_instantiation.routine] = def;
  }

  // Prototypes and plain routines first, instances after: a diagnostic found in
  // both is recorded once, without the instantiation context.
  std::vector<a_routine_ptr> order;
  for (int pass = 0; pass < 2; ++pass)
    for (a_routine_ptr r : g_c.routines)
      if ((pass == 0) == !is_instance(r)) order.push_back(r);

  // Every routine that carries a cerastes attribute is checked wherever it is
  // (a system header, a #pragma GCC system_header, a linemarker, a redeclaration of
  // a library function); routines without one are skipped in system files, and the
  // implicit ones.
  std::vector<a_routine_ptr> checked;
  for (a_routine_ptr r : order) {
    SrcPos pos = resolve(r->source_corresp.decl_position);
    const bool attributed = has_cerastes_attribute(r);
    if (!pos.known) {
      if (!attributed) continue;
      // An annotated routine that cannot be placed in the source: checked, and an
      // error (it cannot be vouched for).
      for (an_attribute_ptr a = r->source_corresp.attributes; a; a = a->next)
        if (is_registered_cerastes(a)) {
          report(Diag::unsupported_annotation_expression, a->position, {"an annotated routine that has no source position"});
          break;
        }
    } else if (!attributed && (pos.in_system || r->compiler_generated)) {
      continue;
    }
    checked.push_back(r);
  }
  // Bodies first: they hold the statement annotations, and show which routines
  // are lambdas.  Then the routines' own annotations, then the calls of
  // specification functions, which need the lambdas of annotations found by the
  // scans before.
  g_spec_calls.clear();
  g_checked_statements.clear();
  mark_exempt_nodes();
  for (a_routine_ptr r : checked) check_body(r);
  // Statements with an annotation that no body walk reached (a body in a system
  // file that is not checked).
  for (a_statement_ptr st : g_c.annotated_statements) {
    if (g_checked_statements.count(st)) continue;
    for (an_attribute_ptr a = st->attributes; a; a = a->next)
      if (is_registered_cerastes(a)) check_attribute(a, nullptr, st);
  }
  check_initializers();
  for (a_routine_ptr r : checked) {
    // The call operator of a lambda cannot carry a contract: found from the IL (the
    // closure class), not by where the lambda is written.
    if (r->is_lambda_body || g_lambda_routines.count(r)) {
      for (an_attribute_ptr a = r->source_corresp.attributes; a; a = a->next)
        if (is_registered_cerastes(a)) report(Diag::annotation_on_lambda, a->position, {a->name ? a->name : "?"});
      continue;
    }
    check_routine_level(r, g_proto_owner);
    for (an_attribute_ptr a = r->source_corresp.attributes; a; a = a->next)
      if (is_registered_cerastes(a)) check_attribute(a, r, nullptr);
  }
  for (const SpecCall& c : g_spec_calls)
    if (!g_annotation_lambda_routines.count(c.caller)) report(Diag::spec_call_in_code, c.at, {c.callee}, instance_context(c.caller));
  check_all_spec_references();
  for (a_routine_ptr r : g_c.routines) check_in_class_specialization(r);
  check_modules();
  check_macro_unchanged();
  check_macros_in_il();
}

}  // namespace cerastes
