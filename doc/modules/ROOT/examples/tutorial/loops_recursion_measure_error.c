// expect: error termination @ L16
// mode: c23, c++20
// The measure 1000 - n grows with each recursive call, so it does not decrease.
// Derived from loops_recursion.c.
// tag::source[]
#include <cerastes.h>

int sum_to(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000)
    CERASTES_DECREASES(1000 - n)
    CERASTES_ENSURES(r, r == n * (n + 1) / 2)
{
  if (n == 0) {
    return 0;
  }
  return n + sum_to(n - 1);
}
// end::source[]
