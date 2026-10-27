// expect: error postcondition @ L10
// mode: c23, c++20
// twin: tests/scalar/mask_remainder_low_byte.c
// Pins: 0026 item 9: for a negative x the mask is not C's x % 256 (x == -1 gives 255, not -1)
#include <cerastes.h>

int low_byte(int x)
    CERASTES_ENSURES(r, r == x % 256)
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
