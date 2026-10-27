// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0026 items 7 to 9: a 64-bit value masked by 0xFFFFFFFF is its remainder modulo 2^32, also through the conversion to unsigned
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
    CERASTES_ENSURES(r, r == v % 4294967296)
{
  return v & ~0u;
}
