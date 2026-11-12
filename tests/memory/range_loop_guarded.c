// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 11 (S44 (g) fill_g): a loop over a range under the guard n == 0 ||: the place's conjuncts and its marked entry and back-edge rows are under the guard (if (g) { })
// Pre-registered from record 0060 item 11 (Part D), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

void fill_g(int *p, int n)
    CERASTES_REQUIRES(n == 0 || cerastes_owns_range(p, n))
    CERASTES_ENSURES(n == 0 || cerastes_owns_range(p, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) CERASTES_DECREASES(n - i) while (i < n) {
    p[i] = i;
    i++;
  }
}
