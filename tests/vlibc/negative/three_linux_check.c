// expect: error check @ L20
// mode: c23, c++20; platform: linux
// twin: tests/vlibc/three_linux.c
// Pins: 0046 item 4 and 0031 item 10: each of the three trusted contracts is used on glibc
// The exit call is removed and _Exit's guard is x == 6 (the positive's x > 5 would end the path of x == 7 and x == 8 as well): x == 7 and x == 8 reach the check.
#include <stdlib.h>
#include <cerastes.h>

int pick(int x)
    CERASTES_REQUIRES(0 <= x && x <= 9)
{
  if (x > 8) {
    abort();
  }
  if (x > 6) {
  }
  if (x == 6) {
    _Exit(2);
  }
  CERASTES_CHECK(x <= 5);
  return x;
}
