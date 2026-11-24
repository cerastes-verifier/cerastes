// expect: error postcondition @ L31
// mode: c23, c++20
// The ensures says every element is below the result; the maximum itself is not.
// Derived from arrays_max.c.
// tag::source[]
#include <cerastes.h>

CERASTES_PURE bool at_most(int k, const int *a, int n, int m)
{
  return !(0 <= k && k < n) || a[k] <= m;
}
CERASTES_PURE bool below(int k, const int *a, int n, int m)
{
  return !(0 <= k && k < n) || a[k] < m;
}

int max_of(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && cerastes_forall(below, a, n, r))
{
  int m = a[0];
  int i = 1;
  CERASTES_INVARIANT(1 <= i && i <= n && cerastes_forall(at_most, a, i, m))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (a[i] > m) {
      m = a[i];
    }
    i = i + 1;
  }
  return m;
}
// end::source[]
