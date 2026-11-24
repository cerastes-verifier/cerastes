// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 9 and question 262 (audit round 3): a check is split per top-level conjunct as a loop invariant is: `0 <= n && cerastes_forall(...)` checks the plain conjunct, then the quantifier (marked row, flag row, fact) under it
// Pre-registered from record 0061 item 9 (audit round 3, question 262), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int k, const int *a, int n)
{
  return !(0 <= k && k < n) || a[k] >= 0;
}

int check_it(int n)
    CERASTES_REQUIRES(0 <= n && n <= 4)
    CERASTES_ENSURES(r, r == 0)
{
  int a[4] = {1, 2, 3, 4};
  CERASTES_CHECK(0 <= n && cerastes_forall(nonneg, a, n));
  (void)a[0];
  (void)n;
  return 0;
}
