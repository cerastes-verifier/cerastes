// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 1: a for loop's init-statement may declare a local without an initializer; the body assigns it before any read
#include <cerastes.h>

int first(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, r == n)
{
  int total = 0;
  CERASTES_INVARIANT(total <= n)
  CERASTES_DECREASES(n - total)
  for (int i; total < n;) {
    i = total + 1;
    total = i;
  }
  return total;
}
