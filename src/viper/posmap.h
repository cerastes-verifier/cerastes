// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The position map (internal/encoding.md section 5): for every line of the emitted Viper that can produce a
// verification error, which C/C++ position and message a failure stands for.
#pragma once

#include <string>
#include <vector>

namespace cerastes::viper {

struct MapEntry {
  int vpr_line = 0;
  std::string viper_id;   // the id of Silicon's error kind (scripts/run_tests.py table)
  std::string category;   // a driver category (0011 item 10) or "internal"
  std::string file;
  int line = 0;
  int column = 0;
  std::string message;
  bool has_note = false;
  std::string note_file;
  int note_line = 0;
  std::string note_text;
};

// One line per entry, tab-separated: vpr line, viper error id, category, file, line, column, message[, note],
// where a note is `<file>:<line>:<text>`.  Tabs and newlines in free text become spaces.
//
// An entry whose C/C++ position (or the position of its note) is below line 1 or column 1 is an internal
// inconsistency: it is described in `problems` (when given) and written as it is, never clamped to a position
// the program does not have.
std::string render_map(const std::vector<MapEntry>& entries, std::vector<std::string>* problems = nullptr);

}  // namespace cerastes::viper
