// expect: error postcondition @ L10
// mode: c23, c++20
// twin: tests/scalar/mask_or_xor_round_trip.c
// Pins: 0027 item 1: one ^ 255 is not the round trip (x == 0 gives 255, not 0)
#include <cerastes.h>

int xor_round_trip(int x)
    CERASTES_ENSURES(r, r == (x ^ 255))
{
  return (x ^ 255) ^ 255;
}
