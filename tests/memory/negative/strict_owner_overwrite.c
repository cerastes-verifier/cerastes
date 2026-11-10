// expect: error leak @ L23
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p4.c
// Pins: 0058 item 3 ("accepted although wrong"): an assignment to an owner that still holds a block: v = make() twice is p4-accepted both times, and the default mode's leak reports the assignment
// Pre-registered from record 0058 item 3: one leak at the second assignment under --strict-owner
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING int *make(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a != NULL) {
    *a = 0;
  }
  return a;
}

int f(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING int *v = make();
  v = make();  // <- here
  free(v);
  return 0;
}
