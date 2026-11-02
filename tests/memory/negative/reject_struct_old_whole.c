// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 9: old(*p) is a whole struct value in a contract: rejected (old(p->x) is the place)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void f(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_old(*p).x == 1)  // <- here
{
}
