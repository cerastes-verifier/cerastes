// expect: error check @ L18
// mode: c++20
// twin: tests/memory/global_array_struct_escape.cpp
// Pins: 0064 item 9 (the escape through a pointer leaf of an element): the address of h is the pointer leaf of an element of tab, an array of structures of external linkage whose values are unknown (another unit's initializer may write *tab[0].p before main): h is unknown in main
// Pre-registered from record 0064 item 9 (Part C2), before any run: the one error
#include <cerastes.h>

struct Q {
  int *p;
  int y;
};

static int h = 7;
struct Q tab[2] = {{&h, 1}, {0, 2}};

int main()
{
  CERASTES_CHECK(h == 7);  // <- here
  return 0;
}
