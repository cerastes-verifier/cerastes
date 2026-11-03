// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 items 4, 9: a callee that takes owns(p->x) and returns nothing keeps that leaf: the caller owned it, so it ends without ensuring it
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void dispose(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(p->x));

int use_then_dispose(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->x < 100)
    CERASTES_ENSURES(r, r == cerastes_old(p->x) + 1)
{
  int v = p->x + 1;
  dispose(p);
  return v;
}
