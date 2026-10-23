// expect: error conversion @ L22
// mode: c23, c++20
// twin: tests/scalar/assign_value_narrow.c
// Pins: 0020 item 1: a may be 200, which does not fit the signed char of c = a
#include <cerastes.h>

int widen(int a)
    CERASTES_REQUIRES(-32768 <= a && a <= 32767)
    CERASTES_ENSURES(r, r == 2 * a + 1)
{
  short s;
  int t = (s = a) + 1;
  return t + s;
}

int chain_narrow(int a)
    CERASTES_REQUIRES(-200 <= a && a <= 200)
    CERASTES_ENSURES(r, r == 2 * a)
{
  signed char c;
  int w;
  w = c = a;
  return w + c;
}
