// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// Pins: 0059 item 11 (0042 items 3 and 8): cerastes::uninit on an array local without an initializer is accepted and --strict-init asks for it, as for a scalar: the elements start unwritten
// Pre-registered from record 0059 item 11 (Part E), before the code: verifies
#include <cerastes.h>

int first(void)
    CERASTES_ENSURES(r, r == 1)
{
  CERASTES_UNINIT int a[3];
  a[0] = 1;
  return a[0];
}
