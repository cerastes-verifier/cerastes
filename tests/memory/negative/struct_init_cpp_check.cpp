// expect: error check @ L15
// mode: c++20
// twin: tests/memory/struct_init_cpp.cpp
// Pins: 0043 item 6: S s{} zero-initializes every leaf
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int value_init()
{
  S s{};
  CERASTES_CHECK(s.x == 1);  // <- here
  return s.y;
}
