// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 6: us *= us is computed in int (and converts back, wrapping)
// With both at most 100 the product is at most 10000 and fits unsigned short.
#include <cerastes.h>

unsigned short mul_us(unsigned short a, unsigned short b)
    CERASTES_REQUIRES(a <= 100 && b <= 100)
    CERASTES_ENSURES(r, r == a * b)
{
  a *= b;
  return a;
}
