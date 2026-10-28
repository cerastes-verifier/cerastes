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

unsigned xor_round_trip_unsigned(unsigned u)
    CERASTES_ENSURES(r, r == u)
{
  return (u ^ 4294967295u) ^ 4294967295u;
}

int xor_round_trip_complement(int x)
    CERASTES_ENSURES(r, r == x)
{
  return (x ^ ~15) ^ ~15;
}

int xor_chain(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r / 256 == x / 256)
{
  return ((x ^ 255) ^ 15) ^ 3;
}

int xor_chain_value(void)
    CERASTES_ENSURES(r, r == 151)
{
  int x = 100;
  return ((x ^ 255) ^ 15) ^ 3;
}

int xor_chain4(int x)
    CERASTES_ENSURES(r, r == x)
{
  return (((x ^ 255) ^ 255) ^ 15) ^ 15;
}

int xor_chain8_range(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, 0 <= r)
{
  return ((((((((x ^ 255) ^ 15) ^ 255) ^ 15) ^ 255) ^ 15) ^ 255) ^ 15);
}
