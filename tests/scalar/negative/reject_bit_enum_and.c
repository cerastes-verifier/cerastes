// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/bit_and.c
// Pins: 0023 item 4: a value of an enumeration as an operand of & (0018's rule for arithmetic)
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int test(enum color c)
{
  return c & 1;
}
