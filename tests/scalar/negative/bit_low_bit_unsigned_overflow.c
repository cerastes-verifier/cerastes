// expect: error overflow @ L10
// mode: c23, c++20
// twin: tests/scalar/bit_and.c
// Pins: 0023 item 3 and 0022 item 1: for an unsigned int x, -x overflows for x > 0 (x & -x is the idiom for int only)
#include <cerastes.h>

unsigned low_bit_uns(unsigned x)
    CERASTES_REQUIRES(0u < x)
{
  return x & -x;
}
