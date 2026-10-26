// expect: rejected cerastes-unsupported_expression @ L6
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 2 / Consequences: `static_assert(1.5, "")` (C++) is rejected too: the condition is a double converted to bool, which is not an integer condition. C names the type (unsupported_type, reject_static_assert_float.c); C++ names no type (unsupported_expression)
#include <cerastes.h>

static_assert(1.5, "a float condition");

int one()
{
  return 1;
}
