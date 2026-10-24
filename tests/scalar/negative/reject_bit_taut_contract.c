// expect: rejected cerastes-tautological_bitwise_compare @ L8
// mode: c23, c++20
// twin: tests/scalar/bit_contract.c
// Pins: 0023 item 5: a never-true bitwise comparison in a requires (the compilers see nothing in a contract, the rule applies)
#include <cerastes.h>

int pick(int x)
    CERASTES_REQUIRES((x & 2) == 1)
{
  return x;
}
