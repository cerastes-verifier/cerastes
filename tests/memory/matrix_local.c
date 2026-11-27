// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 3, 5 and 6 (Part E): a local int m[2][3] is an array of two rows, each row an array object in its own block; m[i][j] is written and read for symbolic indexes (the first index against the matrix, then the second against m[i]'s own length, then the cell's rows), two writes at cells that differ in a row or in a column are kept apart (the cells are distinct slots of distinct or equal rows), and the six cells are written and read at constant indexes (array_reject_array_of_arrays.c, the staging pin of the retired rejection, is this program's declaration)
// Pre-registered from record 0064 items 3, 5 and 6 (Part E), before any run: verifies
#include <cerastes.h>

int rw(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, r == 7)
{
  int m[2][3];
  m[i][j] = 7;
  return m[i][j];
}

int two(int i, int j, int k, int l)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3 && 0 <= k && k < 2 && 0 <= l && l < 3 && (i != k || j != l))
    CERASTES_ENSURES(r, r == 7)
{
  int m[2][3];
  m[i][j] = 3;
  m[k][l] = 4;
  return m[i][j] + m[k][l];
}

int cells(void)
    CERASTES_ENSURES(r, r == 21)
{
  int m[2][3];
  m[0][0] = 1;
  m[0][1] = 2;
  m[0][2] = 3;
  m[1][0] = 4;
  m[1][1] = 5;
  m[1][2] = 6;
  return m[0][0] + m[0][1] + m[0][2] + m[1][0] + m[1][1] + m[1][2];
}
