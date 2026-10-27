// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The state of a `cerastes verify` run that the in-process back end needs (decision 0011), and the
// classification of source files into verified and system files by path (0011 item 4).  No EDG headers:
// the driver's unit tests (`cerastes --selftest`) use the path classifier without a front end.
#pragma once

#include <string>
#include <vector>

namespace cerastes {

// What the encoder produced (set by verify_encode(), read by the driver after edg_main returns).
struct VerifyOutput {
  bool encoded = false;  // the encoder ran to the end without rejecting anything
  std::string vpr;       // the Viper program
  std::string map;       // its position map (internal/encoding.md section 5)
  int functions = 0;     // functions whose bodies were verified
  // Decision 0022 item 12: lowering's internal errors, the emitter's problems or the encoder's "encoded twice" guard fired.  The
  // diagnostics are shown as usual, but the run is a tool failure (exit status 3), never a rejection (2).
  bool internal_inconsistency = false;
};

struct VerifyConfig {
  bool on = false;
  // The self-test hook of 0011 item 13 ("" for none): the names and what each does are listed above is_hook in
  // src/driver/verify_main.cpp, the one place that accepts them.
  std::string hook;
  // Canonical (realpath) system directories: the driver's own system include directories, without
  // /usr/local/include (a user directory).  A file under one of them is a system file.
  std::vector<std::string> system_dirs;
  // Canonical path of cerastes's own include/cerastes.h: the one file of that directory that is a system file.
  std::string cerastes_header;
  VerifyOutput out;
};
VerifyConfig& verify_config();

// The driver's status after the front end returned `edg_status` (nonzero: a diagnostic was reported, by EDG or by the back end):
// 3 when the back end flagged an internal inconsistency (decision 0022 item 12, which amends 0011 item 8), else 2 for a nonzero
// `edg_status`, else 0.
int front_end_exit_status(const VerifyOutput& out, int edg_status);

// The canonical physical path (realpath: `..`, `.` and symbolic links resolved), or "" when the file does
// not exist.
std::string canonical_path(const std::string& path);

// Whether `path` lies strictly below `dir`, compared component by component (so `/a/bc` is not under `/a/b`).
// Both must be canonical.
bool path_is_under(const std::string& path, const std::string& dir);

// The canonicalized, de-duplicated system directories out of the driver's include directories: each is
// made canonical (a missing one is dropped), and `/usr/local/include` and anything the caller names in
// `user_dirs` is left out, as a user directory.
std::vector<std::string> system_directories(const std::vector<std::string>& dirs,
                                            const std::vector<std::string>& user_dirs);

// 0011 item 4: a system file is one under a system directory, or exactly the cerastes header; decided by
// canonical path.  A file that does not exist (so that it cannot be canonicalized) is not a system file:
// it is verified, or the run fails when the front end could not read it.
bool is_system_file(const std::vector<std::string>& system_dirs, const std::string& cerastes_header,
                    const std::string& path);

// Convenience: is_system_file with the configuration of this run.
bool is_system_file(const std::string& path);

}  // namespace cerastes
