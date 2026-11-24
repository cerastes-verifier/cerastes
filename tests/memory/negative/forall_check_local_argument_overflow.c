// expect: error overflow @ L22
// mode: c23, c++20
// twin: tests/memory/forall_check_local.c
// Pins: 0061 items 3 and 10 (audit round 1 B1): the trailing argument x + 1 may overflow (the parameter m of the pure function is unused): the argument's obligation is a row at the check, before the quantifier's rows
// Pre-registered from record 0061 item 10 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}
CERASTES_PURE bool ge_none(int k, const int *a, int n, int m)
{
  return !(0 <= k && k < n) || a[k] >= 0;
}

int check_it(int x)
    CERASTES_ENSURES(r, r == 0)
{
  int a[4] = {1, 2, 3, 4};
  CERASTES_CHECK(cerastes_forall(ge_none, a, 4, x + 1));
  (void)x;
  (void)a[0];
  return 0;
}
