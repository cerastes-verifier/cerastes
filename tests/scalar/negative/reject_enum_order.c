// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_eq.c
// Pins: 0018 items 6 and 7: an ordering comparison of an unscoped enumeration value is rejected (== and != are accepted)
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int below_green(enum color c)
{
  if (c < GREEN) {
    return 1;
  }
  return 0;
}
