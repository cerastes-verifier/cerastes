// expect: error postcondition @ L34
// mode: c23, c++20
// twin: tests/scalar/bit_int128.c
// Pins: 0023 decision: 12 & 10 is 8 in 128 bits too, not 9
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

__int128 and_wide_value(void)
    CERASTES_ENSURES(r, r == 9)
{
  return and_wide(12, 10);
}

__int128 not_wide_zero(void)
    CERASTES_ENSURES(r, r == -1)
{
  return not_wide(0);
}
