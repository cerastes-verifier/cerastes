// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 items 6 and 8: unsigned char << 24 is a signed int shift (it can reach the sign bit)
// 127 << 24 fits; 128 << 24 does not.
#include <cerastes.h>

int top_byte(unsigned char c)
    CERASTES_REQUIRES(c <= 127)
    CERASTES_ENSURES(r, r >= 0 && r == c * 16777216)
{
  return c << 24;
}
