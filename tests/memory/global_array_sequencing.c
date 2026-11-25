// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 11 (0059 item 9, 0041 item 11): g[0] = g[1]++ over two constant elements of an array global is sequenced enough (distinct addresses), and g[i] = g[j]++ under i != j and the owns atom
// Pre-registered from record 0063 item 11 (Part B), before any run: verifies
#include <cerastes.h>

static int g[3] = {1, 2, 3};

int shift(int i, int j)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= i && i < 3 && 0 <= j && j < 3 && i != j && 0 <= g[j] && g[j] < 100)
    CERASTES_ENSURES(r, cerastes_owns(g) && r == g[j])
{
  g[i] = g[j]++;
  return g[j];
}

int main(void)
{
  g[0] = g[1]++;
  CERASTES_CHECK(g[0] == 2 && g[1] == 3);
  return 0;
}
