// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "render.h"

#include <cstdio>

#include "il_names.h"

namespace cerastes {
namespace {

// il_to_str writes through callbacks with no user data, so the target string
// and the active control block are file-locals.  Single-threaded by
// construction.
std::string* g_target = nullptr;
an_il_to_str_output_control_block* g_octl = nullptr;
bool g_in_type = false;  // rendering through form_type (see name_hook)

void sink(a_const_char* s, an_il_to_str_output_control_block_ptr) {
  if (g_target) g_target->append(s);
}

// Closure class of a lambda (a_class_type_supplement.is_lambda_closure_class).
// Decided from the IL flag, never from the printed name.
bool is_closure_class(a_type_ptr t) {
  return is_immediate_class_type(t) && class_type_supp(t)->is_lambda_closure_class;
}

// EDG prints a closure as "lambda [] type at line <seq>, col. <c>", where
// <seq> is its internal sequence number, not a line (il_to_str.c:951-960).
// Print "<lambda@L<line>:<col>>" with the resolved position instead.
void put_closure(a_type_ptr t) {
  g_octl->output_str(("<lambda@" + resolve(t->source_corresp.decl_position).str() + ">").c_str(), g_octl);
}

// output_name hook: closure classes get our spelling (and no qualifier, as in
// EDG's default).  Every other entity must render exactly as EDG's default, so
// the hook removes itself and calls EDG's own routine: form_type for a type
// reached through form_type (that is form_tag_reference, which adds the
// struct/union/enum keyword in C and for unnamed tags; il_to_str.c:1346-1362),
// form_name otherwise.  Names nested inside that default output (for example a
// closure used as a template argument) are therefore rendered by EDG's
// default, not by this hook.
void name_hook(char* entry, an_il_entry_kind kind) {
  auto saved = g_octl->output_name;
  if (kind == iek_type) {
    auto t = reinterpret_cast<a_type_ptr>(entry);
    if (is_closure_class(t)) {
      put_closure(t);
      return;
    }
    g_octl->output_name = nullptr;
    if (g_in_type)
      form_type(t, g_octl);
    else
      form_name(&t->source_corresp, kind, g_octl);
  } else {
    g_octl->output_name = nullptr;
    form_name(reinterpret_cast<a_source_correspondence*>(entry), kind, g_octl);
  }
  g_octl->output_name = saved;
}

// output_class_qualifier hook: a member of a closure class (operator(), the
// conversion function, ...) is qualified by "<lambda@L:C>::".
void class_qualifier_hook(a_type_ptr type, a_boolean for_ptr_to_data_member) {
  if (is_closure_class(type)) {
    put_closure(type);
    g_octl->output_str("::", g_octl);
    return;
  }
  auto saved = g_octl->output_class_qualifier;
  g_octl->output_class_qualifier = nullptr;
  a_parent_class_or_namespace parent;
  parent.class_type = type;
  (void)for_ptr_to_data_member;  // only true for pointer-to-member types
  form_class_or_namespace_qualifier(TRUE, parent, g_octl);
  g_octl->output_class_qualifier = saved;
}

// Run `fn` with a fresh output control block that appends to a string.
template <class Fn>
std::string capture(Fn fn) {
  std::string out;
  std::string* saved = g_target;
  an_il_to_str_output_control_block* saved_octl = g_octl;
  g_target = &out;
  an_il_to_str_output_control_block octl;
  clear_il_to_str_output_control_block(&octl);
  octl.output_str = sink;
  octl.output_name = name_hook;
  octl.output_class_qualifier = class_qualifier_hook;
  octl.gen_pcc_code = il_header.pcc_compatibility_mode;
  g_octl = &octl;
  fn(&octl);
  g_octl = saved_octl;
  g_target = saved;
  return out;
}

}  // namespace

std::string SrcPos::str() const {
  return "L" + std::to_string(line) + ":" + std::to_string(col);
}

SrcPos resolve(const a_source_position& p) {
  SrcPos r;
  if (p.seq == 0) return r;
  r.known = true;
  r.col = p.column;
  a_const_char* file = nullptr;
  a_const_char* full = nullptr;
  a_line_number line = 0;
  a_boolean at_end = FALSE;
  conv_seq_to_file_and_line(p.seq, &file, &full, &line, &at_end);
  r.line = line;
  r.file = file ? file : "";
  r.in_primary = !seq_is_in_include_file(p.seq);
  r.in_system = seq_is_in_system_header(p.seq);
  return r;
}

std::string render_type(a_type_ptr t) {
  if (t == nullptr) return "<null>";
  bool saved = g_in_type;
  g_in_type = true;
  std::string s = capture([&](an_il_to_str_output_control_block_ptr o) { form_type(t, o); });
  g_in_type = saved;
  // EDG's formatter prints exactly "<unnamed>" for a type it cannot name
  // (dependent expression types).  Show the type's kind instead, which is the
  // fact the IL checks care about: tk_template_param subkind
  // (il_def.h:9286-9321) or tk_*.  An unnamed class inside a larger type
  // ("struct <unnamed> &") is a real unnamed tag and is left to EDG.
  if (s == "<unnamed>") {
    a_type_ptr u = skip_typerefs(t);
    s = std::string("<") +
        (u->kind == tk_template_param ? name_of(u->variant.template_param.kind) : name_of(u->kind)) + ">";
  }
  return s;
}

std::string render_name(a_source_correspondence* scp, an_il_entry_kind kind) {
  return capture([&](an_il_to_str_output_control_block_ptr o) { form_name(scp, kind, o); });
}

// Template arguments contain types, so they render through form_type's paths
// (g_in_type): an unnamed struct/enum keeps its keyword as in EDG's default.
std::string render_template_arg(a_template_arg_ptr a) {
  bool saved = g_in_type;
  g_in_type = true;
  std::string s = capture([&](an_il_to_str_output_control_block_ptr o) { form_a_template_arg(a, o); });
  g_in_type = saved;
  return s;
}

std::string render_attribute(an_attribute_ptr a) {
  // namespace_name: as written ([[ns::name]]); NULL when none (il_def.h:3206).
  std::string s = "[[";
  if (a->namespace_name) {
    s += a->namespace_name;
    s += "::";
  }
  s += a->name ? a->name : "<unnamed>";
  if (a->arguments) {
    // Arguments: aak_raw_token / aak_token carry text (il_def.h:2747-2837);
    // aak_raw_token sequences are terminated by an aak_empty entry, which is
    // not text.  Recognized attributes with other argument kinds are shown by
    // kind.
    std::string args;
    bool first = true;
    for (an_attribute_arg_ptr g = a->arguments; g; g = g->next) {
      std::string piece;
      switch (g->kind) {
        case aak_empty: continue;
        case aak_raw_token:
        case aak_token: piece = g->variant.token ? g->variant.token : ""; break;
        case aak_constant: piece = "<aak_constant>"; break;
        case aak_type: piece = "<aak_type>"; break;
        case aak_expression: piece = "<aak_expression>"; break;
        default: piece = "<aak?>"; break;
      }
      if (!first) args += ' ';
      args += piece;
      first = false;
    }
    s += "(" + args + ")";
  }
  s += "]]";
  return s;
}

std::string render_constraint(an_expr_node_ptr e) {
  if (e == nullptr) return "<null>";
  switch (e->kind) {
    case enk_concept_id: {
      // il_def.h:15710-15718.  For a type constraint (is_type_constraint) the
      // first template argument is implicit and absent from `args`.
      auto& c = e->variant.concept_id;
      std::string s = render_name(&c.concept_template->source_corresp, iek_template);
      if (c.args) {
        bool saved = g_in_type;
        g_in_type = true;  // see render_template_arg
        s += capture([&](an_il_to_str_output_control_block_ptr o) {
          form_template_args(c.args, nullptr, o);
        });
        g_in_type = saved;
      }
      return s;
    }
    case enk_operation: {
      auto& op = e->variant.operation;
      if ((op.kind == eok_land || op.kind == eok_lor) && op.operands && op.operands->next) {
        auto side = [&](an_expr_node_ptr n) {
          std::string r = render_constraint(n);
          bool other = n->kind == enk_operation &&
                       (n->variant.operation.kind == eok_land || n->variant.operation.kind == eok_lor) &&
                       n->variant.operation.kind != op.kind;
          return other ? "(" + r + ")" : r;
        };
        return side(op.operands) + (op.kind == eok_land ? " && " : " || ") + side(op.operands->next);
      }
      return std::string("<expr:") + name_of(op.kind) + ">";
    }
    default:
      return std::string("<expr:") + name_of(static_cast<an_expr_node_kind>(e->kind)) + ">";
  }
}

}  // namespace cerastes
