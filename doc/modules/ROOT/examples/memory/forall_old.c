// expect: verified
// expect: native-ok
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>

CERASTES_PURE bool kept(int i, const int *a, int n, int j)
{
  return !(0 <= i && i < n && i != j) || a[i] == cerastes_old(a[i]);
}

void set_one(int *a, int n, int j, int v)
    CERASTES_REQUIRES(cerastes_owns_range(a, n) && 0 <= j && j < n)
    CERASTES_ENSURES(cerastes_owns_range(a, n) && a[j] == v && cerastes_forall(kept, a, n, j))
{
  (void)n;
  a[j] = v;
}

int caller(void)
    CERASTES_ENSURES(r, r == 5)
{
  int a[4] = {1, 2, 3, 4};
  set_one(a, 4, 2, 9);
  return a[0] + a[3];
}
// end::source[]
