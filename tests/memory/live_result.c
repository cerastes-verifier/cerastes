// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0037 items 3, 5: live(p) in requires and ensures; returning a parameter is a use, so the contract gives live(p)
#include <cerastes.h>
#include <stddef.h>

int *id(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(r, r == p && cerastes_live(p))
{
  return p;
}

int use_id(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 1;
  int *q = id(&x);
  return *q;
}
