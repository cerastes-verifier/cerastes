// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 item 8, Loops bullet (re-audit 2): f holds owns(*s) with s == &g and the loop writes g.x by its name: the leaf of g is the same cell as the parameter's leaf, so the automatic place rows must not give it twice (the correct program that the global-leaf loop rule broke)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

static struct S g = {0, 5};
void f(struct S *s)
    CERASTES_REQUIRES(cerastes_owns(*s) && s == &g)
    CERASTES_ENSURES(cerastes_owns(*s) && s->x == 1)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && (i == 0 || s->x == 1))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    g.x = 1;
    i += 1;
  }
  if (i == 0) {
    s->x = 1;
  }
}
int main(void)
{
  f(&g);
  CERASTES_CHECK(g.x == 1);
  return 0;
}
