// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_conv_cast.c
// Pins: 0018 item 5: a conversion from bool into an enumeration is rejected (bool is a keyword in C23 and C++)
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

enum color from_bool(bool b)
{
  return (enum color)b;
}
