// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 3: without an initializer the flags stay false; a write sets the element's flag
// Pre-registered from record 0059 item 4: verifies
#include <cerastes.h>

int one(void)
    CERASTES_ENSURES(r, r == 7)
{
  int a[3];
  a[1] = 7;
  return a[1];
}
