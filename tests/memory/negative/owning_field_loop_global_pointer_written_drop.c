// expect: error leak @ L32
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_global_pointer_written.c
// Pins: 0056 item 6 (limit 3): the same loop without free(gp->data): an iteration ends owning the block of gp->data it replaced, which the loop does not carry, so the back-edge row reports it at the loop body's closing brace
// Pre-registered from record 0056 item 6 and S41 (a) (the twin of the hand-edited fixture, not yet run on the L-2 build): leak at the closing brace of the loop body, in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

CERASTES_OWNING static struct buf *gp = NULL;

int main(void)
{
  gp = (struct buf *)calloc(1, sizeof(struct buf));
  if (gp == NULL) {
    return 1;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 5)
  CERASTES_INVARIANT(gp == NULL || cerastes_owns_block(gp))
  CERASTES_DECREASES(5 - i)
  while (i < 5) {
    gp->data = (int *)malloc(sizeof(int));
    if (gp->data != NULL) {
      *gp->data = i;
    }
    i += 1;
  }
  return 0;
}
