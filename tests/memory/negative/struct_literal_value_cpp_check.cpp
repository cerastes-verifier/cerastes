// expect: error check @ L24
// twin: tests/memory/struct_literal_value_cpp.cpp
// Pins: 0044 item 7: t = (S){1, 2} gives t.x == 1: check(t.x == 2) fails
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int get(S s)
    CERASTES_ENSURES(r, r == s.y)
{
  return s.y;
}

int use()
    CERASTES_ENSURES(r, r == 4 + 9 + 1)
{
  S t = {0, 0};
  t = (S){1, 2};
  int a = (S){3, 4}.y;
  int b = get((S){8, 9});
  CERASTES_CHECK(t.x == 2 && a == 4 && b == 9);  // <- here
  return a + b + t.x;
}
