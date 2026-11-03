// expect: error permission @ L23
// mode: c23, c++20
// twin: tests/memory/struct_value_result_use.c
// Pins: 0044 item 5: *p = mk(7) with no atom for *p: permission at the store, on the first member written
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct S mk(int a)
    CERASTES_REQUIRES(0 <= a && a < 100)
    CERASTES_ENSURES(r, r.x == a && r.y == a + 1)
{
  struct S s = {a, a + 1};
  return s;
}

void store(struct S *p)
    CERASTES_REQUIRES(p != 0)
{
  *p = mk(7);  // <- here
}
