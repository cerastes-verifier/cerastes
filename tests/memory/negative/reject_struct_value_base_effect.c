// expect: rejected cerastes-unsupported_expression @ L20
// mode: c23, c++20
// twin: tests/memory/struct_value_pass_object.c
// Pins: 0044 item 5 (audit S4; M8): a base with a side effect passed by value is read once per member: first(*p++) is rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int first(struct S s)
{
  return s.x;
}

int f(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
{
  return first(*p++);  // <- here
}
