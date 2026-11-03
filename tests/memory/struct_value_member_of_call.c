// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 item 5: a struct member of a struct-valued call is a copy source: struct S u = mk().t; and v = mk().t; (the leaves of the nested member of the call's results)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct U {
  struct S t;
  int z;
};

struct U mk(void)
    CERASTES_ENSURES(r, r.t.x == 1 && r.t.y == 2 && r.z == 3)
{
  struct U w = {{1, 2}, 3};
  return w;
}

int use(void)
    CERASTES_ENSURES(r, r == 1 + 2 + 2)
{
  struct S u = mk().t;
  struct S v = {0, 0};
  v = mk().t;
  CERASTES_CHECK(u.x == 1 && u.y == 2 && v.x == 1 && v.y == 2);
  return u.x + u.y + v.y;
}
