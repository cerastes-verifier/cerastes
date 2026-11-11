// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 items 1 and 2: an unsigned char element, promoted to int in the sum
// Pre-registered from record 0059 item 2: verifies
#include <cerastes.h>

int bytes(void)
    CERASTES_ENSURES(r, r == 255)
{
  unsigned char a[2] = {200, 55};
  return a[0] + a[1];
}
