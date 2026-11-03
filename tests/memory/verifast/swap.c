// expect: verified
// expect: native-ok
// mode: c23, c++20
// Port of VeriFast's examples/swap.c `swap` (L4-11; ~/Documents/verifast at 0fb63506) to cerastes (decision 0040, M4.5b1c3).
// Adaptation: `//@ requires integer(a, ?x) &*& integer(b, ?y)` / `ensures integer(a, y) &*& integer(b, x)` become `cerastes_owns` of both cells and `*a == cerastes_old(*b) && *b == cerastes_old(*a)`; `tmp` is `t`.  `point_mirror`'s `//@ requires p->x |-> ?x &*& p->y |-> ?y` / `ensures p->x |-> y &*& p->y |-> x` (decision 0043, M4.5c1) become `cerastes_owns(p->x) && cerastes_owns(p->y)` and `p->x == cerastes_old(p->y) && p->y == cerastes_old(p->x)`; `use_mirror` is a caller.
// Left out: `main` (`malloc`, M4.5d2).  `use_swap`, `swap_twice` and `use_mirror` are callers; the aliasing traps are tests/memory/negative/swap_vf_alias.c (`swap(&x, &x)`) and tests/memory/negative/swap_vf_point_alias.c (`swap(&p->x, &p->x)`).
// Line map (this file : VeriFast's):  10 : 4   14 : 8   15 : 9   16 : 10   17 : 11   37 : 13   42 : 18   46 : 22
#include <cerastes.h>

void swap(int *a, int *b)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*a) && cerastes_owns(*b) && *a == cerastes_old(*b) && *b == cerastes_old(*a))
{
  int t = *a;
  *a = *b;
  *b = t;
}

void swap_twice(int *a, int *b)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b))
    CERASTES_ENSURES(cerastes_owns(*a) && cerastes_owns(*b) && *a == cerastes_old(*a) && *b == cerastes_old(*b))
{
  swap(a, b);
  swap(a, b);
}

int use_swap(void)
    CERASTES_ENSURES(r, r == 21)
{
  int x = 1;
  int y = 2;
  swap(&x, &y);
  swap_twice(&x, &y);
  return x * 10 + y;
}

struct point {
  int x;
  int y;
};

void point_mirror(struct point *p)
    CERASTES_REQUIRES(cerastes_owns(p->x) && cerastes_owns(p->y))
    CERASTES_ENSURES(cerastes_owns(p->x) && cerastes_owns(p->y) && p->x == cerastes_old(p->y) && p->y == cerastes_old(p->x))
{
  swap(&p->x, &p->y);
}

int use_mirror(void)
    CERASTES_ENSURES(r, r == 21)
{
  struct point q = {1, 2};
  point_mirror(&q);
  return q.x * 10 + q.y;
}
