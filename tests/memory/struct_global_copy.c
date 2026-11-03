// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 items 5, 8: a nested struct global {1, {2, 3}} (the slot facts), copies out of a member of a global (t = g.s), into a global (g.s = origin, origin = make(4)), by value (first(origin)) and between structs (t = origin)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct T {
  int a;
  struct S s;
};

static struct T g = {1, {2, 3}};
static struct S origin;

struct S make(int a)
    CERASTES_REQUIRES(0 <= a && a < 100)
    CERASTES_ENSURES(r, r.x == a && r.y == a + 1)
{
  struct S s = {a, a + 1};
  return s;
}

int first(struct S s)
    CERASTES_ENSURES(r, r == s.x)
{
  return s.x;
}

int main(void)
{
  struct S t = g.s;
  CERASTES_CHECK(t.x == 2 && t.y == 3);
  origin = make(4);
  CERASTES_CHECK(origin.x == 4 && origin.y == 5);
  int a = first(origin);
  g.s = origin;
  CERASTES_CHECK(g.s.x == 4 && g.s.y == 5 && g.a == 1);
  t = origin;
  CERASTES_CHECK(t.y == 5);
  return a + t.y + g.s.y;
}
