// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 9: owns(*p) of a struct S *: every leaf with its flag and range; p->x++ writes one leaf; old(p->x) and old(p->y) in the ensures; the caller gets every leaf back
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void bump(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->x < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && p->x == cerastes_old(p->x) + 1 && p->y == cerastes_old(p->y))
{
  p->x++;
}

int use_bump(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct S s = {4, 9};
  bump(&s);
  CERASTES_CHECK(s.y == 9);
  return s.x;
}
