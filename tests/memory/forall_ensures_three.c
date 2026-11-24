// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 items 6 and 7 (S45 (a)): a straight-line body proves a quantified ensures over three elements, and its caller uses it (a[2] < 10)
// Pre-registered from record 0061 item 7 (Part B), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool small(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] < 10;
}

void fill3(int *a)
    CERASTES_REQUIRES(cerastes_owns_range(a, 3))
    CERASTES_ENSURES(cerastes_owns_range(a, 3) && cerastes_forall(small, a, 3))
{
  a[0] = 1;
  a[1] = 2;
  a[2] = 3;
}

int caller(void)
    CERASTES_ENSURES(r, r < 10)
{
  int a[3] = {0, 0, 0};
  fill3(a);
  return a[2];
}
