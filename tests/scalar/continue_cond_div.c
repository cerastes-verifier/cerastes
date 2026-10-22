// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 4: a continue path evaluates the loop condition, with its obligations
// The division in the condition is safe on both paths back to the test.
// The invariant has no separate d != 0 (n <= d and 1 <= n imply it); that the condition's division is checked on the
// continue path is pinned by the twin continue_cond_div_zero.c, which breaks it there.
// steps counts the iterations, and the invariant says exactly how d moves: one per iteration, one more when the
// walk jumps over 3 (only if it starts below 3), so the continue branch is observed.
#include <cerastes.h>

int walk(int n)
    CERASTES_REQUIRES(1 <= n && n <= 5)
    CERASTES_ENSURES(r, r == (n <= 2 ? 5 - n : 6 - n))
{
  int d = n;
  int steps = 0;
  CERASTES_INVARIANT(0 <= d && d <= 6 && n <= d && (d != 3 || n == 3) && steps == d - n - ((n <= 2 && d >= 4) ? 1 : 0))
  CERASTES_DECREASES(6 - d)
  while (20 / d > 3) {
    d += 1;
    steps += 1;
    if (d == 3) {
      d += 1;
      continue;
    }
  }
  CERASTES_CHECK(d == 6);
  return steps;
}
