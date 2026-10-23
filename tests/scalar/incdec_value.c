// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 1: x++ has the old value and ++x the new one; each is also the store
#include <cerastes.h>

int both(int x)
    CERASTES_REQUIRES(-1073741824 <= x && x <= 1073741822)
    CERASTES_ENSURES(r, r == 2 * x + 2)
{
  int a = x++;
  int b = ++x;
  return a + b;
}

int next(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x)
{
  int a = x++;
  return a;
}
