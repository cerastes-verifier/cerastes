// expect: error conversion @ L11
// mode: c23, c++20
// twin: tests/scalar/conv_unsigned_sites.c
// Pins: 0022 item 1: u += x with x == -1 converts x to unsigned: a conversion error at the compound assignment
#include <cerastes.h>

unsigned from_compound(int x)
    CERASTES_REQUIRES(x == -1)
{
  unsigned u = 5u;
  u += x;
  return u;
}
