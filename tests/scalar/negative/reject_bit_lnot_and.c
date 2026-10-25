// expect: rejected cerastes-logical_not_bitwise @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_and.c
// Pins: 0023 item 5: !x & y (both compilers warn)
#include <cerastes.h>

int test(int x, int y)
{
  return !x & y;
}
