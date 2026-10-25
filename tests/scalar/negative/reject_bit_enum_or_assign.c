// expect: rejected cerastes-unsupported_expression @ L11
// c-only: C++ refuses |= on an enumeration in the front end (error #511, reject_bit_enum_or_assign_cpp.cpp); C accepts it and cerastes rejects it
// twin: tests/scalar/bit_compound.c
// Pins: 0023 item 4: |= on a value of an enumeration (0018 item 8)
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

enum color widen(enum color c)
{
  c |= 1;
  return c;
}
