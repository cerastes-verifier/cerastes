// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "run_state.h"

#include <cstdio>
#include <cstdlib>

#include "edg_il.h"
#include "cmd_line.h"

namespace cerastes {
namespace {
bool g_back_end_ran = false;
}

void note_back_end_ran() { g_back_end_ran = true; }
bool back_end_ran() { return g_back_end_ran; }

void print_refusal(const char* message) { std::fprintf(stderr, "cerastes-fe: error: %s\n", message); }

void refuse_unsupported_mode() {
  if (C_mode() && !c23_mode) {
    print_refusal("C before C23 is not supported: use --c23 (older C lets identifier-list "
                  "declarators bind a contract's names to globals; decision 0007)");
    std::exit(2);
  }
  if (gnu_mode && !clang_mode) {
    print_refusal("GNU emulation (--gcc, --g++, --gnu_version) is not supported: cerastes reads "
                  "C++ in Clang emulation mode (decision 0007)");
    std::exit(2);
  }
}

void guard_back_end_ran() {
  // Early exits (--dump_configuration, --version, -E) never start a
  // translation unit and leave suppress_back_end FALSE.  When it is TRUE and
  // nothing ran, the user asked for a front-end-only run (--no_code_gen):
  // an ordinary EDG success, but for cerastes nothing was checked.
  if (g_back_end_ran || is_at_least_one_error() || do_preprocessing_only || !suppress_back_end) return;
  print_refusal("the cerastes back end did not run (for example --no_code_gen): nothing was checked");
  std::fflush(stdout);
  std::_Exit(2);
}

}  // namespace cerastes
