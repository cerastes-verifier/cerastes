// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 3 (an argument's own obligations, audit round 1 B1): the trailing argument x + 1 is evaluated once at the site and cannot overflow under -100 < x && x < 100: no row fails
// Pre-registered from record 0061 item 7 (Part B), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool ge_m(int i, const int *a, int n, int m)
{
  return !(0 <= i && i < n) || a[i] >= m;
}

void bounded(const int *a, int n, int x)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && -100 < x && x < 100 && cerastes_forall(ge_m, a, n, x + 1))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
  (void)x;
}
