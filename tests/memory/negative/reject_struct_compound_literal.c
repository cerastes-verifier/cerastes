// expect: rejected cerastes-unsupported_expression @ L14
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 15: the address of a struct compound literal is rejected (M4.5c2)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int compound(void)
{
  struct S *p = &(struct S){1, 2};  // <- here
  return p->x;
}
