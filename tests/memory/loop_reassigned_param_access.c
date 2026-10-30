// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 5 and question 68 (was the pinned false alarm of 0038 Consequences): a parameter the function assigns has no automatic place; the written atom owns(*p) in the invariant carries it through the loop
#include <cerastes.h>

int moved(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
{
  int *q = p;
  p = q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (*p > 0) {
      i += 1;
    } else {
      i += 1;
    }
  }
  return i;
}
