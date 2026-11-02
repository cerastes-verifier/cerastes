// expect: error check @ L15
// c-only: a partial designated list is -Wmissing-field-initializers in g++ -Wextra
// twin: tests/memory/struct_init_partial.c
// Pins: 0043 item 6: the leaf the designated list leaves out is zero, not 1
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int designated(void)
{
  struct S s = {.y = 2};
  CERASTES_CHECK(s.x == 1);  // <- here
  return s.y;
}
