// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0037 item 13: a callee that takes owns(*p) and returns nothing keeps the cell; the caller owned it, so it ends without ensuring it
#include <cerastes.h>
#include <stddef.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int use_then_dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, r == cerastes_old(*p) + 1)
{
  int v = *p + 1;
  dispose(p);
  return v;
}

void pass_on(int *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
{
  dispose(p);
  dispose(q);
}
