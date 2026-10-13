// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Front-end driver: strips cerastes's own options, then runs EDG in process.
// EDG calls back_end() (src/backend/back_end.cpp) when parsing succeeds.
#include <cstdio>
#include <cstring>
#include <vector>

#include "options.h"

// EDG's entry point, renamed from main by EDG_MAIN (host_envir.h).
extern int edg_main(int argc, char** argv);

int main(int argc, char** argv) {
  std::vector<char*> args{argv[0]};
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--remove_unneeded_entities") == 0) {
      // The walker relies on EDG keeping unused entities (never-instantiated
      // templates, G1 check 2); the configuration pins
      // DEFAULT_REMOVE_UNNEEDED_ENTITIES=0 and this option would undo it.
      std::fprintf(stderr,
                   "cerastes-fe: error: --remove_unneeded_entities is not supported: cerastes "
                   "needs the unpruned IL (unused templates and entities)\n");
      return 2;
    }
    if (std::strcmp(argv[i], "--cerastes_walk") == 0)
      cerastes::options().walk = true;
    else if (std::strcmp(argv[i], "--cerastes_walk_selftest") == 0)
      cerastes::options().walk = cerastes::options().walk_selftest = true;
    else
      args.push_back(argv[i]);
  }
  args.push_back(nullptr);
  return edg_main(static_cast<int>(args.size()) - 1, args.data());
}
