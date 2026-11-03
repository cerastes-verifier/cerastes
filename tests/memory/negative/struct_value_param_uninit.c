// expect: error uninitialized @ L24
// mode: c23, c++20
// twin: tests/memory/struct_value_param.c
// Pins: 0044 item 5 (question 97): passing a struct by value reads every member, and s.y was never written: uninitialized at the call, naming the member
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int get(struct S s)
    CERASTES_REQUIRES(s.x > 0)
    CERASTES_ENSURES(r, r == s.x)
{
  return s.x;
}

int use_get(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct S s;
  s.x = 3;
  return get(s);  // <- here
}
