// expect: rejected cerastes-bitwise_operand_parentheses @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_or.c
// Pins: 0023 item 5: (1 << 4) | 1 == 17 is folded by the front end (EDG keeps the original expression: P40); the rule reads it there
#include <cerastes.h>

int folded(void)
{
  return (1 << 4) | 1 == 17;
}
