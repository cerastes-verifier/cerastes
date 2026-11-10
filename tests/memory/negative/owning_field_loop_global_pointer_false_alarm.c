// expect: error invalid-free @ L31
// expect: error invalid-free @ L31
// expect: error invalid-free @ L31
// expect: error leak @ L37
// expect: error permission @ L37
// expect: error permission @ L37
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_global_pointer_n.c
// Pins: 0055 item 7 (a false alarm pinned, not a defect of the program): a loop that frees and replaces gp->data of an owning struct buf * global is rejected today: the loop's places do not include the cells of an owning global pointer's block, so the free of gp->data is not backed by anything the loop holds, and a hand-written invariant fails the same way; a loop over an owning local struct pointer, or one that touches only a non-owning field of the global, verifies (owning_field_loop_owning_local.c, owning_field_loop_global_pointer_n.c). The expectations are the errors the encoder reports today; lifting them is a later session's (the deferred list)
// Pre-registered from the il-analyst's Part D probes (il/p r1, r2, r3, lg6, lg): lg.c, observed on aux 3eb1d23: the expect lines are the actual outcome of the program, recorded as a pin
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
  CERASTES_DECREASES(5 - i)
  while (i < 5) {
    free(gp->data);
    gp->data = (int *)malloc(sizeof(int));
    if (gp->data != NULL) {
      *gp->data = i;
    }
    i += 1;
  }
  return 0;
}
