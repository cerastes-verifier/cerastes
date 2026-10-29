// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/memory/swap_owns.c
// Pins: 0037 item 10: a last store undoes the swap of *a, so *a == old(*b) fails at the closing brace
#include <cerastes.h>
#include <stddef.h>

void swap(int *a, int *b)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*a) && cerastes_owns(*b) && *a == cerastes_old(*b) && *b == cerastes_old(*a))
{
  int t = *a;
  *a = *b;
  *b = t;
  *a = t;
}

int use_swap(void)
    CERASTES_ENSURES(r, r == 21)
{
  int x = 1;
  int y = 2;
  swap(&x, &y);
  return x * 10 + y;
}
