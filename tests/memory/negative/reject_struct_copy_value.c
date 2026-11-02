// expect: rejected cerastes-unsupported_expression @ L16
// mode: c23, c++20
// twin: tests/memory/struct_copy.c
// Pins: 0043 item 7: a struct copy used as a value is rejected (the copy is a statement or an initializer)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int copy_value(void)
{
  struct S s = {1, 2};
  struct S t = {3, 4};
  int v = (t = s).x;  // <- here
  return v + t.y;
}
