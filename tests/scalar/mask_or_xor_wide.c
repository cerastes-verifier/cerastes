// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0027 items 1 and 4: the same rule at 64 and 128 bits: the all-ones mask of a 64-bit value, the low 64 bits of an unsigned __int128, a complement of a signed long long and of a signed __int128, and INT64_MAX as the mask
#include <cerastes.h>

unsigned long long or_u64_all(unsigned long long v)
    CERASTES_ENSURES(r, r == 18446744073709551615ull)
{
  return v | 18446744073709551615ull;
}

unsigned long long xor_u64_all(unsigned long long v)
    CERASTES_ENSURES(r, r == 18446744073709551615ull - v)
{
  return v ^ 18446744073709551615ull;
}

unsigned __int128 or_u128_low64(unsigned __int128 w)
    CERASTES_ENSURES(r, r == w - w % ((unsigned __int128)1 << 64) + 18446744073709551615ull)
{
  return w | 18446744073709551615ull;
}

unsigned __int128 xor_u128_low64(unsigned __int128 w)
    CERASTES_ENSURES(r, r == (w - w % ((unsigned __int128)1 << 64)) + (18446744073709551615ull - w % ((unsigned __int128)1 << 64)))
{
  return w ^ 18446744073709551615ull;
}

long long or_i64_complement(long long x)
    CERASTES_ENSURES(r, r == ((x % 16) + 16) % 16 - 16)
{
  return x | ~15ll;
}

long long xor_i64_max(long long x)
    CERASTES_ENSURES(r, (x >= 0 && r == 9223372036854775807ll - x) || (x < 0 && r == (-1 - x) - 9223372036854775807ll - 1))
{
  return x ^ 9223372036854775807ll;
}

__int128 or_i128_complement(__int128 x)
    CERASTES_ENSURES(r, r == ((x % 16) + 16) % 16 - 16)
{
  return x | ~15;
}

__int128 xor_i128_complement(__int128 x)
    CERASTES_ENSURES(r, r == 2 * (((x % 16) + 16) % 16) - 16 - x)
{
  return x ^ ~15;
}
