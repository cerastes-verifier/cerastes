// expect: error postcondition @ L23
// mode: c23, c++20
// twin: tests/scalar/mask_remainder_clear.c
// Pins: 0026 item 9: u & ~7u is u - u % 8, not u - u % 4 (u == 4 refutes it)
#include <cerastes.h>

int clear_nibble(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == x - x % 16)
{
  return x & ~15;
}

int clear_nibble_any(int x)
    CERASTES_ENSURES(r, r == x - ((x % 16) + 16) % 16)
{
  return x & ~15;
}

unsigned clear_low_bits(unsigned u)
    CERASTES_ENSURES(r, r == u - u % 4u)
{
  return u & ~7u;
}
