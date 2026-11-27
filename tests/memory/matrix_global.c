// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 9 and 3 (Part E; 0063 item 9): a global int g[2][3] with initial values by linkage (a static one is known in main: each listed cell, the written cell, a symbolic read), and a static local matrix c (an implicit owns atom in the contracts of next and of the functions that reach it, its entry value unknown, so next guards the cell it reads)
// Pre-registered from record 0064 items 3 and 9 (Part E), before any run: verifies
#include <cerastes.h>

static int g[2][3] = {{1, 2, 3}, {4, 5, 6}};

int next(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
    CERASTES_ENSURES(r, 1 <= r && r <= 100)
{
  static int c[2][3] = {{0}};
  int v = c[i][j];
  if (v < 0 || v > 99) {
    v = 0;
  }
  c[i][j] = v + 1;
  return c[i][j];
}

int main(void)
{
  CERASTES_CHECK(g[0][0] == 1 && g[1][2] == 6);
  g[1][1] = 50;
  CERASTES_CHECK(g[1][1] == 50 && g[1][0] == 4);
  int i = 1;
  int j = 2;
  CERASTES_CHECK(g[i][j] == 6);
  int a = next(0, 1);
  CERASTES_CHECK(1 <= a && a <= 100);
  return g[0][1] - 2 + (a - a) + (i - 1) + (j - 2);
}
