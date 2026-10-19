// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0013 item 7: a measure that is an expression is checked like any annotation (here it is in range)
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 1000)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n * 1000 - i)
  while (i < n) {
    i += 1;
  }
  return i;
}
