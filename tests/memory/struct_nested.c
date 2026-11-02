// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 items 3, 5: a nested struct member is a slot whose members are blocks of their own; u.t.x, u.t.y and u.z are leaf cells of one object
#include <cerastes.h>

struct S {
  int x;
  int y;
};
struct T {
  struct S t;
  int z;
};

int nested(void)
    CERASTES_ENSURES(r, r == 9)
{
  struct T u;
  u.t.x = 1;
  u.t.y = 2;
  u.z = 3;
  CERASTES_CHECK(u.t.x + u.t.y + u.z == 6);
  u.t.x = 4;
  return u.t.x + u.t.y + u.z;
}
