// expect: error check @ L13
// twin: tests/scalar/assert_cassert.cpp
// Pins: 0015 items 6 and 7: a failing assert from <cassert> is a check error at the assert
// y <= 100 fails for x == 100.
#include <cassert>
#include <cerastes.h>

int next(int x)
    CERASTES_REQUIRES(0 <= x && x <= 100)
    CERASTES_ENSURES(r, r == x + 1)
{
  int y = x + 1;
  assert(y > 0 && y <= 100);
  return y;
}
