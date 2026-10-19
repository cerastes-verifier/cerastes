// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/unsigned_neg.c
// Pins: 0010 item 10: -1u is UINT_MAX, not 1
#include <cerastes.h>

unsigned negate_u(unsigned a)
    CERASTES_REQUIRES(a == 1u)
    CERASTES_ENSURES(r, r == 1u)
{
  return -a;
}
