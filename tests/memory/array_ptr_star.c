// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 10: *a is a[0] (the decay of a under the dereference)
// Pre-registered from record 0059 items 4 and 10 (Part D), before the code: verifies
#include <cerastes.h>

int star(void)
    CERASTES_ENSURES(r, r == 5)
{
  int a[3] = {1, 2, 3};
  *a = 5;
  return a[0];
}
