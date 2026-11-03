// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 item 5: get(*p) reads the members of *p and q->y = 1 writes one of *q: two atoms are two places, path-precise, and the places differ
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

int both(struct S *p, struct S *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q) && 0 <= p->x && p->x < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*q) && r == cerastes_old(p->x) + 1 && q->y == 1)
{
  return get(*p) + (q->y = 1);
}
