// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 item 7 (S46 (d) copy): a loop that also fills another array (b[i] = a[i], an owns place beside the reads place): the write does not touch what sum_to reads, so the invariant is kept
// Pre-registered from record 0062 (Part D), before the code: verifies. the pair's twin is pure_sum_write_prefix.c (a write inside the summed range)
#include <cerastes.h>

CERASTES_PURE long long sum_to(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : sum_to(a, n - 1) + a[n - 1];
}

long long sum_copy(const int *a, int *b, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n) && cerastes_owns_range(b, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && cerastes_owns_range(b, n) && r == sum_to(a, n))
{
  long long s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s == sum_to(a, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    b[i] = a[i];
    s = s + a[i];
    i = i + 1;
  }
  return s;
}
