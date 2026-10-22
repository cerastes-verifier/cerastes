// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_arith.c
// Pins: 0018 item 6: c + 1 is computed in C's unsigned int and in C++'s int, so an enumeration value in arithmetic is rejected at its conversion
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int plus_one(enum color c)
{
  return c + 1;
}
