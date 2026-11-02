// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0042 items 5, 7: a must_init parameter passed on to another must_init callee (no flag row at either call)
#include <cerastes.h>

void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 0)
{
  *p = 0;
}

void init_twice(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 0)
{
  init(p);
}

int use_twice(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x;
  init_twice(&x);
  return x;
}
