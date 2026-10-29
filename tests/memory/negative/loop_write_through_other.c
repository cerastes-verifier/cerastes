// expect: error postcondition @ L22
// mode: c23, c++20
// twin: tests/memory/loop_write_other_cell.c
// Pins: 0038 item 3 (the owner's rule): a store through a pointer to an int havocs every int place, so x loses its value 7
#include <cerastes.h>

int other_cell(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 7)
{
  int x = 7;
  int y = 0;
  int *p = &x;
  int *q = &y;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *q = i;
    i += 1;
  }
  return *p;
}
