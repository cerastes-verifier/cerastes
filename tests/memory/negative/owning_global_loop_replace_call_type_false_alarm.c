// expect: error invalid-free @ L37
// expect: error invalid-free @ L37
// expect: error invalid-free @ L37
// expect: error lifetime @ L39
// expect: error permission @ L42
// expect: error lifetime @ L43
// expect: error permission @ L44
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_global_pointer.c
// Pins: 0056 item 6 (S3: the replace test of an owning global; a false alarm pinned, the program is correct): a loop that frees gp->data in its first iteration and frees and replaces the owning struct buf * global gp by a call whose contract covers gp's cell by type (`renew(&gp)`, requires and ensures `owns(*pp)` with a `struct buf **pp`): a closure of gp->data carried across the replacement would be unsound, so none is carried and the free of gp->data is reported (invalid-free), as for an owning local (owning_field_loop_owner_replaced_no_invariant.c)
// Pre-registered from record 0056 item 6 (no closure carried across a replacement of the owner) and audit round 1 S3; the expect lines are the outcome of the L-1 build, which has no closure for gp at all and which Part L-2 must not change for these programs: the invalid-free x3 at free(gp->data) is the designed content
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

CERASTES_OWNING static struct buf *gp = NULL;

void renew(struct buf **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && (*pp == NULL || (cerastes_owns(**pp) && (*pp)->data == NULL)))
    CERASTES_ENSURES(cerastes_owns(*pp));

int main(void)
{
  gp = (struct buf *)calloc(1, sizeof(struct buf));
  if (gp == NULL) {
    return 1;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (gp != NULL && i == 0) {
      free(gp->data);
    }
    if (gp != NULL) {
      free(gp);
    }
    renew(&gp);
    if (gp != NULL) {
      gp->n = i;
    }
    i = i + 1;
  }
  if (gp != NULL) {
    free(gp);
  }
  return 0;
}
