// expect: rejected unrecognized_pragma @ L8
// expect: rejected unrecognized_pragma @ L15
// mode: c23, c++20
// twin: tests/scalar/const_global.c
// Pins: 0017 item 2: #pragma clang attribute can make a function weak; the pragma is an error in verify mode
#include <cerastes.h>

#pragma clang attribute push (__attribute__((weak)), apply_to = function)
int f(int x)
    CERASTES_REQUIRES(0 <= x && x < 100)
    CERASTES_ENSURES(r, r == x + 1)
{
  return x + 1;
}
#pragma clang attribute pop
