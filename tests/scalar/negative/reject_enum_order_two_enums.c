// expect: rejected cerastes-unsupported_expression @ L12
// mode: c23, c++20
// twin: tests/scalar/enum_order.c
// Pins: 0019 item 10: values of two different enumerations are not ordered
#include <cerastes.h>

enum color { RED, GREEN, BLUE };
enum other { OA, OB };

int mixed(enum color c, enum other o)
{
  if (c < o) {
    return 1;
  }
  return 0;
}
