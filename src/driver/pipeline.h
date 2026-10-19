// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// From a Viper program and its position map to the driver's verdict, output and exit status (decision 0011
// items 8, 9 and 11).  No EDG headers.
#pragma once

#include <string>

#include "silicon.h"

namespace cerastes {

struct PipelineOptions {
  SiliconTools tools;
  int timeout_seconds = 300;
  std::string keep_vpr;      // "": do not keep; else write this path and `<path>.map`
  std::string primary_file;  // the verdict line names it
  int functions = 0;         // how many functions were verified or used through a contract (the verdict line says so)
  std::string hook;          // the self-test hook of a hooked run (decision 0011 item 13); "" for none
};

struct PipelineResult {
  int exit_code = 3;  // 0 verified, 1 verification errors, 3 tool failure (always 3 for a hooked run)
  std::string out;    // for standard output
  std::string err;    // for standard error
};

// Writes the kept files, runs Silicon, maps its errors.  Never returns exit status 2.
PipelineResult run_pipeline(const PipelineOptions& options, const std::string& vpr, const std::string& map);

}  // namespace cerastes
