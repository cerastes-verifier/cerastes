// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 5: _Exit never returns (C23 7.24.4.5)
// The path of x > 5 ends in _Exit, so the postcondition may assume x <= 5.
#include <stdlib.h>
#include <cerastes.h>

int pick(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x && x <= 5)
{
  if (x > 5) {
    _Exit(1);
  }
  return x;
}
