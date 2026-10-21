// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 items 4 and 5: break in a do ... while (c): the break exit skips the condition, the condition exit is the other
// Left by the break (i == limit <= n) or by the condition (i == n < limit).
#include <cerastes.h>

int count_until(int n, int limit)
    CERASTES_REQUIRES(1 <= n && 1 <= limit)
    CERASTES_ENSURES(r, (r == limit && limit <= n) || (r == n && n < limit))
{
  int i = 0;
  CERASTES_INVARIANT(i < n && i < limit)
  CERASTES_DECREASES(n - i)
  do {
    i += 1;
    if (i == limit) {
      break;
    }
  } while (i < n);
  return i;
}
