// expect: rejected cerastes-diagnostic_pragma_in_verify @ L5
// mode: c23, c++20
// twin: tests/scalar/const_global.c
// Pins: 0017 item 2: EDG's own diagnostic pragmas cannot be used in a verified file to silence its refusals
#pragma diag_suppress 161
#include <cerastes.h>

#pragma clang attribute push (__attribute__((loader_uninitialized)), apply_to = variable(is_global))
static const int z = 0;
#pragma clang attribute pop

int f(void)
    CERASTES_ENSURES(r, r == 0)
{
  return z;
}
