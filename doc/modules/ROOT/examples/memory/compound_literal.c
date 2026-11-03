// expect: verified
// expect: native-ok
// mode: c23
// c-only: compound literals are C (C++ has them only as a GNU extension, as temporaries)
// Compound literals: as values, and the object `&(struct S){...}` designates, which lives until its block ends.
// tag::source[]
#include <cerastes.h>

struct point {
  int x;
  int y;
};

void shift(struct point *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->x < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && p->x == cerastes_old(p->x) + 1 && p->y == cerastes_old(p->y))
{
  p->x += 1;
}

int first(struct point p)
    CERASTES_ENSURES(r, r == p.x)
{
  return p.x;
}

int use(void)
    CERASTES_ENSURES(r, r == 11)
{
  struct point a = (struct point){.y = 2};
  int *n = &(int){5};
  struct point *q = &(struct point){1, 2};
  shift(q);
  *n += q->x;
  return *n + first((struct point){4, a.y}) + a.x;
}
// end::source[]
