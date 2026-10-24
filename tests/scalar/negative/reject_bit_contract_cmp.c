// expect: rejected cerastes-bitwise_operand_parentheses @ L8
// mode: c23, c++20
// twin: tests/scalar/bit_contract.c
// Pins: 0023 item 5: the shapes apply in contracts too: a comparison under & in a requires
#include <cerastes.h>

int mask_is(int a, int b)
    CERASTES_REQUIRES(a & b == 0)
{
  return a & b;
}
