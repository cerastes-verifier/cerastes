// expect: error overflow @ L18
// expect: error postcondition @ L20
// mode: c23, c++20
// The invariant no longer relates s to i: s += i may overflow (L18), and the postcondition does not follow (L20).
// tag::source[]
#include <cerastes.h>

int sum_to(int n)
    CERASTES_REQUIRES(0 <= n && n <= 1000)
    CERASTES_ENSURES(r, r == n * (n + 1) / 2)
{
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
    s += i;
  }
  return s;
}
// end::source[]
