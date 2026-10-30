// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Small questions about the IL that more than one pass asks (the annotation checks, the walker and
// the verify-mode encoder).  Each was an anonymous-namespace helper of one of them; they live here
// so that there is one answer.
#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "annotation_il.h"
#include "edg_il.h"

namespace cerastes {

// Qualified name of a routine, as EDG prints it.
std::string rname(a_routine_ptr r);

// The routine's own function scope, or null when it has none in the IL.  function_def_number selects
// the routine's descriptor (mem_tables.h:80-107); unlike il_header.region_scope_entry[memory_region]
// this is correct for lambdas and members of local classes, which share the enclosing function's
// memory region (B1).
a_scope_ptr body_scope(a_routine_ptr r);

// A call expression kind: a plain call, or a member call through `.` or `->`.
bool is_call_kind(an_expr_operator_kind k);

// The routine called by a call expression whose callee is named directly, or null.
a_routine_ptr direct_callee(an_expr_node_ptr e);

// The names of cerastes's own specification functions ("cerastes::owns", ... and the C spellings "cerastes_owns", ...).
bool is_spec_function_name(const std::string& n);

// A routine marked predicate or pure, on its own attribute list or its root template's (an instance of a
// marked template).
bool marked(a_routine_ptr r, Annot k);

// A predicate, a pure function, or one of cerastes's spec functions: specification, never run.
bool is_spec_routine(a_routine_ptr r);

// The file that sequence number `seq` physically lies in (the file EDG read, whatever `#line` or a
// linemarker made of its name), or null for sequence number 0 (a compiler-generated entity).
a_source_file_ptr physical_file(a_seq_number seq);

// The canonical physical path (realpath) of the file `p` lies in; "" when the position is unknown or the
// file cannot be resolved.
std::string physical_path(const a_source_position& p);

// 0011 item 4: the position lies in a file that `cerastes verify` verifies (known, and not a system file by
// canonical physical path).
bool in_verified_file(const a_source_position& p);

// The positions of the EDG diagnostic pragmas (`#pragma diag_suppress` and the like, `diagnostic push/pop`, also through
// `_Pragma` and macros) the front end processed, recorded by the hook install_diag_pragma_recorder() (register_attributes.h)
// puts in front of EDG's handlers (decision 0017 item 2).  EDG keeps no IL entry for them.
const std::vector<a_source_position>& diag_pragma_sites();

// The entities of the translation unit that the verify-mode encoder looks at, each once, in the order the
// IL walk reports them (file scope first, then the routine-scope regions in region order).
struct IlEntities {
  std::vector<a_routine_ptr> routines;
  std::vector<a_template_ptr> templates;
  std::vector<a_variable_ptr> variables;  // those not declared inside a routine
  std::vector<a_type_ptr> types;
  std::vector<an_asm_entry_ptr> asm_entries;  // file-scope asm declarations (not the asm statements of bodies)
  // Decision 0024 item 2: the static assertions of the file scope's source sequence list (file, namespace, `extern "C"` and
  // class scope, in source order, every header's included), and those the file-scope walk reached that are not on it (a
  // fail-closed cross-check: the encoder reports one in a verified file).
  std::vector<a_static_assertion_ptr> static_assertions;
  std::vector<a_static_assertion_ptr> other_static_assertions;
  // Decision 0025 item 5: the static assertions each function memory region holds (block scope), with the region's number, in
  // region order and then in the walk's order.
  std::vector<std::pair<a_memory_region_number, a_static_assertion_ptr>> region_static_assertions;
  // The declarations of a routine other than the one `source_corresp.decl_position` names (a_src_seq_secondary_decl entries whose entity is the routine, il_def.h:2131), in the
  // order the walk reports them; its first declaration is `source_corresp.decl_position`.  Decision 0040 item 3: where each declaration's parameters
  // lie, to check that every declaration of a parameter carries the same `cerastes::owning`.
  struct SecondaryDecl {
    a_source_position position;
    a_type_ptr declared_type = nullptr;  // the type as the declaration wrote it (a typedef'd function type stays a typeref)
  };
  std::map<a_routine_ptr, std::vector<SecondaryDecl>> secondary_decls;
  // Decision 0041 item 2: the other declarations of a file-scope variable (a_src_seq_secondary_decl entries whose entity is the variable): one in a
  // system file makes the variable shared with the runtime (`environ`), which is rejected.
  std::map<a_variable_ptr, std::vector<a_source_position>> variable_secondary_decls;
  // Decision 0041 item 5 (P63 H6): the initializer of a `static` local whose `init_kind` is `initk_function_local` is the
  // local-static-variable-init entry of its scope, not the variable's own; the entries by variable.
  std::map<a_variable_ptr, a_local_static_variable_init_ptr> static_local_inits;
};
IlEntities collect_entities();

}  // namespace cerastes
