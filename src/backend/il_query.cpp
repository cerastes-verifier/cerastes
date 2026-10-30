// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "il_query.h"

#include <set>

#include "render.h"
#include "verify_config.h"

namespace cerastes {

std::string rname(a_routine_ptr r) { return render_name(&r->source_corresp, iek_routine); }

a_scope_ptr body_scope(a_routine_ptr r) {
  if (r->function_def_number == NULL_function_def_number) return nullptr;
  a_scope_ptr sp = scope_for_routine_or_null(r);
  if (sp == nullptr || sp->kind != sck_function || sp->variant.routine.ptr != r) return nullptr;
  return sp;
}

bool is_call_kind(an_expr_operator_kind k) {
  return k == eok_call || k == eok_dot_member_call || k == eok_points_to_member_call;
}

a_routine_ptr direct_callee(an_expr_node_ptr e) {
  if (e == nullptr || e->kind != enk_operation) return nullptr;
  const auto& op = e->variant.operation;
  if (!is_call_kind(op.kind)) return nullptr;
  if (op.operands == nullptr || op.operands->kind != enk_routine) return nullptr;
  return op.operands->variant.routine.ptr;
}

bool is_spec_function_name(const std::string& n) {
  return n == "cerastes::owns" || n == "cerastes::reads" || n == "cerastes::old" || n == "cerastes::live" ||
         n == "cerastes::forall" || n == "cerastes::owns_range" ||
         // The C spellings of <cerastes.h> (decisions 0029 item 3, 0037 item 5): declared at file scope in both languages.
         n == "cerastes_owns" || n == "cerastes_reads" || n == "cerastes_live" || n == "cerastes_old_";
}

bool marked(a_routine_ptr r, Annot k) {
  if (has_annot(r, k)) return true;
  a_routine_ptr root = root_routine(r);
  return root != nullptr && has_annot(root, k);
}

bool is_spec_routine(a_routine_ptr r) {
  return marked(r, Annot::predicate) || marked(r, Annot::pure) || is_spec_function_name(rname(r));
}

a_source_file_ptr physical_file(a_seq_number seq) {
  if (seq == 0) return nullptr;
  a_source_file_ptr f = nullptr;
  a_line_number line = 0;
  a_boolean at_end = FALSE;
  conv_seq_to_physical_file_and_line(seq, &f, &line, &at_end);
  return f;
}

std::string physical_path(const a_source_position& p) {
  a_source_file_ptr f = physical_file(p.seq);
  if (f == nullptr || f->full_name == nullptr) return "";
  return canonical_path(f->full_name);
}

bool in_verified_file(const a_source_position& p) {
  if (p.seq == 0) return false;  // no position: compiler-generated
  a_source_file_ptr f = physical_file(p.seq);
  // A file that cannot be placed is a verified file, never a system file (decision 0011 item 4: fail closed).
  if (f == nullptr || f->full_name == nullptr) return true;
  const std::string c = canonical_path(f->full_name);
  if (c.empty()) return true;
  const VerifyConfig& cfg = verify_config();
  return !is_system_file(cfg.system_dirs, cfg.cerastes_header, c);
}

namespace {
IlEntities* g_collect_out = nullptr;
std::set<char*>* g_collect_seen = nullptr;
bool g_collect_in_region = false;
a_memory_region_number g_collect_region = NULL_region_number;
void collect_entry(char* entry, an_il_entry_kind kind) {
  if (!g_collect_seen->insert(entry).second) return;
  switch (kind) {
    case iek_routine: g_collect_out->routines.push_back(reinterpret_cast<a_routine_ptr>(entry)); break;
    case iek_template: g_collect_out->templates.push_back(reinterpret_cast<a_template_ptr>(entry)); break;
    case iek_variable: {
      auto v = reinterpret_cast<a_variable_ptr>(entry);
      if (v->source_corresp.enclosing_routine == nullptr) g_collect_out->variables.push_back(v);
      break;
    }
    case iek_type: g_collect_out->types.push_back(reinterpret_cast<a_type_ptr>(entry)); break;
    case iek_src_seq_secondary_decl: {
      auto sd = reinterpret_cast<a_src_seq_secondary_decl_ptr>(entry);
      if (sd->entity.kind == iek_routine) g_collect_out->secondary_decls[reinterpret_cast<a_routine_ptr>(sd->entity.ptr)].push_back({sd->decl_position, sd->declared_type});
      break;
    }
    case iek_asm_entry: {
      auto a = reinterpret_cast<an_asm_entry_ptr>(entry);
      // Only those the file-scope walk reports: an asm statement of a body is in a routine's memory region.
      if (!g_collect_in_region && a->source_corresp.enclosing_routine == nullptr) g_collect_out->asm_entries.push_back(a);
      break;
    }
    case iek_static_assertion: {
      auto sa = reinterpret_cast<a_static_assertion_ptr>(entry);
      // Block-scope ones are in function regions (kept with the region's number, decision 0025 item 5); the file scope's are on its
      // source sequence list (decision 0024).
      if (g_collect_in_region) g_collect_out->region_static_assertions.push_back({g_collect_region, sa});
      else g_collect_out->other_static_assertions.push_back(sa);
      break;
    }
    default: break;
  }
}
}  // namespace

IlEntities collect_entities() {
  IlEntities out;
  std::set<char*> seen;
  g_collect_out = &out;
  g_collect_seen = &seen;
  walk_file_scope_il(collect_entry, nullptr, nullptr, nullptr, nullptr, /*clear_fe_pointers=*/FALSE);
  g_collect_in_region = true;
  for (a_memory_region_number region = FILE_SCOPE_REGION_NUMBER + 1; region <= highest_used_region_number; ++region) {
    if (mem_region_table[region] == nullptr) continue;
    g_collect_region = region;
    walk_routine_scope_il(region, collect_entry, nullptr, nullptr, nullptr, nullptr, /*clear_fe_pointers=*/FALSE);
  }
  g_collect_in_region = false;
  g_collect_region = NULL_region_number;
  if (il_header.primary_scope != nullptr) {
    // Only the static assertions are read from the list; every other kind is stepped over, sublists included.
    for (a_source_sequence_entry_ptr e = il_header.primary_scope->source_sequence_list; e != nullptr; e = e->next)
      if (e->entity.kind == iek_static_assertion)
        out.static_assertions.push_back(reinterpret_cast<a_static_assertion_ptr>(e->entity.ptr));
  }
  {
    std::set<a_static_assertion_ptr> listed(out.static_assertions.begin(), out.static_assertions.end());
    std::vector<a_static_assertion_ptr> rest;
    for (a_static_assertion_ptr a : out.other_static_assertions)
      if (listed.count(a) == 0) rest.push_back(a);
    out.other_static_assertions = std::move(rest);
  }
  g_collect_out = nullptr;
  g_collect_seen = nullptr;
  return out;
}

}  // namespace cerastes
