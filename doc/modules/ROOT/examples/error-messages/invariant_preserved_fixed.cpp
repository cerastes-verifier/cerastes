// expect: verified
// expect: native-ok
// The same program, fixed.
// tag::source[]
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000)
    CERASTES_ENSURES(r, r >= 0)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return i;
}
// end::source[]
