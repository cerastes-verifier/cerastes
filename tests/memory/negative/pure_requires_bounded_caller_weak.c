// expect: error precondition @ L28
// mode: c23, c++20
// twin: tests/memory/pure_requires_bounded.c
// Pins: 0062 item 7: a caller that cannot prove the callee's requires sum_to(a, n) < 100: the precondition error at the call (no native error)
// Pre-registered from record 0062 (Part C), before the code: precondition
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
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == n)
{
  return bounded(a, n);
}
