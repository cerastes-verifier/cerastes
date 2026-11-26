// expect: error check @ L22
// mode: c23, c++20
// twin: tests/memory/struct_member_array_struct_global.c
// Pins: 0064 item 9: g.v[1].y is 4 by the initializer's nested list, not 3
// Pre-registered from record 0064 item 9 (Part C2), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

static struct T g = {{{1, 2}, {3, 4}}, 5};

int main(void)
{
  CERASTES_CHECK(g.v[1].y == 3);  // <- here
  return g.n - 5;
}
