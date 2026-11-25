// expect: error check @ L13
// mode: c++20
// twin: tests/memory/global_array_escape_elem_decay_static.cpp
// Pins: 0063 item 9 (audit round 2, S1, p03): an external array whose element is the decay of g: another unit's initializer may write through tab[0] before main, so g is unknown
// Pre-registered from audit round 2 of record 0063 (S1), before any run: the one error
#include <cerastes.h>

static int g[2] = {1, 2};
int *tab[1] = {g};

int main()
{
  CERASTES_CHECK(g[0] == 1);  // <- here
  return 0;
}
