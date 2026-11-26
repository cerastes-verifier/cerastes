// expect: error permission @ L26
// mode: c23, c++20
// twin: tests/memory/global_array_struct_calls.c
// Pins: 0064 items 7 and 9 (the alias twin): pair(&g[1].x) against requires(owns(*p) && owns(g)): the cell *p is a leaf of an element of g, so the call's separation row fails (permission at the call); the positive passes another object
// Pre-registered from record 0064 items 7 and 9 (Part C2), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

static struct P g[3] = {{1, 2}, {3, 4}};

int pair(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(g) && *p == 1 && g[0].x == 2 && r == 0)
{
  *p = 1;
  g[0].x = 2;
  return 0;
}

int main(void)
{
  pair(&g[1].x);  // <- here
  return 0;
}
