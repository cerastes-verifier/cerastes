// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 items 5 and 9: give(&p[1]) takes one slot of the caller's range and hands it back with *q == 5: the return rows hold, and the ensures reads p[1]
// Pre-registered from record 0060 items 5 and 9, before the code: verifies
#include <cerastes.h>

void give(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*q) && *q == 5);

void f(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n >= 2)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[1] == 5)
{
  (void)n;
  give(&p[1]);
}
