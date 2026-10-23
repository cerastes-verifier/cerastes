// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/scalar/compound_value.c
// Pins: 0020 item 1: x += y may overflow when x is not bounded by y
#include <cerastes.h>

int bump(int x, int y)
    CERASTES_REQUIRES(0 <= y)
    CERASTES_ENSURES(r, r == x)
{
  int t = (x += y);
  return t - y;
}

int shrink(int x, int d)
    CERASTES_REQUIRES(1 <= d)
    CERASTES_ENSURES(r, r == x / d)
{
  int t = (x /= d);
  return t;
}
