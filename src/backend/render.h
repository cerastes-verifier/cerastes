// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Rendering of EDG IL entities as text.  Types, names and template arguments
// go through EDG's own formatter (il_to_str.c), as diagnostics and il_display
// do; only positions, attributes and constraint expressions are rendered here.
#pragma once

#include <string>

#include "edg_il.h"

namespace cerastes {

// A resolved source position.  `known` is false for compiler-generated
// entities (sequence number 0).
struct SrcPos
{
    bool known = false;
    bool in_primary = false; // not in an include file (system headers included)
    // in a system header: EDG's own notion, except that in verify mode a library file (0046 item 3) is not one
    bool in_system = false;
    std::string file; // display file name of the position, "" if unknown
    unsigned long line = 0;
    unsigned long col = 0;
    std::string str() const; // "L<line>:<col>"
};
SrcPos resolve(const a_source_position& p);

// Whether sequence number `seq` lies in a system file: EDG's own flag in walk mode; in verify mode "not a verified file" by canonical path
// (decision 0046 item 3: a library file is not a system file, and neither is a user header that a system header includes, which EDG
// flags).  Every use of the flag outside this function is by path (is_system_file, in_verified_file) or reads a file record's own flag
// for a directive (annotation_checks.cpp).
bool seq_is_in_system_file(a_seq_number seq);

// "L<line>:<col>" of a position that is part of an annotation or names a lambda,
// or "-": the original position of text from a macro argument when it lies in a
// user file, else the position in the source file (the same rule as cerastes
// diagnostics, 0006 item 10).  A position that is not in the primary source
// file is prefixed with its file name as diagnostics print it: "<file>:L<l>:<c>".
std::string position_label(const a_source_position& p);

std::string render_type(a_type_ptr t);
// "T=int, U=short" for a template's argument list (a pack as "Ts=pack{int,
// long}"); parameter names come from `tmpl` (may be null).  "none" for an
// empty list.
std::string render_template_args(a_template_ptr tmpl, a_template_arg_ptr list);
// The value of a constant: an integer constant as its value (no cast), any other
// kind as "<ck_...>".
std::string render_constant(a_constant_ptr c);
// Qualified name of an entity that starts with a source correspondence.
std::string render_name(a_source_correspondence* scp, an_il_entry_kind kind);
std::string render_template_arg(a_template_arg_ptr a);
// "[[ns::name(raw tokens joined by single spaces)]]"
std::string render_attribute(an_attribute_ptr a);
// Constraint expression: concept-ids, && and ||; anything else <expr:kind>.
std::string render_constraint(an_expr_node_ptr e);

} // namespace cerastes
