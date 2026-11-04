// expect: verified
// expect: native-ok
// mode: c23, c++20
// Members are owned one by one: the contract owns exactly what the function writes.
// tag::source[]
#include <cerastes.h>
#include <limits.h>

struct point {
  int x;
  int y;
};

void move_right(struct point *p, int dx)
    CERASTES_REQUIRES(cerastes_owns(p->x) && 0 <= dx && p->x <= INT_MAX - dx)
    CERASTES_ENSURES(cerastes_owns(p->x) && p->x == cerastes_old(p->x) + dx)
{
  p->x += dx;
}

int use_move(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct point pt = {.x = 2, .y = 7};
  move_right(&pt, 3);
  CERASTES_CHECK(pt.y == 7);
  return pt.x;
}
// end::source[]
