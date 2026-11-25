// expect: error invariant-preserved @ L22
// mode: c23, c++20
// twin: tests/memory/pure_sum.c
// Pins: 0062 item 7 (S46 (d)): the body adds a[i] + 1: the invariant is not preserved (invariant-preserved at the clause)
// Pre-registered from record 0062 (Part D), before the code: invariant-preserved
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

long long sum(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == sum_to(a, n))
{
  long long s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s == sum_to(a, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s = s + a[i] + 1;
    i = i + 1;
  }
  return s;
}
