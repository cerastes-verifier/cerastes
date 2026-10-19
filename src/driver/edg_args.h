// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The command-line refusals that `cerastes-fe` and `cerastes verify` share (decisions 0007, 0008, 0011).
// No EDG headers.
#pragma once

#include <string>
#include <string_view>

namespace cerastes::args {

// What checking one `cerastes-fe` argument found.  `message` is null when the argument is acceptable (it is
// then passed on to EDG); `consumes_next` says that the following word is this option's value.
struct FeCheck {
  const char* message = nullptr;
  bool consumes_next = false;
};

// `cerastes-fe` passes every EDG option through, so it refuses the ones cerastes cannot support (the list in
// src/driver/main.cpp's header comment), under every spelling EDG resolves to them.  `next` is the following
// argument or nullptr.
FeCheck check_fe_argument(const char* arg, const char* next);

// The refusal for `-D<value>` / `-U<value>` (`option` is "-D", "-U", "--define_macro" or "--undefine_macro"), or an
// empty string.  Always refused, in both drivers: `__CERASTES__` (0008), `defined`, every reserved identifier (`__x`,
// `_X`: the predefined macros among them) and the names the annotation machinery reads (`cerastes`; `requires`,
// `ensures`, `invariant`, `check`, `fold`, `unfold`, `predicate`, `pure`; `old`, `owns`, `reads`, `forall`,
// `owns_range`; `CERASTES_H`; `CERASTES_*`).  With `verify_mode` (`cerastes verify`, decision 0011 item 2) every
// `-U` is refused too, and a name that is not an identifier.
std::string macro_refusal(std::string_view value, std::string_view option, bool verify_mode);

}  // namespace cerastes::args
