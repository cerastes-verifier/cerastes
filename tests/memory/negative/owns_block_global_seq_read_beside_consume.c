// expect: error unsequenced @ L26
// mode: c23, c++20
// twin: tests/memory/owns_block_global_seq.c
// Pins: 0054 item 2 and audit round 2 item 2 (probe s3): a read of *g beside a call that consumes the global's block
// Pre-registered from the audit probe s3_seq_global as observed: unsequenced at the statement
#include <cerastes.h>
#include <stdlib.h>

static int *g = NULL;

int consume(void)
    CERASTES_REQUIRES(cerastes_owns(g) && cerastes_owns_block(g))
    CERASTES_ENSURES(r, cerastes_owns(g) && r == 0)
{
  free(g);
  return 0;
}

int main(void)
{
  g = (int *)malloc(sizeof(int));
  if (g == NULL) {
    return 1;
  }
  *g = 1;
  int r = *g + consume();  // <- here
  return r;
}
