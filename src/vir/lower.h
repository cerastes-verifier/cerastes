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
  // Move the first write (an assignment used as a value) that has a non-empty set before every fixed point, so that `check_fixed_order`
  // (a write comes after every member of its set, decision 0025 item 2) reports its internal error.
  bool perturb_write_slot = false;
  // Undo decision 0025 item 1: a flagged snapshot records only its own set as the access to its local, not its `uninitialized` check's, so a later
  // write may be emitted before the check; `check_variable_order`'s counting of flag reads must report it (internal error).
  bool drop_flag_access = false;
  // Move the first write that a later item depends on (a later item whose set contains the write's non-empty set) to the last slot, so that the
  // half of `check_fixed_order` that requires such an item to be emitted after the write reports its internal error (decision 0025 item 2).
  // The self-test case must place the dependent item before the last slot: the check fires only when the dependent item lands in an earlier
  // slot than the delayed write, as in `hook_delay_write_slot`'s `h(b) < 100 / (x = g(a))` (the divisor check, slot 0, the write moved to slot 1).
  bool delay_write_slot = false;
  // Swap the first two fixed points of a full expression's order, so that `check_fixed_order` (the fail-closed check that the fixed points
  // emitted before an item are exactly its set) reports its internal error (decision 0021).
  bool perturb_fixed_order = false;
  // Skip step 1 of `flush` (decision 0026 item 2): every full expression takes its base sets, today's (decision 0021 item 10), exactly as
  // when the exact family is not a chain, and a conflict is reported from the base family.  The Viper of a program is then the one emitted
  // before exact sets (decision 0026 item 2: step 2's Viper is that of M4.4c), and a case of a check that only the base sets need (such as
  // `drop_flag_access`, whose shape the exact sets already order) can reach it.
  bool force_base_sets = false;
  // Clear the exact half of the set of the first `requires` obligation whose base half is not empty, so that the base half is no longer
  // contained in the exact half: the permanent check of `flush` that every base half is contained in its exact half (decision 0026
  // item 3) reports its internal error ("the base half of a set of a full expression is not contained in its exact half").  The self-test
  // case needs a call whose argument is a call that a `requires` clause reads (`h(g(a))`, `requires` on the argument).
  bool drop_exact_set = false;
  // Forget the calls inside `if`, loops and blocks when computing the recursion components, as an under-approximation would: the check of
  // the components against the emitted methods must report it (decision 0021 item 4).
  bool cycles_ignore_nested_calls = false;
  // Move the first write of a cell used as a value, whose set is not empty, before every fixed point, so that the fail-closed check of decision
  // 0039 item 4 (a cell write is on C's side of every memory call it is ordered with) reports its internal error.
  bool perturb_memory_order = false;
  // A snapshot of a cell read does not follow its guard's set (decision 0039 item 2, audit B1): the same check must report it for
  // `bump(&x) && x == 0`, whose read would be emitted before the call.
  bool drop_snapshot_guard_set = false;
  // Skip the join of a left operand's (or a `?:` condition's) cell writes and memory calls into the sets of the right operand's cell reads and
  // branches (decision 0039 item 2, audit R2-B1): `return ((*pick(q) = 5) == 5) && *q == 5;` would read *q before the store, and the access-pair half of the
  // fail-closed check (item 4) must report it.
  bool drop_left_floor = false;
};

struct LowerResult {
  low::LProgram program;
  // Internal inconsistencies found while lowering a program that passed check_program (never expected).
  std::vector<Diag> errors;
  // Programs refused by a rule only lowering can decide exactly (decision 0021: the calls of a full expression
  // whose checks do not form a chain, on the exact sets and on the base sets of decision 0026 part A; the report is the base family's).
  // Reported to the user as rejections (`unsequenced_calls`), at `loc`.
  std::vector<Diag> rejections;
};

LowerResult lower_program(const Program& program, const LowerOptions& options = {});

}  // namespace cerastes::vir
