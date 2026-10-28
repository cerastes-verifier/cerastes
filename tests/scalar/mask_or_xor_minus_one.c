// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0027 items 1 and 4: k = 0 for the complement: x ^ -1 and x ^ ~0 are -1 - x (the bits flipped), x | -1 is -1, and the unsigned x ^ ~0u and x | ~0u are the all-ones mask (UINT_MAX - u, UINT_MAX)
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
    CERASTES_ENSURES(r, r == 4294967295u - u)
{
  return u ^ ~0u;
}

unsigned or_not_zero_unsigned(unsigned u)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return u | ~0u;
}
