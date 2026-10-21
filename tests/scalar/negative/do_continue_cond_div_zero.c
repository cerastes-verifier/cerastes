// expect: error division-by-zero @ L20
// mode: c23, c++20
// twin: tests/scalar/do_continue_cond_div.c
// Pins: 0014 items 4 and 5: the condition's division is broken only on the continue path of a do loop
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
      d = 0;
      continue;
    }
  } while (20 / d > 3);
  return d;
}
