// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0027 item 1: the mask on the left, 255 | x and 255 ^ x, signed and unsigned: the same remainders as the mask on the right
#include <cerastes.h>

int or_left(int x)
    CERASTES_ENSURES(r, r == x - ((x % 256) + 256) % 256 + 255)
{
  return 255 | x;
}

int xor_left(int x)
    CERASTES_ENSURES(r, r == (x - ((x % 256) + 256) % 256) + (255 - ((x % 256) + 256) % 256))
{
  return 255 ^ x;
}

int or_left_nonneg(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == x - x % 256 + 255)
{
  return 255 | x;
}

int xor_left_nonneg(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == (x - x % 256) + (255 - x % 256))
{
  return 255 ^ x;
}

unsigned or_left_unsigned(unsigned u)
    CERASTES_ENSURES(r, r == u - u % 256 + 255)
{
  return 255 | u;
}

unsigned xor_left_unsigned(unsigned u)
    CERASTES_ENSURES(r, r == (u - u % 256) + (255 - u % 256))
{
  return 255 ^ u;
}
