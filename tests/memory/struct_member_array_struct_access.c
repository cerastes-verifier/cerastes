// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 3, 5 and 6 (Part C2; Flips A of Part C2, the row of negative/struct_array_reject_members.c at L36 lifted): an array member whose elements are structures in a local: t.v[i].x written and read at a symbolic index, the nested initializer per element, C's zero fill of the elements a list does not reach (the member struct P v[2] of the lifted row's program)
// Pre-registered from record 0064 items 3, 5 and 6 (Part C2), before any run: verifies
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
    CERASTES_REQUIRES(0 <= i && i < 2)
    CERASTES_ENSURES(r, r == 6)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  t.v[i].x = 6;
  CERASTES_CHECK(t.n == 5 && t.v[0].y == 2 && t.v[1].y == 4);
  return t.v[i].x;
}

int zero_fill(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct T u = {{{1, 2}}, 5};
  CERASTES_CHECK(u.v[0].y == 2 && u.v[1].x == 0 && u.v[1].y == 0 && u.n == 5);
  return u.v[1].x;
}
