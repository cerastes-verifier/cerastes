// expect: error permission @ L23
// mode: c23, c++20
// twin: tests/memory/struct_overlap_trap.c
// Pins: 0043 item 9: &s.x is a leaf of *s: the two atoms share the cell s.x, and the pairwise separation fails at the call whatever the atoms' pointee types
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void f(struct S *s, int *q)
    CERASTES_REQUIRES(cerastes_owns(*s) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*s) && cerastes_owns(*q) && s->x == 1 && *q == 2 && s->y == cerastes_old(s->y))
{
  s->x = 1;
  *q = 2;
}

int use_f(void)
{
  struct S s = {5, 6};
  f(&s, &s.x);  // <- here
  return s.y;
}
