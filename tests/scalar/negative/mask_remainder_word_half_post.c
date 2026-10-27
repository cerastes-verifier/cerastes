// expect: error postcondition @ L10
// mode: c23, c++20
// twin: tests/scalar/mask_remainder_word.c
// Pins: 0026 item 9: v & 0xFFFFFFFF is v % 2^32, not v % 2^31 (v == 2^31 refutes it)
#include <cerastes.h>

unsigned long long low_half(unsigned long long v)
    CERASTES_ENSURES(r, r == v % 2147483648)
{
  return v & 0xFFFFFFFFu;
}

unsigned low_word(unsigned long long v)
    CERASTES_ENSURES(r, r == v % 4294967296)
{
  return (unsigned)(v & 4294967295u);
}

// ~0u is the unsigned int 4294967295, so the mask keeps 32 bits of a 64-bit value.
unsigned long long low_half_complement(unsigned long long v)
    CERASTES_ENSURES(r, r == v % 4294967296)
{
  return v & ~0u;
}
