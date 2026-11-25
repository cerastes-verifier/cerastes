// expect: error check @ L16
// mode: c23, c++20
// twin: tests/memory/array_typedef_scopes.c
// Pins: 0063 item 14: the global array declared through the typedef holds the written 3 at index 2, not 2 (the one check that is wrong)
// Pre-registered from record 0063 item 14 (Part D), before any run: the one error
#include <cerastes.h>

typedef int T3[3];

static T3 g = {1, 2, 3};

int main(void)
{
  typedef int T2[2];
  T2 c = {4, 5};
  CERASTES_CHECK(g[2] == 2);  // <- here
  CERASTES_CHECK(c[1] == 5);
  return g[0] + c[0] - 5;
}
