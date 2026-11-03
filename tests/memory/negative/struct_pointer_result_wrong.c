// expect: error postcondition @ L16
// mode: c23, c++20
// twin: tests/memory/struct_pointer_result.c
// Pins: 0043 item 8: the arms of the result are swapped
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct S *pick(struct S *a, struct S *b, int c)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b))
    CERASTES_ENSURES(r, cerastes_owns(*a) && cerastes_owns(*b) && r == (c != 0 ? a : b) && a->x == cerastes_old(a->x) && a->y == cerastes_old(a->y) && b->x == cerastes_old(b->x) && b->y == cerastes_old(b->y))
{
  return c != 0 ? b : a;  // <- here
}
