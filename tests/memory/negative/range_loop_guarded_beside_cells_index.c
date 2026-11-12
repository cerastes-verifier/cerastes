// expect: error out-of-bounds @ L20
// mode: c23, c++20
// twin: tests/memory/range_loop_guarded_beside_cells.c
// Pins: 0060 item 11 (0059 item 4): the loop stores a[i] into the three-element local array with i up to n: the bound row of the named array fails at the store, one error (the array's permission is known, so nothing else)
// Pre-registered from record 0060 item 11 (Part D), before the code
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
    a[i] = i;
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
