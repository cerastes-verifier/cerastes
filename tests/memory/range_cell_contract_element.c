// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 5 (audit round 2, N1): element reads p[0] in a cell contract (requires, ensures and old) at a call: the caller proves them from its cell, and the result and the new value follow
// Pre-registered from record 0060 item 5 (audit round 2), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

int g(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p[0] > 0 && p[0] < 1000)
    CERASTES_ENSURES(r, cerastes_owns(*p) && p[0] == cerastes_old(p[0]) + 1 && r == p[0]);

int use(void)
    CERASTES_ENSURES(r, r == 6)
{
  int x = 5;
  int y = g(&x);
  CERASTES_CHECK(x == 6);
  return y;
}
