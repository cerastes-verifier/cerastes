// expect: error overflow @ L10
// mode: c23, c++20
// twin: tests/scalar/compound_ushort.c
// Pins: 0010 item 6: 65535 * 65535 overflows the int the product is computed in
// A faithful consequence of the promotions, documented on the Integer semantics page.
#include <cerastes.h>

unsigned short mul_us(unsigned short a, unsigned short b)
{
  a *= b;
  return a;
}
