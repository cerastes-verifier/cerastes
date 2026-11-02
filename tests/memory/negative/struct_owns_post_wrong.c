// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/memory/struct_owns_param.c
// Pins: 0043 item 9: the ensures also claims p->y grew; only p->x did
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void bump(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->x < 100 && p->y < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && p->x == cerastes_old(p->x) + 1 && p->y == cerastes_old(p->y) + 1)
{
  p->x++;
}  // <- here
