// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_arith.c
// Pins: 0018 item 8: an enumeration value as the right-hand side of a compound assignment is converted to unsigned int in C and int in C++, so it is rejected
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int add(int x, enum color c)
{
  x += c;
  return x;
}
