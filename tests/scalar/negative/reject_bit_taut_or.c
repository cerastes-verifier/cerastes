// expect: rejected cerastes-tautological_bitwise_compare @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_or.c
// Pins: 0023 item 5: if (x | 4) is always true (clang warns)
#include <cerastes.h>

int always(int x)
{
  if (x | 4) {
    return 1;
  }
  return 0;
}
