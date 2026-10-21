// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 4: a continue path evaluates the loop condition, with its obligations
// The division in the condition is safe on both paths back to the test.
// The invariant does not mention d != 0: the obligations of the condition are checked on every path to the test.
#include <cerastes.h>

int walk(int n)
    CERASTES_REQUIRES(1 <= n && n <= 5)
    CERASTES_ENSURES(r, r == 6)
{
  int d = n;
  CERASTES_INVARIANT(0 <= d && d <= 6)
  CERASTES_DECREASES(6 - d)
  while (20 / d > 3) {
    d += 1;
    if (d == 3) {
      d += 1;
      continue;
    }
  }
  return d;
}
