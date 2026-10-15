// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Text rendering of the typed expression trees of registered `cerastes::`
// attributes, for --cerastes_walk, and the same walk as the check that nothing
// in a tree is beyond what this version understands.  The grammar is listed at
// the top of expr_tree.cpp.
#pragma once

#include <string>
#include <vector>

#include "edg_il.h"

namespace cerastes {

// A construct in an annotation tree that this version does not understand
// (invariant 3: it is rejected, never approximated): a node kind the printer has
// no line for, a parameter reference that the convention does not explain, a
// node reached twice, a tree deeper than the cap, a lambda whose body is not a
// single `return <expression>;`.
struct TreeProblem {
  a_source_position at;
  std::string what;
  // The problem is explained by the side-effect diagnostic when the annotation
  // has one (a lambda body of several statements that assigns): the check does not
  // report it a second time.
  bool explained_by_side_effect = false;
};

struct TreeWalk {
  std::vector<TreeProblem> problems;
  // The parameter references of the tree in order, as `P<k>` (parameter k), `R`
  // (the named result) and `T` (this): what a root template and its instance must
  // agree on.
  std::vector<std::string> references;
};

enum class TreeMode { print, check, signature, signature_renumbered };

// Walks every argument of the registered attribute `a`.  With TreeMode::print
// the lines are appended to *out (see print_annotation_args); in the other modes
// *out is not used.  `owner` is the routine the attribute is attached to (it
// names parameter references and the result), or null for a statement.  The
// self-test hook (--cerastes_checks_selftest) acts on the first tree of the
// print mode and of the check mode, separately.
TreeWalk annotation_tree(an_attribute_ptr a, a_routine_ptr owner, TreeMode mode, std::string* out,
                         int indent);

// Whether the contract of `r` (an instance, a specialization or a member-template
// prototype) agrees with that of its root template `root`: the same
// requires/ensures in the same order with the same number of arguments, and the
// same parameter, result and `this` references in each (so a parameter that is
// numbered differently in an instance is found).
//
// `perturb` is the self-test (--cerastes_checks_selftest): the data of the
// instance is changed so that the real comparison fails: the first instance
// loses its first contract, the others have their parameter references
// renumbered.
bool contracts_agree(a_routine_ptr root, a_routine_ptr r, bool perturb = false);

// "L<line>:<col>" of a position in an annotation, or "-" (see position_label).
std::string annotation_position(const a_source_position& p);

// Appends one `arg` line per argument of the registered attribute `a`, each
// followed by its tree.  `indent` is the number of spaces before `arg`; the
// tree is indented two more per level.  The output ends with a newline.
void print_annotation_args(std::string& out, an_attribute_ptr a, a_routine_ptr owner, int indent);

}  // namespace cerastes
