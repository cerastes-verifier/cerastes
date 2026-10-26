// expect: rejected cerastes-unsupported_type @ L9
// mode: c23, c++20
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6: _Alignof/alignof operand types follow the type rule (C23 spells it alignof; _Alignof in older C, EDG accepts both: see INDEX)
#include <cerastes.h>

int align(void)
{
  return (int)alignof(char[4]);
}
