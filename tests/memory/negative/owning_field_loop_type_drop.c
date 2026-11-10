// expect: error leak @ L43
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_type.c
// Pins: 0055 item 5: after the loop the function sets the global's owning leaf to NULL without freeing the block it holds: the reassignment row at the store
// Pre-registered from record 0055 item 5 (the il-analyst's intent): leak at the store
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf g;

void bump(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp))
{
  (void)pp;
}

int tick(int i)
    CERASTES_REQUIRES(0 <= i && i < 10)
    CERASTES_ENSURES(r, r == i + 1)
{
  return i + 1;
}

void f(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  int *z = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    bump(&z);
    g.n = i;
    i = tick(i);
  }
  g.data = NULL;  // <- here
}
