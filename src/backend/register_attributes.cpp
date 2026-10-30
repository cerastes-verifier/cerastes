// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The registration table of the `cerastes` attribute namespace: decision 0007,
// "The cerastes registration table".  Signatures use EDG's argument-signature
// grammar (attribute.c, an_attr_descr) and targets its appertainment grammar:
//   (Xb)  one unevaluated expression, contextually converted to bool
//   (R,Xb) an optional result name (an identifier), then (Xb)
//   (X)   one unevaluated expression
//   (X+)  one or more unevaluated expressions
//   r     a routine (a function; for RAF_ENTITY_ATTACHED, after the parameter list)
// Flags used below: RAF_ENTITY_ATTACHED (the attribute is attached to the routine), RAF_MUST_MATCH (redeclarations
// must repeat it), RAF_NO_CONSTANT_EVALUATION (no compile-time evaluation of the routine)
//   s:+i! a statement that is an iteration statement
//   s:+n! a statement that is a null statement
#include "register_attributes.h"

#include <vector>

#include "edg_il.h"
#include "attribute.h"
#include "il_query.h"
#include "error.h"
#include "pragma.h"

extern a_function_pointer function_pointers[];  // EDG's table of handler functions (see install_diag_pragma_recorder)

#if !EDG_ATTRIBUTE_REGISTRATION
#error "cerastes M2 needs EDG attribute registration: set CERASTES_EDG_SRC (decision 0005) or pin the EDG fork (decision 0009)"
#endif

namespace cerastes {
namespace {

constexpr an_attr_registration_flag_set kContract = RAF_ENTITY_ATTACHED | RAF_MUST_MATCH;

// The table lives for the life of the program (register_attribute_namespace
// keeps the pointer).
const an_attr_registration kAttributes[] = {
    {"requires", "(Xb)", "r", kContract},
    // "R": an optional first argument names the function's result (the fork's
    // result-name signature code, decision 0007 item 6); it needs RAF_ENTITY_ATTACHED.
    {"ensures", "(R,Xb)", "r", kContract},
    {"invariant", "(Xb)", "s:+i!", RAF_NO_FLAGS},
    // decreases (decision 0013): the termination measure, a list of integer expressions; written before a loop
    // (like `invariant`) or after the parameter list (a contract, like `requires`).  One name with two targets: the
    // flags only matter for the routine placement.
    {"decreases", "(X+)", "r|s:+i!", kContract},
    {"check", "(Xb)", "s:+n!", RAF_NO_FLAGS},
    {"fold", "(X)", "s:+n!", RAF_NO_FLAGS},
    {"unfold", "(X)", "s:+n!", RAF_NO_FLAGS},
    // predicate/pure accumulate (no must-match): the mark on the first
    // declaration covers the definition (0006 item 6).
    // RAF_NO_CONSTANT_EVALUATION (fork commit P): the front end does not evaluate a call of such a routine at
    // compile time (a constexpr/consteval use is refused with EDG's own diagnostic, a defaulted function cannot
    // carry the marker).  It reads the attributes the routine has when it is called, so a marker that is only on a
    // later redeclaration does not stop an earlier use (first-declaration gap, M5b; decision 0007).
    // owning (decisions 0029 item 1 and 0040 item 3): on a parameter, with no argument list or `owning(n)` (an unevaluated expression, M4.5e1:
    // the encoder rejects it today).  The attribute lands on the parameter's `param_type.attributes` (P52); registration accepts a non-pointer
    // parameter and a duplicate, and does not honour RAF_MUST_MATCH there, so the encoder checks every declaration.  Target `p` only: a
    // function, a local, a global and a field wait for M4.5d2 and M4.5c, and EDG refuses them itself until then.
    {"owning", "?(X)", "p", RAF_NO_FLAGS},
    {"predicate", "", "r", RAF_ENTITY_ATTACHED | RAF_NO_CONSTANT_EVALUATION},
    {"pure", "", "r", RAF_ENTITY_ATTACHED | RAF_NO_CONSTANT_EVALUATION},
    {nullptr, nullptr, nullptr, RAF_NO_FLAGS},
};

an_attr_namespace_registration g_namespace = {"cerastes", RANF_CLOSED, kAttributes, nullptr};

}  // namespace

void register_cerastes_annotations() { register_attribute_namespace(&g_namespace); }

// EDG's handlers for the diagnostic pragmas are reached through the function_pointers table (mem_manage.h: an index in
// a_pragma_kind_description, so that PCH files survive ASLR).  The table is a plain (non-const) array, filled statically
// and read at each call (pragma.c, index_to_function_pointer), so entries replaced before edg_main() take effect;
// mem_manage.h declares it already, and the declaration above only restates it.
namespace {
std::vector<a_source_position> g_diag_pragma_sites;

void record_diag_pragma(a_pending_pragma_ptr ppp) {
  a_source_position p = ppp->pragma_position;  // the start of the directive (for _Pragma, its string literal is id_position)
  if (p.seq == 0) p = ppp->id_position;
  g_diag_pragma_sites.push_back(p);
}
void diag_pragma_recorder(a_pending_pragma_ptr ppp) {
  record_diag_pragma(ppp);
  diag_pragma(ppp);  // EDG's handler, unchanged
}
void diagnostic_pragma_recorder(a_pending_pragma_ptr ppp) {
  record_diag_pragma(ppp);
  diagnostic_pragma(ppp);
}
}  // namespace

void install_diag_pragma_recorder() {
  function_pointers[fn_for_function(diag_pragma)] = reinterpret_cast<a_function_pointer>(&diag_pragma_recorder);
  function_pointers[fn_for_function(diagnostic_pragma)] = reinterpret_cast<a_function_pointer>(&diagnostic_pragma_recorder);
}

const std::vector<a_source_position>& diag_pragma_sites() { return g_diag_pragma_sites; }

}  // namespace cerastes
