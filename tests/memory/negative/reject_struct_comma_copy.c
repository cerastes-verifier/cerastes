// expect: rejected cerastes-unsupported_expression @ L16
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 7, audit B3: a struct copy as the left operand of a comma is a copy used inside an expression, rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int comma_copy(void)
{
  struct S s = {1, 2};
  struct S t = {3, 4};
  return (t = s, 0) + t.x;  // <- here
}
