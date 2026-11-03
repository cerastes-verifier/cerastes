// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 item 8, Loops bullet (re-audit 2): the parameter s is reassigned and p = &s->x keeps the entry address; the invariant owns(*p) names the held cell while the loop writes g.x by name (the correct program that the global-leaf loop rule broke; the final test of p only keeps p used for the ordinary compilers)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

static struct S g = {0, 5};

void f(struct S *s)
    CERASTES_REQUIRES(cerastes_owns(*s) && s == &g)
    CERASTES_ENSURES(cerastes_owns(*s))
{
  int *p = &s->x;
  s = &g;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && cerastes_owns(*p))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    g.x = 1;
    i += 1;
  }
  if (p == &g.x) {
    i = 0;
  }
}

int main(void)
{
  f(&g);
  return 0;
}
