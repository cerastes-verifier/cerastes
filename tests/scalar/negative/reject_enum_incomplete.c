// expect: rejected cerastes-unsupported_declaration @ L7
// c-only: C++ refuses a forward declaration of an enumeration without a fixed type (error #102)
// twin: tests/scalar/enum_switch.c
// Pins: 0018 item 2: an incomplete enumeration has no enumerator list and so no value range: rejected at the declaration
#include <cerastes.h>

enum later;

int ident(int a)
{
  return a;
}
