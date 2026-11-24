// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 items 4 and 9: a ref_to_uninit parameter's element read under the binder in an invariant is accepted (flag row): a filler whose invariant says the elements below i are zero verifies
// Pre-registered from record 0061 item 9 (Part C), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool zero_at(int k, const int *p, int m)
{
  return !(0 <= k && k < m) || p[k] == 0;
}

void fill(int *p CERASTES_REF_TO_UNINIT, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_forall(zero_at, p, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p[i] = 0;
    i++;
  }
}
