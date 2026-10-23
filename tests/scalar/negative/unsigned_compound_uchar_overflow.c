// expect: error overflow @ L10
// mode: c23, c++20
// twin: tests/scalar/unsigned_compound_mixed.c
// Pins: 0022 item 1: uc -= 1u at 0 is computed in unsigned int: an overflow, not a conversion
#include <cerastes.h>

unsigned char sub_uchar(unsigned char uc)
    CERASTES_REQUIRES(uc == 0)
{
  uc -= 1u;
  return uc;
}
