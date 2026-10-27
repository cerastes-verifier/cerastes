// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0026 items 7 and 8: the same rule at 64 and 128 bits: the full 64-bit masks, the low 64 bits of an unsigned __int128, and a cleared nibble of a signed __int128
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
    CERASTES_ENSURES(r, r == x - x % 16)
{
  return x & ~15;
}
