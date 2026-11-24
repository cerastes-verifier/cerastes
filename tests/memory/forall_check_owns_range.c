// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 10 (S45 (f) check_owns): a forall in a check over an owns_range parameter: the null and liveness rows, the bound, the marked rows, the flag row and the fact (which rests on the quantified requires)
// Pre-registered from record 0061 item 10 (Part C), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

void keep_nonneg(int *a, int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && cerastes_forall(nonneg, a, n))
    CERASTES_ENSURES(cerastes_owns_range(a, n))
{
  CERASTES_CHECK(cerastes_forall(nonneg, a, n));
  (void)a;
  (void)n;
}
