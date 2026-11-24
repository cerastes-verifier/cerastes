// expect: error check @ L20
// mode: c23, c++20
// twin: tests/memory/pure_check_parameters.c
// Pins: 0062 item 7 (audit round 2 R2-S3): over a reads_range parameter, sum_to(a, 1) is a[0], so == a[0] + 1 is the check error
// Pre-registered from record 0062 (Part B), before the code: check. the value-observing twin of the positive: one check error, at the check
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

int over_read(const int *a, int n)
    CERASTES_REQUIRES(1 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n))
{
  CERASTES_CHECK(sum_to(a, 1) == a[0] + 1LL);
  return n;
}
