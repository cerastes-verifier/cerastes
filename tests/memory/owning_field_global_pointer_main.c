// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5, 0055 items 3, 5 (S4, question 174): an owning struct buf * global whose structure holds a block in its owning field gp->data when main ends: the global's block and the field's block are both exempt
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), verifies in both modes (new: no base form for the field of an owning global's pointee)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int *view;
};

CERASTES_OWNING static struct buf *gp = NULL;

int main(void)
{
  gp = (struct buf *)calloc(1, sizeof(struct buf));
  if (gp != NULL) {
    gp->data = (int *)malloc(sizeof(int));
    if (gp->data != NULL) {
      *gp->data = 1;
    }
  }
  return 0;
}
