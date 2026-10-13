// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Enumerator names of EDG IL enums (EDG's own tables are static).  The bodies
// are generated from il_def.h at build time by cmake/gen_enum_names.py.
#pragma once

#include "edg_il.h"

const char* name_of(an_expr_operator_kind v);   // eok_*
const char* name_of(an_expr_node_kind v);       // enk_*
const char* name_of(a_type_kind v);             // tk_*
const char* name_of(a_statement_kind v);        // stmk_*
const char* name_of(a_template_param_constant_kind v);  // tpck_*
const char* name_of(a_constant_repr_kind v);    // ck_*
const char* name_of(a_template_param_type_kind v);  // tptk_*
