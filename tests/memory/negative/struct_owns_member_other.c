// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/struct_owns_member.c
// Pins: 0043 item 9: owns(p->x) says nothing about p->y: the write fails its permission check
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void set_y(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(p->x))
    CERASTES_ENSURES(cerastes_owns(p->x))
{
  p->y = 5;  // <- here
}
