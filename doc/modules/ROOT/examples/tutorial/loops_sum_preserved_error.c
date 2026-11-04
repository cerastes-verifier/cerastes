// expect: error invariant-preserved @ L14
// mode: c23, c++20
// The body adds 1 instead of i, so the invariant does not hold at the next test.
// Derived from loops_sum.c.
// tag::source[]
#include <cerastes.h>

int sum_to(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000)
    CERASTES_ENSURES(r, r == n * (n + 1) / 2)
{
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s == i * (i + 1) / 2)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
    s += 1;
  }
  return s;
}
// end::source[]
