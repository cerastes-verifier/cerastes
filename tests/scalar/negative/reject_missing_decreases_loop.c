// expect: rejected cerastes-missing_decreases @ L13
// mode: c23, c++20
// twin: tests/scalar/measure_loop.c
// Pins: 0013 item 3: a while loop without a decreases is rejected
// The loop has an invariant but no measure.
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  while (i < n) {
    i += 1;
  }
  return i;
}
