// expect: rejected cerastes-tautological_bitwise_compare @ L10
// mode: c23, c++20
// twin: tests/scalar/bit_contract.c
// Pins: 0023 item 5: x | 4 as the whole clause of a loop invariant
#include <cerastes.h>

int count(int x)
{
  int i = 0;
  CERASTES_INVARIANT(x | 4)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i += 1;
  }
  return i;
}
