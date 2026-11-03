// expect: error check @ L15
// mode: c23, c++20
// twin: tests/memory/struct_init_forms.c
// Pins: 0043 item 6: a positional list shorter than the struct zero-fills the rest (accepted: -Wextra only, so no native-ok positive): s.y is 0
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int short_list(void)
{
  struct S s = {1};
  CERASTES_CHECK(s.y == 1);  // <- here
  return s.x;
}
