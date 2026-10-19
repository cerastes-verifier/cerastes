// expect: error shift @ L11
// mode: c23, c++20
// twin: tests/scalar/shift_amount.c
// Pins: 0010 item 8: a negative shift amount
#include <cerastes.h>

int pow2(int n)
    CERASTES_REQUIRES(n >= -1 && n <= 30)
    CERASTES_ENSURES(r, r >= 1)
{
  return 1 << n;
}
