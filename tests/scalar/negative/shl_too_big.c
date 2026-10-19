// expect: error shift @ L11
// mode: c23, c++20
// twin: tests/scalar/shl_const.c
// Pins: 0010 item 8: a << 4 does not fit int for a = 200000000
#include <cerastes.h>

int times16(int a)
    CERASTES_REQUIRES(a >= 0 && a <= 200000000)
    CERASTES_ENSURES(r, r >= a)
{
  return a << 4;
}
