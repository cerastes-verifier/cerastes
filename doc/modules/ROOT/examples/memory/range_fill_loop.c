// expect: verified
// expect: native-ok
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>

void fill(int *p, int n, int v)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(n == 0 || p[0] == v)
{
  int i;
  CERASTES_INVARIANT(0 <= i && i <= n && (i == 0 || p[0] == v)) CERASTES_DECREASES(n - i) for (i = 0; i < n; i++) {
    p[i] = v;
  }
}

int caller(void)
    CERASTES_ENSURES(r, r == 5)
{
  int a[3] = {0};
  fill(a, 3, 5);
  return a[0];
}
// end::source[]
