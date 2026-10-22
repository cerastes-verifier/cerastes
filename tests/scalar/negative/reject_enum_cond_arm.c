// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_to_int.c
// Pins: 0018 item 6: an enumeration value as an arm of ?: is converted to unsigned int in C and int in C++, so it is rejected
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int pick(int x, enum color c)
{
  return x ? c : 0;
}
