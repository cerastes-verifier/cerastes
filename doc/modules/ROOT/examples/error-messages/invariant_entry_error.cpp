// expect: error invariant-entry @ L11
// A program that fails with the invariant-entry error.
// tag::source[]
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_ENSURES(r, r == n)
{
  int i = 1;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return i;
}
// end::source[]
