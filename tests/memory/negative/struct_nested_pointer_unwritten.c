// expect: error uninitialized @ L28
// mode: c23, c++20
// twin: tests/memory/struct_nested_pointer.c
// Pins: 0043 items 7, 9 and 0042: owns(*s) includes every leaf's flag, and u.t.y was never written: the call's flag row is uninitialized
#include <cerastes.h>

struct S {
  int x;
  int y;
};
struct T {
  struct S t;
  int z;
};

void set_x(struct S *s)
    CERASTES_REQUIRES(cerastes_owns(*s))
    CERASTES_ENSURES(cerastes_owns(*s) && s->x == 7 && s->y == cerastes_old(s->y))
{
  s->x = 7;
}

int use_nested(void)
{
  struct T u;
  u.t.x = 1;
  u.z = 3;
  set_x(&u.t);  // <- here
  return u.z;
}
