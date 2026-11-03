// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 items 3-5: a struct local is a block of member cells; each member is written, then read by name; the flags follow the cells
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int second(void)
{
  struct S s;
  s.x = 3;
  s.y = 4;
  CERASTES_CHECK(s.x + s.y == 7);
  return s.y;
}
