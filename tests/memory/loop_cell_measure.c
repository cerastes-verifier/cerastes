// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 items 3 and 6: a cell in a loop measure and condition (DECREASES(k) with a write through a pointer)
#include <cerastes.h>

int countdown(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 0)
{
  int k = n;
  int *p = &k;
  CERASTES_INVARIANT(0 <= k && k <= 100)
  CERASTES_DECREASES(k)
  while (k > 0) {
    *p -= 1;
  }
  return k;
}
