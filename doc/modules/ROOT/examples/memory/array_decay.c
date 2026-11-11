// expect: verified
// expect: native-ok
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>

void set_first(int *p, int v)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == v)
{
  p[0] = v;
}

int second_after(void)
    CERASTES_ENSURES(r, r == 9)
{
  int a[3] = {1, 2, 3};
  set_first(a, 7);
  int *q = &a[1];
  q[1] = 9;
  return a[2] + a[0] - 7;
}
// end::source[]
