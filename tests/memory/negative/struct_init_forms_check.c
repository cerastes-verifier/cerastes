// expect: error check @ L15
// mode: c23, c++20
// twin: tests/memory/struct_init_forms.c
// Pins: 0043 item 6: {} zero-initializes every leaf
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int empty_braces(void)
{
  struct S s = {};
  CERASTES_CHECK(s.x == 1);  // <- here
  return s.y;
}
