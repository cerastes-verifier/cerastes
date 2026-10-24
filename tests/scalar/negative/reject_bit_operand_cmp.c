// expect: rejected cerastes-bitwise_operand_parentheses @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_and.c
// Pins: 0023 item 5: a comparison as an unparenthesized operand of & (both compilers warn)
#include <cerastes.h>

int mask_is(int a, int b, int c)
{
  return a & b == c;
}
