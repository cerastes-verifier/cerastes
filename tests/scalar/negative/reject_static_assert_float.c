// expect: rejected cerastes-unsupported_type @ L8
// mode: c23
// twin: tests/scalar/sizeof_value.c
// c-only: C++ rejects it with another tag, unsupported_expression (reject_static_assert_float.cpp)
// Pins: 0024 item 2 / Consequences: `_Static_assert(1.5, "")` verified before and is rejected now
#include <cerastes.h>

static_assert(1.5, "a float condition");

int one(void)
{
  return 1;
}
