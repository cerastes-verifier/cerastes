// expect: error check @ L21
// mode: c23, c++20
// twin: tests/scalar/break_search.c
// Pins: 0014 item 4: after the loop only the facts of some exit path hold
// i == n holds on the condition exit only; the break exit has i == limit < n.
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
  CERASTES_CHECK(i == n);
  return i;
}
