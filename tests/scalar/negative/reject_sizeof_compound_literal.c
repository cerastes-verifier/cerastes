// expect: rejected cerastes-unsupported_type @ L10
// mode: c23
// twin: tests/scalar/sizeof_value.c
// c-only: compound literals are C (C++ has no such expression)
// Pins: 0024 item 6: the type of a sizeof expression operand is checked (optional cut 2 of the plan: if cut, this becomes a verified-today shape: see INDEX)
#include <cerastes.h>

int literal(void)
{
  return (int)sizeof((char[4]){0});
}
