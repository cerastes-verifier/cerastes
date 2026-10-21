// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 6: the expression of an assert is code: its own obligations (division) are checked first
// y > 0 makes the division defined; y <= x makes the assertion true.
#include <assert.h>
#include <cerastes.h>

int ratio(int x, int y)
    CERASTES_REQUIRES(0 < y && y <= x)
    CERASTES_ENSURES(r, r > 0)
{
  assert(x / y > 0);
  return x / y;
}
