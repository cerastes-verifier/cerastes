// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/memory/loop_cell_declared_after.c
// Pins: 0038 items 3 and 7: the return inside the loop is checked, not skipped: it returns 4, the contract says 3
#include <cerastes.h>

int later_cell(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 3)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == 5) {
      return 4;
    }
    i += 1;
  }
  int x = 3;
  int *p = &x;
  return *p;
}
