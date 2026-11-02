// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 12: s.x and s.y, u.t.x and u.t.y are different objects; p->x against q->x with two owns atoms is path-precise and the places differ
#include <cerastes.h>
#include <stddef.h>

struct S {
  int x;
  int y;
};
struct T {
  struct S t;
  int z;
};

int member_distinct(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct S s = {1, 2};
  int a = s.x + (s.y = 1);
  return a + s.y;
}

int nested_distinct(void)
    CERASTES_ENSURES(r, r == 6)
{
  struct T u = {{1, 2}, 3};
  int a = u.t.x + (u.t.y = 2);
  return a + u.z;
}

int two_pointers(struct S *p, struct S *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q) && -100 < p->x && p->x < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*q) && r == cerastes_old(p->x) + 1 && q->x == 1)
{
  return p->x + (q->x = 1);
}
