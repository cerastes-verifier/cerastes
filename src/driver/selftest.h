// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// `cerastes --selftest`: the driver's own unit tests (decision 0011 item 13).  The path rules of item 4 need
// absolute paths and directories no test file can name, so they are checked here, not by programs; so is the
// reading of Silicon's output, the position map, and a whole pipeline from VIR to Silicon's verdict.
#pragma once

#include "silicon.h"

namespace cerastes {

// Runs every check; prints a line for each failure and a summary.  Returns 0 when all pass, 1 otherwise.
int run_selftest(const SiliconTools& tools);

}  // namespace cerastes
