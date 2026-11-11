// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 3 (audit round 2, item 5c): every constant element goes through the type's range check: 255 fits an unsigned char
// Pre-registered from audit round 2 of record 0059 (item 5c): verifies
#include <cerastes.h>

int f(void)
    CERASTES_ENSURES(r, r == 1)
{
  unsigned char a[2] = {255, 1};
  return a[1];
}
