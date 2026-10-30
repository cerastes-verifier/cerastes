// expect: error postcondition @ L23
// mode: c23, c++20
// twin: tests/memory/loop_read_only_cell.c
// Pins: 0038 item 3: the cell keeps 7, not 8
#include <cerastes.h>

int keeps(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 8)
{
  int x = 7;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (*p == 7) {
      i += 1;
    } else {
      i += 1;
    }
  }
  return x;
}
