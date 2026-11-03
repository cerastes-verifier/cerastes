// expect: error precondition @ L23
// mode: c23, c++20
// twin: tests/memory/struct_value_param.c
// Pins: 0044 item 3: the argument has s.x == 0, get requires s.x > 0 (a requires of the by-value leaf): precondition at the call
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
    CERASTES_ENSURES(r, r == 0)
{
  struct S s = {0, 4};
  return get(s);  // <- here
}
