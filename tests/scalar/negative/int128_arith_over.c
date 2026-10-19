// expect: error overflow @ L17
// mode: c23, c++20
// twin: tests/scalar/int128_arith.c
// Pins: 0010 item 3: __int128 addition may overflow
#include <cerastes.h>

__int128 widen(long a, long b)
    CERASTES_ENSURES(r, r == (__int128)a + b)
{
  return (__int128)a + b;
}

__int128 twice(__int128 a)
    CERASTES_REQUIRES(a >= 0)
    CERASTES_ENSURES(r, r >= a)
{
  return a + a;
}
