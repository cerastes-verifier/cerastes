// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/shift_unsigned.c
// Pins: 0010 item 8: the shifted value wraps, it is not 6000000000
#include <cerastes.h>

unsigned shl_u(unsigned a)
    CERASTES_REQUIRES(a == 3000000000u)
    CERASTES_ENSURES(r, r == 6000000000u)
{
  return a << 1;
}
