// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/bit_not.c
// Pins: 0023 item 4: ~ of a value of an enumeration (C's bare enumeration operand, P26)
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int test(enum color c)
{
  return ~c;
}
