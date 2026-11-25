// expect: error check @ L14
// mode: c++20
// twin: tests/memory/global_array_escape.cpp
// Pins: 0063 item 9 (audit round 2, S1): transitively: the decay of the pointer array tab is the initial value of gp (external linkage), tab's element holds &h, so h is unknown in main
// Pre-registered from audit round 2 of record 0063 (S1), before any run: the one error
#include <cerastes.h>

static int h = 5;
static int *tab[1] = {&h};
int **gp = tab;

int main()
{
  CERASTES_CHECK(h == 5);  // <- here
  return 0;
}
