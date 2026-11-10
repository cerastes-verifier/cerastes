// expect: error uninitialized @ L38
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_site_addr_null_written.c
// Pins: 0056 item 2, audit round 2 B1 (address-taken variable route): an owning address-taken local int *v is NULL at the loop's entry and the loop writes it by name (`v = malloc(...)`) with a block it never writes: the route is seen, wef is false and the read after the loop is uninitialized. A scan that drops the route makes wef true and moves the error to the loop's back-edge row
// Pre-registered from the auditor's model (build/m4.5d2b2c/t7/model_addr.c, run on the L-1 build): uninitialized at the read `x = *v;` in both modes; with wef flipped by hand the error moved to the back-edge row (auditor, round 2 B1)
#include <cerastes.h>
#include <stdlib.h>

int tick(int i)
    CERASTES_REQUIRES(0 <= i && i < 10)
    CERASTES_ENSURES(r, r == i + 1)
{
  return i + 1;
}

void keep(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp))
{
  (void)pp;
}

int f(void)
{
  int *v CERASTES_OWNING = NULL;
  int **pv = &v;
  keep(pv);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    free(v);
    v = (int *)malloc(sizeof(int));
    i = tick(i);
  }
  int x = 0;
  if (v != NULL) {
    x = *v;
  }
  free(v);
  v = NULL;
  return x;
}
