// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/scalar/unsigned_compound_mixed.c
// Pins: 0022 item 1: uc -= 1u at 1 gives 0, not 1
#include <cerastes.h>

unsigned char sub_uchar(unsigned char uc)
    CERASTES_REQUIRES(uc == 1)
    CERASTES_ENSURES(r, r == 1)
{
  uc -= 1u;
  return uc;
}
