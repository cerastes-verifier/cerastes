// expect: error leak @ L18
// mode: c23, c++20
// twin: tests/memory/owns_block_load_owner_parked.c
// Pins: 0054 item 2 and audit round 2 item 2 (probe o2): the owner is not parked in *pp (*pp = NULL): the ensures site gives nothing, and the owner still holds its block when its lifetime ends
// Pre-registered from the audit probe o2_owner_not_parked as observed: leak at the closing brace (the owner's exit)
#include <cerastes.h>
#include <stdlib.h>

void make(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp)))
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v != NULL) {
    *v = 1;
  }
  *pp = NULL;
}  // <- here
