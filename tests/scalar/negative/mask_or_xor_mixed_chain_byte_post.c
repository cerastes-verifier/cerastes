// expect: error postcondition @ L28
// mode: c23, c++20
// twin: tests/scalar/mask_or_xor_mixed.c
// Pins: 0027 item 1: the chain ends with | 4095u, so it sets 12 bits, not 8 (u == 0 gives 4095, not 255)
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
    CERASTES_ENSURES(r, r == (u | 255u))
{
  return ((u & ~4095u) ^ 15) | 4095u;
}
