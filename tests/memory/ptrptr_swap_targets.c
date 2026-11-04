// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 7 (question 108): a swap of two int ** targets: each ensures atom on **a and **b returns the amount of the object it now designates
#include <cerastes.h>
#include <stddef.h>

void swap_targets(int **a, int **b)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b) && cerastes_owns(**a) && cerastes_owns(**b))
    CERASTES_ENSURES(cerastes_owns(*a) && cerastes_owns(*b) && *a == cerastes_old(*b) && *b == cerastes_old(*a) && cerastes_owns(**a) && cerastes_owns(**b) && **a == cerastes_old(**b) && **b == cerastes_old(**a))
{
  int *t = *a;
  *a = *b;
  *b = t;
}

int use_swap_targets(void)
    CERASTES_ENSURES(r, r == 12)
{
  int x = 1;
  int y = 2;
  int *p = &x;
  int *q = &y;
  swap_targets(&p, &q);
  CERASTES_CHECK(*p == 2 && *q == 1);
  return x * 10 + y;
}
