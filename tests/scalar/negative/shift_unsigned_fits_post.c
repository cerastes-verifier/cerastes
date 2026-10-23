// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/shift_unsigned_fits.c
// Pins: 0022 item 1: 3u << 30 is 3221225472u, not 3221225471u
#include <cerastes.h>

unsigned shl_u(unsigned a)
    CERASTES_REQUIRES(a == 3u)
    CERASTES_ENSURES(r, r == 3221225471u)
{
  return a << 30;
}
