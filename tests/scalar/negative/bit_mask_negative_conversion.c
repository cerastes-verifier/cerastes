// expect: error conversion @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_mask.c
// Pins: 0023 item 3: u & -8 converts the negative int -8 to unsigned int (the rewrite is u & ~7u)
#include <cerastes.h>

unsigned mask_down(unsigned u)
{
  return u & -8;
}
