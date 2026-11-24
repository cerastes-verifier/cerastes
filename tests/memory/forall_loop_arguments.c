// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 3 (an argument's own obligations in an invariant, audit round 1 B1): the trailing argument x + 1 is evaluated once at the site and cannot overflow under -100 < x && x < 100, and a local with an initializer is a valid argument (run_local)
// Pre-registered from record 0061 item 3 (Part C), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool ge_slack(int k, const int *a, int n, int m)
{
  return !(0 <= k && k < n) || a[k] > m - 1000;
}

int run(int x)
    CERASTES_REQUIRES(-100 < x && x < 100)
    CERASTES_ENSURES(r, r == 0)
{
  int a[3] = {1, 2, 3};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && cerastes_forall(ge_slack, a, i, x + 1))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i = i + 1;
  }
  (void)a[0];
  (void)x;
  return 0;
}

int run_local(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[3] = {1, 2, 3};
  int m = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && cerastes_forall(ge_slack, a, i, m))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i = i + 1;
  }
  (void)a[0];
  return m;
}
