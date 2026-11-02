// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0042 items 4-7: must_init on a pointer parameter: the callee gets an uninitialized *p (its requires has no flag) and must initialize it; the caller passes &x of a local without an initializer and reads x after
#include <cerastes.h>

void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 0)
{
  *p = 0;
}

int use_init(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x;
  init(&x);
  return x;
}
