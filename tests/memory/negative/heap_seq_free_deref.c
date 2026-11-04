// expect: error unsequenced @ L23
// mode: c23, c++20
// twin: tests/memory/heap_seq_distinct.c
// Pins: 0047 item 9: the free consumes p's token and liveness cell while the other operand uses p (a read of the cell): unsequenced, path-precise
// Pre-registered from record 0047 item 9 (unsequenced over the liveness and token cells; cut 3 would make it unsequenced_calls): unsequenced at the expression
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
  int s = (free(p), 0) + *p;  // <- here
  free(q);
  return s;
}
