// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0027 items 1 and 4: x | ~15 and x ^ ~15 keep the low four bits and set or flip every higher one; u | ~0xFFu and u ^ ~0xFFu do the same in u's own unsigned type
#include <cerastes.h>

int or_complement(int x)
    CERASTES_ENSURES(r, r == ((x % 16) + 16) % 16 - 16)
{
  return x | ~15;
}

int xor_complement(int x)
    CERASTES_ENSURES(r, r == 2 * (((x % 16) + 16) % 16) - 16 - x)
{
  return x ^ ~15;
}

int or_complement_left(int x)
    CERASTES_ENSURES(r, r == ((x % 16) + 16) % 16 - 16)
{
  return ~15 | x;
}

unsigned or_complement_unsigned(unsigned u)
    CERASTES_ENSURES(r, r == u % 256 + 4294967040u)
{
  return u | ~0xFFu;
}

unsigned xor_complement_unsigned(unsigned u)
    CERASTES_ENSURES(r, r == (4294967040u - (u - u % 256)) + u % 256)
{
  return u ^ ~0xFFu;
}
