// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/compound_ushort.c
// Pins: 0010 item 6 and 0022: 256 * 256 fits int, but converting 65536 back to unsigned short does not fit
#include <cerastes.h>

unsigned short mul_us(unsigned short a, unsigned short b)
    CERASTES_REQUIRES(a == 256 && b == 256)
{
  a *= b;
  return a;
}
