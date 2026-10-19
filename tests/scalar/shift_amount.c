// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 8: the shift amount must be in [0, width)
#include <cerastes.h>

int pow2(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 30)
    CERASTES_ENSURES(r, r >= 1)
{
  return 1 << n;
}
