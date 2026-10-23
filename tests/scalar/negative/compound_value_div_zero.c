// expect: error division-by-zero @ L19
// mode: c23, c++20
// twin: tests/scalar/compound_value.c
// Pins: 0020 item 1: x /= d divides by zero when d may be 0
#include <cerastes.h>

int bump(int x, int y)
    CERASTES_REQUIRES(0 <= y && x <= 2147483647 - y)
    CERASTES_ENSURES(r, r == x)
{
  int t = (x += y);
  return t - y;
}

int shrink(int x, int d)
    CERASTES_REQUIRES(0 <= d)
    CERASTES_ENSURES(r, d == 0 || r == x / d)
{
  int t = (x /= d);
  return t;
}
