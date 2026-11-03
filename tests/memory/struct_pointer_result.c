// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 8: a struct S * result: the result is a or b by the ensures, the caller case-splits and writes through it
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct S *pick(struct S *a, struct S *b, int c)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b))
    CERASTES_ENSURES(r, cerastes_owns(*a) && cerastes_owns(*b) && r == (c != 0 ? a : b) && a->x == cerastes_old(a->x) && a->y == cerastes_old(a->y) && b->x == cerastes_old(b->x) && b->y == cerastes_old(b->y))
{
  return c != 0 ? a : b;
}

int use_pick(void)
    CERASTES_ENSURES(r, r == 4)
{
  struct S s = {1, 2};
  struct S t = {3, 4};
  struct S *p = pick(&s, &t, 1);
  p->x = 5;
  CERASTES_CHECK(s.x == 5 && t.x == 3);
  return t.y;
}
