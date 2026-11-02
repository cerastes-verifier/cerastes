// expect: error permission @ L14
// mode: c23, c++20
// twin: tests/memory/struct_owns_member.c
// Pins: 0043 item 9: the ensures reads p->y, which no atom of the function frames
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int get_y(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(p->x))
    CERASTES_ENSURES(r, cerastes_owns(p->x) && r == p->y)  // <- here
{
  return 0;
}
