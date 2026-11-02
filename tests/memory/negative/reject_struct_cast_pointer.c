// expect: rejected cerastes-unsupported_expression @ L15
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 5, 0028 item 1: a conversion between a struct pointer and another object pointer would make &s and &s.x equal in C and different in the model
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int cast_pointer(void)
{
  struct S s = {1, 2};
  int *q = (int *)&s;  // <- here
  return q == &s.x;
}
