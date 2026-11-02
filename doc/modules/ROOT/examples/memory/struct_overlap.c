// expect: error permission @ L23
// mode: c23, c++20
// Two owning parameters of different types that reach one cell: s.x is both a member of *s and *q.
// tag::source[]
#include <cerastes.h>

struct point {
  int x;
  int y;
};

void place(struct point *s, int *q)
    CERASTES_REQUIRES(cerastes_owns(*s) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*s) && cerastes_owns(*q))
{
  s->x = 1;
  *q = 2;
}

void trap(void)
{
  struct point pt = {0, 0};
  place(&pt, &pt.x);
}
// end::source[]
