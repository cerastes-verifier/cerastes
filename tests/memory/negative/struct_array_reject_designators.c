// expect: rejected cerastes-unsupported_declaration @ L18
// expect: rejected cerastes-unsupported_expression @ L24
// expect: frontend-warning no_out_of_order_init_in_cpp_mode @ L24
// c-only: array designators and a member designated twice are C (C++20 has its own front-end errors for both)
// twin: tests/memory/struct_array_init.c
// Pins: 0063 item 6: an array designator inside a member's list is rejected in C (EDG keeps only the folded index, as array_reject_designator.c; in C++ it is the front end's c99_designator) and a member designated twice is rejected in C (as reject_struct_designator_override.c; both lines to confirm at the joined run, the warning as that pin lists it)
// Pre-registered from record 0063 item 6 (Part C), before any run: rejected
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int array_designator(void)
{
  struct S s = {.a = {[1] = 2}, .n = 3};  // <- here
  return s.n;
}

int repeated(void)
{
  struct S t = {.n = 1, .n = 2};  // <- here
  return t.n;
}
