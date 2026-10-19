// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 8: a for loop is init; while (cond) { body; incr }; the counter is declared before it
// (The invariant cannot name a for-init variable: 0006 item 2.)
#include <cerastes.h>

int twice_count(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 100)
    CERASTES_ENSURES(r, r == 2 * n)
{
  int i = 0;
  int total = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && total == 2 * i)
  CERASTES_DECREASES(n - i)
  for (i = 0; i < n; i++) {
    total += 2;
  }
  return total;
}
