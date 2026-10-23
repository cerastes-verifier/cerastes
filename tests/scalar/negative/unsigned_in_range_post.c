// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/unsigned_in_range.c
// Pins: 0022 item 1: the sum is exact (4294967295u), not 4294967294u
#include <cerastes.h>

unsigned add_u(unsigned a)
    CERASTES_REQUIRES(a == 4294967294u)
    CERASTES_ENSURES(r, r == 4294967294u)
{
  return a + 1u;
}
