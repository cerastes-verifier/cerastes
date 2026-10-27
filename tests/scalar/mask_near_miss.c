// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0026 items 7 and 8: a near miss (254, 256) is not a remainder mask and keeps its bit encoding; with closed operands it is decided by evaluation
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
    CERASTES_ENSURES(r, r == 256)
{
  return and_256(511);
}
