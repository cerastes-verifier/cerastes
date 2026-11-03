// expect: error check @ L29
// mode: c23, c++20
// twin: tests/memory/struct_value_literal.c
// Pins: 0044 item 7: t = (struct S){1, 2} gives t.y == 2, so check(t.y == 1) fails
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct S mk(void)
    CERASTES_ENSURES(r, r.x == 5 && r.y == 6)
{
  return (struct S){5, 6};
}

int get(struct S s)
    CERASTES_ENSURES(r, r == s.x)
{
  return s.x;
}

int use(void)
    CERASTES_ENSURES(r, r == 9)
{
  struct S t = {0, 0};
  t = (struct S){1, 2};
  CERASTES_CHECK(t.x == 1 && t.y == 1);  // <- here
  int a = get((struct S){.x = 3, .y = 2});
  struct S m = mk();
  return t.x + a + m.x;
}
