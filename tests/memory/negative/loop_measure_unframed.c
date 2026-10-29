// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/loop_cell_measure.c
// Pins: 0038 item 6 (framing): the measure reads k, whose object a callee kept before the loop: reported at the decreases clause
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int measure_cell(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int k = 5;
  int *p = &k;
  dispose(p);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i + k)
  while (i < n) {
    i += 1;
  }
  return i;
}
