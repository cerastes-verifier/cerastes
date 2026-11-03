// expect: error check @ L22
// mode: c23, c++20
// twin: tests/memory/struct_nested.c
// Pins: 0043 item 5: the nested leaf holds 1, not 2
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
{
  struct T u;
  u.t.x = 1;
  u.t.y = 2;
  u.z = 3;
  CERASTES_CHECK(u.t.x == 2);  // <- here
  return u.z;
}
