// expect: verified
// expect: native-ok
// mode: c23, c++20
// Port of VeriFast's examples/swap.c `swap` (L4-11; ~/Documents/verifast at 0fb63506) to cerastes (decision 0040, M4.5b1c3).
// Adaptation: `//@ requires integer(a, ?x) &*& integer(b, ?y)` / `ensures integer(a, y) &*& integer(b, x)` become `cerastes_owns` of both cells and `*a == cerastes_old(*b) && *b == cerastes_old(*a)`; `tmp` is `t`.
// Left out: `point_mirror` (a struct, M4.5c) and `main` (`malloc`, M4.5d2).  `use_swap` and `swap_twice` are callers; the aliasing trap `swap(&x, &x)` is tests/memory/negative/swap_vf_alias.c.
// Line map (this file : VeriFast's):  10 : 4   14 : 8   15 : 9   16 : 10   17 : 11
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
