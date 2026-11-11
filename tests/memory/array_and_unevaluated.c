// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 4 (as updated): the right operand of && with a false left is not evaluated, so 0 && a[3] is accepted (the guarded row cannot fail; no compiler warns)
// Pre-registered from record 0059 item 4 (as updated): verifies
#include <cerastes.h>

int never(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[3] = {1, 2, 3};
  return 0 && a[3];
}
