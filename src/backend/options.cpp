// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "options.h"

namespace cerastes {
Options& options() {
  static Options o;
  return o;
}
}  // namespace cerastes
