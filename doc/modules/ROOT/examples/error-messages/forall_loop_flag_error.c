// expect: error uninitialized @ L17
// mode: c23, c++20
// The invariant claims a[0], which the loop has not written when it is first entered.
// tag::source[]
#include <cerastes.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

int fill(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[3];
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && cerastes_forall(nonneg, a, i + 1)) // <- error
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    a[i] = i;
    i = i + 1;
  }
  (void)a[0];
  return 0;
}
// end::source[]
