// expect: error division-by-zero @ L15
// mode: c23, c++20
// twin: tests/scalar/continue_cond_div.c
// Pins: 0014 item 4: the condition's division is broken only on the continue path
// The continue branch sets d to 0; the normal path keeps d >= 1.
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
      d = 0;
      continue;
    }
  }
  return d;
}
