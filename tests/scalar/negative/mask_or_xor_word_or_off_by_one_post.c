// expect: error postcondition @ L10
// mode: c23, c++20
// twin: tests/scalar/mask_or_xor_word.c
// Pins: 0027 item 1: v | 0xFFFFFFFF sets all 32 low bits (v == 0 gives 4294967295, not 4294967294)
#include <cerastes.h>

unsigned long long or_word(unsigned long long v)
    CERASTES_ENSURES(r, r == v - v % 4294967296 + 4294967294)
{
  return v | 0xFFFFFFFFu;
}

unsigned long long xor_word(unsigned long long v)
    CERASTES_ENSURES(r, r == (v - v % 4294967296) + (4294967295 - v % 4294967296))
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
