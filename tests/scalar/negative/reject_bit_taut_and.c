// expect: rejected cerastes-tautological_bitwise_compare @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_and.c
// Pins: 0023 item 5: (x & 2) == 1 is never true (both compilers warn)
#include <cerastes.h>

int never(int x)
{
  return (x & 2) == 1;
}
