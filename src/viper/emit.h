// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Printing the lowered program as Viper text and its position map (internal/encoding.md sections 4 and 5).
// It prints; it decides nothing.  Every check is on a line of its own and has a map entry.
#pragma once

#include <string>
#include <vector>

#include "lowered.h"

namespace cerastes::viper {

struct EmitOptions {
  // Text written before everything else (for a harness `// expect:` header); it should end with a newline.
  // The lines it contains count in the position map.
  std::string header;
};

struct Emitted {
  std::string vpr;  // the Viper program
  std::string map;  // the position map (posmap.h)
  // Internal inconsistencies found while printing (a map entry without a C/C++ position); empty for a sound
  // program.  A caller treats any as a failure of the encoder.
  std::vector<std::string> problems;
};

Emitted emit(const low::LProgram& program, const EmitOptions& options = {});

}  // namespace cerastes::viper
