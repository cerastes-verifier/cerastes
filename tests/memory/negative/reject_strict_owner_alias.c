// expect: rejected cerastes-owner_release_unowned @ L23
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p6.c
// Pins: 0058 item 3 (p6, "rejected although harmless"): an alias of an owner is released: t = v; free(t) frees the block of v, which the default mode accepts, but t is no owner by name: owner_release_unowned at the free
// Pre-registered from record 0058 item 3: rejected owner_release_unowned at the call of free
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
{
  CERASTES_OWNING int *v = make();
  int *t = v;
  free(t);  // <- here
  return 0;
}
