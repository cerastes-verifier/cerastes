// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/scalar/unsigned_incdec_range.c
// Pins: 0022 item 1: u-- at 0 is an overflow at the operator
#include <cerastes.h>

unsigned dec(unsigned u)
    CERASTES_REQUIRES(u == 0u)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  u--;
  return u;
}
