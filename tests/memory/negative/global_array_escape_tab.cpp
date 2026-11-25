// expect: error check @ L13
// mode: c++20
// twin: tests/memory/global_array_escape.cpp
// Pins: 0063 item 9 (the escape through an element): the address of h is an element of tab, an array of external linkage whose values are unknown (another unit's initializer may write *tab[0] before main): h is unknown in main
// Pre-registered from record 0063 item 9 (Part B), before any run: the one error
#include <cerastes.h>

static int h = 7;
int *tab[2] = {&h, 0};

int main()
{
  CERASTES_CHECK(h == 7);  // <- here
  return 0;
}
