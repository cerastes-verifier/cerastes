// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0026 items 7 to 9: a mask by a complement ~(2^k - 1) clears the low k bits: x - x % 2^k for x >= 0, and with the non-negative remainder for every x; signed and unsigned
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
    CERASTES_ENSURES(r, r == u - u % 8u)
{
  return u & ~7u;
}
