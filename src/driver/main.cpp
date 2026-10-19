// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Front-end driver: registers the cerastes annotations with EDG, refuses the
// options cerastes does not support, strips its own options, defines
// __CERASTES__, then runs EDG in process.  EDG calls back_end()
// (src/backend/back_end.cpp) when parsing succeeds.
//
// Every refusal is ONE line on stderr, `cerastes-fe: error: <message>`, and exit
// status 2, before EDG runs (tests/README.md, "Refused options").  The options
// refused here, and why:
//   --remove_unneeded_entities  the walker needs the unpruned IL (decision 0002)
//   -U / -D / --undefine_macro / --define_macro of __CERASTES__
//                               cerastes defines it itself (decision 0008)
//   --gcc --no_gcc --g++ --no_g++ --gnu_version
//                               GNU emulation (decision 0007)
//   --c --c89 --c99 --no_c99 --c11 --c17 --c18 --old_c -K
//                               C before C23 (decision 0007)
//   --create_pch --use_pch --pch --pch_dir
//                               a precompiled header skips the reading of its files
//   --output_mode=sarif         back-end diagnostics cannot be emitted as SARIF yet
// EDG accepts an unambiguous abbreviation of a long option, so a refused option
// is recognised under every spelling EDG would resolve to it.  The language
// modes are checked again on the state itself in back_end() (run_state.cpp).
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

#include "edg_args.h"
#include "options.h"
#include "register_attributes.h"
#include "run_state.h"

// EDG's entry point, renamed from main by EDG_MAIN (host_envir.h).
extern int edg_main(int argc, char** argv);

int main(int argc, char** argv) {
  std::vector<char*> args{argv[0]};
  // cerastes defines __CERASTES__ itself, so that a direct run of this binary
  // cannot silently drop every contract (decision 0008).  It comes before the
  // user's options.
  static char define_cerastes[] = "-D__CERASTES__=1";
  args.push_back(define_cerastes);
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--cerastes_walk") == 0) {
      cerastes::options().walk = true;
    } else if (std::strcmp(argv[i], "--cerastes_walk_selftest") == 0) {
      cerastes::options().walk = cerastes::options().walk_selftest = true;
    } else if (std::strcmp(argv[i], "--cerastes_checks_selftest") == 0) {
      cerastes::options().checks_selftest = true;
    } else {
      cerastes::args::FeCheck check = cerastes::args::check_fe_argument(argv[i], i + 1 < argc ? argv[i + 1] : nullptr);
      if (check.message != nullptr) {
        cerastes::print_refusal(check.message);
        return 2;
      }
      args.push_back(argv[i]);
      if (check.consumes_next) args.push_back(argv[++i]);
    }
  }
  args.push_back(nullptr);
  cerastes::register_cerastes_annotations();
  std::atexit(cerastes::guard_back_end_ran);
  return edg_main(static_cast<int>(args.size()) - 1, args.data());
}
