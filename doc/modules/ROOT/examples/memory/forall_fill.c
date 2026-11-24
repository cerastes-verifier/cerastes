// expect: verified
// expect: native-ok
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>

CERASTES_PURE bool filled(int k, const int *a, int m)
{
  return !(0 <= k && k < m) || a[k] == k;
}

int fill_and_read(void)
    CERASTES_ENSURES(r, r == 3)
{
  int a[4];
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4 && cerastes_forall(filled, a, i))
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    a[i] = i;
    i = i + 1;
  }
  return a[0] + a[3];
}
// end::source[]
