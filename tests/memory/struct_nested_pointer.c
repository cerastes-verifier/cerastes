// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 items 5, 9: &u.t (the address of a nested struct) is a struct S * whose owns(*s) is every leaf of u.t; u.z is untouched across the call; the scope exit finds every leaf whole
#include <cerastes.h>

struct S {
  int x;
  int y;
};
struct T {
  struct S t;
  int z;
};

void set_x(struct S *s)
    CERASTES_REQUIRES(cerastes_owns(*s))
    CERASTES_ENSURES(cerastes_owns(*s) && s->x == 7 && s->y == cerastes_old(s->y))
{
  s->x = 7;
}

int use_nested(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct T u;
  u.t.x = 1;
  u.t.y = 2;
  u.z = 3;
  set_x(&u.t);
  CERASTES_CHECK(u.t.x == 7 && u.t.y == 2 && u.z == 3);
  return u.z;
}
