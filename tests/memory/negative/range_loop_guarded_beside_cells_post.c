// expect: error postcondition @ L23
// mode: c23, c++20
// twin: tests/memory/range_loop_guarded_beside_cells.c
// Pins: 0060 item 11: the ensures claims r == 1 where the function returns 0: the postcondition fails at the return, one error (the loop and its places are fine)
// Pre-registered from record 0060 item 11 (Part D), before the code
#include <cerastes.h>
#include <stddef.h>

int fill_beside(int *p, int n)
    CERASTES_REQUIRES(n == 0 || cerastes_owns_range(p, n))
    CERASTES_ENSURES(r, (n == 0 || cerastes_owns_range(p, n)) && r == 1)
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

