// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 items 4 and 5: continue in a do loop goes to the condition, with its obligations
// steps counts the iterations, and the invariant says exactly how d moves: one per iteration, one more when the
// walk jumps over 3 (only if it starts below 3), so the continue branch is observed.
#include <cerastes.h>

int walk_do(int n)
    CERASTES_REQUIRES(1 <= n && n <= 5)
    CERASTES_ENSURES(r, r == (n <= 2 ? 5 - n : 6 - n))
{
  int d = n;
  int steps = 0;
  CERASTES_INVARIANT(1 <= d && d <= 5 && n <= d && (d != 3 || n == 3) && steps == d - n - ((n <= 2 && d >= 4) ? 1 : 0))
  CERASTES_DECREASES(6 - d)
  do {
    d += 1;
    steps += 1;
    if (d == 3) {
      d += 1;
      continue;
    }
  } while (20 / d > 3);
  CERASTES_CHECK(d == 6);
  return steps;
}
