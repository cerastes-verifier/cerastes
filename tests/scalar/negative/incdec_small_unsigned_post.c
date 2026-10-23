// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/scalar/incdec_small_unsigned.c
// Pins: 0022 item 1: uc++ at 254 gives 255, not 254
#include <cerastes.h>

unsigned char inc_uc(unsigned char v)
    CERASTES_REQUIRES(v == 254)
    CERASTES_ENSURES(r, r == 254)
{
  v++;
  return v;
}
