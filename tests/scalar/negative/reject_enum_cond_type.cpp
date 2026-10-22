// expect: rejected cerastes-unsupported_expression @ L10
// twin: tests/scalar/enum_contract.c
// Pins: 0018 item 8: in C++ x ? RED : GREEN has the enumeration type (in C it is int, and C accepts it), so it is rejected
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

enum color pick(int x)
{
  return x ? RED : GREEN;
}
