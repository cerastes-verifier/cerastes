// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/memory/matrix_address.c
// Pins: 0064 item 5: the value read back is the value written through the pointer, nine, so a result of ten fails the postcondition of nine at the return
// Pre-registered from record 0064 item 5 (Part E), before any run: the one error
#include <cerastes.h>

int addr(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, r == 9)
{
  int m[2][3];
  int *p = &m[i][j];
  *p = 9;
  return m[i][j] + 1;  // <- here
}
