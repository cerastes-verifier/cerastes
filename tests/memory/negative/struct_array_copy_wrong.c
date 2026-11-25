// expect: error check @ L20
// mode: c23, c++20
// twin: tests/memory/struct_array_copy.c
// Pins: 0063 item 13: the copy carries the source's values: t.a[2] is 3 after t = s, so a check that it is 4 fails
// Pre-registered from record 0063 item 13 (Part E), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int values(void)
    CERASTES_ENSURES(r, r == 8)
{
  struct S s = {{1, 2, 3, 4}, 5};
  struct S t = {{0, 0, 0, 0}, 0};
  t = s;
  CERASTES_CHECK(t.a[2] == 4);  // <- here
  struct S u = s;
  CERASTES_CHECK(u.a[3] == 4);
  t.a[0] = 9;
  CERASTES_CHECK(s.a[0] == 1 && t.a[0] == 9 && u.a[0] == 1);
  return t.a[2] + u.n;
}
