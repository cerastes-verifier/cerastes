// expect: error termination @ L14
// mode: c23, c++20
// twin: tests/scalar/measure_loop.c
// Pins: 0013 item 2: a measure that grows
// Counting up with the measure i: it increases, the loop is bounded by the invariant but the measure is not.
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(i)
  while (i < n) {
    i += 1;
  }
  return i;
}
