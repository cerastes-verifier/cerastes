// expect: error check @ L15
// twin: tests/memory/struct_global_cpp.cpp
// Pins: 0044 item 8: the start of g.y is unknown (another unit may initialize it first): check(g.y == 2) fails
#include <cerastes.h>

struct S {
  int x;
  int y;
};

S g = {1, 2};

int main()
{
  CERASTES_CHECK(g.y == 2);  // <- here
  g.x = 6;
  CERASTES_CHECK(g.x == 6);
  return g.x < g.y;
}
