// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 item 8 (S46 (d) recorded): a value over known elements needs one written step per element: with check(sum_to(a, 1) == a[0]) before it, check(sum_to(a, 2) == a[0] + a[1]) holds (the Memory page states the limit; lemmas are M4.5f3's)
// Pre-registered from record 0062 (Part D), before the code: verifies
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

int use(void)
{
  int a[4] = {1, 2, 3, 4};
  CERASTES_CHECK(sum_to(a, 1) == a[0]);
  CERASTES_CHECK(sum_to(a, 2) == a[0] + a[1]);
  return a[0];
}
