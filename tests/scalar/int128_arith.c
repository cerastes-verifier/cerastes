// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 2: __int128 is a supported type (exact constants, P9)
// Two 64-bit values added in 128 bits.
#include <cerastes.h>

__int128 widen(long a, long b)
    CERASTES_ENSURES(r, r == (__int128)a + b)
{
  return (__int128)a + b;
}
