// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 5: p[0] in a function's decreases, framed by the range atom (as decreases(p->x) is by owns(*p), 0044 item 10)
// Pre-registered from record 0060 item 5, before the code: verifies
#include <cerastes.h>

int f(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0 && p[0] >= 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == n)
    CERASTES_DECREASES(p[0])
{
  (void)p;
  return n;
}
