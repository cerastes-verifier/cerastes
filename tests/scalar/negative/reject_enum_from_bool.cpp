// expect: rejected cerastes-unsupported_expression @ L10
// twin: tests/scalar/enum_class.cpp
// Pins: 0018 item 5: static_cast from bool into an enumeration is rejected
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

color from_bool(bool b)
{
  return static_cast<color>(b);
}
