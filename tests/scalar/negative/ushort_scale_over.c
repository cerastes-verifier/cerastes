// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/scalar/ushort_scale.c
// Pins: 0010 item 6: unsigned short * int overflows int
#include <cerastes.h>

int scale(unsigned short a)
    CERASTES_REQUIRES(a <= 32769)
    CERASTES_ENSURES(r, r >= 0)
{
  return a * 65535;
}
