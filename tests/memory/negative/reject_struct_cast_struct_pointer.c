// expect: rejected cerastes-unsupported_expression @ L19
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 5: a conversion between struct pointer types is rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};
struct T {
  struct S t;
  int z;
};

int cast_struct_pointer(void)
{
  struct S s = {1, 2};
  struct T *p = (struct T *)&s;  // <- here
  return p != 0;
}
