// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 4 (0037 item 8 over a quantified chunk): &a[i] with a symbolic index given to a callee, the element read back
// Pre-registered from record 0059 item 4: verifies
#include <cerastes.h>

void set(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 5)
{
  *p = 5;
}

int use_at(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
    CERASTES_ENSURES(r, r == 5)
{
  int a[3] = {1, 2, 3};
  set(&a[i]);
  return a[i];
}
