// expect: error permission @ L21
// mode: c23, c++20
// twin: tests/memory/array_swap_elements.c
// Pins: 0059 item 4 and 0037 item 8: swap(&a[0], &a[0]) passes one element as two full permissions: the pairwise separation fails at the call
// Pre-registered from record 0059 item 4
#include <cerastes.h>

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
  int a[3] = {1, 2, 3};
  swap(&a[0], &a[0]);
  return a[0] * 10 + a[1];
}
