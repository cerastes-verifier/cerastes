// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0046 items 2 and 4: an explicit `#include <vlibc/stdlib.h>` reaches vlibc's pinned header, and exit's trusted `ensures(false)` ends the path
// With x < 0 the call of exit never returns, so the postcondition only has to hold for x >= 0.
#include <vlibc/stdlib.h>
#include <cerastes.h>

int nonneg(int x)
    CERASTES_REQUIRES(-10 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x && r >= 0)
{
  if (x < 0) {
    exit(1);
  }
  return x;
}
