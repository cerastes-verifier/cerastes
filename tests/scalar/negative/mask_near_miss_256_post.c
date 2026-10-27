// expect: error postcondition @ L28
// mode: c23, c++20
// twin: tests/scalar/mask_near_miss.c
// Pins: 0026 item 7: 511 & 256 is 256, not 0
#include <cerastes.h>

int and_254(int x)
    CERASTES_ENSURES(r, r == (x & 254))
{
  return x & 254;
}

int and_256(int x)
    CERASTES_ENSURES(r, r == (x & 256))
{
  return x & 256;
}

int and_254_of_253(void)
    CERASTES_ENSURES(r, r == 252)
{
  return and_254(253);
}

int and_256_of_511(void)
    CERASTES_ENSURES(r, r == 0)
{
  return and_256(511);
}
