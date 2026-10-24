// expect: rejected cerastes-xor_as_power @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_xor.c
// Pins: 0023 item 5: 0x2 ^ 8, silent in every compiler, is rejected by value (the owner's broader shape)
#include <cerastes.h>

int big(void)
{
  return 0x2 ^ 8;
}
