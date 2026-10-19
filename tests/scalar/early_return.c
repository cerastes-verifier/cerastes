// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 9: several returns in one function
#include <cerastes.h>

int sign(int x)
    CERASTES_ENSURES(r, (x < 0 && r == -1) || (x == 0 && r == 0) || (x > 0 && r == 1))
{
  if (x < 0) {
    return -1;
  }
  if (x > 0) {
    return 1;
  }
  return 0;
}
