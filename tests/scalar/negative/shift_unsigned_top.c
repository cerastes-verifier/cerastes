// expect: error shift @ L11
// mode: c23, c++20
// twin: tests/scalar/shift_unsigned_fits.c
// Pins: 0022 item 1: 2u << 31 is 2^32, one bit too many: shift (the boundary)
#include <cerastes.h>

unsigned top_bit(unsigned a)
    CERASTES_REQUIRES(a == 2u)
    CERASTES_ENSURES(r, r == 0u)
{
  return a << 31;
}
