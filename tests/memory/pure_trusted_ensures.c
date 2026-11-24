// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 items 7 and 8 (S46 (b), (e)): a bodiless (trusted) function whose ensures names the sum: the caller learns x == sum_to(a, 4) from its postcondition, read in the caller's state
// Pre-registered from record 0062 (Part C), before the code: verifies
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

long long total(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == sum_to(a, n));

int use(void)
{
  int a[4] = {1, 2, 3, 4};
  long long x = total(a, 4);
  CERASTES_CHECK(x == sum_to(a, 4));
  (void)x;
  return a[0];
}
