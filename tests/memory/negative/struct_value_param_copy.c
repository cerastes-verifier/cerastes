// expect: error check @ L25
// mode: c23, c++20
// twin: tests/memory/struct_value_param_entry.c
// Pins: 0044 items 3, 5: the callee changes its copy, not the caller's struct: check(s.x == 6) fails
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int bump(struct S s)
    CERASTES_REQUIRES(s.x < 100)
    CERASTES_ENSURES(r, r == s.x + 1)
{
  s.x += 1;
  return s.x;
}

int use_bump(void)
    CERASTES_ENSURES(r, r == 6)
{
  struct S s = {5, 1};
  int t = bump(s);
  CERASTES_CHECK(s.x == 6);  // <- here
  return t;
}
