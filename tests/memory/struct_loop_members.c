// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 11: every leaf of a struct local in scope is an automatic place of the loop (its own value, flag and range rows), and the liveness cell is one row for the root block
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int loop_members(void)
    CERASTES_ENSURES(r, r == 30)
{
  struct S s = {0, 0};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10 && s.x == i && s.y == 2 * i)
  CERASTES_DECREASES(10 - i)
  while (i < 10) {
    s.x += 1;
    s.y += 2;
    i += 1;
  }
  return s.x + s.y;
}
