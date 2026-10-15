// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Diagnostics of the cerastes back end.  They are printed the way EDG prints
// its own (default form: position, severity, message, source line, caret and a
// blank line; with --brief_diagnostics one line with the column; with
// --display_error_number the id `#cerastes-<tag>`), and added to EDG's error
// count, so that EDG's sign-off line and exit status (2) reflect them.
#pragma once

#include <string>
#include <vector>

#include "edg_il.h"

namespace cerastes {

enum class Diag {
#define CERASTES_DIAG(tag, severity, message) tag,
#include "diagnostics.def"
#undef CERASTES_DIAG
};

// The tag of a diagnostic, as tests name it (without the `cerastes-` prefix).
const char* tag_of(Diag d);

// Where a diagnostic about the text at `p` points: the original position of the
// text when it lies in a user file (a macro argument, 0006 item 10), else the
// position in the source file (the start of the macro invocation).
struct DiagPos {
  bool known = false;
  bool in_primary = false;  // the position is not in an include file
  unsigned long seq = 0;  // for ordering
  unsigned long line = 0;
  unsigned long col = 0;
  std::string file;       // display file name
  std::string full_name;  // path used to read the source line, "" if unknown
};
DiagPos diag_position(const a_source_position& p);

// Records a diagnostic.  `args` replace the `%s` of the message in order.
// `context` is one extra line printed after the source line, for example
// `detected during instantiation of "inc<int>"`; empty for none.  Identical
// reports (same tag, position, text) are recorded once, the first one winning.
void report(Diag d, const a_source_position& at, const std::vector<std::string>& args = {},
            const std::string& context = std::string());

// As report(), for a position that is not in the IL: a line (and column) of a
// source file, read from `full_name`.  Identical reports are recorded once.
void report_at(Diag d, const DiagPos& at, const std::vector<std::string>& args = {},
               const std::string& context = std::string());

// Number of distinct diagnostics recorded so far.
size_t reported_count();

// Prints the recorded diagnostics, sorted by position, to stderr and adds them
// to EDG's error count.  Does nothing when none were recorded.
void flush_diagnostics();

}  // namespace cerastes
