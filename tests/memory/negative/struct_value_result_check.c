// expect: error check @ L26
// mode: c23, c++20
// twin: tests/memory/struct_value_result.c
// Pins: 0044 item 4: the ensures says r.y == 2, so check(s.y == 1) after the call fails
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct S make(int a)
    CERASTES_ENSURES(r, r.x == a && r.y == 2)
{
  struct S s;
  s.x = a;
  s.y = 2;
  return s;
}

int use_make(void)
    CERASTES_ENSURES(r, r == 1)
{
  struct S s = make(1);
  CERASTES_CHECK(s.x == 1);
  CERASTES_CHECK(s.y == 1);  // <- here
  return s.x;
}
