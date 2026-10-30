// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 5 and question 68 (was the pinned false alarm of 0038 Consequences): a return inside a loop whose ensures returns the place of a parameter the function assigns; the written atom owns(*p) holds it (p is not assigned in the loop)
#include <cerastes.h>

int moved(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  int *q = p;
  p = q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == 5) {
      return i;
    }
    i += 1;
  }
  return n;
}
