// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 5: a written decreases on do ... while (0) is allowed and checked like any other
#include <cerastes.h>

int add_one(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1)
{
  int y = x;
  CERASTES_DECREASES(1)
  do {
    y += 1;
  } while (0);
  return y;
}
