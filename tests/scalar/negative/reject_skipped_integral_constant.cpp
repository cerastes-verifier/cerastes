// expect: rejected cerastes-unsupported_type @ L7
// twin: tests/scalar/static_assert_skipped.cpp
// Pins: 0024 item 3: the template arguments of the class instance holding the named static member go through item 6's helper
#include <cerastes.h>
#include <type_traits>

static_assert(std::integral_constant<unsigned, 65536u * 65536u>::value == 0u, "wrapping argument");

int one()
{
  return 1;
}
