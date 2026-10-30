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

// Checks every function of the program; returns all violations in a deterministic order (functions in
// definition order, nodes in evaluation order).  An empty result means `lower_program` may be called.
std::vector<Diag> check_program(const Program& program);

}  // namespace cerastes::vir
