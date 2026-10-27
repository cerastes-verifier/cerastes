// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/mask_remainder_signed_char.c
// Pins: 0026 items 7 and 8 (pin): w &= 0xFF on a signed char w == -1 is 255, which does not fit: the conversion back fails (a rewrite that kept the value of w would verify)
#include <cerastes.h>

signed char low_byte_char(signed char sc)
{
  signed char w = sc;
  w &= 0xFF;
  return w;
}
