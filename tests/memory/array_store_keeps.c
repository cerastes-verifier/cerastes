// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 items 2 and 4: a store to a[1] changes that element and keeps its neighbours (a read of a[i] in a check)
// Pre-registered from record 0059 item 4: verifies
#include <cerastes.h>

int store(void)
    CERASTES_ENSURES(r, r == 6)
{
  int a[3] = {1, 2, 3};
  a[1] = 5;
  CERASTES_CHECK(a[0] == 1);
  CERASTES_CHECK(a[1] == 5);
  CERASTES_CHECK(a[2] == 3);
  return a[0] + a[1];
}
