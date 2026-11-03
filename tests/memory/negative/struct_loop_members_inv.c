// expect: error invariant-entry @ L17
// mode: c23, c++20
// twin: tests/memory/struct_loop_members.c
// Pins: 0043 item 11: s.y is 0 on entry, the invariant says 1 more than twice i
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int loop_members(void)
    CERASTES_ENSURES(r, r == 31)
{
  struct S s = {0, 0};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10 && s.x == i && s.y == 2 * i + 1)  // <- here
  CERASTES_DECREASES(10 - i)
  while (i < 10) {
    s.x += 1;
    s.y += 2;
    i += 1;
  }
  return s.x + s.y;
}
