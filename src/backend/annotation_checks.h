// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The checks of the annotation layer that EDG does not make (decision 0006
// item 10).  Each reports through diagnostics.h; nothing is printed here.
#pragma once

namespace cerastes {

// Runs every check over the IL of the translation unit (routines, their
// contracts, and the statements with annotations).  Called from back_end() on
// every run; the caller prints the diagnostics with flush_diagnostics().
void check_annotations();

}  // namespace cerastes
