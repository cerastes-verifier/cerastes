// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 4 (as updated): an operand of sizeof is not evaluated, so sizeof(a[3]) is accepted: no compiler warns and no row is made for it
// Pre-registered from record 0059 item 4 (as updated): verifies
#include <cerastes.h>

int size(void)
    CERASTES_ENSURES(r, r == 5)
{
  int a[3] = {1, 2, 3};
  return (int)sizeof(a[3]) + a[0];
}
