// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/usual_conversions.c
// Pins: 0010 item 6: the sum is 0, not 2
#include <cerastes.h>

unsigned mix_add(int i, unsigned u)
    CERASTES_REQUIRES(i == -1 && u == 1u)
    CERASTES_ENSURES(r, r == 2u)
{
  return i + u;
}
