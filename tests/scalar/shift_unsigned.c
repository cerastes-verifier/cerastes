// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 8: left shift of an unsigned value wraps
// 3000000000 << 1 = 6000000000 mod 2^32 = 1705032704.
#include <cerastes.h>

unsigned shl_u(unsigned a)
    CERASTES_REQUIRES(a == 3000000000u)
    CERASTES_ENSURES(r, r == 1705032704u)
{
  return a << 1;
}
