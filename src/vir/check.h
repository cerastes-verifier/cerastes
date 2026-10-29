// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The typing and well-formedness rules of VIR (internal/encoding.md section 2).  They fail closed: a tree that
// the lowering could mis-evaluate is rejected here, so that "an operand type is not what the operation
// requires" (a conversion the IL left implicit) can never turn into a silently wrong Viper program.
#pragma once

#include <string>
#include <vector>

#include "vir.h"

namespace cerastes::vir {

// One violation: where (the offending node) and why.  The encoder reports it as `unsupported_expression`
// (or `unsupported_statement` for a statement-level reason) at `loc`.
struct Diag {
  SrcLoc loc;
  std::string reason;
};

// Decision 0037 item 8 (placement until M4.5b1c2): a full expression that holds a call of a function with a permission atom in its `requires`
// has no other access to a cell (a `Deref`, a read of an address-taken variable by name) outside that call's arguments, and no second such
// call.  The target of a statement's own store is no expression and is no access; the read of a compound assignment's target is one.  `loc` is
// the offending access or second call, `what` says it in the user's terms (naming M4.5b1c2 and decision 0028 item 13).  The encoder reports
// each as `unsupported_expression`; check_program repeats them as a backstop.
struct PlacementProblem {
  SrcLoc loc;
  std::string what;
};
std::vector<PlacementProblem> placement_problems(const Function& f);

// Checks every function of the program; returns all violations in a deterministic order (functions in
// definition order, nodes in evaluation order).  An empty result means `lower_program` may be called.
std::vector<Diag> check_program(const Program& program);

}  // namespace cerastes::vir
