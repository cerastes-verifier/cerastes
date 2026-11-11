// expect: verified
// expect: native-ok
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>

void swap(int *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(*p == cerastes_old(*q) && *q == cerastes_old(*p))
{
  int t = *p;
  *p = *q;
  *q = t;
}

int first_after_swap(void)
    CERASTES_ENSURES(r, r == 2)
{
  int pair[2] = {1, 2};
  swap(&pair[0], &pair[1]);
  return pair[0];
}
// end::source[]
