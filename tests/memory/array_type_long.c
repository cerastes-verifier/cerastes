// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 items 1 and 2: a long element
// Pre-registered from record 0059 item 2: verifies
#include <cerastes.h>

long wide(void)
    CERASTES_ENSURES(r, r == 3000000001L)
{
  long a[2] = {3000000000L, 1L};
  return a[0] + a[1];
}
