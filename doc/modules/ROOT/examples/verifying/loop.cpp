// expect: verified
// expect: native-ok
// A loop with an invariant; every arithmetic step is checked.
// tag::source[]
#include <cerastes.h>

int double_count(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000)
    CERASTES_ENSURES(r, r == 2 * n)
{
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s == 2 * i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s += 2;
    i += 1;
  }
  return s;
}
// end::source[]
