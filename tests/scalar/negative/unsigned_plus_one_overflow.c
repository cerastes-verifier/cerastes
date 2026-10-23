// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/scalar/unsigned_incdec_range.c
// Pins: 0022 items 1 and 4: u += 1 at UINT_MAX overflows (the literal needs no check, the sum does)
#include <cerastes.h>

unsigned plus_one(unsigned u)
    CERASTES_REQUIRES(u == 4294967295u)
    CERASTES_ENSURES(r, r == 0u)
{
  u += 1;
  return u;
}
