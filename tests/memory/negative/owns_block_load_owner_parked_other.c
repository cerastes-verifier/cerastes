// expect: error leak @ L19
// mode: c23, c++20
// twin: tests/memory/owns_block_load_owner_parked.c
// Pins: 0054 item 2 and audit round 2 item 2 (probe o3): the owner is parked in *qq, which has no block site in the ensures: only the site on *pp counts as a returned owner, so the block leaks
// Pre-registered from the audit probe o3_owner_parked_other as observed: leak at the closing brace
#include <cerastes.h>
#include <stdlib.h>

void make(int **pp, int **qq)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(*qq))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns(*qq) && (*pp == NULL || cerastes_owns_block(*pp)))
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v != NULL) {
    *v = 1;
  }
  *pp = NULL;
  *qq = v;
}  // <- here
