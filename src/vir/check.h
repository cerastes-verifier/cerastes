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

// What decision 0032 item 10 keeps out of b1a: a loop (every form) holding a pointer-typed node, a cell, `&` or `*`; a `return` inside a
// loop while a block that declares a cell is open; a `break`, `continue` or `goto` that leaves a block declaring a cell (a switch body
// included).  `loc` is the outermost loop statement for a loop (its `loc` is where the encoder put the loop; the encoder reports it at the loop keyword), the `return` or the jump and `what`
// says it in the user's terms, ending "(decision 0032)".  The encoder reports each as `unsupported_statement`; check_program repeats
// them as a backstop.
struct MemoryProblem {
  SrcLoc loc;
  enum class Kind { pointer_in_loop, return_in_loop, jump_out } kind = Kind::pointer_in_loop;
  std::string what;
  const Stmt* loop = nullptr;  // pointer_in_loop: the (outermost) loop statement, whose `loc` is the problem's
};
std::vector<MemoryProblem> memory_problems(const Function& f);

// Checks every function of the program; returns all violations in a deterministic order (functions in
// definition order, nodes in evaluation order).  An empty result means `lower_program` may be called.
std::vector<Diag> check_program(const Program& program);

}  // namespace cerastes::vir
