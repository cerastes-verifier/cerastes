// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_arith.c
// Pins: 0018 item 6: -c is computed in C's unsigned int and in C++'s int, so it is rejected at the conversion
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int negated(enum color c)
{
  return -c;
}
