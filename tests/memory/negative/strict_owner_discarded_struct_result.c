// expect: error leak @ L37
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p4.c
// Pins: 0058 item 3, audit S7 ("accepted although wrong"): a discarded by-value struct result with an owning leaf: the struct result is not a gives_result_block call for p4, and the default mode reports the leaked block
// Pre-registered from record 0058 item 3 (audit round 1, S7): one leak under --strict-owner at the closing brace (observed: build/m4.5d2b2d/tests-A runs)
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
  mk();
  return 0;
}  // <- here
