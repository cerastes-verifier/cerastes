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
#include <vector>

#include "annotation_il.h"
#include "decls.h"
#include "diagnostics.h"
#include "expr_tree.h"
#include "il_names.h"
#include "options.h"
#include "render.h"

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
    if (v->source_corresp.enclosing_routine == nullptr) g_c.variables.push_back(v);
  } else if (kind == iek_field) {
    g_c.fields.push_back(reinterpret_cast<a_field_ptr>(entry));
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

// The name of the callee that a dependent call (a template-parameter constant of
// kind tpck_unknown_function, or tpck_template_ref for `f<T>(x)`) records, or null.
const char* unknown_callee_name(a_constant_ptr c) {
  if (c == nullptr || c->kind != ck_template_param) return nullptr;
  const auto& tp = c->variant.template_param;
  if (tp.kind == tpck_unknown_function) return c->source_corresp.name;
  if (tp.kind == tpck_template_ref && tp.variant.template_ref.con != nullptr)
    return tp.variant.template_ref.con->source_corresp.name;
  return nullptr;
}

// The specification functions of <cerastes.h> (namespace cerastes), by the name a
// dependent call records: the lookup is done in each instance, where the entity
// itself is known (is_spec_function_name); a never-instantiated template is
// checked by name, so that a spelling (using cerastes::old, old<T>(x)) cannot hide one.
bool is_spec_name(const char* n) {
  return n != nullptr && (std::strcmp(n, "old") == 0 || std::strcmp(n, "owns") == 0 || std::strcmp(n, "reads") == 0 ||
                          std::strcmp(n, "forall") == 0 || std::strcmp(n, "owns_range") == 0);
}

bool is_function_template(a_template_ptr t) {
  return t->kind == templk_function || t->kind == templk_member_function;
}

// ---- Helpers -----------------------------------------------------------------

std::string rname(a_routine_ptr r) { return render_name(&r->source_corresp, iek_routine); }

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

bool is_spec_function_name(const std::string& n) {
  return n == "cerastes::owns" || n == "cerastes::reads" || n == "cerastes::old" ||
         n == "cerastes::forall" || n == "cerastes::owns_range";
}

// A routine marked predicate or pure, on its own attribute list or its root
// template's (an instance of a marked template).
bool marked(a_routine_ptr r, Annot k) {
  if (has_annot(r, k)) return true;
  a_routine_ptr root = root_routine(r);
  return root != nullptr && has_annot(root, k);
}

// The routine called by a call expression whose callee is named directly, or
// null.
a_routine_ptr direct_callee(an_expr_node_ptr e) {
  if (e == nullptr || e->kind != enk_operation) return nullptr;
  const auto& op = e->variant.operation;
  if (op.kind != eok_call && op.kind != eok_dot_member_call && op.kind != eok_points_to_member_call)
    return nullptr;
  if (op.operands == nullptr || op.operands->kind != enk_routine) return nullptr;
  return op.operands->variant.routine.ptr;
}

bool is_call_kind(an_expr_operator_kind k) {
  return k == eok_call || k == eok_dot_member_call || k == eok_points_to_member_call;
}

a_scope_ptr body_scope(a_routine_ptr r) {
  if (r->function_def_number == NULL_function_def_number) return nullptr;
  a_scope_ptr sp = scope_for_routine_or_null(r);
  if (sp == nullptr || sp->kind != sck_function || sp->variant.routine.ptr != r) return nullptr;
  return sp;
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

// ---- Scanning an annotation argument -----------------------------------------

struct Scan {
  std::set<std::pair<int, std::string>> reported;  // one diagnostic per kind and text per attribute
  Annot annot = Annot::none;
  a_routine_ptr owner = nullptr;
  std::string context;
  std::set<std::string> loop_names;        // invariant: names the loop declares
  std::set<a_variable_ptr> lambda_vars;    // variables declared in lambdas of the argument
  std::set<an_expr_node_ptr> seen;
  std::set<a_lambda_ptr> lambdas;
};
Scan* g_scan = nullptr;
bool g_error_injected = false;  // --cerastes_checks_selftest
// Lambdas written in annotation arguments: their bodies are annotation code,
// not ordinary code (spec_call_in_code).
std::set<a_routine_ptr> g_annotation_lambda_routines;
std::set<a_lambda_ptr> g_lambdas_checked;
// The call operators of every lambda seen: a contract cannot be written on one,
// so its attributes are reported once, as annotation_on_lambda.
std::set<a_routine_ptr> g_lambda_routines;

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

void scan_lambda_body(a_lambda_ptr l, an_expr_or_stmt_traversal_block_ptr tb) {
  a_routine_ptr r = l ? l->lambda_routine : nullptr;
  if (r == nullptr) return;
  a_scope_ptr sp = body_scope(r);
  if (sp == nullptr) return;
  add_scope_variables(sp, nullptr, &g_scan->lambda_vars);
  if (sp->assoc_block) traverse_statement(sp->assoc_block, tb);
}

void scan_on_expr(an_expr_node_ptr e, an_expr_or_stmt_traversal_block_ptr tb) {
  Scan& sc = *g_scan;
  if (!sc.seen.insert(e).second) {
    tb->suppress_subtree_walk = TRUE;
    return;
  }
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
  else if (e->type != nullptr && is_error_type(e->type))
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
    case enk_constant: {
      // A call of `cerastes::old` that depends on a template parameter is a
      // constant that names the callee (tpck_unknown_function, or tpck_template_ref
      // for old<T>(x)); in an instance the call is resolved and found above.
      const char* callee = unknown_callee_name(e->variant.constant.ptr);
      if (sc.annot != Annot::ensures && callee != nullptr && std::strcmp(callee, "old") == 0)
        scan_report(sc, Diag::old_outside_ensures, e->position);
      break;
    }
    case enk_variable: {
      a_variable_ptr v = e->variant.variable.ptr;
      if (v && v->source_corresp.name && sc.loop_names.count(v->source_corresp.name) &&
          !sc.lambda_vars.count(v))
        scan_report(sc, Diag::invariant_shadowed_by_loop_variable, e->position, {v->source_corresp.name});
      break;
    }
    case enk_lambda: {
      a_lambda_ptr l = e->variant.init.source.lambda;
      if (l && (l->is_generic || l->has_template_param_list))
        scan_report(sc, Diag::generic_lambda_in_annotation, e->position);
      if (l && sc.lambdas.insert(l).second) {
        if (l->lambda_routine) g_annotation_lambda_routines.insert(l->lambda_routine);
        note_lambda(l);
        scan_lambda_body(l, tb);
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
  tb.shared_expr_budget = kBudget;
  traverse_expr(root, &tb);
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
  const std::string context = instance_context(owner);
  if (is_contract(k) && a->syntactic_location != al_post_func)
    report(Diag::contract_not_after_parameters, a->position, {a->name}, context);

  Scan sc;
  sc.annot = k;
  sc.owner = owner;
  sc.context = context;
  if (k == Annot::invariant && stmt != nullptr) {
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
// (decision 0008): the front end defines the macro itself, and without it every
// CERASTES_* macro expands to nothing, silently dropping the annotations.  The
// macro table is gone when the back end runs, so the text of the files is read:
// a line of that shape (also in a skipped #if block, which is rejected too).
void scan_file_for_macro_change(a_source_file_ptr f) {
  if (f == nullptr || f->full_name == nullptr) return;
  std::ifstream in(f->full_name);
  std::string line;
  for (unsigned long n = 1; std::getline(in, line); ++n) {
    if (line.find("__CERASTES__") == std::string::npos) continue;
    size_t i = 0;
    while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i]))) ++i;
    if (i >= line.size() || line[i] != '#') continue;
    const size_t hash = i++;
    while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i]))) ++i;
    const char* what = nullptr;
    if (line.compare(i, 5, "undef") == 0) { what = "undefined"; i += 5; }
    else if (line.compare(i, 6, "define") == 0) { what = "redefined"; i += 6; }
    if (what == nullptr || i >= line.size() || !std::isspace(static_cast<unsigned char>(line[i]))) continue;
    while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i]))) ++i;
    if (line.compare(i, 12, "__CERASTES__") != 0) continue;
    if (i + 12 < line.size() && (std::isalnum(static_cast<unsigned char>(line[i + 12])) || line[i + 12] == '_')) continue;
    DiagPos p;
    p.known = true;
    p.file = f->file_name ? f->file_name : f->full_name;
    p.full_name = f->full_name;
    p.line = n;
    p.col = hash + 1;
    report_at(Diag::cerastes_macro_changed, p, {what});
  }
}

void scan_files_for_macro_change(a_source_file_ptr f, int depth) {
  for (; f != nullptr && depth < 64; f = f->next) {
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
  a_routine_ptr routine = nullptr;
  bool spec_caller = false;       // the routine is itself a specification function
  std::set<a_statement_ptr> statements;
  std::set<an_expr_node_ptr> seen;
};
BodyState* g_body = nullptr;
std::set<a_statement_ptr> g_checked_statements;  // statements with annotations checked in a body

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
  if (b.spec_caller) return;
  // A reference to a specification function in ordinary code: a call, and also
  // taking its address (a call through the pointer would go unseen).  The routine
  // is found by entity, whatever its spelling (using, qualified or not).
  if (e->kind == enk_routine) {
    a_routine_ptr callee = e->variant.routine.ptr;
    if (callee == nullptr) return;
    const std::string n = rname(callee);
    if (marked(callee, Annot::predicate) || marked(callee, Annot::pure) || is_spec_function_name(n))
      g_spec_calls.push_back({b.routine, e->position, n});
  } else if (e->kind == enk_constant) {
    // A dependent call of a specification function in a template that may never be
    // instantiated: only its name is known.
    const char* callee = unknown_callee_name(e->variant.constant.ptr);
    if (is_spec_name(callee)) g_spec_calls.push_back({b.routine, e->position, std::string("cerastes::") + callee});
  }
}

void body_on_constant(a_constant_ptr c, an_expr_or_stmt_traversal_block_ptr tb) {
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
  tb->shared_expr_budget = kBudget;
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
    if (v->init_kind == initk_dynamic && v->initializer.dynamic != nullptr)
      traverse_dynamic_init(v->initializer.dynamic, &tb);
    else if (v->init_kind == initk_static && v->initializer.constant != nullptr)
      traverse_constant(v->initializer.constant, &tb);
  }
  for (a_field_ptr f : g_c.fields) {
    SrcPos pos = resolve(f->source_corresp.decl_position);
    if (!pos.known || pos.in_system) continue;
    if (f->initializer != nullptr) traverse_dynamic_init(f->initializer, &tb);
  }
  g_body = saved;
}

}  // namespace

void check_annotations() {
  g_c = Collected();
  g_proto_owner.clear();
  g_annotation_lambda_routines.clear();
  g_lambdas_checked.clear();
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
    if (!pos.known) continue;
    if (!has_cerastes_attribute(r) && (pos.in_system || r->compiler_generated)) continue;
    checked.push_back(r);
  }
  // Bodies first: they hold the statement annotations, and show which routines
  // are lambdas.  Then the routines' own annotations, then the calls of
  // specification functions, which need the lambdas of annotations found by the
  // scans before.
  g_spec_calls.clear();
  g_checked_statements.clear();
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
  check_modules();
  check_macro_unchanged();
}

}  // namespace cerastes
