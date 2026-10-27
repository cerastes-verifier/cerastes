// expect: error postcondition @ L22
// mode: c23, c++20
// twin: tests/scalar/mask_near_miss.c
// Pins: 0026 item 7: 253 & 254 is 252, not 253
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
    CERASTES_ENSURES(r, r == 253)
{
  return and_254(253);
}

int and_256_of_511(void)
    CERASTES_ENSURES(r, r == 256)
{
  return and_256(511);
}
