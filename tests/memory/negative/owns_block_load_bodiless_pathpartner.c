// expect: error permission @ L11
// mode: c23, c++20
// twin: tests/memory/owns_block_load_bodiless_tied.c
// Pins: 0054 item 2 ("Bodiless contracts", the tie) and audit round 2 item 2 (probe p1): the requires takes the block of *pp only where *qq != NULL, the ensures returns it unconditionally: on the other path the callee returns a block it never took (the caller then frees it twice and everything verifies)
// Pre-registered from the audit probe p1_pathpartner (audit round 2 B1): permission at the ensures clause line (f__wf's row); the probe VERIFIED when this was written, so the test fails until the fix lands
#include <cerastes.h>
#include <stdlib.h>

void f(int **pp, int **qq)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(*qq) && (*qq == NULL || cerastes_owns_block(*pp)))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns(*qq) && cerastes_owns_block(*pp) && *pp == cerastes_old(*pp));  // <- here

int use(void)
{
  int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return 1;
  }
  *a = 1;
  int *b = NULL;
  f(&a, &b);
  free(a);
  free(a);
  return 0;
}
