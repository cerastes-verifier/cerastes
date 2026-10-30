// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 5 and question 68 (was 0038 items 3 and 6, audit round 2, S1): p is reassigned, so *q is no automatic place; the written atom owns(*q) frames the invariant's read of *q
#include <cerastes.h>
#include <stddef.h>

int unowned_read(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p == 0 && 0 <= n && n <= 10)
{
  int *q = p;
  p = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*q) && (i == 0 || *q == 0))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i++;
  }
  return q == NULL;
}
