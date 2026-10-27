// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/scalar/mask_remainder_clear.c
// Pins: 0026 item 9: for a negative x the cleared value is below x: x - x % 16 is 0 for x == -1 (C's % keeps the sign: -1 % 16 is -1), but x & ~15 is -16
#include <cerastes.h>

int clear_nibble(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == x - x % 16)
{
  return x & ~15;
}

int clear_nibble_any(int x)
    CERASTES_ENSURES(r, r == x - x % 16)
{
  return x & ~15;
}

unsigned clear_low_bits(unsigned u)
    CERASTES_ENSURES(r, r == u - u % 8u)
{
  return u & ~7u;
}
