// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 items 1 and 2: an enumeration element
// Pre-registered from record 0059 item 2: verifies
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int third(void)
    CERASTES_ENSURES(r, r == 2)
{
  enum color a[2] = {RED, BLUE};
  return (int)a[1];
}
