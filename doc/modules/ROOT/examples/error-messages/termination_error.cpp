// expect: error termination @ L12
// A program that fails with the termination error.
// tag::source[]
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
  }
  return i;
}
// end::source[]
