// expect: rejected cerastes-owner_release_unowned @ L35
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p6.c
// Pins: 0058 item 3 ("rejected although harmless"): the leaf of a by-value struct result released directly, free(mk().data): the block is the callee's result, but the leaf of a temporary is no owner by name: owner_release_unowned at the free (the default mode verifies the program)
// Pre-registered from record 0058 item 3: rejected owner_release_unowned at the call of free
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

CERASTES_OWNING int *make(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a != NULL) {
    *a = 0;
  }
  return a;
}

struct buf mk(void)
    CERASTES_ENSURES(r, r.data == NULL || cerastes_owns_block(r.data))
{
  struct buf s = { NULL, 0 };
  s.data = make();
  return s;
}

int f(void)
    CERASTES_ENSURES(r, r == 0)
{
  free(mk().data);  // <- here
  return 0;
}
