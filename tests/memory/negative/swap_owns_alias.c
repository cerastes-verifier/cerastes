// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/swap_owns.c
// Pins: 0037 item 8: swap(&x, &x) passes the one cell as two full permissions: the pairwise separation fails at the call
#include <cerastes.h>
#include <stddef.h>

void swap(int *a, int *b)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*a) && cerastes_owns(*b) && *a == cerastes_old(*b) && *b == cerastes_old(*a))
{
  int t = *a;
  *a = *b;
  *b = t;
}

int use_swap(void)
    CERASTES_ENSURES(r, r == 21)
{
  int x = 1;
  int y = 2;
  swap(&x, &x);
  return x * 10 + y;
}
