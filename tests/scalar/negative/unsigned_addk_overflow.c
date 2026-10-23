// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/scalar/unsigned_incdec_range.c
// Pins: 0022 item 1: u += k whose sum exceeds UINT_MAX is an overflow
#include <cerastes.h>

unsigned add_k(unsigned u, unsigned k)
    CERASTES_REQUIRES(u == 4294967291u && k == 5u)
    CERASTES_ENSURES(r, r == 0u)
{
  u += k;
  return u;
}
