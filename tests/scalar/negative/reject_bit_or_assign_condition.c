// expect: rejected cerastes-assignment_as_condition @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_compound.c
// Pins: 0023 item 5, extension: if (x |= a) (clang -Wparentheses)
#include <cerastes.h>

int test(int x, int a)
{
  if (x |= a) {
    return x;
  }
  return 0;
}
