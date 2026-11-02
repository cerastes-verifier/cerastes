// expect: error unsequenced @ L23
// mode: c23, c++20
// twin: tests/memory/struct_seq_distinct.c
// Pins: 0043 item 12, audit B3: s.x beside a call that writes s.x is unsequenced (the order is C's to leave open): the access and the call's owns place may be one cell
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int set_x(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && p->x == 1 && r == 1)
{
  p->x = 1;
  return 1;
}

int call_beside(void)
{
  struct S s = {0, 0};
  int r = s.x + set_x(&s);  // <- here
  CERASTES_CHECK(r == 1);
  return r;
}
