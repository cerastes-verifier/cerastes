// expect: error invariant-preserved @ L21
// mode: c23, c++20
// twin: tests/memory/global_pointer_moved_loop.c
// Pins: 0045 item 10: the invariant claims *gp == 1, the swap makes *gp the other cell (2): invariant-preserved at the clause
#include <cerastes.h>
#include <stddef.h>

static int *gp;

void f(void)
    CERASTES_REQUIRES(cerastes_owns(gp))
    CERASTES_ENSURES(cerastes_owns(gp) && gp == NULL)
{
  int x = 1;
  int y = 2;
  int *r = &y;
  gp = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_INVARIANT(cerastes_owns(*gp) && cerastes_owns(*r) && ((gp == &x && r == &y) || (gp == &y && r == &x)))
  CERASTES_INVARIANT(*gp == 1)  // <- here
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    int *t = gp;
    gp = r;
    r = t;
    i = i + 1;
  }
  gp = NULL;
}
