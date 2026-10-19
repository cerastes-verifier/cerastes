// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 8: a loop condition is checked each time it is evaluated (before the loop and after each iteration)
// n >= 1 keeps the division in the condition safe.
#include <cerastes.h>

int steps(int n)
    CERASTES_REQUIRES(n >= 1 && n <= 100)
    CERASTES_ENSURES(r, r == 1000 / n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 1000 / n)
  CERASTES_DECREASES(1000 - i)
  while (i < 1000 / n) {
    i += 1;
  }
  return i;
}
