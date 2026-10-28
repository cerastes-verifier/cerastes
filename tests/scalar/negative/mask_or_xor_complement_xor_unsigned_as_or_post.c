// expect: error postcondition @ L34
// mode: c23, c++20
// twin: tests/scalar/mask_or_xor_complement.c
// Pins: 0027 item 1: u ^ ~0xFFu flips the high 24 bits, u | ~0xFFu sets them (u == 256 gives 4294966784, not 4294967040)
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
    CERASTES_ENSURES(r, r == u % 256 + 4294967040u)
{
  return u ^ ~0xFFu;
}
