// expect: rejected cerastes-unsupported_expression @ L15
// expect: frontend-warning no_out_of_order_init_in_cpp_mode @ L15
// mode: c23, c++20
// twin: tests/memory/struct_init_forms.c
// Pins: 0043 item 6: designators out of declaration order are rejected (EDG's #2904-D in both modes, an error in C++), listed as the warning EDG prints
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int out_of_order(void)
{
  struct S s = {.y = 2, .x = 1};  // <- here
  return s.x;
}
