// expect: rejected cerastes-missing_decreases @ L12
// A loop without a measure is rejected: termination is part of the result.
// tag::source[]
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  while (i < n) {
    i += 1;
  }
  return i;
}
// end::source[]
