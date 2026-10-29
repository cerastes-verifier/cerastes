// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0037 items 6, 8, 9, 10: two owns atoms, the amounts per atom, the separation of the two owned places at the call and at entry
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
  swap(&x, &y);
  return x * 10 + y;
}
