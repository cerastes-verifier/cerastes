// expect: error postcondition @ L58
// mode: c23, c++20
// twin: tests/scalar/mask_or_xor_low_byte.c
// Pins: 0027 item 1: w ^= 255 is not w |= 255 (u == 1 gives 254, not 255)
#include <cerastes.h>

int or_low_byte(int x)
    CERASTES_ENSURES(r, r == x - ((x % 256) + 256) % 256 + 255)
{
  return x | 255;
}

int or_low_byte_nonneg(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == x - x % 256 + 255)
{
  return x | 255;
}

int xor_low_byte(int x)
    CERASTES_ENSURES(r, r == (x - ((x % 256) + 256) % 256) + (255 - ((x % 256) + 256) % 256))
{
  return x ^ 255;
}

int xor_low_byte_nonneg(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == (x - x % 256) + (255 - x % 256))
{
  return x ^ 255;
}

unsigned or_low_byte_unsigned(unsigned u)
    CERASTES_ENSURES(r, r == u - u % 256 + 255)
{
  return u | 255;
}

unsigned xor_low_byte_unsigned(unsigned u)
    CERASTES_ENSURES(r, r == (u - u % 256) + (255 - u % 256))
{
  return u ^ 255;
}

unsigned or_assign(unsigned u)
    CERASTES_ENSURES(r, r == u - u % 256 + 255)
{
  unsigned w = u;
  w |= 255;
  return w;
}

unsigned xor_assign(unsigned u)
    CERASTES_ENSURES(r, r == u - u % 256 + 255)
{
  unsigned w = u;
  w ^= 255;
  return w;
}

int or_assign_signed(int x)
    CERASTES_ENSURES(r, r == x - ((x % 256) + 256) % 256 + 255)
{
  int w = x;
  w |= 255;
  return w;
}

int xor_assign_signed(int x)
    CERASTES_ENSURES(r, r == (x - ((x % 256) + 256) % 256) + (255 - ((x % 256) + 256) % 256))
{
  int w = x;
  w ^= 255;
  return w;
}
