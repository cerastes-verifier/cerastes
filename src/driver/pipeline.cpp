// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "pipeline.h"

#include <fstream>

#include "report.h"
#include "silicon_output.h"

namespace cerastes {
namespace {

bool write_file(const std::string& path, const std::string& text) {
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  out << text;
  out.flush();
  return static_cast<bool>(out);
}

}  // namespace

PipelineResult run_pipeline(const PipelineOptions& options, const std::string& vpr, const std::string& map) {
  PipelineResult r;
  auto fail = [&](const std::string& why) {
    r.err += "cerastes: error: " + why + "\n";
    r.exit_code = 3;
  };
  // The kept files first, so that a crash or a timeout still leaves them (decision 0011 item 9).
  if (!options.keep_vpr.empty()) {
    if (!write_file(options.keep_vpr, vpr) || !write_file(options.keep_vpr + ".map", map)) {
      fail("cannot write " + options.keep_vpr + " (--keep-vpr)");
      return r;
    }
  }
  std::string problem;
  std::vector<MapRow> rows = parse_position_map(map, &problem);
  if (!problem.empty()) {
    fail(problem);
    return r;
  }

  SiliconRun run = run_silicon(options.tools, vpr, options.timeout_seconds);
  if (!run.started) {
    fail("Silicon could not be run: " + run.error);
    return r;
  }
  if (run.timed_out) {
    fail("Silicon did not finish within " + std::to_string(options.timeout_seconds) + " seconds (--timeout)");
    return r;
  }
  if (run.signaled) {
    fail("Silicon was killed by a signal");
    return r;
  }
  SiliconVerdict verdict = parse_silicon_output(run.output, run.exit_code);
  MappedVerdict mapped = map_errors(verdict, rows, run.vpr_name);
  for (const UserError& e : mapped.errors) r.out += render_error(e);
  for (const std::string& f : mapped.failures) r.err += "cerastes: error: " + f + "\n";
  if (!mapped.failures.empty()) {
    r.exit_code = 3;
    return r;
  }
  if (!mapped.errors.empty()) {
    r.exit_code = 1;
    if (!options.hook.empty()) {
      r.exit_code = 3;
      r.err += "cerastes: error: self-test hook " + options.hook + " was active: exit status forced to 3\n";
    }
    return r;
  }
  if (!verdict.verified) {
    fail("Silicon gave no verdict");
    return r;
  }
  if (!options.hook.empty()) {
    // A hooked run takes a path no correct program can (0011 item 13); it never ends in `verified`.
    fail("self-test hook " + options.hook + " was active and the program still verified");
    return r;
  }
  r.out += options.primary_file + ": verified (" + std::to_string(options.functions) +
           (options.functions == 1 ? " function" : " functions") +
           "; one translation unit: assumes that functions defined elsewhere satisfy the contracts declared here)\n";
  r.exit_code = 0;
  return r;
}

}  // namespace cerastes
