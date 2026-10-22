// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 1: each declarator is its own local; the one with an initializer is defined
#include <cerastes.h>

int two(int a)
    CERASTES_REQUIRES(a <= 2147483644)
    CERASTES_ENSURES(r, r == a + 3)
{
  int x, y = 1;
  x = a;
  y += 2;
  return x + y;
}

int second_only(int a)
    CERASTES_ENSURES(r, (a > 0 && r == a) || (a <= 0 && r == 3))
{
  int x, y = 3;
  if (a > 0) {
    x = a;
    y = x;
  }
  return y;
}
