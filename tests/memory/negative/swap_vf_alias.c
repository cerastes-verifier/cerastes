// expect: error permission @ L28
// mode: c23, c++20
// twin: tests/memory/verifast/swap.c
// Pins: 0037 item 8 and 0040's port of VeriFast's swap: swap(&x, &x) passes the one cell as two full permissions: the pairwise separation fails at the call
#include <cerastes.h>

void swap(int *a, int *b)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*a) && cerastes_owns(*b) && *a == cerastes_old(*b) && *b == cerastes_old(*a))
{
  int t = *a;
  *a = *b;
  *b = t;
}

void swap_twice(int *a, int *b)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*a) && cerastes_owns(*b) && *a == cerastes_old(*a) && *b == cerastes_old(*b))
{
  swap(a, b);
  swap(a, b);
}

int use_swap(void)
{
  int x = 1;
  int y = 2;
  swap(&x, &x);
  swap_twice(&x, &y);
  return x * 10 + y;
}
