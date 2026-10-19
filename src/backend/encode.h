// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The verify-mode encoder: the IL of the translation unit to VIR (decision 0012, internal/encoding.md
// section 7), then VIR to Viper text and its position map.
#pragma once

namespace cerastes {

// Called from back_end() in verify mode, after the annotation checks found nothing.  Rejections are
// reported through diagnostics.h (the caller flushes them).  When nothing was rejected, the program and
// its position map are in verify_config().out.
void verify_encode();

}  // namespace cerastes
