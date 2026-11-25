// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 items 7 and 8: owns(g) and reads(g) of an array global in callees called from main (every element, the ensures with old of an element, a decreases that reads an element), and the alias positive: pair(&x) against requires(owns(*p) && owns(g)) with another object
// Pre-registered from record 0063 item 7 (Part B), before any run: verifies
#include <cerastes.h>

static int g[4] = {1, 2, 3, 4};
static int x = 9;

void bump(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g[1] < 100)
    CERASTES_ENSURES(cerastes_owns(g) && g[1] == cerastes_old(g[1]) + 1 && g[0] == cerastes_old(g[0]))
{
  g[1] += 1;
}

int peek(void)
    CERASTES_REQUIRES(cerastes_reads(g))
    CERASTES_ENSURES(r, cerastes_reads(g) && r == g[2])
{
  return g[2];
}

int pair(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(g) && *p == 1 && g[0] == 2 && r == 0)
{
  *p = 1;
  g[0] = 2;
  return 0;
}

void down(void)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= g[0] && g[0] <= 100)
    CERASTES_ENSURES(cerastes_owns(g) && g[0] == 0)
    CERASTES_DECREASES(g[0])
{
  if (g[0] > 0) {
    g[0] -= 1;
    down();
  }
}

int main(void)
{
  CERASTES_CHECK(g[1] == 2);
  bump();
  CERASTES_CHECK(g[1] == 3 && g[0] == 1);
  int v = peek();
  CERASTES_CHECK(v == g[2]);
  (void)v;
  pair(&x);
  CERASTES_CHECK(g[0] == 2 && x == 1);
  g[0] = 3;
  down();
  CERASTES_CHECK(g[0] == 0);
  return 0;
}
