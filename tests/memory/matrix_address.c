// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 5 (Part E): &m[i][j] formed for a cell inside the matrix is a pointer to that cell (the first bound strictly inside, then the second against the row's own length), a write through it is the write to m[i][j], and the cell is read back by name
// Pre-registered from record 0064 item 5 (Part E), before any run: verifies
#include <cerastes.h>

int addr(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, r == 9)
{
  int m[2][3];
  int *p = &m[i][j];
  *p = 9;
  return m[i][j];
}
