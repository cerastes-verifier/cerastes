// expect: error shift @ L13
// mode: c23, c++20
// twin: tests/scalar/static_assert_nested.c
// Pins: 0025 item 5: an assertion in a block nested in a switch body is checked too
#include <cerastes.h>

int pick(int k)
    CERASTES_REQUIRES(0 <= k && k < 3)
{
  switch (k) {
  case 0: {
    {
      static_assert((1 << 31) != 0, "the shift does not fit int");
    }
    return 10;
  }
  default:
    break;
  }
  return 12;
}
