// expect: error unsequenced @ L23
// mode: c23, c++20
// twin: tests/memory/struct_value_seq_distinct.c
// Pins: 0044 item 5: w may be p, so the read of *p by value and the write w->y may be one object: unsequenced at the store
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int get(struct S s)
    CERASTES_ENSURES(r, r == s.x)
{
  return s.x;
}

int both(struct S *p, struct S *q, int c)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q) && 0 <= p->x && p->x < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*q))
{
  struct S *w = c ? p : q;
  return get(*p) + (w->y = 1);  // <- here
}
