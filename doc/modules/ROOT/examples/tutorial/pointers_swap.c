// expect: verified
// expect: native-ok
// mode: c23, c++20
// swap owns both pointees; the caller sees the exchanged values.
// tag::source[]
#include <cerastes.h>

void swap(int *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(*q)
                     && *p == cerastes_old(*q) && *q == cerastes_old(*p))
{
  int t = *p;
  *p = *q;
  *q = t;
}

int use_swap(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a = 1;
  int b = 2;
  swap(&a, &b);
  return b;
}
// end::source[]
