// expect: error conversion @ L12
// mode: c23, c++20
// twin: tests/scalar/assign_value_narrow.c
// Pins: 0020 item 1: a may be 40000, which does not fit the short the assignment converts to
#include <cerastes.h>

int widen(int a)
    CERASTES_REQUIRES(-40000 <= a && a <= 40000)
    CERASTES_ENSURES(r, r == 2 * a + 1)
{
  short s;
  int t = (s = a) + 1;
  return t + s;
}

int chain_narrow(int a)
    CERASTES_REQUIRES(-128 <= a && a <= 127)
    CERASTES_ENSURES(r, r == 2 * a)
{
  signed char c;
  int w;
  w = c = a;
  return w + c;
}
