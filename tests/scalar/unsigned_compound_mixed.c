// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0022 item 1: u += 1L is computed in long and converted back (exact while it fits); uc -= 1u is computed in unsigned int (exact while not below zero)
#include <cerastes.h>

unsigned add_long(unsigned u)
    CERASTES_REQUIRES(u == 4294967294u)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  u += 1L;
  return u;
}

unsigned char sub_uchar(unsigned char uc)
    CERASTES_REQUIRES(uc == 1)
    CERASTES_ENSURES(r, r == 0)
{
  uc -= 1u;
  return uc;
}
