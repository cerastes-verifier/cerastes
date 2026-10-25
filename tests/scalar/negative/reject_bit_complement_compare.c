// expect: rejected cerastes-tautological_bitwise_compare @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_not.c
// Pins: 0023 item 5 (clause 6c): ~uc is a negative int, so ~uc == 15 is never true (GCC -Wsign-compare)
#include <cerastes.h>

int never(unsigned char uc)
{
  return ~uc == 15;
}
