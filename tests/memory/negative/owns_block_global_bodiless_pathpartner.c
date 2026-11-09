// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/owns_block_global_bodiless.c
// Pins: 0054 item 2 ("Bodiless contracts", the tie) and audit round 2 item 2 (probe p4): the same path-dependent partner for a block site on a global
// Pre-registered from the audit probe p4_global_pathpartner (audit round 2 B1): permission at the ensures clause line (f__wf's row); the probe VERIFIED when this was written
#include <cerastes.h>
#include <stdlib.h>

static int *g = NULL;

void f(int **qq)
    CERASTES_REQUIRES(cerastes_owns(g) && cerastes_owns(*qq) && (*qq == NULL || cerastes_owns_block(g)))
    CERASTES_ENSURES(cerastes_owns(g) && cerastes_owns(*qq) && cerastes_owns_block(g) && g == cerastes_old(g));  // <- here

int main(void)
{
  g = (int *)malloc(sizeof(int));
  if (g == NULL) {
    return 1;
  }
  *g = 1;
  int *b = NULL;
  f(&b);
  free(g);
  free(g);
  return 0;
}
