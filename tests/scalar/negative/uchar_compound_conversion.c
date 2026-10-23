// expect: error conversion @ L11
// mode: c23, c++20
// twin: tests/scalar/incdec_small_unsigned.c
// Pins: 0022 item 1: uc += 1 at 255 is a conversion error (computed in int)
#include <cerastes.h>

unsigned char add_uc(unsigned char v)
    CERASTES_REQUIRES(v == 255)
    CERASTES_ENSURES(r, r == 0)
{
  v += 1;
  return v;
}
