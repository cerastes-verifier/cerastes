// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0046 items 2 and 4: a bare `#include <stdlib.h>` reaches the shim, so abort has vlibc's contract
// The path of x > 5 ends in abort, so the postcondition may assume x <= 5.
#include <stdlib.h>
#include <cerastes.h>

int small(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x && x <= 5)
{
  if (x > 5) {
    abort();
  }
  return x;
}
