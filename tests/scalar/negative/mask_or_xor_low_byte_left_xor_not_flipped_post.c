// expect: error postcondition @ L30
// mode: c23, c++20
// twin: tests/scalar/mask_or_xor_low_byte_left.c
// Pins: 0027 item 1: the low byte of 255 ^ x is flipped, not kept (x == 0 gives 255, not 0)
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
    CERASTES_ENSURES(r, r == (x - x % 256) + x % 256)
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
