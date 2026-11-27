// expect: error unsequenced @ L12
// mode: c23, c++20
// twin: tests/memory/matrix_sequencing.c
// Pins: 0064 item 11 (0039 item 3): without (i != k || j != l) the write to m[i][j] and the increment of m[k][l] may reach one cell of one row (the path-precise row)
// Pre-registered from record 0064 item 11 (Part E), before any run: the one error
#include <cerastes.h>

int cells(int i, int j, int k, int l)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3 && 0 <= k && k < 2 && 0 <= l && l < 3)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  m[i][j] = m[k][l]++;  // <- here
  return m[0][0];
}
