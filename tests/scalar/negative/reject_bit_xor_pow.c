// expect: rejected cerastes-xor_as_power @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_xor.c
// Pins: 0023 item 5: 2 ^ 32 is 34, not a power (all four warn)
#include <cerastes.h>

int big(void)
{
  return 2 ^ 32;
}
