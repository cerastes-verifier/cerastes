// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 items 2 and 4 (audit B2): a call that never returns (ensures false) in an expression: the checks of the accesses C may evaluate before it (a store's null check, a divisor, a recursive call's decrease) hold, so the functions verify
#include <cerastes.h>
#include <stddef.h>

int nr(void)
    CERASTES_ENSURES(r, false);

int store_beside(int *r, int *q)
    CERASTES_REQUIRES(cerastes_owns(*r) && cerastes_owns(*q))
    CERASTES_ENSURES(res, cerastes_owns(*r) && cerastes_owns(*q))
{
  return (*r = nr()) + (*q = 1);
}

int divide_beside(int *q, int *r)
    CERASTES_REQUIRES(cerastes_owns(*q) && cerastes_owns(*r) && *r != 0)
    CERASTES_ENSURES(res, cerastes_owns(*q) && cerastes_owns(*r))
{
  return (*q = nr()) + 100 / *r;
}

int down(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p >= 0 && *p < 100)
    CERASTES_DECREASES(*p)
    CERASTES_ENSURES(res, cerastes_owns(*p))
{
  int a = 0;
  int b = 0;
  int *q = &a;
  int *s = &b;
  if (*p == 0) {
    return 0;
  }
  *p -= 1;
  return (*q = nr()) + (*s = 1, down(p));
}
