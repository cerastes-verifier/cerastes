// expect: verified
// expect: native-ok
// mode: c23, c++20
// A pure function that cerastes verify reads is a function with a meaning of its own: a check applies it.
// tag::tri[]
#include <cerastes.h>

CERASTES_PURE int tri(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000)
    CERASTES_ENSURES(r, 0 <= r && r <= 1000 * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : tri(n - 1) + n;
}
// end::tri[]

// tag::sum[]
CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}
// end::sum[]

// tag::check[]
int use(void)
{
  int a[3] = {1, 2, 3};
  CERASTES_CHECK(sum_to(a, 1) == 1);
  CERASTES_CHECK(sum_to(a, 2) == 3);
  CERASTES_CHECK(tri(1) == 1);
  CERASTES_CHECK(tri(2) == 3);
  CERASTES_CHECK(tri(3) == 6);
  return a[0];
}
// end::check[]
