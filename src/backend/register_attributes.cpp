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

extern a_function_pointer function_pointers[]; // EDG's table of handler functions (see install_diag_pragma_recorder)

#if !EDG_ATTRIBUTE_REGISTRATION
// A #error message is one line, over 120 columns here: the formatter would continue it after a backslash.
// clang-format off
#error "cerastes M2 needs EDG attribute registration: set CERASTES_EDG_SRC (decision 0005) or pin the EDG fork (decision 0009)"
// clang-format on
#endif

namespace cerastes {
namespace {

constexpr an_attr_registration_flag_set kContract {RAF_ENTITY_ATTACHED | RAF_MUST_MATCH};

// The table lives for the life of the program (register_attribute_namespace
// keeps the pointer).
const an_attr_registration kAttributes[] {
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
    // owning (decisions 0029 item 1, 0040 item 3 and 0052 item 3): on a parameter, with no argument list or `owning(n)`
    // (an unevaluated expression over the earlier parameters, decision 0060 item 13: the encoder reads it on a parameter).  The attribute lands on the parameter's
    // `param_type.attributes` (P52); registration accepts a non-pointer parameter and a duplicate, and does not honour
    // RAF_MUST_MATCH there, so the encoder checks every declaration.  A
    // function (target `r`, M4.5d2a, decision 0047 item 5; RAF_ENTITY_ATTACHED, so its position is after the parameter list or in the leading
    // attribute list) is accepted.  A variable (target `v`, M4.5d2b1, decision 0052 item 3) lands on its
    // `source_corresp.attributes` in both positions (P74); registration accepts a non-pointer and a `void *` too, and
    // the encoder rejects each of them.  From M4.5d2b2a (decision 0054 items 5 and 6) the encoder also accepts it on a
    // mutable pointer global, a `static` local and an address-taken local (P75): the variable's list is the union over
    // its declarations and EDG raises no mismatch, so the encoder compares each declaration's own list.  A field (target
    // `d`, M4.5d2b2b, decision 0055 item 2; P74 (b), P76) lands on the field's own `source_corresp.attributes` in both positions and for each declarator
    // (C structures, C++ aggregates, a structure named only by a typedef); registration accepts a non-pointer field, `owning(n)`, a duplicate and a
    // pointer to `const`, and `struct_info` rejects each of them.  `ref_to_uninit` and `must_init` on a field stay EDG's refusal (#1835).
    {"owning", "?(X)", "r|v|p|d", RAF_ENTITY_ATTACHED},
    // The initialization markers (decision 0042 item 2; P4222's markers in the cerastes spelling; P54, P64).  `uninit` is a declaration
    // attribute on the definition of an automatic local (`v:+a!`, like `invariant`'s `s:+i!`: EDG itself refuses it on a global, a static
    // local (#1862), a parameter, a field and a function (#1835)).  `ref_to_uninit` is a marker on a parameter, a
    // function (target `r`, M4.5d2a: `p` alone with the flag aborts EDG at startup, `r|p` works, P52) and, from
    // M4.5d2b1 (decision 0052 item 4), a variable (target `v`: the encoder accepts an automatic pointer local and,
    // from M4.5d2b2a, decision 0054 item 5, a mutable pointer global and a `static` local, and rejects a non-pointer
    // and a `void *`).  `must_init` stays `p` (question 155: P4222
    // defines it for parameters only; EDG refuses it on a local and a field, #1835).  The attributes accumulate over
    // redeclarations and registration accepts a non-pointer parameter, a duplicate and both markers on one parameter,
    // so the encoder checks each declaration.
    {"uninit", "", "v:+a!", RAF_NO_FLAGS},
    // From M4.5d2b2d (decision 0058 item 4; P77) also a field (target `d`, as `owning`'s): it lands on the field's own `source_corresp.attributes`;
    // registration accepts a non-pointer field, a duplicate, a bit-field and a union member, and `struct_info` rejects each of them.
    {"ref_to_uninit", "", "r|v|p|d", RAF_ENTITY_ATTACHED},
    {"must_init", "", "p", RAF_NO_FLAGS},
    // not_invalidating (decision 0058 item 2; P4418R0, P77): a marker on a pointer parameter, target `p` as `must_init` is (it is no initialization
    // marker and has no argument list); EDG refuses it on a function and after the `*` (#1835).  The encoder reads it like the other parameter markers.
    {"not_invalidating", "", "p", RAF_NO_FLAGS},
    {"predicate", "", "r", RAF_ENTITY_ATTACHED | RAF_NO_CONSTANT_EVALUATION},
    {"pure", "", "r", RAF_ENTITY_ATTACHED | RAF_NO_CONSTANT_EVALUATION},
    {nullptr, nullptr, nullptr, RAF_NO_FLAGS},
};

an_attr_namespace_registration g_namespace {"cerastes", RANF_CLOSED, kAttributes, nullptr};

} // namespace

void register_cerastes_annotations()
{
    register_attribute_namespace(&g_namespace);
}

// EDG's handlers for the diagnostic pragmas are reached through the function_pointers table (mem_manage.h: an index in
// a_pragma_kind_description, so that PCH files survive ASLR).  The table is a plain (non-const) array, filled statically
// and read at each call (pragma.c, index_to_function_pointer), so entries replaced before edg_main() take effect;
// mem_manage.h declares it already, and the declaration above only restates it.
namespace {
std::vector<a_source_position> g_diag_pragma_sites;

void record_diag_pragma(a_pending_pragma_ptr ppp)
{
    // the start of the directive (for _Pragma, its string literal is id_position)
    a_source_position p {ppp->pragma_position};
    if (p.seq == 0U)
    {
        p = ppp->id_position;
    }
    g_diag_pragma_sites.push_back(p);
}
void diag_pragma_recorder(a_pending_pragma_ptr ppp)
{
    record_diag_pragma(ppp);
    diag_pragma(ppp); // EDG's handler, unchanged
}
void diagnostic_pragma_recorder(a_pending_pragma_ptr ppp)
{
    record_diag_pragma(ppp);
    diagnostic_pragma(ppp);
}
} // namespace

void install_diag_pragma_recorder()
{
    function_pointers[fn_for_function(diag_pragma)] = reinterpret_cast<a_function_pointer>(&diag_pragma_recorder);
    function_pointers[fn_for_function(diagnostic_pragma)] =
        reinterpret_cast<a_function_pointer>(&diagnostic_pragma_recorder);
}

const std::vector<a_source_position>& diag_pragma_sites()
{
    return g_diag_pragma_sites;
}

} // namespace cerastes
