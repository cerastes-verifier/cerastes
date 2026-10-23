// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0022 item 1: a left shift of an unsigned value whose result fits unsigned int is exact (3 << 30 and 1 << 31)
#include <cerastes.h>

unsigned shl_u(unsigned a)
    CERASTES_REQUIRES(a == 3u)
    CERASTES_ENSURES(r, r == 3221225472u)
{
  return a << 30;
}

unsigned top_bit(unsigned a)
    CERASTES_REQUIRES(a == 1u)
    CERASTES_ENSURES(r, r == 2147483648u)
{
  return a << 31;
}
