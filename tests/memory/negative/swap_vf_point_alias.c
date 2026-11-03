// expect: error permission @ L25
// mode: c23, c++20
// twin: tests/memory/verifast/swap.c
// Pins: 0043 items 5, 9 and the port of VeriFast's examples/swap.c point_mirror: swap(&p->x, &p->x) passes the one leaf as two full permissions: the pairwise separation fails at the call
#include <cerastes.h>

void swap(int *a, int *b)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*a) && cerastes_owns(*b) && *a == cerastes_old(*b) && *b == cerastes_old(*a))
{
  int t = *a;
  *a = *b;
  *b = t;
}

struct point {
  int x;
  int y;
};

void point_mirror(struct point *p)
    CERASTES_REQUIRES(cerastes_owns(p->x) && cerastes_owns(p->y))
    CERASTES_ENSURES(cerastes_owns(p->x) && cerastes_owns(p->y))
{
  swap(&p->x, &p->x);
}
