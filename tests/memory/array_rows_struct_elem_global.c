// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 9 and 3 (Part E; the program of a depth-2 leaf path as a global array): a global array of structures with an array member, g[i].a[j], with its initial values per leaf (listed, symbolic read, the written cell, the neighbours unchanged)
// Pre-registered from record 0064 items 3 and 9 (Part E), before any run: verifies
#include <cerastes.h>

struct W {
  int a[4];
  int n;
};

static struct W g[2] = {{{1, 2, 3, 4}, 5}, {{6, 7, 8, 9}, 10}};

int main(void)
{
  CERASTES_CHECK(g[0].a[0] == 1 && g[1].a[3] == 9 && g[1].n == 10);
  int i = 1;
  int j = 2;
  g[i].a[j] = 70;
  CERASTES_CHECK(g[i].a[j] == 70 && g[1].a[1] == 7 && g[0].a[2] == 3);
  (void)i;
  (void)j;
  return g[0].n - 5;
}
