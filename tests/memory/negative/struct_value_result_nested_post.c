// expect: error postcondition @ L22
// mode: c23, c++20
// twin: tests/memory/struct_value_result_nested.c
// Pins: 0044 item 4: the ensures says r.t.y == a, but mkt returns a + 1 (the caller keeps an ensures that holds either way)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct T {
  struct S t;
  int z;
};

struct T mkt(int a)
    CERASTES_REQUIRES(0 <= a && a < 100)
    CERASTES_ENSURES(r, r.t.x == a && r.t.y == a && r.z == 0)
{
  struct T w = {{a, a + 1}, 0};
  return w;  // <- here
}

int use_t(void)
    CERASTES_ENSURES(r, r == 4 || r == 5)
{
  struct T w = mkt(4);
  return w.t.y;
}
