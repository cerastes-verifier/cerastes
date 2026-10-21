// expect: error invariant-preserved @ L14
// mode: c23, c++20
// twin: tests/scalar/do_continue_cond_div.c
// Pins: 0014 item 4: the exact steps invariant and ensures of the positive are what notice the continue branch
// The continue branch adds 2, so d jumps from 3 to 5 and the walk takes one step too few: the steps invariant breaks.
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
      d += 2;
      continue;
    }
  } while (20 / d > 3);
  CERASTES_CHECK(d == 6);
  return steps;
}
