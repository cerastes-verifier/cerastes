// expect: error postcondition @ L26
// mode: c23, c++20
// twin: tests/memory/struct_pointer_cell.c
// Pins: 0043 item 8: p points to b after the call, whose y is 4, not a's 2
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void set(struct S **pp, struct S *q)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_live(q))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == q && cerastes_live(q))
{
  *pp = q;
}

int use_set(void)
    CERASTES_ENSURES(r, r == 2)
{
  struct S a = {1, 2};
  struct S b = {3, 4};
  struct S *p = &a;
  set(&p, &b);
  return p->y;  // <- here
}
