// expect: error uninitialized @ L17
// mode: c23, c++20
// twin: tests/memory/struct_value_result.c
// Pins: 0044 item 4 (question 97): return s reads every member and s.y was never written: uninitialized at the return, naming the member
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
  return s;  // <- here
}

int use_make(void)
    CERASTES_ENSURES(r, r == 1)
{
  struct S s = make(1);
  CERASTES_CHECK(s.x == 1);
  CERASTES_CHECK(s.y == 2);
  return s.x;
}
