// expect: error postcondition @ L29
// mode: c23, c++20
// twin: tests/scalar/mask_remainder_wide.c
// Pins: 0026 item 8: x & ~15 is x - x % 16 for a 128-bit x too, not x - x % 8 (x == 8 refutes it)
#include <cerastes.h>

unsigned long long and_u64_max(unsigned long long v)
    CERASTES_ENSURES(r, r == v)
{
  return v & 18446744073709551615ull;
}

long long and_i64_max(long long x)
    CERASTES_ENSURES(r, (x >= 0 && r == x) || (x < 0 && r == x + 9223372036854775807ll + 1))
{
  return x & 9223372036854775807ll;
}

unsigned __int128 and_u128_low64(unsigned __int128 w)
    CERASTES_ENSURES(r, r == w % ((unsigned __int128)1 << 64))
{
  return w & 18446744073709551615ull;
}

__int128 clear_nibble_128(__int128 x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == x - x % 8)
{
  return x & ~15;
}
