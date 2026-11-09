// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/owns_block_load_given.c
// Pins: 0054 item 2: a given site has no amount, so the callee must hold the whole block at its return; this one freed it after storing it in *pp (the contract gives no value claim, so nothing else fails)
// Pre-registered from record 0054 item 2 ("given", S39 a1x): permission at the closing brace of the void function (the block site's return row; as block_site_given_not_held.c)
#include <cerastes.h>
#include <stdlib.h>

void make(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_block(*pp))
{
  *pp = (int *)malloc(sizeof(int));
  if (*pp == NULL) {
    abort();
  }
  **pp = 5;
  free(*pp);
}  // <- here
