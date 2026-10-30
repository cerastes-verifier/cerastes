// expect: error invariant-entry @ L14
// mode: c23, c++20
// twin: tests/memory/loop_cell_invariant.c
// Pins: 0038 item 6: the clause about the cell is checked on entry like any other
#include <cerastes.h>

int invariant_cell(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 0;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && x == 1)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return *p;
}
