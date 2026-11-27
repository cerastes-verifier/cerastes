// expect: error check @ L23
// mode: c23, c++20
// twin: tests/memory/struct_member_array_struct_copy.c
// Pins: 0064 item 12: the copy u of t is its own object: a write to u.v[0].x does not reach t, so t.v[0].x == 9 is not a fact (t.v[0].x is 1)
// Pre-registered from record 0064 item 12 (Part C2), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

int use(void)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  struct T u = t;
  u.v[0].x = 9;
  CERASTES_CHECK(t.v[0].x == 9);  // <- here
  return u.n;
}
