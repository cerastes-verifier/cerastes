// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_order.c
// Pins: 0019 item 10: c < -1 is true in C (unsigned int) and false in C++ (int), so it is rejected
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int below_minus_one(enum color c)
{
  if (c < -1) {
    return 1;
  }
  return 0;
}
