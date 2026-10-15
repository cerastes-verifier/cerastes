// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The registration table of the `cerastes` attribute namespace: decision 0007,
// "The cerastes registration table".  Signatures use EDG's argument-signature
// grammar (attribute.c, an_attr_descr) and targets its appertainment grammar:
//   (Xb)  one unevaluated expression, contextually converted to bool
//   (R,Xb) an optional result name (an identifier), then (Xb)
//   (X)   one unevaluated expression
//   r     a routine (a function; for RAF_ENTITY_ATTACHED, after the parameter list)
//   s:+i! a statement that is an iteration statement
//   s:+n! a statement that is a null statement
#include "register_attributes.h"

#include "edg_il.h"
#include "attribute.h"

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
    // result-name signature code, decision 0007 item 4); it needs RAF_ENTITY_ATTACHED.
    {"ensures", "(R,Xb)", "r", kContract},
    {"invariant", "(Xb)", "s:+i!", RAF_NO_FLAGS},
    {"check", "(Xb)", "s:+n!", RAF_NO_FLAGS},
    {"fold", "(X)", "s:+n!", RAF_NO_FLAGS},
    {"unfold", "(X)", "s:+n!", RAF_NO_FLAGS},
    // predicate/pure accumulate (no must-match): the mark on the first
    // declaration covers the definition (0006 item 6).
    {"predicate", "", "r", RAF_ENTITY_ATTACHED},
    {"pure", "", "r", RAF_ENTITY_ATTACHED},
    {nullptr, nullptr, nullptr, RAF_NO_FLAGS},
};

an_attr_namespace_registration g_namespace = {"cerastes", RANF_CLOSED, kAttributes, nullptr};

}  // namespace

void register_cerastes_annotations() { register_attribute_namespace(&g_namespace); }

}  // namespace cerastes
