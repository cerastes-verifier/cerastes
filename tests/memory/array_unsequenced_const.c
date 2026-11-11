// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 9: two constant indices differ, so the unsequenced row holds with no precondition
// Pre-registered from record 0059 item 9: verifies
#include <cerastes.h>

int shift(void)
    CERASTES_ENSURES(r, r == 23)
{
  int a[3] = {1, 2, 3};
  a[0] = a[1]++;
  return a[0] * 10 + a[1];
}
