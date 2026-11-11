// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// Pins: 0059 item 11 (0042 item 3): the marker after the declarator-id (int a [[cerastes::uninit]] [3]) marks the array like the leading one
// Pre-registered from record 0059 item 11 (Part E), before the code: verifies
#include <cerastes.h>

int first(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a CERASTES_UNINIT [3];
  a[0] = 1;
  return a[0];
}
