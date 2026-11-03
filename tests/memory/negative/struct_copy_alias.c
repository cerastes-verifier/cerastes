// expect: error permission @ L29
// mode: c23, c++20
// twin: tests/memory/struct_copy.c
// Pins: 0043 items 7, 9: the destination and the source are the one object s: the separation of the two owns atoms fails at the call
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void copy(struct S *dst, struct S *src)
    CERASTES_REQUIRES(cerastes_owns(*dst) && cerastes_owns(*src))
    CERASTES_ENSURES(cerastes_owns(*dst) && cerastes_owns(*src) && dst->x == src->x && dst->y == src->y && src->x == cerastes_old(src->x) && src->y == cerastes_old(src->y))
{
  *dst = *src;
}

void self_copy(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->x == cerastes_old(p->x) && p->y == cerastes_old(p->y))
{
  *p = *p;
}

int use_copy(void)
{
  struct S s = {1, 2};
  copy(&s, &s);  // <- here
  return s.x;
}
