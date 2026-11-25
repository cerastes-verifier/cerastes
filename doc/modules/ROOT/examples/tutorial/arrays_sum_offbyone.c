// expect: error permission @ L25
// expect: error invariant-entry @ L25
// expect: error invariant-preserved @ L25
// expect: error postcondition @ L31
// mode: c23, c++20
// The invariant claims the sum of one element more than the loop has read.
// Derived from arrays_sum.c.
// tag::source[]
#include <cerastes.h>

CERASTES_PURE long long prefix_sum(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, -2147483648LL * n <= r && r <= 2147483647LL * n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : prefix_sum(a, n - 1) + a[n - 1];
}

long long sum(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && n < 2147483647 && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == prefix_sum(a, n))
{
  long long s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s == prefix_sum(a, i + 1)) // <- error
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s = s + a[i];
    i = i + 1;
  }
  return s;
}
// end::source[]
