// expect: error termination @ L14
// mode: c23, c++20
// twin: tests/memory/loop_cell_measure.c
// Pins: 0038 item 6: the measure k does not decrease
#include <cerastes.h>

int countdown(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 0)
{
  int k = n;
  int *p = &k;
  CERASTES_INVARIANT(0 <= k && k <= 100)
  CERASTES_DECREASES(k)
  while (k > 0) {
    *p -= 0;
  }
  return k;
}
