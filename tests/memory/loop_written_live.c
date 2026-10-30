// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 5: live(p) written in an invariant for a reassigned parameter: the liveness comes back after the loop for the ensures
#include <cerastes.h>
#include <stddef.h>

int live_loop(int *p, int n)
    CERASTES_REQUIRES(cerastes_live(p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_live(p) && r == n)
{
  int *q = p;
  p = q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_live(p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return i;
}
