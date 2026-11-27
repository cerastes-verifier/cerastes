// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/memory/matrix_cells.c
// Pins: 0064 items 2 and 3: the value read through the pointer cell is x, five, so a result of six fails the postcondition at the return
// Pre-registered from record 0064 items 2 and 3 (Part E), before any run: the one error
#include <cerastes.h>

int ptrs(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 2)
    CERASTES_ENSURES(r, r == 5)
{
  int x = 5;
  int *p[2][2] = {{0, 0}, {0, 0}};
  p[i][j] = &x;
  return *p[i][j] + 1;  // <- here
}
