// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 9: owns(p->t) holds the leaves of the nested struct (p->t.x, p->t.y) and a liveness fraction; u.z is not part of it and stays the caller's
#include <cerastes.h>

struct S {
  int x;
  int y;
};
struct T {
  struct S t;
  int z;
};

void bump_t(struct T *p)
    CERASTES_REQUIRES(cerastes_owns(p->t) && p->t.x < 100)
    CERASTES_ENSURES(cerastes_owns(p->t) && p->t.x == cerastes_old(p->t.x) + 1 && p->t.y == cerastes_old(p->t.y))
{
  p->t.x += 1;
}

int use_bump_t(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct T u;
  u.t.x = 1;
  u.t.y = 2;
  u.z = 3;
  bump_t(&u);
  CERASTES_CHECK(u.t.x == 2 && u.t.y == 2);
  return u.z;
}
