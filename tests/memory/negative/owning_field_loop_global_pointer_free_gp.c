// expect: error permission @ L32
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_global_pointer.c
// Pins: 0056 items 6 and 7, audit round 1 S1: a loop over an owning struct buf * global (gp itself is freed, gp->data first) that frees the block the carried sites hang from in one iteration (i == 1): the carried sites read through a freed block at the back edge, which must be a mapped error at the loop and never an exit 3 (the unmapped Silicon `assignment.failed` of a pre-read placed before the rows of the site it reads through)
// Pre-registered from audit round 1 S1 (not yet run on the L-2 build; the base build exits 3 on the depth-2 form): a mapped permission error at the closing brace of the loop body, in both modes, exit status 1; question 160 applies to the line and the category within permission/leak/lifetime, and an exit 3 is a stop
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
    if (i == 1) {
      free(gp->data);
      gp->data = NULL;
      free(gp);
    }
    i += 1;
  }
  return 0;
}
