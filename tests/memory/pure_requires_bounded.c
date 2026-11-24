// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 items 4 and 7 (S46 (e)): a callee whose requires applies a pure function (sum_to(a, n) < 100) and a caller that gives the same clause: the precondition row at the call holds
// Pre-registered from record 0062 (Part C), before the code: verifies
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

int bounded(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n) && sum_to(a, n) < 100)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == n)
{
  (void)a;
  return n;
}

int caller(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n) && sum_to(a, n) < 100)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == n)
{
  return bounded(a, n);
}
