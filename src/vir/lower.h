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
  // Leave out the invariants `snapshot ==> flag` of loops (decision 0019 item 4), so that a read of a variable given a value before
  // a loop fails inside it: the self-test of why they are there.
  bool drop_snapshot_invariant = false;
  // Move the first snapshot or write of a full expression behind everything, so that `check_variable_order` (the fail-closed check that
  // the accesses to a local are emitted in the order they were created) reports its internal error: the self-test of why it is there.
  bool perturb_access_order = false;
  // Swap the first two fixed points of a full expression's order, so that `check_fixed_order` (the fail-closed check that the fixed points
  // emitted before an item are exactly its set) reports its internal error (decision 0021).
  bool perturb_fixed_order = false;
  // Forget the calls inside `if`, loops and blocks when computing the recursion components, as an under-approximation would: the check of
  // the components against the emitted methods must report it (decision 0021 item 4).
  bool cycles_ignore_nested_calls = false;
};

struct LowerResult {
  low::LProgram program;
  // Internal inconsistencies found while lowering a program that passed check_program (never expected).
  std::vector<Diag> errors;
  // Programs refused by a rule only lowering can decide exactly (decision 0021: the calls of a full expression
  // whose checks do not form a chain).  Reported to the user as rejections (`unsequenced_calls`), at `loc`.
  std::vector<Diag> rejections;
};

LowerResult lower_program(const Program& program, const LowerOptions& options = {});

}  // namespace cerastes::vir
