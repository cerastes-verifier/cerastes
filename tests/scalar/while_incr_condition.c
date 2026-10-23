// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 3: the effect of a loop condition runs at every evaluation; the invariant holds before each one
#include <cerastes.h>

int count_up(int n)
    CERASTES_REQUIRES(0 <= n && n <= 715827882)
    CERASTES_ENSURES(r, r == 3 * n + 1)
{
  int i = 0;
  int total = 0;
  CERASTES_INVARIANT(i <= n && total == 2 * i)
  CERASTES_DECREASES(n - i)
  while (i++ < n) {
    total += 2;
  }
  return total + i;
}
