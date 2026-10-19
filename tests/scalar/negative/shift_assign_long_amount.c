// expect: error shift @ L12
// mode: c23, c++20
// twin: tests/scalar/shift_assign_long.c
// Pins: 0010 item 8: the amount of a <<= n must be below the width of int, not of long
// The value 0 always fits, so only the amount (up to 40 here) is wrong.
#include <cerastes.h>

int shl_assign(int a, long n)
    CERASTES_REQUIRES(a == 0 && n >= 0 && n <= 40)
    CERASTES_ENSURES(r, r == 0)
{
  a <<= n;
  return a;
}
