// expect: error overflow @ L10
// mode: c23, c++20
// twin: tests/scalar/compound_ushort.c
// Pins: 0010 item 6: 65535 * 65535 overflows the int the product is computed in
// The requires makes the overflow certain: no path survives it, so no conversion error follows (compound_ushort_conversion.c).
#include <cerastes.h>
unsigned short mul_us(unsigned short a, unsigned short b)
    CERASTES_REQUIRES(a == 65535 && b == 65535)
{
  a *= b;
  return a;
}
