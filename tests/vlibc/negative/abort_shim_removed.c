// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/vlibc/abort_shim.c
// Pins: 0046 item 8: abort's trusted `ensures(false)` is what the positive relies on
// The abort call is removed: x > 5 reaches the return and x <= 5 fails.
#include <stdlib.h>
#include <cerastes.h>

int small(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x && x <= 5)
{
  if (x > 5) {
  }
  return x;
}
