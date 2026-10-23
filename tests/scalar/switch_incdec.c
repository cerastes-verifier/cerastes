// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 1: the controlling expression of a switch may have a side effect; the cases see the new value
#include <cerastes.h>

int dispatch(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == (x == 0 ? 1 : (x == 1 ? 20 : -(x + 1))))
{
  switch (x++) {
  case 0:
    return x;
  case 1:
    return x * 10;
  default:
    return -x;
  }
}
