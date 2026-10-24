// expect: rejected cerastes-tautological_bitwise_compare @ L8
// mode: c23, c++20
// twin: tests/scalar/bit_contract.c
// Pins: 0023 item 5: x | 4 as the whole clause of an ensures
#include <cerastes.h>

int pick(int x)
    CERASTES_ENSURES(r, x | 4)
{
  return x;
}
