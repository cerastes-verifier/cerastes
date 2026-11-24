// expect: verified
// expect: native-ok
// mode: c23, c++20
// Every element is at most the result (the second property, that the result is one of the elements, needs an exists quantifier, which cerastes does not have yet).
// tag::source[]
#include <cerastes.h>

CERASTES_PURE bool at_most(int k, const int *a, int n, int m)
{
  return !(0 <= k && k < n) || a[k] <= m;
}

int max_of(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && cerastes_forall(at_most, a, n, r))
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
