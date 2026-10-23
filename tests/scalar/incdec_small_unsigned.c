// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0022 item 1: ++, += and -- on unsigned char and unsigned short are computed in int and converted back (an exact conversion)
#include <cerastes.h>

unsigned char inc_uc(unsigned char v)
    CERASTES_REQUIRES(v == 254)
    CERASTES_ENSURES(r, r == 255)
{
  v++;
  return v;
}

unsigned char add_uc(unsigned char v)
    CERASTES_REQUIRES(v == 254)
    CERASTES_ENSURES(r, r == 255)
{
  v += 1;
  return v;
}

unsigned short dec_us(unsigned short v)
    CERASTES_REQUIRES(v == 1)
    CERASTES_ENSURES(r, r == 0)
{
  v--;
  return v;
}
