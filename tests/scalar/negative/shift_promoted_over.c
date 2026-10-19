// expect: error shift @ L11
// mode: c23, c++20
// twin: tests/scalar/shift_promoted.c
// Pins: 0010 items 6 and 8: 128 << 24 does not fit int
#include <cerastes.h>

int top_byte(unsigned char c)
    CERASTES_REQUIRES(c <= 128)
    CERASTES_ENSURES(r, r >= 0)
{
  return c << 24;
}
