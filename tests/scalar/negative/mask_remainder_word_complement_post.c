// expect: error postcondition @ L23
// mode: c23, c++20
// twin: tests/scalar/mask_remainder_word.c
// Pins: 0026 item 7 (pin): ~0u in a 64-bit operation is 2^32 - 1, so v & ~0u is the low 32 bits of v and not v (v == 2^32 refutes it); a mask evaluated in the operand's own type would be all ones
#include <cerastes.h>

unsigned long long low_half(unsigned long long v)
    CERASTES_ENSURES(r, r == v % 4294967296)
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
    CERASTES_ENSURES(r, r == v)
{
  return v & ~0u;
}
