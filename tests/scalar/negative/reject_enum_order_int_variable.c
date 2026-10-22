// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_order.c
// Pins: 0019 item 10: ordering against an int variable is rejected
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int below(enum color c, int n)
{
  if (c <= n) {
    return 1;
  }
  return 0;
}
