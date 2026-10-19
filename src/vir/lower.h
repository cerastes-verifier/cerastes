// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Lowering: VIR (checked by check_program) to the lowered program (lowered.h).  This is where integer
// semantics live (decisions 0010 and 0012; internal/encoding.md section 3).  The soundness auditor reviews it.
#pragma once

#include <string>
#include <vector>

#include "check.h"
#include "lowered.h"
#include "vir.h"

namespace cerastes::vir {

// Hooks for paths no correct program reaches (0011 item 13), off by default.
struct LowerOptions {
  // Drop the `n`-th assert (counting from 0, in emission order) so that Silicon's own check of a method
  // precondition or postcondition fires.  -1: drop none.
  int drop_assert = -1;
  // Weaken every automatic range invariant so that it fails.
  bool weaken_auto_invariant = false;
};

struct LowerResult {
  low::LProgram program;
  // Internal inconsistencies found while lowering a program that passed check_program (never expected).
  std::vector<Diag> errors;
};

LowerResult lower_program(const Program& program, const LowerOptions& options = {});

}  // namespace cerastes::vir
