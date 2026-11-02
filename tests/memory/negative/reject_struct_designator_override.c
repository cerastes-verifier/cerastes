// expect: rejected cerastes-unsupported_expression @ L18
// expect: frontend-warning no_out_of_order_init_in_cpp_mode @ L18
// c-only: C's overriding designators (C++20 requires declaration order, each member once)
// twin: tests/memory/struct_local.c
// Pins: 0043 audit B4: a designator that overrides an earlier one in the same list is rejected (the first value is evaluated and dropped)
#include <cerastes.h>
#include <stddef.h>

struct S {
  int x;
  int y;
};

int g(void);

int override(void)
{
  struct S s = {.y = g(), .y = 2};  // <- here
  return s.y;
}
