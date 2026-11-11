// expect: error termination @ L13
// mode: c23, c++20
// twin: tests/memory/array_loop_measure.c
// Pins: 0059 item 8 (audit round 3, item 2): the body stores 3 into a[0] instead of decrementing it, so the measure a[0] does not decrease: termination at the clause
// Pre-registered from audit round 3 of record 0059
#include <cerastes.h>

int countdown(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[1] = {3};
  CERASTES_INVARIANT(a[0] >= 0)
  CERASTES_DECREASES(a[0])
  while (a[0] > 0) {
    a[0] = 3;
  }
  return a[0];
}
