// expect: error unsequenced @ L23
// mode: c23, c++20
// twin: tests/memory/heap_seq_distinct.c
// Pins: 0047 item 9 (audit B3): a block site is a full-permission atom on every leaf cell of its block for the pair checks, so the read of *p and the free are unsequenced in either evaluation order (the other order is heap_seq_free_deref.c)
// Pre-registered from record 0047 item 9: unsequenced at the expression; re-registered after audit round 1 (record 0047 item 9): both orders of the free and the read
#include <cerastes.h>
#include <stdlib.h>

int use_live(int *q)
    CERASTES_REQUIRES(cerastes_live(q))
    CERASTES_ENSURES(r, cerastes_live(q) && r == 0);

int distinct(void)
{
  int *p = (int *)malloc(sizeof(int));
  int *q = (int *)malloc(sizeof(int));
  if (p == NULL || q == NULL) {
    free(p);
    free(q);
    return -1;
  }
  *p = 1;
  int s = *p + (free(p), 0);  // <- here
  free(q);
  return s;
}
