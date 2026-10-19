// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Reading Silicon's command-line output (decision 0011 item 11).  This is a port of `parse_silicon_output` and
// `classify` in scripts/run_tests.py: the table of error kinds below is a copy of `_VIPER_ERRORS` and
// `_VIPER_REASONS` there, pinned to the Silicon version; the harness self-test runs both on the same
// recorded outputs (`cerastes --cerastes_classify_silicon`) and requires agreement.  No EDG headers.
#pragma once

#include <string>
#include <vector>

namespace cerastes {

struct SiliconError {
  std::string viper_id;   // Viper's error id ("assert.failed"), "unknown" for a message that is not a verification error
  std::string category;   // the harness name of the kind ("assert", "postcondition", ...), "unknown"
  std::string reason;     // the Viper reason id, "timeout" for a solver timeout, "unknown"
  std::string file;       // the file of the position Silicon printed ("" when it printed none)
  int line = 0;
  std::string message;    // the whole message on one line
};

struct SiliconVerdict {
  bool verified = false;
  std::vector<SiliconError> errors;
  std::string problem;  // set when the output is not a clean verdict
};

SiliconError classify_silicon_message(const std::string& message);

// `output` is Silicon's standard output and standard error; `exit_code` its exit status.
SiliconVerdict parse_silicon_output(const std::string& output, int exit_code);

}  // namespace cerastes
