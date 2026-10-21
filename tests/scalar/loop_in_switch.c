// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 6: a loop inside a case; its break leaves the loop, the case's break leaves the switch
// The statement after the loop in case 2 (total += 1) runs on both exits of the loop.
#include <cerastes.h>

int run(int mode, int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, (mode == 1 && r == n) || (mode == 2 && ((n <= 5 && r == 2 * n + 1) || (n > 5 && r == 11))) || (mode != 1 && mode != 2 && r == 9))
{
  int total = 0;
  int i = 0;
  switch (mode) {
  case 1: {
    CERASTES_INVARIANT(i <= n && total == i)
    CERASTES_DECREASES(n - i)
    while (i < n) {
      total += 1;
      i += 1;
    }
    break;
  }
  case 2: {
    CERASTES_INVARIANT(i <= n && i <= 5 && total == 2 * i)
    CERASTES_DECREASES(n - i)
    while (i < n) {
      if (i == 5) {
        break;
      }
      total += 2;
      i += 1;
    }
    total += 1;
    break;
  }
  default:
    total = 9;
    break;
  }
  return total;
}
