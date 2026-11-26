// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 7, 8 and 9 (Part C2): owns(g) and reads(g) of a global array of structures in callees called from main (every leaf of every element; an element read in the requires, the ensures with old of a leaf, the result read from a leaf), and the alias positive: pair(&x) against requires(owns(*p) && owns(g)) with another object
// Pre-registered from record 0064 items 7 and 9 (Part C2), before any run: verifies
#include <cerastes.h>

struct P {
  int x;
  int y;
};

static struct P g[3] = {{1, 2}, {3, 4}};
static int x = 9;

void bump(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g[1].x < 100)
    CERASTES_ENSURES(cerastes_owns(g) && g[1].x == cerastes_old(g[1].x) + 1 && g[0].y == cerastes_old(g[0].y))
{
  g[1].x += 1;
}

int peek(void)
    CERASTES_REQUIRES(cerastes_reads(g))
    CERASTES_ENSURES(r, cerastes_reads(g) && r == g[2].y)
{
  return g[2].y;
}

int pair(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(g) && *p == 1 && g[0].x == 2 && r == 0)
{
  *p = 1;
  g[0].x = 2;
  return 0;
}

int main(void)
{
  CERASTES_CHECK(g[1].x == 3);
  bump();
  CERASTES_CHECK(g[1].x == 4 && g[0].y == 2);
  int v = peek();
  CERASTES_CHECK(v == g[2].y);
  (void)v;
  pair(&x);
  CERASTES_CHECK(g[0].x == 2 && x == 1);
  return 0;
}
