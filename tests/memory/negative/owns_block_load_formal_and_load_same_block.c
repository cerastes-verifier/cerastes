// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/owns_block_load_formal_and_load.c
// Pins: 0054 item 2 and audit round 2 item 2 (probe p5): the formal p and *pp name the same block (f(a, &a)): the block is needed twice
// Pre-registered from derived from the rules (not observed): permission at the call
#include <cerastes.h>
#include <stdlib.h>

void f(int *p, int **pp)
    CERASTES_REQUIRES(cerastes_owns_block(p) && cerastes_owns(*pp) && cerastes_owns_block(*pp))
    CERASTES_ENSURES(cerastes_owns_block(p) && cerastes_owns(*pp) && cerastes_owns_block(*pp) && *pp == cerastes_old(*pp));

int use(void)
{
  int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return 1;
  }
  *a = 1;
  f(a, &a);  // <- here
  free(a);
  return 0;
}
