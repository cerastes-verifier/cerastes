// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0027 items 1 and 4: | and ^ with a constant mask on the result of the other and of &: (x | 255) ^ 255 clears the low byte, (x ^ 255) | 255 and (x & ~255) ^ 255 set it
#include <cerastes.h>

int or_then_xor(int x)
    CERASTES_ENSURES(r, r == (x & ~255))
{
  return (x | 255) ^ 255;
}

int xor_then_or(int x)
    CERASTES_ENSURES(r, r == (x | 255))
{
  return (x ^ 255) | 255;
}

int clear_then_xor(int x)
    CERASTES_ENSURES(r, r == (x | 255))
{
  return (x & ~255) ^ 255;
}

unsigned unsigned_chain(unsigned u)
    CERASTES_ENSURES(r, r == (u | 4095u))
{
  return ((u & ~4095u) ^ 15) | 4095u;
}
