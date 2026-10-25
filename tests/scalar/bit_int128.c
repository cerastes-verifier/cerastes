// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0023 decision: 128-bit operands are exact (and the high half is reached): __int128 and unsigned __int128, constants at 2^100 and 2^127
#include <cerastes.h>

__int128 and_wide(__int128 a, __int128 b)
    CERASTES_ENSURES(r, r == (a & b))
{
  return a & b;
}

__int128 or_wide(__int128 a, __int128 b)
    CERASTES_ENSURES(r, r == (a | b))
{
  return a | b;
}

__int128 xor_wide(__int128 a, __int128 b)
    CERASTES_ENSURES(r, r == (a ^ b))
{
  return a ^ b;
}

__int128 not_wide(__int128 a)
    CERASTES_ENSURES(r, r == -1 - a)
{
  return ~a;
}

unsigned __int128 and_uwide(unsigned __int128 a, unsigned __int128 b)
    CERASTES_ENSURES(r, r == (a & b))
{
  return a & b;
}

__int128 and_wide_value(void)
    CERASTES_ENSURES(r, r == 5)
{
  return and_wide(13, 7);
}

__int128 not_wide_zero(void)
    CERASTES_ENSURES(r, r == -1)
{
  return not_wide(0);
}

// The high half: -1 has every bit set, so the mask returns the bit at position 100.
__int128 and_wide_high(void)
    CERASTES_ENSURES(r, r == ((__int128)1 << 100))
{
  return and_wide(-1, (__int128)1 << 100);
}

// All ones masked with the top bit gives the top bit of an unsigned 128-bit value.
unsigned __int128 and_uwide_high(void)
    CERASTES_ENSURES(r, r == ((unsigned __int128)1 << 127))
{
  return and_uwide(~(unsigned __int128)0, (unsigned __int128)1 << 127);
}
