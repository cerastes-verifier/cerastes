// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0027 items 1 and 4: v | 0xFFFFFFFF sets and v ^ 0xFFFFFFFF flips the low 32 bits of an unsigned 64-bit v, with the mask written 0xFFFFFFFFu, as ~0u and on the left
#include <cerastes.h>

unsigned long long or_word(unsigned long long v)
    CERASTES_ENSURES(r, r == v - v % 4294967296 + 4294967295)
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
