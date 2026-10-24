// expect: error conversion @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_promoted.c
// Pins: 0023 item 3: uc = ~uc converts a negative int to unsigned char
#include <cerastes.h>

unsigned char flip(unsigned char uc)
{
  uc = ~uc;
  return uc;
}
