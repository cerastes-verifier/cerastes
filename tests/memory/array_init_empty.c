// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 3: an empty braced list (C23 and C++) zero-fills every element
// Pre-registered from record 0059 item 4: verifies
#include <cerastes.h>

int none(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[2] = {};
  return a[0] + a[1];
}
