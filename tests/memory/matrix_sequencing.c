// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 11 (Part E; 0039 item 3): the object keys of a matrix carry both index steps, so m[0][0] = m[1][1]++ (two constant cells), m[i][j] = m[k][l]++ under (i != k || j != l) (two cells that differ in a row or in a column) and m[i][0] = m[i][1]++ (two cells of one row) are sequenced enough; no compiler warns on any of them (array_struct_sequencing.c is the same rule for elements that are structures)
// Pre-registered from record 0064 item 11 (Part E), before any run: verifies
#include <cerastes.h>

int consts(void)
    CERASTES_ENSURES(r, r == 5)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  m[0][0] = m[1][1]++;
  CERASTES_CHECK(m[0][0] == 5 && m[1][1] == 6);
  return m[0][0];
}

int cells(int i, int j, int k, int l)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3 && 0 <= k && k < 2 && 0 <= l && l < 3 && (i != k || j != l))
    CERASTES_ENSURES(r, r == 0)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int old = m[k][l];
  m[i][j] = m[k][l]++;
  CERASTES_CHECK(m[i][j] == old && m[k][l] == old + 1);
  return m[i][j] - old;
}

int row(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
    CERASTES_ENSURES(r, r == 0)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int old = m[i][1];
  m[i][0] = m[i][1]++;
  CERASTES_CHECK(m[i][0] == old && m[i][1] == old + 1);
  return m[i][0] - old;
}
