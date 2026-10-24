// expect: rejected cerastes-logical_not_bitwise @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_and.c
// Pins: 0023 item 5: !x & 1 (clang warns on & with any right operand that is not a truth value)
#include <cerastes.h>

int test(int x)
{
  return !x & 1;
}
