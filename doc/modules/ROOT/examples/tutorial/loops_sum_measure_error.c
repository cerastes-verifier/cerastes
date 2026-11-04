// expect: error termination @ L15
// mode: c23, c++20
// The measure n does not decrease, so the loop may not end.
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
  CERASTES_DECREASES(n)
  while (i < n) {
    i += 1;
    s += i;
  }
  return s;
}
// end::source[]
