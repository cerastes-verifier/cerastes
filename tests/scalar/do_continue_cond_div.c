// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 items 4 and 5: continue in a do loop goes to the condition, with its obligations
#include <cerastes.h>

int walk_do(int n)
    CERASTES_REQUIRES(1 <= n && n <= 5)
    CERASTES_ENSURES(r, r == 6)
{
  int d = n;
  CERASTES_INVARIANT(1 <= d && d <= 5)
  CERASTES_DECREASES(6 - d)
  do {
    d += 1;
    if (d == 3) {
      d += 1;
      continue;
    }
  } while (20 / d > 3);
  return d;
}
