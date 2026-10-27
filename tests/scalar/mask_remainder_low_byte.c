// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0026 items 7 to 9: x & 255 is the non-negative remainder modulo 256 for every x, also with the mask on the left and in a compound assignment
#include <cerastes.h>

int low_byte(int x)
    CERASTES_ENSURES(r, r == ((x % 256) + 256) % 256)
{
  return x & 255;
}

int low_byte_left(int x)
    CERASTES_ENSURES(r, r == ((x % 256) + 256) % 256)
{
  return 255 & x;
}

unsigned low_byte_assign(unsigned u)
    CERASTES_ENSURES(r, r == u % 256)
{
  unsigned w = u;
  w &= 255;
  return w;
}

int low_byte_assign_signed(int x)
    CERASTES_ENSURES(r, r == ((x % 256) + 256) % 256)
{
  int w = x;
  w &= 255;
  return w;
}
