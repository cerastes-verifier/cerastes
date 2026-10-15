// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The in-process EDG back end.  cfe.c calls the global back_end() after the
// front end has finished without errors (decision 0002).  EDG declares it as
// `extern void back_end(void)` in the global namespace with C++ linkage.
#include <cstdio>
#include <cstdlib>

#include "edg_il.h"
#include "options.h"
#include "run_state.h"
#include "walker.h"

void back_end(void) {
  cerastes::note_back_end_ran();
  cerastes::refuse_unsupported_mode();
  // EDG accepts unambiguous abbreviations of --remove_unneeded_entities, so the
  // driver's own check on the option string can be bypassed.  Check the runtime
  // state itself: with pruning on, never-instantiated templates and unused
  // entities are missing from the IL, which the walker would then report as
  // genuinely absent.
  if (remove_unneeded_entities || okay_to_eliminate_unneeded_il_entries) {
    std::fprintf(stderr, "error: cerastes requires --no_remove_unneeded_entities\n");
    std::exit(2);
  }
  if (cerastes::options().walk) cerastes::walk_il_and_print();
}
