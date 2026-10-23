// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 1: x op= e used as a value is the new value of x, with the operation checked like x = x op e
#include <cerastes.h>

int bump(int x, int y)
    CERASTES_REQUIRES(0 <= y && x <= 2147483647 - y)
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
