// expect: error postcondition @ L29
// expect: error check @ L44
// mode: c23, c++20
// twin: tests/memory/global_array_struct_folded.c
// Pins: 0064 items 5, 7 and 9 (audit round C2, finding 3, folded addresses): after the store through the folded pointer c to &gt.v[1].y the leaf holds 50, so the claim of the value from before the store (5) is a `check` that fails in main; and wr_wrong, which writes gt.v[0].y through e[-1] with e the folded address &gt.v[1], breaks its own postcondition (gt.v[0].y == 3), reported at the function's closing brace (the function is void). Two paths, one reason each: the folded index reaches exactly the leaf it names
// Registered from audit-C2 probes f01 (the folded stores and the checks measured on snapshot a044aff629b8; the stale check replaces its designed-false `0 == 1`, unmeasured text, the same line shape) and f01b (`wr_wrong`: postcondition at the closing brace, measured); the rows' lines are inferred from the layout
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T2 {
  int n;
  struct P v[2];
  int m;
};

static struct T2 gt = {1, {{2, 3}, {4, 5}}, 8};
static struct P g[3] = {{1, 2}, {3, 4}, {5, 6}};

void wr_wrong(void)
    CERASTES_REQUIRES(cerastes_owns(gt) && gt.v[0].y == 3)
    CERASTES_ENSURES(cerastes_owns(gt) && gt.v[0].y == 3)
{
  struct P *e = &gt.v[1];
  e[-1].y = 40;
}

int main(void)
{
  int *c = &gt.v[1].y;
  *c = 50;
  CERASTES_CHECK(gt.n == 1 && gt.v[0].x == 2 && gt.v[0].y == 3 && gt.v[1].x == 4 && gt.v[1].y == 50 && gt.m == 8);
  struct P *e = &gt.v[1];
  e->x = 40;
  int *d = &g[2].x;
  *d = 55;
  struct P *h = &g[1];
  h->y = 44;
  h[1].y = 66;
  CERASTES_CHECK(g[0].x == 1 && g[0].y == 2 && g[1].x == 3 && g[1].y == 44 && g[2].x == 55 && g[2].y == 66);
  CERASTES_CHECK(gt.v[1].y == 5);  // <- here
  return gt.m - 8;
}
