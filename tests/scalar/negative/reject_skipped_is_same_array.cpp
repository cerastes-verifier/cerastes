// expect: rejected cerastes-unsupported_type @ L7
// twin: tests/scalar/static_assert_skipped.cpp
// Pins: 0024 item 3: the variable template's own arguments go through item 6's helper: the front end makes char[65536u * 65536u + 1] the type char[1], so the assertion holds in EDG and the wrapped bound must not pass unseen
#include <cerastes.h>
#include <type_traits>

static_assert(std::is_same_v<char[65536u * 65536u + 1], char[1]>, "wrapping bound");

int one()
{
  return 1;
}
