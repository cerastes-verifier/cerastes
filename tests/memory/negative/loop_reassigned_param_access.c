// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/loop_owns_fill.c
// Pins: 0038 Consequences (audit N2, a documented false alarm until b1c3's written atoms): a parameter the function assigns has no automatic place, so an access to it inside a loop is reported
#include <cerastes.h>

int moved(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
{
  int *q = p;
  p = q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
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
