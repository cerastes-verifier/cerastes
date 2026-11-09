// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/owns_block_load_two_tied.c
// Pins: 0054 item 2 ("separation rows between every pair of block sites") and audit round 2 item 2 (probe p10): the caller passes the same pointer cell twice, so the two sites name one cell and one block: the call needs permissions the caller cannot give twice
// Pre-registered from derived from the rules (not observed): permission at the call
#include <cerastes.h>
#include <stdlib.h>

void both(int **pp, int **qq)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(*qq) && cerastes_owns_block(*pp) && cerastes_owns_block(*qq))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns(*qq) && cerastes_owns_block(*pp) && cerastes_owns_block(*qq) && *pp == cerastes_old(*pp) && *qq == cerastes_old(*qq));

int use(void)
{
  int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return 1;
  }
  *a = 1;
  both(&a, &a);  // <- here
  free(a);
  return 0;
}
