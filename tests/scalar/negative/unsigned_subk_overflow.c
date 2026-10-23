// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/scalar/unsigned_incdec_range.c
// Pins: 0022 item 1: u -= k below zero is an overflow
#include <cerastes.h>

unsigned sub_k(unsigned u, unsigned k)
    CERASTES_REQUIRES(u == 3u && k == 4u)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  u -= k;
  return u;
}
