// expect: verified
// expect: native-ok
// A recursive function has a measure: the recursive call has a smaller one.
// tag::source[]
#include <cerastes.h>

int twice(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, r == 2 * n)
{
  if (n == 0) {
    return 0;
  }
  return 2 + twice(n - 1);
}
// end::source[]
