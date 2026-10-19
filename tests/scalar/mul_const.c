// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 3: signed multiplication by a constant, at the exact bound
// 715827882 * 3 = 2147483646 fits; the next value does not.
#include <cerastes.h>

int triple(int a)
    CERASTES_REQUIRES(a >= -715827882 && a <= 715827882)
    CERASTES_ENSURES(r, r == a + a + a)
{
  return a * 3;
}
