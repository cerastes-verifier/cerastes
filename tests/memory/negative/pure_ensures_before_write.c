// expect: error postcondition @ L26
// mode: c23, c++20
// twin: tests/memory/pure_ensures_after_write.c
// Pins: 0062 item 7: the sum is computed before the write: the postcondition r == sum_to(a, n) is read in the state after it, so the postcondition error, at the return
// Pre-registered from record 0062 (Part C), before the code: postcondition
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
  long long s = total(a, n);
  a[0] = 5;
  return s;
}
