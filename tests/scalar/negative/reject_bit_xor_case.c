// expect: rejected cerastes-xor_as_power @ L10
// mode: c23, c++20
// twin: tests/scalar/bit_xor.c
// Pins: 0023 item 5: 2 ^ 8 as a case value (encoded by its original expression, 0014 item 6)
#include <cerastes.h>

int pick(int x)
{
  switch (x) {
    case 2 ^ 8:
      return 1;
    default:
      return 0;
  }
}
