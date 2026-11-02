// expect: error null-dereference @ L14
// mode: c23, c++20
// twin: tests/memory/struct_owning_param.c
// Pins: 0043 items 5, 9: owning allows NULL and p->x is applied without the test
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void set_x(CERASTES_OWNING struct S *p)
{
  p->x = 1;  // <- here
}
