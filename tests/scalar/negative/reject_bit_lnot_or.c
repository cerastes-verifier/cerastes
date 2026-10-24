// expect: rejected cerastes-logical_not_bitwise @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_or.c
// Pins: 0023 item 5: !x | 2 (GCC warns; only the literals 0 and 1 are silent)
#include <cerastes.h>

int test(int x)
{
  return !x | 2;
}
