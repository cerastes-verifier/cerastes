// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/ptrptr_owns_inner.c
// Pins: 0045 item 6: **pp and *q are the one cell x: the separation row at the call fails (permission)
#include <cerastes.h>
#include <stddef.h>

void set2(int **pp, int *q)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp) && cerastes_owns(*q));

int use(void)
{
  int x = 0;
  int *p = &x;
  set2(&p, &x);  // <- here
  return x;
}
