// expect: error unsequenced @ L24
// mode: c23, c++20
// twin: tests/memory/owns_block_load_seq.c
// Pins: 0054 item 2 and audit round 2 item 2 (probe s2): a write of *a beside the consuming call
// Pre-registered from the audit probe s2_seq_write_beside_consume as observed: unsequenced at the statement
#include <cerastes.h>
#include <stdlib.h>

int consume(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns_block(*pp))
    CERASTES_ENSURES(r, cerastes_owns(*pp) && r == 0)
{
  free(*pp);
  return 0;
}

int use(void)
{
  int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return 1;
  }
  *a = 1;
  int r = (*a = 2) + consume(&a);  // <- here
  return r;
}
