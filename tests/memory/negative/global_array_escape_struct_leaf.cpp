// expect: error check @ L18
// mode: c++20
// twin: tests/memory/global_array_escape.cpp
// Pins: 0063 item 9 (audit round 2, S1; the record's own example): the decay of g is the initial value of a leaf of the structure global ps, which has external linkage: another unit's initializer may write through ps.p before main, so every element of g is unknown
// Pre-registered from audit round 2 of record 0063 (S1), before any run: the one error
#include <cerastes.h>

static int g[3] = {1, 2, 3};

struct P {
  int *p;
};

P ps = {g};

int main()
{
  CERASTES_CHECK(g[1] == 2);  // <- here
  return 0;
}
