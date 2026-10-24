// expect: rejected cerastes-tautological_bitwise_compare @ L10
// mode: c23, c++20
// twin: tests/scalar/bit_contract.c
// Pins: 0023 item 5: x | 4 as the condition of assert (checked like check, 0015 item 6)
#include <assert.h>
#include <cerastes.h>

int pick(int x)
{
  assert(x | 4);
  return x;
}
