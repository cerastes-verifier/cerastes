// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Grammar of the lines (two spaces per level under the attribute line; POS is
// `L<line>:<col>` by annotation_position, prefixed with the file name when the
// position is not in the primary source file):
//   arg K @ POS                                expression argument; its tree follows
//   arg K binder NAME: TYPE @ POS              the result name of ensures(r, p)
//   arg K const VALUE type=TYPE @ POS / arg K type TYPE @ POS
//   op EOK type_kind=TK type=TYPE [lvalue|xvalue] [cast=KIND] @ POS   children are the operands
//        (cast=static|reinterpret|const|functional|c-style|implicit on casts)
//   const VALUE type=TYPE @ POS                VALUE: decimal, true/false, nullptr, a float or address
//   tconst tpck_param NAME #POS@DEPTH type=TYPE @ POS       a non-type template parameter
//   tconst tpck_sizeof|... operand=TYPE type=TYPE @ POS     (a type operand; an expression operand is a child)
//   tconst tpck_unknown_function NAME type=TYPE @ POS       the callee of a dependent call
//   tconst TPCK [NAME] type=TYPE @ POS         other template-parameter constants; child: expression
//   param N NAME: TYPE [lvalue] @ POS          NAME and TYPE from the routine's parameter list
//   binder NAME: TYPE [lvalue] @ POS           the named result
//   this: TYPE @ POS                           `this`, however it is reached
//   var QNAME: TYPE decl=POS [lvalue] @ POS
//   field QNAME: TYPE [lvalue] @ POS
//   routine QNAME [args <T=int>]: TYPE @ POS   a callee
//   lambda <lambda@L:C> @ POS                  children: `capture ...` lines, then `body return` + tree
//   capture by-ref|by-value NAME: TYPE [implicit] @ POS / capture this|*this @ POS
//   capture init NAME by-ref|by-value @ POS    child: the initializer expression
//   sizeof... TYPE|(child expression) type=TYPE @ POS
//   type TYPE
//   node ENK type=TYPE @ POS (children not printed)    visible fallback (a problem)
//   node enk_param_ref ... (unmapped)          a parameter reference the convention does not explain (a problem)
//   (repeated node), (depth limit)             (problems)
//
// A problem is a construct that this version does not understand: the checks
// reject it with `unsupported_annotation_expression` (invariant 3).
//
// The tree printer behind print_annotation_args.
#include "expr_tree.h"

#include <string>
#include <unordered_set>

#include "annotation_il.h"
#include "diagnostics.h"
#include "il_names.h"
#include "options.h"
#include "render.h"

namespace cerastes {
namespace {

constexpr int kDepthCap = 256;

std::string at(const a_source_position& p) { return annotation_position(p); }

// The self-test hook acts once on the first tree of each of the two modes.
bool g_hook_done[2] = {false, false};

struct Printer {
  std::string& out;
  an_attribute_ptr attr;
  Annot annot;
  a_routine_ptr owner;
  TreeMode mode;
  TreeWalk& walk;
  const char* binder_name = nullptr;
  bool binder_present = false;
  // Levels of parameter scopes that nested constructs of the tree have opened
  // since the attribute's own (a requires-expression's parameters, a function
  // type in the tree): a parameter reference counts them in its levels_up.
  // Nothing opens one today; the one that could (enk_requires) is not entered.
  int nesting = 0;
  // Depth inside an unevaluated operand (sizeof, alignof, noexcept, typeid): an
  // instance folds such an operand to a constant, so the references in it do not
  // count in the sequence a root template and its instance must agree on.
  int unevaluated = 0;
  std::unordered_set<const void*> visited{};
  bool hook_unmapped = false;  // self-test: read the next param_ref as unmapped
  bool hook_revisit = false;   // self-test: reach the next operation node twice

  void line(int indent, const std::string& text) {
    if (mode != TreeMode::print) return;
    out.append(static_cast<size_t>(indent), ' ');
    out += text;
    out += '\n';
  }

  void problem(const a_source_position& p, const std::string& what, bool explained_by_side_effect = false) {
    walk.problems.push_back({p, what, explained_by_side_effect});
  }

  // Parameter number `n` (1-based) of the owner's routine type.
  a_param_type_ptr nth_param(unsigned n) const {
    if (owner == nullptr || owner->type == nullptr) return nullptr;
    a_type_ptr rt = skip_typerefs(owner->type);
    if (rt->kind != tk_routine) return nullptr;
    a_routine_type_supplement_ptr x = rt->variant.routine.extra_info;
    unsigned i = 1;
    for (a_param_type_ptr p = x ? x->param_type_list : nullptr; p; p = p->next, ++i)
      if (i == n) return p;
    return nullptr;
  }

  std::string result_type() const {
    if (owner == nullptr || owner->type == nullptr) return "<unknown>";
    a_type_ptr rt = skip_typerefs(owner->type);
    if (rt->kind != tk_routine) return "<unknown>";
    return render_type(rt->variant.routine.return_type);
  }

  static std::string value_flags(an_expr_node_ptr e) {
    return e->is_lvalue ? " lvalue" : (e->is_xvalue ? " xvalue" : "");
  }

  // The kind of a cast operation: the flags it carries (il_def.h:15003), or
  // c-style when the program wrote it without one, implicit when the front end did.
  static std::string cast_kind(an_expr_node_ptr e) {
    const auto& op = e->variant.operation;
    if (op.kind != eok_cast && op.kind != eok_lvalue_cast && op.kind != eok_ref_cast) return "";
    const char* k = "c-style";
    if (op.is_reinterpret_cast) k = "reinterpret";
    else if (op.is_const_cast) k = "const";
    else if (e->is_static_cast) k = "static";
    else if (e->is_functional_notation_cast) k = "functional";
    else if (e->compiler_generated) k = "implicit";
    return std::string(" cast=") + k;
  }

  void dynamic_init_child(a_dynamic_init_ptr d, int indent, int depth, const a_source_position& where) {
    if (d == nullptr) {
      line(indent, "node <no initializer>");
      problem(where, "an init-capture without an initializer expression");
      return;
    }
    if (d->kind == dik_expression && d->variant.expression != nullptr) {
      node(d->variant.expression, indent, depth + 1);
    } else if (d->kind == dik_constant && d->variant.constant.ptr != nullptr) {
      line(indent, "const " + render_constant(d->variant.constant.ptr) + " type=" +
                       render_type(d->variant.constant.ptr->type) + " @ " + at(where));
    } else {
      line(indent, "node init-kind-" + std::to_string(static_cast<int>(d->kind)) +
                       " (children not printed) @ " + at(where));
      problem(where, "an initializer of a kind that is not understood");
    }
  }

  void captures(a_lambda_ptr l, int indent, int depth) {
    for (a_lambda_capture_ptr c = l->capture_list; c; c = c->next) {
      const std::string pos = at(c->position);
      const char* how = c->capture_by_reference ? "by-ref" : "by-value";
      if (c->is_init_capture) {
        std::string name = c->closure_field && c->closure_field->source_corresp.name
                               ? c->closure_field->source_corresp.name
                               : "_";
        line(indent, std::string("capture init ") + name + " " + how + " @ " + pos);
        dynamic_init_child(c->captured.initializer, indent + 2, depth, c->position);
      } else if (c->is_indirect_init_capture) {
        std::string name = c->captured.init_capture_field && c->captured.init_capture_field->source_corresp.name
                               ? c->captured.init_capture_field->source_corresp.name
                               : "_";
        line(indent, std::string("capture indirect-init ") + name + " " + how + " @ " + pos);
      } else if (c->captured.variable && c->captured.variable->is_this_parameter) {
        line(indent, std::string("capture ") + (c->capture_by_reference ? "this" : "*this") +
                         (c->is_implicit ? " implicit" : "") + " @ " + pos);
      } else if (c->captured.variable) {
        a_variable_ptr v = c->captured.variable;
        line(indent, std::string("capture ") + how + " " + render_name(&v->source_corresp, iek_variable) + ": " +
                         render_type(v->type) + (c->is_implicit ? " implicit" : "") + " @ " + pos);
      } else {
        line(indent, std::string("capture ") + how + " <unknown> @ " + pos);
        problem(c->position, "a capture that is not understood");
      }
    }
  }

  // The body of a lambda: `{ return e; }` is the one form this version has a
  // meaning for.
  void lambda_body(a_lambda_ptr l, const a_source_position& where, int indent, int depth) {
    a_routine_ptr r = l ? l->lambda_routine : nullptr;
    a_scope_ptr sp = nullptr;
    if (r && r->function_def_number != NULL_function_def_number) sp = scope_for_routine_or_null(r);
    if (sp == nullptr || sp->kind != sck_function || sp->variant.routine.ptr != r || sp->assoc_block == nullptr) {
      line(indent, "body <unavailable>");
      problem(where, "a lambda whose body is not available");
      return;
    }
    a_statement_ptr b = sp->assoc_block;
    int n = 0;
    a_statement_ptr only = nullptr;
    if (b->kind == stmk_block) {
      for (a_statement_ptr s = b->variant.block.statements; s; s = s->next) {
        ++n;
        only = s;
      }
    }
    if (n == 1 && only->kind == stmk_return && only->expr != nullptr) {
      line(indent, "body return");
      node(only->expr, indent + 2, depth + 1);
    } else {
      line(indent, "body <" + std::to_string(n) + " statements>");
      problem(where, "a lambda whose body is not a single return statement", /*explained_by_side_effect=*/true);
    }
  }

  void tconst(an_expr_node_ptr e, a_constant_ptr c, int indent, int depth, const std::string& pos) {
    const auto& tp = c->variant.template_param;
    const std::string kind = name_of(tp.kind);
    const std::string ty = " type=" + render_type(e->type) + " @ " + pos;
    const char* nm = c->source_corresp.name;
    switch (tp.kind) {
      case tpck_param:
        line(indent, "tconst " + kind + " " + (nm ? nm : "_") + " #" + std::to_string(tp.variant.coordinates.position) +
                         "@" + std::to_string(tp.variant.coordinates.depth) + ty);
        return;
      case tpck_unknown_function:
        line(indent, "tconst " + kind + " " + (nm ? render_name(&c->source_corresp, iek_constant) : "_") + ty);
        return;
      case tpck_sizeof:
      case tpck_datasizeof:
      case tpck_alignof:
      case tpck_uuidof:
      case tpck_typeid:
      case tpck_noexcept: {
        a_type_ptr t = tp.variant.templ_sizeof.type;
        line(indent, "tconst " + kind + (t ? " operand=" + render_type(t) : "") + ty);
        an_expr_node_ptr own = tp.variant.templ_sizeof.expr ? tp.variant.templ_sizeof.expr : c->expr;
        ++unevaluated;
        if (own != nullptr) node(own, indent + 2, depth + 1);
        --unevaluated;
        return;
      }
      default: break;
    }
    // (T)100 and similar: a template-parameter constant that records the
    // expression it stands for.
    line(indent, "tconst " + kind + (nm ? std::string(" ") + nm : "") + ty);
    an_expr_node_ptr own = tp.kind == tpck_expression ? tp.variant.expr : nullptr;
    if (own == nullptr) own = c->expr;
    if (own != nullptr) node(own, indent + 2, depth + 1);
  }

  void node(an_expr_node_ptr e, int indent, int depth) {
    if (e == nullptr) {
      line(indent, "node <null>");
      return;
    }
    if (depth >= kDepthCap) {
      line(indent, "(depth limit)");
      problem(e->position, "an expression nested deeper than " + std::to_string(kDepthCap) + " levels");
      return;
    }
    if (!visited.insert(e).second) {
      line(indent, "(repeated node)");
      problem(e->position, "a node that is part of the tree twice");
      return;
    }
    const std::string pos = at(e->position);
    switch (e->kind) {
      case enk_operation: {
        // il_def.h:15126: kind, and the kind of type the operation acts on
        // (tk_template_param or tk_unknown when dependent).
        const auto& op = e->variant.operation;
        line(indent, std::string("op ") + name_of(op.kind) + " type_kind=" + name_of(op.type_kind) +
                         " type=" + render_type(e->type) + value_flags(e) + cast_kind(e) + " @ " + pos);
        if (op.kind == eok_noexcept) ++unevaluated;
        for (an_expr_node_ptr c = op.operands; c; c = c->next) node(c, indent + 2, depth + 1);
        if (op.kind == eok_noexcept) --unevaluated;
        if (hook_revisit) {
          hook_revisit = false;
          node(e, indent + 2, depth + 1);  // self-test: reach this node a second time
        }
        break;
      }
      case enk_constant: {
        a_constant_ptr c = e->variant.constant.ptr;
        if (c != nullptr && c->kind == ck_template_param) {
          tconst(e, c, indent, depth, pos);
        } else {
          line(indent, "const " + render_constant(c) + " type=" + render_type(e->type) + " @ " + pos);
        }
        break;
      }
      case enk_param_ref: {
        ParamRef r = interpret_param_ref(e, annot, binder_present, nesting);
        if (hook_unmapped) {
          hook_unmapped = false;
          r.kind = ParamRef::Kind::unmapped;  // self-test
        }
        switch (r.kind) {
          case ParamRef::Kind::param: {
            a_param_type_ptr p = nth_param(r.num);
            if (unevaluated == 0) walk.references.push_back("P" + std::to_string(r.num + (mode == TreeMode::signature_renumbered ? 1 : 0)));
            line(indent, "param " + std::to_string(r.num) + " " + (p && p->name ? p->name : "_") + ": " +
                             render_type(p ? p->type : e->type) + value_flags(e) + " @ " + pos);
            break;
          }
          case ParamRef::Kind::binder:
            if (unevaluated == 0) walk.references.push_back("R");
            line(indent, std::string("binder ") + (binder_name ? binder_name : "_") + ": " + result_type() +
                             value_flags(e) + " @ " + pos);
            break;
          case ParamRef::Kind::this_param:
            if (unevaluated == 0) walk.references.push_back("T");
            line(indent, "this: " + render_type(e->type) + " @ " + pos);
            break;
          case ParamRef::Kind::unmapped:
            line(indent, "node enk_param_ref param_num=" + std::to_string(e->variant.param_ref.param_num) +
                             " levels_up=" + std::to_string(e->variant.param_ref.levels_up) +
                             " type=" + render_type(e->type) + " @ " + pos + " (unmapped)");
            problem(e->position, "a parameter reference that is not understood");
            break;
        }
        break;
      }
      case enk_variable: {
        a_variable_ptr v = e->variant.variable.ptr;
        if (v == nullptr) {
          line(indent, "node enk_variable type=" + render_type(e->type) + " @ " + pos + " (no variable)");
          problem(e->position, "a variable reference without a variable");
          break;
        }
        if (v->is_this_parameter) {
          if (unevaluated == 0) walk.references.push_back("T");
          line(indent, "this: " + render_type(e->type) + " @ " + pos);
          break;
        }
        line(indent, "var " + render_name(&v->source_corresp, iek_variable) + ": " + render_type(e->type) +
                         " decl=" + at(v->source_corresp.decl_position) + value_flags(e) + " @ " + pos);
        break;
      }
      case enk_field: {
        a_field_ptr f = e->variant.field.ptr;
        line(indent, "field " + (f ? render_name(&f->source_corresp, iek_field) : std::string("<null>")) +
                         ": " + render_type(e->type) + value_flags(e) + " @ " + pos);
        break;
      }
      case enk_routine: {
        a_routine_ptr r = e->variant.routine.ptr;
        if (r == nullptr) {
          line(indent, "routine <none>: " + render_type(e->type) + " @ " + pos);
          break;
        }
        std::string s = "routine " + render_name(&r->source_corresp, iek_routine);
        if (r->template_arg_list) s += " args <" + render_template_args(r->assoc_template, r->template_arg_list) + ">";
        line(indent, s + ": " + render_type(e->type) + " @ " + pos);
        break;
      }
      case enk_lambda: {
        a_lambda_ptr l = e->variant.init.source.lambda;
        line(indent, "lambda " + (l ? render_type(l->closure_class) : std::string("<null>")) + " @ " + pos);
        if (l) captures(l, indent + 2, depth);
        lambda_body(l, e->position, indent + 2, depth);
        break;
      }
      case enk_sizeof_pack: {
        // sizeof...(Ts), sizeof...(args) or sizeof...(TT) (il_def.h:15410).
        const auto& sp = e->variant.sizeof_pack;
        if (sp.is_template_template) {
          line(indent, "sizeof... " + (sp.variant.templ ? render_name(&sp.variant.templ->source_corresp, iek_template) : std::string("<null>")) +
                           " type=" + render_type(e->type) + " @ " + pos);
        } else if (sp.is_type) {
          line(indent, "sizeof... " + render_type(sp.variant.type) + " type=" + render_type(e->type) + " @ " + pos);
        } else {
          line(indent, "sizeof... type=" + render_type(e->type) + " @ " + pos);
          node(sp.variant.expr, indent + 2, depth + 1);
        }
        break;
      }
      case enk_type_operand: {
        a_type_ptr t = e->variant.type_operand.type;
        line(indent, "type " + (t ? render_type(t) : std::string("<default>")));
        break;
      }
      default:
        // Visible fallback: the kind, the type and the position; children are
        // not printed.  It is a problem: the checks reject the annotation.
        line(indent, std::string("node ") + name_of(static_cast<an_expr_node_kind>(e->kind)) +
                         " type=" + render_type(e->type) + " @ " + pos + " (children not printed)");
        // (new, delete, throw and statement expressions have their own
        // diagnostics: one owner per check.)
        if (e->kind != enk_new_delete && e->kind != enk_throw && e->kind != enk_statement)
          problem(e->position, std::string("an expression of kind ") + name_of(static_cast<an_expr_node_kind>(e->kind)));
        break;
    }
  }
};

}  // namespace

std::string annotation_position(const a_source_position& p) { return position_label(p); }

TreeWalk annotation_tree(an_attribute_ptr a, a_routine_ptr owner, TreeMode mode, std::string* out, int indent) {
  TreeWalk walk;
  std::string scratch;
  Printer p{*(out ? out : &scratch), a, classify(a), owner, mode, walk};
  std::vector<AnnotArg> args = arguments_of(a);
  for (const AnnotArg& g : args) {
    if (g.kind == AnnotArg::Kind::binder) {
      p.binder_name = g.binder_name;
      p.binder_present = true;
    }
  }
  const int slot = mode == TreeMode::print ? 0 : 1;
  if (options().checks_selftest && (mode == TreeMode::print || mode == TreeMode::check) && !g_hook_done[slot]) {
    // Self-test: the first tree reads one parameter reference as unmapped and
    // reaches one operation node twice, so that those two paths can be proven
    // to fire (no source reaches them).
    g_hook_done[slot] = true;
    p.hook_unmapped = true;
    p.hook_revisit = true;
  }
  for (const AnnotArg& g : args) {
    const std::string pos = at(g.raw->position);
    const std::string k = std::to_string(g.index);
    switch (g.kind) {
      case AnnotArg::Kind::binder:
        p.line(indent, "arg " + k + " binder " + (g.binder_name ? g.binder_name : "_") + ": " +
                           p.result_type() + " @ " + pos);
        break;
      case AnnotArg::Kind::expression:
        p.line(indent, "arg " + k + " @ " + pos);
        p.node(g.expr, indent + 2, 0);
        break;
      case AnnotArg::Kind::other:
        if (g.raw->kind == aak_constant) {
          p.line(indent, "arg " + k + " const " + render_constant(g.raw->variant.constant) + " type=" +
                             render_type(g.raw->variant.constant ? g.raw->variant.constant->type : nullptr) +
                             " @ " + pos);
        } else if (g.raw->kind == aak_type) {
          p.line(indent, "arg " + k + " type " + render_type(g.raw->variant.type) + " @ " + pos);
        } else {
          p.line(indent, "arg " + k + " <unsupported argument kind> @ " + pos);
          p.problem(g.raw->position, "an argument of a kind that is not understood");
        }
        break;
    }
  }
  return walk;
}

bool contracts_agree(a_routine_ptr root, a_routine_ptr r, bool perturb) {
  static bool first_instance_done = false;
  std::vector<an_attribute_ptr> a = contracts_of(root), b = contracts_of(r);
  bool renumber = false;
  if (perturb) {
    if (!first_instance_done) {
      first_instance_done = true;
      if (!b.empty()) b.erase(b.begin());  // the first instance loses its first contract
    } else {
      renumber = true;  // the others read their parameters one further
    }
  }
  if (a.size() != b.size()) return false;
  for (size_t i = 0; i < a.size(); ++i) {
    if (std::string(a[i]->name ? a[i]->name : "") != std::string(b[i]->name ? b[i]->name : "")) return false;
    if (arguments_of(a[i]).size() != arguments_of(b[i]).size()) return false;
    TreeWalk x = annotation_tree(a[i], root, TreeMode::signature, nullptr, 0);
    TreeWalk y = annotation_tree(b[i], r, renumber ? TreeMode::signature_renumbered : TreeMode::signature, nullptr, 0);
    if (x.references != y.references) return false;
  }
  return true;
}

void print_annotation_args(std::string& out, an_attribute_ptr a, a_routine_ptr owner, int indent) {
  (void)annotation_tree(a, owner, TreeMode::print, &out, indent);
}

}  // namespace cerastes
