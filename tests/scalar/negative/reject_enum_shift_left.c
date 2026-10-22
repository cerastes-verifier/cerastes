// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_arith.c
// Pins: 0018 item 6: an enumeration value as the shifted operand is rejected at its conversion (unsigned int in C, int in C++)
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int shifted(enum color c)
{
  return c << 1;
}
