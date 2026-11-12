// expect: verified
// expect: native-ok
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>

void set(int *p, int n, int i, int v)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[i] == v)
{
  (void)n;
  p[i] = v;
}

int caller(void)
    CERASTES_ENSURES(r, r == 7)
{
  int a[4] = {1, 2, 3, 4};
  set(&a[1], 2, 0, 9);  // a[1..3)
  set(a, 4, 2, 7);      // all of a
  return a[2];
}
// end::source[]
