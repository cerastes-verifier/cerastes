// expect: rejected unrecognized_pragma @ L8
// expect: rejected unrecognized_pragma @ L10
// mode: c23, c++20
// twin: tests/scalar/const_global.c
// Pins: 0017 item 2: #pragma clang attribute could change what a constant means; verify mode makes an unrecognized pragma an error (#161)
#include <cerastes.h>

#pragma clang attribute push (__attribute__((loader_uninitialized)), apply_to = variable(is_global))
static const int z = 0;
#pragma clang attribute pop

int f(void)
    CERASTES_ENSURES(r, r == 0)
{
  return z;
}
