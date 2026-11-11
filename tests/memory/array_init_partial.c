// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 3: elements without an initializer are zero (C23 6.7.11p21): zero_arr then one store
// Pre-registered from record 0059 item 4: verifies
#include <cerastes.h>

int partial(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[4] = {1};
  CERASTES_CHECK(a[0] == 1);
  CERASTES_CHECK(a[3] == 0);
  return a[0] + a[3];
}
