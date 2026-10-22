// expect: error postcondition @ L25
// mode: c23, c++20
// twin: tests/scalar/do_continue_cond_div.c
// Pins: 0014 item 4: the exact steps invariant and ensures of the positive are what notice the continue branch
// The ensures drops the n >= 3 case: those walks take 6 - n steps, not 5 - n.
#include <cerastes.h>

int walk_do(int n)
    CERASTES_REQUIRES(1 <= n && n <= 5)
    CERASTES_ENSURES(r, r == 5 - n)
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
