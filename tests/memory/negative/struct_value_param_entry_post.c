// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/memory/struct_value_param_entry.c
// Pins: 0044 item 3: the ensures r == s.x reads the entry value, which the body's s.x += 1 made wrong
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int bump(struct S s)
    CERASTES_REQUIRES(s.x < 100)
    CERASTES_ENSURES(r, r == s.x)
{
  s.x += 1;
  return s.x;  // <- here
}

int use_bump(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 10)
{
  struct S s = {5, 1};
  int t = bump(s);
  CERASTES_CHECK(s.x == 5);
  return t;
}
