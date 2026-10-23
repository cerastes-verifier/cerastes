// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/conv_unsigned_sites.c
// Pins: 0022 item 1: a cast of 0 <= x <= 1000 keeps its value, so r == x + 1 is wrong
#include <cerastes.h>

unsigned from_cast(int x)
    CERASTES_REQUIRES(0 <= x && x <= 1000)
    CERASTES_ENSURES(r, r == (unsigned)x + 1u)
{
  return (unsigned)x;
}
