// expect: rejected cerastes-self_assignment @ L13
// expect: rejected cerastes-self_assignment @ L19
// expect: rejected cerastes-self_assignment @ L25
// mode: c23, c++20
// twin: tests/scalar/self_assign_not_warned.c
// Pins: 0021 item 8 (clang -Wself-assign): the type of the variable does not matter: short, bool and an enumeration
#include <cerastes.h>

enum color { RED, GREEN };

int short_type(short s)
{
  s = s;
  return s;
}

int bool_type(bool b)
{
  b = b;
  return b;
}

int enum_type(enum color e)
{
  e = e;
  return e == RED;
}
