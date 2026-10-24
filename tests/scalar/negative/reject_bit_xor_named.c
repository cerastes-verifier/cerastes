// expect: rejected cerastes-xor_as_power @ L7
// mode: c23, c++20
// twin: tests/scalar/bit_xor.c
// Pins: 0023 item 5: 2 ^ 8 in a named constant's initializer (checked in the prologue, 0017): the detectors run where the initializer is encoded
#include <cerastes.h>

const int K = 2 ^ 8;

int get(void)
{
  return K;
}
