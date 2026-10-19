// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 8: left shift of a non-negative value that fits
#include <cerastes.h>

int times16(int a)
    CERASTES_REQUIRES(a >= 0 && a <= 100000000)
    CERASTES_ENSURES(r, r == a * 16)
{
  return a << 4;
}
