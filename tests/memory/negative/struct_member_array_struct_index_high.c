// expect: error out-of-bounds @ L22
// mode: c23, c++20
// twin: tests/memory/struct_member_array_struct_access.c
// Pins: 0064 items 3 and 5: the member's length is 2: i == 2 is out-of-bounds at t.v[i].x
// Pre-registered from record 0064 items 3 and 5 (Part C2), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

int rw(int i)
    CERASTES_REQUIRES(0 <= i && i <= 2)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  t.v[i].x = 6;  // <- here
  return t.n;
}
