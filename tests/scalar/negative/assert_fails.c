// expect: error check @ L14
// mode: c23, c++20
// twin: tests/scalar/assert_holds.c
// Pins: 0015 items 6 and 7: a failing assert is a check error, reported at the assert
// y <= 100 fails for x == 100.
#include <assert.h>
#include <cerastes.h>

int next(int x)
    CERASTES_REQUIRES(0 <= x && x <= 100)
    CERASTES_ENSURES(r, r == x + 1)
{
  int y = x + 1;
  assert(y > 0 && y <= 100);
  return y;
}
