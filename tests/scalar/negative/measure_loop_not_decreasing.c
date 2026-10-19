// expect: error termination @ L14
// mode: c23, c++20
// twin: tests/scalar/measure_loop.c
// Pins: 0013 item 8: an iteration that does not change the measure
// The body adds 0, so the loop does not terminate; the termination error is at the measure.
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 0;
  }
  return i;
}
