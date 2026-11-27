// expect: error out-of-bounds @ L13
// mode: c23, c++20
// twin: tests/memory/matrix_local.c
// Pins: 0064 item 5: m[i][j] with j == 3 reaches no cell of the three-cell row m[i] although the matrix has i's row: out-of-bounds at the first access (the second bound row, against the row's own length: 'whose second index may be outside m[i]'), and nothing else is reported on the path
// Pre-registered from record 0064 item 5 (Part E), before any run: the one error
#include <cerastes.h>

int rw(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j <= 3)
    CERASTES_ENSURES(r, r == 7)
{
  int m[2][3];
  m[i][j] = 7;  // <- here
  return m[i][j];
}
