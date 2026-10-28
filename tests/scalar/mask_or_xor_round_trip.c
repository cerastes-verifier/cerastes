// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0027 items 1, 3 and 4: applying the same constant mask twice gives x back, four masks that cancel in adjacent pairs give x back, an eight-mask chain keeps a non-negative x non-negative (a range claim: an identity through a deep chain may be beyond what Z3 decides, item 4), and a nested ^ chain keeps the bits above the masks
#include <cerastes.h>

int xor_round_trip(int x)
    CERASTES_ENSURES(r, r == x)
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
