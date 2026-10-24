// expect: rejected cerastes-bitwise_operand_parentheses @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_and.c
// Pins: 0023 item 5: two unparenthesized comparisons under & (GCC warns, clang does not: the union)
#include <cerastes.h>

int both(int a, int b, int c, int d)
{
  return a == b & c == d;
}
