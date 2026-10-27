// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/mask_remainder_clear.c
// Pins: 0026 item 9: x & ~15 is x - x % 16, not x - x % 8 (x == 8 refutes it)
#include <cerastes.h>

int clear_nibble(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == x - x % 8)
{
  return x & ~15;
}

int clear_nibble_any(int x)
    CERASTES_ENSURES(r, r == x - ((x % 16) + 16) % 16)
{
  return x & ~15;
}

unsigned clear_low_bits(unsigned u)
    CERASTES_ENSURES(r, r == u - u % 8u)
{
  return u & ~7u;
}
