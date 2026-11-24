// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 7: a trusted (bodiless) declaration with a quantified ensures: the caller learns every element is non-negative from the callee's postcondition
// Pre-registered from record 0061 item 7 (Part B), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

void fill_nonneg(int *a, int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n))
    CERASTES_ENSURES(cerastes_owns_range(a, n) && cerastes_forall(nonneg, a, n));

int use(void)
    CERASTES_ENSURES(r, r >= 0)
{
  int a[3] = {0, 0, 0};
  fill_nonneg(a, 3);
  return a[1];
}
