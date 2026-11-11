// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 4 and 0037 items 8 to 10: two elements of one block as two owns atoms, separate by their offsets
// Pre-registered from record 0059 item 4: verifies
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
  swap(&a[0], &a[1]);
  return a[0] * 10 + a[1];
}
