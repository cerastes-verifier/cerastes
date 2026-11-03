// expect: error check @ L20
// twin: tests/memory/struct_global_escape_cpp.cpp
// Pins: 0044 item 8 (audit N4): T t = {&g} has external linkage in C++: another unit may run its initialization first, so g's value is unknown in main (the escape rule per leaf)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct T {
  S *p;
};

static S g{};
T t = {&g};

int main()
{
  CERASTES_CHECK(g.x == 0 && g.y == 0);  // <- here
  return g.x;
}
