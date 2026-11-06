// expect: error uninitialized @ L24
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/strict_marked_returns.c
// Pins: 0052 item 4: the marked function's result has no value (its block comes from malloc through a marked local): the caller's read before its write is uninitialized, under --strict-init as without it
// Pre-registered from record 0052 item 4: uninitialized at the read
#include <cerastes.h>
#include <stdlib.h>

CERASTES_REF_TO_UNINIT int *raw(void)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns_block(r))
{
  CERASTES_REF_TO_UNINIT int *p = (int *)malloc(sizeof(int));
  return p;
}

int use_raw(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_REF_TO_UNINIT int *p = raw();
  if (p == NULL) {
    return -1;
  }
  int v = *p;  // <- here
  *p = v;
  free(p);
  return 0;
}
