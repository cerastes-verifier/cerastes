// expect: error check @ L15
// mode: c++20
// twin: tests/memory/struct_init_cpp.cpp
// Pins: 0043 item 6 (C++): {0} on a two-member aggregate zero-fills s.y (accepted: -Wextra only, so no native-ok positive)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int zero_braces()
{
  S s = {0};
  CERASTES_CHECK(s.y == 1);  // <- here
  return s.x;
}
