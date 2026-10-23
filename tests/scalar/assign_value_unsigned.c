// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 1 and 0022: the value of an assignment to an unsigned variable is the converted value (exact: a fits unsigned char)
#include <cerastes.h>

int wrap(int a)
    CERASTES_REQUIRES(0 <= a && a <= 255)
    CERASTES_ENSURES(r, r == 2 * a + 1)
{
  unsigned char b;
  int t = (b = a) + 1;
  return t + b;
}
