// expect: error leak @ L26
// mode: c23, c++20
// twin: tests/memory/owning_field_global_pointer_main.c
// Pins: 0055 item 5 (S4): the block goes to the plain field gp->view at the same depth: not exempt at main's end
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), leak at main's closing brace
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
    gp->view = (int *)malloc(sizeof(int));
    if (gp->view != NULL) {
      *gp->view = 1;
    }
  }
  return 0;
}  // <- here
