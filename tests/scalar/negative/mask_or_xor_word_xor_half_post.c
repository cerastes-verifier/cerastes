// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/scalar/mask_or_xor_word.c
// Pins: 0027 item 1: v ^ 0xFFFFFFFF flips 32 bits, not 31 (v == 2147483648 gives 2147483647, not 4294967295)
#include <cerastes.h>

unsigned long long or_word(unsigned long long v)
    CERASTES_ENSURES(r, r == v - v % 4294967296 + 4294967295)
{
  return v | 0xFFFFFFFFu;
}

unsigned long long xor_word(unsigned long long v)
    CERASTES_ENSURES(r, r == (v - v % 2147483648) + (2147483647 - v % 2147483648))
{
  return v ^ 0xFFFFFFFFu;
}

unsigned long long or_word_complement(unsigned long long v)
    CERASTES_ENSURES(r, r == v - v % 4294967296 + 4294967295)
{
  // ~0u is the unsigned int 4294967295, so the mask sets 32 bits of a 64-bit value.
  return v | ~0u;
}

unsigned long long xor_word_left(unsigned long long v)
    CERASTES_ENSURES(r, r == (v - v % 4294967296) + (4294967295 - v % 4294967296))
{
  return 4294967295u ^ v;
}
