// expect: rejected cerastes-unsupported_expression @ L12
// mode: c23, c++20
// twin: tests/scalar/bit_or.c
// Pins: 0023 item 4: RED | NA with two enumerations (C++20's deprecated enumeration-enumeration conversion)
#include <cerastes.h>

enum color { RED, GREEN, BLUE };
enum neg { NA = -1, NB };

int test(void)
{
  return RED | NA;
}
