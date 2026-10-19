// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// From Silicon's verdict to what `cerastes verify` prints (decision 0011 items 8, 9 and 11): the position map is
// read, each Silicon error is matched to its entry by Viper line and error id, and the user-facing errors are
// sorted and rendered.  No EDG headers.
#pragma once

#include <string>
#include <vector>

#include "silicon_output.h"

namespace cerastes {

// One line of the position map (internal/encoding.md section 5).
struct MapRow {
  int vpr_line = 0;
  std::string viper_id;
  std::string category;
  std::string file;
  int line = 0;
  int column = 0;
  std::string message;
  bool has_note = false;
  std::string note_file;
  int note_line = 0;
  std::string note_text;
};

// Parses the map text; `problem` is set (and the rows so far returned) when a line is malformed.
std::vector<MapRow> parse_position_map(const std::string& text, std::string* problem);

// One verification error as printed.
struct UserError {
  std::string file;
  int line = 0;
  std::string category;
  std::string message;
  bool has_note = false;
  std::string note_file;
  int note_line = 0;
  std::string note_text;
};

struct MappedVerdict {
  std::vector<UserError> errors;      // sorted, without duplicates
  std::vector<std::string> failures;  // reasons for exit status 3 (tool failures), one line each
};

// `vpr_name` is the file name Silicon was given (its positions must name it).
MappedVerdict map_errors(const SiliconVerdict& verdict, const std::vector<MapRow>& map, const std::string& vpr_name);

// `<file>:<line>: error: <category>: <message>` and the `note:` line, each ending in a newline.
std::string render_error(const UserError& e);

}  // namespace cerastes
