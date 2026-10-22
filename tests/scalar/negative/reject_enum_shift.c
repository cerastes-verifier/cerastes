// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_arith.c
// Pins: 0018 item 6: an enumeration value as the shift count is rejected at its conversion
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int shifted(enum color c)
{
  return 1 << c;
}
