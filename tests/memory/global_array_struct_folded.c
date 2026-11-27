// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 5, 7 and 9 (audit round C2, finding 3, folded addresses): the address of a leaf or an element of a global structure's member array (&gt.v[1].y, &gt.v[1], &gt.v[0]) and of an element of a global array of structures (&g[2].x, &g[1], then h[1]) is a folded index into the held place: stores through the pointers reach exactly that leaf (the values of the other leaves stay), in main with the program's initial values and in a callee that owns gt. The twin negative/global_array_struct_folded_stale.c claims the value from before the store, and a callee that writes the neighbouring element through e[-1]
// Registered from audit-C2 probes f01 (.c and .cpp, both measured on snapshot a044aff629b8: the four checks verified, the final designed-false `0 == 1` failed at its line; it is dropped here) and f01b (`wr` verified; `wr_wrong` and the second check are the twin's); the final return is added (g read in code), unmeasured
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

void wr(void)
    CERASTES_REQUIRES(cerastes_owns(gt))
    CERASTES_ENSURES(cerastes_owns(gt) && gt.v[1].y == 50 && gt.v[0].x == 40 && gt.n == cerastes_old(gt.n) && gt.m == cerastes_old(gt.m))
{
  int *c = &gt.v[1].y;
  *c = 50;
  struct P *e = &gt.v[0];
  e->x = 40;
}

int main(void)
{
  int *c = &gt.v[1].y;
  *c = 50;
  CERASTES_CHECK(gt.n == 1 && gt.v[0].x == 2 && gt.v[0].y == 3 && gt.v[1].x == 4 && gt.v[1].y == 50 && gt.m == 8);
  struct P *e = &gt.v[1];
  e->x = 40;
  CERASTES_CHECK(gt.n == 1 && gt.v[0].x == 2 && gt.v[0].y == 3 && gt.v[1].x == 40 && gt.v[1].y == 50 && gt.m == 8);
  int *d = &g[2].x;
  *d = 55;
  struct P *h = &g[1];
  h->y = 44;
  h[1].y = 66;
  CERASTES_CHECK(g[0].x == 1 && g[0].y == 2 && g[1].x == 3 && g[1].y == 44 && g[2].x == 55 && g[2].y == 66);
  int *m = &gt.m;
  *m = 9;
  CERASTES_CHECK(gt.m == 9 && gt.v[1].y == 50 && gt.n == 1);
  return gt.m - 9;
}
