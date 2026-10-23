// expect: error conversion @ L11
// mode: c23, c++20
// twin: tests/scalar/incdec_small_unsigned.c
// Pins: 0022 item 1: uc++ at 255 computes 256 in int; converting it back does not fit unsigned char
#include <cerastes.h>

unsigned char inc_uc(unsigned char v)
    CERASTES_REQUIRES(v == 255)
    CERASTES_ENSURES(r, r == 0)
{
  v++;
  return v;
}
