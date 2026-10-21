// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/scalar/noreturn_underscore_exit.c
// Pins: 0015 item 5: _Exit ends only the path of x > 5; x == 5 returns
// The ensures claims x <= 4, which fails for x == 5.
#include <stdlib.h>
#include <cerastes.h>

int pick(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x && x <= 4)
{
  if (x > 5) {
    _Exit(1);
  }
  return x;
}
