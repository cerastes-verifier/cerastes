// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 item 7: an ensures application is read in the post-state: the body writes a[0] and then asks the trusted total for the sum, so the result is the sum of the written array
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

long long set_and_sum(int *a, int n)
    CERASTES_REQUIRES(1 <= n && cerastes_owns_range(a, n))
    CERASTES_ENSURES(r, cerastes_owns_range(a, n) && r == sum_to(a, n))
{
  a[0] = 5;
  return total(a, n);
}
