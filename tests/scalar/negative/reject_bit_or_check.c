// expect: rejected cerastes-tautological_bitwise_compare @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_contract.c
// Pins: 0023 item 5: x | 4 as the whole condition of a check
#include <cerastes.h>

int pick(int x)
{
  CERASTES_CHECK(x | 4);
  return x;
}
