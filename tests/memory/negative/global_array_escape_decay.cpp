// expect: error check @ L13
// mode: c++20
// twin: tests/memory/global_array_escape.cpp
// Pins: 0063 item 9 (the other direction): the decay of g is the initial value of gp, a global of external linkage: another unit's initializer may write gp[1] before main, so every element of g is unknown
// Pre-registered from record 0063 item 9 (Part B), before any run: the one error
#include <cerastes.h>

static int g[3] = {1, 2, 3};
int *gp = g;

int main()
{
  CERASTES_CHECK(g[1] == 2);  // <- here
  return 0;
}
