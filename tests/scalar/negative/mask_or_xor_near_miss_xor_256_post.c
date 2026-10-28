// expect: error postcondition @ L28
// mode: c23, c++20
// twin: tests/scalar/mask_or_xor_near_miss.c
// Pins: 0027 item 1: 511 ^ 256 is 255, not 511 (256 is not a mask)
#include <cerastes.h>

int or_254(int x)
    CERASTES_ENSURES(r, r == (x | 254))
{
  return x | 254;
}

int xor_256(int x)
    CERASTES_ENSURES(r, r == (x ^ 256))
{
  return x ^ 256;
}

int or_254_of_253(void)
    CERASTES_ENSURES(r, r == 255)
{
  return or_254(253);
}

int xor_256_of_511(void)
    CERASTES_ENSURES(r, r == 511)
{
  return xor_256(511);
}

int or_254_of_1(void)
    CERASTES_ENSURES(r, r == 255)
{
  return or_254(1);
}

int xor_256_of_255(void)
    CERASTES_ENSURES(r, r == 511)
{
  return xor_256(255);
}
