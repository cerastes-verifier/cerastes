// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// cerastes's own command-line options, removed from argv before EDG sees it.
#pragma once

namespace cerastes {
struct Options {
  bool walk = false;  // --cerastes_walk: print the IL walk from back_end()
  // --cerastes_walk_selftest: test hook that makes the walker's cross-checks
  // behave as if no prototype routine had been seen and no instance had an
  // assoc_template, to prove the two `warning:` lines fire.
  bool walk_selftest = false;
};
Options& options();
}  // namespace cerastes
