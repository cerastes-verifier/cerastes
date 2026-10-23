// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 1: return x++ returns the old value, return ++x the new one
#include <cerastes.h>

int post(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x)
{
  return x++;
}

int pre(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1)
{
  return ++x;
}
