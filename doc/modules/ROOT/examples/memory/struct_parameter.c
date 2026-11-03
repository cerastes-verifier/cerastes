// expect: verified
// expect: native-ok
// mode: c23, c++20
// A pointer to a struct: owns(*p) covers every member; old reads a member's value on entry.
// tag::source[]
#include <cerastes.h>

struct point {
  int x;
  int y;
};

void mirror(struct point *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->x == cerastes_old(p->y) && p->y == cerastes_old(p->x))
{
  int t = p->x;
  p->x = p->y;
  p->y = t;
}

int use_mirror(void)
    CERASTES_ENSURES(r, r == 2)
{
  struct point pt = {1, 2};
  mirror(&pt);
  return pt.x;
}
// end::source[]
