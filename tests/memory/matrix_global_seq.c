// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 11 (Part E; 0063 item 9): the object keys of a global matrix carry both index steps: g[0][0] = g[1][1]++ in main (two constant cells), and g[i][j] = g[k][l]++ under owns(g) and (i != k || j != l) (two cells that differ in a row or in a column); the twin removes the distinctness
// Pre-registered from record 0064 item 11 (Part E), before any run: verifies
#include <cerastes.h>

static int g[2][3] = {{1, 2, 3}, {4, 5, 6}};

int cells(int i, int j, int k, int l)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 2 && 0 <= j && j < 3 && 0 <= k && k < 2 && 0 <= l && l < 3 && (i != k || j != l))
    CERASTES_ENSURES(cerastes_owns(g))
{
  int old = g[k][l];
  if (old < 0 || old > 99) {
    return 0;
  }
  g[i][j] = g[k][l]++;
  return old;
}

int main(void)
{
  g[0][0] = g[1][1]++;
  CERASTES_CHECK(g[0][0] == 5 && g[1][1] == 6);
  return g[0][0] - 5;
}
