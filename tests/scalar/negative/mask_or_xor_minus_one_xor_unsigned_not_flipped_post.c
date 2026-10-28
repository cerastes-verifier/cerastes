// expect: error postcondition @ L40
// mode: c23, c++20
// twin: tests/scalar/mask_or_xor_minus_one.c
// Pins: 0027 item 1: u ^ ~0u is UINT_MAX - u, not u (u == 0 gives UINT_MAX)
#include <cerastes.h>

int xor_minus_one(int x)
    CERASTES_ENSURES(r, r == -1 - x)
{
  return x ^ -1;
}

int xor_not_zero(int x)
    CERASTES_ENSURES(r, r == -1 - x)
{
  return x ^ ~0;
}

int xor_minus_one_left(int x)
    CERASTES_ENSURES(r, r == -1 - x)
{
  return -1 ^ x;
}

int or_minus_one(int x)
    CERASTES_ENSURES(r, r == -1)
{
  return x | -1;
}

int or_not_zero(int x)
    CERASTES_ENSURES(r, r == -1)
{
  return x | ~0;
}

unsigned xor_not_zero_unsigned(unsigned u)
    CERASTES_ENSURES(r, r == u)
{
  return u ^ ~0u;
}

unsigned or_not_zero_unsigned(unsigned u)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return u | ~0u;
}
