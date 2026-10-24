// expect: rejected cerastes-bitwise_operand_parentheses @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_xor.c
// Pins: 0023 item 5: & under | without parentheses (both compilers warn)
#include <cerastes.h>

int merge(int a, int b, int c)
{
  return a | b & c;
}
