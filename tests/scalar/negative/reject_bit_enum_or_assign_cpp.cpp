// expect: rejected enum_type_not_allowed @ L10
// twin: tests/scalar/bit_compound.c
// Pins: 0023 item 4: in C++ |= on a value of an enumeration is a front-end error (#511); the C test is reject_bit_enum_or_assign.c
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

enum color widen(enum color c)
{
  c |= 1;
  return c;
}
