// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 8: a while loop with a user invariant; the automatic range invariants hold
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return i;
}
