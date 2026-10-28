// expect: error postcondition @ L10
// mode: c23, c++20
// twin: tests/scalar/mask_or_xor_complement.c
// Pins: 0027 item 1: x | ~15 is L - 16 for L the low nibble, not L - 15 (x == 0 gives -16)
#include <cerastes.h>

int or_complement(int x)
    CERASTES_ENSURES(r, r == ((x % 16) + 16) % 16 - 15)
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
