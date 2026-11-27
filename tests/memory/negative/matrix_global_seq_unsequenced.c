// expect: error unsequenced @ L18
// mode: c23, c++20
// twin: tests/memory/matrix_global_seq.c
// Pins: 0064 item 11 (0039 item 3): without (i != k || j != l) the write to g[i][j] and the increment of g[k][l] may reach one cell of a global matrix (the path-precise row)
// Pre-registered from record 0064 item 11 (Part E), before any run: the one error
#include <cerastes.h>

static int g[2][3] = {{1, 2, 3}, {4, 5, 6}};

int cells(int i, int j, int k, int l)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3 && 0 <= k && k < 2 && 0 <= l && l < 3)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int old = g[k][l];
  if (old < 0 || old > 99) {
    return 0;
  }
  g[i][j] = g[k][l]++;  // <- here
  return old;
}

int main(void)
{
  return 0;
}
