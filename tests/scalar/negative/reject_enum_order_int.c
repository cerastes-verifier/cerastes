// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_order.c
// Pins: 0019 item 10: ordering an enumeration value against another integer is rejected, even a constant
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int below_two(enum color c)
{
  if (c < 2) {
    return 1;
  }
  return 0;
}
