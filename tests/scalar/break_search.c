// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 4: after a loop left by break, what holds on each exit path is known
// The loop is left either by its condition (i == n) or by the break (i == limit).
#include <cerastes.h>

int count_until(int n, int limit)
    CERASTES_REQUIRES(0 <= n && 0 <= limit)
    CERASTES_ENSURES(r, r <= n && r <= limit && (r == n || r == limit))
{
  int i = 0;
  CERASTES_INVARIANT(i <= n && i <= limit)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == limit) {
      break;
    }
    i += 1;
  }
  return i;
}
