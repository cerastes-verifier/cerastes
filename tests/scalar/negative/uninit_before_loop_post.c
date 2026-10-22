// expect: error invariant-preserved @ L15
// mode: c23, c++20
// twin: tests/scalar/uninit_before_loop.c
// Pins: 0019 item 4: step is assigned and read in the loop, with the wrong value
#include <cerastes.h>

int count_by_two(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 2 * n)
{
  int step;
  int i = 0;
  int s = 0;
  step = 3;
  CERASTES_INVARIANT(0 <= i && i <= n && s == 2 * i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s += step;
    i += 1;
  }
  return s;
}
