// expect: error leak @ L26
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p6.c
// Pins: 0058 item 3 ("accepted although wrong"): a release on one path only, if (c) free(v);: the owner by name is released by a call that p6 accepts, and the default mode's leak reports the other path
// Pre-registered from record 0058 item 3: one leak at the return under --strict-owner
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

int f(int c)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING int *v = make();
  if (c) {
    free(v);
  }
  return 0;  // <- here
}
