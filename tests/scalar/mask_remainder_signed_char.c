// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0026 items 7 and 8 (pin of the conversion): w &= ~0x0F on a signed char stays in range (the complement of 2^k - 1 only lowers the value), and w &= 0x7F too; the conversion back to signed char is exact
#include <cerastes.h>

signed char clear_low_nibble_char(signed char sc)
    CERASTES_ENSURES(r, r == sc - ((sc % 16) + 16) % 16)
{
  signed char w = sc;
  w &= ~0x0F;
  return w;
}

signed char low_seven_char(signed char sc)
    CERASTES_ENSURES(r, r == ((sc % 128) + 128) % 128)
{
  signed char w = sc;
  w &= 0x7F;
  return w;
}
