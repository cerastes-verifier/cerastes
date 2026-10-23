// expect: error shift @ L11
// mode: c23, c++20
// twin: tests/scalar/shift_unsigned_fits.c
// Pins: 0022 item 1: 3000000000u << 1 does not fit unsigned int: shift, no wrap
#include <cerastes.h>

unsigned shl_u(unsigned a)
    CERASTES_REQUIRES(a == 3000000000u)
    CERASTES_ENSURES(r, r == 1705032704u)
{
  return a << 1;
}
