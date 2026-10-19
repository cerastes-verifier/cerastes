// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 9: the postcondition is reported at the return that breaks it
#include <cerastes.h>

int clamp(int x, int lo, int hi)
    CERASTES_REQUIRES(lo <= hi)
    CERASTES_ENSURES(r, r >= lo && r <= hi)
{
  if (x < lo) {
    return lo;
  } else if (x > hi) {
    return x;
  }
  return x;
}
