// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 11 (S44 (g) fill_g, with the cell and array places): a loop over a range under the guard n == 0 ||, beside an address-taken local (px -> x) and a local array of the same field (a), all written in the body: three places in one loop, the range's entry and back-edge rows inside if (g), the cell and the array outside it; called with both guard values, n == 0 (the guard false, nothing given) and n > 0
// Pre-registered from record 0060 item 11 (Part D), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

int fill_beside(int *p, int n)
    CERASTES_REQUIRES(n == 0 || cerastes_owns_range(p, n))
    CERASTES_ENSURES(r, (n == 0 || cerastes_owns_range(p, n)) && r == 0)
{
  int x = 1;
  int *px = &x;
  int a[3] = {1, 2, 3};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) CERASTES_DECREASES(n - i) while (i < n) {
    p[i] = i;
    *px = i;
    a[0] = i;
    i++;
  }
  return a[1] - a[1];
}

int use_both(void)
    CERASTES_ENSURES(r, r == 0)
{
  int buf[4] = {1, 2, 3, 4};
  int r0 = fill_beside(buf, 0);
  int r1 = fill_beside(buf, 4);
  return r0 + r1;
}
