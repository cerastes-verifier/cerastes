// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_order.c
// Pins: 0019 item 10: the integer on the left is rejected too
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int above_two(enum color c)
{
  if (2 > c) {
    return 1;
  }
  return 0;
}
