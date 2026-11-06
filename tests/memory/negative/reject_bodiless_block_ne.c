// expect: rejected cerastes-unsupported_annotation @ L12
// mode: c23, c++20
// twin: tests/memory/result_bodiless.c
// Pins: 0052 item 2a (audit B1, b_block_ne.c; reopens d2a's hole): a bodiless result block site tied to a formal through != (r != p ? r == NULL : owns_block(r)): the block would be held twice
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) B1: rejected unsupported_annotation at the clause line
#include <cerastes.h>
#include <stdlib.h>

int *again(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(cerastes_live(p))
    CERASTES_ENSURES(r, r != p ? r == NULL : cerastes_owns_block(r));  // <- here

int main(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return 1;
  }
  *p = 1;
  int *q = again(p);
  if (q != NULL) {
    free(q);
    free(p);
    return 2;
  }
  free(p);
  return 0;
}
