// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 11 (0059 item 8's callee in a body): give(&p[i]) takes one element of the loop's range and gives it back each iteration: the back-edge marked rows hold
// Pre-registered from record 0060 item 11 (Part D), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

void give(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*q) && *q == 5);

void fill5(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) CERASTES_DECREASES(n - i) while (i < n) {
    give(&p[i]);
    i++;
  }
}
