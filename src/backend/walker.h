// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#pragma once

namespace cerastes {

// Print a deterministic text description of the routines (plain, prototype,
// instance, specialization) in the primary source file to stdout.  Called from
// back_end() under --cerastes_walk; its output is the basis of the golden
// tests in tests/il/.
void walk_il_and_print();

}  // namespace cerastes
