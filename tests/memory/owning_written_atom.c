// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 items 3 and 5 (audit S1): an owning parameter the function reassigns, a written atom on it, and a call in the loop that needs it live
#include <cerastes.h>
#include <stddef.h>

void touch(int *q)
    CERASTES_REQUIRES(cerastes_live(q))
    CERASTES_ENSURES(cerastes_live(q));

int use_owning(CERASTES_OWNING int *p, int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
    CERASTES_ENSURES(r, r == n)
{
  int *q = p;
  p = q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (p == NULL || cerastes_owns(*p)))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (p != NULL) {
      touch(p);
    }
    i += 1;
  }
  return i;
}
