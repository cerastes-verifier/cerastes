// expect: error shift @ L12
// mode: c23, c++20
// twin: tests/scalar/shift_amount.c
// Pins: 0010 item 8: a shift amount of 32 or more on a 32-bit int
// The value 0 always fits, so only the amount is wrong.
#include <cerastes.h>

int pow2(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 40)
    CERASTES_ENSURES(r, r >= 0)
{
  return 0 << n;
}
