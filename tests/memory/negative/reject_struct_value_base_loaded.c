// expect: rejected cerastes-unsupported_expression @ L24
// mode: c23, c++20
// twin: tests/memory/struct_value_pass_object.c
// Pins: 0044 item 5 (audit S4; M8): a base read through a loaded pointer passed by value: first(*w->q) is rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct W {
  struct S *q;
};

int first(struct S s)
{
  return s.x;
}

int f(struct W *w)
    CERASTES_REQUIRES(cerastes_owns(*w))
{
  return first(*w->q);  // <- here
}
