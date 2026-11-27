// expect: error check @ L24
// mode: c23, c++20
// twin: tests/memory/array_struct_access.c
// Pins: 0064 item 5: v[i].t.x was written 7, so the check that it is 8 fails (the member steps of a nested structure inside an element name one leaf)
// Pre-registered from record 0064 item 5 (Part B), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct Q {
  struct P t;
  int z;
};

int nested(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
    CERASTES_ENSURES(r, r == 0)
{
  struct Q v[2];
  v[i].t.x = 7;
  CERASTES_CHECK(v[i].t.x == 8);  // <- here
  v[i].t.y = 8;
  v[i].z = 9;
  return v[i].t.x - 7 + v[i].z - 9;
}
