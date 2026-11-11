// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 items 2 to 4: a local array with a braced initializer, a[0] and a[2] read as elements of one block
// Pre-registered from record 0059 item 4: verifies
#include <cerastes.h>

int sum_ends(void)
    CERASTES_ENSURES(r, r == 4)
{
  int a[3] = {1, 2, 3};
  return a[0] + a[2];
}
