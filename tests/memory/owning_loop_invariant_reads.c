// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3 Loops, shape L7: a user invariant that reads the cell of an owning loop variable (v == NULL || *v >= 0): the owner's clauses come first in the invariant list, so the read is framed
// Pre-registered from record 0052 item 3 Loops (il-analyst's probes build/m4.5d2b/loop/p): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v == NULL) {
    return 1;
  }
  *v = 0;
  int k = n;
  CERASTES_INVARIANT(0 <= k && k <= n)
  CERASTES_INVARIANT(v == NULL || *v >= 0)
  CERASTES_DECREASES(k)
  while (k > 0) {
    k -= 1;
    *v = k;
  }
  free(v);
  return 0;
}
