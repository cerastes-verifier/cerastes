// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 items 4 and 6: &a[1] given to a callee that owns *p: it takes one element's cells and gives them back
// Pre-registered from record 0059 item 4: verifies
#include <cerastes.h>

void set(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 5)
{
  *p = 5;
}

int use_set(void)
    CERASTES_ENSURES(r, r == 5)
{
  int a[3] = {1, 2, 3};
  set(&a[1]);
  return a[1];
}
