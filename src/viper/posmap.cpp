// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "posmap.h"

namespace cerastes::viper {

namespace {
std::string flat(std::string s) {
  for (char& c : s)
    if (c == '\t' || c == '\n' || c == '\r') c = ' ';
  return s;
}
}  // namespace

std::string render_map(const std::vector<MapEntry>& entries, std::vector<std::string>* problems) {
  std::string out;
  for (const MapEntry& e : entries) {
    if (problems != nullptr && (e.line < 1 || e.column < 1))
      problems->push_back("the position map entry for line " + std::to_string(e.vpr_line) + " of the Viper program (" +
                          e.category + ": " + flat(e.message) + ") has no C/C++ position");
    if (problems != nullptr && e.has_note && e.note_line < 1)
      problems->push_back("the note of the position map entry for line " + std::to_string(e.vpr_line) +
                          " of the Viper program has no C/C++ position");
    out += std::to_string(e.vpr_line) + '\t' + e.viper_id + '\t' + e.category + '\t' + flat(e.file) + '\t' +
           std::to_string(e.line) + '\t' + std::to_string(e.column) + '\t' + flat(e.message);
    if (e.has_note) out += '\t' + flat(e.note_file) + ':' + std::to_string(e.note_line) + ':' + flat(e.note_text);
    out += '\n';
  }
  return out;
}

}  // namespace cerastes::viper
