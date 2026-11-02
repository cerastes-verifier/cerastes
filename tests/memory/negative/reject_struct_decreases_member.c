// expect: rejected cerastes-unsupported_annotation @ L15
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 15: decreases(p->x) reads memory at a measure: rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int count(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= p->x && p->x <= 10)
    CERASTES_ENSURES(r, cerastes_owns(*p))
    CERASTES_DECREASES(p->x)  // <- here
{
  return 0;
}
