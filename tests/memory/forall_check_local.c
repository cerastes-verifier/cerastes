// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 10 (S45 (f) check_local): a forall in a check over a local array: the bound, the marked row (permission), the flag row and then the fact, in that order, all hold
// Pre-registered from record 0061 item 10 (Part C), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

int check_it(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[4] = {1, 2, 3, 4};
  CERASTES_CHECK(cerastes_forall(nonneg, a, 4));
  (void)a[0];
  return 0;
}
