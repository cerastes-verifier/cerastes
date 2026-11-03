// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/memory/struct_value_param.c
// Pins: 0044 item 3: the ensures says r == s.y but get returns s.x (the caller keeps an ensures that holds either way)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int get(struct S s)
    CERASTES_REQUIRES(s.x > 0)
    CERASTES_ENSURES(r, r == s.y)
{
  return s.x;  // <- here
}

int use_get(void)
    CERASTES_ENSURES(r, r == 3 || r == 4)
{
  struct S s = {3, 4};
  return get(s);
}
