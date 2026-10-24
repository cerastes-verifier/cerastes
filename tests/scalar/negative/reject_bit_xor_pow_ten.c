// expect: rejected cerastes-xor_as_power @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_xor.c
// Pins: 0023 item 5: 10 ^ 3 is 9, not 1000
#include <cerastes.h>

int thousand(void)
{
  return 10 ^ 3;
}
