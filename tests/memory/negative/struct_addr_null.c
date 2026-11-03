// expect: error null-dereference @ L18
// mode: c23, c++20
// twin: tests/memory/struct_owning_param.c
// Pins: 0043 item 5: &p->x is arithmetic on p, so it checks p != NULL: owning allows NULL
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void sink(int *q)
    CERASTES_REQUIRES(cerastes_live(q))
    CERASTES_ENSURES(cerastes_live(q));

void touch(CERASTES_OWNING struct S *p)
{
  sink(&p->x);  // <- here
}
