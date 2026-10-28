// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0027 items 1 and 4: x | 255 sets and x ^ 255 flips the low byte for every x, signed and unsigned, and in a compound assignment; the claims use the non-negative remainder where x may be negative (C's % is negative there)
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
    CERASTES_ENSURES(r, r == (u - u % 256) + (255 - u % 256))
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
