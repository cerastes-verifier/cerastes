// expect: error out-of-bounds @ L14
// mode: c23, c++20
// twin: tests/memory/array_loop_measure.c
// Pins: 0059 item 8 (audit round 3, item 2): the measure reads a[i] with i up to 3 (a[i] * 0 keeps the measure 2 - n): out-of-bounds at the clause that reads it
// Pre-registered from audit round 3 of record 0059
#include <cerastes.h>

int count(int i)
    CERASTES_REQUIRES(0 <= i && i <= 3)
{
  int a[3] = {1, 2, 3};
  int n = 0;
  CERASTES_INVARIANT(0 <= n && n <= 2)
  CERASTES_DECREASES(2 - n + a[i] * 0)
  while (n < 2) {
    n = n + 1;
  }
  return n + a[0];
}
